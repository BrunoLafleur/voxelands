/************************************************************************
* Minetest-c55
* Copyright (C) 2010-2011 celeron55, Perttu Ahola <celeron55@gmail.com>
*
* environment.cpp
* voxelands - 3d voxel world sandbox game
* Copyright (C) Lisa 'darkrose' Milne 2013-2014 <lisa@ltmnet.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>
*
* License updated from GPLv2 or later to GPLv3 or later by Lisa Milne
* for Voxelands.
************************************************************************/

#include "common.h"

#include "environment.h"
#include "porting.h"
#include "collision.h"
#include "content_mapnode.h"
#include "mineral.h"
#include "nodemetadata.h"
#include "mapblock.h"
#include "serverobject.h"
#include "content_sao.h"
#include "content_mob.h"
#include "plantgrowth.h"
#include "log.h"
#include "profiler.h"
#include "server.h"
#include "client.h"

#include "nvp.h"
#include "path.h"

#define PP(x) "("<<(x).X<<","<<(x).Y<<","<<(x).Z<<")"

Environment::Environment():
	m_time(0),
	m_time_of_day(9000),
	m_time_of_day_f(9000./24000),
	m_time_of_day_speed(0),
	m_time_counter(0)
{
	m_players = array_create(ARRAY_TYPE_PTR);
}

Environment::~Environment()
{
	// Deallocate players
	Player *player;
	uint32_t i;
	for (i=0; i<m_players->length; i++) {
		player = (Player*)array_get_ptr(m_players,i);
		if (!player)
			continue;
		array_set_ptr(m_players,NULL,i);
		delete player;
	}

	array_free(m_players,1);
}

void Environment::addPlayer(Player *player)
{
	DSTACK(__FUNCTION_NAME);
	/*
		Check that peer_ids are unique.
		Also check that names are unique.
		Exception: there can be multiple players with peer_id=0
	*/
	// If peer id is non-zero, it has to be unique.
	if (player->peer_id != 0 && getPlayer(player->peer_id) != NULL)
		return;
	// Name has to be unique.
	if (getPlayer(player->getName()) != NULL)
		return;

	array_insert_ptr(m_players,player);
}

void Environment::removePlayer(u16 peer_id)
{
	DSTACK(__FUNCTION_NAME);
	Player *player;
	uint32_t i;
	int k;

	do {
		k = 0;
		for (i=0; i<m_players->length; i++) {
			player = (Player*)array_get_ptr(m_players,i);
			if (!player)
				continue;
			if (player->peer_id != peer_id)
				continue;
			array_set_ptr(m_players,NULL,i);
			delete player;
			k = 1;
		}
	} while (k > 0);
}

Player * Environment::getPlayer(u16 peer_id)
{
	Player *player;
	uint32_t i;
	for (i=0; i<m_players->length; i++) {
		player = (Player*)array_get_ptr(m_players,i);
		if (!player)
			continue;
		if (player->peer_id == peer_id)
			return player;
	}
	return NULL;
}

Player * Environment::getPlayer(const char *name)
{
	Player *player;
	uint32_t i;
	for (i=0; i<m_players->length; i++) {
		player = (Player*)array_get_ptr(m_players,i);
		if (!player)
			continue;
		if (!strcmp(player->getName(), name))
			return player;
	}
	return NULL;
}

Player * Environment::getRandomConnectedPlayer()
{
	array_t *connected_players = getPlayers(true);
	uint32_t i = myrand() % connected_players->length;
	Player *r = (Player*)array_get_ptr(connected_players,i);
	array_free(connected_players,1);
	return r;
}

Player * Environment::getNearestConnectedPlayer(v3f pos)
{
	array_t *connected_players = getPlayers(true);
	float nearest_d = 0;
	Player *nearest_player = NULL;
	Player *player;
	uint32_t i;
	for (i=0; i<connected_players->length; i++) {
		player = (Player*)array_get_ptr(connected_players,i);
		if (!player)
			continue;
		float d = player->getPosition().getDistanceFrom(pos);
		if (d < nearest_d || nearest_player == NULL) {
			nearest_d = d;
			nearest_player = player;
		}
	}
	return nearest_player;
}

array_t *Environment::getPlayers()
{
	return m_players;
}

array_t *Environment::getPlayers(bool ignore_disconnected)
{
	array_t *newlist;
	uint32_t i;
	Player *player;

	newlist = array_create(ARRAY_TYPE_PTR);

	for (i=0; i<m_players->length; i++) {
		player = (Player*)array_get_ptr(m_players,i);
		if (!player)
			continue;

		if (ignore_disconnected && player->peer_id == 0)
			continue;

		array_push_ptr(newlist,player);
	}
	return newlist;
}

void Environment::printPlayers(std::ostream &o)
{
	o<<"Players in environment:"<<std::endl;
	uint32_t i;
	Player *player;
	for (i=0; i<m_players->length; i++) {
		player = (Player*)array_get_ptr(m_players,i);
		if (!player)
			continue;
		o<<"Player peer_id="<<player->peer_id<<std::endl;
	}
}

u32 Environment::getDayNightRatio()
{
	return time_to_daynight_ratio(m_time_of_day_f*24000.0);
}

float Environment::stepTimeOfDay(float dtime)
{
	float r = m_time_of_day_f;
	m_time_counter += dtime;
	f32 speed = m_time_of_day_speed * 24000./(24.*3600);
	u32 units = (u32)(m_time_counter*speed);
	m_time_counter -= (f32)units / speed;
	bool sync_f = false;
	if (units > 0) {
		// Sync at overflow
		if (m_time_of_day + units >= 24000) {
			m_time++;
			sync_f = true;
		}
		m_time_of_day = (m_time_of_day + units) % 24000;
		if (sync_f)
			m_time_of_day_f = (float)m_time_of_day / 24000.0;
	}

	if (!sync_f) {
		m_time_of_day_f += m_time_of_day_speed/24/3600*dtime;
		if (m_time_of_day_f > 1.0)
			m_time_of_day_f -= 1.0;
		if (m_time_of_day_f < 0.0)
			m_time_of_day_f += 1.0;
	}
	if (m_time_of_day_f < r)
		return (m_time_of_day_f+1.0)-r;
	return m_time_of_day_f-r;
}

/*
	ActiveBlockList
*/

void fillRadiusBlock(v3s16 p0, s16 r, std::set<v3s16> &list)
{
	v3s16 p;
	for(p.X=p0.X-r; p.X<=p0.X+r; p.X++)
	for(p.Y=p0.Y-r; p.Y<=p0.Y+r; p.Y++)
	for(p.Z=p0.Z-r; p.Z<=p0.Z+r; p.Z++)
	{
		// Set in list
		list.insert(p);
	}
}

void ActiveBlockList::update(std::list<v3s16> &active_positions,
		s16 radius,
		std::set<v3s16> &blocks_removed,
		std::set<v3s16> &blocks_added)
{
	/*
		Create the new list
	*/
	std::set<v3s16> newlist;
	for (std::list<v3s16>::iterator i = active_positions.begin(); i != active_positions.end(); i++) {
		fillRadiusBlock(*i, radius, newlist);
	}

	/*
		Find out which blocks on the old list are not on the new list
	*/
	// Go through old list
	for (std::set<v3s16>::iterator i = m_list.begin(); i != m_list.end(); i++) {
		v3s16 p = *i;
		// If not on new list, it's been removed
		if (newlist.find(p) == newlist.end())
			blocks_removed.insert(p);
	}

	/*
		Find out which blocks on the new list are not on the old list
	*/
	// Go through new list
	for (std::set<v3s16>::iterator i = newlist.begin(); i != newlist.end(); i++) {
		v3s16 p = *i;
		// If not on old list, it's been added
		if (m_list.find(p) == m_list.end())
			blocks_added.insert(p);
	}

	/*
		Update m_list
	*/
	m_list.clear();
	for (std::set<v3s16>::iterator i = newlist.begin(); i != newlist.end(); i++) {
		v3s16 p = *i;
		m_list.insert(p);
	}
}

/*
	ServerEnvironment
*/

ServerEnvironment::ServerEnvironment(ServerMap *map, Server *server):
	m_map(map),
	m_server(server),
	m_send_recommended_timer(0),
	m_game_time(0),
	m_game_time_fraction_counter(0),
	m_players_sleeping(false)
{
}

ServerEnvironment::~ServerEnvironment()
{
	// Clear active block list.
	// This makes the next one delete all active objects.
	m_active_blocks.clear();

	// Convert all objects to static and delete the active objects
	deactivateFarObjects(true);

	// Drop/delete map
	m_map->drop();
}

void ServerEnvironment::serializePlayers()
{
	Player *player;
	uint32_t i;
	dirlist_t *list;
	dirlist_t *list_file;
	nvp_t *saved_players = NULL;
	char path[1024];

	if (path_create((char*)"player",NULL)) {
		infostream<<"failed to open players path"<<std::endl;
		return;
	}

	list = path_dirlist((char*)"player",NULL);
	list_file = list;

	while (list_file) {
		if (list_file->dir) {
			list_file = list_file->next;
			continue;
		}

		// Load player to see what is its name
		ServerRemotePlayer testplayer;
		{
			if (!path_get((char*)"player",list_file->name,1,path,1024)) {
				list_file = list_file->next;
				continue;
			}
			// Open file and deserialize
			std::ifstream is(path, std::ios_base::binary);
			if (is.good() == false) {
				infostream<<"Failed to read "<<path<<std::endl;
				list_file = list_file->next;
				continue;
			}
			testplayer.deSerialize(is);
		}

		//infostream<<"Loaded test player with name "<<testplayer.getName()<<std::endl;

		// Search for the player
		std::string playername = testplayer.getName();
		player = getPlayer(playername.c_str());
		if (player == NULL) {
			path_remove(NULL,path);
			list_file = list_file->next;
			continue;
		}

		//infostream<<"Found matching player, overwriting."<<std::endl;

		// OK, found. Save player there.
		{
			// Open file and serialize
			std::ofstream os(path, std::ios_base::binary);
			if (os.good() == false) {
				infostream<<"Failed to overwrite "<<path<<std::endl;
				list_file = list_file->next;
				continue;
			}
			player->serialize(os);
			nvp_set(&saved_players, (char*)player->getName(), (char*)player->getName(), player);
		}
		list_file = list_file->next;
	}
	
	path_dirlist_free(list);
	
	for (i=0; i<m_players->length; i++) {
		player = (Player*)array_get_ptr(m_players,i);
		if (!player)
			continue;
		if (nvp_get(&saved_players, (char*)player->getName()) != NULL)
			continue;
		char* playername = const_cast<char*>(player->getName());
		/* don't save unnamed player */
		if (!playername[0])
			continue;
		/* ... or dodgy names */
		if (string_allowed(playername, PLAYERNAME_ALLOWED_CHARS) == false)
			continue;

		if (path_get((char*)"player",playername,0,path,1024)) {
			// Open file and serialize
			std::ofstream os(path, std::ios_base::binary);
			if (os.good() == false) {
				infostream<<"Failed to overwrite "<<path<<std::endl;
				continue;
			}
			player->serialize(os);
			nvp_set(&saved_players, (char*)player->getName(), (char*)player->getName(), player);
		}
	}

	nvp_free(&saved_players,0);
}

void ServerEnvironment::deSerializePlayers()
{

	char path[1024];
	dirlist_t *list;
	dirlist_t *list_file;

	if (!path_get((char*)"player",NULL,1,path,1024))
		return;

	list = path_dirlist((char*)"player",NULL);
	list_file = list;

	while (list_file) {
		if (list_file->dir) {
			list_file = list_file->next;
			continue;
		}

		// Load player to see what is its name
		ServerRemotePlayer testplayer;
		{
			if (!path_get((char*)"player",list_file->name,1,path,1024)) {
				list_file = list_file->next;
				continue;
			}
			// Open file and deserialize
			std::ifstream is(path, std::ios_base::binary);
			if (is.good() == false) {
				infostream<<"Failed to read "<<path<<std::endl;
				list_file = list_file->next;
				continue;
			}
			testplayer.deSerialize(is);
		}

		if (!string_allowed(testplayer.getName(), PLAYERNAME_ALLOWED_CHARS)) {
			path_remove(NULL,path);
			list_file = list_file->next;
			continue;
		}

		infostream<<"Loaded test player with name "<<testplayer.getName()
				<<std::endl;

		// Search for the player
		std::string playername = testplayer.getName();
		Player *player = getPlayer(playername.c_str());
		bool newplayer = false;
		if (player == NULL) {
			infostream<<"Is a new player"<<std::endl;
			player = new ServerRemotePlayer();
			newplayer = true;
		}

		// Load player
		{
			infostream<<"Reading player "<<testplayer.getName()<<" from "
					<<path<<std::endl;
			// Open file and deserialize
			std::ifstream is(path, std::ios_base::binary);
			if (is.good() == false) {
				infostream<<"Failed to read "<<path<<std::endl;
				list_file = list_file->next;
				continue;
			}
			player->deSerialize(is);
		}

		if (newplayer)
			addPlayer(player);
		list_file = list_file->next;
	}
}

void ServerEnvironment::saveMeta()
{
	file_t *f;

	f = file_load("world","env_meta.txt");
	if (!f)
		return;

	f->len = 0;
	f->pos = 0;

	file_writef(f,"game_time = %u\n",m_game_time);
	file_writef(f,"time_of_day = %u\n",getTimeOfDay());
	file_writef(f,"world_time = %u\n",m_time);

	file_flush(f);
	file_free(f);
}

void ServerEnvironment::loadMeta()
{
	file_t *f;
	char buff[1024];
	char* n;
	char* v;

	f = file_load("world","env_meta.txt");
	if (!f)
		return;

	while (file_readline(f,buff,1024) > 0) {
		n = buff;
		v = strchr(n,'=');
		if (!v)
			continue;
		*v = 0;
		v++;
		n = trim(n);
		v = trim(v);
		if (!n[0] || !v[0])
			continue;

		if (!strcmp(n,"game_time")) {
			m_game_time = strtoll(v,NULL,10);
		}else if (!strcmp(n,"time_of_day")) {
			m_time_of_day = strtoll(v,NULL,10);
		}else if (!strcmp(n,"world_time")) {
			m_time = strtoll(v,NULL,10);
		}
	}

	file_free(f);
}

void ServerEnvironment::activateBlock(MapBlock *block, u32 additional_dtime)
{
	block->resetUsageTimer();
	// Get time difference
	u32 dtime_s = 0;
	u32 stamp = block->getTimestamp();
	if (m_game_time > stamp && stamp != BLOCK_TIMESTAMP_UNDEFINED)
		dtime_s = m_game_time - block->getTimestamp();
	dtime_s += additional_dtime;

	// Set current time as timestamp (and let it set ChangedFlag)
	//block->setTimestamp(m_game_time);
	block->setTimestampNoChangedFlag(m_game_time);

	//infostream<<"Block is "<<dtime_s<<" seconds old."<<std::endl;

	// Activate stored objects
	activateObjects(block);

	// Run node metadata
	bool changed = block->m_node_metadata.step((float)dtime_s, block->getPosRelative(),this);
	bool cchanged = block->m_node_metadata.stepCircuit((float)dtime_s, block->getPosRelative(),this);
	if (changed || cchanged) {
		MapEditEvent event;
		event.type = MEET_BLOCK_NODE_METADATA_CHANGED;
		event.p = block->getPos();
		m_map->dispatchEvent(&event);

		block->setChangedFlag();
	}
}

void ServerEnvironment::clearAllObjects()
{
	infostream<<"ServerEnvironment::clearAllObjects(): "
			<<"Removing all active objects"<<std::endl;
	std::vector<u16> objects_to_remove;
	for (std::map<u16, ServerActiveObject*>::iterator i = m_active_objects.begin(); i != m_active_objects.end(); i++) {
		ServerActiveObject* obj = i->second;
		u16 id = i->first;
		// Delete static object if block is loaded
		if (obj->m_static_exists)
		{
			MapBlock* const block = m_map->getBlockNoCreateNoEx(obj->m_static_block);
			if (block)
			{
				block->m_active_objects.remove(id);
				obj->m_static_exists = false;
				block->ResetCurrent();
			}
		}
		// If known by some client, don't delete immediately
		if (obj->m_known_by_count > 0)
		{
			obj->m_pending_deactivation = true;
			obj->m_removed = true;
			continue;
		}
		// Delete active object
		delete obj;
		// Id to be removed from m_active_objects
		objects_to_remove.push_back(id);
	}
	// Remove references from m_active_objects
	for (std::vector<u16>::iterator i = objects_to_remove.begin(); i != objects_to_remove.end(); i++) {
		m_active_objects.erase(*i);
	}

	core::list<v3s16> loadable_blocks;
	infostream<<"ServerEnvironment::clearAllObjects(): "
			<<"Listing all loadable blocks"<<std::endl;
	m_map->listAllLoadableBlocks(loadable_blocks);
	infostream<<"ServerEnvironment::clearAllObjects(): "
			<<"Done listing all loadable blocks: "
			<<loadable_blocks.size()
			<<", now clearing"<<std::endl;
	u32 report_interval = loadable_blocks.size() / 10;
	u32 num_blocks_checked = 0;
	u32 num_blocks_cleared = 0;
	u32 num_objs_cleared = 0;
	for (core::list<v3s16>::Iterator i = loadable_blocks.begin(); i != loadable_blocks.end(); i++) {
		v3s16 p = *i;
		MapBlock* const block = m_map->emergeBlock(p, false);
		if (!block)
			continue;

		u32 num_stored = block->m_static_objects.m_objects.size();
		u32 num_active = block->m_active_objects.size();
		if (num_stored != 0 || num_active != 0) {
			block->m_static_objects.m_objects.clear();
			block->m_active_objects.clear();
			block->raiseModified(MOD_STATE_WRITE_NEEDED);
			num_objs_cleared += num_stored + num_active;
			num_blocks_cleared++;
		}
		num_blocks_checked++;

		if (num_blocks_checked % report_interval == 0)
		{
			float percent = 100.0 * (float)num_blocks_checked/loadable_blocks.size();
			infostream<<"ServerEnvironment::clearAllObjects(): "
					<<"Cleared "<<num_objs_cleared<<" objects"
					<<" in "<<num_blocks_cleared<<" blocks ("
					<<percent<<"%)"<<std::endl;
		}
		block->ResetCurrent();
	}
	infostream<<"ServerEnvironment::clearAllObjects(): "
			<<"Finished: Cleared "<<num_objs_cleared<<" objects"
			<<" in "<<num_blocks_cleared<<" blocks"<<std::endl;
}

bool ServerEnvironment::searchNear(v3s16 pos, v3s16 radius_min, v3s16 radius_max, std::vector<content_t> c, v3s16 *found)
{
	v3s16 blockpos = getNodeBlockPos(pos);
	MapBlock* const block = m_map->getBlockNoCreateNoEx(blockpos);
	if (!block)
		return false;
	v3s16 relpos = blockpos*MAP_BLOCKSIZE;
	v3s16 p;
	v3s16 bp;

	for(s16 x=radius_min.X; x<=radius_max.X; x++)
	{
		for(s16 y=radius_min.Y; y<=radius_max.Y; y++)
		{
			for(s16 z=radius_min.Z; z<=radius_max.Z; z++)
			{
				if (!x && !y && !z)
					continue;
				p = pos+v3s16(x,y,z);
				MapNode n_test;
				bp = getNodeBlockPos(p);
				if (bp == blockpos)
				{
					bool pos_ok;
					n_test = block->getNodeNoCheck(p-relpos,&pos_ok);
				}
				else
					n_test = m_map->getNodeNoEx(p);
				for (std::vector<content_t>::iterator i=c.begin(); i != c.end(); i++)
				{
					if (n_test.getContent() == *i)
					{
						if (found != NULL)
							*found = p;
						return true;
					}
				}
			}
		}
	}

	block->ResetCurrent();
	return false;
}

bool ServerEnvironment::searchNearInv(v3s16 pos, v3s16 radius_min, v3s16 radius_max, std::vector<content_t> c, v3s16 *found)
{
	v3s16 blockpos = getNodeBlockPos(pos);
	MapBlock* const block = m_map->getBlockNoCreateNoEx(blockpos);
	if (!block)
		return false;
	
	v3s16 relpos = blockpos*MAP_BLOCKSIZE;
	v3s16 p;
	v3s16 bp;
	
	for(s16 x=radius_min.X; x<=radius_max.X; x++)
	{
		for(s16 y=radius_min.Y; y<=radius_max.Y; y++)
		{
			for(s16 z=radius_min.Z; z<=radius_max.Z; z++)
			{
				if (!x && !y && !z)
					continue;
				p = pos+v3s16(x,y,z);
				MapNode n_test;
				bp = getNodeBlockPos(p);
				if (bp == blockpos) {
					bool pos_ok;
					n_test = block->getNodeNoCheck(p-relpos,&pos_ok);
				}else{
					n_test = m_map->getNodeNoEx(p);
				}
				bool s = false;
				for (std::vector<content_t>::iterator i=c.begin(); i != c.end(); i++) {
					if (n_test.getContent() == *i) {
						s = true;
						break;
					}
				}
				if (!s) {
					if (found != NULL)
						*found = p;
					return true;
				}
			}
		}
	}
	
	block->ResetCurrent();
	return false;
}

