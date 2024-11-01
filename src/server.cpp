/************************************************************************
* Minetest-c55
* Copyright (C) 2010-2011 celeron55, Perttu Ahola <celeron55@gmail.com>
*
* server.cpp
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

#include "server.h"
#include "utility.h"
#include <iostream>
#include "clientserver.h"
#include "map.h"
#include "jmutexautolock.h"
#include "main.h"
#include "constants.h"
#include "voxel.h"
#include "mineral.h"
#include "config.h"
#include "content_object.h"
#include "content_mapnode.h"
#include "content_craft.h"
#include "content_craftitem.h"
#include "content_toolitem.h"
#include "content_nodemeta.h"
#include "mapblock.h"
#include "serverobject.h"
#include "content_sao.h"
#include "profiler.h"
#include "log.h"
#include "base64.h"
#include "http.h"
#include "enchantment.h"
#include "path.h"

#define PP(x) "("<<(x).X<<","<<(x).Y<<","<<(x).Z<<")"

#define BLOCK_EMERGE_FLAG_FROMDISK (1<<0)

#define SECONDS_PER_DAY 86400
#define SECONDS_PER_HOUR 3600

class MapEditEventIgnorer
{
public:
	MapEditEventIgnorer(bool *flag):
		m_flag(flag)
	{
		if(*m_flag == false)
			*m_flag = true;
		else
			m_flag = NULL;
	}

	~MapEditEventIgnorer()
	{
		if(m_flag)
		{
			assert(*m_flag);
			*m_flag = false;
		}
	}

private:
	bool *m_flag;
};

void* ServerThread::Thread()
{
	ThreadStarted();
	log_mutex.Lock();
	log_register_thread("ServerThread");
	log_mutex.Unlock();

	DSTACK(__FUNCTION_NAME);

	BEGIN_DEBUG_EXCEPTION_HANDLER

	while(getRun())
	{
		try{
			//TimeTaker timer("AsyncRunStep() + Receive()");

			{
				//TimeTaker timer("AsyncRunStep()");
				m_server->AsyncRunStep();
			}

			//infostream<<"Running m_server->Receive()"<<std::endl;
			m_server->Receive();
		}
		catch(con::NoIncomingDataException &e)
		{
		}
		catch(con::PeerNotFoundException &e)
		{
			infostream<<"Server: PeerNotFoundException"<<std::endl;
		}
	}

	END_DEBUG_EXCEPTION_HANDLER(errorstream)

	return NULL;
}

void * EmergeThread::Thread()
{
	ThreadStarted();
	log_mutex.Lock();
	log_register_thread("EmergeThread");
	log_mutex.Unlock();

	DSTACK(__FUNCTION_NAME);

	BEGIN_DEBUG_EXCEPTION_HANDLER

	/*
		Get block info from queue, emerge them and send them
		to clients.

		After queue is empty, exit.
	*/
	while(getRun())
	{
		QueuedBlockEmerge* const qptr = m_server->m_emerge_queue.pop();
		if (qptr == NULL)
			break;

		SharedPtr<QueuedBlockEmerge> q(qptr);

		v3s16 &p = q->pos;
		v2s16 p2d(p.X,p.Z);

		/*
			Do not generate over-limit
		*/
		if (p.X < -MAP_GENERATION_LIMIT / MAP_BLOCKSIZE
		|| p.X > MAP_GENERATION_LIMIT / MAP_BLOCKSIZE
		|| p.Y < -MAP_GENERATION_LIMIT / MAP_BLOCKSIZE
		|| p.Y > MAP_GENERATION_LIMIT / MAP_BLOCKSIZE
		|| p.Z < -MAP_GENERATION_LIMIT / MAP_BLOCKSIZE
		|| p.Z > MAP_GENERATION_LIMIT / MAP_BLOCKSIZE)
			continue;

		//infostream<<"EmergeThread::Thread(): running"<<std::endl;

		//TimeTaker timer("block emerge");

		/*
			Try to emerge it from somewhere.

			If it is only wanted as optional, only loading from disk
			will be allowed.
		*/

		/*
			Check if any peer wants it as non-optional. In that case it
			will be generated.

			Also decrement the emerge queue count in clients.
		*/

		bool only_from_disk = true;

		{
			core::map<u16, u8>::Iterator i;
			for(i=q->peer_ids.getIterator(); i.atEnd()==false; i++)
			{
				//u16 peer_id = i.getNode()->getKey();

				// Check flags
				u8 flags = i.getNode()->getValue();
				if((flags & BLOCK_EMERGE_FLAG_FROMDISK) == false)
					only_from_disk = false;

			}
		}

		//vlprintf(CN_DEBUG,"EmergeThread: p=(%d,%d,%d) only_from_disk = %d",p.X,p.Y,p.Z,only_from_disk);
		
		JMutexAutoLock envlock(m_server->m_env_mutex);
		ServerMap& map = ((ServerMap&) m_server->m_env.getMap());

		//core::map<v3s16, MapBlock*> changed_blocks;
		//core::map<v3s16, MapBlock*> lighting_invalidated_blocks;

		MapBlock* block = NULL;
		bool got_block = true;
		core::map<v3s16, MapBlock*> modified_blocks;

		/*
			Fetch block from map or generate a single block
		*/
		{
			// Load sector if it isn't loaded
			map.getSectorNoGenerateNoEx(p2d);
			block = map.getBlockNoCreateNoEx(p);
			    
			if (!block || block->isDummy() || !block->isGenerated())
			{
				bool was_generated = false;
				//vlprintf(CN_DEBUG,"EmergeThread: not in memory, loading");

				// Load/generate block
				if(block)
				    block->ResetCurrent();
				block = map.loadBlock(p);

				if (only_from_disk == false)
				{
					if (!block || block->isGenerated() == false)
					{
						//vlprintf(CN_DEBUG,"EmergeThread: generating");
						block = map.generateBlock(p, modified_blocks);
						was_generated = true;
					}
				}

				//vlprintf(CN_DEBUG,"EmergeThread: ended up with: %s",analyze_block(block).c_str());

				if (!block)
					got_block = false;
				else
				{
					/*
						Ignore map edit events, they will not need to be
						sent to anybody because the block hasn't been sent
						to anybody
					*/
					MapEditEventIgnorer ign(&m_server->m_ignore_map_edit_events);

					// Activate objects and stuff
					m_server->m_env.activateBlock(block, 3600);
				}

				if (was_generated && myrand_range(0,27) == 0)
				{
					bool has_spawn = false;
					bool water_spawn = false;
					v3s16 bsp(0,0,0);
					v3s16 sp(0,0,0);
					v3s16 wsp(0,0,0);

					/* find a place to spawn, bias to water */
					v3s16 p0;
					for (p0.X=0; !water_spawn && p0.X<MAP_BLOCKSIZE; p0.X++) {
					for (p0.Y=0; !water_spawn && p0.Y<MAP_BLOCKSIZE; p0.Y++) {
					for (p0.Z=0; !water_spawn && p0.Z<MAP_BLOCKSIZE; p0.Z++) {
						v3s16 p = p0 + block->getPosRelative();
						MapNode n = block->getNodeNoEx(p0);
						MapNode n1 = block->getNodeNoEx(p0+v3s16(0,1,0));
						MapNode n2 = block->getNodeNoEx(p0+v3s16(0,2,0));
						if (n1.getContent() == CONTENT_IGNORE || n2.getContent() == CONTENT_IGNORE)
							continue;
						if (
							n.getContent() == CONTENT_WATERSOURCE
							&& n1.getContent() == CONTENT_WATERSOURCE
							&& n2.getContent() == CONTENT_WATERSOURCE
						) {
							water_spawn = true;
							wsp = p;
							break;
						}
						if (has_spawn)
							continue;
						if (
							content_features(n.getContent()).draw_type == CDT_DIRTLIKE
							&& content_features(n1.getContent()).air_equivalent
							&& content_features(n2.getContent()).air_equivalent
						) {
							has_spawn = true;
							sp = p+v3s16(0,1,0);
							bsp = p0;
						}
					}
					}
					}

					if (water_spawn)
					{
						if (myrand_range(0,5) == 0)
							mob_spawn_hostile(wsp,true,&m_server->m_env);
						else
							mob_spawn_passive(wsp,true,&m_server->m_env);
					}
					else if (has_spawn)
					{
						MapNode n = block->getNodeNoEx(bsp);
						u8 overlay = (n.param1&0x0F);
						if (overlay == 0x01 || overlay == 0x02
								|| (overlay == 0x04 && myrand_range(0,5) == 0)) {
							mob_spawn_passive(sp,false,&m_server->m_env);
						}else if (overlay == 0x00 && block->getPosRelative().Y < -16) {
							mob_spawn(sp,CONTENT_MOB_RAT,&m_server->m_env);
						}else if (overlay == 0x08) {
							for (int i=0; i<4; i++) {
								mob_spawn(sp,CONTENT_MOB_FIREFLY,&m_server->m_env);
							}
						}
					}
				}

				if(block)
				    block->ResetCurrent();
			}

			// TODO: Some additional checking and lighting updating,
			//       see emergeBlock
		}

		/*
			Set sent status of modified blocks on clients
		*/

		// NOTE: Server's clients are also behind the connection mutex
		JMutexAutoLock lock(m_server->m_con_mutex);

		/*
			Add the originally fetched block to the modified list
		*/
		if (got_block)
			modified_blocks.insert(p, block);

		/*
			Set the modified blocks unsent for all the clients
		*/

		for (core::map<u16, RemoteClient*>::Iterator i = m_server->m_clients.getIterator(); i.atEnd() == false; i++)
		{
			RemoteClient* const client = i.getNode()->getValue();

			// Remove block from sent history
			if (modified_blocks.size() > 0)
				client->SetBlocksNotSent(modified_blocks);
		}

		if(block)
		    block->ResetCurrent();
	}

	END_DEBUG_EXCEPTION_HANDLER(errorstream)

	return NULL;
}

void RemoteClient::GetNextBlocks(Server *server, float dtime,
		core::array<PrioritySortedBlockTransfer> &dest)
{
	DSTACK(__FUNCTION_NAME);

    // Protected by mutexs in the caller.
	
	/*u32 timer_result;
	TimeTaker timer("RemoteClient::GetNextBlocks", &timer_result);*/

	// Increment timers
	m_nothing_to_send_pause_timer -= dtime;
	m_nearest_unsent_reset_timer += dtime;

	if (m_nothing_to_send_pause_timer >= 0)
		return;

	// Won't send anything if already sending
	if (m_blocks_sending.size() >= (uint32_t)config_get_int("server.net.client.queue.size"))
		return;

	//TimeTaker timer("RemoteClient::GetNextBlocks");

	Player* const player = server->m_env.getPlayer(peer_id);

	assert(player != NULL);

	v3f playerpos = player->getPosition();
	v3f playerspeed = player->getSpeed();
	v3f playerspeeddir(0,0,0);
	if(playerspeed.getLength() > 1.0*BS)
		playerspeeddir = playerspeed / playerspeed.getLength();
	// Predict to next block
	v3f playerpos_predicted = playerpos + playerspeeddir*MAP_BLOCKSIZE*BS;

	v3s16 center_nodepos = floatToInt(playerpos_predicted, BS);

	v3s16 center = getNodeBlockPos(center_nodepos);

	// Camera position and direction
	v3f camera_pos = player->getEyePosition();
	v3f camera_dir = v3f(0,0,1);
	camera_dir.rotateYZBy(player->getPitch());
	camera_dir.rotateXZBy(player->getYaw());

	/*infostream<<"camera_dir=("<<camera_dir.X<<","<<camera_dir.Y<<","
			<<camera_dir.Z<<")"<<std::endl;*/

	/*
		Get the starting value of the block finder radius.
	*/

	if(m_last_center != center)
	{
		m_nearest_unsent_d = 0;
		m_last_center = center;
	}

	/*infostream<<"m_nearest_unsent_reset_timer="
			<<m_nearest_unsent_reset_timer<<std::endl;*/

	// Reset periodically to workaround for some bugs or stuff
	if(m_nearest_unsent_reset_timer > 20.0)
	{
		m_nearest_unsent_reset_timer = 0;
		m_nearest_unsent_d = 0;
		//infostream<<"Resetting m_nearest_unsent_d for "
		//		<<server->getPlayerName(peer_id)<<std::endl;
	}

	//s16 last_nearest_unsent_d = m_nearest_unsent_d;
	s16 d_start = m_nearest_unsent_d;

	//infostream<<"d_start="<<d_start<<std::endl;

	uint16_t max_simul_sends_setting = config_get_int("server.net.client.queue.size");
	uint16_t max_simul_sends_usually = max_simul_sends_setting;

	/*
		Check the time from last addNode/removeNode.

		Decrease send rate if player is building stuff.
	*/
	m_time_from_building += dtime;
	if (m_time_from_building < config_get_float("server.net.client.queue.delay"))
		max_simul_sends_usually = LIMITED_MAX_SIMULTANEOUS_BLOCK_SENDS;

	/*
		Number of blocks sending + number of blocks selected for sending
	*/
	u32 num_blocks_selected = m_blocks_sending.size();

	/*
		next time d will be continued from the d from which the nearest
		unsent block was found this time.

		This is because not necessarily any of the blocks found this
		time are actually sent.
	*/
	s32 new_nearest_unsent_d = -1;

	int d_max = config_get_int("world.server.chunk.range.send");
	int d_max_gen = config_get_int("world.server.chunk.range.generate");

	// Don't loop very much at a time
	s16 max_d_increment_at_time = 2;
	if(d_max > d_start + max_d_increment_at_time)
		d_max = d_start + max_d_increment_at_time;
	/*if(d_max_gen > d_start+2)
		d_max_gen = d_start+2;*/

	//infostream<<"Starting from "<<d_start<<std::endl;

	s32 nearest_emerged_d = -1;
	s32 nearest_emergefull_d = -1;
	s32 nearest_sent_d = -1;
	//bool queue_is_full = false;

	s16 d;
	for(d = d_start; d <= d_max; d++)
	{
		/*errorstream<<"checking d="<<d<<" for "
				<<server->getPlayerName(peer_id)<<std::endl;*/
		//infostream<<"RemoteClient::SendBlocks(): d="<<d<<std::endl;

		/*
			If m_nearest_unsent_d was changed by the EmergeThread
			(it can change it to 0 through SetBlockNotSent),
			update our d to it.
			Else update m_nearest_unsent_d
		*/
		/*if(m_nearest_unsent_d != last_nearest_unsent_d)
		{
			d = m_nearest_unsent_d;
			last_nearest_unsent_d = m_nearest_unsent_d;
		}*/

		/*
			Get the border/face dot coordinates of a "d-radiused"
			box
		*/
		core::list<v3s16> list;
		getFacePositions(list, d);

		core::list<v3s16>::Iterator li;
		for(li=list.begin(); li!=list.end(); li++)
		{
			v3s16 p = *li + center;

			/*
				Send throttling
				- Don't allow too many simultaneous transfers
				- EXCEPT when the blocks are very close

				Also, don't send blocks that are already flying.
			*/

			// Start with the usual maximum
			u16 max_simul_dynamic = max_simul_sends_usually;

			// If block is very close, allow full maximum
			if(d <= BLOCK_SEND_DISABLE_LIMITS_MAX_D)
				max_simul_dynamic = max_simul_sends_setting;

			// Don't select too many blocks for sending
			if(num_blocks_selected >= max_simul_dynamic)
			{
				//queue_is_full = true;
				goto queue_full_break;
			}

			// Don't send blocks that are currently being transferred
			if(m_blocks_sending.find(p) != NULL)
				continue;

			/*
				Do not go over-limit
			*/
			if(p.X < -MAP_GENERATION_LIMIT / MAP_BLOCKSIZE
			|| p.X > MAP_GENERATION_LIMIT / MAP_BLOCKSIZE
			|| p.Y < -MAP_GENERATION_LIMIT / MAP_BLOCKSIZE
			|| p.Y > MAP_GENERATION_LIMIT / MAP_BLOCKSIZE
			|| p.Z < -MAP_GENERATION_LIMIT / MAP_BLOCKSIZE
			|| p.Z > MAP_GENERATION_LIMIT / MAP_BLOCKSIZE)
				continue;

			// If this is true, inexistent block will be made from scratch
			bool generate = d <= d_max_gen;

			{
				/*// Limit the generating area vertically to 2/3
				if(abs(p.Y - center.Y) > d_max_gen - d_max_gen / 3)
					generate = false;*/

				// Limit the send area vertically to 1/2
				if(abs(p.Y - center.Y) > d_max / 2)
					continue;
			}

#if 0
			/*
				If block is far away, don't generate it unless it is
				near ground level.
			*/
			if(d >= 4)
			{
	#if 1
				// Block center y in nodes
				f32 y = (f32)(p.Y * MAP_BLOCKSIZE + MAP_BLOCKSIZE/2);
				// Don't generate if it's very high or very low
				if(y < -64 || y > 64)
					generate = false;
	#endif
	#if 0
				v2s16 p2d_nodes_center(
					MAP_BLOCKSIZE*p.X,
					MAP_BLOCKSIZE*p.Z);

				// Get ground height in nodes
				s16 gh = server->m_env.getServerMap().findGroundLevel(
						p2d_nodes_center);

				// If differs a lot, don't generate
				if(fabs(gh - y) > MAP_BLOCKSIZE*2)
					generate = false;
					// Actually, don't even send it
					//continue;
	#endif
			}
#endif

			//infostream<<"d="<<d<<std::endl;
#if 1
			/*
				Don't generate or send if not in sight
				FIXME This only works if the client uses a small enough
				FOV setting. The default of 72 degrees is fine.
			*/

			const float camera_fov = (72.0*PI/180) * 4./3.;
			
			if(isBlockInSight(p, camera_pos, camera_dir, camera_fov, 10000*BS) == false)
				continue;
#endif
			/*
				Don't send already sent blocks
			*/
			{
				if(m_blocks_sent.find(p) != NULL)
				{
					continue;
				}
			}

			/*
				Check if map has this block
			*/
			MapBlock* const block = server->m_env.getMap().getBlockNoCreateNoEx(p);
			bool surely_not_found_on_disk = false;
			bool block_is_invalid = false;
			
			if(block)
			{
				// Reset usage timer, this block will be of use in the future.
				block->resetUsageTimer();

				// Block is dummy if data doesn't exist.
				// It means it has been not found from disk and not generated
				if(block->isDummy())
				{
					surely_not_found_on_disk = true;
				}

				// Block is valid if lighting is up-to-date and data exists
				if(block->isValid() == false)
				{
					block_is_invalid = true;
				}

				/*if(block->isFullyGenerated() == false)
				{
					block_is_invalid = true;
				}*/

#if 0
				v2s16 p2d(p.X, p.Z);
				ServerMap *map = (ServerMap*)(&server->m_env.getMap());
				v2s16 chunkpos = map->sector_to_chunk(p2d);
				if(map->chunkNonVolatile(chunkpos) == false)
					block_is_invalid = true;
#endif
				if(block->isGenerated() == false)
					block_is_invalid = true;
#if 1
				/*
					If block is not close, don't send it unless it is near
					ground level.

					Block is near ground level if night-time mesh
					differs from day-time mesh.
				*/
				if(d >= 4)
				{
					if(block->dayNightDiffed() == false)
						continue;
				}
#endif
				block->ResetCurrent();
			}

			/*
				If block has been marked to not exist on disk (dummy)
				and generating new ones is not wanted, skip block.
			*/
			if(generate == false && surely_not_found_on_disk == true)
			{
				// get next one.
				continue;
			}

			/*
				Add inexistent block to emerge queue.
			*/
			if(block == NULL || surely_not_found_on_disk || block_is_invalid)
			{
				//TODO: Get value from somewhere
				// Allow only one block in emerge queue
				//if(server->m_emerge_queue.peerItemCount(peer_id) < 1)
				// Allow two blocks in queue per client
				//if(server->m_emerge_queue.peerItemCount(peer_id) < 2)
				if(server->m_emerge_queue.peerItemCount(peer_id) < 25)
				{
					//infostream<<"Adding block to emerge queue"<<std::endl;

					// Add it to the emerge queue and trigger the thread

					u8 flags = 0;
					if(generate == false)
						flags |= BLOCK_EMERGE_FLAG_FROMDISK;

					server->m_emerge_queue.addBlock(peer_id, p, flags);
					server->m_emergethread.trigger();

					if(nearest_emerged_d == -1)
						nearest_emerged_d = d;
				} else {
					if(nearest_emergefull_d == -1)
						nearest_emergefull_d = d;
				}

				// get next one.
				continue;
			}

			if(nearest_sent_d == -1)
				nearest_sent_d = d;

			/*
				Add block to send queue
			*/

			/*errorstream<<"sending from d="<<d<<" to "
					<<server->getPlayerName(peer_id)<<std::endl;*/

			PrioritySortedBlockTransfer q((float)d, p, peer_id);

			dest.push_back(q);

			num_blocks_selected += 1;
		}
	}
queue_full_break:

	//infostream<<"Stopped at "<<d<<std::endl;

	// If nothing was found for sending and nothing was queued for
	// emerging, continue next time browsing from here
	if (nearest_emerged_d != -1) {
		new_nearest_unsent_d = nearest_emerged_d;
	}else if (nearest_emergefull_d != -1) {
		new_nearest_unsent_d = nearest_emergefull_d;
	}else{
		if (d > config_get_int("world.server.chunk.range.send")) {
			new_nearest_unsent_d = 0;
			m_nothing_to_send_pause_timer = 2.0;
			/*infostream<<"GetNextBlocks(): d wrapped around for "
					<<server->getPlayerName(peer_id)
					<<"; setting to 0 and pausing"<<std::endl;*/
		}else{
			if (nearest_sent_d != -1) {
				new_nearest_unsent_d = nearest_sent_d;
			}else{
				new_nearest_unsent_d = d;
			}
		}
	}

	if (new_nearest_unsent_d != -1)
		m_nearest_unsent_d = new_nearest_unsent_d;

	/*timer_result = timer.stop(true);
	if(timer_result != 0)
		infostream<<"GetNextBlocks duration: "<<timer_result<<" (!=0)"<<std::endl;*/
}

void RemoteClient::GotBlock(v3s16 p)
{
	if(m_blocks_sending.find(p) != NULL)
		m_blocks_sending.remove(p);
	else
	{
		/*infostream<<"RemoteClient::GotBlock(): Didn't find in"
				" m_blocks_sending"<<std::endl;*/
		m_excess_gotblocks++;
	}
	m_blocks_sent.insert(p, true);
}

void RemoteClient::SentBlock(v3s16 p)
{
	if (m_blocks_sending.find(p) == NULL) {
		m_blocks_sending.insert(p, 0.0);
	}else{
		infostream<<"RemoteClient::SentBlock(): Sent block"
				" already in m_blocks_sending"<<std::endl;
	}
}

void RemoteClient::SetBlockNotSent(v3s16 p)
{
	m_nearest_unsent_d = 0;

	if(m_blocks_sending.find(p) != NULL)
		m_blocks_sending.remove(p);
	if(m_blocks_sent.find(p) != NULL)
		m_blocks_sent.remove(p);
}

void RemoteClient::SetBlocksNotSent(core::map<v3s16, MapBlock*> &blocks)
{
	m_nearest_unsent_d = 0;

	for(core::map<v3s16, MapBlock*>::Iterator
			i = blocks.getIterator();
			i.atEnd()==false; i++)
	{
		v3s16 p = i.getNode()->getKey();

		if(m_blocks_sending.find(p) != NULL)
			m_blocks_sending.remove(p);
		if(m_blocks_sent.find(p) != NULL)
			m_blocks_sent.remove(p);
	}
}

/*
	PlayerInfo
*/

PlayerInfo::PlayerInfo()
{
	name[0] = 0;
	avg_rtt = 0;
}

void PlayerInfo::PrintLine(std::ostream *s)
{
	(*s)<<id<<": ";
	(*s)<<"\""<<name<<"\" ("
			<<(position.X/10)<<","<<(position.Y/10)
			<<","<<(position.Z/10)<<") ";
	address.print(s);
	(*s)<<" avg_rtt="<<avg_rtt;
	(*s)<<std::endl;
}

u32 PIChecksum(core::list<PlayerInfo> &l)
{
	core::list<PlayerInfo>::Iterator i;
	u32 checksum = 1;
	u32 a = 10;
	for(i=l.begin(); i!=l.end(); i++)
	{
		checksum += a * (i->id+1);
		checksum ^= 0x435aafcd;
		a *= 10;
	}
	return checksum;
}

/*
	Server
*/

Server::Server():
	m_env(new ServerMap(), this),
	m_con(PROTOCOL_ID, 512, CONNECTION_TIMEOUT, this),
	m_thread(this),
	m_emergethread(this),
	m_time_of_day_send_timer(0),
	m_uptime(0),
	m_shutdown_requested(false),
	m_ignore_map_edit_events(false)
{

	auth_init("auth.txt");
	ban_init("ipban.txt");

	m_liquid_transform_timer = 0.0;
	m_print_info_timer = 0.0;
	m_objectdata_timer = 0.0;
	m_emergethread_trigger_timer = 0.0;
	m_savemap_timer = 0.0;
	m_send_object_info_timer = 0.0;
	m_send_full_inventory_timer = 0.0;

	m_env_mutex.Init();
	m_con_mutex.Init();
	m_step_dtime_mutex.Init();
	m_step_dtime = 0.0;

	// Register us to receive map edit events
	m_env.getMap().addEventReceiver(this);

	infostream<<"Server: Loading environment metadata"<<std::endl;
	m_env.loadMeta();

	// Load players
	infostream<<"Server: Loading players"<<std::endl;
	m_env.deSerializePlayers();

	config_save("world","world","world.cfg");
}

Server::~Server()
{
	infostream<<"Server::~Server()"<<std::endl;

	/*
		Send shutdown message
	*/
	{
		JMutexAutoLock conlock(m_con_mutex);

		std::wstring line = L"*** Server shutting down";

		/*
			Send the message to clients
		*/
		for(core::map<u16, RemoteClient*>::Iterator
			i = m_clients.getIterator();
			i.atEnd() == false; i++)
		{
			// Get client and check that it is valid
			RemoteClient *client = i.getNode()->getValue();
			assert(client->peer_id == i.getNode()->getKey());
			if(client->serialization_version == SER_FMT_VER_INVALID)
				continue;

			try{
				SendChatMessage(client->peer_id, line);
			}
			catch(con::PeerNotFoundException &e)
			{}
		}
	}

	{
		JMutexAutoLock envlock(m_env_mutex);

		/*
			Save players
		*/
		infostream<<"Server: Saving players"<<std::endl;
		m_env.serializePlayers();

		/*
			Save environment metadata
		*/
		infostream<<"Server: Saving environment metadata"<<std::endl;
		m_env.saveMeta();
	}

	/*
		Stop threads
	*/
	stop();

	/*
		Delete clients
	*/
	{
		JMutexAutoLock clientslock(m_con_mutex);

		for(core::map<u16, RemoteClient*>::Iterator
			i = m_clients.getIterator();
			i.atEnd() == false; i++)
		{
			/*// Delete player
			// NOTE: These are removed by env destructor
			{
				u16 peer_id = i.getNode()->getKey();
				JMutexAutoLock envlock(m_env_mutex);
				m_env.removePlayer(peer_id);
			}*/

			// Delete client
			delete i.getNode()->getValue();
		}
	}

	ban_exit();
	auth_exit();
}

void Server::start()
{
	DSTACK(__FUNCTION_NAME);
	// Stop thread if already running
	m_thread.stop();

	uint16_t port = config_get_int("world.server.port");
	if (!port)
		port = 30000;

	// Initialize connection
	m_con.SetTimeoutMs(30);
	m_con.Serve(port);
	if (!m_con.getRun())
		return;

	// Start thread
	m_thread.setRun(true);
	m_thread.Start();

	// Announce game server to api server
	if (config_get_bool("world.server.api.announce")) {
		std::string url("/announce");
		std::string post("");
		std::string pre("");
		std::string sn = config_get("world.server.name");
		if (sn != "") {
			post += pre+"server_name="+http_url_encode(sn);
			pre = "&";
		}
		std::string sa = config_get("world.server.address");
		if (sa != "") {
			post += pre+"server_address="+http_url_encode(sa);
			pre = "&";
		}
		std::string sp = config_get("world.server.port");
		if (sp != "") {
			post += pre+"server_port="+http_url_encode(sp);
			pre = "&";
		}

		std::string response("");
		if (post == "") {
			response = http_request(NULL,(char*)url.c_str());
		}else{
			response = http_request(NULL,(char*)url.c_str(),(char*)post.c_str());
		}

		if (response == "" || response == "Server Not Found") {
			errorstream<<"Server: Failed to announce to api server"<<std::endl;
		}else{
			if (sn == "")
				config_set("world.server.name",const_cast<char*>(response.c_str()));
			if (sa == "")
				config_set("world.server.address",const_cast<char*>(response.c_str()));
			infostream<<"Server: Announced to api server"<<std::endl;
		}
	}

	infostream<<"Server: Started on port "<<port<<std::endl;
}

void Server::stop()
{
	DSTACK(__FUNCTION_NAME);

	infostream<<"Server: Stopping and waiting threads"<<std::endl;

	// Stop threads (set run=false first so both start stopping)
	m_thread.setRun(false);
	m_emergethread.setRun(false);
	m_thread.stop();
	m_emergethread.stop();

	infostream<<"Server: Threads stopped"<<std::endl;
}

bool Server::step(float dtime)
{
	DSTACK(__FUNCTION_NAME);
	if (!m_con.getRun())
		return false;
	// Limit a bit
	if(dtime > 2.0)
		dtime = 2.0;
	{
		JMutexAutoLock lock(m_step_dtime_mutex);
		m_step_dtime += dtime;
	}
	return true;
}

void Server::AsyncRunStep()
{
	DSTACK(__FUNCTION_NAME);

	g_profiler->add("Server::AsyncRunStep (num)", 1);

	float dtime;
	{
		JMutexAutoLock lock1(m_step_dtime_mutex);
		dtime = m_step_dtime;
	}

	{
		ScopeProfiler sp(g_profiler, "Server: sel and send blocks to clients");
		// Send blocks to clients
		SendBlocks(dtime);
	}

	if(dtime < 0.001)
		return;

	g_profiler->add("Server::AsyncRunStep with dtime (num)", 1);

	//infostream<<"Server steps "<<dtime<<std::endl;
	//infostream<<"Server::AsyncRunStep(): dtime="<<dtime<<std::endl;

	{
		JMutexAutoLock lock1(m_step_dtime_mutex);
		m_step_dtime -= dtime;
	}

	/*
		Update uptime
	*/
	{
		m_uptime.set(m_uptime.get() + dtime);
	}

	{
		// Process connection's timeouts
		JMutexAutoLock lock2(m_con_mutex);
		ScopeProfiler sp(g_profiler, "Server: connection timeout processing");
		m_con.RunTimeouts(dtime);
	}

	{
		// This has to be called so that the client list gets synced
		// with the peer list of the connection
		handlePeerChanges();
	}

	/*
		Update time of day and overall game time
	*/
	{
		JMutexAutoLock envlock(m_env_mutex);

		float time_speed = config_get_float("world.game.environment.time.speed");

		m_env.setTimeOfDaySpeed(time_speed);

		/*
			Send to clients at constant intervals
		*/

		m_time_of_day_send_timer -= dtime;
		if (m_time_of_day_send_timer < 0.0) {
			m_time_of_day_send_timer = config_get_float("server.net.client.time.interval");

			//JMutexAutoLock envlock(m_env_mutex);
			JMutexAutoLock conlock(m_con_mutex);

			for (core::map<u16, RemoteClient*>::Iterator i = m_clients.getIterator(); i.atEnd() == false; i++) {
				RemoteClient *client = i.getNode()->getValue();
				//Player *player = m_env.getPlayer(client->peer_id);

				SharedBuffer<u8> data = makePacket_TOCLIENT_TIME_OF_DAY(m_env.getTimeOfDay(),time_speed, m_env.getTime());
				// Send as reliable
				m_con.Send(client->peer_id, 0, data, true);
			}
		}
	}

	{
		JMutexAutoLock lock(m_env_mutex);
		// Step environment
		ScopeProfiler sp(g_profiler, "SEnv step");
		ScopeProfiler sp2(g_profiler, "SEnv step avg", SPT_AVG);
		m_env.step(dtime);
	}

	const float map_timer_and_unload_dtime = 2.92;
	if (m_map_timer_and_unload_interval.step(dtime, map_timer_and_unload_dtime)) {
		JMutexAutoLock lock(m_env_mutex);
		// Run Map's timers and unload unused data
		ScopeProfiler sp(g_profiler, "Server: map timer and unload");
		m_env.getMap().timerUpdate(map_timer_and_unload_dtime,config_get_float("server.chunk.timeout"));
	}

	/*
		Do background stuff
	*/

	/*
		Transform liquids
	*/
	m_liquid_transform_timer += dtime;
	if(m_liquid_transform_timer >= 1.00)
	{
		m_liquid_transform_timer -= 1.00;

		JMutexAutoLock lock(m_env_mutex);

		ScopeProfiler sp(g_profiler, "Server: liquid transform");

		core::map<v3s16, MapBlock*> modified_blocks;
		m_env.getMap().transformLiquids(modified_blocks);
		/*
			Set the modified blocks unsent for all the clients
		*/

		JMutexAutoLock lock2(m_con_mutex);

		for(core::map<u16, RemoteClient*>::Iterator
				i = m_clients.getIterator();
				i.atEnd() == false; i++)
		{
			RemoteClient *client = i.getNode()->getValue();

			if(modified_blocks.size() > 0)
			{
				// Remove block from sent history
				client->SetBlocksNotSent(modified_blocks);
			}
		}
	}

	/*
		Check added and deleted active objects
	*/
	{
		//infostream<<"Server: Checking added and deleted active objects"<<std::endl;
		JMutexAutoLock envlock(m_env_mutex);
		JMutexAutoLock conlock(m_con_mutex);

		ScopeProfiler sp(g_profiler, "Server: checking added and deleted objs");

		// Radius inside which objects are active

		int16_t radius = config_get_int("world.server.mob.range");
		radius *= MAP_BLOCKSIZE;
		bool send_inventory = false;
		m_send_full_inventory_timer += dtime;
		if (m_send_full_inventory_timer >= 60.0) {
			m_send_full_inventory_timer -= 60.0;
			send_inventory = true;
		}

		for (core::map<u16, RemoteClient*>::Iterator i = m_clients.getIterator(); i.atEnd() == false; i++) {
			RemoteClient *client = i.getNode()->getValue();
			Player *player = m_env.getPlayer(client->peer_id);
			if (player == NULL) {
				// This can happen if the client timeouts somehow
				continue;
			}
			v3s16 pos = floatToInt(player->getPosition(), BS);

			std::map<u16, bool> removed_objects;
			std::map<u16, bool> added_objects;
			m_env.getRemovedActiveObjects(pos, radius, client->m_known_objects, removed_objects);
			m_env.getAddedActiveObjects(pos, radius, client->m_known_objects, added_objects);

			if (send_inventory || player->inventory_modified) {
				player->inventory_modified = false;
				SendInventory(client->peer_id,true);
			}

			// Ignore if nothing happened
			if (removed_objects.size() == 0 && added_objects.size() == 0) {
				//infostream<<"active objects: none changed"<<std::endl;
				continue;
			}

			std::string data_buffer;
			char buf[4];

			// Handle removed objects
			writeU16((u8*)buf, removed_objects.size());
			data_buffer.append(buf, 2);
			for (std::map<u16, bool>::iterator i = removed_objects.begin(); i != removed_objects.end(); i++) {
				// Get object
				u16 id = i->first;
				ServerActiveObject* obj = m_env.getActiveObject(id);

				// Add to data buffer for sending
				writeU16((u8*)buf, id);
				data_buffer.append(buf, 2);

				// Remove from known objects
				std::map<u16, bool>::iterator c = client->m_known_objects.find(id);
				client->m_known_objects.erase(c);

				if (obj && obj->m_known_by_count > 0) {
					obj->m_known_by_count--;
					if (obj->m_known_by_count < 1 && obj->m_pending_deactivation == true)
						obj->m_pending_deactivation = false;
				}
			}

			// Handle added objects
			writeU16((u8*)buf, added_objects.size());
			data_buffer.append(buf, 2);
			for (std::map<u16, bool>::iterator i = added_objects.begin(); i != added_objects.end(); i++) {
				// Get object
				u16 id = i->first;
				ServerActiveObject* obj = m_env.getActiveObject(id);

				// Get object type
				u8 type = ACTIVEOBJECT_TYPE_INVALID;
				if (obj == NULL) {
					infostream<<"WARNING: "<<__FUNCTION_NAME
							<<": NULL object"<<std::endl;
				}else{
					type = obj->getType();
				}

				// Add to data buffer for sending
				writeU16((u8*)buf, id);
				data_buffer.append(buf, 2);
				writeU8((u8*)buf, type);
				data_buffer.append(buf, 1);

				if (obj) {
					data_buffer.append(serializeLongString(obj->getClientInitializationData()));
				}else{
					data_buffer.append(serializeLongString(""));
				}
				// Add to known objects
				client->m_known_objects[id] = true;

				if (obj)
					obj->m_known_by_count++;
			}

			// Send packet
			SharedBuffer<u8> reply(2 + data_buffer.size());
			writeU16(&reply[0], TOCLIENT_ACTIVE_OBJECT_REMOVE_ADD);
			memcpy((char*)&reply[2], data_buffer.c_str(), data_buffer.size());
			// Send as reliable
			m_con.Send(client->peer_id, 0, reply, true);

			infostream<<"Server: Sent object remove/add: "
					<<removed_objects.size()<<" removed, "
					<<added_objects.size()<<" added, "
					<<"packet size is "<<reply.getSize()<<std::endl;
		}
	}

	m_send_object_info_timer += dtime;
	if (m_send_object_info_timer > 0.2) {
		m_send_object_info_timer -= 0.2;

		// Send object messages

		JMutexAutoLock envlock(m_env_mutex);
		JMutexAutoLock conlock(m_con_mutex);

		//ScopeProfiler sp(g_profiler, "Server: sending object messages");

		// Key = object id
		// Value = data sent by object
		core::map<u16, core::list<ActiveObjectMessage>* > buffered_messages;

		// Get active object messages from environment
		for (;;) {
			ActiveObjectMessage aom = m_env.getActiveObjectMessage();
			if (aom.id == 0)
				break;

			core::list<ActiveObjectMessage>* message_list = NULL;
			core::map<u16, core::list<ActiveObjectMessage>* >::Node *n;
			n = buffered_messages.find(aom.id);
			if (n == NULL) {
				message_list = new core::list<ActiveObjectMessage>;
				buffered_messages.insert(aom.id, message_list);
			}else{
				message_list = n->getValue();
			}
			/* if this is a position send, replace any previous position data,
			 * sending multiple position data messages in one packet is silly
			 */
			if (aom.datastring[0] == 0) {
				bool was_replaced = false;
				for (core::list<ActiveObjectMessage>::Iterator k = message_list->begin(); k != message_list->end(); k++) {
					// Compose the full new data with header
					ActiveObjectMessage &oaom = *k;
					if (oaom.datastring[0] == 0) {
						oaom.datastring = aom.datastring;
						was_replaced = true;
						break;
					}
				}
				if (was_replaced)
					continue;
			}
			message_list->push_back(aom);
		}

		// Route data to every client
		for(core::map<u16, RemoteClient*>::Iterator
			i = m_clients.getIterator();
			i.atEnd()==false; i++)
		{
			RemoteClient *client = i.getNode()->getValue();
			std::string reliable_data;
			std::string unreliable_data;
			// Go through all objects in message buffer
			for(core::map<u16, core::list<ActiveObjectMessage>* >::Iterator
					j = buffered_messages.getIterator();
					j.atEnd()==false; j++)
			{
				// If object is not known by client, skip it
				u16 id = j.getNode()->getKey();
				if (client->m_known_objects.find(id) == client->m_known_objects.end())
					continue;
				// Get message list of object
				core::list<ActiveObjectMessage>* list = j.getNode()->getValue();
				// Go through every message
				for(core::list<ActiveObjectMessage>::Iterator
						k = list->begin(); k != list->end(); k++)
				{
					// Compose the full new data with header
					ActiveObjectMessage aom = *k;
					std::string new_data;
					// Add object id
					char buf[2];
					writeU16((u8*)&buf[0], aom.id);
					new_data.append(buf, 2);
					// Add data
					new_data += serializeString(aom.datastring);
					// Add data to buffer
					if(aom.reliable)
						reliable_data += new_data;
					else
						unreliable_data += new_data;
				}
			}
			/*
				reliable_data and unreliable_data are now ready.
				Send them.
			*/
			if(reliable_data.size() > 0)
			{
				SharedBuffer<u8> reply(2 + reliable_data.size());
				writeU16(&reply[0], TOCLIENT_ACTIVE_OBJECT_MESSAGES);
				memcpy((char*)&reply[2], reliable_data.c_str(),
						reliable_data.size());
				// Send as reliable
				m_con.Send(client->peer_id, 0, reply, true);
			}
			if(unreliable_data.size() > 0)
			{
				SharedBuffer<u8> reply(2 + unreliable_data.size());
				writeU16(&reply[0], TOCLIENT_ACTIVE_OBJECT_MESSAGES);
				memcpy((char*)&reply[2], unreliable_data.c_str(),
						unreliable_data.size());
				// Send as unreliable
				m_con.Send(client->peer_id, 0, reply, false);
			}
		}

		// Clear buffered_messages
		for(core::map<u16, core::list<ActiveObjectMessage>* >::Iterator
				i = buffered_messages.getIterator();
				i.atEnd()==false; i++)
		{
			delete i.getNode()->getValue();
		}

		// Get env_events from environment and send
		for (;;) {
			EnvEvent ev = m_env.getEnvEvent();
			if (ev.type == ENV_EVENT_NONE)
				break;
			SendEnvEvent(ev.type,ev.pos,ev.data,ev.except_player);
		}
	}

	/*
		Send queued-for-sending map edit events.
	*/
	{
		// Don't send too many at a time
		//u32 count = 0;
		JMutexAutoLock envlock(m_env_mutex);
		JMutexAutoLock conlock(m_con_mutex);

		// Single change sending is disabled if queue size is not small
		bool disable_single_change_sending = false;
		if(m_unsent_map_edit_queue.size() >= 4)
			disable_single_change_sending = true;

		bool got_any_events = false;

		// We'll log the amount of each
		Profiler prof;

		while(m_unsent_map_edit_queue.size() != 0)
		{
			got_any_events = true;

			MapEditEvent* event = m_unsent_map_edit_queue.pop_front();

			// Players far away from the change are stored here.
			// Instead of sending the changes, MapBlocks are set not sent
			// for them.
			core::list<u16> far_players;

			if(event->type == MEET_ADDNODE)
			{
				//infostream<<"Server: MEET_ADDNODE"<<std::endl;
				prof.add("MEET_ADDNODE", 1);
				if(disable_single_change_sending)
					sendAddNode(event->p, event->n, event->already_known_by_peer,
							&far_players, 5);
				else
					sendAddNode(event->p, event->n, event->already_known_by_peer,
							&far_players, 30);
			}
			else if(event->type == MEET_REMOVENODE)
			{
				//infostream<<"Server: MEET_REMOVENODE"<<std::endl;
				prof.add("MEET_REMOVENODE", 1);
				if(disable_single_change_sending)
					sendRemoveNode(event->p, event->already_known_by_peer,
							&far_players, 5);
				else
					sendRemoveNode(event->p, event->already_known_by_peer,
							&far_players, 30);
			}
			else if(event->type == MEET_BLOCK_NODE_METADATA_CHANGED)
			{
				infostream<<"Server: MEET_BLOCK_NODE_METADATA_CHANGED"<<std::endl;
				prof.add("MEET_BLOCK_NODE_METADATA_CHANGED", 1);
				setBlockNotSent(event->p);
			}
			else if(event->type == MEET_OTHER)
			{
				infostream<<"Server: MEET_OTHER"<<std::endl;
				prof.add("MEET_OTHER", 1);
				for(core::map<v3s16, bool>::Iterator
						i = event->modified_blocks.getIterator();
						i.atEnd()==false; i++)
				{
					v3s16 p = i.getNode()->getKey();
					setBlockNotSent(p);
				}
			}
			else
			{
				prof.add("unknown", 1);
				infostream<<"WARNING: Server: Unknown MapEditEvent "
						<<((u32)event->type)<<std::endl;
			}

			/*
				Set blocks not sent to far players
			*/
			if(far_players.size() > 0)
			{
				// Convert list format to that wanted by SetBlocksNotSent
				core::map<v3s16, MapBlock*> modified_blocks2;
				for(core::map<v3s16, bool>::Iterator
						i = event->modified_blocks.getIterator();
						i.atEnd()==false; i++)
				{
					v3s16 p = i.getNode()->getKey();
					MapBlock* const block = m_env.getMap().getBlockNoCreateNoEx(p);

					if(block)
					{
					    modified_blocks2.insert(p,block);
					    block->ResetCurrent();
					}
				}
				// Set blocks not sent
				for(core::list<u16>::Iterator
						i = far_players.begin();
						i != far_players.end(); i++)
				{
					u16 peer_id = *i;
					RemoteClient *client = getClient(peer_id);
					if(client==NULL)
						continue;
					client->SetBlocksNotSent(modified_blocks2);
				}
			}

			delete event;

			/*// Don't send too many at a time
			count++;
			if(count >= 1 && m_unsent_map_edit_queue.size() < 100)
				break;*/
		}

		if (got_any_events) {
			infostream<<"Server: MapEditEvents:"<<std::endl;
			prof.print(infostream);
		}

	}

	/*
		Send object positions
	*/
	{
		float &counter = m_objectdata_timer;
		counter += dtime;
		if (counter >= config_get_float("server.net.client.object.interval")) {
			//ScopeProfiler sp(g_profiler, "Server: sending player positions");

			SendPlayerInfo(counter);
			counter = 0.0;
		}
	}

	/*
		Trigger emergethread (it somehow gets to a non-triggered but
		busy state sometimes)
	*/
	{
		float &counter = m_emergethread_trigger_timer;
		counter += dtime;
		if (counter >= 2.0) {
			counter = 0.0;

			m_emergethread.trigger();
		}
	}

	// Save map, players and auth stuff
	{
		float &counter = m_savemap_timer;
		counter += dtime;
		if (counter >= config_get_float("server.save.interval")) {
			counter = 0.0;

			ScopeProfiler sp(g_profiler, "Server: saving stuff");

			// Auth stuff
			auth_save();

			// Ban stuff
			ban_save();

			// Map
			JMutexAutoLock lock(m_env_mutex);

			// Save only changed parts
			m_env.getMap().save(true);

			// Save players
			m_env.serializePlayers();

			// Save environment metadata
			m_env.saveMeta();

			config_save("world","world","world.cfg");
		}
	}
}

void Server::Receive()
{
	DSTACK(__FUNCTION_NAME);
	SharedBuffer<u8> data;
	u16 peer_id;
	u32 datasize;
	try{
		{
			JMutexAutoLock conlock(m_con_mutex);
			datasize = m_con.Receive(peer_id, data);
		}

		// This has to be called so that the client list gets synced
		// with the peer list of the connection
		handlePeerChanges();

		ProcessData(*data, datasize, peer_id);
	}
	catch(con::InvalidIncomingDataException &e)
	{
		infostream<<"Server::Receive(): "
				"InvalidIncomingDataException: what()="
				<<e.what()<<std::endl;
	}
	catch(con::PeerNotFoundException &e)
	{
		//NOTE: This is not needed anymore

		// The peer has been disconnected.
		// Find the associated player and remove it.

		/*JMutexAutoLock envlock(m_env_mutex);

		infostream<<"ServerThread: peer_id="<<peer_id
				<<" has apparently closed connection. "
				<<"Removing player."<<std::endl;

		m_env.removePlayer(peer_id);*/
	}
}