/* search from pos in direction dir, until a collidable node is hit
 * if pos is a collidable node, then search till not collidable
 * return false if CONTENT_IGNORE is found
 * return true otherwise
 * result is a non-collidable node, which is:
 *	the first liquid node found
 *	if searching till not collidable:
 *		the first walkable node found
 *	else
 *		the last walkable node found
 */
bool ServerEnvironment::getCollidedPosition(v3s16 pos, v3s16 dir, v3s16 *result)
{
	ContentFeatures *f;
	v3s16 cpos = pos;
	MapNode n = m_map->getNodeNoEx(pos);
	if (n.getContent() == CONTENT_IGNORE)
		return false;

	f = &content_features(n.getContent());

	// if pos is a collidable node, search till not collidable
	if (!f->walkable) {
		while (!f->walkable) {
			cpos += dir;
			n = m_map->getNodeNoEx(cpos);
			if (n.getContent() == CONTENT_IGNORE)
				return false;

			f = &content_features(n.getContent());
		}
	//else search till collidable, then go back one
	}else{
		while (f->walkable) {
			cpos += dir;
			n = m_map->getNodeNoEx(cpos);
			if (n.getContent() == CONTENT_IGNORE)
				return false;

			f = &content_features(n.getContent());
		}
		cpos -= dir;
		n = m_map->getNodeNoEx(cpos);
		f = &content_features(n.getContent());
	}

	// if node is liquid, or air_equivalent and buildable_to, return true
	if (f->liquid_type != LIQUID_NONE || (f->air_equivalent && f->buildable_to)) {
		if (result)
			*result = cpos;
		return true;
	}

	// otherwise, reverse direction, but no further than (but including) pos
	// search for last air_equivalent and buildable_to
	while (cpos != pos) {
		cpos -= dir;
		n = m_map->getNodeNoEx(cpos);
		if (n.getContent() == CONTENT_IGNORE)
			return false;

		f = &content_features(n.getContent());
		if (f->liquid_type != LIQUID_NONE || (f->air_equivalent && f->buildable_to)) {
			if (result)
				*result = cpos;
			return true;
		}
	}

	n = m_map->getNodeNoEx(cpos);
	if (n.getContent() == CONTENT_IGNORE)
		return false;

	f = &content_features(n.getContent());

	// if found, return true
	if (f->liquid_type != LIQUID_NONE || (f->air_equivalent && f->buildable_to)) {
		if (result)
			*result = cpos;
		return true;
	}

	// otherwise, return false
	return false;
}

bool ServerEnvironment::dropToParcel(v3s16 pos, InventoryItem *item)
{
	v3s16 ppos;
	NodeMetadata *meta;
	Inventory *inv;
	InventoryList *list;

	if (!item)
		return true;

	// check for single material item and above dirtlike
	// if so, and it has a place_on_drop then
	if (
		item->getCount() == 1
		&& (item->getContent()&0xF000) == 0
		&& content_features(item->getContent()).place_on_drop != CONTENT_IGNORE
		&& getCollidedPosition(pos,v3s16(0,-1,0),&ppos)
		&& content_features(m_map->getNodeNoEx(ppos+v3s16(0,-1,0)).getContent()).draw_type == CDT_DIRTLIKE
	) {
		ContentFeatures *f = &content_features(item->getContent());
		// if it's landing on a place_on_drop, destroy it
		if (m_map->getNodeNoEx(ppos).getContent() == f->place_on_drop) {
			delete item;
			return true;
		// it has a place_on_drop_alternate and a place_on_drop is nearby
		}else if (f->place_on_drop_alternate && searchNear(ppos,v3s16(3,3,3),f->place_on_drop,NULL)) {
			// place place_on_drop_alternate at pos
			MapNode n(f->place_on_drop_alternate);
			m_map->addNodeWithEvent(ppos,n);
			delete item;
			return true;
		// else place place_on_drop
		}else{
			MapNode n(f->place_on_drop);
			m_map->addNodeWithEvent(ppos,n);
			delete item;
			return true;
		}
	}

	// look for a parcel near pos
	if (searchNear(pos,v3s16(3,3,3),CONTENT_PARCEL,&ppos)) {
		// add items if found
		meta = m_map->getNodeMetadata(ppos);
		if (meta) {
			inv = meta->getInventory();
			if (inv) {
				list = inv->getList("0");
				if (list)
					item = list->addItem(item);
			}
		}
	}

	if (!item)
	{
		v3s16 bp = getNodeBlockPos(ppos);
		MapBlock* const block = m_map->getBlockNoCreateNoEx(bp);
		if (block)
		{
			MapEditEvent event;
			event.type = MEET_BLOCK_NODE_METADATA_CHANGED;
			event.p = bp;
			m_map->dispatchEvent(&event);

			block->setChangedFlag();
			block->ResetCurrent();
		}
		return true;
	}

	// if underground, go up to first air_equivalent and buildable_to
	if (!content_features(m_map->getNodeNoEx(pos).getContent()).air_equivalent)
	{
		if (!getCollidedPosition(pos,v3s16(0,1,0),&ppos))
		{
			if (!searchNear(pos,v3s16(2,0,2),CONTENT_AIR,&ppos))
			{
				delete item;
				return false;
			}
		}
		pos = ppos;
	// otherwise go down to first non-air_equivalent and buildable_to
	}else{
		if (!getCollidedPosition(pos,v3s16(0,-1,0),&ppos)) {
			if (!searchNear(pos,v3s16(2,0,2),CONTENT_AIR,&ppos)) {
				delete item;
				return false;
			}
		}
		pos = ppos;
	}

	// look for a parcel near pos
	if (searchNear(pos,v3s16(3,3,3),CONTENT_PARCEL,&ppos))
	{
		// add items if found
		meta = m_map->getNodeMetadata(ppos);
		if (meta)
		{
			inv = meta->getInventory();
			if (inv)
			{
				list = inv->getList("0");
				if (list)
					item = list->addItem(item);
			}
		}
	}

	if (!item) {
		v3s16 bp = getNodeBlockPos(ppos);
		MapBlock* const block = m_map->getBlockNoCreateNoEx(bp);
		if (block)
		{
			MapEditEvent event;
			event.type = MEET_BLOCK_NODE_METADATA_CHANGED;
			event.p = bp;
			m_map->dispatchEvent(&event);

			block->setChangedFlag();
			block->ResetCurrent();
		}
		return true;
	}

	// if liquid, do a slightly wider search for a parcel on shore
	if (content_features(m_map->getNodeNoEx(pos).getContent()).liquid_type != LIQUID_NONE)
	{
		if (content_features(m_map->getNodeNoEx(pos).getContent()).damage_per_second > 0)
		{
			delete item;
			return false;
		}
		else if (searchNear(pos,v3s16(5,5,5),CONTENT_PARCEL,&ppos))
		{
			// add items if found
			meta = m_map->getNodeMetadata(ppos);
			if (meta) {
				inv = meta->getInventory();
				if (inv) {
					list = inv->getList("0");
					if (list)
						item = list->addItem(item);
				}
			}
		}
	}

	if (!item) {
		v3s16 bp = getNodeBlockPos(ppos);
		MapBlock* const block = m_map->getBlockNoCreateNoEx(bp);
		if (block)
		{
			MapEditEvent event;
			event.type = MEET_BLOCK_NODE_METADATA_CHANGED;
			event.p = bp;
			m_map->dispatchEvent(&event);

			block->setChangedFlag();
			block->ResetCurrent();
		}
		return true;
	}

	// check that pos is air_equivalent and buildable_to
	{
		ContentFeatures *f = &content_features(m_map->getNodeNoEx(pos).getContent());
		if (f->liquid_type != LIQUID_NONE || (f->air_equivalent && f->buildable_to)) {
			// create a parcel
			MapNode n(CONTENT_PARCEL);
			m_map->addNodeWithEvent(pos,n);
			// add items
			meta = m_map->getNodeMetadata(pos);
			if (meta) {
				inv = meta->getInventory();
				if (inv) {
					list = inv->getList("0");
					if (list)
						item = list->addItem(item);
				}
			}
		}
	}

	if (!item) {
		v3s16 bp = getNodeBlockPos(pos);
		MapBlock* const block = m_map->getBlockNoCreateNoEx(bp);
		if (block)
		{
			MapEditEvent event;
			event.type = MEET_BLOCK_NODE_METADATA_CHANGED;
			event.p = bp;
			m_map->dispatchEvent(&event);

			block->setChangedFlag();
			block->ResetCurrent();
		}
		return true;
	}

	delete item;
	return false;
}