void Server::ProcessData(u8 *data, u32 datasize, u16 peer_id)
{
	DSTACK(__FUNCTION_NAME);
	// Environment is locked first.
	JMutexAutoLock envlock(m_env_mutex);
	JMutexAutoLock conlock(m_con_mutex);

	try{
		Address address = m_con.GetPeerAddress(peer_id);

		// drop player if is ip is banned
		std::string add = address.serializeString();
		if (ban_ipbanned(const_cast<char*>(add.c_str()))) {
			char* name = ban_ip2name(const_cast<char*>(add.c_str()));
			if (!name)
				name = (char*)"???";
			SendAccessDenied(m_con, peer_id,
					L"Your ip is banned. Banned name was "
					+narrow_to_wide(name));
			m_con.DeletePeer(peer_id);
			return;
		}
	}
	catch(con::PeerNotFoundException &e)
	{
		infostream<<"Server::ProcessData(): Cancelling: peer "
				<<peer_id<<" not found"<<std::endl;
		return;
	}

	u8 peer_ser_ver = getClient(peer_id)->serialization_version;

	try
	{

	if(datasize < 2)
		return;

	ToServerCommand command = (ToServerCommand)readU16(&data[0]);

	if (command == TOSERVER_INIT) {
		// [0] u16 TOSERVER_INIT
		// [2] u8 SER_FMT_VER_HIGHEST
		// [3] u8[20] player_name
		// [23] u8[28] password <--- can be sent without this, from old versions

		if (datasize < 2+1+PLAYERNAME_SIZE)
			return;

		infostream<<"Server: Got TOSERVER_INIT from "<<peer_id<<std::endl;

		// First byte after command is maximum supported
		// serialization version
		u8 client_max = data[2];
		u8 our_max = SER_FMT_VER_HIGHEST;
		// Use the highest version supported by both
		u8 deployed = core::min_(client_max, our_max);
		// If it's lower than the lowest supported, give up.
		if (deployed < SER_FMT_VER_LOWEST)
			deployed = SER_FMT_VER_INVALID;

		//peer->serialization_version = deployed;
		getClient(peer_id)->pending_serialization_version = deployed;

		if (deployed == SER_FMT_VER_INVALID) {
			infostream<<"Server: Cannot negotiate "
					"serialization version with peer "
					<<peer_id<<std::endl;
			SendAccessDenied(m_con, peer_id, L"Your client is too old (map format)");
			return;
		}

		/*
			Read and check network protocol version
		*/

		u16 net_proto_version = 0;
		if (datasize >= 2+1+PLAYERNAME_SIZE+PASSWORD_SIZE+2)
			net_proto_version = readU16(&data[2+1+PLAYERNAME_SIZE+PASSWORD_SIZE]);

		getClient(peer_id)->net_proto_version = net_proto_version;

		if (net_proto_version < PROTOCOL_OLDEST) {
			SendAccessDenied(m_con, peer_id, L"Your client is too old. Please upgrade.");
			return;
		}

		/* Uhh... this should actually be a warning but let's do it like this */
		if (config_get_bool("world.server.client.version.strict")) {
			if (net_proto_version < PROTOCOL_VERSION) {
				SendAccessDenied(m_con, peer_id, L"Your client is too old. Please upgrade.");
				return;
			}
		}

		/*
			Set up player
		*/

		// Get player name
		char playername[PLAYERNAME_SIZE];
		for (u32 i=0; i<PLAYERNAME_SIZE-1; i++) {
			playername[i] = data[3+i];
		}
		playername[PLAYERNAME_SIZE-1] = 0;

		if (playername[0]=='\0') {
			infostream<<"Server: Player has empty name"<<std::endl;
			SendAccessDenied(m_con, peer_id, L"Empty name");
			return;
		}

		if (string_allowed(playername, PLAYERNAME_ALLOWED_CHARS)==false) {
			infostream<<"Server: Player has invalid name"<<std::endl;
			SendAccessDenied(m_con, peer_id, L"Name contains unallowed characters");
			return;
		}

		/* these people piss me off, so let's piss them off */
		if (!strncmp(playername,"player",6) && strlen(playername) > 6) {
			SendAccessDenied(m_con, peer_id, L"Your client is too old. Please upgrade.");
			return;
		}

		// Get password
		char password[PASSWORD_SIZE];
		if (datasize < 2+1+PLAYERNAME_SIZE+PASSWORD_SIZE) {
			// old version - assume blank password
			password[0] = 0;
		}else{
				for (u32 i=0; i<PASSWORD_SIZE-1; i++) {
					password[i] = data[23+i];
				}
				password[PASSWORD_SIZE-1] = 0;
		}

		if (!base64_is_valid(password)) {
			infostream<<"Server: "<<playername<<" supplied invalid password hash"<<std::endl;
			SendAccessDenied(m_con, peer_id, L"Invalid password hash");
			return;
		}

		if (password[0] == '\0' && config_get_bool("world.server.client.emptypwd")) {
			infostream<<"Server: "<<playername<<" supplied no password"<<std::endl;
			SendAccessDenied(m_con, peer_id, L"Empty passwords are not allowed on this server.");
			return;
		}

		if (config_get_bool("world.server.client.private") && !auth_exists(playername)) {
			infostream<<"Server: unknown player "<<playername
				<<" was blocked"<<std::endl;
			SendAccessDenied(m_con, peer_id, L"No unknown players allowed.");
			return;
		}

		char checkpwd[64];
		if (auth_exists(playername)) {
			if (auth_getpwd(playername,checkpwd))
				checkpwd[0] = 0;
		}else{
			const char* default_pwd = config_get("world.server.client.default.password");
			if (default_pwd && default_pwd[0]) {
				std::string defaultpwd = translatePassword(playername,narrow_to_wide(defaultpwd));
				strcpy(checkpwd,defaultpwd.c_str());
			}else{
				strcpy(checkpwd,password);
			}
		}

		/*infostream<<"Server: Client gave password '"<<password
				<<"', the correct one is '"<<checkpwd<<"'"<<std::endl;*/

		if (strcmp(password,checkpwd)) {
			infostream<<"Server: peer_id="<<peer_id
					<<": supplied invalid password for "
					<<playername<<std::endl;
			SendAccessDenied(m_con, peer_id, L"Invalid password");
			return;
		}

		// Add player to auth manager
		if (!auth_exists(playername)) {
			infostream<<"Server: adding player "<<playername<<" to auth manager"<<std::endl;
			uint64_t privs = 0;
			const char* priv = config_get("world.server.client.default.privs");
			if (priv)
				privs = auth_str2privs(priv);

			auth_add(playername);
			auth_setpwd(playername,checkpwd);
			auth_setprivs(playername,privs);
			auth_save();
		}

		// Enforce user limit.
		// Don't enforce for users that have some admin right
		const char* admin_name = config_get("world.server.admin");
		if (
			m_clients.size() >= (uint16_t)config_get_int("world.server.client.max")
			&& (auth_getprivs(playername) & (PRIV_SERVER|PRIV_BAN|PRIV_PRIVS)) == 0
			&& (!admin_name || strcmp(admin_name,playername))
		) {
			SendAccessDenied(m_con, peer_id, L"Too many users.");
			return;
		}

		// Get player
		Player *player = emergePlayer(playername, password, peer_id);

		// If failed, cancel
		if (player == NULL) {
			infostream<<"Server: peer_id="<<peer_id<<": failed to emerge player"<<std::endl;
			return;
		}

		{
			Address address = getPeerAddress(peer_id);
			std::string ip_string = address.serializeString();
			((ServerRemotePlayer*)player)->setAddress(ip_string);
		}

		/*
			Answer with a TOCLIENT_INIT
		*/
		{
			SharedBuffer<u8> reply(2+1+6+8+2);
			writeU16(&reply[0], TOCLIENT_INIT);
			writeU8(&reply[2], deployed);
			writeV3S16(&reply[2+1], floatToInt(player->getPosition()+v3f(0,BS/2,0), BS));
			writeU64(&reply[2+1+6], m_env.getServerMap().getSeed());
			writeU16(&reply[2+1+6+8], (u16)m_env.getServerMap().getType());

			// Send as reliable
			m_con.Send(peer_id, 0, reply, true);
		}

		/*
			Send complete position information
		*/
		SendMovePlayer(player);
		SendSettings(player);

		return;
	}

	if (command == TOSERVER_INIT2) {
		infostream<<"Server: Got TOSERVER_INIT2 from "<<peer_id<<std::endl;


		getClient(peer_id)->serialization_version = getClient(peer_id)->pending_serialization_version;
		std::string chardef = PLAYER_DEFAULT_CHARDEF;

		if (datasize > 2) {
			std::string datastring((char*)&data[2], datasize-2);
			std::istringstream is(datastring, std::ios_base::binary);
			// Read character definition
			chardef = deSerializeString(is);
		}

		Player *player = m_env.getPlayer(peer_id);
		// set the player's character definition
		player->setCharDef(chardef);

		player->updateAnim(PLAYERANIM_STAND,CONTENT_AIR);

		/*
			Send some initialization data
		*/

		// Send player info to all players
		SendPlayerData();

		// Send inventory to player
		UpdateCrafting(peer_id);
		SendInventory(peer_id,true);

		// Send player items to all players
		SendPlayerItems();

		// Send HP
		SendPlayerState(player);

		// Send time of day
		{
			SharedBuffer<u8> data = makePacket_TOCLIENT_TIME_OF_DAY(
				m_env.getTimeOfDay(),
				config_get_float("world.game.environment.time.speed"),
				m_env.getTime()
			);
			m_con.Send(peer_id, 0, data, true);
		}

		// Send information about server to player in chat
		SendChatMessage(peer_id, getStatusString());

		// Send information about joining in chat
		{
			std::wstring name = L"unknown";
			Player *player = m_env.getPlayer(peer_id);
			if (player != NULL)
				name = narrow_to_wide(player->getName());

			std::wstring message;
			message += L"*** ";
			message += name;
			message += L" joined game";
			BroadcastChatMessage(message);
		}

		// Warnings about protocol version can be issued here
		if (getClient(peer_id)->net_proto_version < PROTOCOL_VERSION) {
			SendChatMessage(peer_id, L"# Server: WARNING: YOUR CLIENT IS OLD AND MAY NOT WORK PROPERLY WITH THIS SERVER");
			if (getClient(peer_id)->net_proto_version == PROTOCOL_DOTTHREE) {
				SendChatMessage(peer_id, L"# Server: Please update to Voxelands http://www.voxelands.com");
			}else{
				SendChatMessage(peer_id, L"# Server: The latest client can be downloaded from http://www.voxelands.com/download.html");
			}
		}

		/*
			Check HP, respawn if necessary
		*/
		HandlePlayerHP(player, 0, 0, 0);

		/*
			Print out action
		*/
		{
			std::ostringstream os(std::ios_base::binary);
			for (core::map<u16, RemoteClient*>::Iterator i = m_clients.getIterator(); i.atEnd() == false; i++) {
				RemoteClient *client = i.getNode()->getValue();
				assert(client->peer_id == i.getNode()->getKey());
				if (client->serialization_version == SER_FMT_VER_INVALID)
					continue;
				// Get player
				Player *player = m_env.getPlayer(client->peer_id);
				if (!player)
					continue;
				// Get name of player
				os<<player->getName()<<" ";
			}

			actionstream<<player->getName()<<" joins game. List of players: "
					<<os.str()<<std::endl;
		}

		return;
	}

	if (peer_ser_ver == SER_FMT_VER_INVALID) {
		infostream<<"Server::ProcessData(): Cancelling: Peer"
				" serialization format invalid or not initialized."
				" Skipping incoming command="<<command<<std::endl;
		return;
	}

	Player *player = m_env.getPlayer(peer_id);

	if (player == NULL) {
		infostream<<"Server::ProcessData(): Cancelling: "
				"No player for peer_id="<<peer_id
				<<std::endl;
		return;
	}
	switch (command) {
	case TOSERVER_THROWITEM:
	{
		if (datasize < 2+12+12+2)
			return;
		v3s32 ps = readV3S32(&data[2]);
		v3s32 ss = readV3S32(&data[2+12]);

		u16 item_i = readU16(&data[2+12+12]);

		if ((getPlayerPrivs(player) & PRIV_BUILD) == 0)
			return;

		ToolItem *titem = NULL;
		InventoryList* const ilist = player->inventory.getList("main");
		if (ilist == NULL)
			return;

		// Get item
		InventoryItem *item = ilist->getItem(item_i);

		// If there is no item, it is not possible to throw it
		if (item == NULL)
			return;

		content_t thrown = content_craftitem_features(item->getContent())->thrown_item;
		// We can throw it, right?
		if (thrown == CONTENT_IGNORE) {
			// it may be a tool that throws something else
			thrown = content_toolitem_features(item->getContent()).thrown_item;
			if (thrown == CONTENT_IGNORE)
				return;
			titem = (ToolItem*)item;
			u16 i;
			item = ilist->findItem(thrown,&i);
			if (!item)
				return;
			// We can throw it, right?
			thrown = content_craftitem_features(item->getContent())->shot_item;
			if (thrown == CONTENT_IGNORE)
				return;
			if (config_get_bool("world.player.tool.wear")) {
				bool weared_out = titem->addWear(1);
				if (weared_out) {
					InventoryList *mlist = player->inventory.getList("main");
					mlist->deleteItem(item_i);
				}else{
					ilist->addDiff(item_i,titem);
				}
				SendInventory(player->peer_id);
			}
			item_i = i;
		}

		if (!config_get_bool("world.player.inventory.droppable")) {
			InventoryList *mlist = player->inventory.getList("main");
			mlist->deleteItem(item_i);
		}else if ((getPlayerPrivs(player) & PRIV_BUILD) == 0) {
			infostream<<"Not allowing player to drop item: no build privs"<<std::endl;
			return;
		}

		v3f pf((f32)ps.X/100., (f32)ps.Y/100., (f32)ps.Z/100.);
		v3f sf((f32)ss.X/100., (f32)ss.Y/100., (f32)ss.Z/100.);

		ServerActiveObject* const obj = new MobSAO(&m_env, 0, pf, sf*content_mob_features(thrown).static_thrown_speed*BS, thrown);

		if (obj == NULL) {
			infostream<<"WARNING: item resulted in NULL object, "
							<<"not throwing into map"
							<<std::endl;
		}else{
			actionstream<<player->getName()<<" throws "<<thrown<<" at "<<PP(pf/BS)<<" ("<<PP(sf/BS)")"<<std::endl;

			// Add the object to the environment
			m_env.addActiveObject(obj);
		}
		if (!config_get_bool("world.player.inventory.creative")) {
			// Delete the right amount of items from the slot

			// Delete item if all gone
			if (item->getCount() <= 1) {
				if (item->getCount() < 1)
					infostream<<"WARNING: Server: dropped more items"
							<<" than the slot contains"<<std::endl;

				InventoryList* const ilist = player->inventory.getList("main");
				if (ilist)
					// Remove from inventory and send inventory
					ilist->deleteItem(item_i);
			}else{
				item->remove(1);
			}

			// Send inventory
			UpdateCrafting(peer_id);
			SendInventory(peer_id);
		}
	}
	break;
	case TOSERVER_USEITEM:
	{
		InventoryList* const ilist = player->inventory.getList("main");
		if (ilist == NULL)
			return;

		const u16 item_i = player->getSelectedItem();

		// Get item
		InventoryItem* const item = ilist->getItem(item_i);
		if (item == NULL)
			return;

		// Track changes super-crappily
		u16 oldhp = player->health;
		u16 oldair = player->air;
		u16 oldhunger = player->hunger;

		if (item->use(&m_env,player)) {
			ilist->deleteItem(item_i);
		}else{
			ilist->addDiff(item_i,item);
		}

		// Send inventory
		UpdateCrafting(peer_id);
		SendInventory(peer_id);

		// Send back stuff
		if (player->health != oldhp || player->air != oldair || player->hunger != oldhunger || player->energy_effect || player->cold_effect)
			SendPlayerState(player);
	}
	break;
	case TOSERVER_PLAYERPOS:
	{
		if (datasize < 2+12+12+4+4)
			return;

		u32 start = 0;
		v3s32 ps = readV3S32(&data[start+2]);
		v3s32 ss = readV3S32(&data[start+2+12]);
		f32 pitch = (f32)readS32(&data[2+12+12]) / 100.0;
		f32 yaw = (f32)readS32(&data[2+12+12+4]) / 100.0;
		v3f position((f32)ps.X/100., (f32)ps.Y/100., (f32)ps.Z/100.);
		v3f speed((f32)ss.X/100., (f32)ss.Y/100., (f32)ss.Z/100.);
		pitch = wrapDegrees(pitch);
		yaw = wrapDegrees(yaw);

		player->setPosition(position);
		player->setSpeed(speed);
		player->setPitch(pitch);
		player->setYaw(yaw);

		/*infostream<<"Server::ProcessData(): Moved player "<<peer_id<<" to "
				<<"("<<position.X<<","<<position.Y<<","<<position.Z<<")"
				<<" pitch="<<pitch<<" yaw="<<yaw<<std::endl;*/
	}
	break;
	case TOSERVER_GOTBLOCKS:
	{
		if(datasize < 2+1)
			return;

		/*
			[0] u16 command
			[2] u8 count
			[3] v3s16 pos_0
			[3+6] v3s16 pos_1
			...
		*/

		u16 count = data[2];
		for(u16 i=0; i<count; i++)
		{
			if((s16)datasize < 2+1+(i+1)*6)
				throw con::InvalidIncomingDataException
					("GOTBLOCKS length is too short");
			v3s16 p = readV3S16(&data[2+1+i*6]);
			/*infostream<<"Server: GOTBLOCKS ("
					<<p.X<<","<<p.Y<<","<<p.Z<<")"<<std::endl;*/
			RemoteClient *client = getClient(peer_id);
			client->GotBlock(p);
		}
	}
	break;
	case TOSERVER_DELETEDBLOCKS:
	{
		if(datasize < 2+1)
			return;

		/*
			[0] u16 command
			[2] u8 count
			[3] v3s16 pos_0
			[3+6] v3s16 pos_1
			...
		*/

		u16 count = data[2];
		for(u16 i=0; i<count; i++)
		{
			if((s16)datasize < 2+1+(i+1)*6)
				throw con::InvalidIncomingDataException
					("DELETEDBLOCKS length is too short");
			v3s16 p = readV3S16(&data[2+1+i*6]);
			/*infostream<<"Server: DELETEDBLOCKS ("
					<<p.X<<","<<p.Y<<","<<p.Z<<")"<<std::endl;*/
			RemoteClient *client = getClient(peer_id);
			client->SetBlockNotSent(p);
		}
	}
	break;
	case TOSERVER_CLICK_ACTIVEOBJECT:
	{
		if (datasize < 7)
			return;

		if ((getPlayerPrivs(player) & PRIV_BUILD) == 0)
			return;

		/*
			length: 7
			[0] u16 command
			[2] u8 button (0=left, 1=right)
			[3] u16 id
			[5] u16 item
		*/
		u8 button = readU8(&data[2]);
		u16 id = readS16(&data[3]);
		u16 item_i = readU16(&data[5]);

		ServerActiveObject *obj = m_env.getActiveObject(id);

		if (obj == NULL) {
			infostream<<"Server: CLICK_ACTIVEOBJECT: object not found"
					<<std::endl;
			return;
		}

		// Skip if object has been removed
		if (obj->m_removed) {
			infostream<<"Server: CLICK_ACTIVEOBJECT: object has been removed"
					<<std::endl;
			return;
		}

		//TODO: Check that object is reasonably close

		// Left click, pick object up (usually)
		if (button == 0) {
			content_t wield_item = CONTENT_IGNORE;
			ToolItem *titem = NULL;

			InventoryList* const mlist = player->inventory.getList("main");
			if (mlist != NULL) {
				InventoryItem* const item = mlist->getItem(item_i);
				if (item) {
					wield_item = item->getContent();
					if ((wield_item&CONTENT_TOOLITEM_MASK) == CONTENT_TOOLITEM_MASK)
						titem = (ToolItem*)item;
				}
			}
			/*
				Try creating inventory item
			*/
			InventoryItem* item = obj->createPickedUpItem(wield_item);

			if (item) {
				InventoryList* const ilist = player->inventory.getList("main");
				if (ilist != NULL) {
					actionstream << player->getName() << " picked up "
							<< item->getName() << std::endl;
					if (!config_get_bool("world.player.inventory.creative")) {
						// Skip if inventory has no free space
						if (ilist->roomForItem(item) == false) {
							infostream<<"Player inventory has no free space"<<std::endl;
							return;
						}

						// Add to inventory and send inventory
						ilist->addItem(item);
						UpdateCrafting(player->peer_id);
						SendInventory(player->peer_id);
					}

					// Remove object from environment
					if (obj->getType() != ACTIVEOBJECT_TYPE_MOB)
						obj->m_removed = true;
				}
			}else{
				/*
					Item cannot be picked up. Punch it instead.
				*/

				actionstream<<player->getName()<<" punches object "
						<<obj->getId()<<std::endl;

				v3f playerpos = player->getPosition();
				v3f objpos = obj->getBasePosition();
				v3f dir = (objpos - playerpos).normalize();

				if (!config_get_bool("world.player.inventory.creative") && titem) {
					ToolItemFeatures &tool = content_toolitem_features(wield_item);
					if (
						tool.type == TT_SWORD
						&& tool.param_type == CPT_ENCHANTMENT
						&& enchantment_have(titem->getData(),ENCHANTMENT_DONTBREAK)
						&& obj->getType() == ACTIVEOBJECT_TYPE_MOB
					) {
						InventoryList *mlist = player->inventory.getList("main");
						if (mlist != NULL) {
							for (u32 i=0; i<(8*4); i++) {
								InventoryItem* const item = mlist->getItem(i);
								if (item && item->getContent() == CONTENT_TOOLITEM_MOB_SPAWNER && item->getData() == 0) {
									MobSAO *mob = (MobSAO*)obj;
									item->setData(mob->getContent());
									obj->m_removed = true;
									if (config_get_bool("world.player.tool.wear")) {
										if (titem->addWear(1)) {
											mlist->deleteItem(item_i);
										}else{
											mlist->addDiff(item_i,titem);
										}
									}
									UpdateCrafting(player->peer_id);
									SendInventory(player->peer_id);
									return;
								}
							}
						}
					}
				}

				u16 wear = obj->punch(wield_item, dir, player->getName());
				item = obj->createPickedUpItem(wield_item);
				/* killing something might have caused a drop */
				if (item) {
					InventoryList* const ilist = player->inventory.getList("main");
					if (ilist != NULL) {
						actionstream<<player->getName()<<" picked up "
								<< item->getName() << std::endl;
						if (!config_get_bool("world.player.inventory.creative")) {
							// Skip if inventory has no free space
							if (ilist->roomForItem(item) == false) {
								infostream<<"Player inventory has no free space"<<std::endl;
								return;
							}

							// Add to inventory and send inventory
							ilist->addItem(item);
							UpdateCrafting(player->peer_id);
							SendInventory(player->peer_id);
						}
					}
				}

				if (titem && config_get_bool("world.player.tool.wear")) {
					if (titem->addWear(wear)) {
						mlist->deleteItem(item_i);
					}else{
						mlist->addDiff(item_i,titem);
					}
					SendInventory(player->peer_id);
				}
			}
		}
		// Right click, do something with object
		if (button == 1) {
			actionstream<<player->getName()<<" right clicks object "
					<<obj->getId()<<std::endl;

			// Track changes super-crappily
			u16 oldhp = player->health;
			u16 oldair = player->air;
			u16 oldhunger = player->hunger;

			// Do stuff - resend inventory if returns true
			if (obj->rightClick(player)) {
				UpdateCrafting(peer_id);
				SendInventory(peer_id);
			}

			// Send back stuff
			if (player->health != oldhp || player->air != oldair || player->hunger != oldhunger)
				SendPlayerState(player);
		}
	}
	break;
	case TOSERVER_GROUND_ACTION:
	{
		if(datasize < 17)
			return;
		/*
			length: 17
			[0] u16 command
			[2] u8 action
			[3] v3s16 nodepos_undersurface
			[9] v3s16 nodepos_abovesurface
			[15] u16 item
			actions:
			0: start digging
			1: place block
			2: stop digging (all parameters ignored)
			3: digging completed
		*/
		u8 action = readU8(&data[2]);
		v3s16 p_under;
		p_under.X = readS16(&data[3]);
		p_under.Y = readS16(&data[5]);
		p_under.Z = readS16(&data[7]);
		v3s16 p_over;
		p_over.X = readS16(&data[9]);
		p_over.Y = readS16(&data[11]);
		p_over.Z = readS16(&data[13]);
		u16 item_i = readU16(&data[15]);

		// distance between p_under and p_over must be 1 or 0
		{
			v3s16 p_check = p_over-p_under;
			s16 check = p_check.X+p_check.Y+p_check.Z;
			if (check < -1 || check > 1)
				return;
		}

		// selection distance is 5, give a little leaway and check
		// the player is within 8 nodes of the target
		if (action != 2) {
			v3f p_underf = intToFloat(p_under,BS);
			v3f pp = player->getPosition();
			v3_t sp;
			float sg;
			if (pp.getDistanceFrom(p_underf) > 8.0*BS)
				return;
			/* spawn guard */
			if (
				(getPlayerPrivs(player) & PRIV_SERVER) == 0
				&& config_get("world.spawn.static")
				&& !config_get_v3t("world.spawn.guard.radius",&sp)
			) {
				v3f spf(sp.x,sp.y,sp.z);
				sg = config_get_float("world.spawn.guard.radius");
				spf *= BS;
				sg *= BS;
				if (pp.getDistanceFrom(spf) <= sg)
					return;
			}
		}

		InventoryItem* const wielditem = (InventoryItem*) player->getWieldItem();
		const content_t wieldcontent = wielditem ? wielditem->getContent() : CONTENT_IGNORE;
		ToolItemFeatures wielded_tool_features = content_toolitem_features(wieldcontent);
		CraftItemFeatures *wielded_craft_features = content_craftitem_features(wieldcontent);
		ContentFeatures &wielded_material_features = content_features(wieldcontent);

		bool selected_node_exists = false;
		MapNode selected_node = m_env.getMap().getNodeNoEx(p_under,&selected_node_exists);
		content_t selected_content = selected_node.getContent();
		ContentFeatures &selected_node_features = content_features(selected_node.getContent());

		bool borderstone_locked = false;
		if ((getPlayerPrivs(player) & PRIV_SERVER) == 0) {
			uint16_t max_d = config_get_int("world.game.borderstone.radius");
			v3s16 test_p;
			MapNode testnode;
			// non-admins can't lock thing in another player's area
			for(s16 z=-max_d; !borderstone_locked && z<=max_d; z++) {
			for(s16 y=-max_d; !borderstone_locked && y<=max_d; y++) {
			for(s16 x=-max_d; !borderstone_locked && x<=max_d; x++) {
				test_p = p_under + v3s16(x,y,z);
				NodeMetadata *meta = m_env.getMap().getNodeMetadata(test_p);
				if (meta && meta->typeId() == CONTENT_BORDERSTONE) {
					BorderStoneNodeMetadata *bsm = (BorderStoneNodeMetadata*)meta;
					if (bsm->getOwner() != player->getName())
						borderstone_locked = true;
				}
			}
			}
			}
		}
		/*
			0: start digging
		*/
		if (action == 0) {
			NodeMetadata *meta;
			player->updateAnim(PLAYERANIM_DIG,selected_content);
			if (!wielded_tool_features.has_punch_effect)
				return;
			// KEY
			if (
				wielded_tool_features.has_unlock_effect
				&& (
					selected_node_features.alternate_lockstate_node != CONTENT_IGNORE
					|| (
						wielded_tool_features.has_super_unlock_effect
						&& selected_content == CONTENT_BORDERSTONE
					)
				)
			) {
				meta = m_env.getMap().getNodeMetadata(p_under);
				if ((getPlayerPrivs(player) & PRIV_SERVER) == 0 && !wielded_tool_features.has_super_unlock_effect) {
					// non-admins can't unlock other players things
					if (meta && meta->getOwner() != player->getName()) {
						if (meta->getOwner() != "")
							return;
					}
					if (borderstone_locked)
						return;
				}
				NodeMetadata *ometa = NULL;
				core::list<u16> far_players;
				core::map<v3s16, MapBlock*> modified_blocks;
				if (selected_content == CONTENT_BORDERSTONE) {
					if (!wielded_tool_features.has_super_unlock_effect)
						return;
					selected_node.setContent(CONTENT_STONE);
					m_env.getMap().removeNodeMetadata(p_under);
					// send the node
					sendAddNode(p_under, selected_node, 0, &far_players, 30);
				}else{
					content_t c = selected_node_features.alternate_lockstate_node;
					if (meta) {
						if (meta->getEnergy())
							return;
						ometa = meta->clone();
					}
					selected_node.setContent(c);
					// send the node
					sendAddNode(p_under, selected_node, 0, &far_players, 30);
					// wear out the key - admin's key doesn't wear
					ToolItem* const titem = (ToolItem*)wielditem;
					if (
						!wielded_tool_features.has_super_unlock_effect
						&& (getPlayerPrivs(player) & PRIV_SERVER) == 0
						&& config_get_bool("world.player.tool.wear")
					) {
						bool weared_out = titem->addWear(1);
						InventoryList* const mlist = player->inventory.getList("main");
						if (weared_out) {
							mlist->deleteItem(item_i);
						}else{
							mlist->addDiff(item_i,titem);
						}
						SendInventory(player->peer_id);
					}
				}
				// the slow add to map
				{
					MapEditEventIgnorer ign(&m_ignore_map_edit_events);

					std::string p_name = std::string(player->getName());
					m_env.getMap().addNodeAndUpdate(p_under, selected_node, modified_blocks, p_name);
				}
				
				if (ometa)
				{
					//m_env.getMap().setNodeMetadata(p_under,ometa);
					meta = m_env.getMap().getNodeMetadata(p_under);
					if (meta)
					{
						meta->import(ometa);
						meta->setOwner(player->getName());
						delete ometa;
					}
				}
				
				v3s16 blockpos = getNodeBlockPos(p_under);
				MapBlock* const block = m_env.getMap().getBlockNoCreateNoEx(blockpos);
				if (block)
				{
					block->setChangedFlag();
					block->ResetCurrent();
				}
				
				for(core::map<u16, RemoteClient*>::Iterator
					i = m_clients.getIterator();
					i.atEnd()==false; i++)
				{
					RemoteClient *client = i.getNode()->getValue();
					client->SetBlocksNotSent(modified_blocks);
					client->SetBlockNotSent(blockpos);
				}
			}
			else if (selected_node_features.onpunch_gives_inventory
					&& (meta = m_env.getMap().getNodeMetadata(p_under))
					&& (meta->getInventory())
					&& (meta->getInventory()->getList("main"))
					&& (meta->getInventory()->getList("main")->getUsedSlots() > 0))
			{
				Inventory* const inv = meta->getInventory();
				if (inv)
				{
					InventoryList *list = inv->getList("main");
					if (list) {
						u16 max = list->getSize();
						for (u16 i=0; i<max; i++) {
							InventoryItem* const itm = list->changeItem(i,NULL);
							if (!itm)
								continue;
							player->inventory.addItem("main", itm);
						}
						// Send inventory
						UpdateCrafting(player->peer_id);
						SendInventory(player->peer_id);
					}
				}
			}else if (
				selected_node_features.onpunch_replace_node != CONTENT_IGNORE
				&& !wielded_tool_features.has_rotate_effect
			) {
				if (selected_node_features.onpunch_replace_respects_borderstone && borderstone_locked)
					return;
				core::list<u16> far_players;
				core::map<v3s16, MapBlock*> modified_blocks;

				meta = m_env.getMap().getNodeMetadata(p_under);
				NodeMetadata *ometa = NULL;
				std::string owner;
				if (meta) {
					ometa = meta->clone();
					owner = meta->getOwner();
				}
				m_env.getMap().removeNodeMetadata(p_under);

				selected_node.setContent(selected_node_features.onpunch_replace_node);
				sendAddNode(p_under, selected_node, 0, &far_players, 30);
				if (selected_node_features.sound_punch != "")
					SendEnvEvent(ENV_EVENT_SOUND,intToFloat(p_under,BS),selected_node_features.sound_punch,NULL);
				{
					MapEditEventIgnorer ign(&m_ignore_map_edit_events);

					std::string p_name = std::string(player->getName());
					m_env.getMap().addNodeAndUpdate(p_under, selected_node, modified_blocks, p_name);
				}

				if (ometa) {
					meta = m_env.getMap().getNodeMetadata(p_under);
					if (meta) {
						meta->import(ometa);
						meta->setOwner(owner);
					}

					delete ometa;
				}

				if (selected_node_features.onact_also_affects != v3s16(0,0,0)) {
					v3s16 pa = p_under+selected_node.getEffectedRotation();
					MapNode a_node = m_env.getMap().getNodeNoEx(pa);
					ometa = NULL;
					meta = m_env.getMap().getNodeMetadata(pa);
					if (meta) {
						ometa = meta->clone();
						owner = meta->getOwner();
					}
					m_env.getMap().removeNodeMetadata(pa);

					a_node.setContent(content_features(a_node.getContent()).onpunch_replace_node);
					sendAddNode(pa, a_node, 0, &far_players, 30);
					{
						MapEditEventIgnorer ign(&m_ignore_map_edit_events);

						std::string p_name = std::string(player->getName());
						m_env.getMap().addNodeAndUpdate(pa, a_node, modified_blocks, p_name);
					}

					if (ometa) {
						meta = m_env.getMap().getNodeMetadata(pa);
						if (meta) {
							meta->import(ometa);
							meta->setOwner(owner);
						}

						delete ometa;
					}
				}

				v3s16 blockpos = getNodeBlockPos(p_under);
				MapBlock* const block = m_env.getMap().getBlockNoCreateNoEx(blockpos);
				if (block)
				{
					block->setChangedFlag();
					block->ResetCurrent();
				}
				
				for(core::map<u16, RemoteClient*>::Iterator
					i = m_clients.getIterator();
					i.atEnd()==false; i++)
				{
					RemoteClient *client = i.getNode()->getValue();
					client->SetBlocksNotSent(modified_blocks);
					client->SetBlockNotSent(blockpos);
				}
			}else if (selected_node_features.flammable > 1 && wielded_tool_features.has_fire_effect) {
				if (borderstone_locked)
					return;
				if (selected_node_features.flammable == 2) {
					MapNode a = m_env.getMap().getNodeNoEx(p_under+v3s16(0,1,0));
					if (a.getContent() == CONTENT_AIR || content_features(a).flammable > 0) {
						a.setContent(CONTENT_FIRE);
						core::list<u16> far_players;
						sendAddNode(p_under+v3s16(0,1,0), a, 0, &far_players, 30);
						core::map<v3s16, MapBlock*> modified_blocks;
						{
							MapEditEventIgnorer ign(&m_ignore_map_edit_events);

							std::string p_name = std::string(player->getName());
							m_env.getMap().addNodeAndUpdate(p_under+v3s16(0,1,0), a, modified_blocks, p_name);
						}
						for(core::list<u16>::Iterator i = far_players.begin(); i != far_players.end(); i++) {
							u16 peer_id = *i;
							RemoteClient *client = getClient(peer_id);
							if (client == NULL)
								continue;
							client->SetBlocksNotSent(modified_blocks);
						}
					}
				}else{
					selected_node.setContent(CONTENT_FIRE_SHORTTERM);
					core::list<u16> far_players;
					sendAddNode(p_under, selected_node, 0, &far_players, 30);
					core::map<v3s16, MapBlock*> modified_blocks;
					{
						MapEditEventIgnorer ign(&m_ignore_map_edit_events);

						std::string p_name = std::string(player->getName());
						m_env.getMap().addNodeAndUpdate(p_under, selected_node, modified_blocks, p_name);
					}
					for(core::list<u16>::Iterator i = far_players.begin(); i != far_players.end(); i++) {
						u16 peer_id = *i;
						RemoteClient *client = getClient(peer_id);
						if (client == NULL)
							continue;
						client->SetBlocksNotSent(modified_blocks);
					}
				}
				ToolItem* const titem = (ToolItem*)wielditem;
				if (config_get_bool("world.player.tool.wear")) {
					bool weared_out = titem->addWear(1);
					InventoryList *mlist = player->inventory.getList("main");
					if (weared_out) {
						mlist->deleteItem(item_i);
					}else{
						mlist->addDiff(item_i,titem);
					}
					SendInventory(player->peer_id);
				}
			}else if (selected_node_features.energy_type != CET_NONE && !wielded_tool_features.has_rotate_effect) {
				if (wielded_tool_features.has_fire_effect) {
					if (borderstone_locked)
						return;
					meta = m_env.getMap().getNodeMetadata(p_under);
					if (meta && !meta->getEnergy()) {
						v3s16 pp = floatToInt(player->getPosition(),BS);
						meta->energise(ENERGY_MAX,pp,pp,p_under);
					}
					ToolItem* const titem = (ToolItem*) wielditem;
					if (config_get_bool("world.player.tool.wear")) {
						bool weared_out = titem->addWear(1);
						InventoryList *mlist = player->inventory.getList("main");
						if (weared_out) {
							mlist->deleteItem(item_i);
						}else{
							mlist->addDiff(item_i,titem);
						}
						SendInventory(player->peer_id);
					}
				}else if (selected_node_features.energy_type == CET_SWITCH) {
					NodeMetadata *meta = m_env.getMap().getNodeMetadata(p_under);
					if (meta) {
						u8 energy = ENERGY_MAX;
						if (meta->getEnergy())
							energy = 0;
						meta->energise(energy,p_under,p_under,p_under);
					}
				}
			}else if (
				selected_content == CONTENT_WOOD_BARREL
				|| selected_content == CONTENT_APPLEWOOD_BARREL
				|| selected_content == CONTENT_JUNGLEWOOD_BARREL
				|| selected_content == CONTENT_PINE_BARREL
			) {
				BarrelNodeMetadata *bmeta = (BarrelNodeMetadata*)m_env.getMap().getNodeMetadata(p_under);
				if (!bmeta)
					return;
				if (wielded_tool_features.type == TT_BUCKET)
				{
					const content_t c = wielditem->getData();
					if (!c) {
						if (bmeta->m_water_level < 4)
							return;
						wielditem->setData(CONTENT_WATERSOURCE);
						InventoryList *mlist = player->inventory.getList("main");
						if (mlist)
							mlist->addDiff(item_i,wielditem);
						UpdateCrafting(player->peer_id);
						SendInventory(player->peer_id);
						bmeta->m_water_level -= 4;
					}else if (c == CONTENT_WATERSOURCE) {
						if (bmeta->m_water_level > 39)
							return;
						wielditem->setData(0);
						InventoryList *mlist = player->inventory.getList("main");
						if (mlist)
							mlist->addDiff(item_i,wielditem);
						UpdateCrafting(player->peer_id);
						SendInventory(player->peer_id);
						bmeta->m_water_level += 4;
					}else if (c == CONTENT_LAVASOURCE) {
						return;
					}
					v3s16 blockpos = getNodeBlockPos(p_under);
					MapBlock* const block = m_env.getMap().getBlockNoCreateNoEx(blockpos);
					if (!block)
						return;
					
					block->setChangedFlag();
					core::map<v3s16, MapBlock*> modified_blocks;
					modified_blocks.insert(block->getPos(),block);

					for(core::map<u16, RemoteClient*>::Iterator
						i = m_clients.getIterator();
						i.atEnd()==false; i++)
					{
						RemoteClient *client = i.getNode()->getValue();
						client->SetBlocksNotSent(modified_blocks);
						client->SetBlockNotSent(blockpos);
					}
					block->ResetCurrent();
				}
				else if (wielded_tool_features.type == TT_NONE)
				{
					core::list<u16> far_players;
					core::map<v3s16, MapBlock*> modified_blocks;

					meta = m_env.getMap().getNodeMetadata(p_under);
					NodeMetadata *ometa = NULL;
					if (meta)
						ometa = meta->clone();
					m_env.getMap().removeNodeMetadata(p_under);

					selected_node.setContent(selected_node_features.special_alternate_node);
					sendAddNode(p_under, selected_node, 0, &far_players, 30);
					if (selected_node_features.sound_punch != "")
						SendEnvEvent(ENV_EVENT_SOUND,intToFloat(p_under,BS),selected_node_features.sound_punch,NULL);
					{
						MapEditEventIgnorer ign(&m_ignore_map_edit_events);

						std::string p_name = std::string(player->getName());
						m_env.getMap().addNodeAndUpdate(p_under, selected_node, modified_blocks, p_name);
					}

					if (ometa) {
						meta = m_env.getMap().getNodeMetadata(p_under);
						if (meta)
							meta->import(ometa);

						delete ometa;
					}

					v3s16 blockpos = getNodeBlockPos(p_under);
					MapBlock* const block = m_env.getMap().getBlockNoCreateNoEx(blockpos);
					if (block)
					{
						block->setChangedFlag();
						block->ResetCurrent();
					}
					
					for(core::map<u16, RemoteClient*>::Iterator
						i = m_clients.getIterator();
						i.atEnd()==false; i++)
					{
						RemoteClient *client = i.getNode()->getValue();
						client->SetBlocksNotSent(modified_blocks);
						client->SetBlockNotSent(blockpos);
					}
				}
			}
			else if (selected_content == CONTENT_WOOD_BARREL_SEALED
					|| selected_content == CONTENT_APPLEWOOD_BARREL_SEALED
					|| selected_content == CONTENT_JUNGLEWOOD_BARREL_SEALED
					|| selected_content == CONTENT_PINE_BARREL_SEALED)
			{
				if (wielded_tool_features.type == TT_NONE) {
					core::list<u16> far_players;
					core::map<v3s16, MapBlock*> modified_blocks;

					meta = m_env.getMap().getNodeMetadata(p_under);
					NodeMetadata *ometa = NULL;
					if (meta)
						ometa = meta->clone();
					m_env.getMap().removeNodeMetadata(p_under);

					selected_node.setContent(selected_node_features.special_alternate_node);
					sendAddNode(p_under, selected_node, 0, &far_players, 30);
					if (selected_node_features.sound_punch != "")
						SendEnvEvent(ENV_EVENT_SOUND,intToFloat(p_under,BS),selected_node_features.sound_punch,NULL);
					{
						MapEditEventIgnorer ign(&m_ignore_map_edit_events);

						std::string p_name = std::string(player->getName());
						m_env.getMap().addNodeAndUpdate(p_under, selected_node, modified_blocks, p_name);
					}

					if (ometa) {
						meta = m_env.getMap().getNodeMetadata(p_under);
						if (meta)
							meta->import(ometa);

						delete ometa;
					}

					v3s16 blockpos = getNodeBlockPos(p_under);
					MapBlock* const block = m_env.getMap().getBlockNoCreateNoEx(blockpos);
					if (block)
					{
						block->setChangedFlag();
						block->ResetCurrent();
					}
					
					for(core::map<u16, RemoteClient*>::Iterator
						i = m_clients.getIterator();
						i.atEnd()==false; i++)
					{
						RemoteClient *client = i.getNode()->getValue();
						client->SetBlocksNotSent(modified_blocks);
						client->SetBlockNotSent(blockpos);
					}
				}
			}
			else if (selected_content == CONTENT_CAULDRON)
			{
				CauldronNodeMetadata *cmeta = (CauldronNodeMetadata*)m_env.getMap().getNodeMetadata(p_under);
				if (!cmeta)
					return;
				if (wielded_tool_features.type == TT_BUCKET) {
					const content_t c = wielditem->getData();
					if (!c) {
						if (cmeta->m_water_level != 4)
							return;
						wielditem->setData(CONTENT_WATERSOURCE);
						InventoryList *mlist = player->inventory.getList("main");
						if (mlist)
							mlist->addDiff(item_i,wielditem);
						UpdateCrafting(player->peer_id);
						SendInventory(player->peer_id);
						cmeta->m_water_level = 0;
					}else if (c == CONTENT_WATERSOURCE) {
						if (cmeta->m_water_level > 3)
							return;
						wielditem->setData(0);
						InventoryList *mlist = player->inventory.getList("main");
						if (mlist)
							mlist->addDiff(item_i,wielditem);
						UpdateCrafting(player->peer_id);
						SendInventory(player->peer_id);
						cmeta->m_water_level = 4;
					}else if (c == CONTENT_LAVASOURCE) {
						return;
					}
					v3s16 blockpos = getNodeBlockPos(p_under);
					MapBlock* const block = m_env.getMap().getBlockNoCreateNoEx(blockpos);
					if (!block)
						return;
					block->setChangedFlag();
					core::map<v3s16, MapBlock*> modified_blocks;
					modified_blocks.insert(block->getPos(),block);

					for(core::map<u16, RemoteClient*>::Iterator
						i = m_clients.getIterator();
						i.atEnd()==false; i++)
					{
						RemoteClient *client = i.getNode()->getValue();
						client->SetBlocksNotSent(modified_blocks);
						client->SetBlockNotSent(blockpos);
					}
					block->ResetCurrent();
				}
				else if (wieldcontent == CONTENT_CRAFTITEM_IRON_BOTTLE)
				{
					if (cmeta->m_water_level && cmeta->m_water_hot && wielditem->getCount() == 1)
					{
						InventoryItem* const itm = InventoryItem::create(CONTENT_CRAFTITEM_IRON_BOTTLE_WATER,1,0);
						InventoryList* const mlist = player->inventory.getList("main");
						InventoryItem* const old = mlist->changeItem(item_i,itm);
						if (old)
							delete old;
						cmeta->m_water_level--;
						SendInventory(player->peer_id);
						v3s16 blockpos = getNodeBlockPos(p_under);
						MapBlock* const block = m_env.getMap().getBlockNoCreateNoEx(blockpos);
						if (!block)
							return;
						block->setChangedFlag();
						core::map<v3s16, MapBlock*> modified_blocks;
						modified_blocks.insert(block->getPos(),block);

						for(core::map<u16, RemoteClient*>::Iterator
							i = m_clients.getIterator();
							i.atEnd()==false; i++)
						{
							RemoteClient *client = i.getNode()->getValue();
							client->SetBlocksNotSent(modified_blocks);
							client->SetBlockNotSent(blockpos);
						}
						block->ResetCurrent();
					}
				}
				else if (wieldcontent == CONTENT_CRAFTITEM_GLASS_BOTTLE)
				{
					if (cmeta->m_water_level && cmeta->m_water_heated
							&& !cmeta->m_water_hot && wielditem->getCount() == 1)
					{
						InventoryItem* const itm = InventoryItem::create(CONTENT_CRAFTITEM_GLASS_BOTTLE_WATER,1,0);
						InventoryList* const mlist = player->inventory.getList("main");
						InventoryItem* const old = mlist->changeItem(item_i,itm);
						if (old)
							delete old;
						cmeta->m_water_level--;
						SendInventory(player->peer_id);
						v3s16 blockpos = getNodeBlockPos(p_under);
						MapBlock* const block = m_env.getMap().getBlockNoCreateNoEx(blockpos);
						if (!block)
							return;
						block->setChangedFlag();
						core::map<v3s16, MapBlock*> modified_blocks;
						modified_blocks.insert(block->getPos(),block);

						for(core::map<u16, RemoteClient*>::Iterator
							i = m_clients.getIterator();
							i.atEnd()==false; i++)
						{
							RemoteClient *client = i.getNode()->getValue();
							client->SetBlocksNotSent(modified_blocks);
							client->SetBlockNotSent(blockpos);
						}
						block->ResetCurrent();
					}
				}
			}
			else if (selected_content == CONTENT_INCINERATOR)
			{
				meta = m_env.getMap().getNodeMetadata(p_under);
				if (!meta)
					return;
				InventoryList *ilist = meta->getInventory()->getList("fuel");
				InventoryList *plist = player->inventory.getList("main");
				if (plist == NULL || ilist == NULL)
					return;
				if (((IncineratorNodeMetadata*)meta)->m_fuel_totaltime <= ((IncineratorNodeMetadata*)meta)->m_fuel_time) {
					InventoryItem* const fitem = ilist->getItem(0);
					if (!fitem || !fitem->getCount() || !fitem->isFuel())
						return;
					((IncineratorNodeMetadata*)meta)->m_should_fire = true;
				}
				plist->deleteItem(item_i);
				UpdateCrafting(peer_id);
				SendInventory(peer_id);
			}else if (
				selected_node_features.draw_type != CDT_TORCHLIKE
				&& wielded_tool_features.has_rotate_effect
				&& (
					selected_node_features.param_type == CPT_FACEDIR_SIMPLE
					|| selected_node_features.param2_type == CPT_FACEDIR_SIMPLE
					|| selected_node_features.param_type == CPT_FACEDIR_WALLMOUNT
					|| selected_node_features.param2_type == CPT_FACEDIR_WALLMOUNT
				)
			) {
				if (borderstone_locked)
					return;
				if (
					(
						selected_content < CONTENT_BED_MIN
						|| selected_content > CONTENT_BED_MAX
					)
				) {
					u8 rot = 0;
					NodeMetadata *meta = m_env.getMap().getNodeMetadata(p_under);
					NodeMetadata *ometa = NULL;
					std::string owner;
					if (meta) {
						if (meta->getEnergy())
							return;
						ometa = meta->clone();
						owner = meta->getOwner();
					}
					// get the rotation
					if (
						selected_node_features.param_type == CPT_FACEDIR_SIMPLE
						|| selected_node_features.param_type == CPT_FACEDIR_WALLMOUNT
					) {
						rot = selected_node.param1;
					}else{
						rot = selected_node.param2&0x0F;
					}
					// rotate it
					rot++;
					if (rot > 3)
						rot = 0;
					// set the rotation
					if (
						selected_node_features.param_type == CPT_FACEDIR_SIMPLE
						|| selected_node_features.param_type == CPT_FACEDIR_WALLMOUNT
					) {
						selected_node.param1 = rot;
					}else{
						selected_node.param2 &= 0xF0;
						selected_node.param2 |= rot;
					}
					// send the node
					core::list<u16> far_players;
					core::map<v3s16, MapBlock*> modified_blocks;
					sendAddNode(p_under, selected_node, 0, &far_players, 30);
					// wear out the crowbar
					ToolItem* const titem = (ToolItem*)wielditem;
					if (config_get_bool("world.player.tool.wear")) {
						bool weared_out = titem->addWear(1);
						InventoryList *mlist = player->inventory.getList("main");
						if (weared_out) {
							mlist->deleteItem(item_i);
						}else{
							mlist->addDiff(item_i,titem);
						}
						SendInventory(player->peer_id);
					}
					// the slow add to map
					{
						MapEditEventIgnorer ign(&m_ignore_map_edit_events);

						std::string p_name = std::string(player->getName());
						m_env.getMap().addNodeAndUpdate(p_under, selected_node, modified_blocks, p_name);
					}
					if (ometa) {
						m_env.getMap().setNodeMetadata(p_under,ometa);
						meta = m_env.getMap().getNodeMetadata(p_under);
						if (meta)
							meta->setOwner(owner);
					}
					v3s16 blockpos = getNodeBlockPos(p_under);
					MapBlock* const block = m_env.getMap().getBlockNoCreateNoEx(blockpos);
					if (block)
					{
						block->setChangedFlag();
						block->ResetCurrent();
					}
					
					for(core::map<u16, RemoteClient*>::Iterator
						i = m_clients.getIterator();
						i.atEnd()==false; i++)
					{
						RemoteClient *client = i.getNode()->getValue();
						client->SetBlocksNotSent(modified_blocks);
						client->SetBlockNotSent(blockpos);
					}
				}
			}
			else if (wieldcontent == CONTENT_CRAFTITEM_FERTILIZER
					&& selected_node_features.fertilizer_affects)
			{
				selected_node.envticks = 1024;
				// send the node
				core::list<u16> far_players;
				core::map<v3s16, MapBlock*> modified_blocks;
				sendAddNode(p_under, selected_node, 0, &far_players, 30);
				/*
					Handle inventory
				*/
				InventoryList* const ilist = player->inventory.getList("main");
				if (!config_get_bool("world.player.inventory.creative") && ilist) {
					// Remove from inventory and send inventory
					if (wielditem->getCount() == 1) {
						ilist->deleteItem(item_i);
					}else{
						wielditem->remove(1);
						ilist->addDiff(item_i,wielditem);
					}
					// Send inventory
					UpdateCrafting(peer_id);
					SendInventory(peer_id);
				}
				// the slow add to map
				{
					MapEditEventIgnorer ign(&m_ignore_map_edit_events);

					std::string p_name = std::string(player->getName());
					m_env.getMap().addNodeAndUpdate(p_under, selected_node, modified_blocks, p_name);
				}
				v3s16 blockpos = getNodeBlockPos(p_under);
				MapBlock* const block = m_env.getMap().getBlockNoCreateNoEx(blockpos);
				if (block)
				{
					block->setChangedFlag();
					block->ResetCurrent();
				}
				
				for(core::map<u16, RemoteClient*>::Iterator
					i = m_clients.getIterator();
					i.atEnd()==false; i++)
				{
					RemoteClient *client = i.getNode()->getValue();
					client->SetBlocksNotSent(modified_blocks);
					client->SetBlockNotSent(blockpos);
				}
			}
			/*
				NOTE: This can be used in the future to check if
				somebody is cheating, by checking the timing.
			*/
		} // action == 0

		/*
			2: stop digging
		*/
		else if (action == 2) {
			player->updateAnim(PLAYERANIM_STAND,CONTENT_AIR);
#if 0
			RemoteClient *client = getClient(peer_id);
			JMutexAutoLock digmutex(client->m_dig_mutex);
			client->m_dig_tool_item = -1;
#endif
		}

		/*
			3: Digging completed
		*/
		else if (action == 3) {
			// used by map manipulation functions
			core::map<v3s16, MapBlock*> modified_blocks;
			core::list<u16> far_players;

			u8 p2 = selected_node.param2;
			u8 mineral = selected_node.getMineral();
			NodeMetadata *meta = NULL;
			bool cannot_remove_node = !selected_node_exists;
			bool node_replaced = false;

			if (cannot_remove_node) {
				infostream<<"Server: Not finishing digging: Node not found."
						<<" Adding block to emerge queue."
						<<std::endl;
				m_emerge_queue.addBlock(peer_id, getNodeBlockPos(p_over), BLOCK_EMERGE_FLAG_FROMDISK);
			}else{
				if (!selected_node_features.diggable) {
					infostream<<"Server: Not finishing digging: "
							<<"Node not diggable"
							<<std::endl;
					cannot_remove_node = true;
				}else{
					meta = m_env.getMap().getNodeMetadata(p_under);
					if (meta && meta->nodeRemovalDisabled() == true) {
						infostream<<"Server: Not finishing digging: "
								<<"Node metadata disables removal"
								<<std::endl;
						cannot_remove_node = true;
					}else if (selected_node_features.onact_also_affects != v3s16(0,0,0)) {
						NodeMetadata *ameta = m_env.getMap().getNodeMetadata(p_under+selected_node.getEffectedRotation());
						if (ameta && ameta->nodeRemovalDisabled() == true) {
							infostream<<"Server: Not finishing digging: "
									<<"Sibling Node metadata disables removal"
									<<std::endl;
							cannot_remove_node = true;
						}
					}
				}
			}

			// check for a diggable tool
			if (wielded_tool_features.type == TT_SPECIAL) {
				infostream<<"Server: Not finished digging: impossible tool"<<std::endl;
				cannot_remove_node = true;
			// Make sure the player is allowed to do it
			}else if ((getPlayerPrivs(player) & PRIV_BUILD) == 0) {
				infostream<<"Player "<<player->getName()<<" cannot remove node"
						<<" because privileges are "<<getPlayerPrivs(player)
						<<std::endl;
				cannot_remove_node = true;
			// check for borderstone
			}else if (borderstone_locked && !selected_node_features.borderstone_diggable) {
				infostream<<"Player "<<player->getName()<<" cannot remove node: borderstone protected"<<std::endl;
				cannot_remove_node = true;
			// check for locked items and borderstone
			}else if((getPlayerPrivs(player) & PRIV_SERVER) == 0) {
				if (
					meta
					&& (
						meta->typeId() == CONTENT_LOCKABLE_CHEST_DEPRECATED
						|| meta->typeId() == CONTENT_LOCKABLE_SIGN
						|| meta->typeId() == CONTENT_LOCKABLE_SIGN_WALL
						|| meta->typeId() == CONTENT_LOCKABLE_SIGN_UD
						|| meta->typeId() == CONTENT_LOCKABLE_FURNACE_DEPRECATED
						|| meta->typeId() == CONTENT_FLAG
						|| meta->typeId() == CONTENT_FLAG_BLUE
						|| meta->typeId() == CONTENT_FLAG_GREEN
						|| meta->typeId() == CONTENT_FLAG_ORANGE
						|| meta->typeId() == CONTENT_FLAG_PURPLE
						|| meta->typeId() == CONTENT_FLAG_RED
						|| meta->typeId() == CONTENT_FLAG_YELLOW
						|| meta->typeId() == CONTENT_FLAG_BLACK
						|| meta->typeId() == CONTENT_SAFE
					)
					&& meta->getOwner() != ""
					&& meta->getOwner() != player->getName()
				) {
					infostream<<"Player "<<player->getName()<<" cannot remove node: not node owner"<<std::endl;
					cannot_remove_node = true;
				}
			}

			/*
				If node can't be removed, set block to be re-sent to
				client and quit.
			*/
			if (cannot_remove_node) {
				infostream<<"Server: Not finishing digging."<<std::endl;

				// Client probably has wrong data.
				// Set block not sent, so that client will get
				// a valid one.
				infostream<<"Client "<<peer_id<<" tried to dig "
						<<"node; but node cannot be removed."
						<<" setting MapBlock not sent."<<std::endl;
				RemoteClient *client = getClient(peer_id);
				v3s16 blockpos = getNodeBlockPos(p_under);
				client->SetBlockNotSent(blockpos);

				return;
			}

			// This was pretty much the entirety of farming
			if (selected_node_features.farm_ploughable && wielded_tool_features.type == TT_SHOVEL) {
				v3s16 temp_p = p_under;
				v3s16 test_p;
				MapNode testnode;
				test_p = temp_p + v3s16(0,1,0);
				testnode = m_env.getMap().getNodeNoEx(test_p);
				bool is_farm_swap = false;
				if (testnode.getContent() == CONTENT_AIR) {
					for(s16 z=-3; !is_farm_swap && z<=3; z++) {
					for(s16 x=-3; !is_farm_swap && x<=3; x++)
					{
						test_p = temp_p + v3s16(x,0,z);
						testnode = m_env.getMap().getNodeNoEx(test_p);
						if (
							testnode.getContent() == CONTENT_WATERSOURCE
						) {
							is_farm_swap = true;
							break;
						}
					}
					}
				}
				if (is_farm_swap) {
					selected_node.setContent(CONTENT_FARM_DIRT);

					actionstream<<player->getName()<<" ploughs "<<PP(p_under)
							<<" into farm dirt"<<std::endl;

					sendAddNode(p_under, selected_node, 0, &far_players, 30);

					/*
						Add node.

						This takes some time so it is done after the quick stuff
					*/
					core::map<v3s16, MapBlock*> modified_blocks;
					{
						MapEditEventIgnorer ign(&m_ignore_map_edit_events);

						std::string p_name = std::string(player->getName());
						m_env.getMap().addNodeAndUpdate(p_under, selected_node, modified_blocks, p_name);
					}
					/*
						Set blocks not sent to far players
					*/
					for (core::list<u16>::Iterator i = far_players.begin(); i != far_players.end(); i++) {
						u16 peer_id = *i;
						RemoteClient *client = getClient(peer_id);
						if (client==NULL)
							continue;
						client->SetBlocksNotSent(modified_blocks);
					}
					return;
				}
			}

			actionstream<<player->getName()<<" digs "<<PP(p_under)
					<<", gets material "<<(int)selected_content<<", mineral "
					<<(int)mineral<<std::endl;

			if (selected_node_features.onact_also_affects != v3s16(0,0,0)) {
				v3s16 p_other = selected_node.getEffectedRotation();
				v3s16 p_also = p_under + p_other;
				sendRemoveNode(p_also, 0, &far_players, 30);
				{
					MapEditEventIgnorer ign(&m_ignore_map_edit_events);
					m_env.getMap().removeNodeAndUpdate(p_also, modified_blocks);
				}
				sendRemoveNode(p_under, 0, &far_players, 30);
			}else{
				if (selected_node_features.home_node > -1)
					player->unsetHome(selected_node_features.home_node);
				if (selected_content == CONTENT_FLOWER_POT) {
					MapNode a = m_env.getMap().getNodeNoEx(p_under+v3s16(0,1,0));
					if (
						a.getContent() == CONTENT_FLOWER_ROSE
						|| a.getContent() == CONTENT_FLOWER_DAFFODIL
						|| a.getContent() == CONTENT_FLOWER_TULIP
						|| a.getContent() == CONTENT_FLOWER_STEM
						|| a.getContent() == CONTENT_WILDGRASS_SHORT
						|| a.getContent() == CONTENT_WILDGRASS_LONG
					) {
						sendRemoveNode(p_under+v3s16(0,1,0), 0, &far_players, 30);
						{
							MapEditEventIgnorer ign(&m_ignore_map_edit_events);
							m_env.getMap().removeNodeAndUpdate(p_under+v3s16(0,1,0), modified_blocks);
						}
					}
				}
				MapNode a = m_env.getMap().getNodeNoEx(p_under+v3s16(0,1,0));
				if (a.getContent() == CONTENT_FIRE) {
					sendRemoveNode(p_under+v3s16(0,1,0), 0, &far_players, 30);
					{
						MapEditEventIgnorer ign(&m_ignore_map_edit_events);
						m_env.getMap().removeNodeAndUpdate(p_under+v3s16(0,1,0), modified_blocks);
					}
				}
			/*
				Send the removal to all close-by players.
				- If other player is close, send REMOVENODE
				- Otherwise set blocks not sent
			*/
				sendRemoveNode(p_under, peer_id, &far_players, 30);
			}

			if (selected_node_features.ondig_replace_node != CONTENT_IGNORE) {
				bool will_replace = true;
				if (selected_node_features.ondig_replace_node_requires != CONTENT_IGNORE) {
					will_replace = false;
					v3s16 test_p;
					MapNode testnode;
					for(s16 z=-3; !will_replace && z<=3; z++) {
					for(s16 y=-3; !will_replace && y<=3; y++) {
					for(s16 x=-3; !will_replace && x<=3; x++) {
						test_p = p_under + v3s16(x,y,z);
						testnode = m_env.getMap().getNodeNoEx(test_p);
						if (testnode.getContent() == selected_node_features.ondig_replace_node_requires) {
							will_replace = true;
							break;
						}
					}
					}
					}
				}
				if (will_replace) {
					MapNode n = selected_node;
					n.setContent(selected_node_features.ondig_replace_node);
					sendAddNode(p_under, n, 0, &far_players, 30);
					{
						MapEditEventIgnorer ign(&m_ignore_map_edit_events);

						std::string p_name = std::string(player->getName());
						m_env.getMap().addNodeAndUpdate(p_under, n, modified_blocks, p_name);
					}
					node_replaced = true;
				}
			}

			/*
				Update and send inventory
			*/

			if (!config_get_bool("world.player.inventory.creative")) {
				/*
					Wear out tool
				*/
				InventoryList *mlist = player->inventory.getList("main");
				if (mlist != NULL && config_get_bool("world.player.tool.wear")) {
					InventoryItem* const item = mlist->getItem(item_i);
					if (item && (item->getContent()&CONTENT_TOOLITEM_MASK) == CONTENT_TOOLITEM_MASK) {
						ToolItem* const titem = (ToolItem*) item;
						// Get digging properties for material and tool
						tooluse_t usage;
						if (get_tool_use(&usage,selected_content,mineral,titem->getContent(),titem->getData())) {
							infostream<<"Server: WARNING: Player digged"
								<<" with impossible material + tool"
								<<" combination"<<std::endl;
						}else{
							if (!usage.diggable) {
								infostream<<"Server: WARNING: Player digged"
									<<" with impossible material + tool"
									<<" combination"<<std::endl;
							}
							if (usage.wear) {
								if (titem->addWear(usage.wear)) {
									mlist->deleteItem(item_i);
								}else{
									mlist->addDiff(item_i,titem);
								}
							}
						}
					}
				}

				/*
					Add dug item to inventory
				*/

				InventoryItem *item = NULL;

				if (selected_content == CONTENT_PARCEL) {
					NodeMetadata *meta = m_env.getMap().getNodeMetadata(p_under);
					if (meta) {
						Inventory *inv = meta->getInventory();
						if (inv) {
							InventoryList *l = inv->getList("0");
							if (l) {
								for (u32 i=0; i<32; i++) {
									InventoryItem* const itm = l->changeItem(i,NULL);
									if (!itm)
										continue;
									player->inventory.addItem("main", itm);
								}
								// Send inventory
								UpdateCrafting(player->peer_id);
								SendInventory(player->peer_id);
							}
						}
					}
				}else if (
					selected_node_features.ondig_special_drop != CONTENT_IGNORE
					&& selected_node_features.ondig_special_tool == wielded_tool_features.type
				) {
					item = InventoryItem::create(
						selected_node_features.ondig_special_drop,
						selected_node_features.ondig_special_drop_count
					);
				}else if (selected_node_features.liquid_type != LIQUID_NONE) {
					if (selected_node_features.liquid_type == LIQUID_SOURCE && wielded_tool_features.type == TT_BUCKET) {
						if (selected_node_features.damage_per_second > 0 && !wielded_tool_features.damaging_nodes_diggable) {
							mlist->deleteItem(item_i);
							UpdateCrafting(player->peer_id);
							SendInventory(player->peer_id);
							HandlePlayerHP(player,4,0,0);
							return;
						}else if (wielded_tool_features.param_type == CPT_CONTENT) {
							wielditem->setData(selected_node.getContent());
							mlist->addDiff(item_i,wielditem);
							UpdateCrafting(player->peer_id);
							SendInventory(player->peer_id);
						}
					}
				}else if (selected_node_features.liquid_type == LIQUID_NONE) {
					std::string &dug_s = selected_node_features.dug_item;
					if (wielded_tool_features.type != TT_NONE
							&& enchantment_have(wielditem->getData(),ENCHANTMENT_DONTBREAK)) {
						u16 data = 0;
						if (selected_node_features.param_type == CPT_MINERAL
								|| selected_node_features.param_type == CPT_BLOCKDATA)
						    data = selected_node.param1;
						item = InventoryItem::create(selected_content,1,0,data);
					}else if (
						wielded_tool_features.type != TT_NONE
						&& enchantment_have(wielditem->getData(),ENCHANTMENT_FLAME)
						&& selected_node_features.cook_result != ""
					) {
						std::istringstream is(selected_node_features.cook_result, std::ios::binary);
						item = InventoryItem::deSerialize(is);
					}else if (dug_s != "") {
						std::istringstream is(dug_s, std::ios::binary);
						item = InventoryItem::deSerialize(is);
						if (selected_node_features.draw_type == CDT_DIRTLIKE) {
							content_t extra = CONTENT_IGNORE;
							switch (selected_node.param1&0x0F) {
							case 0x01:
							case 0x02:
								extra = CONTENT_WILDGRASS_SHORT;
								break;
							case 0x04:
								extra = CONTENT_CRAFTITEM_SNOW_BALL;
								break;
							default:;
							}
							if (extra == CONTENT_IGNORE) {
								if ((selected_node.param1&0x20) == 0x20)
									mineral = MINERAL_SALT;
							}else if (myrand_range(0,5) == 0) {
								InventoryItem* const eitem = InventoryItem::create(extra,1,0);
								player->inventory.addItem("main", eitem);
							}
						}
						if (item && selected_node_features.item_param_type == CPT_METADATA) {
							NodeMetadata* const meta = m_env.getMap().getNodeMetadata(p_under);
							if (meta) {
								uint16_t v = meta->getData();
								item->setData(v);
							}
						}
					}else if (selected_node_features.param2_type == CPT_PLANTGROWTH) {
						if (
							selected_node_features.draw_type == CDT_PLANTLIKE
							|| selected_node_features.draw_type == CDT_CROPLIKE
						) {
							if (p2 && p2 < 8) {
								if (selected_node_features.plantgrowth_small_dug_node != CONTENT_IGNORE) {
									u16 count = 1;
									if (p2 > 3)
										count = 2;
									item = InventoryItem::create(
										selected_node_features.plantgrowth_small_dug_node,count);
								}
							}else{
								if (selected_node_features.plantgrowth_large_gives_small) {
									item = InventoryItem::create(
										selected_node_features.plantgrowth_small_dug_node,
										2
									);
									player->inventory.addItem("main", item);
								}
								u16 count = selected_node_features.plantgrowth_large_count;
								if (p2) {
									if (p2 < 12) {
										count -= count/3;
									}else{
										count /= 2;
									}
								}
								if (!count)
									count = 1;
								item = InventoryItem::create(
									selected_node_features.plantgrowth_large_dug_node,count);
							}
						}else if (selected_node_features.draw_type == CDT_MELONLIKE) {
							if (p2) {
								if (p2 > 4) {
									u16 count = 1;
									if (p2 > 11) {
										count = 3;
									}else if (p2 > 7) {
										count = 2;
									}
									item = InventoryItem::create(CONTENT_CRAFTITEM_MUSH,count,0);
									player->inventory.addItem("main", item);
								}
								if (selected_node_features.plantgrowth_small_dug_node != CONTENT_IGNORE)
									item = InventoryItem::create(
										selected_node_features.plantgrowth_small_dug_node,1);
							}else{
								if (selected_node_features.plantgrowth_large_gives_small) {
									item = InventoryItem::create(
										selected_node_features.plantgrowth_small_dug_node,1);
									player->inventory.addItem("main", item);
								}
								item = InventoryItem::create(
									selected_node_features.plantgrowth_large_dug_node,
									selected_node_features.plantgrowth_large_count);
							}
						}
					}
				}

				bool dosend = false;
				if (item != NULL) {
					// Add a item to inventory
					player->inventory.addItem("main", item);

					// Send inventory
					dosend = true;
				}

				item = NULL;

				if (mineral != MINERAL_NONE)
					item = getDiggedMineralItem(mineral,player,wielditem);

				// If not mineral
				if (item == NULL) {
					std::string &extra_dug_s = selected_node_features.extra_dug_item;
					s32 extra_rarity = selected_node_features.extra_dug_item_rarity;
					if (
						extra_dug_s != ""
						&& extra_rarity != 0
						&& selected_node_features.extra_dug_item_min_level <= wielded_tool_features.diginfo.level
						&& selected_node_features.extra_dug_item_max_level >= wielded_tool_features.diginfo.level
						&& myrand_range(0,extra_rarity) == 0
					) {
						std::istringstream is(extra_dug_s, std::ios::binary);
						item = InventoryItem::deSerialize(is);
					}
				}

				if (item != NULL) {
					// Add a item to inventory
					player->inventory.addItem("main", item);

					// Send inventory
					dosend = true;
				}
				if (dosend) {
					UpdateCrafting(player->peer_id);
					SendInventory(player->peer_id);
				}
			}

			/*
				Remove the node
				(this takes some time so it is done after the quick stuff)
			*/
			if (!node_replaced) {
				MapEditEventIgnorer ign(&m_ignore_map_edit_events);

				m_env.getMap().removeNodeAndUpdate(p_under, modified_blocks);
			}
			/*
				Set blocks not sent to far players
			*/
			for (core::list<u16>::Iterator i = far_players.begin(); i != far_players.end(); i++) {
				u16 peer_id = *i;
				RemoteClient *client = getClient(peer_id);
				if(client==NULL)
					continue;
				client->SetBlocksNotSent(modified_blocks);
			}
		}

		/*
			1: place block
		*/
		else if(action == 1)
		{
			v3s16 p_dir = p_under - p_over;
			InventoryList* const ilist = player->inventory.getList("main");
			if (ilist == NULL)
				return;

			// Get item
			InventoryItem* const item = ilist->getItem(item_i);

			// If there is no item, it is not possible to add it anywhere
			if (item == NULL)
				return;

			if (borderstone_locked)
				return;

			/*
				Handle material items
			*/
			if (
				(wieldcontent&0xF000) == 0
				|| (
					(wieldcontent&CONTENT_TOOLITEM_MASK) == CONTENT_TOOLITEM_MASK
					&& wielded_tool_features.param_type == CPT_CONTENT
					&& item->getData() != 0
				)
				|| (
					(wieldcontent&CONTENT_CRAFTITEM_MASK) == CONTENT_CRAFTITEM_MASK
					&& wielded_craft_features->param_type == CPT_CONTENT
					&& item->getData() != 0
				)
			) {
				bool replaced_node_exists = false;
				MapNode replaced_node = m_env.getMap().getNodeNoEx(p_over,&replaced_node_exists);
				if (!replaced_node_exists) {
					infostream<<"Server: Ignoring ADDNODE: Node not found"
							<<" Adding block to emerge queue."
							<<std::endl;
					m_emerge_queue.addBlock(peer_id,
							getNodeBlockPos(p_over), BLOCK_EMERGE_FLAG_FROMDISK);
					return;
				}

				content_t replaced_content = replaced_node.getContent();
				ContentFeatures &replaced_node_features = content_features(replaced_content);

				if ((getPlayerPrivs(player)&PRIV_BUILD) == 0) {
					infostream<<"Player "<<player->getName()<<" cannot add node"
						<<" because privileges are "<<getPlayerPrivs(player)
						<<std::endl;
					return;
				}

				if (!replaced_node_features.buildable_to) {
					// Client probably has wrong data.
					// Set block not sent, so that client will get
					// a valid one.
					infostream<<"Client "<<peer_id<<" tried to place"
							<<" node in invalid position; setting"
							<<" MapBlock not sent."<<std::endl;
					RemoteClient *client = getClient(peer_id);
					v3s16 blockpos = getNodeBlockPos(p_over);
					client->SetBlockNotSent(blockpos);
					return;
				}

				// Reset build time counter
				getClient(peer_id)->m_time_from_building = 0.0;

				// the node type
				content_t addedcontent = wieldcontent;
				if (
					(wieldcontent&CONTENT_TOOLITEM_MASK) == CONTENT_TOOLITEM_MASK
					|| (wieldcontent&CONTENT_CRAFTITEM_MASK) == CONTENT_CRAFTITEM_MASK
				) {
					addedcontent = item->getData();
				}

				// don't allow borderstone to be place near another player's borderstone
				if (addedcontent == CONTENT_BORDERSTONE) {
					uint16_t max_d = config_get_int("world.game.borderstone.radius");
					v3s16 test_p;
					MapNode testnode;
					max_d *= 2;
					for(s16 z=-max_d; z<=max_d; z++) {
					for(s16 y=-max_d; y<=max_d; y++) {
					for(s16 x=-max_d; x<=max_d; x++) {
						test_p = p_over + v3s16(x,y,z);
						NodeMetadata *meta = m_env.getMap().getNodeMetadata(test_p);
						if (meta && meta->typeId() == CONTENT_BORDERSTONE) {
							BorderStoneNodeMetadata *bsm = (BorderStoneNodeMetadata*)meta;
							if (bsm->getOwner() != player->getName())
								return;
						}
					}
					}
					}
				// Stairs and Slabs special functions
				}else if (addedcontent >= CONTENT_SLAB_STAIR_MIN && addedcontent <= CONTENT_SLAB_STAIR_UD_MAX) {
					MapNode abv = m_env.getMap().getNodeNoEx(p_over+p_dir);
					// Make a cube if two slabs of the same type are stacked on each other
					if (
						p_dir.X == 0
						&& p_dir.Z == 0
						&& wielded_material_features.special_alternate_node != CONTENT_IGNORE
						&& abv.getContent() >= CONTENT_SLAB_STAIR_MIN
						&& abv.getContent() <= CONTENT_SLAB_STAIR_UD_MAX
						&& wielded_material_features.special_alternate_node == content_features(abv).special_alternate_node
					) {
						addedcontent = wielded_material_features.special_alternate_node;
						p_over += p_dir;
					// Flip it upside down if it's being placed on the roof
					// or if placing against an upside down node
					}else if (
						p_dir.Y == 1
						|| (
							abv.getContent() >= CONTENT_SLAB_STAIR_UD_MIN
							&& abv.getContent() <= CONTENT_SLAB_STAIR_UD_MAX
						)
					) {
						addedcontent |= CONTENT_SLAB_STAIR_FLIP;
					}
				// roof mount
				}else if (p_dir.Y == 1) {
					if (wielded_material_features.roofmount_alternate_node != CONTENT_IGNORE)
						addedcontent = wielded_material_features.roofmount_alternate_node;
				// floor mount
				}else if (p_dir.Y == -1){
					if (wielded_material_features.floormount_alternate_node != CONTENT_IGNORE)
						addedcontent = wielded_material_features.floormount_alternate_node;
				// wall mount
				}else{
					if (wielded_material_features.wallmount_alternate_node != CONTENT_IGNORE)
						addedcontent = wielded_material_features.wallmount_alternate_node;
				}

				actionstream<<player->getName()<<" places material "
						<<(int)addedcontent
						<<" at "<<PP(p_under)<<std::endl;

				// Create node data
				MapNode n(addedcontent);
				ContentFeatures &added_content_features = content_features(addedcontent);
				if (added_content_features.param_type == CPT_MINERAL)
					n.param1 = item->getData();

				// Calculate the direction for directional and wall mounted nodes
				if (added_content_features.param2_type == CPT_FACEDIR_SIMPLE) {
					v3f playerpos = player->getPosition();
					v3f blockpos = intToFloat(p_over, BS) - playerpos;
					blockpos = blockpos.normalize();
					n.param2 &= 0xF0;
					if (fabs(blockpos.X) > fabs(blockpos.Z)) {
						if (blockpos.X < 0)
							n.param2 |= 0x03;
						else
							n.param2 |= 0x01;
					} else {
						if (blockpos.Z < 0)
							n.param2 |= 0x02;
						else
							n.param2 |= 0x00;
					}
				}else if (added_content_features.param_type == CPT_FACEDIR_SIMPLE) {
					v3f playerpos = player->getPosition();
					v3f blockpos = intToFloat(p_over, BS) - playerpos;
					blockpos = blockpos.normalize();
					n.param1 = 0;
					if (fabs(blockpos.X) > fabs(blockpos.Z)) {
						if (blockpos.X < 0)
							n.param1 = 3;
						else
							n.param1 = 1;
					} else {
						if (blockpos.Z < 0)
							n.param1 = 2;
						else
							n.param1 = 0;
					}
				}else if (added_content_features.param2_type == CPT_FACEDIR_WALLMOUNT) {
					n.param2 &= 0xF0;
					u8 pd = packDir(p_under - p_over);
					n.param2 |= (pd&0x0F);
				}else if (added_content_features.param_type == CPT_FACEDIR_WALLMOUNT) {
					n.param1 = packDir(p_under - p_over);
				}

				core::list<u16> far_players;
				core::map<v3s16, MapBlock*> modified_blocks;
				if (addedcontent >= CONTENT_BED_MIN && addedcontent <= CONTENT_BED_MAX) {
					v3s16 p_foot = v3s16(0,0,0);
					u8 d = n.param2&0x0F;
					switch (d) {
					case 1:
						p_foot.X = 1;
						break;
					case 2:
						p_foot.Z = -1;
						break;
					case 3:
						p_foot.X = -1;
						break;
					default:
						p_foot.Z = 1;
						break;
					}
					MapNode foot = m_env.getMap().getNodeNoEx(p_over+p_foot);
					if (!content_features(foot).buildable_to)
						return;
					foot.setContent(addedcontent);
					foot.param2 &= 0xF0;
					foot.param2 |= d;
					n.setContent(addedcontent|CONTENT_BED_FOOT_MASK);
					sendAddNode(p_over+p_foot, foot, 0, &far_players, 30);
					{
						MapEditEventIgnorer ign(&m_ignore_map_edit_events);

						std::string p_name = std::string(player->getName());
						m_env.getMap().addNodeAndUpdate(p_over+p_foot, foot, modified_blocks, p_name);
					}
				}else if (
					addedcontent >= CONTENT_DOOR_MIN
					&& addedcontent <= CONTENT_DOOR_MAX
					&& (addedcontent&CONTENT_HATCH_MASK) != CONTENT_HATCH_MASK
				) {
					MapNode abv = m_env.getMap().getNodeNoEx(p_over+v3s16(0,1,0));
					MapNode und = m_env.getMap().getNodeNoEx(p_over+v3s16(0,-1,0));
					if (abv.getContent() == CONTENT_AIR) {
						n.setContent(addedcontent&~CONTENT_DOOR_SECT_MASK);
						MapNode m;
						m.setContent(addedcontent|CONTENT_DOOR_SECT_MASK);
						m.param1 = n.param1;
						m.param2 |= n.param2&0x0F;
						sendAddNode(p_over+v3s16(0,1,0), m, 0, &far_players, 30);
						{
							MapEditEventIgnorer ign(&m_ignore_map_edit_events);

							std::string p_name = std::string(player->getName());
							m_env.getMap().addNodeAndUpdate(p_over+v3s16(0,1,0), m, modified_blocks, p_name);
						}
					}else if (und.getContent() == CONTENT_AIR) {
						n.setContent(addedcontent|CONTENT_DOOR_SECT_MASK);
						MapNode m;
						m.setContent(addedcontent&~CONTENT_DOOR_SECT_MASK);
						m.param1 = n.param1;
						m.param2 |= n.param2&0x0F;
						sendAddNode(p_over+v3s16(0,-1,0), m, 0, &far_players, 30);
						{
							MapEditEventIgnorer ign(&m_ignore_map_edit_events);

							std::string p_name = std::string(player->getName());
							m_env.getMap().addNodeAndUpdate(p_over+v3s16(0,-1,0), m, modified_blocks, p_name);
						}
					}else{
						return;
					}
				}

				if (content_features(n).home_node > -1) {
					v3f player_home = intToFloat(p_under,BS);
					player_home.Y += 0.6*BS;
					player->setHome(content_features(n).home_node,player_home);
					std::string msg = std::string("Server: -!- Your home is now set to ")+itos(p_under.X)+","+itos(p_under.Y+1)+","+itos(p_under.Z);
					SendChatMessage(player->peer_id,narrow_to_wide(msg).c_str());
				}

				/*
					Send to all close-by players
				*/
				sendAddNode(p_over, n, 0, &far_players, 30);
				uint16_t idata = item->getData();

				/*
					Handle inventory
				*/
				InventoryList* const ilist = player->inventory.getList("main");
				if (!config_get_bool("world.player.inventory.creative") && ilist) {
					if ((wieldcontent&CONTENT_TOOLITEM_MASK) == CONTENT_TOOLITEM_MASK) {
						ToolItem* const titem = (ToolItem*)wielditem;
						if (titem->addWear(1)) {
							ilist->deleteItem(item_i);
						}else{
							wielditem->setData(0);
							ilist->addDiff(item_i,wielditem);
						}
					}else if ((wieldcontent&CONTENT_CRAFTITEM_MASK) == CONTENT_CRAFTITEM_MASK) {
						wielditem->setData(0);
						ilist->addDiff(item_i,wielditem);
					}else if (wielditem->getCount() == 1) {
						ilist->deleteItem(item_i);
					}else{
						wielditem->remove(1);
						ilist->addDiff(item_i,wielditem);
					}
					// Send inventory
					UpdateCrafting(peer_id);
					SendInventory(peer_id);
				}

				/*
					Add node.

					This takes some time so it is done after the quick stuff
				*/
				{
					MapEditEventIgnorer ign(&m_ignore_map_edit_events);

					std::string p_name = std::string(player->getName());
					m_env.getMap().addNodeAndUpdate(p_over, n, modified_blocks, p_name);
				}

				if (content_features(addedcontent).item_param_type == CPT_METADATA) {
					NodeMetadata *meta = m_env.getMap().getNodeMetadata(p_over);
					if (meta)
						meta->setData(idata);
					sendAddNode(p_over, n, 0, &far_players, 30);
				}
				/*
					Set blocks not sent to far players
				*/
				for(core::list<u16>::Iterator
						i = far_players.begin();
						i != far_players.end(); i++)
				{
					u16 peer_id = *i;
					RemoteClient *client = getClient(peer_id);
					if(client==NULL)
						continue;
					client->SetBlocksNotSent(modified_blocks);
				}

				/*
					Calculate special events
				*/
			}else if (wielded_tool_features.onplace_node != CONTENT_IGNORE) {
				bool replaced_node_exists = false;
				MapNode replaced_node = m_env.getMap().getNodeNoEx(p_over,&replaced_node_exists);
				if (!replaced_node_exists) {
					infostream<<"Server: Ignoring ADDNODE: Node not found"
							<<" Adding block to emerge queue."
							<<std::endl;
					m_emerge_queue.addBlock(peer_id,
							getNodeBlockPos(p_over), BLOCK_EMERGE_FLAG_FROMDISK);
					return;
				}

				content_t replaced_content = replaced_node.getContent();
				ContentFeatures &replaced_node_features = content_features(replaced_content);

				if ((getPlayerPrivs(player)&PRIV_BUILD) == 0) {
					infostream<<"Player "<<player->getName()<<" cannot add node"
						<<" because privileges are "<<getPlayerPrivs(player)
						<<std::endl;
					return;
				}

				if (!replaced_node_features.buildable_to) {
					// Client probably has wrong data.
					// Set block not sent, so that client will get
					// a valid one.
					infostream<<"Client "<<peer_id<<" tried to place"
							<<" node in invalid position; setting"
							<<" MapBlock not sent."<<std::endl;
					RemoteClient *client = getClient(peer_id);
					v3s16 blockpos = getNodeBlockPos(p_over);
					client->SetBlockNotSent(blockpos);
					return;
				}

				if (!config_get_bool("world.player.inventory.creative")) {
					if (wielded_tool_features.onplace_replace_item != CONTENT_IGNORE) {
						u16 wear = ((ToolItem*)wielditem)->getWear();
						InventoryItem* const item = InventoryItem::create(
							wielded_tool_features.onplace_replace_item,1,wear);
						InventoryItem* const citem = ilist->changeItem(item_i,item);
						if (citem)
							delete citem;
					}else{
						ilist->deleteItem(item_i);
					}
					UpdateCrafting(player->peer_id);
					SendInventory(player->peer_id);
				}

				MapNode n(wielded_tool_features.onplace_node);
				core::list<u16> far_players;
				sendAddNode(p_over, n, 0, &far_players, 30);

				/*
					Add node.

					This takes some time so it is done after the quick stuff
				*/
				core::map<v3s16, MapBlock*> modified_blocks;
				{
					MapEditEventIgnorer ign(&m_ignore_map_edit_events);

					std::string p_name = std::string(player->getName());
					m_env.getMap().addNodeAndUpdate(p_over, n, modified_blocks, p_name);
				}
				/*
					Set blocks not sent to far players
				*/
				for(core::list<u16>::Iterator
						i = far_players.begin();
						i != far_players.end(); i++)
				{
					u16 peer_id = *i;
					RemoteClient *client = getClient(peer_id);
					if(client==NULL)
						continue;
					client->SetBlocksNotSent(modified_blocks);
				}
			}else if (
				wielded_craft_features->drop_item != CONTENT_IGNORE
				&& (wielded_craft_features->drop_item&CONTENT_MOB_MASK) != CONTENT_MOB_MASK
			) {
				if ((getPlayerPrivs(player) & PRIV_BUILD) == 0) {
					infostream<<"Not allowing player to drop item: "
							"no build privs"<<std::endl;
					return;
				}
				MapNode n = m_env.getMap().getNodeNoEx(p_over);
				if (n.getContent() != CONTENT_AIR || !item)
					return;
				n.setContent(content_craftitem_features(item->getContent())->drop_item);
				core::list<u16> far_players;
				sendAddNode(p_over, n, 0, &far_players, 30);
				if (!config_get_bool("world.player.inventory.creative")) {
					// Delete the right amount of items from the slot
					u16 dropcount = item->getDropCount();
					InventoryList* const ilist = player->inventory.getList("main");
					// Delete item if all gone
					if (item->getCount() <= dropcount) {
						if (item->getCount() < dropcount)
							infostream<<"WARNING: Server: dropped more items"
								<<" than the slot contains"<<std::endl;
						// Remove from inventory and send inventory
						if (ilist)
							ilist->deleteItem(item_i);
					}
					// Else decrement it
					else{
						item->remove(dropcount);
						ilist->addDiff(item_i,item);
					}
					// Send inventory
					UpdateCrafting(peer_id);
					SendInventory(peer_id);
				}
				/*
				Add node.
				This takes some time so it is done after the quick stuff
				*/
				core::map<v3s16, MapBlock*> modified_blocks;
				{
					MapEditEventIgnorer ign(&m_ignore_map_edit_events);
					std::string p_name = std::string(player->getName());
					m_env.getMap().addNodeAndUpdate(p_over, n, modified_blocks, p_name);
				}
				/*
				Set blocks not sent to far players
				*/
				for(core::list<u16>::Iterator
					i = far_players.begin();
					i != far_players.end(); i++)
				{
					u16 peer_id = *i;
					RemoteClient *client = getClient(peer_id);
					if(client==NULL)
						continue;
					client->SetBlocksNotSent(modified_blocks);
				}
			}
			else if ((wielded_tool_features.param_type == CPT_DROP
					|| wielded_craft_features->param_type == CPT_DROP)
				&& wielditem->getData() != 0)
			{
				v3s16 blockpos = getNodeBlockPos(p_over);

				/*
					Check that the block is loaded so that the item
					can properly be added to the static list too
				*/
				MapBlock* const block = m_env.getMap().getBlockNoCreateNoEx(blockpos);
				if (!block)
				{
					infostream<<"Error while placing object: "
							"block not found"<<std::endl;
					return;
				}

				if (!config_get_bool("world.player.inventory.droppable"))
				{
					InventoryList* const mlist = player->inventory.getList("main");
					mlist->deleteItem(item_i);
				}
				else if ((getPlayerPrivs(player) & PRIV_BUILD) == 0)
				{
					infostream<<"Not allowing player to drop item: no build privs"<<std::endl;
					return;
				}

				// Calculate a position for it
				v3f pos = intToFloat(p_over, BS);
				//pos.Y -= BS*0.45;
				pos.Y -= BS*0.25; // let it drop a bit
				// Randomize a bit
				pos.X += BS*0.2*(float)myrand_range(-1000,1000)/1000.0;
				pos.Z += BS*0.2*(float)myrand_range(-1000,1000)/1000.0;

				/*
					Create the object
				*/
				ServerActiveObject* const obj = wielditem->createSAO(&m_env, 0, pos);

				if (obj == NULL)
				{
					InventoryItem *nitem;
					if (!config_get_bool("world.player.inventory.creative"))
					{
						// Delete the right amount of items from the slot
						InventoryList* const ilist = player->inventory.getList("main");
						nitem = ilist->changeItem(item_i,NULL);
						// Send inventory
						UpdateCrafting(peer_id);
						SendInventory(peer_id);
					}
					else
						nitem = wielditem->clone();
					m_env.dropToParcel(p_over,nitem);
				}
				else
				{
					actionstream<<player->getName()<<" places "<<item->getName()
							<<" at "<<PP(p_over)<<std::endl;

					// Add the object to the environment
					m_env.addActiveObject(obj);

					infostream<<"Placed object"<<std::endl;

					if (!config_get_bool("world.player.inventory.creative")) {
						// Delete the right amount of items from the slot
						InventoryList* const ilist = player->inventory.getList("main");

						wielditem->setData(0);
						ilist->addDiff(item_i,wielditem);

						// Send inventory
						UpdateCrafting(peer_id);
						SendInventory(peer_id);
					}
				}
				block->ResetCurrent();
			}
			else if (wielded_craft_features->teleports > -2)
			{
				s8 dest = wielded_craft_features->teleports;
				/*
					If in creative mode, item dropping is disabled unless
					player has build privileges
				*/
				if ((getPlayerPrivs(player) & PRIV_BUILD) == 0) {
					infostream<<"Not allowing player to drop item: "
							"no build privs"<<std::endl;
					return;
				}
				if (!config_get_bool("world.player.inventory.creative")) {
					// Delete the right amount of items from the slot
					u16 dropcount = item->getDropCount();
					InventoryList* const ilist = player->inventory.getList("main");

					// Delete item if all gone
					if (item->getCount() <= dropcount)
					{
						if (item->getCount() < dropcount)
							infostream<<"WARNING: Server: dropped more items"
									<<" than the slot contains"<<std::endl;

						if (ilist)
							// Remove from inventory and send inventory
							ilist->deleteItem(item_i);
					}else{
						item->remove(dropcount);
						ilist->addDiff(item_i,item);
					}

					// Send inventory
					UpdateCrafting(peer_id);
					SendInventory(peer_id);
				}
				v3f pos;
				if (!player->getHome(dest,pos) && !player->getHome(PLAYERFLAG_HOME,pos))
					pos = findSpawnPos(m_env.getServerMap());
				player->setPosition(pos);
				SendMovePlayer(player);
			}
			/*
				Place other item (not a block)
			*/
			else
			{
				v3s16 blockpos = getNodeBlockPos(p_over);

				/*
					Check that the block is loaded so that the item
					can properly be added to the static list too
				*/
				MapBlock* const block = m_env.getMap().getBlockNoCreateNoEx(blockpos);
				if (!block)
				{
					infostream<<"Error while placing object: "
							"block not found"<<std::endl;
					return;
				}

				if (!config_get_bool("world.player.inventory.droppable"))
				{
					InventoryList *mlist = player->inventory.getList("main");
					mlist->deleteItem(item_i);
				}
				else if ((getPlayerPrivs(player) & PRIV_BUILD) == 0)
				{
					infostream<<"Not allowing player to drop item: no build privs"<<std::endl;
					return;
				}

				// Calculate a position for it
				v3f pos = intToFloat(p_over, BS);
				//pos.Y -= BS*0.45;
				pos.Y -= BS*0.25; // let it drop a bit
				// Randomize a bit
				pos.X += BS*0.2*(float)myrand_range(-1000,1000)/1000.0;
				pos.Z += BS*0.2*(float)myrand_range(-1000,1000)/1000.0;

				/*
					Create the object
				*/
				ServerActiveObject *obj = NULL;
				/* createSAO will drop all craft items, we may not want that */
				if (wielded_craft_features->content == wieldcontent
						&& wielded_craft_features->drop_item == CONTENT_IGNORE)
				{
				    InventoryItem* const ditem = InventoryItem::create(wieldcontent,item->getDropCount());
				    obj = ditem->createSAO(&m_env, 0, pos);
				    delete ditem;
				}
				else
				{
				    InventoryItem* const ditem = ilist->getItem(item_i);

				    if(ditem)
					obj = ditem->createSAO(&m_env, 0, pos);
				}

				if (obj == NULL)
				{
					InventoryItem* nitem = NULL;
					if (!config_get_bool("world.player.inventory.creative"))
					{
						// Delete the right amount of items from the slot
						InventoryList* const ilist = player->inventory.getList("main");
						nitem = ilist->changeItem(item_i,NULL);
						// Send inventory
						UpdateCrafting(peer_id);
						SendInventory(peer_id);
					}
					else
					{
					/* BUG PB : Get again, it can have disapperead. */
					    InventoryItem* const wielditem = (InventoryItem*) player->getWieldItem();

					    if(wielditem)
						nitem = wielditem->clone();
					}
					if(nitem)
					    m_env.dropToParcel(p_over,nitem);
					block->ResetCurrent();
				}
				else
				{
					actionstream<<player->getName()<<" places "<<item->getName()
							<<" at "<<PP(p_over)<<std::endl;

					// Add the object to the environment
					m_env.addActiveObject(obj);

					infostream<<"Placed object"<<std::endl;

					if (!config_get_bool("world.player.inventory.creative"))
					{
						// Delete the right amount of items from the slot
						u16 dropcount = item->getDropCount();
						InventoryList* const ilist = player->inventory.getList("main");

						// Delete item if all gone
						if (item->getCount() <= dropcount) {
							if(item->getCount() < dropcount)
								infostream<<"WARNING: Server: dropped more items"
										<<" than the slot contains"<<std::endl;

							if(ilist)
								// Remove from inventory and send inventory
								ilist->deleteItem(item_i);
						}
						// Else decrement it
						else{
							item->remove(dropcount);
							ilist->addDiff(item_i,item);
						}

						// Send inventory
						UpdateCrafting(peer_id);
						SendInventory(peer_id);
					}
				}
			}

		} // action == 1

		/*
			Catch invalid actions
		*/
		else{
			infostream<<"WARNING: Server: Invalid action " << action << std::endl;
		}
	}
	break;
	case TOSERVER_PLAYERDAMAGE:
	{
		std::string datastring((char*)&data[2], datasize-2);
		std::istringstream is(datastring, std::ios_base::binary);
		s8 damage = readS8(is);
		s8 suffocate = readS8(is);
		s8 hunger = readS8(is);

		if (config_get_bool("world.player.damage")) {
			if (damage) {
				actionstream<<player->getName()<<" damaged by "
						<<(int)damage<<" hp at "<<PP(player->getPosition()/BS)
						<<std::endl;
			}
			if (suffocate && config_get_bool("world.player.suffocation")) {
				actionstream<<player->getName()<<" lost "
						<<(int)suffocate<<" air at "<<PP(player->getPosition()/BS)
						<<std::endl;
			}else{
				suffocate = 0;
			}
			if (hunger && config_get_bool("world.player.hunger")) {
				actionstream<<player->getName()<<" lost "
						<<(int)hunger<<" hunger at "<<PP(player->getPosition()/BS)
						<<std::endl;
			}else{
				hunger = 0;
			}
		}else{
			damage = 0;
			suffocate = 0;
			hunger = 0;
		}

		if (!damage && !suffocate && !hunger) {
			SendPlayerState(player);
		}else{
			HandlePlayerHP(player, damage, suffocate, hunger);
		}
	}
	break;
	case TOSERVER_PLAYERWEAR:
	{
		std::string datastring((char*)&data[2], datasize-2);
		std::istringstream is(datastring, std::ios_base::binary);
		u16 wear = readU16(is);
		if (!config_get_bool("world.player.damage"))
			return;

		f32 bonus = 1.0;
		{
			InventoryList *l = player->inventory.getList("decorative");
			if (l) {
				ClothesItem *i = (ClothesItem*)l->getItem(0);
				if (i) {
					bonus = content_clothesitem_features(i->getContent())->effect;
				}
			}
		}

		const char* list[7] = {"hat","shirt","jacket","decorative","belt","pants","boots"};
		for (int j=0; j<7; j++) {
			InventoryList *l = player->inventory.getList(list[j]);
			if (l == NULL)
				continue;
			ClothesItem *i = (ClothesItem*)l->getItem(0);
			if (i == NULL)
				continue;
			u16 w = wear;
			if (content_clothesitem_features(i->getContent())->durability > 1)
				w /= content_clothesitem_features(i->getContent())->durability;
			if (w < 15)
				w = 15;
			if (bonus > 0.0) {
				f32 fw = w;
				fw /= bonus;
				w = fw;
			}
			if (i->addWear(w)) {
				l->deleteItem(0);
			}else{
				l->addDiff(0,i);
			}
		}
		SendInventory(player->peer_id);
		return;
	}
	break;
	case TOSERVER_INVENTORY_ACTION:
	{
		// Strip command and create a stream
		std::string datastring((char*)&data[2], datasize-2);
		infostream<<"TOSERVER_INVENTORY_ACTION: data="<<datastring<<std::endl;
		std::istringstream is(datastring, std::ios_base::binary);
		// Create an action
		InventoryAction *a = InventoryAction::deSerialize(is);
		if(a != NULL)
		{
			// Create context
			InventoryContext c;
			c.current_player = player;

			/*
				Handle craftresult specially if not in creative mode
			*/
			bool disable_action = false;
			if (a->getType() == IACTION_MOVE && !config_get_bool("world.player.inventory.creative")) {
				IMoveAction *ma = (IMoveAction*)a;
				if(ma->to_inv == "current_player" && ma->from_inv == "current_player") {
					InventoryList *rlist = player->inventory.getList("craftresult");
					assert(rlist);
					InventoryList *clist = player->inventory.getList("craft");
					assert(clist);
					InventoryList *mlist = player->inventory.getList("main");
					assert(mlist);
					/*
						Craftresult is no longer preview if something
						is moved into it
					*/
					if (ma->to_list == "craftresult" && ma->from_list != "craftresult") {
						// If it currently is a preview, remove
						// its contents
						if (player->craftresult_is_preview) {
							rlist->deleteItem(0);
						}
						player->craftresult_is_preview = false;
					}
					/*
						Crafting takes place if this condition is true.
					*/
					bool craft_occurred = false;
					if (player->craftresult_is_preview && ma->from_list == "craftresult") {
						InventoryItem *item = rlist->getItem(0);
						if (item && mlist->roomForItem(item)) {
							player->craftresult_is_preview = false;
							clist->decrementMaterials(1);
							craft_occurred = true;

							std::string itemname = "NULL";
							if (item)
								itemname = item->getName();
							actionstream<<player->getName()<<" crafts "<<itemname<<std::endl;
						}else{
							disable_action = true;
						}
					}
					/*
						If the craftresult is placed on itself, move it to
						main inventory instead of doing the action
					*/
					if (craft_occurred && ma->to_list == "craftresult" && ma->from_list == "craftresult") {
						disable_action = true;

						InventoryItem *item1 = rlist->changeItem(0, NULL);
						mlist->addItem(item1);
					}
				}
				// Disallow moving items if not allowed to build
				else if((getPlayerPrivs(player) & PRIV_BUILD) == 0) {
					return;
				}
				// if it's a locking inventory, only allow the owner or server admins to move items
				else if (ma->from_inv != "current_player" && (getPlayerPrivs(player) & PRIV_SERVER) == 0) {
					Strfnd fn(ma->from_inv);
					std::string id0 = fn.next(":");
					if (id0 == "nodemeta") {
						v3s16 p;
						p.X = mystoi(fn.next(","));
						p.Y = mystoi(fn.next(","));
						p.Z = mystoi(fn.next(","));
						NodeMetadata *meta = m_env.getMap().getNodeMetadata(p);
						if (meta) {
							if (meta->typeId() == CONTENT_CHEST) {
								ChestNodeMetadata *lcm = (ChestNodeMetadata*)meta;
								if (lcm->getInventoryOwner() != "" && lcm->getInventoryOwner() != player->getName())
									return;
							}else if (meta->typeId() == CONTENT_FURNACE) {
								FurnaceNodeMetadata *lfm = (FurnaceNodeMetadata*)meta;
								if (lfm->getInventoryOwner() != "" && lfm->getInventoryOwner() != player->getName())
									return;
							}else if (meta->typeId() == CONTENT_CRUSHER) {
								CrusherNodeMetadata *lcm = (CrusherNodeMetadata*)meta;
								if (lcm->getInventoryOwner() != "" && lcm->getInventoryOwner() != player->getName())
									return;
							}else if (meta->typeId() == CONTENT_LOCKABLE_CHEST_DEPRECATED) {
								LockingDeprecatedChestNodeMetadata *lcm = (LockingDeprecatedChestNodeMetadata*)meta;
								if (lcm->getInventoryOwner() != player->getName())
									return;
							}else if (meta->typeId() == CONTENT_LOCKABLE_FURNACE_DEPRECATED) {
								LockingDeprecatedFurnaceNodeMetadata *lfm = (LockingDeprecatedFurnaceNodeMetadata*)meta;
								std::string name = lfm->getInventoryOwner();
								if (name != "" && name != player->getName() && lfm->getOwner() != player->getName())
									return;
							}else if (meta->typeId() == CONTENT_SAFE) {
								SafeNodeMetadata *sm = (SafeNodeMetadata*)meta;
								std::string name = sm->getInventoryOwner();
								if (name != "" && name != player->getName() && sm->getOwner() != player->getName())
									return;
							}
						}
					}
				}
				else if (ma->to_inv != "current_player" && (getPlayerPrivs(player) & PRIV_SERVER) == 0) {
					Strfnd fn(ma->to_inv);
					std::string id0 = fn.next(":");
					if(id0 == "nodemeta")
					{
						v3s16 p;
						p.X = mystoi(fn.next(","));
						p.Y = mystoi(fn.next(","));
						p.Z = mystoi(fn.next(","));
						NodeMetadata *meta = m_env.getMap().getNodeMetadata(p);
						if (meta) {
							if (meta->typeId() == CONTENT_LOCKABLE_CHEST_DEPRECATED) {
								LockingDeprecatedChestNodeMetadata *lcm = (LockingDeprecatedChestNodeMetadata*)meta;
								if (lcm->getInventoryOwner() != player->getName())
									return;
							}else if (meta->typeId() == CONTENT_LOCKABLE_FURNACE_DEPRECATED) {
								LockingDeprecatedFurnaceNodeMetadata *lfm = (LockingDeprecatedFurnaceNodeMetadata*)meta;
								std::string name = lfm->getInventoryOwner();
								// no owner and inserting to src, claim ownership of the inventory
								if ((name == "" || lfm->getOwner() == player->getName()) && ma->to_list == "src") {
									lfm->setInventoryOwner(player->getName());
								}else if (name != "" && name != player->getName()) {
									return;
								}
							}else if (meta->typeId() == CONTENT_SAFE) {
								SafeNodeMetadata *sm = (SafeNodeMetadata*)meta;
								std::string name = sm->getInventoryOwner();
								if (name != "" && name != player->getName() && sm->getOwner() != player->getName())
									return;
							}
						}
					}
				}
			}

			if (disable_action == false) {
				// Feed action to player inventory
				a->apply(&c, this);
				if (a->getType() == IACTION_MOVE) {
					IMoveAction *ma = (IMoveAction*)a;
					if (
						ma->to_inv == "current_player"
						&& ma->from_inv == "current_player"
						&& ma->to_list == "discard"
					) {
						InventoryList* const list = player->inventory.getList("discard");
						InventoryItem* item = list->getItem(0);
						if (item) {
							if (config_get_bool("world.player.inventory.droppable")) {
								v3f pos = player->getPosition();
								pos.Y += BS;
								v3f dir = v3f(0,0,BS);
								dir.rotateXZBy(player->getYaw());
								pos += dir;
								v3s16 pp = floatToInt(pos,BS);
								item = item->clone();
								m_env.dropToParcel(pp,item);
							}
							if (!config_get_bool("world.player.inventory.creative")) {
								list->deleteItem(0);
								SendInventory(player->peer_id);
							}
						}
					}
				}
				// Eat the action
				delete a;
			}else{
				// Send inventory
				UpdateCrafting(player->peer_id);
				SendInventory(player->peer_id);
			}
		}
		else
		{
			infostream<<"TOSERVER_INVENTORY_ACTION: "
					<<"InventoryAction::deSerialize() returned NULL"
					<<std::endl;
		}
	}
	break;
	case TOSERVER_CHAT_MESSAGE:
	{
		/*
			u16 command
			u16 length
			wstring message
		*/
		u8 buf[6];
		std::string datastring((char*)&data[2], datasize-2);
		std::istringstream is(datastring, std::ios_base::binary);

		// Read stuff
		is.read((char*)buf, 2);
		u16 len = readU16(buf);

		std::string message;
		for (u16 i=0; i<len; i++) {
			is.read((char*)buf, 1);
			message += buf[0];
		}

		// Get player name of this client
		const char* name = player->getName();

		// Line to send to players
		std::wstring line;
		// Whether to send to the player that sent the line
		bool send_to_sender = false;
		// Whether to send to other players
		bool send_to_others = false;

		// Local player gets all privileges regardless of
		// what's set on their account.
		uint64_t privs = getPlayerPrivs(player);

		// Parse commands
		if (message[0] == L'/') {
			command_context_t ctx;

			strcpy(ctx.player,name);
			ctx.privs = privs;
			ctx.flags = 0;
			ctx.bridge_server = this;
			ctx.bridge_env = &m_env;
			ctx.bridge_player = player;

			if (command_exec(&ctx,(char*)message.c_str())) {
				if (ctx.flags) {
					line += narrow_to_wide(ctx.reply);
				}else{
					line += L"Server: Invalid command";
				}
				send_to_sender = true;
			}else if (ctx.flags) {
				std::wstring reply(narrow_to_wide(ctx.reply));
				send_to_sender = ctx.flags & SEND_TO_SENDER;
				send_to_others = ctx.flags & SEND_TO_OTHERS;

				if (ctx.flags & SEND_NO_PREFIX) {
					line += reply;
				}else{
					line += L"Server: " + reply;
				}
			}
		}else{
			if (privs & PRIV_SHOUT) {
				line += L"<";
				line += narrow_to_wide(name);
				line += L"> ";
				line += narrow_to_wide(message);
				send_to_others = true;
			}else{
				line += L"Server: You are not allowed to shout";
				send_to_sender = true;
			}
		}

		if (line != L"") {
			if (send_to_others)
				actionstream<<"CHAT: "<<wide_to_narrow(line)<<std::endl;

			/*
				Send the message to clients
			*/
			for (core::map<u16, RemoteClient*>::Iterator i = m_clients.getIterator(); i.atEnd() == false; i++) {
				// Get client and check that it is valid
				RemoteClient *client = i.getNode()->getValue();
				assert(client->peer_id == i.getNode()->getKey());
				if(client->serialization_version == SER_FMT_VER_INVALID)
					continue;

				// Filter recipient
				bool sender_selected = (peer_id == client->peer_id);
				if (sender_selected == true && send_to_sender == false)
					continue;
				if (sender_selected == false && send_to_others == false)
					continue;

				SendChatMessage(client->peer_id, line);
			}
		}
	}
	break;
	case TOSERVER_PASSWORD:
	{
		/*
			[0] u16 TOSERVER_PASSWORD
			[2] u8[28] old password
			[30] u8[28] new password
		*/

		if(datasize != 2+PASSWORD_SIZE*2)
			return;
		/*char password[PASSWORD_SIZE];
		for(u32 i=0; i<PASSWORD_SIZE-1; i++)
			password[i] = data[2+i];
		password[PASSWORD_SIZE-1] = 0;*/
		std::string oldpwd;
		for(u32 i=0; i<PASSWORD_SIZE-1; i++)
		{
			char c = data[2+i];
			if(c == 0)
				break;
			oldpwd += c;
		}
		std::string newpwd;
		for(u32 i=0; i<PASSWORD_SIZE-1; i++)
		{
			char c = data[2+PASSWORD_SIZE+i];
			if(c == 0)
				break;
			newpwd += c;
		}

		if (!base64_is_valid(newpwd)) {
			infostream<<"Server: "<<player->getName()<<" supplied invalid password hash"<<std::endl;
			SendChatMessage(peer_id, L"Invalid new password hash supplied. Password NOT changed.");
			return;
		}

		infostream<<"Server: Client requests a password change from "
				<<"'"<<oldpwd<<"' to '"<<newpwd<<"'"<<std::endl;

		std::string playername = player->getName();

		if (!auth_exists(const_cast<char*>(playername.c_str()))) {
			infostream<<"Server: playername not found in authmanager"<<std::endl;
			// Wrong old password supplied!!
			SendChatMessage(peer_id, L"playername not found in authmanager");
			return;
		}

		char checkpwd[64];

		if (auth_getpwd(const_cast<char*>(playername.c_str()),checkpwd) || oldpwd != checkpwd) {
			infostream<<"Server: invalid old password"<<std::endl;
			// Wrong old password supplied!!
			SendChatMessage(peer_id, L"Invalid old password supplied. Password NOT changed.");
			return;
		}

		actionstream<<player->getName()<<" changes password"<<std::endl;

		auth_setpwd(const_cast<char*>(playername.c_str()),const_cast<char*>(newpwd.c_str()));

		infostream<<"Server: password change successful for "<<playername
				<<std::endl;
		SendChatMessage(peer_id, L"Password change successful");
	}
	break;
	case TOSERVER_PLAYERITEM:
	{
		if (datasize < 2+2)
			return;

		u16 item = readU16(&data[2]);
		player->wieldItem(item);
		SendPlayerItems(player);
	}
	break;
	case TOSERVER_RESPAWN:
	{
		if(player->health != 0)
			return;

		RespawnPlayer(player);

		actionstream<<player->getName()<<" respawns at "
				<<PP(player->getPosition()/BS)<<std::endl;
	}
	break;
	case TOSERVER_NODEMETA_FIELDS:
	{
		std::string datastring((char*)&data[2], datasize-2);
		std::istringstream is(datastring, std::ios_base::binary);
		u8 buf[6];
		// Read p
		is.read((char*)buf, 6);
		v3s16 p = readV3S16(buf);

		// Read formname
		std::string formname = deSerializeString(is);

		// Read number of fields
		is.read((char*)buf, 2);
		u16 num = readU16(buf);

		// Read Fields
		std::map<std::string, std::string> fields;
		for (int k=0; k<num; k++) {
			std::string fieldname = deSerializeString(is);
			std::string fieldvalue = deSerializeLongString(is);
			fields[fieldname] = fieldvalue;
		}

		NodeMetadata *meta = m_env.getMap().getNodeMetadata(p);
		if (!meta)
			return;

		if (meta->receiveFields(formname,fields,player))
		{
			v3s16 blockpos = getNodeBlockPos(p);
			MapBlock* const block = m_env.getMap().getBlockNoCreateNoEx(blockpos);
			if (block)
			{
				block->setChangedFlag();
				block->ResetCurrent();
			}
			
			for(core::map<u16, RemoteClient*>::Iterator
				i = m_clients.getIterator();
				i.atEnd()==false; i++)
			{
				RemoteClient *client = i.getNode()->getValue();
				client->SetBlockNotSent(blockpos);
			}
		}
	}
	break;
	default:
	{
		infostream<<"Server::ProcessData(): Ignoring "
				"unknown command "<<command<<std::endl;
	}
	}

	} //try
	catch(SendFailedException &e)
	{
		errorstream<<"Server::ProcessData(): SendFailedException: "
				<<"what="<<e.what()
				<<std::endl;
	}
}

void Server::onMapEditEvent(MapEditEvent *event)
{
	//infostream<<"Server::onMapEditEvent()"<<std::endl;
	if(m_ignore_map_edit_events)
		return;
	MapEditEvent *e = event->clone();
	m_unsent_map_edit_queue.push_back(e);
}

Inventory* Server::getInventory(InventoryContext *c, std::string id)
{
	if (id == "current_player") {
		assert(c->current_player);
		return &(c->current_player->inventory);
	}

	Strfnd fn(id);
	std::string id0 = fn.next(":");

	if (id0 == "nodemeta") {
		v3s16 p;
		p.X = mystoi(fn.next(","));
		p.Y = mystoi(fn.next(","));
		p.Z = mystoi(fn.next(","));
		NodeMetadata *meta = m_env.getMap().getNodeMetadata(p);
		if (meta)
			return meta->getInventory();
		infostream<<"nodemeta at ("<<p.X<<","<<p.Y<<","<<p.Z<<"): "
				<<"no metadata found"<<std::endl;
		return NULL;
	}

	infostream<<__FUNCTION_NAME<<": unknown id "<<id<<std::endl;
	return NULL;
}

void Server::inventoryModified(InventoryContext *c, std::string id)
{
	if (id == "current_player")
	{
		assert(c->current_player);
		// Send inventory
		UpdateCrafting(c->current_player->peer_id);
		SendInventory(c->current_player->peer_id);
		return;
	}

	Strfnd fn(id);
	std::string id0 = fn.next(":");

	if (id0 == "nodemeta")
	{
		v3s16 p;
		p.X = mystoi(fn.next(","));
		p.Y = mystoi(fn.next(","));
		p.Z = mystoi(fn.next(","));
		v3s16 blockpos = getNodeBlockPos(p);

		NodeMetadata* const meta = m_env.getMap().getNodeMetadata(p);
		if (meta)
			meta->inventoryModified();

		MapBlock* const block = m_env.getMap().getBlockNoCreateNoEx(blockpos);
		if (block)
		{
			block->raiseModified(MOD_STATE_WRITE_NEEDED);
			block->ResetCurrent();
		}
		
		setBlockNotSent(blockpos);
		return;
	}

	infostream<<__FUNCTION_NAME<<": unknown id "<<id<<std::endl;
}