void ServerEnvironment::step(float dtime)
{
	DSTACK(__FUNCTION_NAME);

	//TimeTaker timer("ServerEnv step");

	float time_diff = stepTimeOfDay(dtime);
	time_diff *= 24000.0;

	// Get some settings
	bool footprints = config_get_bool("world.game.environment.footprints");

	/*
		Increment game time
	*/
	{
		m_game_time_fraction_counter += dtime;
		u32 inc_i = (u32)m_game_time_fraction_counter;
		m_game_time += inc_i;
		m_game_time_fraction_counter -= (float)inc_i;
	}


	bool blockstep = m_active_blocks_management_interval.step(dtime, 2.0);
	std::list<v3s16> players_blockpos;

	bool sleepskip = true;

	/*
		Handle players
	*/
	{
		ScopeProfiler sp(g_profiler, "SEnv: handle players avg", SPT_AVG);
		int pc = 0;
		Player *player;
		uint32_t i;
		for (i=0; i<m_players->length; i++) {
			player = (Player*)array_get_ptr(m_players,i);
			if (!player)
				continue;

			// Ignore disconnected players
			if (player->peer_id == 0)
				continue;
			pc++;

			if (!player->in_bed)
				sleepskip = false;

			if (player->wake_timeout > 0.0)
				player->wake_timeout -= time_diff;

			v3f playerpos = player->getPosition();

			// Move
			player->move(dtime, *m_map, 100*BS);

			/*
				Add footsteps to grass
			*/
			if (footprints) {
				// Get node that is at BS/4 under player
				v3s16 bottompos = floatToInt(playerpos + v3f(0,-BS/4,0), BS);
				MapNode n = m_map->getNodeNoEx(bottompos);
				if (n.getContent() == CONTENT_MUD && (n.param1&0xF0) != 0x10) {
					n.param1 |= 0x10;
					n.envticks = 0;
					m_map->setNode(bottompos, n);
				}else{
					bottompos.Y += 1;
					MapNode n = m_map->getNodeNoEx(bottompos);
					if (
						n.getContent() == CONTENT_CIRCUIT_PRESSUREPLATE_STONE
						|| n.getContent() == CONTENT_CIRCUIT_PRESSUREPLATE_WOOD
					) {
						NodeMetadata *meta = m_map->getNodeMetadata(bottompos);
						if (meta && !meta->getEnergy())
							meta->energise(ENERGY_MAX,bottompos,bottompos,bottompos);
					}
				}
			}
			/*
				Get player block positions
			*/
			if (blockstep) {
				v3s16 blockpos = getNodeBlockPos(floatToInt(playerpos, BS));
				players_blockpos.push_back(blockpos);
			}
		}
		if (!pc)
			sleepskip = false;
	}

	/*
		Manage active block list
	*/
	if (blockstep) {
		if (sleepskip) {
			if (m_players_sleeping) {
				// advance time to dawn or dusk
				if (m_time_of_day < 6000) {
					setTimeOfDay(6000);
				}else if (m_time_of_day > 18000) {
					m_time++;
					setTimeOfDay(6000);
				}else{
					setTimeOfDay(18000);
				}
				// wake up
				addEnvEvent(ENV_EVENT_WAKE,v3f(0,0,0),"");
				Player *player;
				uint32_t i;
				for (i=0; i<m_players->length; i++) {
					player = (Player*)array_get_ptr(m_players,i);
					if (!player)
						continue;

					// Ignore disconnected players
					if (player->peer_id == 0)
						continue;
					player->in_bed = false;
					player->wake_timeout = 10000;
				}
			}else{
				// go to sleep
				addEnvEvent(ENV_EVENT_SLEEP,v3f(0,0,0),"");
			}

			m_players_sleeping = !m_players_sleeping;
		}else if (m_players_sleeping) {
			// wake up
			addEnvEvent(ENV_EVENT_WAKE,v3f(0,0,0),"");
			Player *player;
			uint32_t i;
			for (i=0; i<m_players->length; i++) {
				player = (Player*)array_get_ptr(m_players,i);
				if (!player)
					continue;

				// Ignore disconnected players
				if (player->peer_id == 0)
					continue;
				player->in_bed = false;
			}
			m_players_sleeping = false;
		}

		ScopeProfiler sp(g_profiler, "SEnv: manage act. block list avg /2s", SPT_AVG);

		/*
			Update list of active blocks, collecting changes
		*/
		const s16 active_block_range = config_get_int("world.server.chunk.range.active");
		std::set<v3s16> blocks_removed;
		std::set<v3s16> blocks_added;
		m_active_blocks.update(players_blockpos, active_block_range, blocks_removed, blocks_added);

		/*
			Handle removed blocks
		*/

		// Convert active objects that are no more in active blocks to static
		deactivateFarObjects(false);

		for (std::set<v3s16>::iterator i = blocks_removed.begin(); i != blocks_removed.end(); i++)
		{
			v3s16 p = *i;

			/*infostream<<"Server: Block ("<<p.X<<","<<p.Y<<","<<p.Z
					<<") became inactive"<<std::endl;*/

			MapBlock* const block = m_map->getBlockNoCreateNoEx(p);
			if (!block)
				continue;

			// Set current time as timestamp (and let it set ChangedFlag)
			block->setTimestamp(m_game_time);
			block->ResetCurrent();
		}

		/*
			Handle added blocks
		*/

		for (std::set<v3s16>::iterator i = blocks_added.begin(); i != blocks_added.end(); i++)
		{
			v3s16 p = *i;

			/*infostream<<"Server: Block ("<<p.X<<","<<p.Y<<","<<p.Z
					<<") became active"<<std::endl;*/

			MapBlock* const block = m_map->emergeBlock(p,false);
			if (!block)
				continue;

			activateBlock(block);
			block->ResetCurrent();
		}
	}

	/*
		Mess around in active blocks
	*/
	bool circuitstep = m_active_blocks_circuit_interval.step(dtime, 0.5);
	bool metastep = m_active_blocks_nodemetadata_interval.step(dtime, 1.0);
	bool nodestep = m_active_blocks_test_interval.step(dtime, 10.0);

	if (circuitstep || metastep || nodestep)
	{
		float circuit_dtime = 0.5;
		float meta_dtime = 1.0;
		u16 season = getSeason();
		uint16_t time = getTimeOfDay();
		bool unsafe_fire = config_get_bool("world.game.environment.fire.spread");
		
		for (std::set<v3s16>::iterator i = m_active_blocks.m_list.begin();
		     i != m_active_blocks.m_list.end(); i++)
		{
			v3s16 bp = *i;

			MapBlock* const block = m_map->getBlockNoCreateNoEx(bp);
			if (!block)
				continue;

			std::list<u16> new_list;
			for (std::list<u16>::iterator oi = block->m_active_objects.begin();
			     oi != block->m_active_objects.end(); oi++)
			{
				ServerActiveObject *obj = getActiveObject(*oi);
				if (obj && obj->m_static_exists && obj->m_static_block == block->getPos())
					new_list.push_back(*oi);
			}
			block->m_active_objects.swap(new_list);

			// Reset block usage timer
			if (circuitstep || metastep)
				block->resetUsageTimer();

			// Set current time as timestamp
			block->setTimestampNoChangedFlag(m_game_time);

			bool blockchanged = false;

			m_poststep_nodeswaps.clear();

			if (circuitstep)
			{
				// Run node metadata
				bool changed = block->m_node_metadata.stepCircuit(circuit_dtime, block->getPosRelative(), this);
				if (changed)
					blockchanged = true;
			}

			if (metastep)
			{
				// Run node metadata
				bool changed = block->m_node_metadata.step(meta_dtime, block->getPosRelative(), this);
				if (changed)
					blockchanged = true;
			}

			if (blockchanged)
			{
				MapEditEvent event;
				event.type = MEET_BLOCK_NODE_METADATA_CHANGED;
				event.p = bp;
				m_map->dispatchEvent(&event);

				block->setChangedFlag();
			}

			if (m_poststep_nodeswaps.size() > 0)
			{
				for (std::map<v3s16,MapNode>::iterator i = m_poststep_nodeswaps.begin();
				     i != m_poststep_nodeswaps.end(); i++) {
					v3s16 sp = i->first;
					MapNode n = i->second;
					NodeMetadata *meta = NULL;
					std::string n_owner = "";
					std::string i_owner = "";
					meta = m_map->getNodeMetadata(sp);
					if (meta) {
						n_owner = meta->getOwner();
						i_owner = meta->getInventoryOwner();
						meta = meta->clone();
					}
					m_map->addNodeWithEvent(sp, n);
					if (meta) {
						m_map->setNodeMetadata(sp,meta);
						meta = m_map->getNodeMetadata(sp);
						if (meta) {
							if (n_owner != "")
								meta->setOwner(n_owner);
							if (i_owner != "")
								meta->setInventoryOwner(i_owner);
						}
					}
				}
				m_poststep_nodeswaps.clear();
			}

			if (!nodestep)
				continue;

			bool has_steam_sound = false;

			/*
				Do stuff!

				Note that map modifications should be done using the event-
				making map methods so that the server gets information
				about them.

				Reading can be done quickly directly from the block.

				Everything should bind to inside this single content
				searching loop to keep things fast.
			*/

			if (block->last_spawn < m_time_of_day-6000)
			{
				MapNode n1 = block->getNodeNoEx(block->spawn_area+v3s16(0,1,0));
				MapNode n2 = block->getNodeNoEx(block->spawn_area+v3s16(0,2,0));
				u8 light = n1.getLightBlend(getDayNightRatio());
				if (!content_features(n1.getContent()).air_equivalent
						|| !content_features(n2.getContent()).air_equivalent)
					block->has_spawn_area = false;

				if (block->has_spawn_area && m_time_of_day > 19000 && m_time_of_day < 20000)
				{
					if (light <= LIGHT_SPAWN_DARK)
					{
						if (block->getPos().Y > 0 || myrand_range(0,5) == 0)
						{
							mob_spawn_hostile(block->spawn_area+block->getPosRelative(),false,this);
						}
					}
					block->last_spawn = m_time_of_day;
				}
			}

			v3s16 p0;
			uint8_t biome = block->getBiome();
			bool coldzone = false;
			
			switch (biome)
			{
			case BIOME_JUNGLE:
				if (season == ENV_SEASON_WINTER && time > 4000 && time < 8000)
					coldzone = true;
				break;
			case BIOME_OCEAN:
				if (season == ENV_SEASON_WINTER)
					coldzone = true;
				break;
			case BIOME_PLAINS:
				if (season == ENV_SEASON_WINTER && (time < 6000 || time > 18000))
					coldzone = true;
				break;
			case BIOME_FOREST:
				if (season == ENV_SEASON_WINTER)
					coldzone = true;
				break;
			case BIOME_SKY:
			case BIOME_SNOWCAP:
				coldzone = true;
				break;
			case BIOME_LAKE:
			if (
				season == ENV_SEASON_WINTER
				|| (
					season == ENV_SEASON_AUTUMN
					&& (time < 6000 || time > 18000)
				) || (
					season == ENV_SEASON_SPRING
					&& time > 4000
					&& time < 8000
				)
			) {
				coldzone = true;
			}
				break;
			case BIOME_BEACH:
				if (season == ENV_SEASON_WINTER)
					coldzone = true;
				break;
			case BIOME_UNKNOWN:
			case BIOME_WOODLANDS:
				if (season == ENV_SEASON_WINTER)
					coldzone = true;
			default:
				break;
			}
			
			for (p0.X=0; p0.X<MAP_BLOCKSIZE; p0.X++)
			for (p0.Y=0; p0.Y<MAP_BLOCKSIZE; p0.Y++)
			for (p0.Z=0; p0.Z<MAP_BLOCKSIZE; p0.Z++) {
				v3s16 p = p0 + block->getPosRelative();
				block->incNodeTicks(p0);
				MapNode n = block->getNodeNoEx(p0);
				if (!block->has_spawn_area
					&& (content_features(n.getContent()).draw_type == CDT_DIRTLIKE
						|| n.getContent() == CONTENT_SAND
						|| n.getContent() == CONTENT_STONE))
				{
					MapNode n1 = block->getNodeNoEx(p0+v3s16(0,1,0));
					MapNode n2 = block->getNodeNoEx(p0+v3s16(0,2,0));
					if (content_features(n1.getContent()).air_equivalent
						&& content_features(n2.getContent()).air_equivalent
						&& myrand_range(0,5) == 0)
					{
						block->spawn_area = p0;
						block->has_spawn_area = true;
					}
				}

				switch(n.getContent())
				{
	/*
	 * param1:
	 * 	top nibble:
	 * 		0 - no effect
	 * 		1 - footsteps
	 * 		2 - salt
	 * 	bottom nibble:
	 *		0 - no overlay
	 * 		1 - spring grass
	 * 		2 - autumn grass
	 * 		4 - snow
	 * 		6 - polluted
	 * 		8 - jungle grass
	 *
	 * param2:
	 *  plantgrowth, only valid if param1 bottom nibble is 1 or 2
	 *	top nibble:
	 * 		bitwise OR direction of growth
	 *	bottom nibble:
	 *		0 - fully grown
	 *		1-15 - growth stages
	 */
				case CONTENT_MUD:
					if (season == ENV_SEASON_SPRING) {
						MapNode n_top = m_map->getNodeNoEx(p+v3s16(0,1,0));
						if (n_top.getContent() == CONTENT_AIR) {
							uint32_t r_result = myrand_range(0,500);
							content_t add_content = CONTENT_IGNORE;
							switch (biome) {
							case BIOME_PLAINS:
								if (r_result == 1) {
									add_content = CONTENT_FARM_WHEAT;
								}else if (r_result == 2) {
									add_content = CONTENT_FARM_PUMPKIN;
								}
								break;
							case BIOME_FOREST:
								if (r_result == 1) {
									add_content = CONTENT_FARM_COTTON;
								}else if (r_result == 2) {
									add_content = CONTENT_FARM_MELON;
								}
								break;
							case BIOME_UNKNOWN:
							case BIOME_WOODLANDS:
								if (r_result == 1) {
									add_content = CONTENT_FARM_POTATO;
								}else if (r_result == 2) {
									add_content = CONTENT_FARM_CARROT;
								}else if (r_result == 3) {
									add_content = CONTENT_FARM_BEETROOT;
								}
							default:
								break;
							}
							if (add_content != CONTENT_IGNORE) {
								std::vector<content_t> search;
								search.push_back(add_content);
								search.push_back(CONTENT_IGNORE);
								if (!searchNear(p,v3s16(3,3,3),search,NULL)) {
									n_top.setContent(add_content);
									m_map->updateNodeWithEvent(p+v3s16(0,1,0), n_top);
								}
							}
						}
					}
				case CONTENT_CLAY:
				{
					MapNode n_top = m_map->getNodeNoEx(p+v3s16(0,1,0));
					if (content_features(n_top).air_equivalent) {
						// coldzone, change to snow
						if (coldzone && (n.param1&0x0F) != 0x04) {
							// should only change to snow if there's nothing above it
							std::vector<content_t> search;
							search.push_back(CONTENT_SNOW);
							search.push_back(CONTENT_AIR);
							if (!searchNearInv(p,v3s16(0,0,0),v3s16(0,32,0),search,NULL)) {
								n.param1 = 0x04;
								m_map->updateNodeWithEvent(p, n);
							}
						// footsteps fade out
						}else if ((n.param1&0x10) == 0x10 && n.envticks > 3) {
							n.param1 &= ~0x10;
							m_map->updateNodeWithEvent(p,n);
						// autumn grass in autumn/winter
						}else if (
							biome == BIOME_DESERT
							|| (
								biome != BIOME_WASTELANDS
								&& biome != BIOME_SPACE
								&& biome != BIOME_THEDEEP
								&& (
									(n.param1&0x0F) == 0x01
								) && (
									season == ENV_SEASON_WINTER
									|| (
										season == ENV_SEASON_AUTUMN
										&& (
											getSeasonDay() > 20
											|| myrand_range(0,5) == 0
										)
									)
								)
							)
						) {
							n.param1 &= ~0x0F;
							n.param1 |= 0x02;
							m_map->updateNodeWithEvent(p,n);
						// green grass in spring/summer
						}else if (
							biome != BIOME_WASTELANDS
							&& biome != BIOME_THEDEEP
							&& (
								biome == BIOME_SPACE
								|| (
									(
										(n.param1&0x0F) == 0x02
									) && (
										season == ENV_SEASON_SUMMER
										|| (
											season == ENV_SEASON_SPRING
											&& (
												getSeasonDay() > 20
												|| myrand_range(0,5) == 0
											)
										)
									)
								)
							)
						) {
							n.param1 &= ~0x0F;
							n.param1 |= 0x01;
							m_map->updateNodeWithEvent(p,n);
						// melt snow out of the coldzone
						}else if (
							(n.param1&0x0F) == 0x04
							&& !coldzone
						) {
							n.param1 &= ~0x0F;
							if (season == ENV_SEASON_SUMMER) {
								n.param1 |= 0x01;
							}else{
								n.param1 |= 0x02;
							}
							n.param2 = 0;
							m_map->updateNodeWithEvent(p,n);
						// grow
						}else if (
							(n.param1&0x0F) != 0x04
							&& n_top.getLightBlend(getDayNightRatio()) >= 13
						) {
							plantgrowth_grass(this,p);
						}
					}else{
						if (n.param1 == 0x01) {
							n.param1 = 0x02;
							m_map->updateNodeWithEvent(p,n);
						}else if (n.param1 != 0) {
							n.param1 = 0;
							n.param2 = 0;
							m_map->updateNodeWithEvent(p,n);
						}
					}
					break;
				}
				case CONTENT_FARM_WHEAT:
				case CONTENT_FARM_MELON:
				case CONTENT_FARM_PUMPKIN:
				case CONTENT_FARM_POTATO:
				case CONTENT_FARM_CARROT:
				case CONTENT_FARM_BEETROOT:
				case CONTENT_FARM_COTTON:
				{
					MapNode n_btm = m_map->getNodeNoEx(p+v3s16(0,-1,0));
					if (n_btm.getContent() == CONTENT_FARM_DIRT) {
						/* requires a glass roof (greenhouse) */
						if (coldzone) {
							std::vector<content_t> search;
							search.push_back(CONTENT_GLASS);
							search.push_back(CONTENT_GLASS_SLAB);
							search.push_back(CONTENT_GLASS_SLAB_UD);
							search.push_back(CONTENT_GLASSLIGHT);
							search.push_back(CONTENT_GLASS_BLACK);
							search.push_back(CONTENT_GLASS_BLACK_SLAB);
							search.push_back(CONTENT_GLASS_BLACK_SLAB_UD);
							search.push_back(CONTENT_GLASS_BLUE);
							search.push_back(CONTENT_GLASS_BLUE_SLAB);
							search.push_back(CONTENT_GLASS_BLUE_SLAB_UD);
							search.push_back(CONTENT_GLASS_GREEN);
							search.push_back(CONTENT_GLASS_GREEN_SLAB);
							search.push_back(CONTENT_GLASS_GREEN_SLAB_UD);
							search.push_back(CONTENT_GLASS_ORANGE);
							search.push_back(CONTENT_GLASS_ORANGE_SLAB);
							search.push_back(CONTENT_GLASS_ORANGE_SLAB_UD);
							search.push_back(CONTENT_GLASS_PURPLE);
							search.push_back(CONTENT_GLASS_PURPLE_SLAB);
							search.push_back(CONTENT_GLASS_PURPLE_SLAB_UD);
							search.push_back(CONTENT_GLASS_RED);
							search.push_back(CONTENT_GLASS_RED_SLAB);
							search.push_back(CONTENT_GLASS_RED_SLAB_UD);
							search.push_back(CONTENT_GLASS_YELLOW);
							search.push_back(CONTENT_GLASS_YELLOW_SLAB);
							search.push_back(CONTENT_GLASS_YELLOW_SLAB_UD);
							search.push_back(CONTENT_ROOFTILE_GLASS);
							search.push_back(CONTENT_ROOFTILE_GLASS_BLACK);
							search.push_back(CONTENT_ROOFTILE_GLASS_BLUE);
							search.push_back(CONTENT_ROOFTILE_GLASS_GREEN);
							search.push_back(CONTENT_ROOFTILE_GLASS_ORANGE);
							search.push_back(CONTENT_ROOFTILE_GLASS_PURPLE);
							search.push_back(CONTENT_ROOFTILE_GLASS_RED);
							search.push_back(CONTENT_ROOFTILE_GLASS_YELLOW);
							search.push_back(CONTENT_IGNORE);
							if (!searchNearInv(p,v3s16(0,0,0),v3s16(0,16,0),search,NULL)) {
								m_map->removeNodeWithEvent(p);
							}
						}
					}else if (
						season == ENV_SEASON_WINTER
						|| (
							season == ENV_SEASON_AUTUMN
							&& myrand_range(0,10) == 0
						)
					) {
						m_map->removeNodeWithEvent(p);
					}
					break;
				}

				case CONTENT_WATER:
				case CONTENT_WATERSOURCE:
				{
					if (p.Y < 1024) {
						bool chill = false;
						if (coldzone) {
							std::vector<content_t> search;
							search.push_back(CONTENT_LAVASOURCE);
							search.push_back(CONTENT_LAVA);
							search.push_back(CONTENT_FIRE);
							if (searchNear(p,v3s16(3,2,3),search,NULL))
								chill = true;
						}else if (season == ENV_SEASON_WINTER && biome != BIOME_DESERT) {
							MapNode nu = m_map->getNodeNoEx(p+v3s16(0,-1,0));
							MapNode no = m_map->getNodeNoEx(p+v3s16(0,1,0));
							if (nu.getContent() == CONTENT_MUD && no.getContent() == CONTENT_AIR && myrand_range(0,3) == 0)
								chill = true;
						}

						if (chill) {
							n.setContent(CONTENT_ICE);
							m_map->addNodeWithEvent(p, n);
						}
					}
					break;
				}

				case CONTENT_ICE:
				{
					bool found = false;
					if (coldzone) {
						std::vector<content_t> search;
						search.push_back(CONTENT_LAVASOURCE);
						search.push_back(CONTENT_LAVA);
						search.push_back(CONTENT_FIRE);
						found = searchNear(p,v3s16(3,2,3),search,NULL);
					}else{
						found = true;
					}
					if (found) {
						if (searchNear(p,v3s16(5,1,5),CONTENT_WATERSOURCE,NULL)) {
							n.setContent(CONTENT_WATER);
							m_map->addNodeWithEvent(p, n);
						}else{
							n.setContent(CONTENT_WATERSOURCE);
							m_map->addNodeWithEvent(p, n);
						}
					}
					break;
				}

				case CONTENT_SNOW:
				{
					MapNode n_test = m_map->getNodeNoEx(p+v3s16(0,-1,0));
					if (n_test.getContent() == CONTENT_AIR || !coldzone)
						m_map->removeNodeWithEvent(p);
					break;
				}

				case CONTENT_SNOW_BLOCK:
				{
					if (p.Y < 1) {
						if (searchNear(p,v3s16(3,1,3),CONTENT_WATERSOURCE,NULL)) {
							n.setContent(CONTENT_WATER);
							m_map->addNodeWithEvent(p, n);
						}else{
							n.setContent(CONTENT_WATERSOURCE);
							m_map->addNodeWithEvent(p, n);
						}
					}else{
						std::vector<content_t> search;
						search.push_back(CONTENT_LAVASOURCE);
						search.push_back(CONTENT_LAVA);
						search.push_back(CONTENT_FIRE);
						if (searchNear(p,v3s16(3,2,3),search,NULL)) {
							n.setContent(CONTENT_WATERSOURCE);
							m_map->addNodeWithEvent(p, n);
						}
					}
					break;
				}

				// Grow stuff on farm dirt
				case CONTENT_FARM_DIRT:
				{
					if (n.envticks%4 == 0) { // with this plants take around 10 minutes to grow
						s16 max_d = 3;
						v3s16 temp_p = p;
						v3s16 test_p;
						MapNode testnode;
						u8 water_found = 0; // 1 = flowing, 2 = source
						bool ignore_found = false;
						for(s16 z=-max_d; water_found < 2 && z<=max_d; z++) {
						for(s16 x=-max_d; water_found < 2 && x<=max_d; x++) {
							test_p = temp_p + v3s16(x,0,z);
							testnode = m_map->getNodeNoEx(test_p);
							if (testnode.getContent() == CONTENT_WATERSOURCE) {
								water_found = 2;
							}else if (testnode.getContent() == CONTENT_WATER) {
								water_found = 1;
							}else if (testnode.getContent() == CONTENT_IGNORE) {
								ignore_found = true;
							}
						}
						}

						if (water_found) {
							test_p = temp_p + v3s16(0,1,0);
							testnode = m_map->getNodeNoEx(test_p);
							if (
								content_features(testnode).draw_type == CDT_PLANTLIKE
								|| content_features(testnode).draw_type == CDT_CROPLIKE
							) {
								if (content_features(testnode).param2_type == CPT_PLANTGROWTH) {
									plantgrowth_plant(this,test_p);
								}else if (content_features(testnode).special_alternate_node != CONTENT_IGNORE) {
									plantgrowth_seed(this,test_p);
								}
							}else if (content_features(testnode).draw_type == CDT_MELONLIKE) {
								if (content_features(testnode).param2_type == CPT_PLANTGROWTH)
									plantgrowth_plant(this,test_p);
							}else if (testnode.getContent() == CONTENT_CACTUS) {
								plantgrowth_cactus(this,test_p);
							}else if (testnode.getContent() == CONTENT_FERTILIZER) {
								plantgrowth_fertilizer(this,test_p);
							}else if (testnode.getContent() == CONTENT_AIR) {
								int chance = 5;
								if (water_found == 1)
									chance = 2;
								if (myrand()%chance != 0) {
									// grow flower
									n.setContent(CONTENT_FLOWER_STEM);
									m_map->addNodeWithEvent(test_p,n);
								}
							}
						}else if (!ignore_found) {
							// revert to mud
							n.setContent(CONTENT_MUD);
							m_map->addNodeWithEvent(p,n);
						}
					}
					break;
				}
				/*
					make vines die
				 */
				case CONTENT_FARM_GRAPEVINE:
				{
					if (n.envticks%3 == 0) {
						MapNode n_btm = m_map->getNodeNoEx(p+v3s16(0,-1,0));
						if (
							n_btm.getContent() != CONTENT_FARM_GRAPEVINE
							&& n_btm.getContent() != CONTENT_FARM_DIRT
							&& n_btm.getContent() != CONTENT_MUD
						) {
							n.setContent(CONTENT_DEAD_VINE);
							m_map->addNodeWithEvent(p, n);
						}
					}
					break;
				}
				case CONTENT_FARM_TRELLIS_GRAPE:
				{
					if (n.envticks%3 == 0) {
						MapNode n_btm = m_map->getNodeNoEx(p+v3s16(0,-1,0));
						if (
							n_btm.getContent() != CONTENT_FARM_TRELLIS_GRAPE
							&& n_btm.getContent() != CONTENT_FARM_DIRT
							&& n_btm.getContent() != CONTENT_MUD
						) {
							n.setContent(CONTENT_TRELLIS_DEAD_VINE);
							m_map->addNodeWithEvent(p, n);
						}
					}
					break;
				}

				case CONTENT_WILDGRASS_SHORT:
				{
					MapNode n_btm = m_map->getNodeNoEx(p+v3s16(0,-1,0));
					if (
						n_btm.getContent() == CONTENT_GRASS
						|| n_btm.getContent() == CONTENT_GRASS_AUTUMN
						|| n_btm.getContent() == CONTENT_MUDSNOW
						|| n_btm.getContent() == CONTENT_MUD
					) {
						if (p.Y > -1 && n.envticks > 10) {
							MapNode n_top = m_map->getNodeNoEx(p+v3s16(0,1,0));
							if (n_btm.getContent() != CONTENT_MUD) {
								if (n_top.getLightBlend(getDayNightRatio()) >= 13) {
									u32 chance = 20;
									switch (season) {
									case ENV_SEASON_SUMMER:
										chance = 10;
										break;
									case ENV_SEASON_AUTUMN:
										chance = 15;
										break;
									case ENV_SEASON_SPRING:
										chance = 5;
										break;
									default:;
									}
									if (myrand_range(0,chance) == 0) {
										n.setContent(CONTENT_FLOWER_STEM);
										m_map->addNodeWithEvent(p, n);
									}else{
										n.setContent(CONTENT_WILDGRASS_LONG);
										m_map->addNodeWithEvent(p, n);
									}
								}
							}
						}
					}else{
						m_map->removeNodeWithEvent(p);
					}
					break;
				}

				case CONTENT_WILDGRASS_LONG:
				{
					MapNode n_btm = m_map->getNodeNoEx(p+v3s16(0,-1,0));
					if (
						n_btm.getContent() != CONTENT_GRASS
						&& n_btm.getContent() != CONTENT_GRASS_AUTUMN
						&& n_btm.getContent() != CONTENT_MUDSNOW
						&& n_btm.getContent() != CONTENT_MUD
					) {
						m_map->removeNodeWithEvent(p);
					}
					break;
				}

				case CONTENT_FLOWER_STEM:
				{
					MapNode n_btm = m_map->getNodeNoEx(p+v3s16(0,-1,0));
					int ch = 0;
					if (
						n_btm.getContent() == CONTENT_GRASS
						|| n_btm.getContent() == CONTENT_GRASS_AUTUMN
						|| n_btm.getContent() == CONTENT_MUD
					)
						ch = 100;
					if (n_btm.getContent() == CONTENT_FARM_DIRT)
						ch = 50;
					if (ch) {
						if (season == ENV_SEASON_SPRING)
							break;
						if ((ch == 50 || p.Y > -1) && n.envticks > 20) {
							MapNode n_top = m_map->getNodeNoEx(p+v3s16(0,1,0));
							if (n_top.getLightBlend(getDayNightRatio()) >= 13) {
								switch (myrand()%3) {
								case 0:
									n.setContent(CONTENT_FLOWER_ROSE);
									m_map->addNodeWithEvent(p, n);
									break;
								case 1:
									n.setContent(CONTENT_FLOWER_DAFFODIL);
									m_map->addNodeWithEvent(p, n);
									break;
								case 2:
									n.setContent(CONTENT_FLOWER_TULIP);
									m_map->addNodeWithEvent(p, n);
									break;
								}
							}
						}
					}else{
						m_map->removeNodeWithEvent(p);
					}
					break;
				}

				case CONTENT_DEADGRASS:
				{
					MapNode n_btm = m_map->getNodeNoEx(p+v3s16(0,-1,0));
					if (
						n_btm.getContent() == CONTENT_GRASS
						|| n_btm.getContent() == CONTENT_GRASS_AUTUMN
						|| n_btm.getContent() == CONTENT_MUDSNOW
						|| n_btm.getContent() == CONTENT_MUD
					) {
						n.setContent(CONTENT_WILDGRASS_LONG);
						m_map->addNodeWithEvent(p, n);
					}else{
						m_map->removeNodeWithEvent(p);
					}
					break;
				}

				case CONTENT_FLOWER_ROSE:
				case CONTENT_FLOWER_DAFFODIL:
				case CONTENT_FLOWER_TULIP:
				{
					MapNode n_under = m_map->getNodeNoEx(p+v3s16(0,-1,0));
					if (n_under.getContent() == CONTENT_GRASS || n_under.getContent() == CONTENT_GRASS_AUTUMN) {
						u32 chance = 0;
						switch (season) {
						case ENV_SEASON_AUTUMN:
							chance = 10;
							break;
						case ENV_SEASON_WINTER:
							chance = 5;
							break;
						default:;
						}
						if (chance && myrand_range(0,chance) == 0) {
							n.setContent(CONTENT_WILDGRASS_SHORT);
							m_map->addNodeWithEvent(p, n);
						}
					}else if (n_under.getContent() != CONTENT_FLOWER_POT && n_under.getContent() != CONTENT_FARM_DIRT) {
						n.setContent(CONTENT_WILDGRASS_SHORT);
						m_map->addNodeWithEvent(p, n);
					}
					break;
				}

				// cactus flowers and fruit
				case CONTENT_CACTUS:
				{
					if (n.envticks > 30) {
						bool fully_grown = false;
						int found = 1;
						v3s16 p_test = p;
						MapNode n_test = m_map->getNodeNoEx(v3s16(p.X, p.Y+1, p.Z));

						// can't grow anything if there's something above the cactus
						if (n_test.getContent() != CONTENT_AIR)
							break;

						while (fully_grown == false) {
							p_test.Y--;
							n_test = m_map->getNodeNoEx(p_test);

							// look down the cactus counting the number of cactus nodes
							if (n_test.getContent() == CONTENT_CACTUS) {
								found++;

								// cacti don't grow above 4 naturally, don't grow flowers on tall cactus-pillars
								if (found > 4) {
									break;
								}
							}else{
								// cacti grow to 3 nodes on sand
								// and 4 nodes on farm dirt
								if (n_test.getContent() == CONTENT_SAND) {
									if (found == 3) {
										fully_grown = true;
										break;
									}else{
										break;
									}
								}else if (n_test.getContent() == CONTENT_FARM_DIRT) {
									if (found == 4) {
										fully_grown = true;
										break;
									}else{
										break;
									}
								}
							}
						}

						if (fully_grown == true) {
							n.setContent(CONTENT_CACTUS_BLOSSOM);
							m_map->addNodeWithEvent(v3s16(p.X, p.Y+1, p.Z), n);
						}
					}
					break;
				}

				case CONTENT_CACTUS_BLOSSOM:
				{
					if (n.envticks > 30) {
						MapNode n_test=m_map->getNodeNoEx(v3s16(p.X, p.Y-1, p.Z));
						if (n_test.getContent() == CONTENT_CACTUS) {
							n.setContent(CONTENT_CACTUS_FLOWER);
							m_map->addNodeWithEvent(p, n);
						}
					}
					break;
				}

				case CONTENT_CACTUS_FLOWER:
				{
					if (n.envticks > 30) {
						MapNode n_test=m_map->getNodeNoEx(v3s16(p.X, p.Y-1, p.Z));
						// sometimes fruit, sometimes the flower dies
						if (n_test.getContent() == CONTENT_CACTUS && myrand()%10 < 6) {
							n.setContent(CONTENT_CACTUS_FRUIT);
							m_map->addNodeWithEvent(p, n);
						}else{
							m_map->removeNodeWithEvent(p);
						}
					}
					break;
				}

				case CONTENT_CACTUS_FRUIT:
				{
					if (n.envticks > 60) {
						MapNode n_test=m_map->getNodeNoEx(v3s16(p.X, p.Y-1, p.Z));
						// when the fruit dies, sometimes a new blossom appears
						if (n_test.getContent() == CONTENT_CACTUS && myrand()%10 == 0) {
							n.setContent(CONTENT_CACTUS_BLOSSOM);
							m_map->addNodeWithEvent(p, n);
						}else{
							m_map->removeNodeWithEvent(p);
							InventoryItem *item = InventoryItem::create(CONTENT_CRAFTITEM_MUSH,1,0,0);
							dropToParcel(p,item);
						}
					}
					break;
				}

				// growing apples!
				case CONTENT_APPLE_LEAVES:
				{
					std::vector<content_t> search;
					search.push_back(CONTENT_APPLE_TREE);
					search.push_back(CONTENT_YOUNG_APPLE_TREE);
					search.push_back(CONTENT_IGNORE);
					if (!searchNear(p,v3s16(3,3,3),search,NULL)) {
						m_map->removeNodeWithEvent(p);
						if (myrand()%10 == 0) {
							InventoryItem *item = InventoryItem::create(CONTENT_APPLE_LEAVES,1,0,0);
							/* actual drops/places grass or a sapling */
							dropToParcel(p,item);
						}
					}else if (
						n.envticks%30 == 0
						&& (
							season == ENV_SEASON_WINTER
							|| season == ENV_SEASON_SPRING
						) && (
							biome == BIOME_UNKNOWN
							|| biome == BIOME_LAKE
							|| biome == BIOME_WOODLANDS
							|| biome == BIOME_FOREST
							|| biome == BIOME_PLAINS
						)
					) {
						if (searchNear(p,v3s16(3,3,3),CONTENT_APPLE_TREE,NULL)) {
							if (!searchNear(p,v3s16(1,1,1),CONTENT_APPLE_BLOSSOM,NULL)) {
								n.setContent(CONTENT_APPLE_BLOSSOM);
								m_map->addNodeWithEvent(p, n);
							}
						}
					}
					break;
				}
				case CONTENT_JUNGLELEAVES:
				{
					std::vector<content_t> search;
					search.push_back(CONTENT_JUNGLETREE);
					search.push_back(CONTENT_YOUNG_JUNGLETREE);
					search.push_back(CONTENT_IGNORE);
					if (!searchNear(p,v3s16(3,3,3),search,NULL)) {
						m_map->removeNodeWithEvent(p);
						if (myrand()%10 == 0) {
							InventoryItem *item = InventoryItem::create(CONTENT_JUNGLELEAVES,1,0,0);
							/* actual drops/places grass or a sapling */
							dropToParcel(p,item);
						}
					}
					break;
				}
				case CONTENT_CONIFER_LEAVES:
				{
					std::vector<content_t> search;
					search.push_back(CONTENT_CONIFER_TREE);
					search.push_back(CONTENT_YOUNG_CONIFER_TREE);
					search.push_back(CONTENT_IGNORE);
					if (!searchNear(p,v3s16(3,3,3),search,NULL)) {
						m_map->removeNodeWithEvent(p);
						if (myrand()%10 == 0) {
							InventoryItem *item = InventoryItem::create(CONTENT_CONIFER_LEAVES,1,0,0);
							/* actual drops/places grass or a sapling */
							dropToParcel(p,item);
						}
					}
					break;
				}

				// leaf decay
				case CONTENT_LEAVES:
				case CONTENT_LEAVES_AUTUMN:
				case CONTENT_LEAVES_WINTER:
				case CONTENT_LEAVES_SNOWY:
				{
					if (myrand()%4 == 0) {
						v3s16 leaf_p = p;
						std::vector<content_t> search;
						search.push_back(CONTENT_TREE);
						search.push_back(CONTENT_YOUNG_TREE);
						search.push_back(CONTENT_IGNORE);
						if (biome == BIOME_WASTELANDS) {
							m_map->removeNodeWithEvent(leaf_p);
						}else if (!searchNear(p,v3s16(3,3,3),search,NULL)) {
							m_map->removeNodeWithEvent(leaf_p);
							if (myrand()%10 == 0) {
								InventoryItem *item = InventoryItem::create(CONTENT_LEAVES,1,0,0);
								/* actual drops/places grass or a sapling */
								dropToParcel(p,item);
							}
						}else if (biome == BIOME_DESERT || biome == BIOME_THEDEEP) {
							if (n.getContent() != CONTENT_LEAVES_AUTUMN) {
								n.setContent(CONTENT_LEAVES_AUTUMN);
								m_map->addNodeWithEvent(p,n);
							}
						}else if (biome == BIOME_SPACE) {
							if (n.getContent() != CONTENT_LEAVES) {
								n.setContent(CONTENT_LEAVES);
								m_map->addNodeWithEvent(p,n);
							}
						}else if (n.getContent() == CONTENT_LEAVES) {
							if (season == ENV_SEASON_AUTUMN) {
								n.setContent(CONTENT_LEAVES_AUTUMN);
								m_map->addNodeWithEvent(p,n);
							}else if (season == ENV_SEASON_WINTER) {
								n.setContent(CONTENT_LEAVES_WINTER);
								m_map->addNodeWithEvent(p,n);
							}
						}else if (n.getContent() == CONTENT_LEAVES_AUTUMN) {
							if (season == ENV_SEASON_WINTER) {
								n.setContent(CONTENT_LEAVES_WINTER);
								m_map->addNodeWithEvent(p,n);
							}else if (season != ENV_SEASON_AUTUMN) {
								n.setContent(CONTENT_LEAVES);
								m_map->addNodeWithEvent(p,n);
							}
						}else if (n.getContent() == CONTENT_LEAVES_WINTER) {
							if (season == ENV_SEASON_AUTUMN) {
								n.setContent(CONTENT_LEAVES_AUTUMN);
								m_map->addNodeWithEvent(p,n);
							}else if (season == ENV_SEASON_WINTER) {
								if (myrand_range(0,5) && p.Y > 0) {
									n.setContent(CONTENT_LEAVES_SNOWY);
									m_map->addNodeWithEvent(p,n);
								}
							}else{
								n.setContent(CONTENT_LEAVES);
								m_map->addNodeWithEvent(p,n);
							}
						}else if (n.getContent() == CONTENT_LEAVES_SNOWY) {
							if (season != ENV_SEASON_WINTER) {
								n.setContent(CONTENT_LEAVES_WINTER);
								m_map->addNodeWithEvent(p,n);
							}
						}
					}
					break;
				}

				case CONTENT_APPLE_BLOSSOM:
				{
					if (n.envticks > 30) {
						// don't turn all blossoms to apples
						// blossoms look nice
						if (searchNear(p,v3s16(3,3,3),CONTENT_APPLE_TREE,NULL)) {
							int found_apple = 0;

							for(s16 x=-2; x<=2; x++)
							for(s16 y=-2; y<=2; y++)
							for(s16 z=-2; z<=2; z++)
							{
								MapNode n_test = m_map->getNodeNoEx(p+v3s16(x,y,z));
								if (n_test.getContent() == CONTENT_APPLE) {
									++found_apple;
								}
							}
							if (found_apple < season) {
								n.setContent(CONTENT_APPLE);
								m_map->addNodeWithEvent(p, n);
							}
						}else{
							m_map->removeNodeWithEvent(p);
							if (myrand()%5 == 0) {
								m_map->removeNodeWithEvent(p);
								InventoryItem *item = InventoryItem::create(CONTENT_CRAFTITEM_APPLE_BLOSSOM,1,0,0);
								dropToParcel(p,item);
							}
						}
					}
					break;
				}

				// fire that goes out
				case CONTENT_FIRE_SHORTTERM:
				{
					if (unsafe_fire) {
						if (n.envticks > 2) {
							s16 bs_rad = config_get_int("world.game.borderstone.radius");
							bs_rad += 2;
							// if any node is border stone protected, don't spread
							if (!searchNear(p,v3s16(bs_rad,bs_rad,bs_rad),CONTENT_BORDERSTONE,NULL)) {
								for(s16 x=-1; x<=1; x++)
								for(s16 y=-1; y<=1; y++)
								for(s16 z=-1; z<=1; z++)
								{
									MapNode n_test = m_map->getNodeNoEx(p+v3s16(x,y,z));
									if (n_test.getContent() == CONTENT_FIRE || n_test.getContent() == CONTENT_FIRE_SHORTTERM)
										continue;
									if (content_features(n_test).flammable > 0) {
										content_t c = n_test.getContent();
										if (content_features(c).onact_also_affects != v3s16(0,0,0)) {
											v3s16 p_other = p+v3s16(x,y,z)+n_test.getEffectedRotation();
											n_test.setContent(CONTENT_FIRE_SHORTTERM);
											m_map->addNodeWithEvent(p_other, n_test);
										}
										n_test.setContent(CONTENT_FIRE_SHORTTERM);
										m_map->addNodeWithEvent(p+v3s16(x,y,z), n_test);
									}
								}
							}
						}
						if (n.envticks > 10) {
							m_map->removeNodeWithEvent(p);
							InventoryItem *item = InventoryItem::create(CONTENT_CRAFTITEM_ASH,1,0,0);
							dropToParcel(p,item);
						}
					}else if (n.envticks > 2) {
						m_map->removeNodeWithEvent(p);
						InventoryItem *item = InventoryItem::create(CONTENT_CRAFTITEM_ASH,1,0,0);
						dropToParcel(p,item);
					}
					break;
				}

				// fire that spreads just a little
				case CONTENT_FIRE:
				{
					MapNode n_below = m_map->getNodeNoEx(p+v3s16(0,-1,0));
					if (!content_features(n_below).flammable) {
						m_map->removeNodeWithEvent(p);
					}else{
						s16 bs_rad = config_get_int("world.game.borderstone.radius");
						bs_rad += 2;
						// if any node is border stone protected, don't spread
						if (!searchNear(p,v3s16(bs_rad,bs_rad,bs_rad),CONTENT_BORDERSTONE,NULL)) {
							for(s16 x=-1; x<=1; x++)
							for(s16 y=0; y<=1; y++)
							for(s16 z=-1; z<=1; z++)
							{
								MapNode n_test = m_map->getNodeNoEx(p+v3s16(x,y,z));
								if (n_test.getContent() == CONTENT_FIRE || n_test.getContent() == CONTENT_FIRE_SHORTTERM)
									continue;
								if (content_features(n_test).flammable > 0) {
									content_t c = n_test.getContent();
									if (content_features(c).onact_also_affects != v3s16(0,0,0)) {
										v3s16 p_other = p+v3s16(x,y,z)+n_test.getEffectedRotation();
										n_test.setContent(CONTENT_FIRE_SHORTTERM);
										m_map->addNodeWithEvent(p_other, n_test);
									}
									n_test.setContent(CONTENT_FIRE_SHORTTERM);
									m_map->addNodeWithEvent(p+v3s16(x,y,z), n_test);
								}
							}
						}
					}
					break;
				}

				// boom
				case CONTENT_FLASH:
					m_map->removeNodeWithEvent(p);
					break;

				// boom
				case CONTENT_TNT:
				{
					NodeMetadata *meta = m_map->getNodeMetadata(p);
					if (meta && meta->getEnergy() == ENERGY_MAX) {
						if (config_get_bool("world.game.environment.tnt")) {
							s16 bs_rad = config_get_int("world.game.borderstone.radius");
							bs_rad += 3;
							// if any node is border stone protected, don't destroy anything
							if (!searchNear(p,v3s16(bs_rad,bs_rad,bs_rad),CONTENT_BORDERSTONE,NULL)) {
								for(s16 x=-2; x<=2; x++)
								for(s16 y=-2; y<=2; y++)
								for(s16 z=-2; z<=2; z++)
								{
									MapNode n_test = m_map->getNodeNoEx(p+v3s16(x,y,z));
									if (n_test.getContent() == CONTENT_AIR)
										continue;
									if (n_test.getContent() == CONTENT_TNT) {
										meta = m_map->getNodeMetadata(p+v3s16(x,y,z));
										if (meta && !meta->getEnergy())
											meta->energise(ENERGY_MAX,p,p,p+v3s16(x,y,z));
										continue;
									}
									if (
										(x == -2 && y == -2)
										|| (x == 2 && y == -2)
										|| (x == -2 && y == 2)
										|| (x == 2 && y == 2)
										|| (z == -2 && y == -2)
										|| (z == 2 && y == -2)
										|| (z == -2 && y == 2)
										|| (z == 2 && y == 2)
										|| (x == -2 && z == -2)
										|| (x == 2 && z == -2)
										|| (x == -2 && z == 2)
										|| (x == 2 && z == 2)
									) {
										if (myrand()%3 == 0)
											continue;
									}
									n_test.setContent(CONTENT_FLASH);
									m_delayed_node_changes[p+v3s16(x,y,z)] = n_test;
								}
							}
						}
						// but still blow up
						m_map->removeNodeWithEvent(p);
						addEnvEvent(ENV_EVENT_SOUND,intToFloat(p,BS),"env-tnt");
					}
					break;
				}

				// cobble becomes mossy underwater
				case CONTENT_COBBLE:
				{
					if (n.envticks > 30 && n.envticks%4 == 0) {
						MapNode a = m_map->getNodeNoEx(p+v3s16(0,1,0));
						if (a.getContent() == CONTENT_WATERSOURCE) {
							n.setContent(CONTENT_MOSSYCOBBLE);
							m_map->addNodeWithEvent(p,n);
						}else{
							bool found = false;
							/* moss also grows */
							for (s16 i=0; !found && i<6; i++) {
								a = m_map->getNodeNoEx(p+g_6dirs[i]);
								if (a.getContent() == CONTENT_MOSSYCOBBLE) {
									n.setContent(CONTENT_MOSSYCOBBLE);
									m_map->addNodeWithEvent(p,n);
									found = true;
								}
							}
						}
					}
					break;
				}

				// Make trees from saplings!
				case CONTENT_SAPLING:
				{
					if (n.envticks > 1000) {
						// full grown tree
						actionstream<<"A sapling grows into a tree at "<<PP(p)<<std::endl;
						std::vector<content_t> search;
						search.push_back(CONTENT_AIR);
						search.push_back(CONTENT_TREE);
						search.push_back(CONTENT_YOUNG_TREE);
						search.push_back(CONTENT_APPLE_TREE);
						search.push_back(CONTENT_YOUNG_APPLE_TREE);
						search.push_back(CONTENT_JUNGLETREE);
						search.push_back(CONTENT_YOUNG_JUNGLETREE);
						search.push_back(CONTENT_CONIFER_TREE);
						search.push_back(CONTENT_YOUNG_CONIFER_TREE);
						search.push_back(CONTENT_LEAVES);
						search.push_back(CONTENT_LEAVES_AUTUMN);
						search.push_back(CONTENT_LEAVES_WINTER);
						search.push_back(CONTENT_LEAVES_SNOWY);
						search.push_back(CONTENT_JUNGLELEAVES);
						search.push_back(CONTENT_CONIFER_LEAVES);
						search.push_back(CONTENT_APPLE_LEAVES);
						search.push_back(CONTENT_APPLE_BLOSSOM);
						search.push_back(CONTENT_APPLE);
						search.push_back(CONTENT_IGNORE);

						core::map<v3s16, MapBlock*> modified_blocks;
						if (!searchNearInv(p,v3s16(-10,2,-10),v3s16(10,12,10),search,NULL)) {
							plantgrowth_largetree(this,p);
						}else{
							plantgrowth_tree(this,p);
						}
					}else if (n.envticks > 15) {
						std::vector<content_t> search;
						search.push_back(CONTENT_AIR);
						search.push_back(CONTENT_TREE);
						search.push_back(CONTENT_YOUNG_TREE);
						search.push_back(CONTENT_APPLE_TREE);
						search.push_back(CONTENT_YOUNG_APPLE_TREE);
						search.push_back(CONTENT_JUNGLETREE);
						search.push_back(CONTENT_YOUNG_JUNGLETREE);
						search.push_back(CONTENT_CONIFER_TREE);
						search.push_back(CONTENT_YOUNG_CONIFER_TREE);
						search.push_back(CONTENT_LEAVES);
						search.push_back(CONTENT_LEAVES_AUTUMN);
						search.push_back(CONTENT_LEAVES_WINTER);
						search.push_back(CONTENT_LEAVES_SNOWY);
						search.push_back(CONTENT_JUNGLELEAVES);
						search.push_back(CONTENT_CONIFER_LEAVES);
						search.push_back(CONTENT_APPLE_LEAVES);
						search.push_back(CONTENT_APPLE_BLOSSOM);
						search.push_back(CONTENT_APPLE);
						search.push_back(CONTENT_IGNORE);
						content_t below = m_map->getNodeNoEx(p+v3s16(0,-1,0)).getContent();
						if (
							below == CONTENT_MUD
						) {
							v3s16 h;
							if (!searchNearInv(p,v3s16(-2,2,-2),v3s16(2,7,2),search,&h)) {
								// young tree 1
								MapNode nn(CONTENT_YOUNG_TREE);
								m_map->addNodeWithEvent(p,nn);
								nn.setContent(CONTENT_LEAVES);
								m_map->addNodeWithEvent(p+v3s16(0,1,0),nn);
							}
						}
					}
				}
				break;
				case CONTENT_YOUNG_TREE:
				{
					if (n.envticks > 15) {
						content_t below = m_map->getNodeNoEx(p+v3s16(0,-1,0)).getContent();
						if (
							below == CONTENT_MUD
						) {
							content_t above = m_map->getNodeNoEx(p+v3s16(0,1,0)).getContent();
							std::vector<content_t> search;
							search.push_back(CONTENT_AIR);
							search.push_back(CONTENT_TREE);
							search.push_back(CONTENT_YOUNG_TREE);
							search.push_back(CONTENT_APPLE_TREE);
							search.push_back(CONTENT_YOUNG_APPLE_TREE);
							search.push_back(CONTENT_JUNGLETREE);
							search.push_back(CONTENT_YOUNG_JUNGLETREE);
							search.push_back(CONTENT_CONIFER_TREE);
							search.push_back(CONTENT_YOUNG_CONIFER_TREE);
							search.push_back(CONTENT_LEAVES);
							search.push_back(CONTENT_LEAVES_AUTUMN);
							search.push_back(CONTENT_LEAVES_WINTER);
							search.push_back(CONTENT_LEAVES_SNOWY);
							search.push_back(CONTENT_JUNGLELEAVES);
							search.push_back(CONTENT_CONIFER_LEAVES);
							search.push_back(CONTENT_APPLE_LEAVES);
							search.push_back(CONTENT_APPLE_BLOSSOM);
							search.push_back(CONTENT_APPLE);
							search.push_back(CONTENT_IGNORE);
							if (above == CONTENT_LEAVES) {
								// young tree 2
								v3s16 h;
								if (!searchNearInv(p,v3s16(-1,2,-1),v3s16(1,4,1),search,&h)) {
									MapNode nn(CONTENT_YOUNG_TREE);
									m_map->addNodeWithEvent(p+v3s16(0,1,0),nn);
									m_map->addNodeWithEvent(p+v3s16(0,2,0),nn);
									nn.setContent(CONTENT_LEAVES);
									m_map->addNodeWithEvent(p+v3s16(0,3,0),nn);
									m_map->addNodeWithEvent(p+v3s16(1,2,0),nn);
									m_map->addNodeWithEvent(p+v3s16(-1,2,0),nn);
									m_map->addNodeWithEvent(p+v3s16(0,2,1),nn);
									m_map->addNodeWithEvent(p+v3s16(0,2,-1),nn);
								}
							}else if (above == CONTENT_YOUNG_TREE && n.envticks > 40) {
								content_t abv = m_map->getNodeNoEx(p+v3s16(0,2,0)).getContent();
								content_t top = m_map->getNodeNoEx(p+v3s16(0,3,0)).getContent();
								if (abv == CONTENT_YOUNG_TREE && top == CONTENT_LEAVES) {
									if (m_map->getNodeNoEx(p+v3s16(1,2,0)).getContent() == CONTENT_LEAVES)
										m_map->removeNodeWithEvent(p+v3s16(1,2,0));
									if (m_map->getNodeNoEx(p+v3s16(-1,2,0)).getContent() == CONTENT_LEAVES)
										m_map->removeNodeWithEvent(p+v3s16(-1,2,0));
									if (m_map->getNodeNoEx(p+v3s16(0,2,1)).getContent() == CONTENT_LEAVES)
										m_map->removeNodeWithEvent(p+v3s16(0,2,1));
									if (m_map->getNodeNoEx(p+v3s16(0,2,-1)).getContent() == CONTENT_LEAVES)
										m_map->removeNodeWithEvent(p+v3s16(0,2,-1));
									// full grown tree
									actionstream<<"A sapling grows into a tree at "
										<<PP(p)<<std::endl;

									if (!searchNearInv(p,v3s16(-10,2,-10),v3s16(10,12,10),search,NULL)) {
										plantgrowth_largetree(this,p);
									}else{
										plantgrowth_tree(this,p);
									}
								}
							}
						}
					}
					break;
				}

				case CONTENT_APPLE_SAPLING:
				{
					if (n.envticks > 1000) {
						actionstream<<"A sapling grows into a tree at "<<PP(p)<<std::endl;

						plantgrowth_appletree(this,p);
					}else if (n.envticks > 15) {
						std::vector<content_t> search;
						search.push_back(CONTENT_AIR);
						search.push_back(CONTENT_TREE);
						search.push_back(CONTENT_YOUNG_TREE);
						search.push_back(CONTENT_APPLE_TREE);
						search.push_back(CONTENT_YOUNG_APPLE_TREE);
						search.push_back(CONTENT_JUNGLETREE);
						search.push_back(CONTENT_YOUNG_JUNGLETREE);
						search.push_back(CONTENT_CONIFER_TREE);
						search.push_back(CONTENT_YOUNG_CONIFER_TREE);
						search.push_back(CONTENT_LEAVES);
						search.push_back(CONTENT_LEAVES_AUTUMN);
						search.push_back(CONTENT_LEAVES_WINTER);
						search.push_back(CONTENT_LEAVES_SNOWY);
						search.push_back(CONTENT_JUNGLELEAVES);
						search.push_back(CONTENT_CONIFER_LEAVES);
						search.push_back(CONTENT_APPLE_LEAVES);
						search.push_back(CONTENT_APPLE_BLOSSOM);
						search.push_back(CONTENT_APPLE);
						search.push_back(CONTENT_IGNORE);
						content_t below = m_map->getNodeNoEx(p+v3s16(0,-1,0)).getContent();
						if (
							below == CONTENT_MUD
						) {
							v3s16 h;
							if (!searchNearInv(p,v3s16(-2,2,-2),v3s16(2,7,2),search,&h)) {
								// young tree 1
								MapNode nn(CONTENT_YOUNG_APPLE_TREE);
								m_map->addNodeWithEvent(p,nn);
								nn.setContent(CONTENT_APPLE_LEAVES);
								m_map->addNodeWithEvent(p+v3s16(0,1,0),nn);
							}
						}
					}
				}
				break;
				case CONTENT_YOUNG_APPLE_TREE:
				{
					if (n.envticks > 15) {
						content_t below = m_map->getNodeNoEx(p+v3s16(0,-1,0)).getContent();
						if (
							below == CONTENT_MUD
						) {
							content_t above = m_map->getNodeNoEx(p+v3s16(0,1,0)).getContent();
							std::vector<content_t> search;
							search.push_back(CONTENT_AIR);
							search.push_back(CONTENT_TREE);
							search.push_back(CONTENT_YOUNG_TREE);
							search.push_back(CONTENT_APPLE_TREE);
							search.push_back(CONTENT_YOUNG_APPLE_TREE);
							search.push_back(CONTENT_JUNGLETREE);
							search.push_back(CONTENT_YOUNG_JUNGLETREE);
							search.push_back(CONTENT_CONIFER_TREE);
							search.push_back(CONTENT_YOUNG_CONIFER_TREE);
							search.push_back(CONTENT_LEAVES);
							search.push_back(CONTENT_LEAVES_AUTUMN);
							search.push_back(CONTENT_LEAVES_WINTER);
							search.push_back(CONTENT_LEAVES_SNOWY);
							search.push_back(CONTENT_JUNGLELEAVES);
							search.push_back(CONTENT_CONIFER_LEAVES);
							search.push_back(CONTENT_APPLE_LEAVES);
							search.push_back(CONTENT_APPLE_BLOSSOM);
							search.push_back(CONTENT_APPLE);
							search.push_back(CONTENT_IGNORE);
							if (above == CONTENT_APPLE_LEAVES) {
								// young tree 2
								v3s16 h;
								if (!searchNearInv(p,v3s16(-1,2,-1),v3s16(1,4,1),search,&h)) {
									MapNode nn(CONTENT_YOUNG_APPLE_TREE);
									m_map->addNodeWithEvent(p+v3s16(0,1,0),nn);
									m_map->addNodeWithEvent(p+v3s16(0,2,0),nn);
									nn.setContent(CONTENT_APPLE_LEAVES);
									m_map->addNodeWithEvent(p+v3s16(0,3,0),nn);
									m_map->addNodeWithEvent(p+v3s16(1,2,0),nn);
									m_map->addNodeWithEvent(p+v3s16(-1,2,0),nn);
									m_map->addNodeWithEvent(p+v3s16(0,2,1),nn);
									m_map->addNodeWithEvent(p+v3s16(0,2,-1),nn);
								}
							}else if (above == CONTENT_YOUNG_APPLE_TREE && n.envticks > 40) {
								content_t abv = m_map->getNodeNoEx(p+v3s16(0,2,0)).getContent();
								content_t top = m_map->getNodeNoEx(p+v3s16(0,3,0)).getContent();
								if (abv == CONTENT_YOUNG_APPLE_TREE && top == CONTENT_APPLE_LEAVES) {
									if (m_map->getNodeNoEx(p+v3s16(1,2,0)).getContent() == CONTENT_APPLE_LEAVES)
										m_map->removeNodeWithEvent(p+v3s16(1,2,0));
									if (m_map->getNodeNoEx(p+v3s16(-1,2,0)).getContent() == CONTENT_APPLE_LEAVES)
										m_map->removeNodeWithEvent(p+v3s16(-1,2,0));
									if (m_map->getNodeNoEx(p+v3s16(0,2,1)).getContent() == CONTENT_APPLE_LEAVES)
										m_map->removeNodeWithEvent(p+v3s16(0,2,1));
									if (m_map->getNodeNoEx(p+v3s16(0,2,-1)).getContent() == CONTENT_APPLE_LEAVES)
										m_map->removeNodeWithEvent(p+v3s16(0,2,-1));
									actionstream<<"A sapling grows into a tree at "
										<<PP(p)<<std::endl;

									plantgrowth_appletree(this,p);
								}
							}
						}
					}
					break;
				}

				case CONTENT_JUNGLESAPLING:
				{
					if (n.envticks > 1000) {
						actionstream<<"A sapling grows into a jungle tree at "<<PP(p)<<std::endl;

						plantgrowth_jungletree(this,p);
					}else if (n.envticks > 15) {
						std::vector<content_t> search;
						search.push_back(CONTENT_AIR);
						search.push_back(CONTENT_TREE);
						search.push_back(CONTENT_YOUNG_TREE);
						search.push_back(CONTENT_APPLE_TREE);
						search.push_back(CONTENT_YOUNG_APPLE_TREE);
						search.push_back(CONTENT_JUNGLETREE);
						search.push_back(CONTENT_YOUNG_JUNGLETREE);
						search.push_back(CONTENT_CONIFER_TREE);
						search.push_back(CONTENT_YOUNG_CONIFER_TREE);
						search.push_back(CONTENT_LEAVES);
						search.push_back(CONTENT_LEAVES_AUTUMN);
						search.push_back(CONTENT_LEAVES_WINTER);
						search.push_back(CONTENT_LEAVES_SNOWY);
						search.push_back(CONTENT_JUNGLELEAVES);
						search.push_back(CONTENT_CONIFER_LEAVES);
						search.push_back(CONTENT_APPLE_LEAVES);
						search.push_back(CONTENT_APPLE_BLOSSOM);
						search.push_back(CONTENT_APPLE);
						search.push_back(CONTENT_IGNORE);
						content_t below = m_map->getNodeNoEx(p+v3s16(0,-1,0)).getContent();
						if (
							below == CONTENT_MUD
						) {
							v3s16 h;
							if (!searchNearInv(p,v3s16(-2,2,-2),v3s16(2,10,2),search,&h)) {
								// young tree 1
								MapNode nn(CONTENT_YOUNG_JUNGLETREE);
								m_map->addNodeWithEvent(p,nn);
								nn.setContent(CONTENT_JUNGLELEAVES);
								m_map->addNodeWithEvent(p+v3s16(0,1,0),nn);
							}
						}
					}
				}
				break;
				case CONTENT_YOUNG_JUNGLETREE:
				{
					if (n.envticks > 15) {
						content_t below = m_map->getNodeNoEx(p+v3s16(0,-1,0)).getContent();
						if (
							below == CONTENT_MUD
						) {
							content_t above = m_map->getNodeNoEx(p+v3s16(0,1,0)).getContent();
							std::vector<content_t> search;
							search.push_back(CONTENT_AIR);
							search.push_back(CONTENT_TREE);
							search.push_back(CONTENT_YOUNG_TREE);
							search.push_back(CONTENT_APPLE_TREE);
							search.push_back(CONTENT_YOUNG_APPLE_TREE);
							search.push_back(CONTENT_JUNGLETREE);
							search.push_back(CONTENT_YOUNG_JUNGLETREE);
							search.push_back(CONTENT_CONIFER_TREE);
							search.push_back(CONTENT_YOUNG_CONIFER_TREE);
							search.push_back(CONTENT_LEAVES);
							search.push_back(CONTENT_LEAVES_AUTUMN);
							search.push_back(CONTENT_LEAVES_WINTER);
							search.push_back(CONTENT_LEAVES_SNOWY);
							search.push_back(CONTENT_JUNGLELEAVES);
							search.push_back(CONTENT_CONIFER_LEAVES);
							search.push_back(CONTENT_APPLE_LEAVES);
							search.push_back(CONTENT_APPLE_BLOSSOM);
							search.push_back(CONTENT_APPLE);
							search.push_back(CONTENT_IGNORE);
							if (above == CONTENT_JUNGLELEAVES) {
								// young tree 2
								v3s16 h;
								if (!searchNearInv(p,v3s16(-1,2,-1),v3s16(1,5,1),search,&h)) {
									MapNode nn(CONTENT_YOUNG_JUNGLETREE);
									m_map->addNodeWithEvent(p+v3s16(0,1,0),nn);
									m_map->addNodeWithEvent(p+v3s16(0,2,0),nn);
									m_map->addNodeWithEvent(p+v3s16(0,3,0),nn);
									nn.setContent(CONTENT_JUNGLELEAVES);
									m_map->addNodeWithEvent(p+v3s16(0,4,0),nn);
									m_map->addNodeWithEvent(p+v3s16(1,3,0),nn);
									m_map->addNodeWithEvent(p+v3s16(-1,3,0),nn);
									m_map->addNodeWithEvent(p+v3s16(0,3,1),nn);
									m_map->addNodeWithEvent(p+v3s16(0,3,-1),nn);
								}
							}else if (above == CONTENT_YOUNG_JUNGLETREE && n.envticks > 40) {
								content_t abv = m_map->getNodeNoEx(p+v3s16(0,2,0)).getContent();
								content_t abv1 = m_map->getNodeNoEx(p+v3s16(0,3,0)).getContent();
								content_t top = m_map->getNodeNoEx(p+v3s16(0,4,0)).getContent();
								if (abv == CONTENT_YOUNG_JUNGLETREE && abv1 == CONTENT_YOUNG_JUNGLETREE && top == CONTENT_JUNGLELEAVES) {
									if (m_map->getNodeNoEx(p+v3s16(1,3,0)).getContent() == CONTENT_JUNGLELEAVES)
										m_map->removeNodeWithEvent(p+v3s16(1,3,0));
									if (m_map->getNodeNoEx(p+v3s16(-1,3,0)).getContent() == CONTENT_JUNGLELEAVES)
										m_map->removeNodeWithEvent(p+v3s16(-1,3,0));
									if (m_map->getNodeNoEx(p+v3s16(0,3,1)).getContent() == CONTENT_JUNGLELEAVES)
										m_map->removeNodeWithEvent(p+v3s16(0,3,1));
									if (m_map->getNodeNoEx(p+v3s16(0,3,-1)).getContent() == CONTENT_JUNGLELEAVES)
										m_map->removeNodeWithEvent(p+v3s16(0,3,-1));
									actionstream<<"A sapling grows into a jungle tree at "
										<<PP(p)<<std::endl;

									plantgrowth_jungletree(this,p);
								}
							}
						}
					}
					break;
				}

				case CONTENT_CONIFER_SAPLING:
				{
					if (n.envticks > 1000) {
						actionstream<<"A sapling grows into a conifer tree at "<<PP(p)<<std::endl;

						plantgrowth_conifertree(this,p);
					}else if (n.envticks > 15) {
						std::vector<content_t> search;
						search.push_back(CONTENT_AIR);
						search.push_back(CONTENT_TREE);
						search.push_back(CONTENT_YOUNG_TREE);
						search.push_back(CONTENT_APPLE_TREE);
						search.push_back(CONTENT_YOUNG_APPLE_TREE);
						search.push_back(CONTENT_JUNGLETREE);
						search.push_back(CONTENT_YOUNG_JUNGLETREE);
						search.push_back(CONTENT_CONIFER_TREE);
						search.push_back(CONTENT_YOUNG_CONIFER_TREE);
						search.push_back(CONTENT_LEAVES);
						search.push_back(CONTENT_LEAVES_AUTUMN);
						search.push_back(CONTENT_LEAVES_WINTER);
						search.push_back(CONTENT_LEAVES_SNOWY);
						search.push_back(CONTENT_JUNGLELEAVES);
						search.push_back(CONTENT_CONIFER_LEAVES);
						search.push_back(CONTENT_APPLE_LEAVES);
						search.push_back(CONTENT_APPLE_BLOSSOM);
						search.push_back(CONTENT_APPLE);
						search.push_back(CONTENT_IGNORE);
						content_t below = m_map->getNodeNoEx(p+v3s16(0,-1,0)).getContent();
						if (
							below == CONTENT_MUD
						) {
							v3s16 h;
							if (!searchNearInv(p,v3s16(-2,2,-2),v3s16(2,12,2),search,&h)) {
								// young tree 1
								MapNode nn(CONTENT_YOUNG_CONIFER_TREE);
								m_map->addNodeWithEvent(p,nn);
								nn.setContent(CONTENT_CONIFER_LEAVES);
								m_map->addNodeWithEvent(p+v3s16(0,1,0),nn);
							}
						}
					}
				}
				break;
				case CONTENT_YOUNG_CONIFER_TREE:
				{
					if (n.envticks > 15) {
						content_t below = m_map->getNodeNoEx(p+v3s16(0,-1,0)).getContent();
						if (
							below == CONTENT_MUD
						) {
							content_t above = m_map->getNodeNoEx(p+v3s16(0,1,0)).getContent();
							std::vector<content_t> search;
							search.push_back(CONTENT_AIR);
							search.push_back(CONTENT_TREE);
							search.push_back(CONTENT_YOUNG_TREE);
							search.push_back(CONTENT_APPLE_TREE);
							search.push_back(CONTENT_YOUNG_APPLE_TREE);
							search.push_back(CONTENT_JUNGLETREE);
							search.push_back(CONTENT_YOUNG_JUNGLETREE);
							search.push_back(CONTENT_CONIFER_TREE);
							search.push_back(CONTENT_YOUNG_CONIFER_TREE);
							search.push_back(CONTENT_LEAVES);
							search.push_back(CONTENT_LEAVES_AUTUMN);
							search.push_back(CONTENT_LEAVES_WINTER);
							search.push_back(CONTENT_LEAVES_SNOWY);
							search.push_back(CONTENT_JUNGLELEAVES);
							search.push_back(CONTENT_CONIFER_LEAVES);
							search.push_back(CONTENT_APPLE_LEAVES);
							search.push_back(CONTENT_APPLE_BLOSSOM);
							search.push_back(CONTENT_APPLE);
							search.push_back(CONTENT_IGNORE);
							if (above == CONTENT_CONIFER_LEAVES) {
								// young tree 2
								v3s16 h;
								if (!searchNearInv(p,v3s16(-1,2,-1),v3s16(1,5,1),search,&h)) {
									MapNode nn(CONTENT_YOUNG_CONIFER_TREE);
									m_map->addNodeWithEvent(p+v3s16(0,1,0),nn);
									m_map->addNodeWithEvent(p+v3s16(0,2,0),nn);
									nn.setContent(CONTENT_CONIFER_LEAVES);
									m_map->addNodeWithEvent(p+v3s16(0,3,0),nn);
									m_map->addNodeWithEvent(p+v3s16(0,4,0),nn);
									m_map->addNodeWithEvent(p+v3s16(1,2,0),nn);
									m_map->addNodeWithEvent(p+v3s16(-1,2,0),nn);
									m_map->addNodeWithEvent(p+v3s16(0,2,1),nn);
									m_map->addNodeWithEvent(p+v3s16(0,2,-1),nn);
								}
							}else if (above == CONTENT_YOUNG_CONIFER_TREE && n.envticks > 40) {
								content_t abv = m_map->getNodeNoEx(p+v3s16(0,2,0)).getContent();
								content_t top = m_map->getNodeNoEx(p+v3s16(0,3,0)).getContent();
								if (abv == CONTENT_YOUNG_CONIFER_TREE && top == CONTENT_CONIFER_LEAVES) {
									if (m_map->getNodeNoEx(p+v3s16(1,2,0)).getContent() == CONTENT_CONIFER_LEAVES)
										m_map->removeNodeWithEvent(p+v3s16(1,2,0));
									if (m_map->getNodeNoEx(p+v3s16(-1,2,0)).getContent() == CONTENT_CONIFER_LEAVES)
										m_map->removeNodeWithEvent(p+v3s16(-1,2,0));
									if (m_map->getNodeNoEx(p+v3s16(0,2,1)).getContent() == CONTENT_CONIFER_LEAVES)
										m_map->removeNodeWithEvent(p+v3s16(0,2,1));
									if (m_map->getNodeNoEx(p+v3s16(0,2,-1)).getContent() == CONTENT_CONIFER_LEAVES)
										m_map->removeNodeWithEvent(p+v3s16(0,2,-1));
									actionstream<<"A sapling grows into a conifer tree at "
										<<PP(p)<<std::endl;

									plantgrowth_conifertree(this,p);
								}
							}
						}
					}
					break;
				}

				// Apples should fall if there is no leaves block holding it
				case CONTENT_APPLE:
				{
					v3s16 apple_p = p;
					std::vector<content_t> search;
					search.push_back(CONTENT_APPLE_LEAVES);
					search.push_back(CONTENT_IGNORE);
					if (!searchNear(p,v3s16(1,1,1),search,NULL)) {
						m_map->removeNodeWithEvent(apple_p);
						InventoryItem *item = InventoryItem::create(CONTENT_CRAFTITEM_APPLE,1,0,0);
						dropToParcel(p,item);
					}
					break;
				}

				// grow sponges on sand in water
				case CONTENT_SAND:
				{
					if (n.envticks%30 == 0) {
						MapNode n_top1 = m_map->getNodeNoEx(p+v3s16(0,1,0));
						MapNode n_top2 = m_map->getNodeNoEx(p+v3s16(0,2,0));
						if (
							biome == BIOME_OCEAN
							&& n_top1.getContent() == CONTENT_WATERSOURCE
							&& n_top2.getContent() == CONTENT_WATERSOURCE
						) {
							std::vector<content_t> search;
							search.push_back(CONTENT_SPONGE_FULL);
							search.push_back(CONTENT_IGNORE);
							if (!searchNear(p,v3s16(3,2,3),search,NULL)) {
								n_top1.setContent(CONTENT_SPONGE_FULL);
								m_map->addNodeWithEvent(p+v3s16(0,1,0), n_top1);
							}
						}
					}
					break;
				}

				// make sponge soak up water
				case CONTENT_SPONGE:
				{
					v3s16 test_p;
					MapNode testnode;
					bool sponge_soaked = false;
					s16 max_d = 2;
					for(s16 z=-max_d; z<=max_d; z++) {
					for(s16 y=-max_d; y<=max_d; y++) {
					for(s16 x=-max_d; x<=max_d; x++) {
						test_p = p + v3s16(x,y,z);
						testnode = m_map->getNodeNoEx(test_p);
						if (testnode.getContent() == CONTENT_WATERSOURCE) {
							sponge_soaked = true;
							testnode.setContent(CONTENT_AIR);
							m_map->addNodeWithEvent(test_p, testnode);
						}
					}
					}
					}
					if (sponge_soaked) {
						n.setContent(CONTENT_SPONGE_FULL);
						m_map->addNodeWithEvent(p, n);
					}
					break;
				}

				// make papyrus grow near water
				case CONTENT_PAPYRUS:
				{
					if (n.envticks%10 == 0) {
						MapNode n_btm = m_map->getNodeNoEx(p+v3s16(0,-1,0));
						if (n_btm.getContent() == CONTENT_MUD) {
							if (searchNear(p,v3s16(2,2,2),CONTENT_WATERSOURCE,NULL))
								plantgrowth_plant(this,p,3);
						}
					}
					break;
				}

				// steam dissipates
				case CONTENT_STEAM:
					m_map->removeNodeWithEvent(p);
					break;

				// make lava cool near water
				case CONTENT_LAVASOURCE:
				case CONTENT_LAVA:
				{
					MapNode testnode;
					v3s16 test_p;
					std::vector<content_t> search;
					bool found = false;
					search.push_back(CONTENT_WATER);
					search.push_back(CONTENT_VACUUM);
					search.push_back(CONTENT_WATERSOURCE);
					if (coldzone) {
						found = true;
					}else if (searchNear(p,v3s16(1,1,1),search,&test_p)) {
						testnode = m_map->getNodeNoEx(test_p);
						found = true;
						testnode.setContent(CONTENT_STEAM);
						m_delayed_node_changes[test_p] = testnode;
						if (!has_steam_sound) {
							addEnvEvent(ENV_EVENT_SOUND,intToFloat(p,BS),"env-steam");
							has_steam_sound = true;
						}
					}

					if (found == true && n.getContent() == CONTENT_LAVASOURCE) {
						int material = myrand()%50;
						switch(material) {
						case 0:
						case 1:
						case 2:
						case 3:
						case 4:
						case 5:
						case 6:
						case 7:
							n = MapNode(CONTENT_STONE, MINERAL_COAL);
							break;
						case 8:
						case 9:
						case 10:
						case 11:
							n = MapNode(CONTENT_STONE, MINERAL_IRON);
							break;
						case 12:
						case 13:
						case 14:
						case 15:
							n = MapNode(CONTENT_STONE, MINERAL_TIN);
							break;
						case 16:
						case 17:
						case 18:
						case 19:
							n = MapNode(CONTENT_STONE, MINERAL_QUARTZ);
							break;
						case 20:
						case 21:
						case 22:
						case 23:
							n = MapNode(CONTENT_STONE, MINERAL_COPPER);
							break;
						case 24:
							n = MapNode(CONTENT_STONE, MINERAL_SILVER);
							break;
						case 25:
							n = MapNode(CONTENT_STONE, MINERAL_GOLD);
							break;
						default:
							n.setContent(CONTENT_ROUGHSTONE);
							break;
						}
						m_map->addNodeWithEvent(p, n);
					}else if (found == true) {
						n.setContent(CONTENT_ROUGHSTONE);
						m_map->addNodeWithEvent(p, n);
					}

					break;
				}
				case CONTENT_AIR:
				{
					if (biome == BIOME_SPACE && !searchNear(p,v3s16(5,5,5),CONTENT_LIFE_SUPPORT,NULL)) {
						n.setContent(CONTENT_VACUUM);
						m_map->addNodeWithEvent(p,n);
					}
					break;
				}
				case CONTENT_VACUUM:
				{
					if (biome != BIOME_SPACE) {
						n.setContent(CONTENT_AIR);
						m_map->addNodeWithEvent(p,n);
					}
					break;
				}
				case CONTENT_LIFE_SUPPORT:
				{
					MapNode testnode;
					v3s16 testpos;
					for (s16 x=-5; x<5; x++)
					for (s16 y=-5; y<5; y++)
					for (s16 z=-5; z<5; z++) {
						testpos = p+v3s16(x,y,z);
						testnode = m_map->getNodeNoEx(testpos);
						if (testnode.getContent() != CONTENT_VACUUM)
							continue;
						testnode.setContent(CONTENT_AIR);
						m_map->addNodeWithEvent(testpos,testnode);
					}
					break;
				}
				}

				if (coldzone && biome != BIOME_BEACH
					&& (content_features(n).draw_type == CDT_CUBELIKE
						|| content_features(n).draw_type == CDT_GLASSLIKE
						|| (content_features(n).draw_type == CDT_DIRTLIKE
							&& (n.param1&0x20) != 0x20
							&& ((n.param1&0x0F) == 0x00
								|| (n.param1&0x0F) == 0x04))))
				{
					if (myrand()%20 == 0)
					{
						std::vector<content_t> search;
						search.push_back(CONTENT_AIR);
						// check that it's on top, and somewhere snow could fall
						// not 100% because torches
						if (!searchNearInv(p,v3s16(0,1,0),v3s16(0,16,0),search,NULL)
							&& !searchNear(p,v3s16(3,3,3),CONTENT_FIRE,NULL))
						{
							MapNode nn(CONTENT_SNOW);
							m_map->addNodeWithEvent(p+v3s16(0,1,0),nn);
							//n.param1 &= ~0x0F;
							//n.param1 |= 0x04;
							//n.envticks = 0;
							//m_map->addNodeWithEvent(p,n);
						}
					}
				}
			}

			for (std::map<v3s16,MapNode>::iterator it = m_delayed_node_changes.begin();
			     it != m_delayed_node_changes.end(); it++)
			{
				m_map->addNodeWithEvent(it->first, it->second);
			}
			m_delayed_node_changes.clear();

			if(block)
			    block->ResetCurrent();		
		}
	}

	/*
		Step active objects
	*/
	{
		ScopeProfiler sp(g_profiler, "SEnv: step act. objs avg", SPT_AVG);
		//TimeTaker timer("Step active objects");

		g_profiler->avg("SEnv: num of objects", m_active_objects.size());

		// This helps the objects to send data at the same time
		bool send_recommended = false;
		u8 mob_level = mobLevelI(config_get("world.game.mob.spawn.level"));
		m_send_recommended_timer += dtime;
		if (m_send_recommended_timer > 0.10) {
			m_send_recommended_timer = 0;
			send_recommended = true;
		}

		for (std::map<u16, ServerActiveObject*>::iterator i = m_active_objects.begin(); i != m_active_objects.end(); i++) {
			ServerActiveObject* obj = i->second;

			if(!obj)
			    continue;
			
			// Remove non-peaceful mobs on peaceful mode
			if (obj->level() > mob_level)
				obj->m_removed = true;
			// Don't step if is to be removed or stored statically
			if (obj->m_removed)
				continue;
			if (obj->m_pending_deactivation) {
				// The block in which the object resides in
				v3s16 blockpos = getNodeBlockPos(floatToInt(obj->getBasePosition(), BS));
				if (!m_active_blocks.contains(blockpos))
					continue;
				obj->m_pending_deactivation = false;
			}
			// Step object
			obj->step(dtime, send_recommended);
			// Read messages from object
			while (obj->m_messages_out.size() > 0) {
				m_active_object_messages.push_back(obj->m_messages_out.pop_front());
			}
		}
	}

	/*
		Manage active objects
	*/
	if (m_object_management_interval.step(dtime, 0.5)) {
		ScopeProfiler sp(g_profiler, "SEnv: remove removed objs avg /.5s", SPT_AVG);
		/*
			Remove objects that satisfy (m_removed && m_known_by_count==0)
		*/
		removeRemovedObjects();
	}
}

ServerActiveObject* ServerEnvironment::getActiveObject(u16 id)
{
	std::map<u16, ServerActiveObject*>::iterator i = m_active_objects.find(id);
	if (i == m_active_objects.end())
		return NULL;
	return i->second;
}

void ServerEnvironment::getActiveObjects(v3f origin, f32 max_d, core::array<DistanceSortedActiveObject> &dest)
{
	for (std::map<u16, ServerActiveObject*>::iterator i = m_active_objects.begin(); i != m_active_objects.end(); i++) {
		ServerActiveObject* obj = i->second;

		if(!obj)
		    continue;
		
		f32 d = (obj->getBasePosition() - origin).getLength();

		if (d > max_d)
		    continue;

		DistanceSortedActiveObject dso(obj, d);

		dest.push_back(dso);
	}
}

bool ServerEnvironment::propogateEnergy(u8 level, v3s16 powersrc, v3s16 signalsrc, v3s16 pos, core::map<v3s16,MapBlock*> &modified_blocks)
{
	MapNode n = m_map->getNodeNoEx(pos);
	NodeMetadata *m;
	if (n.getContent() == CONTENT_IGNORE)
		return false;
	ContentFeatures &f = content_features(n);
	if (f.energy_type == CET_NONE) {
		if (powersrc != signalsrc || (n.getContent() != CONTENT_STONE && n.getContent() != CONTENT_LIMESTONE))
			return false;
	}else{
		if ((f.energy_type == CET_SOURCE || f.energy_type == CET_SWITCH) && pos != powersrc)
			return false;
		if (f.energy_type == CET_GATE && pos == powersrc && level != ENERGY_MAX)
			return false;
		m = m_map->getNodeMetadata(pos);
		if (!m)
			return false;
		{
			MapNode n = m_map->getNodeNoEx(pos);
			content_t c = n.getContent();
			if (c >= CONTENT_DOOR_MIN && c <= CONTENT_DOOR_MAX) {
				v3s16 mp(0,1,0);
				if ((c&CONTENT_DOOR_SECT_MASK) == CONTENT_DOOR_SECT_MASK)
					mp.Y = -1;

				if (signalsrc != pos+mp)
					propogateEnergy(level,powersrc,pos,pos+mp, modified_blocks);
			}
		}
		if (!m->energise(level,powersrc,signalsrc,pos))
			return false;
		if (f.energy_type == CET_GATE)
			level = ENERGY_MAX;
		if (f.energy_type == CET_GATE && pos != powersrc)
			return false;
	}
	if (level) {
		if (f.powered_node != CONTENT_IGNORE) {
			n.setContent(f.powered_node);
			std::string st("");
			m_map->addNodeAndUpdate(pos, n, modified_blocks, st);
		}
	}
	if (f.energy_type == CET_DEVICE) {
	    // devices receive power, but don't propogate it further
	    return false;
	}

	if (f.energy_type != CET_SOURCE && f.energy_type != CET_SWITCH)
		level -= f.energy_drop;

	if (level < 1) {
	    return false;
	}

	MapNode n_plus_y = m_map->getNodeNoEx(pos + v3s16(0,1,0));
	MapNode n_minus_x = m_map->getNodeNoEx(pos + v3s16(-1,0,0));
	MapNode n_plus_x = m_map->getNodeNoEx(pos + v3s16(1,0,0));
	MapNode n_minus_z = m_map->getNodeNoEx(pos + v3s16(0,0,-1));
	MapNode n_plus_z = m_map->getNodeNoEx(pos + v3s16(0,0,1));
	MapNode n_minus_xy = m_map->getNodeNoEx(pos + v3s16(-1,1,0));
	MapNode n_plus_xy = m_map->getNodeNoEx(pos + v3s16(1,1,0));
	MapNode n_minus_zy = m_map->getNodeNoEx(pos + v3s16(0,1,-1));
	MapNode n_plus_zy = m_map->getNodeNoEx(pos + v3s16(0,1,1));
	MapNode n_minus_x_y = m_map->getNodeNoEx(pos + v3s16(-1,-1,0));
	MapNode n_plus_x_y = m_map->getNodeNoEx(pos + v3s16(1,-1,0));
	MapNode n_minus_z_y = m_map->getNodeNoEx(pos + v3s16(0,-1,-1));
	MapNode n_plus_z_y = m_map->getNodeNoEx(pos + v3s16(0,-1,1));
	bool x_plus = false;
	bool x_plus_y = false;
	bool x_plus_y_minus = false;
	bool x_minus = false;
	bool x_minus_y = false;
	bool x_minus_y_minus = false;
	bool z_plus = false;
	bool z_plus_y = false;
	bool z_plus_y_minus = false;
	bool z_minus = false;
	bool z_minus_y = false;
	bool z_minus_y_minus = false;
	bool y_plus = false;
	// +Y
	if (n_plus_y.getContent() == CONTENT_AIR || content_features(n_plus_y).energy_type != CET_NONE)
		y_plus = true;
	// +X
	if (
		content_features(n_plus_x).energy_type == CET_NONE
		&& content_features(n_plus_x).flammable != 2
	) {
		if (
			y_plus
			&& (
				content_features(n_plus_x).draw_type == CDT_CUBELIKE
				|| content_features(n_plus_x).draw_type == CDT_GLASSLIKE
			)
		) {
			if (content_features(n_plus_xy).energy_type != CET_NONE) {
				x_plus_y = true;
			}
		}else if (
			n_plus_x.getContent() == CONTENT_AIR
			&& content_features(n_plus_x_y).energy_type != CET_NONE
		) {
			x_plus_y_minus = true;
		}
	}else{
		x_plus = true;
	}
	// -X
	if (content_features(n_minus_x).energy_type == CET_NONE && content_features(n_minus_x).flammable != 2) {
		if (
			y_plus
			&& (
				content_features(n_minus_x).draw_type == CDT_CUBELIKE
				|| content_features(n_minus_x).draw_type == CDT_GLASSLIKE
			)
		) {
			if (content_features(n_minus_xy).energy_type != CET_NONE) {
				x_minus_y = true;
			}
		}else if (
			n_minus_x.getContent() == CONTENT_AIR
			&& content_features(n_minus_x_y).energy_type != CET_NONE
		) {
			x_minus_y_minus = true;
		}
	}else{
		x_minus = true;
	}
	// +Z
	if (
		content_features(n_plus_z).energy_type == CET_NONE
		&& content_features(n_plus_z).flammable != 2
	) {
		if (
			y_plus
			&& (
				content_features(n_plus_z).draw_type == CDT_CUBELIKE
				|| content_features(n_plus_z).draw_type == CDT_GLASSLIKE
			)
		) {
			if (content_features(n_plus_zy).energy_type != CET_NONE) {
				z_plus_y = true;
			}
		}else if (
			n_plus_z.getContent() == CONTENT_AIR
			&& content_features(n_plus_z_y).energy_type != CET_NONE
		) {
			z_plus_y_minus = true;
		}
	}else{
		z_plus = true;
	}
	// -Z
	if (
		content_features(n_minus_z).energy_type == CET_NONE
		&& content_features(n_minus_z).flammable != 2
	) {
		if (
			y_plus
			&& (
				content_features(n_minus_z).draw_type == CDT_CUBELIKE
				|| content_features(n_minus_z).draw_type == CDT_GLASSLIKE
			)
		) {
			if (content_features(n_minus_zy).energy_type != CET_NONE) {
				z_minus_y = true;
			}
		}else if (
			n_minus_z.getContent() == CONTENT_AIR
			&& content_features(n_minus_z_y).energy_type != CET_NONE
		) {
			z_minus_y_minus = true;
		}
	}else{
		z_minus = true;
	}

	bool gate_x_plus = true;
	bool gate_x_minus = true;
	bool gate_z_plus = true;
	bool gate_z_minus = true;
	if (f.energy_type == CET_GATE) {
		gate_x_plus = false;
		gate_x_minus = false;
		gate_z_plus = false;
		gate_z_minus = false;
		powersrc = pos;
		v3s16 dir = n.getRotation();
		if (dir == v3s16(1,1,1)) {
			gate_z_plus = true;
		}else if (dir == v3s16(-1,1,1)) {
			gate_x_plus = true;
		}else if (dir == v3s16(-1,1,-1)) {
			gate_z_minus = true;
		}else if (dir == v3s16(1,1,-1)) {
			gate_x_minus = true;
		}
	}

	if (gate_x_plus) {
		if (x_plus) {
			if ((pos+v3s16(1,0,0)) != signalsrc)
				propogateEnergy(level,powersrc,pos,pos+v3s16(1,0,0), modified_blocks);
		}else if (x_plus_y) {
			if ((pos+v3s16(1,1,0)) != signalsrc)
				propogateEnergy(level,powersrc,pos,pos+v3s16(1,1,0), modified_blocks);
		}else if (x_plus_y_minus) {
			if ((pos+v3s16(1,-1,0)) != signalsrc)
				propogateEnergy(level,powersrc,pos,pos+v3s16(1,-1,0), modified_blocks);
		}else if (powersrc == pos) {
			if ((pos+v3s16(1,0,0)) != signalsrc && (n_plus_x.getContent() == CONTENT_STONE || n_plus_x.getContent() == CONTENT_LIMESTONE))
				propogateEnergy(level,powersrc,pos,pos+v3s16(1,0,0), modified_blocks);
			if ((pos+v3s16(1,1,0)) != signalsrc && (n_plus_xy.getContent() == CONTENT_STONE || n_plus_xy.getContent() == CONTENT_LIMESTONE))
				propogateEnergy(level,powersrc,pos,pos+v3s16(1,1,0), modified_blocks);
			if ((pos+v3s16(1,-1,0)) != signalsrc && (n_plus_x_y.getContent() == CONTENT_STONE || n_plus_x_y.getContent() == CONTENT_LIMESTONE))
				propogateEnergy(level,powersrc,pos,pos+v3s16(1,-1,0), modified_blocks);
		}
	}
	if (gate_x_minus) {
		if (x_minus) {
			if ((pos+v3s16(-1,0,0)) != signalsrc)
				propogateEnergy(level,powersrc,pos,pos+v3s16(-1,0,0), modified_blocks);
		}else if (x_minus_y) {
			if ((pos+v3s16(-1,1,0)) != signalsrc)
				propogateEnergy(level,powersrc,pos,pos+v3s16(-1,1,0), modified_blocks);
		}else if (x_minus_y_minus) {
			if ((pos+v3s16(-1,-1,0)) != signalsrc)
				propogateEnergy(level,powersrc,pos,pos+v3s16(-1,-1,0), modified_blocks);
		}else if (powersrc == pos) {
			if ((pos+v3s16(-1,0,0)) != signalsrc && (n_minus_x.getContent() == CONTENT_STONE || n_minus_x.getContent() == CONTENT_LIMESTONE))
				propogateEnergy(level,powersrc,pos,pos+v3s16(-1,0,0), modified_blocks);
			if ((pos+v3s16(-1,1,0)) != signalsrc && (n_minus_xy.getContent() == CONTENT_STONE || n_minus_xy.getContent() == CONTENT_LIMESTONE))
				propogateEnergy(level,powersrc,pos,pos+v3s16(-1,1,0), modified_blocks);
			if ((pos+v3s16(-1,-1,0)) != signalsrc && (n_minus_x_y.getContent() == CONTENT_STONE || n_minus_x_y.getContent() == CONTENT_LIMESTONE))
				propogateEnergy(level,powersrc,pos,pos+v3s16(-1,-1,0), modified_blocks);
		}
	}
	if (gate_z_plus) {
		if (z_plus) {
			if ((pos+v3s16(0,0,1)) != signalsrc)
				propogateEnergy(level,powersrc,pos,pos+v3s16(0,0,1), modified_blocks);
		}else if (z_plus_y) {
			if ((pos+v3s16(0,1,1)) != signalsrc)
				propogateEnergy(level,powersrc,pos,pos+v3s16(0,1,1), modified_blocks);
		}else if (z_plus_y_minus) {
			if ((pos+v3s16(0,-1,1)) != signalsrc)
				propogateEnergy(level,powersrc,pos,pos+v3s16(0,-1,1), modified_blocks);
		}else if (powersrc == pos) {
			if ((pos+v3s16(0,0,1)) != signalsrc && (n_plus_z.getContent() == CONTENT_STONE || n_plus_z.getContent() == CONTENT_LIMESTONE))
				propogateEnergy(level,powersrc,pos,pos+v3s16(0,0,1), modified_blocks);
			if ((pos+v3s16(0,1,1)) != signalsrc && (n_plus_zy.getContent() == CONTENT_STONE || n_plus_zy.getContent() == CONTENT_LIMESTONE))
				propogateEnergy(level,powersrc,pos,pos+v3s16(0,1,1), modified_blocks);
			if ((pos+v3s16(0,-1,1)) != signalsrc && (n_plus_z_y.getContent() == CONTENT_STONE || n_plus_z_y.getContent() == CONTENT_LIMESTONE))
				propogateEnergy(level,powersrc,pos,pos+v3s16(0,-1,1), modified_blocks);
		}
	}
	if (gate_z_minus) {
		if (z_minus) {
			if ((pos+v3s16(0,0,-1)) != signalsrc)
				propogateEnergy(level,powersrc,pos,pos+v3s16(0,0,-1), modified_blocks);
		}else if (z_minus_y) {
			if ((pos+v3s16(0,1,-1)) != signalsrc)
				propogateEnergy(level,powersrc,pos,pos+v3s16(0,1,-1), modified_blocks);
		}else if (z_minus_y_minus) {
			if ((pos+v3s16(0,-1,-1)) != signalsrc)
				propogateEnergy(level,powersrc,pos,pos+v3s16(0,-1,-1), modified_blocks);
		}else if (powersrc == pos) {
			if ((pos+v3s16(0,0,-1)) != signalsrc && (n_minus_z.getContent() == CONTENT_STONE || n_minus_z.getContent() == CONTENT_LIMESTONE))
				propogateEnergy(level,powersrc,pos,pos+v3s16(0,0,-1), modified_blocks);
			if ((pos+v3s16(0,1,-1)) != signalsrc && (n_minus_zy.getContent() == CONTENT_STONE || n_minus_zy.getContent() == CONTENT_LIMESTONE))
				propogateEnergy(level,powersrc,pos,pos+v3s16(0,1,-1), modified_blocks);
			if ((pos+v3s16(0,-1,-1)) != signalsrc && (n_minus_z_y.getContent() == CONTENT_STONE || n_minus_z_y.getContent() == CONTENT_LIMESTONE))
				propogateEnergy(level,powersrc,pos,pos+v3s16(0,-1,-1), modified_blocks);
		}
	}
	return false;
}