core::list<PlayerInfo> Server::getPlayerInfo()
{
	DSTACK(__FUNCTION_NAME);
	JMutexAutoLock envlock(m_env_mutex);
	JMutexAutoLock conlock(m_con_mutex);

	core::list<PlayerInfo> list;

	array_t *players = m_env.getPlayers();

	Player *player;
	uint32_t i;
	for (i=0; i<players->length; i++) {
		player = (Player*)array_get_ptr(players,i);
		if (!player)
			continue;
		PlayerInfo info;

		try{
			// Copy info from connection to info struct
			info.id = player->peer_id;
			info.address = m_con.GetPeerAddress(player->peer_id);
			info.avg_rtt = m_con.GetPeerAvgRTT(player->peer_id);
		}
		catch(con::PeerNotFoundException &e)
		{
			// Set dummy peer info
			info.id = 0;
			info.address = Address(0,0,0,0,0);
			info.avg_rtt = 0.0;
		}

		snprintf(info.name, PLAYERNAME_SIZE, "%s", player->getName());
		info.position = player->getPosition();

		list.push_back(info);
	}

	return list;
}


void Server::peerAdded(con::Peer *peer)
{
	DSTACK(__FUNCTION_NAME);
	infostream<<"Server::peerAdded(): peer->id="
			<<peer->id<<std::endl;

	PeerChange c;
	c.type = PEER_ADDED;
	c.peer_id = peer->id;
	c.timeout = false;
	m_peer_change_queue.push_back(c);
}