bool isFreeServerActiveObjectId(u16 id, std::map<u16, ServerActiveObject*> &objects)
{
	if (id == 0)
		return false;

	std::map<u16, ServerActiveObject*>::iterator i = objects.find(id);
	if (i == objects.end())
		return true;
	return false;
}

u16 getFreeServerActiveObjectId(std::map<u16, ServerActiveObject*> &objects)
{
	//try to reuse id's as late as possible
	static u16 last_used_id = 0;
	u16 startid = last_used_id;

	for (;;) {
		last_used_id++;
		if (isFreeServerActiveObjectId(last_used_id, objects))
			return last_used_id;

		if (last_used_id == startid)
			return 0;
	}

	return 0;
}

u16 ServerEnvironment::addActiveObject(ServerActiveObject *object)
{
	assert(object);
	u16 id = addActiveObjectRaw(object, true);
	return id;
}

/*
	Finds out what new objects have been added to
	inside a radius around a position
*/
void ServerEnvironment::getAddedActiveObjects(v3s16 pos, s16 radius,
		std::map<u16, bool> &current_objects,
		std::map<u16, bool> &added_objects)
{
	v3f pos_f = intToFloat(pos, BS);
	f32 radius_f = radius * BS;
	/*
		Go through the object list,
		- discard m_removed objects,
		- discard objects that are too far away,
		- discard objects that are found in current_objects.
		- add remaining objects to added_objects
	*/
	for (std::map<u16, ServerActiveObject*>::iterator i = m_active_objects.begin(); i != m_active_objects.end(); i++) {
		u16 id = i->first;
		// Get object
		ServerActiveObject *object = i->second;
		if (object == NULL)
			continue;
		// Discard if removed
		if (object->m_removed || object->m_pending_deactivation)
			continue;
		// Discard if too far
		f32 distance_f = object->getBasePosition().getDistanceFrom(pos_f);
		if (distance_f > radius_f)
			continue;
		// Discard if already on current_objects
		std::map<u16, bool>::iterator c = current_objects.find(id);
		if (c != current_objects.end())
			continue;
		// Add to added_objects
		added_objects[id] = true;
	}
}

/*
	Finds out what objects have been removed from
	inside a radius around a position
*/
void ServerEnvironment::getRemovedActiveObjects(v3s16 pos, s16 radius,
		std::map<u16, bool> &current_objects,
		std::map<u16, bool> &removed_objects)
{
	v3f pos_f = intToFloat(pos, BS);
	// bigger radius for removing than adding
	// this gives us a buffer to prevent anything getting lost
	f32 radius_f = radius * BS * 2.0;
	/*
		Go through current_objects; object is removed if:
		- object is not found in m_active_objects (this is actually an
		  error condition; objects should be set m_removed=true and removed
		  only after all clients have been informed about removal), or
		- object has m_removed=true, or
		- object has m_pending_deactivation=true, or
		- object is too far away
	*/
	for (std::map<u16, bool>::iterator i = current_objects.begin(); i != current_objects.end(); i++) {
		u16 id = i->first;
		ServerActiveObject *object = getActiveObject(id);
		if (object == NULL) {
			infostream<<"ServerEnvironment::getRemovedActiveObjects():"
					<<" object in current_objects is NULL"<<std::endl;
		}else if (object->m_removed == false && object->m_pending_deactivation == false) {
			f32 distance_f = object->getBasePosition().getDistanceFrom(pos_f);
			if (distance_f < radius_f) {
				// Not removed
				continue;
			}
		}
		removed_objects[id] = true;
	}
}