void Server::deletingPeer(con::Peer *peer, bool timeout)
{
	DSTACK(__FUNCTION_NAME);
	infostream<<"Server::deletingPeer(): peer->id="
			<<peer->id<<", timeout="<<timeout<<std::endl;

	PeerChange c;
	c.type = PEER_REMOVED;
	c.peer_id = peer->id;
	c.timeout = timeout;
	m_peer_change_queue.push_back(c);
}

/*
	Static send methods
*/

void Server::SendState(
	con::Connection &con,
	u16 peer_id,
	u8 health,
	u8 air,
	u8 hunger,
	u8 dirt,
	u8 wet,
	u8 blood,
	u16 energy_effect,
	u16 cold_effect
)
{
	DSTACK(__FUNCTION_NAME);
	std::ostringstream os(std::ios_base::binary);

	writeU16(os, TOCLIENT_PLAYERSTATE);
	writeU8(os, health);
	writeU8(os, air);
	writeU8(os, hunger);
	writeU8(os, dirt);
	writeU8(os, wet);
	writeU8(os, blood);
	writeU16(os, energy_effect);
	writeU16(os, cold_effect);

	// Make data buffer
	std::string s = os.str();
	SharedBuffer<u8> data((u8*)s.c_str(), s.size());
	// Send as reliable
	con.Send(peer_id, 0, data, true);
}