ActiveObjectMessage ServerEnvironment::getActiveObjectMessage()
{
	if (m_active_object_messages.size() == 0)
		return ActiveObjectMessage(0);

	return m_active_object_messages.pop_front();
}


void ServerEnvironment::addEnvEvent(u8 type, v3f pos, std::string data)
{
	EnvEvent ev(type,pos,data);
	m_env_events.push_back(ev);
}

EnvEvent ServerEnvironment::getEnvEvent()
{
	if (m_env_events.size() == 0)
		return EnvEvent();

	return m_env_events.pop_front();
}

/*
	************ Private methods *************
*/

u16 ServerEnvironment::addActiveObjectRaw(ServerActiveObject *object, bool set_changed)
{
	assert(object);
	if (object->getId() == 0) {
		u16 new_id = getFreeServerActiveObjectId(m_active_objects);
		if (new_id == 0) {
			errorstream<<"ServerEnvironment::addActiveObjectRaw(): "
					<<"no free ids available"<<std::endl;
			delete object;
			return 0;
		}
		object->setId(new_id);
	}else{
		verbosestream<<"ServerEnvironment::addActiveObjectRaw(): "
				<<"supplied with id "<<object->getId()<<std::endl;
	}
	if (isFreeServerActiveObjectId(object->getId(), m_active_objects) == false) {
		errorstream<<"ServerEnvironment::addActiveObjectRaw(): "
				<<"id is not free ("<<object->getId()<<")"<<std::endl;
		delete object;
		return 0;
	}

	m_active_objects[object->getId()] = object;

	verbosestream<<"ServerEnvironment::addActiveObjectRaw(): "
			<<"Added id="<<object->getId()<<"; there are now "
			<<m_active_objects.size()<<" active objects."
			<<std::endl;

	// Add to the block where the object is located in
	v3s16 blockpos = getNodeBlockPos(floatToInt(object->getBasePosition(), BS));
	MapBlock* const block = m_map->getBlockNoCreateNoEx(blockpos);
	if (block)
	{
		block->m_active_objects.push_back(object->getId());
		object->m_static_exists = true;
		object->m_static_block = blockpos;

		if (set_changed)
			block->raiseModified(MOD_STATE_WRITE_NEEDED);
		block->ResetCurrent();
	}
	else
	{
		errorstream<<"ServerEnvironment::addActiveObjectRaw(): "
				<<"could not find block for storing id="<<object->getId()
				<<" statically"<<std::endl;
	}

	return object->getId();
}

/*
	Remove objects that satisfy (m_removed && m_known_by_count==0)
*/
void ServerEnvironment::removeRemovedObjects()
{
	std::list<u16> objects_to_remove;
	for (std::map<u16, ServerActiveObject*>::iterator i = m_active_objects.begin(); i != m_active_objects.end(); ++i) {
		u16 id = i->first;
		ServerActiveObject* obj = i->second;
		// this shouldn't happen but check it
		if (obj == NULL) {
			infostream<<"NULL object found in ServerEnvironment"
					<<" while finding removed objects. id="<<id<<std::endl;
			// id to be removed from m_active_objects
			objects_to_remove.push_back(id);
			continue;
		}

		/*
			We will delete objects that are marked as removed
			and are not known by clients
		*/
		if (obj->m_removed == false)
		{
			// enforce static data
			if (obj->m_static_exists)
			{
				MapBlock* const block = m_map->emergeBlock(obj->m_static_block, false);
				if (block)
				{
					bool keep = false;
					for (std::list<u16>::iterator oi = block->m_active_objects.begin();
					     oi != block->m_active_objects.end(); oi++) {
						if (*oi == id)
						{
							keep = true;
							break;
						}
					}
					obj->m_static_exists = keep;
					block->ResetCurrent();
				}
			}
			if (!obj->m_static_exists)
			{
				obj->m_static_block = getNodeBlockPos(floatToInt(obj->getBasePosition(), BS));
				MapBlock* const block = m_map->emergeBlock(obj->m_static_block, false);
				if (block)
				{
					block->m_active_objects.push_back(id);
					obj->m_static_exists = true;
					block->ResetCurrent();
				}
			}
			continue;
		}
		if (obj->m_known_by_count > 0) {
			obj->m_pending_deactivation = true;
			continue;
		}

		/*
			Delete static data from block if is marked as removed
		*/
		if (obj->m_static_exists)
		{
			MapBlock* const block = m_map->emergeBlock(obj->m_static_block, false);
			if (block)
			{
				obj->m_static_exists = false;
				block->m_active_objects.remove(id);
				block->ResetCurrent();
			}
		}

		// Delete
		delete obj;
		// Id to be removed from m_active_objects
		objects_to_remove.push_back(id);
	}
	// Remove references from m_active_objects
	for (std::list<u16>::iterator i = objects_to_remove.begin(); i != objects_to_remove.end(); ++i) {
		m_active_objects.erase(*i);
	}
}

static void print_hexdump(std::ostream &o, const std::string &data)
{
	const int linelength = 16;
	for(int l=0; ; l++){
		int i0 = linelength * l;
		bool at_end = false;
		int thislinelength = linelength;
		if(i0 + thislinelength > (int)data.size()){
			thislinelength = data.size() - i0;
			at_end = true;
		}
		for(int di=0; di<linelength; di++){
			int i = i0 + di;
			char buf[4];
			if(di<thislinelength)
				snprintf(buf, 4, "%.2x ", data[i]);
			else
				snprintf(buf, 4, "   ");
			o<<buf;
		}
		o<<" ";
		for(int di=0; di<thislinelength; di++){
			int i = i0 + di;
			if(data[i] >= 32)
				o<<data[i];
			else
				o<<".";
		}
		o<<std::endl;
		if(at_end)
			break;
	}
}

/*
	Convert stored objects from blocks near the players to active.
*/
void ServerEnvironment::activateObjects(MapBlock *block)
{
	if (block==NULL)
		return;
	// Ignore if no stored objects (to not set changed flag)
	if (block->m_static_objects.m_objects.size() == 0)
		return;

	verbosestream<<"ServerEnvironment::activateObjects(): "
			<<"activating objects of block "<<PP(block->getPos())
			<<" ("<<block->m_static_objects.m_objects.size()
			<<" objects)"<<std::endl;
	if (block->m_static_objects.m_objects.size() > 49) {
		errorstream<<"suspiciously large amount of objects detected: "
				<<block->m_static_objects.m_objects.size()<<" in "
				<<PP(block->getPos())
				<<"; removing all of them."<<std::endl;
		// Clear stored list
		block->m_static_objects.m_objects.clear();
		block->raiseModified(MOD_STATE_WRITE_NEEDED);
		return;
	}

	// Activate stored objects
	std::list<StaticObject> new_stored;
	for (std::list<StaticObject>::iterator i = block->m_static_objects.m_objects.begin(); i != block->m_static_objects.m_objects.end(); i++) {
		StaticObject &s_obj = *i;
		// Create an active object from the data
		ServerActiveObject *obj = ServerActiveObject::create(s_obj.type, this, 0, s_obj.pos, s_obj.data);
		// If couldn't create object, store static data back.
		if (obj==NULL) {
			errorstream<<"ServerEnvironment::activateObjects(): "
					<<"failed to create active object from static object "
					<<"in block "<<PP(s_obj.pos/BS)
					<<" type="<<(int)s_obj.type<<" data:"<<std::endl;
			print_hexdump(verbosestream, s_obj.data);

			new_stored.push_back(s_obj);
			continue;
		}
		verbosestream<<"ServerEnvironment::activateObjects(): "
				<<"activated static object pos="<<PP(s_obj.pos/BS)
				<<" type="<<(int)s_obj.type<<std::endl;
		// This will also add the object to the active static list
		addActiveObjectRaw(obj, false);
	}
	// Clear stored list
	// Add leftover failed stuff to stored list
	block->m_static_objects.m_objects.swap(new_stored);
	new_stored.clear();

	// objects that were pending deactivation, shouldn't be anymore
	for (std::list<u16>::iterator i = block->m_active_objects.begin(); i != block->m_active_objects.end(); i++) {
		ServerActiveObject *object = getActiveObject(*i);
		if (object)
			object->m_pending_deactivation = false;
	}

	/*
		Note: Block hasn't really been modified here.
		The objects have just been activated and moved from the stored
		static list to the active static list.
		As such, the block is essentially the same.
		Thus, do not call block->raiseModified(MOD_STATE_WRITE_NEEDED).
		Otherwise there would be a huge amount of unnecessary I/O.
		P.S. it's been tried, it doesn't fix anything
	*/
	block->raiseModified(MOD_STATE_WRITE_NEEDED);
}

/*
	Convert objects that are not standing inside active blocks to static.

	If m_known_by_count != 0, active object is not deleted, but static
	data is still updated.

	If force_delete is set, active object is deleted nevertheless. It
	shall only be set so in the destructor of the environment.
*/
void ServerEnvironment::deactivateFarObjects(bool force_delete)
{
	//ScopeProfiler sp(g_profiler, "SEnv: deactivateFarObjects");

	std::vector<u16> objects_to_remove;
	for (std::map<u16, ServerActiveObject*>::iterator i = m_active_objects.begin(); i != m_active_objects.end(); i++) {
		ServerActiveObject* obj = i->second;
		if (!obj)
			continue;

		// If pending deactivation or removed, let removeRemovedObjects() do it
		if (!force_delete && (obj->m_pending_deactivation || obj->m_removed))
			continue;

		u16 id = i->first;
		v3f objectpos = obj->getBasePosition();

		// The block in which the object resides in
		v3s16 blockpos_o = getNodeBlockPos(floatToInt(objectpos, BS));

		if (force_delete)
		{
			// if it isn't m_removed, then ensure it's stored
			if (!obj->m_removed)
			{
				if (!obj->m_static_exists)
					obj->m_static_block = blockpos_o;

				MapBlock* const block = m_map->emergeBlock(obj->m_static_block, false);
				if (block)
				{
					// create new static object
					std::string staticdata_new = obj->getStaticData();
					StaticObject s_obj(obj->getType(), objectpos, staticdata_new);
					block->m_static_objects.m_objects.push_back(s_obj);
					block->raiseModified(MOD_STATE_WRITE_NEEDED);
					block->ResetCurrent();
				}
			}
			// delete active object
			delete obj;
			// id to be removed from m_active_objects
			objects_to_remove.push_back(id);
			continue;
		}

		// if object is stored in a different block to where it's located, remove it
		if (obj->m_static_exists && obj->m_static_block != blockpos_o)
		{
			MapBlock* const block = m_map->emergeBlock(obj->m_static_block, false);
			if (block)
			{
				obj->m_static_exists = false;
				block->m_active_objects.remove(id);
				block->ResetCurrent();
			}
		}

		if (!obj->m_static_exists)
		{
			obj->m_static_block = blockpos_o;

			// Save to block where object is located
			MapBlock* const block = m_map->emergeBlock(blockpos_o, false);
			if (!block)
			{
				obj->m_removed = true;
				continue;
			}

			block->m_active_objects.push_back(id);
			obj->m_static_exists = true;
			block->ResetCurrent();
		}

		// If block is active, don't remove
		if (!force_delete && m_active_blocks.contains(blockpos_o))
			continue;

		if (obj->m_known_by_count > 0) {
			obj->m_pending_deactivation = true;
			continue;
		}

		/*
			Update the static data
		*/

		// Create new static object
		std::string staticdata_new = obj->getStaticData();
		StaticObject s_obj(obj->getType(), objectpos, staticdata_new);

		// Add to the block where the object is located in
		v3s16 blockpos = getNodeBlockPos(floatToInt(objectpos, BS));
		// Get or generate the block
		MapBlock *block = NULL;
		bool was_generated = false;
		try
		{
			block = m_map->emergeBlock(blockpos,true,&was_generated);
		}
		catch(InvalidPositionException &e)
		{
			// Handled via NULL pointer
			// NOTE: emergeBlock's failure is usually determined by it
			//       actually returning NULL
		}

		if (block)
		{
			if (block->m_static_objects.m_objects.size() >= 50)
			{
				errorstream<<"ServerEnv: Trying to store id="<<obj->getId()
						<<" statically but block "<<PP(blockpos)
						<<" already contains "
						<<block->m_static_objects.m_objects.size()
						<<" objects."
						<<" Forcing delete."<<std::endl;
				force_delete = true;
			}
			else
			{
				// Store static data
				block->m_static_objects.m_objects.push_back(s_obj);
				block->raiseModified(MOD_STATE_WRITE_NEEDED);
			}
			block->ResetCurrent();
		}
		else
		{
			if (!force_delete) {
				v3s16 p = floatToInt(objectpos, BS);
				errorstream<<"ServerEnv: Could not find or generate "
						<<"a block for storing id="<<obj->getId()
						<<" statically (pos="<<PP(p)<<")"<<std::endl;
				continue;
			}
		}


		// Delete active object
		delete obj;
		// Id to be removed from m_active_objects
		objects_to_remove.push_back(id);
	}

	// Remove references from m_active_objects
	for (std::vector<u16>::iterator i = objects_to_remove.begin(); i != objects_to_remove.end(); ++i) {
		m_active_objects.erase(*i);
	}
}


#ifndef SERVER

/*
	ClientEnvironment
*/

ClientEnvironment::ClientEnvironment(Client *client, ClientMap *map, scene::ISceneManager *smgr):
	m_client(client),
	m_map(map),
	m_local_player(NULL),
	m_smgr(smgr)
{
	assert(m_map);
	assert(m_smgr);
}

ClientEnvironment::~ClientEnvironment()
{
	// delete active objects
	for (std::map<u16, ClientActiveObject*>::iterator i = m_active_objects.begin(); i != m_active_objects.end(); i++) {
		delete i->second;
	}

	// Drop/delete map
	m_map->drop();
}

void ClientEnvironment::addPlayer(Player *player)
{
	DSTACK(__FUNCTION_NAME);
	/*
		It is a failure if player is local and there already is a local
		player
	*/
	if (player->isLocal()) {
		assert(m_local_player == NULL);
		m_local_player = (LocalPlayer*)player;
	}

	Environment::addPlayer(player);
}

LocalPlayer * ClientEnvironment::getLocalPlayer()
{
	return m_local_player;
}