void Server::SendAccessDenied(con::Connection &con, u16 peer_id,
		const std::wstring &reason)
{
	DSTACK(__FUNCTION_NAME);
	std::ostringstream os(std::ios_base::binary);

	writeU16(os, TOCLIENT_ACCESS_DENIED);
	os<<serializeWideString(reason);

	// Make data buffer
	std::string s = os.str();
	SharedBuffer<u8> data((u8*)s.c_str(), s.size());
	// Send as reliable
	con.Send(peer_id, 0, data, true);
}

void Server::SendDeathscreen(con::Connection &con, u16 peer_id,
		bool set_camera_point_target, v3f camera_point_target)
{
	DSTACK(__FUNCTION_NAME);
	std::ostringstream os(std::ios_base::binary);

	writeU16(os, TOCLIENT_DEATHSCREEN);
	writeU8(os, set_camera_point_target);
	writeV3F1000(os, camera_point_target);

	// Make data buffer
	std::string s = os.str();
	SharedBuffer<u8> data((u8*)s.c_str(), s.size());
	// Send as reliable
	con.Send(peer_id, 0, data, true);
}

/*
	Non-static send methods
*/

void Server::SendPlayerInfo(float dtime)
{
	DSTACK(__FUNCTION_NAME);

	JMutexAutoLock lock1(m_env_mutex);
	JMutexAutoLock lock2(m_con_mutex);

	std::ostringstream os(std::ios_base::binary);
	u8 buf[12];

	// Write command
	writeU16(buf, TOCLIENT_PLAYERINFO);
	os.write((char*)buf, 2);

	// Get connected players
	array_t *players = m_env.getPlayers(true);

	// players ignore their own data, so don't bother sending for one player
	if (players->length < 2)
		return;

	// Write player count
	u16 playercount = players->length;
	writeU16(buf, playercount);
	os.write((char*)buf, 2);

	ServerRemotePlayer *player;
	uint32_t i;
	for (i=0; i<players->length; i++) {
		player = (ServerRemotePlayer*)array_get_ptr(players,i);
		if (!player)
			continue;

		writeU16(os, player->peer_id);
		writeV3F1000(os, player->getPosition());
		writeV3F1000(os, player->getSpeed());
		writeF1000(os, player->getPitch());
		writeF1000(os, player->getYaw());
		writeU8(os, player->animation_id);
		writeU16(os, player->pointed_id);
	}

	array_free(players,1);

	// Make data buffer
	std::string s = os.str();
	SharedBuffer<u8> data((u8*)s.c_str(), s.size());
	// Send as unreliable
	m_con.SendToAll(0, data, false);
}

void Server::SendPlayerData()
{
	DSTACK(__FUNCTION_NAME);

	// Get connected players
	array_t *players = m_env.getPlayers(true);

	std::ostringstream os(std::ios_base::binary);
	writeU16(os, TOCLIENT_PLAYERDATA);
	writeU16(os,(u16)players->length);
	// this is the number of serialized strings sent below
	writeU16(os,1);
	char name[PLAYERNAME_SIZE];

	Player *player;
	uint32_t i;
	for (i=0; i<players->length; i++) {
		player = (Player*)array_get_ptr(players,i);
		if (!player)
			continue;

		writeU16(os, player->peer_id);
		memset(name, 0, PLAYERNAME_SIZE);
		snprintf(name, PLAYERNAME_SIZE, "%s", player->getName());
		os.write(name,PLAYERNAME_SIZE);
		os<<serializeString(player->getCharDef());
		// serialized strings can be added here
	}

	array_free(players,1);

	std::string s = os.str();
	SharedBuffer<u8> data((u8*)s.c_str(), s.size());

	// Send as reliable
	m_con.SendToAll(0, data, true);
}

void Server::SendInventory(u16 peer_id, bool full)
{
	DSTACK(__FUNCTION_NAME);

	Player* player = m_env.getPlayer(peer_id);
	assert(player);

	// get also clears, so get diff for full and partial sends
	InventoryDiff idiff = player->inventory.getDiff();

	if (full) {
		/*
			Serialize it
		*/

		std::ostringstream os(std::ios_base::binary);

		writeU16(os,TOCLIENT_INVENTORY);
		player->inventory.serialize(os);

		// Make data buffer
		std::string s = os.str();
		SharedBuffer<u8> data((u8*)s.c_str(), s.size());

		// Send as reliable
		m_con.Send(peer_id, 0, data, true);
		return;
	}
	{
		if (idiff.m_data.size() == 0)
			return;

		std::ostringstream os(std::ios_base::binary);

		writeU16(os, TOCLIENT_INVENTORY_UPDATE);
		writeU16(os, idiff.m_data.size());
		for (std::map<std::string,std::map<u32,InventoryDiffData> >::iterator l = idiff.m_data.begin(); l != idiff.m_data.end(); l++) {
			os<<serializeString(l->first);
			writeU16(os,l->second.size());
			for (std::map<u32,InventoryDiffData>::iterator i = l->second.begin(); i != l->second.end(); i++) {
				writeU16(os,i->second.index);
				writeU16(os,i->second.type);
				writeU16(os,i->second.wear_count);
				writeU16(os,i->second.data);
			}
		}

		// Make data buffer
		std::string s = os.str();
		SharedBuffer<u8> data((u8*)s.c_str(), s.size());

		// Send as reliable
		m_con.Send(peer_id, 0, data, true);
	}
}

void Server::SendPlayerItems()
{
	DSTACK(__FUNCTION_NAME);

	std::ostringstream os(std::ios_base::binary);
	array_t *players = m_env.getPlayers(true);

	writeU16(os, TOCLIENT_PLAYERITEMS);
	writeU16(os, players->length);
	writeU16(os, 8);

	Player *player;
	uint32_t i;
	for (i=0; i<players->length; i++) {
		player = (Player*)array_get_ptr(players,i);
		if (!player)
			continue;

		writeU16(os, player->peer_id);
		InventoryItem* const item = (InventoryItem*)player->getWieldItem();
		if (item == NULL) {
			writeU16(os,CONTENT_IGNORE);
		}else{
			writeU16(os,item->getContent());
		}
		const char* list[7] = {"hat","shirt","pants","boots","decorative","jacket","belt"};
		for (int j=0; j<7; j++) {
			InventoryList* const l = player->inventory.getList(list[j]);
			if (l == NULL)
				continue;
			InventoryItem* const itm = l->getItem(0);
			if (itm == NULL) {
				writeU16(os,CONTENT_IGNORE);
				continue;
			}
			writeU16(os,itm->getContent());
		}
	}

	array_free(players,1);

	// Make data buffer
	std::string s = os.str();
	SharedBuffer<u8> data((u8*)s.c_str(), s.size());

	m_con.SendToAll(0, data, true);
}

void Server::SendPlayerItems(Player *player)
{
	DSTACK(__FUNCTION_NAME);

	std::ostringstream os(std::ios_base::binary);

	writeU16(os, TOCLIENT_PLAYERITEMS);
	writeU16(os, 1);
	writeU16(os, 8);
	writeU16(os, player->peer_id);
	InventoryItem* const item = (InventoryItem*)player->getWieldItem();
	if (item == NULL) {
		writeU16(os,CONTENT_IGNORE);
	}else{
		writeU16(os,item->getContent());
	}
	const char* list[7] = {"hat","shirt","pants","boots","decorative","jacket","belt"};
	for (int j=0; j<7; j++) {
		InventoryList* const l = player->inventory.getList(list[j]);
		if (l == NULL)
			continue;
		InventoryItem* const itm = l->getItem(0);
		if (itm == NULL) {
			writeU16(os,CONTENT_IGNORE);
			continue;
		}
		writeU16(os,itm->getContent());
	}

	// Make data buffer
	std::string s = os.str();
	SharedBuffer<u8> data((u8*)s.c_str(), s.size());

	m_con.SendToAll(0, data, true);
}

void Server::SendChatMessage(u16 peer_id, const std::wstring &message)
{
	DSTACK(__FUNCTION_NAME);

	std::ostringstream os(std::ios_base::binary);
	u8 buf[12];

	// Write command
	writeU16(buf, TOCLIENT_CHAT_MESSAGE);
	os.write((char*)buf, 2);

	// Write length
	writeU16(buf, message.size());
	os.write((char*)buf, 2);

	// Write string
	for (u32 i=0; i<message.size(); i++) {
		u16 w = message[i];
		writeU16(buf, w);
		os.write((char*)buf, 2);
	}

	// Make data buffer
	std::string s = os.str();
	SharedBuffer<u8> data((u8*)s.c_str(), s.size());
	// Send as reliable
	m_con.Send(peer_id, 0, data, true);
}