void ClientEnvironment::step(float dtime)
{
	DSTACK(__FUNCTION_NAME);

	stepTimeOfDay(dtime);

	// Get some settings
	bool footprints = config_get_bool("world.game.environment.footprints");

	// Get local player
	LocalPlayer *lplayer = getLocalPlayer();
	assert(lplayer);
	// collision info queue
	core::list<CollisionInfo> player_collisions;

	/*
		Get the speed the player is going
	*/
	bool is_climbing = lplayer->is_climbing;

	f32 player_speed = lplayer->getSpeed().getLength();

	/*
		Maximum position increment
	*/
	//f32 position_max_increment = 0.05*BS;
	f32 position_max_increment = 0.1*BS;

	// Maximum time increment (for collision detection etc)
	// time = distance / speed
	f32 dtime_max_increment = 1;
	if (player_speed > 0.001)
		dtime_max_increment = position_max_increment / player_speed;

	// Maximum time increment is 10ms or lower
	if (dtime_max_increment > 0.01)
		dtime_max_increment = 0.01;

	// Don't allow overly huge dtime
	if (dtime > 0.5)
		dtime = 0.5;

	f32 dtime_downcount = dtime;

	/*
		Stuff that has a maximum time increment
	*/

	u32 loopcount = 0;
	do {
		loopcount++;

		f32 dtime_part;
		if (dtime_downcount > dtime_max_increment) {
			dtime_part = dtime_max_increment;
			dtime_downcount -= dtime_part;
		}else{
			dtime_part = dtime_downcount;
			/*
				Setting this to 0 (no -=dtime_part) disables an infinite loop
				when dtime_part is so small that dtime_downcount -= dtime_part
				does nothing
			*/
			dtime_downcount = 0;
		}

		/*
			Handle local player
		*/

		{
			// Apply physics
			if (lplayer->control.free == false && is_climbing == false) {
				bool space = (lplayer->getPosition().Y > 1000*BS);
				// Gravity
				v3f speed = lplayer->getSpeed();
				if (lplayer->swimming_up == false) {
					float pull = 9.81;
					if (space)
						pull = 1.0;
					speed.Y -= (pull*BS)*(dtime_part*2);
				}

				// Water resistance
				if (lplayer->in_water_stable || lplayer->in_water) {
					f32 max_down = 2.0*BS;
					if (!space && speed.Y < -max_down)
						speed.Y = -max_down;

					f32 max = 2.5*BS;
					if (lplayer->control.fast && lplayer->getEnergy() > 0.0)
						max *= 1.5;
					if (speed.getLength() > max)
						speed = speed / speed.getLength() * max;
				}

				lplayer->setSpeed(speed);
			}

			/*
				Move the lplayer.
				This also does collision detection.
			*/
			lplayer->move(dtime_part, *m_map, position_max_increment,
					&player_collisions);
		}
	}
	while(dtime_downcount > 0.001);

	//std::cout<<"Looped "<<loopcount<<" times."<<std::endl;

	for (core::list<CollisionInfo>::Iterator i = player_collisions.begin(); i != player_collisions.end(); i++) {
		CollisionInfo &info = *i;
		if (info.t == COLLISION_FALL) {
			//f32 tolerance = BS*10; // 2 without damage
			f32 tolerance = BS*12; // 3 without damage
			f32 factor = 5;
			if (info.speed > BS*4)
				m_client->playStepSound(0);
			if (info.speed > tolerance) {
				f32 damage_f = (info.speed - tolerance)/BS*factor;
				u16 damage = (u16)(damage_f+0.5);
				lplayer->addDamage(PLAYER_FEET|PLAYER_LLEG|PLAYER_RLEG,DAMAGE_FALL,damage);

				ClientEnvEvent event;
				event.type = CEE_PLAYER_DAMAGE;
				event.player_damage.amount = damage;
				m_client_event_queue.push_back(event);
			}
		}
	}

	if (lplayer->health > 0)
	{
		/* player damage */
		if (m_damage_interval.step(dtime, 1.0))
		{
			v3f pf = lplayer->getPosition();
			v3s16 pp = floatToInt(pf, BS);

			uint32_t time = getTimeOfDay();
			uint16_t season = getSeason();
			uint8_t biome = BIOME_UNKNOWN;
			MapBlock* const block = m_map->getBlockNoCreateNoEx(getNodeBlockPos(pp));
			if (block)
			{
				biome = block->getBiome();
				block->ResetCurrent();
			}
			
			v3f ps[7] = {
				v3f(0, BS*-0.1, 0),
				v3f(0, BS*0.1, 0),
				v3f(0, BS*0.1, 0),
				v3f(0, BS*0.8, 0),
				v3f(BS*0.4, BS*0.8, 0),
				v3f(BS*-0.4, BS*0.8, 0),
				v3f(0, BS*1.6, 0)
			};

			u8 area[7] = {
				PLAYER_FEET,
				PLAYER_FEET,
				(PLAYER_LLEG|PLAYER_RLEG),
				PLAYER_TORSO,
				PLAYER_RARM,
				PLAYER_LARM,
				PLAYER_HEAD
			};
			ps[3].rotateXZBy(lplayer->getYaw());
			ps[4].rotateXZBy(lplayer->getYaw());

			for (int i=0; i<7; i++) {
				v3s16 p = floatToInt(pf+ps[i],BS);
				MapNode n = m_map->getNodeNoEx(p);
				u32 damage = content_features(n).damage_per_second;
				u32 suffocation = content_features(n).suffocation_per_second;
				u32 warmth = content_features(n).warmth_per_second;
				u32 pressure = content_features(n).pressure_per_second;
				if (damage > 0) {
					u8 t = DAMAGE_UNKNOWN;
					switch (n.getContent()) {
					case CONTENT_LAVA:
					case CONTENT_LAVASOURCE:
						t = DAMAGE_LAVA;
						break;
					case CONTENT_CACTUS:
						t = DAMAGE_CACTUS;
						break;
					case CONTENT_FIRE:
					case CONTENT_FIRE_SHORTTERM:
					case CONTENT_CAMPFIRE:
						t = DAMAGE_FIRE;
						break;
					case CONTENT_TNT:
					case CONTENT_FLASH:
						t = DAMAGE_TNT;
						break;
					case CONTENT_STEAM:
						t = DAMAGE_STEAM;
						break;
					default:;
					}
					damageLocalPlayer(area[i],t,damage);
				}
				if (warmth > 0)
					damageLocalPlayer(area[i],DAMAGE_COLD,warmth);
				if (pressure > 0)
					damageLocalPlayer(area[i],DAMAGE_SPACE,pressure);
				if (i < 6)
					continue;
				if (suffocation > 0) {
					damageLocalPlayer(area[i],DAMAGE_AIR,suffocation);
				}else if (lplayer->air < 100) {
					ClientEnvEvent event;
					event.type = CEE_PLAYER_SUFFOCATE;
					event.player_damage.amount = -100;
					m_client_event_queue.push_back(event);
				}
			}

			switch (biome) {
			case BIOME_JUNGLE:
			if (season == ENV_SEASON_WINTER && time > 4000 && time < 8000) {
				std::vector<content_t> search;
				search.push_back(CONTENT_FIRE);
				search.push_back(CONTENT_CAMPFIRE);
				if (!searchNear(pp,v3s16(-4,-2,-4),v3s16(5,5,5),search,NULL))
					damageLocalPlayer(PLAYER_ALL,DAMAGE_COLD,5);
			}
				break;
			case BIOME_OCEAN:
			if (season == ENV_SEASON_WINTER) {
				std::vector<content_t> search;
				search.push_back(CONTENT_FIRE);
				search.push_back(CONTENT_CAMPFIRE);
				if (!searchNear(pp,v3s16(-4,-2,-4),v3s16(5,5,5),search,NULL))
					damageLocalPlayer(PLAYER_ALL,DAMAGE_COLD,5);
			}
				break;
			case BIOME_DESERT:
				break;
			case BIOME_PLAINS:
			if (season == ENV_SEASON_WINTER && (time < 6000 || time > 18000)) {
				std::vector<content_t> search;
				search.push_back(CONTENT_FIRE);
				search.push_back(CONTENT_CAMPFIRE);
				if (!searchNear(pp,v3s16(-4,-2,-4),v3s16(5,5,5),search,NULL))
					damageLocalPlayer(PLAYER_ALL,DAMAGE_COLD,5);
			}
				break;
			case BIOME_FOREST:
			if (season == ENV_SEASON_WINTER) {
				std::vector<content_t> search;
				search.push_back(CONTENT_FIRE);
				search.push_back(CONTENT_CAMPFIRE);
				if (!searchNear(pp,v3s16(-4,-2,-4),v3s16(5,5,5),search,NULL))
					damageLocalPlayer(PLAYER_ALL,DAMAGE_COLD,5);
			}
				break;
			case BIOME_SKY:
			case BIOME_SNOWCAP:
			{
				std::vector<content_t> search;
				search.push_back(CONTENT_FIRE);
				search.push_back(CONTENT_CAMPFIRE);
				if (!searchNear(pp,v3s16(-4,-2,-4),v3s16(5,5,5),search,NULL))
					damageLocalPlayer(PLAYER_ALL,DAMAGE_COLD,5);
			}
				break;
			case BIOME_LAKE:
			if (
				season == ENV_SEASON_WINTER
				|| (
					season == ENV_SEASON_AUTUMN
					&& (time < 6000 || time > 18000)
				) || (
					season == ENV_SEASON_SPRING
					&& time > 4000
					&& time < 8000
				)
			) {
				std::vector<content_t> search;
				search.push_back(CONTENT_FIRE);
				search.push_back(CONTENT_CAMPFIRE);
				if (!searchNear(pp,v3s16(-4,-2,-4),v3s16(5,5,5),search,NULL))
					damageLocalPlayer(PLAYER_ALL,DAMAGE_COLD,5);
			}
				break;
			case BIOME_BEACH:
			if (season == ENV_SEASON_WINTER) {
				std::vector<content_t> search;
				search.push_back(CONTENT_FIRE);
				search.push_back(CONTENT_CAMPFIRE);
				if (!searchNear(pp,v3s16(-4,-2,-4),v3s16(5,5,5),search,NULL))
					damageLocalPlayer(PLAYER_ALL,DAMAGE_COLD,5);
			}
				break;
			case BIOME_SPACE:
				break;
			case BIOME_THEDEEP:
			if (lplayer->shadow_timer >= 5.0) {
				/* this means it takes 5 seconds to start taking damage, then damage every second after that */
				lplayer->shadow_timer -= 2.0;
				damageLocalPlayer(PLAYER_ALL,DAMAGE_SHADOW,5);
			}
			if (lplayer->light < 8) {
				lplayer->shadow_timer += 1.0;
			}else{
				lplayer->shadow_timer = 0.0;
			}
				break;
			case BIOME_WASTELANDS:
			{
				std::vector<content_t> search;
				search.push_back(CONTENT_LIFE_SUPPORT);
				if (!searchNear(pp,v3s16(-4,-2,-4),v3s16(5,5,5),search,NULL))
					damageLocalPlayer(PLAYER_ALL,DAMAGE_POISON,5);
			}
				break;
			case BIOME_UNKNOWN:
			case BIOME_WOODLANDS:
			if (season == ENV_SEASON_WINTER) {
				std::vector<content_t> search;
				search.push_back(CONTENT_FIRE);
				search.push_back(CONTENT_CAMPFIRE);
				if (!searchNear(pp,v3s16(-4,-2,-4),v3s16(5,5,5),search,NULL))
					damageLocalPlayer(PLAYER_ALL,DAMAGE_COLD,5);
			}
			default:
				break;
			}
		}

		if (m_hunger_interval.step(dtime,5.0)) {
			f32 speed = lplayer->getSpeed().getLength();
			s8 hungry = 0;
			s32 chance = 200;
			if (speed > 1.0) {
				chance = 50;
				if (speed > 50.0) {
					chance = 0;
					hungry = 1;
				}
			}
			if (chance && myrand()%chance == 0)
				hungry = 1;
			if (hungry) {
				ClientEnvEvent event;
				event.type = CEE_PLAYER_HUNGER;
				event.player_damage.amount = 3;
				m_client_event_queue.push_back(event);
			}
			// a little discouragement for running around naked
			{
				bool safe = false;
				InventoryList *sl = lplayer->inventory.getList("shirt");
				InventoryList *jl = lplayer->inventory.getList("jacket");
				InventoryList *pl = lplayer->inventory.getList("pants");
				if ((sl || jl) && pl) {
					InventoryItem *si = NULL;
					if (sl)
						si = sl->getItem(0);
					InventoryItem *ji = NULL;
					if (jl)
						ji = jl->getItem(0);
					InventoryItem *pi = pl->getItem(0);
					if ((si || ji) && pi)
						safe = true;
				}
				if (!safe)
					damageLocalPlayer((PLAYER_TORSO|PLAYER_RLEG|PLAYER_LLEG),DAMAGE_EXPOSURE,2);
			}
		}
	}

	/*
		Stuff that can be done in an arbitarily large dtime
	*/
	Player *player;
	uint32_t i;
	for (i=0; i<m_players->length; i++) {
		player = (Player*)array_get_ptr(m_players,i);
		if (!player)
			continue;
		v3f playerpos = player->getPosition();

		/*
			Handle non-local players
		*/
		if (player->isLocal() == false) {
			// Move
			player->move(dtime, *m_map, 100*BS);
		}

		// Update lighting on all players on client
		u8 light = LIGHT_MAX;
		{
			// Get node at head
			bool pos_ok;
			v3s16 p = player->getLightPosition();
			MapNode n = m_map->getNodeNoEx(p,&pos_ok);
			if (pos_ok)
				light = n.getLightBlend(getDayNightRatio());
		}
		player->updateLight(light);

		/*
			Add footsteps to grass
		*/
		if (footprints) {
			// Get node that is at BS/4 under player
			v3s16 bottompos = floatToInt(playerpos + v3f(0,-BS/4,0), BS);
			MapNode n = m_map->getNodeNoEx(bottompos);
			if (content_features(n.getContent()).draw_type == CDT_DIRTLIKE && (n.param1&0xF0) == 0) {
				n.param1 |= 0x10;
				n.envticks = 0;
				m_map->setNode(bottompos, n);
				// Update mesh on client
				if (m_map->mapType() == MAPTYPE_CLIENT) {
					v3s16 p_blocks = getNodeBlockPos(bottompos);
					m_client->addUpdateMeshTask(p_blocks);
				}
			}
		}
	}

	/*
		Step active objects and update lighting of them
	*/

	for (std::map<u16, ClientActiveObject*>::iterator i = m_active_objects.begin(); i != m_active_objects.end(); i++) {
		ClientActiveObject* obj = i->second;
		// Step object
		obj->step(dtime, this);

		if (m_active_object_light_update_interval.step(dtime, 0.21)) {
			u8 light = 0;
			bool pos_ok;
			v3s16 p = obj->getLightPosition();
			MapNode n = m_map->getNodeNoEx(p,&pos_ok);
			if (pos_ok)
				light = n.getLightBlend(getDayNightRatio());
			obj->updateLight(light);
		}
	}
}

void ClientEnvironment::expireMeshes(bool only_daynight_diffed)
{
	m_map->expireMeshes(only_daynight_diffed);
}

ClientActiveObject* ClientEnvironment::getActiveObject(u16 id)
{
	std::map<u16, ClientActiveObject*>::iterator i = m_active_objects.find(id);
	if (i == m_active_objects.end())
		return NULL;
	return i->second;
}

bool isFreeClientActiveObjectId(u16 id, std::map<u16, ClientActiveObject*> &objects)
{
	if (id == 0)
		return false;

	std::map<u16, ClientActiveObject*>::iterator i = objects.find(id);
	if (i == objects.end())
		return true;
	return false;
}

u16 getFreeClientActiveObjectId(std::map<u16, ClientActiveObject*> &objects)
{
	//try to reuse id's as late as possible
	static u16 last_used_id = 0;
	u16 startid = last_used_id;

	for (;;) {
		last_used_id ++;
		if (isFreeClientActiveObjectId(last_used_id, objects))
			return last_used_id;

		if(last_used_id == startid)
			return 0;
	}
	return 0;
}

u16 ClientEnvironment::addActiveObject(ClientActiveObject *object)
{
	assert(object);
	if(object->getId() == 0)
	{
		u16 new_id = getFreeClientActiveObjectId(m_active_objects);
		if(new_id == 0)
		{
			infostream<<"ClientEnvironment::addActiveObject(): "
					<<"no free ids available"<<std::endl;
			delete object;
			return 0;
		}
		object->setId(new_id);
	}
	if(isFreeClientActiveObjectId(object->getId(), m_active_objects) == false)
	{
		infostream<<"ClientEnvironment::addActiveObject(): "
				<<"id is not free ("<<object->getId()<<")"<<std::endl;
		delete object;
		return 0;
	}
	infostream<<"ClientEnvironment::addActiveObject(): "
			<<"added (id="<<object->getId()<<")"<<std::endl;
	m_active_objects[object->getId()] = object;
	object->addToScene(m_smgr);
	{ // Update lighting immediately
		u8 light = 0;
		bool pos_ok;
		v3s16 p = object->getLightPosition();
		MapNode n = m_map->getNodeNoEx(p,&pos_ok);
		if (pos_ok)
			light = n.getLightBlend(getDayNightRatio());
		object->updateLight(light);
	}
	return object->getId();
}

void ClientEnvironment::addActiveObject(u16 id, u8 type, const std::string &init_data)
{
	ClientActiveObject* obj = ClientActiveObject::create(type);
	if (obj == NULL) {
		infostream<<"ClientEnvironment::addActiveObject(): "
				<<"id="<<id<<" type="<<type<<": Couldn't create object"
				<<std::endl;
		return;
	}

	obj->setId(id);

	obj->initialize(init_data);

	addActiveObject(obj);
}

void ClientEnvironment::removeActiveObject(u16 id)
{
	infostream<<"ClientEnvironment::removeActiveObject(): "
			<<"id="<<id<<std::endl;
	ClientActiveObject* obj = getActiveObject(id);
	if (obj == NULL) {
		infostream<<"ClientEnvironment::removeActiveObject(): "
				<<"id="<<id<<" not found"<<std::endl;
		return;
	}
	obj->removeFromScene();
	delete obj;
	m_active_objects.erase(id);
}

void ClientEnvironment::processActiveObjectMessage(u16 id,
		const std::string &data)
{
	ClientActiveObject* obj = getActiveObject(id);
	if (obj == NULL) {
		infostream<<"ClientEnvironment::processActiveObjectMessage():"
				<<" got message for id="<<id<<", which doesn't exist."
				<<std::endl;
		return;
	}
	obj->processMessage(data);
}

/*
	Callbacks for activeobjects
*/

void ClientEnvironment::damageLocalPlayer(u8 area, u8 type, u8 damage)
{
	if (!m_client->getServerDamage())
		return;
	LocalPlayer *lplayer = getLocalPlayer();
	assert(lplayer);
	f32 effect = lplayer->getProtection(area,type);
	f32 f_damage = damage;

	if (lplayer->health < 1)
		return;

	if (damage > 0 && effect > 0.0) {
		f_damage -= f_damage*effect;
		ClientEnvEvent event;
		event.type = CEE_PLAYER_WEARCLOTHES;
		event.player_wear.amount = damage*(10*(2.0-effect));
		m_client_event_queue.push_back(event);
		if (f_damage < 1.0 && f_damage > 0.0) {
			damage = 1.0/f_damage;
			if (myrand_range(0,damage) == 0)
				f_damage = 1.0;
		}
		damage = f_damage;
		if (damage < 1)
			return;
	}

	ClientEnvEvent event;
	if (type == DAMAGE_AIR) {
		event.type = CEE_PLAYER_SUFFOCATE;
	}else{
		lplayer->addDamage(area,type,damage);
		event.type = CEE_PLAYER_DAMAGE;
	}
	event.player_damage.amount = damage;
	m_client_event_queue.push_back(event);
}

/*
	Client likes to call these
*/

void ClientEnvironment::getActiveObjects(v3f origin, f32 max_d, core::array<DistanceSortedActiveObject> &dest)
{
	for (std::map<u16, ClientActiveObject*>::iterator i = m_active_objects.begin(); i != m_active_objects.end(); i++) {
		ClientActiveObject* obj = i->second;

		f32 d = (obj->getPosition() - origin).getLength();

		if(d > max_d)
			continue;

		DistanceSortedActiveObject dso(obj, d);

		dest.push_back(dso);
	}
}

ClientEnvEvent ClientEnvironment::getClientEvent()
{
	if(m_client_event_queue.size() == 0)
	{
		ClientEnvEvent event;
		event.type = CEE_NONE;
		return event;
	}
	return m_client_event_queue.pop_front();
}

bool ClientEnvironment::searchNear(v3s16 pos, v3s16 radius_min, v3s16 radius_max, std::vector<content_t> c, v3s16 *found)
{
	v3s16 blockpos = getNodeBlockPos(pos);
	MapBlock* const block = m_map->getBlockNoCreateNoEx(blockpos);
	if (!block)
		return false;
	v3s16 relpos = blockpos*MAP_BLOCKSIZE;
	v3s16 p;
	v3s16 bp;

	for(s16 x=radius_min.X; x<=radius_max.X; x++)
	{
		for(s16 y=radius_min.Y; y<=radius_max.Y; y++)
		{
			for(s16 z=radius_min.Z; z<=radius_max.Z; z++)
			{
				if (!x && !y && !z)
					continue;
				p = pos+v3s16(x,y,z);
				MapNode n_test;
				bp = getNodeBlockPos(p);
				if (bp == blockpos)
				{
					bool pos_ok;
					n_test = block->getNodeNoCheck(p-relpos,&pos_ok);
				}
				else
					n_test = m_map->getNodeNoEx(p);

				for (std::vector<content_t>::iterator i=c.begin(); i != c.end(); i++)
				{
					if (n_test.getContent() == *i)
					{
						if (found != NULL)
							*found = p;
						return true;
					}
				}
			}
		}
	}
	
	block->ResetCurrent();
	return false;
}

bool ClientEnvironment::searchNearInv(v3s16 pos, v3s16 radius_min, v3s16 radius_max, std::vector<content_t> c, v3s16 *found)
{
	v3s16 blockpos = getNodeBlockPos(pos);
	MapBlock* const block = m_map->getBlockNoCreateNoEx(blockpos);
	if (!block)
		return false;
	v3s16 relpos = blockpos*MAP_BLOCKSIZE;
	v3s16 p;
	v3s16 bp;
	
	for(s16 x=radius_min.X; x<=radius_max.X; x++)
	{
		for(s16 y=radius_min.Y; y<=radius_max.Y; y++)
		{
			for(s16 z=radius_min.Z; z<=radius_max.Z; z++)
			{
				if (!x && !y && !z)
					continue;
				p = pos+v3s16(x,y,z);
				MapNode n_test;
				bp = getNodeBlockPos(p);
				if (bp == blockpos)
				{
					bool pos_ok;
					n_test = block->getNodeNoCheck(p-relpos,&pos_ok);
				}
				else
					n_test = m_map->getNodeNoEx(p);

				bool s = false;
				for (std::vector<content_t>::iterator i=c.begin(); i != c.end(); i++)
				{
					if (n_test.getContent() == *i)
					{
						s = true;
						break;
					}
				}
				if (!s)
				{
					if (found != NULL)
						*found = p;
					return true;
				}
			}
		}
	}

	block->ResetCurrent();
	return false;
}

void ClientEnvironment::updateObjectsCameraOffset(v3s16 camera_offset)
{
	for (std::map<u16, ClientActiveObject*>::iterator i = m_active_objects.begin(); i != m_active_objects.end(); i++)
	{
		ClientActiveObject* obj = i->second;
		obj->updateCameraOffset(camera_offset);
	}

	Player *player;
	uint32_t i;
	for (i=0; i<m_players->length; i++)
	{
		player = (Player*)array_get_ptr(m_players,i);
		if (!player)
			continue;
		if (!player->isLocal())
			((RemotePlayer*)player)->updateCameraOffset(camera_offset);
	}
}

#endif // #ifndef SERVER