void Server::BroadcastChatMessage(const std::wstring &message)
{
	for (core::map<u16, RemoteClient*>::Iterator i = m_clients.getIterator(); i.atEnd() == false; i++) {
		// Get client and check that it is valid
		RemoteClient *client = i.getNode()->getValue();
		assert(client->peer_id == i.getNode()->getKey());
		if(client->serialization_version == SER_FMT_VER_INVALID)
			continue;

		SendChatMessage(client->peer_id, message);
	}
}

void Server::SendPlayerState(Player *player)
{
	u8 hp = player->health;
	u8 air = player->air;
	u8 hunger = player->hunger;
	if (hp < 100 && !config_get_bool("world.player.damage"))
		hp = 100;
	if (air < 100 && !config_get_bool("world.player.suffocation"))
		air = 100;
	if (hunger < 100 && !config_get_bool("world.player.hunger"))
		hunger = 100;
	SendState(
		m_con,
		player->peer_id,
		hp,
		air,
		hunger,
		player->dirt,
		player->wet,
		player->blood,
		player->energy_effect,
		player->cold_effect
	);
	player->energy_effect = 0;
	player->cold_effect = 0;
}

void Server::SendSettings(Player *player)
{
	std::ostringstream os(std::ios_base::binary);

	u8 setting;

	writeU16(os, TOCLIENT_SERVERSETTINGS);
	setting = config_get_bool("world.player.damage");
	writeU8(os, setting);
	setting = config_get_bool("world.player.suffocation");
	writeU8(os, setting);
	setting = config_get_bool("world.player.hunger");
	writeU8(os, setting);

	// Make data buffer
	std::string s = os.str();
	SharedBuffer<u8> data((u8*)s.c_str(), s.size());
	// Send as reliable
	m_con.Send(player->peer_id, 0, data, true);
}

void Server::SendMovePlayer(Player *player)
{
	DSTACK(__FUNCTION_NAME);
	std::ostringstream os(std::ios_base::binary);

	writeU16(os, TOCLIENT_MOVE_PLAYER);
	writeV3F1000(os, player->getPosition());
	writeF1000(os, player->getPitch());
	writeF1000(os, player->getYaw());

	{
		v3f pos = player->getPosition();
		f32 pitch = player->getPitch();
		f32 yaw = player->getYaw();
		infostream<<"Server sending TOCLIENT_MOVE_PLAYER"
				<<" pos=("<<pos.X<<","<<pos.Y<<","<<pos.Z<<")"
				<<" pitch="<<pitch
				<<" yaw="<<yaw
				<<std::endl;
	}

	// Make data buffer
	std::string s = os.str();
	SharedBuffer<u8> data((u8*)s.c_str(), s.size());
	// Send as reliable
	m_con.Send(player->peer_id, 0, data, true);

	{
		std::string snd = "env-teleport";
		SendEnvEvent(ENV_EVENT_SOUND,player->getPosition(),snd,NULL);
	}
}

void Server::sendRemoveNode(v3s16 p, u16 ignore_id,
	core::list<u16> *far_players, float far_d_nodes)
{
	float maxd = far_d_nodes*BS;
	v3f p_f = intToFloat(p, BS);

	// Create packet
	u32 replysize = 8;
	SharedBuffer<u8> reply(replysize);
	writeU16(&reply[0], TOCLIENT_REMOVENODE);
	writeS16(&reply[2], p.X);
	writeS16(&reply[4], p.Y);
	writeS16(&reply[6], p.Z);

	for(core::map<u16, RemoteClient*>::Iterator
		i = m_clients.getIterator();
		i.atEnd() == false; i++)
	{
		// Get client and check that it is valid
		RemoteClient *client = i.getNode()->getValue();
		assert(client->peer_id == i.getNode()->getKey());
		if(client->serialization_version == SER_FMT_VER_INVALID)
			continue;

		// Don't send if it's the same one
		if(client->peer_id == ignore_id)
			continue;

		if(far_players)
		{
			// Get player
			Player *player = m_env.getPlayer(client->peer_id);
			if(player)
			{
				// If player is far away, only set modified blocks not sent
				v3f player_pos = player->getPosition();
				if(player_pos.getDistanceFrom(p_f) > maxd)
				{
					far_players->push_back(client->peer_id);
					continue;
				}
			}
		}

		// Send as reliable
		m_con.Send(client->peer_id, 0, reply, true);
	}
}

void Server::sendAddNode(v3s16 p, MapNode n, u16 ignore_id,
		core::list<u16> *far_players, float far_d_nodes)
{
	float maxd = far_d_nodes*BS;
	v3f p_f = intToFloat(p, BS);

	for(core::map<u16, RemoteClient*>::Iterator
		i = m_clients.getIterator();
		i.atEnd() == false; i++)
	{
		// Get client and check that it is valid
		RemoteClient *client = i.getNode()->getValue();
		assert(client->peer_id == i.getNode()->getKey());
		if(client->serialization_version == SER_FMT_VER_INVALID)
			continue;

		// Don't send if it's the same one
		if(client->peer_id == ignore_id)
			continue;

		if(far_players)
		{
			// Get player
			Player *player = m_env.getPlayer(client->peer_id);
			if(player)
			{
				// If player is far away, only set modified blocks not sent
				v3f player_pos = player->getPosition();
				if(player_pos.getDistanceFrom(p_f) > maxd)
				{
					far_players->push_back(client->peer_id);
					continue;
				}
			}
		}

		// Create packet
		u32 replysize = 8 + MapNode::serializedLength(client->serialization_version);
		SharedBuffer<u8> reply(replysize);
		writeU16(&reply[0], TOCLIENT_ADDNODE);
		writeS16(&reply[2], p.X);
		writeS16(&reply[4], p.Y);
		writeS16(&reply[6], p.Z);
		n.serialize(&reply[8], client->serialization_version);

		// Send as reliable
		m_con.Send(client->peer_id, 0, reply, true);
	}
}

void Server::setBlockNotSent(v3s16 p)
{
	for(core::map<u16, RemoteClient*>::Iterator
		i = m_clients.getIterator();
		i.atEnd()==false; i++)
	{
		RemoteClient *client = i.getNode()->getValue();
		client->SetBlockNotSent(p);
	}
}

void Server::SendBlockNoLock(u16 peer_id, MapBlock *block, u8 ver)
{
	DSTACK(__FUNCTION_NAME);

	v3s16 p = block->getPos();

#if 0
	// Analyze it a bit
	bool completely_air = true;
	for(s16 z0=0; z0<MAP_BLOCKSIZE; z0++)
	for(s16 x0=0; x0<MAP_BLOCKSIZE; x0++)
	for(s16 y0=0; y0<MAP_BLOCKSIZE; y0++)
	{
		if(block->getNodeNoEx(v3s16(x0,y0,z0)).d != CONTENT_AIR)
		{
			completely_air = false;
			x0 = y0 = z0 = MAP_BLOCKSIZE; // Break out
		}
	}

	// Print result
	infostream<<"Server: Sending block ("<<p.X<<","<<p.Y<<","<<p.Z<<"): ";
	if(completely_air)
		infostream<<"[completely air] ";
	infostream<<std::endl;
#endif

	/*
		Create a packet with the block in the right format
	*/

	std::ostringstream os(std::ios_base::binary);
	block->serialize(os, ver);
	std::string s = os.str();
	SharedBuffer<u8> blockdata((u8*)s.c_str(), s.size());

	u32 replysize = 8 + blockdata.getSize();
	SharedBuffer<u8> reply(replysize);
	writeU16(&reply[0], TOCLIENT_BLOCKDATA);
	writeS16(&reply[2], p.X);
	writeS16(&reply[4], p.Y);
	writeS16(&reply[6], p.Z);
	memcpy(&reply[8], *blockdata, blockdata.getSize());

	/*infostream<<"Server: Sending block ("<<p.X<<","<<p.Y<<","<<p.Z<<")"
			<<":  \tpacket size: "<<replysize<<std::endl;*/

	/*
		Send packet
	*/
	m_con.Send(peer_id, 1, reply, true);
}

void Server::SendBlocks(float dtime)
{
	DSTACK(__FUNCTION_NAME);

	JMutexAutoLock envlock(m_env_mutex);
	JMutexAutoLock conlock(m_con_mutex);

	int max = config_get_int("server.net.chunk.max");

	//TimeTaker timer("Server::SendBlocks");

	core::array<PrioritySortedBlockTransfer> queue;

	s32 total_sending = 0;

	{
		ScopeProfiler sp(g_profiler, "Server: selecting blocks for sending");

		for (core::map<u16, RemoteClient*>::Iterator i = m_clients.getIterator(); i.atEnd() == false; i++) {
			RemoteClient *client = i.getNode()->getValue();
			assert(client->peer_id == i.getNode()->getKey());

			total_sending += client->SendingCount();

			if (client->serialization_version == SER_FMT_VER_INVALID)
				continue;

			client->GetNextBlocks(this, dtime, queue);
		}
	}

	// Sort.
	// Lowest priority number comes first.
	// Lowest is most important.
	queue.sort();

	for (u32 i=0; i<queue.size(); i++)
	{
		//TODO: Calculate limit dynamically
		if (total_sending >= max)
			break;

		PrioritySortedBlockTransfer q = queue[i];

		MapBlock* block = NULL;
		try
		{
			block = m_env.getMap().getBlockNoCreate(q.pos);
		}
		catch (InvalidPositionException &e)
		{
			continue;
		}

		RemoteClient* const client = getClient(q.peer_id);

		SendBlockNoLock(q.peer_id, block, client->serialization_version);

		if(block)
		    block->ResetCurrent();

		client->SentBlock(q.pos);

		total_sending++;
	}
}

void Server::SendEnvEvent(u8 type, v3f pos, std::string &data, Player *except_player)
{
	// Create packet
	std::ostringstream os(std::ios_base::binary);

	/*
		u16 command
		u8 event type (sound,nodemod,particles,etc)
		v3f1000 event position
		u16 length of serialised event data
		string serialised event data
	*/

	writeU16(os, TOCLIENT_ENV_EVENT);
	writeU8(os,type);
	writeV3F1000(os, pos);
	os<<serializeString(data);

	// Make data buffer
	std::string s = os.str();
	SharedBuffer<u8> reply((u8*)s.c_str(), s.size());

	for (core::map<u16, RemoteClient*>::Iterator i = m_clients.getIterator(); i.atEnd() == false; i++) {
		// Get client and check that it is valid
		RemoteClient *client = i.getNode()->getValue();
		assert(client->peer_id == i.getNode()->getKey());
		if (client->serialization_version == SER_FMT_VER_INVALID)
			continue;

		// Don't send if it's except_player
		if (except_player != NULL && except_player->peer_id == client->peer_id)
			continue;

		if (type != ENV_EVENT_WAKE && type != ENV_EVENT_SLEEP) {
			Player *player = m_env.getPlayer(client->peer_id);
			if (player) {
				// don't send to far off players (2 mapblocks)
				v3f player_pos = player->getPosition();
				if (player_pos.getDistanceFrom(pos) > 320.0)
					continue;
			}
		}

		// Send as reliable
		m_con.Send(client->peer_id, 0, reply, true);
	}
}

/*
	Something random
*/

void Server::HandlePlayerHP(Player *player, s16 damage, s16 suffocate, s16 hunger)
{
	if (player->air > suffocate) {
		player->air -= suffocate;
		if (player->air > 100)
			player->air = 100;
	}else{
		damage += suffocate-player->air;
		player->air = 0;
	}
	if (player->hunger > hunger) {
		player->hunger -= hunger;
		if (player->hunger > 100)
			player->hunger = 100;
	}else{
		damage += hunger-player->hunger;
		player->hunger = 0;
	}
	player->addDamage(PLAYER_ALL,DAMAGE_UNKNOWN,damage);
	if (player->health > 0) {
		SendPlayerState(player);
	}else{
		infostream<<"Server::HandlePlayerHP(): Player "
				<<player->getName()<<" dies"<<std::endl;

		player->health = 0;
		player->dirt = 0;
		player->wet = 0;
		player->blood = 0;

		// Throw items around
		if (!config_get_bool("world.player.inventory.keep")) {
			v3s16 bottompos = floatToInt(player->getPosition() + v3f(0,-BS/4,0), BS);
			v3s16 p = bottompos + v3s16(0,1,0);
			{
				InventoryList *il = player->inventory.getList("main");
				if (il) {
					for (u32 i=0; i<32; i++) {
						InventoryItem *item = il->changeItem(i,NULL);
						if (item)
							m_env.dropToParcel(p,item);
					}
				}
			}

			player->resetInventory(false);
			UpdateCrafting(player->peer_id);
			SendInventory(player->peer_id);
		}

		// Handle players that are not connected
		if (player->peer_id == PEER_ID_INEXISTENT) {
			RespawnPlayer(player);
			return;
		}

		SendPlayerState(player);

		player->updateAnim(PLAYERANIM_DIE,CONTENT_AIR);
		SendDeathscreen(m_con, player->peer_id, false, v3f(0,0,0));
	}
}

void Server::RespawnPlayer(Player *player)
{
	v3f pos;
	if (!player->getHome(PLAYERFLAG_HOME,pos))
		pos = findSpawnPos(m_env.getServerMap());
	player->setPosition(pos);
	player->health = 100;
	player->air = 100;
	player->hunger = 100;
	player->dirt = 0;
	player->wet = 0;
	player->blood = 0;
	player->updateAnim(PLAYERANIM_STAND,CONTENT_AIR);
	SendMovePlayer(player);
	SendPlayerState(player);
}

void Server::UpdateCrafting(u16 peer_id)
{
	DSTACK(__FUNCTION_NAME);

	Player* player = m_env.getPlayer(peer_id);
	assert(player);

	/*
		Calculate crafting stuff
	*/
	if (!config_get_bool("world.player.inventory.creative")) {
		InventoryList *clist = player->inventory.getList("craft");
		InventoryList *rlist = player->inventory.getList("craftresult");

		if(rlist && rlist->getUsedSlots() == 0)
			player->craftresult_is_preview = true;

		if (rlist && player->craftresult_is_preview) {
			rlist->clearItems();
		}
		if (clist && rlist && player->craftresult_is_preview) {
			InventoryItem *items[9];
			for (u16 i=0; i<9; i++) {
				items[i] = clist->getItem(i);
			}

			// Get result of crafting grid
			InventoryItem *result = crafting::getResult(items,player,this);
			if (result)
				rlist->addItem(result);
		}

	}
}

RemoteClient* Server::getClient(u16 peer_id)
{
	DSTACK(__FUNCTION_NAME);
	//JMutexAutoLock lock(m_con_mutex);
	core::map<u16, RemoteClient*>::Node *n;
	n = m_clients.find(peer_id);
	// A client should exist for all peers
	assert(n != NULL);
	return n->getValue();
}

std::wstring Server::getStatusString()
{
	std::wostringstream os(std::ios_base::binary);
	os<<L"# Server: ";
	// Version
	os<<L"version="<<narrow_to_wide(VERSION_STRING);

	// Uptime - human readable format
	float timestamp = m_uptime.get(), seconds = 0.0;
	int days = 0, hours = 0, minutes = 0;

	days = (int)timestamp / SECONDS_PER_DAY;
	hours = ((int)timestamp - (days * SECONDS_PER_DAY)) / SECONDS_PER_HOUR;
	minutes = ((int)timestamp -
				 ((days * SECONDS_PER_DAY) + (hours * SECONDS_PER_HOUR))) / 60;
	seconds = timestamp -
				 ((days * SECONDS_PER_DAY) + (hours * SECONDS_PER_HOUR) +
				 (minutes * 60));

	os<<L", uptime: " << days << "DD, " << hours << ":" << minutes << ":"
	  << seconds;

	// Information about clients
	os<<L", clients={";
	for (core::map<u16, RemoteClient*>::Iterator i = m_clients.getIterator(); i.atEnd() == false; i++) {
		// Get client and check that it is valid
		RemoteClient *client = i.getNode()->getValue();
		assert(client->peer_id == i.getNode()->getKey());
		if(client->serialization_version == SER_FMT_VER_INVALID)
			continue;
		// Get player
		Player *player = m_env.getPlayer(client->peer_id);
		// Get name of player
		std::wstring name = L"unknown";
		if(player != NULL)
			name = narrow_to_wide(player->getName());
		// Add name to information string
		os<<name<<L",";
	}
	os<<L"}";
	const char* motd = config_get("world.game.motd");
	if (motd && motd[0])
		os<<std::endl<<L"# Server: "<<narrow_to_wide(motd);
	return os.str();
}

void Server::notifyPlayer(const char *name, const std::wstring msg)
{
	Player *player = m_env.getPlayer(name);
	if(!player)
		return;
	SendChatMessage(player->peer_id, std::wstring(L"Server: -!- ")+msg);
}

void Server::notifyPlayers(const std::wstring msg)
{
	BroadcastChatMessage(msg);
}

v3f findSpawnPos(ServerMap &map)
{
	v3_t sp;

	if (!config_get_v3t("world.spawn.static",&sp)) {
		v3f pos(sp.x,sp.y,sp.z);
		return pos*BS;
	}

	v2s16 nodepos;
	s16 groundheight = 0;
	// Try to find a good place a few times
	for (s32 i=0; i<1000; i++) {
		s32 range = 1 + i;
		// We're going to try to throw the player to this position
		nodepos = v2s16(-range + (myrand()%(range*2)),
				-range + (myrand()%(range*2)));
		// Get ground height at point (fallbacks to heightmap function)
		groundheight = map.findGroundLevel(nodepos);
		// Don't go underwater
		if (groundheight < WATER_LEVEL) {
			//infostream<<"-> Underwater"<<std::endl;
			continue;
		}
		// Don't go to high places
		if (groundheight > WATER_LEVEL + 4) {
			//infostream<<"-> Underwater"<<std::endl;
			continue;
		}

		// Found a good place
		//infostream<<"Searched through "<<i<<" places."<<std::endl;
		break;
	}

	// If no suitable place was not found, go above water at least.
	if (groundheight < WATER_LEVEL)
		groundheight = WATER_LEVEL;

	return intToFloat(v3s16(nodepos.X,groundheight + 3,nodepos.Y), BS);
}

Player *Server::emergePlayer(const char *name, const char *password, u16 peer_id)
{
	/*
		Try to get an existing player
	*/
	Player *player = m_env.getPlayer(name);
	if (player != NULL) {
		// If player is already connected, cancel
		if (player->peer_id != 0) {
			infostream<<"emergePlayer(): Player already connected"<<std::endl;
			return NULL;
		}

		// Got one.
		player->peer_id = peer_id;

		// Set creative inventory
		if (config_get_bool("world.player.inventory.creative"))
			crafting::giveCreative(player);

		return player;
	}

	/*
		If player with the wanted peer_id already exists, cancel.
	*/
	if (m_env.getPlayer(peer_id) != NULL) {
		infostream<<"emergePlayer(): Player with wrong name but same"
				" peer_id already exists"<<std::endl;
		return NULL;
	}

	/*
		Create a new player
	*/
	{
		uint64_t privs = 0;
		const char* priv = config_get("world.server.client.default.privs");

		if (priv && priv[0])
			privs = auth_str2privs(priv);

		player = new ServerRemotePlayer();
		player->peer_id = peer_id;
		player->updateName(name);

		auth_add((char*)name);
		auth_setpwd((char*)name,(char*)password);
		auth_setprivs((char*)name,privs);

		/*
			Set player position
		*/

		infostream<<"Server: Finding spawn place for player \""
				<<player->getName()<<"\""<<std::endl;

		v3f pos = findSpawnPos(m_env.getServerMap());

		player->setPosition(pos);

		/*
			Add player to environment
		*/

		m_env.addPlayer(player);

		/*
			Add stuff to inventory
		*/

		if (config_get_bool("world.player.inventory.creative")) {
			// Set creative inventory
			crafting::giveCreative(player);
		}else if (config_get_bool("world.player.inventory.starter")) {
			crafting::giveInitial(player);
		}

		return player;

	} // create new player
}

void Server::handlePeerChange(PeerChange &c)
{
	JMutexAutoLock envlock(m_env_mutex);
	JMutexAutoLock conlock(m_con_mutex);

	if(c.type == PEER_ADDED)
	{
		/*
			Add
		*/

		// Error check
		core::map<u16, RemoteClient*>::Node *n;
		n = m_clients.find(c.peer_id);
		// The client shouldn't already exist
		assert(n == NULL);

		// Create client
		RemoteClient *client = new RemoteClient();
		client->peer_id = c.peer_id;
		m_clients.insert(client->peer_id, client);

	} // PEER_ADDED
	else if(c.type == PEER_REMOVED)
	{
		/*
			Delete
		*/

		// Error check
		core::map<u16, RemoteClient*>::Node *n;
		n = m_clients.find(c.peer_id);
		// The client should exist
		assert(n != NULL);

		/*
			Mark objects to be not known by the client
		*/
		RemoteClient *client = n->getValue();
		// Handle objects
		for (std::map<u16, bool>::iterator i = client->m_known_objects.begin(); i != client->m_known_objects.end(); i++) {
			// Get object
			u16 id = i->first;
			ServerActiveObject* obj = m_env.getActiveObject(id);

			if (obj && obj->m_known_by_count > 0) {
				obj->m_known_by_count--;
				if (obj->m_known_by_count < 1 && obj->m_pending_deactivation == true)
					obj->m_pending_deactivation = false;
			}
		}

		// Collect information about leaving in chat
		std::wstring message;
		{
			Player *player = m_env.getPlayer(c.peer_id);
			if (player != NULL) {
				std::wstring name = narrow_to_wide(player->getName());
				message += L"*** ";
				message += name;
				message += L" left game";
				if (c.timeout)
					message += L" (timed out)";
			}
		}

		/*// Delete player
		{
			m_env.removePlayer(c.peer_id);
		}*/

		// Set player client disconnected
		{
			Player *player = m_env.getPlayer(c.peer_id);
			if (player != NULL)
				player->peer_id = 0;

			/*
				Print out action
			*/
			if (player != NULL) {
				std::ostringstream os(std::ios_base::binary);
				for (core::map<u16, RemoteClient*>::Iterator i = m_clients.getIterator(); i.atEnd() == false; i++) {
					RemoteClient *client = i.getNode()->getValue();
					assert(client->peer_id == i.getNode()->getKey());
					if (client->serialization_version == SER_FMT_VER_INVALID)
						continue;
					// Get player
					Player *player = m_env.getPlayer(client->peer_id);
					if (!player)
						continue;
					// Get name of player
					os<<player->getName()<<" ";
				}

				actionstream<<player->getName()<<" "
						<<(c.timeout?"times out.":"leaves game.")
						<<" List of players: "
						<<os.str()<<std::endl;
			}
		}

		// Delete client
		delete m_clients[c.peer_id];
		m_clients.remove(c.peer_id);

		// Send player info to all remaining clients
		SendPlayerData();

		// Send leave chat message to all remaining clients
		BroadcastChatMessage(message);

	} // PEER_REMOVED
	else
	{
		assert(0);
	}
}

void Server::handlePeerChanges()
{
	while(m_peer_change_queue.size() > 0)
	{
		PeerChange c = m_peer_change_queue.pop_front();

		infostream<<"Server: Handling peer change: "
				<<"id="<<c.peer_id<<", timeout="<<c.timeout
				<<std::endl;

		handlePeerChange(c);
	}
}

void Server::addUser(const char *name, const char *password)
{
	auth_add((char*)name);
	auth_setpwd((char*)name,(char*)password);
	auth_save();
}

uint64_t Server::getPlayerPrivs(Player *player)
{

	if (!player)
		return 0;

	const char* playername = player->getName();
	const char* admin_name = config_get("world.server.admin");

	if (admin_name && !strcmp(admin_name,playername))
		return PRIV_ALL;

	return getPlayerAuthPrivs(playername);
}

void dedicated_server_loop(Server &server, bool &kill)
{
	DSTACK(__FUNCTION_NAME);

	infostream<<DTIME<<std::endl;
	infostream<<"========================"<<std::endl;
	infostream<<"Running dedicated server"<<std::endl;
	infostream<<"========================"<<std::endl;
	infostream<<std::endl;

	for (;;) {
		// This is kind of a hack but can be done like this
		// because server.step() is very light
		{
			ScopeProfiler sp(g_profiler, "dedicated server sleep");
			sleep_ms(30);
		}
		server.step(0.030);

		if (server.getShutdownRequested() || kill) {
			infostream<<DTIME<<" dedicated_server_loop(): Quitting."<<std::endl;
			break;
		}

		/*
			Player info
		*/
		static int counter = 0;
		counter--;
		if (counter <= 0) {
			counter = 10;

			core::list<PlayerInfo> list = server.getPlayerInfo();
			core::list<PlayerInfo>::Iterator i;
			static u32 sum_old = 0;
			u32 sum = PIChecksum(list);
			if (sum != sum_old) {
				infostream<<DTIME<<"Player info:"<<std::endl;
				for (i=list.begin(); i!=list.end(); i++) {
					i->PrintLine(&infostream);
				}
			}
			sum_old = sum;
		}
	}
}


