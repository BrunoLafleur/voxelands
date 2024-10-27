/************************************************************************
* Minetest-c55
* Copyright (C) 2010-2011 celeron55, Perttu Ahola <celeron55@gmail.com>
*
* map.cpp
* voxelands - 3d voxel world sandbox game
* Copyright (C) Lisa 'darkrose' Milne 2014 <lisa@ltmnet.com>
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

#include "map.h"
#include "mapsector.h"
#include "mapblock.h"
#include "main.h"
#ifndef SERVER
#include "client.h"
#endif
#include "utility.h"
#include "voxel.h"
#include "porting.h"
#include "nodemetadata.h"
#include "content_mapnode.h"
#include "content_nodemeta.h"
#ifndef SERVER
#include <IMaterialRenderer.h>
#endif
#include "log.h"
#include "profiler.h"
#include "inventory.h"
#include "enchantment.h"
#include "path.h"

#define PP(x) "("<<(x).X<<","<<(x).Y<<","<<(x).Z<<")"

/*
	SQLite format specification:
	- Initially only replaces sectors/ and sectors2/

	If map.sqlite does not exist in the save dir
	or the block was not found in the database
	the map will try to load from sectors folder.
	In either case, map.sqlite will be created
	and all future saves will save there.

	Structure of map.sqlite:
	Tables:
		blocks
			(PK) INT pos
			BLOB data
*/

/*
	Map
*/

Map::Map(std::ostream &dout) : m_dout(dout),
			       m_event_receivers(),
			       m_sectors(),
			       m_sectors_mutex(),
			       m_sector_cache(NULL),
			       m_sector_cache_p(),
			       m_transforming_liquid()
{
	m_sectors_mutex.Init();
	assert(m_sectors_mutex.IsInitialized());
}

Map::~Map()
{
	/*
		Free all MapSectors
	*/
	JMutexAutoLock lock(m_sectors_mutex);
	core::map<v2s16, MapSector*>::Iterator i = m_sectors.getIterator();
	
	for(; i.atEnd() == false; i++)
	{
		MapSector* const sector = i.getNode()->getValue();
		delete sector;
	}
}

void Map::addEventReceiver(MapEventReceiver *event_receiver)
{
	m_event_receivers.insert(event_receiver, false);
}

void Map::removeEventReceiver(MapEventReceiver *event_receiver)
{
	if(m_event_receivers.find(event_receiver) == NULL)
		return;
	m_event_receivers.remove(event_receiver);
}

void Map::dispatchEvent(MapEditEvent *event)
{
	for(core::map<MapEventReceiver*, bool>::Iterator
			i = m_event_receivers.getIterator();
			i.atEnd()==false; i++)
	{
		MapEventReceiver* event_receiver = i.getNode()->getKey();
		event_receiver->onMapEditEvent(event);
	}
}

MapSector* Map::getSectorNoGenerateNoExNoLock(v2s16 p)
{
	if(m_sector_cache != NULL && p == m_sector_cache_p)
	{
		MapSector* const sector = m_sector_cache;
		return sector;
	}

	core::map<v2s16, MapSector*>::Node* const n = m_sectors.find(p);

	if(n == NULL)
		return NULL;

	MapSector* const sector = n->getValue();

	// Cache the last result
	m_sector_cache_p = p;
	m_sector_cache = sector;

	return sector;
}

MapSector* Map::getSectorNoGenerateNoEx(v2s16 p)
{
	JMutexAutoLock lock(m_sectors_mutex);
	return getSectorNoGenerateNoExNoLock(p);
}

MapSector* Map::getSectorNoGenerate(v2s16 p)
{
	MapSector* const sector = getSectorNoGenerateNoEx(p);
	if(sector == NULL)
		throw InvalidPositionException();

	return sector;
}

MapBlock* Map::getBlockNoCreateNoExNoLock(v3s16 p3d)
{
	const v2s16 p2d(p3d.X, p3d.Z);
	MapSector* const sector = getSectorNoGenerateNoExNoLock(p2d);
	if(sector == NULL)
		return NULL;

    // Set mesh_current.
	MapBlock* const block = sector->getBlockNoCreateNoEx(p3d.Y);
	return block;
}

MapBlock* Map::getBlockNoCreateNoEx(v3s16 p3d)
{
	JMutexAutoLock lock(m_sectors_mutex);
	MapBlock* const block = getBlockNoCreateNoExNoLock(p3d);

	return block;
}

MapBlock* Map::getBlockNoCreate(v3s16 p3d)
{
	MapBlock* const block = getBlockNoCreateNoEx(p3d);
	if(block == NULL)
		throw InvalidPositionException();
	return block;
}

bool Map::isNodeUnderground(v3s16 p)
{
	const v3s16 blockpos = getNodeBlockPos(p);
	try
	{
		MapBlock* const block = getBlockNoCreate(blockpos);
		const bool nug = block->getIsUnderground();

		block->ResetCurrent();
		return nug;
	}
	catch(InvalidPositionException &e)
	{
		return false;
	}
}

bool Map::isValidPosition(v3s16 p)
{
	const v3s16 blockpos = getNodeBlockPos(p);
	JMutexAutoLock lock(m_sectors_mutex);
	MapBlock* const block = getBlockNoCreateNoExNoLock(blockpos);
	if(!block)
		throw InvalidPositionException();
	else
	    block->ResetCurrent();
	return block != NULL;
}

// Returns a CONTENT_IGNORE node if not found
MapNode Map::getNodeNoEx(v3s16 p, bool* is_valid_position)
{
	const v3s16 blockpos = getNodeBlockPos(p);
	JMutexAutoLock lock(m_sectors_mutex);
	MapBlock* const block = getBlockNoCreateNoExNoLock(blockpos);
	if (!block)
	{
		if (is_valid_position)
			*is_valid_position = false;
		return MapNode(CONTENT_IGNORE);
	}
	
	const v3s16 relpos = p - blockpos*MAP_BLOCKSIZE;
	bool is_valid_p;
	MapNode node = block->getNodeNoCheck(relpos, &is_valid_p);

	block->ResetCurrent();
	if (is_valid_position != NULL)
		*is_valid_position = is_valid_p;

	return node;
}

// throws InvalidPositionException if not found
MapNode Map::getNode(v3s16 p)
{
	const v3s16 blockpos = getNodeBlockPos(p);
	JMutexAutoLock lock(m_sectors_mutex);
	MapBlock* const block = getBlockNoCreateNoExNoLock(blockpos);
	if (!block)
		throw InvalidPositionException();
	
	const v3s16 relpos = p - blockpos*MAP_BLOCKSIZE;
	bool is_valid_position;
	MapNode node = block->getNodeNoCheck(relpos, &is_valid_position);

	block->ResetCurrent();
	if (!is_valid_position)
		throw InvalidPositionException();

	return node;
}

// throws InvalidPositionException if not found
void Map::setNode(v3s16 p, MapNode & n)
{
	const v3s16 blockpos = getNodeBlockPos(p);
	JMutexAutoLock lock(m_sectors_mutex);
	MapBlock* const block = getBlockNoCreateNoExNoLock(blockpos);
	if (!block)
		throw InvalidPositionException();
	
	const v3s16 relpos = p - blockpos*MAP_BLOCKSIZE;
	block->setNodeNoCheck(relpos, n);
	block->ResetCurrent();
}


/*
	Goes recursively through the neighbours of the node.

	Alters only transparent nodes.

	If the lighting of the neighbour is lower than the lighting of
	the node was (before changing it to 0 at the step before), the
	lighting of the neighbour is set to 0 and then the same stuff
	repeats for the neighbour.

	The ending nodes of the routine are stored in light_sources.
	This is useful when a light is removed. In such case, this
	routine can be called for the light node and then again for
	light_sources to re-light the area without the removed light.

	values of from_nodes are lighting values.
*/
void Map::unspreadLight(enum LightBank bank,
		core::map<v3s16, u8> & from_nodes,
		core::map<v3s16, bool> & light_sources,
		core::map<v3s16, MapBlock*>  & modified_blocks)
{
	const v3s16 dirs[6] = {
		v3s16(0,0,1), // back
		v3s16(0,1,0), // top
		v3s16(1,0,0), // right
		v3s16(0,0,-1), // front
		v3s16(0,-1,0), // bottom
		v3s16(-1,0,0), // left
	};

	if(from_nodes.size() == 0)
		return;

	u32 blockchangecount = 0;
	core::map<v3s16, u8> unlighted_nodes;
	core::map<v3s16, u8>::Iterator j = from_nodes.getIterator();

	/*
		Initialize block cache
	*/
	v3s16 blockpos_last;
	MapBlock* block = NULL;
	// Cache this a bit, too
	bool block_checked_in_modified = false;

	for(; j.atEnd() == false; j++)
	{
		const v3s16 pos = j.getNode()->getKey();
		const v3s16 blockpos = getNodeBlockPos(pos);

		// Only fetch a new block if the block position has changed
		try
		{
			if(block == NULL || blockpos != blockpos_last)
			{
			    
			    if(block)
				block->ResetCurrent();
	
			    block = getBlockNoCreate(blockpos);
			    blockpos_last = blockpos;

			    block_checked_in_modified = false;
			    blockchangecount++;
			}
		}
		catch(InvalidPositionException &e)
		{
			continue;
		}

		if (block->isDummy())
			continue;

		const u8 oldlight = j.getNode()->getValue();

		// Loop through 6 neighbors
		for (u16 i=0; i<6; i++)
		{
			// Get the position of the neighbor node
			const v3s16 n2pos = pos + dirs[i];
			// Get the block where the node is located
			const v3s16 blockpos = getNodeBlockPos(n2pos);

			// Only fetch a new block if the block position has changed
			try
			{
				if (block == NULL || blockpos != blockpos_last)
				{
				    
				    if(block)
					block->ResetCurrent();
	
				    block = getBlockNoCreate(blockpos);
				    blockpos_last = blockpos;

				    block_checked_in_modified = false;
				    blockchangecount++;
				}
			}
			catch(InvalidPositionException &e)
			{
				continue;
			}

			// Calculate relative position in block
			const v3s16 relpos = n2pos - blockpos * MAP_BLOCKSIZE;
			// Get node straight from the block
			bool is_valid_position;
			MapNode n2 = block->getNode(relpos, &is_valid_position);
			if (!is_valid_position)
				continue;

			bool changed = false;

			//TODO: Optimize output by optimizing light_sources?

			/*
				If the neighbor is dimmer than what was specified
				as oldlight (the light of the previous node)
			*/
			if (n2.getLight(bank) < oldlight)
			{
				/*
				  And the neighbor is transparent and it has some light
				*/
				if (content_features(n2).light_propagates && n2.getLight(bank) != 0)
				{
					/*
					  Set light to 0 and add to queue
					*/

					const u8 current_light = n2.getLight(bank);
					n2.setLight(bank, 0);
					block->setNode(relpos, n2);

					unlighted_nodes.insert(n2pos, current_light);
					changed = true;

					/*
					  Remove from light_sources if it is there
					  NOTE: This doesn't happen nearly at all
					*/
					/*if(light_sources.find(n2pos))
					{
						infostream<<"Removed from light_sources"<<std::endl;
						light_sources.remove(n2pos);
					}*/
				}

				/*// DEBUG
				if(light_sources.find(n2pos) != NULL)
					light_sources.remove(n2pos);*/
			}else
				light_sources.insert(n2pos, true);

			// Add to modified_blocks
			if (changed == true && block_checked_in_modified == false)
			{
				// If the block is not found in modified_blocks, add.
				if (modified_blocks.find(blockpos) == NULL)
					modified_blocks.insert(blockpos, block);
				block_checked_in_modified = true;
			}
		}
	}

	if(block)
	    block->ResetCurrent();
	
	/*infostream<<"unspreadLight(): Changed block "
			<<blockchangecount<<" times"
			<<" for "<<from_nodes.size()<<" nodes"
			<<std::endl;*/

	if(unlighted_nodes.size() > 0)
		unspreadLight(bank, unlighted_nodes, light_sources, modified_blocks);
}

/*
	A single-node wrapper of the above
*/
void Map::unLightNeighbors(enum LightBank bank,
		v3s16 pos, u8 lightwas,
		core::map<v3s16, bool> & light_sources,
		core::map<v3s16, MapBlock*>  & modified_blocks)
{
	core::map<v3s16, u8> from_nodes;
	from_nodes.insert(pos, lightwas);

	unspreadLight(bank, from_nodes, light_sources, modified_blocks);
}

/*
	Lights neighbors of from_nodes, collects all them and then
	goes on recursively.
*/
void Map::spreadLight(enum LightBank bank,
		core::map<v3s16, bool> & from_nodes,
		core::map<v3s16, MapBlock*> & modified_blocks)
{
	const v3s16 dirs[6] = {
		v3s16(0,0,1), // back
		v3s16(0,1,0), // top
		v3s16(1,0,0), // right
		v3s16(0,0,-1), // front
		v3s16(0,-1,0), // bottom
		v3s16(-1,0,0), // left
	};

	if(from_nodes.size() == 0)
		return;

	u32 blockchangecount = 0;

	core::map<v3s16, bool> lighted_nodes;
	core::map<v3s16, bool>::Iterator j;
	j = from_nodes.getIterator();

	/*
		Initialize block cache
	*/
	v3s16 blockpos_last;
	MapBlock* block = NULL;
	// Cache this a bit, too
	bool block_checked_in_modified = false;

	for(; j.atEnd() == false; j++)
	//for(; j != from_nodes.end(); j++)
	{
		v3s16 pos = j.getNode()->getKey();
		//v3s16 pos = *j;
		//infostream<<"pos=("<<pos.X<<","<<pos.Y<<","<<pos.Z<<")"<<std::endl;
		v3s16 blockpos = getNodeBlockPos(pos);

		// Only fetch a new block if the block position has changed
		try{
			if(block == NULL || blockpos != blockpos_last)
			{

			    if(block)
				block->ResetCurrent();
	
			    block = getBlockNoCreate(blockpos);
			    blockpos_last = blockpos;

			    block_checked_in_modified = false;
			    blockchangecount++;
			}
		}
		catch(InvalidPositionException &e)
		{
			continue;
		}

		if(block->isDummy())
			continue;

		// Calculate relative position in block
		v3s16 relpos = pos - blockpos_last * MAP_BLOCKSIZE;

		// Get node straight from the block
		bool is_valid_position;
		MapNode n = block->getNode(relpos, &is_valid_position);

		u8 oldlight = n.getLight(bank);
		u8 newlight = diminish_light(oldlight);

		// Loop through 6 neighbors
		for (u16 i=0; i<6; i++)
		{
			// Get the position of the neighbor node
			const v3s16 n2pos = pos + dirs[i];
			// Get the block where the node is located
			const v3s16 blockpos = getNodeBlockPos(n2pos);

			// Only fetch a new block if the block position has changed
			try
			{
				if (block == NULL || blockpos != blockpos_last)
				{
				    
				    if(block)
					block->ResetCurrent();
	
				    block = getBlockNoCreate(blockpos);
				    blockpos_last = blockpos;

				    block_checked_in_modified = false;
				    blockchangecount++;
				}
			}
			catch(InvalidPositionException &e)
			{
				continue;
			}

			// Calculate relative position in block
			const v3s16 relpos = n2pos - blockpos * MAP_BLOCKSIZE;
			// Get node straight from the block
			MapNode n2 = block->getNode(relpos, &is_valid_position);
			if (!is_valid_position)
				continue;

			bool changed = false;
			/*
				If the neighbor is brighter than the current node,
				add to list (it will light up this node on its turn)
			*/
			if (n2.getLight(bank) > undiminish_light(oldlight))
			{
				lighted_nodes.insert(n2pos, true);
				//lighted_nodes.push_back(n2pos);
				changed = true;
			}
			/*
				If the neighbor is dimmer than how much light this node
				would spread on it, add to list
			*/
			if (n2.getLight(bank) < newlight)
				if (content_features(n2).light_propagates)
				{
					n2.setLight(bank, newlight);
					block->setNode(relpos, n2);
					lighted_nodes.insert(n2pos, true);
					//lighted_nodes.push_back(n2pos);
					changed = true;
				}

			// Add to modified_blocks
			if (changed == true && block_checked_in_modified == false)
			{
				// If the block is not found in modified_blocks, add.
				if (modified_blocks.find(blockpos) == NULL)
					modified_blocks.insert(blockpos, block);
				block_checked_in_modified = true;
			}
		}
	}

	if(block)
	    block->ResetCurrent();
	
	/*infostream<<"spreadLight(): Changed block "
			<<blockchangecount<<" times"
			<<" for "<<from_nodes.size()<<" nodes"
			<<std::endl;*/

	if (lighted_nodes.size() > 0)
		spreadLight(bank, lighted_nodes, modified_blocks);
}

/*
	A single-node source variation of the above.
*/
void Map::lightNeighbors(enum LightBank bank,
		v3s16 pos,
		core::map<v3s16, MapBlock*> & modified_blocks)
{
	core::map<v3s16, bool> from_nodes;
	from_nodes.insert(pos, true);
	spreadLight(bank, from_nodes, modified_blocks);
}

v3s16 Map::getBrightestNeighbour(enum LightBank bank, v3s16 p)
{
	v3s16 dirs[6] = {
		v3s16(0,0,1), // back
		v3s16(0,1,0), // top
		v3s16(1,0,0), // right
		v3s16(0,0,-1), // front
		v3s16(0,-1,0), // bottom
		v3s16(-1,0,0), // left
	};

	u8 brightest_light = 0;
	v3s16 brightest_pos(0,0,0);
	bool found_something = false;

	// Loop through 6 neighbors
	for(u16 i=0; i<6; i++)
	{
		// Get the position of the neighbor node
		v3s16 n2pos = p + dirs[i];
		bool pos_ok;
		MapNode n2 = getNodeNoEx(n2pos,&pos_ok);
		if (!pos_ok)
			continue;
		if (n2.getLight(bank) > brightest_light
				|| found_something == false)
		{
			brightest_light = n2.getLight(bank);
			brightest_pos = n2pos;
			found_something = true;
		}
	}

	if(found_something == false)
		throw InvalidPositionException();

	return brightest_pos;
}

/*
	Propagates sunlight down from a node.
	Starting point gets sunlight.

	Returns the lowest y value of where the sunlight went.

	Mud is turned into grass in where the sunlight stops.
*/
s16 Map::propagateSunlight(v3s16 start,
		core::map<v3s16, MapBlock*> & modified_blocks)
{
	s16 y = start.Y;
	for (; ; y--)
	{
		v3s16 pos(start.X, y, start.Z);

		v3s16 blockpos = getNodeBlockPos(pos);
		MapBlock* block = NULL;
		try
		{
			block = getBlockNoCreate(blockpos);
		}
		catch(InvalidPositionException &e)
		{
			break;
		}

		v3s16 relpos = pos - blockpos*MAP_BLOCKSIZE;
		bool is_valid_position;
		MapNode n = block->getNode(relpos, &is_valid_position);
		if (!is_valid_position)
			continue;

		// Sunlight goes no further
		if (!content_features(n).sunlight_propagates)
			break;

		n.setLight(LIGHTBANK_DAY, LIGHT_SUN);
		block->setNode(relpos, n);

		modified_blocks.insert(blockpos, block);
		
		if(block)
		    block->ResetCurrent();
	}
	return y + 1;
}

void Map::updateLighting(enum LightBank bank,
		core::map<v3s16, MapBlock*> & a_blocks,
		core::map<v3s16, MapBlock*> & modified_blocks)
{
	/*m_dout<<DTIME<<"Map::updateLighting(): "
			<<a_blocks.size()<<" blocks."<<std::endl;*/

	//TimeTaker timer("updateLighting");

	core::map<v3s16, MapBlock*> blocks_to_update;

	core::map<v3s16, bool> light_sources;

	core::map<v3s16, u8> unlight_from;

	core::map<v3s16, MapBlock*>::Iterator i = a_blocks.getIterator();
	
	for (; i.atEnd() == false; i++)
	{
		MapBlock* block = i.getNode()->getValue();

		for (;;)
		{
			// Don't bother with dummy blocks.
			if (block->isDummy())
				break;

			v3s16 pos = block->getPos();
			modified_blocks.insert(pos, block);

			blocks_to_update.insert(pos, block);

			/*
				Clear all light from block
			*/
			for (s16 z=0; z<MAP_BLOCKSIZE; z++)
			for (s16 x=0; x<MAP_BLOCKSIZE; x++)
			for (s16 y=0; y<MAP_BLOCKSIZE; y++)
			{
					v3s16 p(x,y,z);
					bool is_valid_position;
					MapNode n = block->getNode(v3s16(x,y,z),&is_valid_position);
					if (!is_valid_position)
						continue;
					u8 oldlight = n.getLight(bank);
					n.setLight(bank, 0);
					block->setNode(v3s16(x,y,z), n);

					// Collect borders for unlighting
					if (x==0 || x == MAP_BLOCKSIZE-1
							|| y==0 || y == MAP_BLOCKSIZE-1
							|| z==0 || z == MAP_BLOCKSIZE-1)
					{
						v3s16 p_map = p + v3s16(
								MAP_BLOCKSIZE*pos.X,
								MAP_BLOCKSIZE*pos.Y,
								MAP_BLOCKSIZE*pos.Z);
						unlight_from.insert(p_map, oldlight);
					}
			}

			if (bank == LIGHTBANK_DAY)
			{
				bool bottom_valid = block->propagateSunlight(light_sources);

				// If bottom is valid, we're done.
				if(bottom_valid)
					break;
			}
			else if (bank == LIGHTBANK_NIGHT)
				// For night lighting, sunlight is not propagated
				break;
			else
				// Invalid lighting bank
				assert(0);

			// Bottom sunlight is not valid; get the block and loop to it

			pos.Y--;
			try
			{				
			    if(block)
				block->ResetCurrent();
				
			    block = getBlockNoCreate(pos);
			}
			catch(InvalidPositionException &e)
			{
			    assert(0);
			}

		}
		
		if(block)
		    block->ResetCurrent();

	}

	{
		//MapVoxelManipulator vmanip(this);

		// Make a manual voxel manipulator and load all the blocks
		// that touch the requested blocks
		ManualMapVoxelManipulator vmanip(this);
		core::map<v3s16, MapBlock*>::Iterator i = blocks_to_update.getIterator();
		
		for (; i.atEnd() == false; i++)
		{
			MapBlock *block = i.getNode()->getValue();
			v3s16 p = block->getPos();

			// Add all surrounding blocks
			vmanip.initialEmerge(p - v3s16(1,1,1), p + v3s16(1,1,1));

			// Lighting of block will be updated completely
			block->setLightingExpired(false);
		}

		{
			//TimeTaker timer("unSpreadLight");
			vmanip.unspreadLight(bank, unlight_from, light_sources);
		}
		{
			//TimeTaker timer("spreadLight");
			vmanip.spreadLight(bank, light_sources);
		}
		{
			//TimeTaker timer("blitBack");
			vmanip.blitBack(modified_blocks);
		}
	}

	//m_dout<<"Done ("<<getTimestamp()<<")"<<std::endl;
}

void Map::updateLighting(core::map<v3s16, MapBlock*> & a_blocks,
		core::map<v3s16, MapBlock*> & modified_blocks)
{
	updateLighting(LIGHTBANK_DAY, a_blocks, modified_blocks);
	updateLighting(LIGHTBANK_NIGHT, a_blocks, modified_blocks);

	/*
		Update information about whether day and night light differ
	*/
	for(core::map<v3s16, MapBlock*>::Iterator
			i = modified_blocks.getIterator();
			i.atEnd() == false; i++)
	{
		MapBlock *block = i.getNode()->getValue();
		block->updateDayNightDiff();
	}
}

/*
*/
void Map::addNodeAndUpdate(v3s16 p, MapNode n,
		core::map<v3s16, MapBlock*> &modified_blocks,
		std::string &player_name)
{
	/*PrintInfo(m_dout);
	m_dout<<DTIME<<"Map::addNodeAndUpdate(): p=("
			<<p.X<<","<<p.Y<<","<<p.Z<<")"<<std::endl;*/

	/*
		From this node to nodes underneath:
		If lighting is sunlight (1.0), unlight neighbours and
		set lighting to 0.
		Else discontinue.
	*/

	const v3s16 toppos = p + v3s16(0,1,0);
	bool node_under_sunlight = true;
	core::map<v3s16, bool> light_sources;

	/*
		If there is a node at top and it doesn't have sunlight,
		there has not been any sunlight going down.

		Otherwise there probably is.
	*/
	{
		bool pos_ok;
		MapNode topnode = getNodeNoEx(toppos,&pos_ok);

		if (pos_ok && topnode.getLight(LIGHTBANK_DAY) != LIGHT_SUN)
			node_under_sunlight = false;
	}

	/*
		Remove all light that has come out of this node
	*/

	enum LightBank banks[] =
	{
		LIGHTBANK_DAY,
		LIGHTBANK_NIGHT
	};
	
	for(s32 i=0; i<2; i++)
	{
		enum LightBank bank = banks[i];

		u8 lightwas = getNode(p).getLight(bank);

		// Add the block of the added node to modified_blocks
		v3s16 blockpos = getNodeBlockPos(p);
		MapBlock* const block = getBlockNoCreate(blockpos);
		assert(block != NULL);
		modified_blocks.insert(blockpos, block);

		assert(isValidPosition(p));

		// Unlight neighbours of node.
		// This means setting light of all consequent dimmer nodes
		// to 0.
		// This also collects the nodes at the border which will spread
		// light again into this.
		unLightNeighbors(bank, p, lightwas, light_sources, modified_blocks);
		n.setLight(bank, 0);
		block->ResetCurrent();
	}

	/*
		If node lets sunlight through and is under sunlight, it has
		sunlight too.
	*/
	if (node_under_sunlight && content_features(n).sunlight_propagates)
		n.setLight(LIGHTBANK_DAY, LIGHT_SUN);

	/*
		Set the node on the map
	*/

	setNode(p, n);

	/*
		Add intial metadata
	*/

	NodeMetadata* const meta_proto = content_features(n).initial_metadata;
	if (meta_proto) {
		NodeMetadata *meta = meta_proto->clone();
		meta->setOwner(player_name);
		setNodeMetadata(p, meta);
	}

	/*
		If node is under sunlight and doesn't let sunlight through,
		take all sunlighted nodes under it and clear light from them
		and from where the light has been spread.
		TODO: This could be optimized by mass-unlighting instead
			  of looping
	*/
	if (node_under_sunlight && !content_features(n).sunlight_propagates)
	{
		s16 y = p.Y - 1;
		for (;; y--)
		{
			//m_dout<<DTIME<<"y="<<y<<std::endl;
			v3s16 n2pos(p.X, y, p.Z);
			bool pos_ok;

			MapNode n2 = getNodeNoEx(n2pos,&pos_ok);
			if (!pos_ok)
				break;

			if (n2.getLight(LIGHTBANK_DAY) == LIGHT_SUN)
			{
				unLightNeighbors(LIGHTBANK_DAY,
						n2pos, n2.getLight(LIGHTBANK_DAY),
						light_sources, modified_blocks);
				n2.setLight(LIGHTBANK_DAY, 0);
				setNode(n2pos, n2);
			}
			else
				break;
		}
	}

	for (s32 i=0; i<2; i++)
	{
		enum LightBank bank = banks[i];

		/*
			Spread light from all nodes that might be capable of doing so
		*/
		spreadLight(bank, light_sources, modified_blocks);
	}

	/*
		Update information about whether day and night light differ
	*/
	for(core::map<v3s16, MapBlock*>::Iterator
			i = modified_blocks.getIterator();
			i.atEnd() == false; i++)
	{
		MapBlock *block = i.getNode()->getValue();
		block->updateDayNightDiff();
	}

	/*
		Add neighboring liquid nodes and the node itself if it is
		liquid (=water node was added) to transform queue.
	*/
	const v3s16 dirs[7] = {
		v3s16(0,0,0), // self
		v3s16(0,0,1), // back
		v3s16(0,1,0), // top
		v3s16(1,0,0), // right
		v3s16(0,0,-1), // front
		v3s16(0,-1,0), // bottom
		v3s16(-1,0,0), // left
	};
	
	for (u16 i=0; i<7; i++)
	{
	    const v3s16 p2 = p + dirs[i];
	    bool pos_ok;
	    const MapNode n2 = getNodeNoEx(p2,&pos_ok);
		
	    if (pos_ok && (content_features(n2).liquid_type != LIQUID_NONE
					    || n2.getContent() == CONTENT_AIR))
		m_transforming_liquid.push_back(p2);
	}
}

/*
*/
void Map::removeNodeAndUpdate(v3s16 p,
		core::map<v3s16, MapBlock*> &modified_blocks)
{
	/*PrintInfo(m_dout);
	m_dout<<DTIME<<"Map::removeNodeAndUpdate(): p=("
			<<p.X<<","<<p.Y<<","<<p.Z<<")"<<std::endl;*/

	bool node_under_sunlight = true;
	const v3s16 toppos = p + v3s16(0,1,0);

	// Node will be replaced with this
	content_t replace_material = CONTENT_AIR;

	/*
		If there is a node at top and it doesn't have sunlight,
		there will be no sunlight going down.
	*/
	{
		bool pos_ok;
		MapNode topnode = getNodeNoEx(toppos,&pos_ok);

		if (pos_ok && topnode.getLight(LIGHTBANK_DAY) != LIGHT_SUN)
			node_under_sunlight = false;
	}

	core::map<v3s16, bool> light_sources;

	enum LightBank banks[] = {
		LIGHTBANK_DAY,
		LIGHTBANK_NIGHT
	};
	
	for (s32 i=0; i<2; i++)
	{
		enum LightBank bank = banks[i];

		/*
			Unlight neighbors (in case the node is a light source)
		*/
		unLightNeighbors(bank, p,
				getNode(p).getLight(bank),
				light_sources, modified_blocks);
	}

	/*
		Remove node metadata
	*/

	removeNodeMetadata(p);

	/*
		Remove the node.
		This also clears the lighting.
	*/

	MapNode n;
	n.setContent(replace_material);
	setNode(p, n);

	for (s32 i=0; i<2; i++)
	{
		enum LightBank bank = banks[i];

		/*
			Recalculate lighting
		*/
		spreadLight(bank, light_sources, modified_blocks);
	}

	// Add the block of the removed node to modified_blocks
	v3s16 blockpos = getNodeBlockPos(p);
	MapBlock* const block = getBlockNoCreate(blockpos);
	assert(block != NULL);
	modified_blocks.insert(blockpos, block);

	/*
		If the removed node was under sunlight, propagate the
		sunlight down from it and then light all neighbors
		of the propagated blocks.
	*/
	if (node_under_sunlight)
	{
		s16 ybottom = propagateSunlight(p, modified_blocks);
		/*m_dout<<DTIME<<"Node was under sunlight. "
				"Propagating sunlight";
		m_dout<<DTIME<<" -> ybottom="<<ybottom<<std::endl;*/
		s16 y = p.Y;
		for (; y >= ybottom; y--)
		{
			v3s16 p2(p.X, y, p.Z);
			/*m_dout<<DTIME<<"lighting neighbors of node ("
					<<p2.X<<","<<p2.Y<<","<<p2.Z<<")"
					<<std::endl;*/
			lightNeighbors(LIGHTBANK_DAY, p2, modified_blocks);
		}
	}
	else
	{
		// Set the lighting of this node to 0
		// TODO: Is this needed? Lighting is cleared up there already.
		bool pos_ok;
		MapNode n = getNodeNoEx(p,&pos_ok);
		if (pos_ok)
		{
			n.setLight(LIGHTBANK_DAY, 0);
			setNode(p, n);
		}
	}

	for (s32 i=0; i<2; i++)
	{
		enum LightBank bank = banks[i];

		// Get the brightest neighbour node and propagate light from it
		v3s16 n2p = getBrightestNeighbour(bank, p);
		bool pos_ok = isValidPosition(n2p);
		if (pos_ok)
			lightNeighbors(bank, n2p, modified_blocks);
	}

	/*
		Update information about whether day and night light differ
	*/
	for(core::map<v3s16, MapBlock*>::Iterator
			i = modified_blocks.getIterator();
			i.atEnd() == false; i++)
	{
		MapBlock *block = i.getNode()->getValue();
		block->updateDayNightDiff();
	}

	/*
		Add neighboring liquid nodes and this node to transform queue.
		(it's vital for the node itself to get updated last.)
	*/
	const v3s16 dirs[7] = {
		v3s16(0,0,1), // back
		v3s16(0,1,0), // top
		v3s16(1,0,0), // right
		v3s16(0,0,-1), // front
		v3s16(0,-1,0), // bottom
		v3s16(-1,0,0), // left
		v3s16(0,0,0), // self
	};
	
	for (u16 i=0; i<7; i++)
	{
	    bool pos_ok;
	    const v3s16 p2 = p + dirs[i];
	    const MapNode n2 = getNodeNoEx(p2,&pos_ok);
		
	    if (pos_ok && (content_features(n2).liquid_type != LIQUID_NONE
					    || n2.getContent() == CONTENT_AIR))
		m_transforming_liquid.push_back(p2);
	}

	if(block)
	    block->ResetCurrent();
}

bool Map::addNodeWithEvent(v3s16 p, MapNode n)
{
	MapEditEvent event;
	event.type = MEET_ADDNODE;
	event.p = p;
	event.n = n;

	bool succeeded = true;
	try
	{
		core::map<v3s16, MapBlock*> modified_blocks;
		std::string st = std::string("");
		addNodeAndUpdate(p, n, modified_blocks, st);

		// Copy modified_blocks to event
		for(core::map<v3s16, MapBlock*>::Iterator
				i = modified_blocks.getIterator();
				i.atEnd()==false; i++)
		{
			event.modified_blocks.insert(i.getNode()->getKey(), false);
		}
	}
	catch(InvalidPositionException &e)
	{
		succeeded = false;
	}

	dispatchEvent(&event);

	return succeeded;
}

bool Map::removeNodeWithEvent(v3s16 p)
{
	MapEditEvent event;
	event.type = MEET_REMOVENODE;
	event.p = p;

	bool succeeded = true;
	try
	{
		core::map<v3s16, MapBlock*> modified_blocks;
		removeNodeAndUpdate(p, modified_blocks);

		// Copy modified_blocks to event
		for(core::map<v3s16, MapBlock*>::Iterator
				i = modified_blocks.getIterator();
				i.atEnd()==false; i++)
		{
			event.modified_blocks.insert(i.getNode()->getKey(), false);
		}
	}
	catch(InvalidPositionException &e)
	{
		succeeded = false;
	}

	dispatchEvent(&event);

	return succeeded;
}

bool Map::updateNodeWithEvent(v3s16 p, MapNode n)
{
	MapEditEvent event;
	event.type = MEET_ADDNODE;
	event.p = p;
	event.n = n;

	const MapNode nn = getNodeNoEx(p);
	if (nn.getContent() != n.getContent())
		return false;

	setNode(p, n);
	dispatchEvent(&event);

	return true;
}

bool Map::dayNightDiffed(v3s16 blockpos)
{
	try
	{
		v3s16 p = blockpos + v3s16(0,0,0);
		MapBlock* const b = getBlockNoCreate(p);
		const bool dnd = b->dayNightDiffed();

		b->ResetCurrent();
		
		if(dnd)
			return true;
	}
	catch(InvalidPositionException &e){}
	
	// Leading edges
	try
	{
		v3s16 p = blockpos + v3s16(-1,0,0);
		MapBlock* const b = getBlockNoCreate(p);
		const bool dnd = b->dayNightDiffed();

		b->ResetCurrent();

		if(dnd)
			return true;
	}
	catch(InvalidPositionException &e){}
	
	try
	{
		v3s16 p = blockpos + v3s16(0,-1,0);
		MapBlock* const b = getBlockNoCreate(p);
		const bool dnd = b->dayNightDiffed();
		
		b->ResetCurrent();

		if(dnd)
			return true;
	}
	catch(InvalidPositionException &e){}
	
	try
	{
		v3s16 p = blockpos + v3s16(0,0,-1);
		MapBlock* const b = getBlockNoCreate(p);
		const bool dnd = b->dayNightDiffed();
		
		b->ResetCurrent();

		if(dnd)
			return true;
	}
	catch(InvalidPositionException &e){}
	
	// Trailing edges
	try
	{
		v3s16 p = blockpos + v3s16(1,0,0);
		MapBlock* const b = getBlockNoCreate(p);
		const bool dnd = b->dayNightDiffed();
		
		b->ResetCurrent();

		if(dnd)
			return true;
	}
	catch(InvalidPositionException &e){}
	
	try
	{
		v3s16 p = blockpos + v3s16(0,1,0);
		MapBlock* const b = getBlockNoCreate(p);
		const bool dnd = b->dayNightDiffed();
		
		b->ResetCurrent();

		if(dnd)
			return true;
	}
	catch(InvalidPositionException &e){}
	
	try
	{
		v3s16 p = blockpos + v3s16(0,0,1);
		MapBlock* const b = getBlockNoCreate(p);
		const bool dnd = b->dayNightDiffed();
		
		b->ResetCurrent();

		if(dnd)
			return true;
	}
	catch(InvalidPositionException &e){}

	return false;
}

/*
	Updates usage timers
*/
void Map::timerUpdate(float dtime, float unload_timeout,
		core::list<v3s16>* unloaded_blocks)
{
	bool save_before_unloading = (mapType() == MAPTYPE_SERVER);
	core::list<v2s16> sector_deletion_queue;
	u32 deleted_blocks_count = 0;
	u32 saved_blocks_count = 0;

	beginSave();
	m_sectors_mutex.Lock();
	
	core::map<v2s16, MapSector*>::Iterator si = m_sectors.getIterator();
	
	for(; si.atEnd() == false; si++)
	{
	    MapSector* const sector = si.getNode()->getValue();
	    bool all_blocks_deleted = true;

	    core::list<MapBlock*> blocks;
	    sector->getBlocks(blocks);

	    for(core::list<MapBlock*>::Iterator i = blocks.begin();
		i != blocks.end(); i++)
	    {
		MapBlock* const block = (*i);

		block->incrementUsageTimer(dtime);

		if(block->getUsageTimer() > unload_timeout)
		{
		    const v3s16 p = block->getPos();

		// Save if modified
		    if(block->getModified() != MOD_STATE_CLEAN
				    && save_before_unloading)
		    {
			saveBlock(block);
			saved_blocks_count++;
		    }

		// If not in current use.
		    if(!block->GetCurrent())
		    {
		    // Delete from memory
			sector->deleteBlock(block);

			if(unloaded_blocks)
			    unloaded_blocks->push_back(p);

			deleted_blocks_count++;
		    }
		    else
			all_blocks_deleted = false;
		}
		else
		    all_blocks_deleted = false;
	    }

	    if(all_blocks_deleted)
		sector_deletion_queue.push_back(si.getNode()->getKey());
	}
	
	m_sectors_mutex.Unlock();
	endSave();

	// Finally delete the empty sectors
	deleteSectors(sector_deletion_queue);

	if(deleted_blocks_count != 0)
	{
	    PrintInfo(infostream); // ServerMap/ClientMap:
	    infostream<<"Unloaded "<<deleted_blocks_count <<" blocks from memory";
	    if(save_before_unloading)
		infostream<<", of which "<<saved_blocks_count<<" were written";
	    infostream<<"."<<std::endl;
	}
}

void Map::deleteSectors(core::list<v2s16> &list)
{
	core::list<v2s16>::Iterator j;
	JMutexAutoLock lock(m_sectors_mutex);

	for(j=list.begin(); j!=list.end(); j++)
	{
	    MapSector* const sector = m_sectors[*j];
	// If sector is in sector cache, remove it from there
	    if(m_sector_cache == sector)
		m_sector_cache = NULL;
	// Remove from map and delete
	    m_sectors.remove(*j);
	    delete sector;
	}
}

void Map::PrintInfo(std::ostream &out)
{
	out<<"Map: ";
}

#define WATER_DROP_BOOST 4

enum NeighborType {
	NEIGHBOR_UPPER,
	NEIGHBOR_SAME_LEVEL,
	NEIGHBOR_LOWER
};

struct NodeNeighbor {
	MapNode n;
	NeighborType t;
	v3s16 p;
};

void Map::transformLiquids(core::map<v3s16, MapBlock*> & modified_blocks)
{
	DSTACK(__FUNCTION_NAME);
	//TimeTaker timer("transformLiquids()");

	u32 loopcount = 0;
	u32 initial_size = m_transforming_liquid.size();

	/*if(initial_size != 0)
		infostream<<"transformLiquids(): initial_size="<<initial_size<<std::endl;*/

	// list of nodes that due to viscosity have not reached their max level height
	UniqueQueue<v3s16> must_reflow;

	// List of MapBlocks that will require a lighting update (due to lava)
	core::map<v3s16, MapBlock*> lighting_modified_blocks;

	while(m_transforming_liquid.size() != 0)
	{
		// This should be done here so that it is done when continue is used
		if(loopcount >= initial_size * 3)
			break;
		loopcount++;

		/*
			Get a queued transforming liquid node
		*/
		v3s16 p0 = m_transforming_liquid.pop_front();
		MapNode n0 = getNodeNoEx(p0);

		/*
			Collect information about current node
		 */
		s8 liquid_level = -1;
		u8 liquid_kind = CONTENT_IGNORE;
		LiquidType liquid_type = content_features(n0.getContent()).liquid_type;
		
		switch (liquid_type)
		{
		  case LIQUID_SOURCE:
		    liquid_level = LIQUID_LEVEL_SOURCE;
		    liquid_kind = content_features(n0.getContent()).liquid_alternative_flowing;
		    break;
		  case LIQUID_FLOWING:
		    liquid_level = (n0.param2 & LIQUID_LEVEL_MASK);
		    liquid_kind = n0.getContent();
		    break;
		  case LIQUID_NONE:
		// if this is an air node, it *could* be transformed into a liquid. otherwise,
		// continue with the next node.
		    if (n0.getContent() != CONTENT_AIR)
			continue;
		    liquid_kind = CONTENT_AIR;
		    break;
		}

		/*
			Collect information about the environment
		 */
		const v3s16 *dirs = g_6dirs;
		NodeNeighbor sources[6]; // surrounding sources
		int num_sources = 0;
		NodeNeighbor flows[6]; // surrounding flowing liquid nodes
		int num_flows = 0;
		NodeNeighbor airs[6]; // surrounding air
		int num_airs = 0;
		NodeNeighbor neutrals[6]; // nodes that are solid or another kind of liquid
		int num_neutrals = 0;
		bool flowing_down = false;
		
		for (u16 i = 0; i < 6; i++)
		{
		    NeighborType nt = NEIGHBOR_SAME_LEVEL;
		    switch (i)
		    {
		      case 1:
			nt = NEIGHBOR_UPPER;
			break;
		      case 4:
			nt = NEIGHBOR_LOWER;
			break;
		    }
			
		    const v3s16 npos = p0 + dirs[i];
		    const NodeNeighbor nb = {getNodeNoEx(npos), nt, npos};
			
		    switch (content_features(nb.n.getContent()).liquid_type)
		    {
		      case LIQUID_NONE:
			if (nb.n.getContent() == CONTENT_AIR)
			{
			    airs[num_airs++] = nb;
			// if the current node is a water source the neighbor
			// should be enqueded for transformation regardless of whether the
			// current node changes or not.
			    if (nb.t != NEIGHBOR_UPPER && liquid_type != LIQUID_NONE)
				m_transforming_liquid.push_back(npos);
			// if the current node happens to be a flowing node, it will start to flow down here.
			    if (nb.t == NEIGHBOR_LOWER)
				flowing_down = true;
			}
			else if (nb.t == NEIGHBOR_LOWER && nb.n.getContent() == CONTENT_IGNORE)
			{
			    flowing_down = true;
			    neutrals[num_neutrals++] = nb;
			}
			else
			    neutrals[num_neutrals++] = nb;
			break;
		      case LIQUID_SOURCE:
		    // if this node is not (yet) of a liquid type, choose the first liquid type we encounter
			if (liquid_kind == CONTENT_AIR)
			    liquid_kind = content_features(nb.n.getContent()).liquid_alternative_flowing;
			if (content_features(nb.n.getContent()).liquid_alternative_flowing !=liquid_kind)
			    neutrals[num_neutrals++] = nb;
			else
			{
			// Do not count bottom source, it will screw things up
			    if (dirs[i].Y != -1)
				sources[num_sources++] = nb;
			}
			break;
		      case LIQUID_FLOWING:
		    // if this node is not (yet) of a liquid type, choose the first liquid type we encounter
			if (liquid_kind == CONTENT_AIR)
			    liquid_kind = content_features(nb.n.getContent()).liquid_alternative_flowing;
			if (content_features(nb.n.getContent()).liquid_alternative_flowing != liquid_kind)
			    neutrals[num_neutrals++] = nb;
			else
			{
			    flows[num_flows++] = nb;
			    if (nb.t == NEIGHBOR_LOWER)
				flowing_down = true;
			}
			break;
		    }
		}

		/*
		  decide on the type (and possibly level) of the current node
		 */
		content_t new_node_content;
		s8 new_node_level = -1;
		s8 max_node_level = -1;
		
		if (num_sources >= 2 || liquid_type == LIQUID_SOURCE)
		// liquid_kind will be set to either the flowing alternative of the node (if it's a liquid)
		// or the flowing alternative of the first of the surrounding sources (if it's air), so
		// it's perfectly safe to use liquid_kind here to determine the new node content.
		    new_node_content = content_features(liquid_kind).liquid_alternative_source;
		else if (num_sources == 1 && sources[0].t != NEIGHBOR_LOWER)
		{
		// liquid_kind is set properly, see above
		    new_node_content = liquid_kind;
		    max_node_level = new_node_level = LIQUID_LEVEL_MAX;
		}
		else
		{
		// no surrounding sources, so get the maximum level that can flow into this node
		    for (u16 i = 0; i < num_flows; i++)
		    {
			u8 nb_liquid_level = (flows[i].n.param2 & LIQUID_LEVEL_MASK);
			switch (flows[i].t)
			{
			  case NEIGHBOR_UPPER:
			    if (nb_liquid_level + WATER_DROP_BOOST > max_node_level)
			    {
				max_node_level = LIQUID_LEVEL_MAX;
				if (nb_liquid_level + WATER_DROP_BOOST < LIQUID_LEVEL_MAX)
				    max_node_level = nb_liquid_level + WATER_DROP_BOOST;
			    } else if (nb_liquid_level > max_node_level)
				max_node_level = nb_liquid_level;
			    break;
			  case NEIGHBOR_LOWER:
			    break;
			  case NEIGHBOR_SAME_LEVEL:
			    if ((flows[i].n.param2 & LIQUID_FLOW_DOWN_MASK) != LIQUID_FLOW_DOWN_MASK &&
					    nb_liquid_level > 0 && nb_liquid_level - 1 > max_node_level)
				max_node_level = nb_liquid_level - 1;
			    break;
			}
		    }

		    u8 viscosity = content_features(liquid_kind).liquid_viscosity;
		    if (viscosity > 1 && max_node_level != liquid_level)
		    {
		    // amount to gain, limited by viscosity
		    // must be at least 1 in absolute value
			s8 level_inc = max_node_level - liquid_level;
			if (level_inc < -viscosity || level_inc > viscosity)
			    new_node_level = liquid_level + level_inc/viscosity;
			else if (level_inc < 0)
			    new_node_level = liquid_level - 1;
			else if (level_inc > 0)
			    new_node_level = liquid_level + 1;
			if (new_node_level != max_node_level)
			    must_reflow.push_back(p0);
		    }
		    else
			new_node_level = max_node_level;

		    if (new_node_level >= 0)
			new_node_content = liquid_kind;
		    else
			new_node_content = CONTENT_AIR;

		}

		/*
		  check if anything has changed. if not, just continue with the next node.
		*/
		if (new_node_content == n0.getContent() &&
				(content_features(n0.getContent()).liquid_type != LIQUID_FLOWING ||
						((n0.param2 & LIQUID_LEVEL_MASK) == (u8)new_node_level &&
								((n0.param2 & LIQUID_FLOW_DOWN_MASK) == LIQUID_FLOW_DOWN_MASK)
								== flowing_down)))
		    continue;


	    /*
	      update the current node
	    */
		if (content_features(new_node_content).liquid_type == LIQUID_FLOWING)
		// set level to last 3 bits, flowing down bit to 4th bit
		    n0.param2 = (flowing_down ? LIQUID_FLOW_DOWN_MASK : 0x00) | (new_node_level & LIQUID_LEVEL_MASK);
		else
		// set the liquid level and flow bit to 0
		    n0.param2 = ~(LIQUID_LEVEL_MASK | LIQUID_FLOW_DOWN_MASK);

		n0.setContent(new_node_content);
		setNode(p0, n0);

	    // PB a traiter.
		v3s16 blockpos = getNodeBlockPos(p0);
		MapBlock* const block = getBlockNoCreateNoEx(blockpos);
		
		if(block)
		{
		    modified_blocks.insert(blockpos, block);
		// If node emits light, MapBlock requires lighting update
		    if(content_features(n0).light_source != 0)
			lighting_modified_blocks[block->getPos()] = block;
		    
		    block->ResetCurrent();
		}

	    /*
	      enqueue neighbors for update if neccessary
	    */
		switch (content_features(n0.getContent()).liquid_type)
		{
		  case LIQUID_SOURCE:
		  case LIQUID_FLOWING:
		// make sure source flows into all neighboring nodes
		    for (u16 i = 0; i < num_flows; i++)
			if (flows[i].t != NEIGHBOR_UPPER)
			    m_transforming_liquid.push_back(flows[i].p);
		    for (u16 i = 0; i < num_airs; i++)
			if (airs[i].t != NEIGHBOR_UPPER)
			    m_transforming_liquid.push_back(airs[i].p);
		    break;
		  case LIQUID_NONE:
		// this flow has turned to air; neighboring flows might need to do the same
		    for (u16 i = 0; i < num_flows; i++)
			m_transforming_liquid.push_back(flows[i].p);
		    break;
		}
	}
	//infostream<<"Map::transformLiquids(): loopcount="<<loopcount<<std::endl;
	while (must_reflow.size() > 0)
	    m_transforming_liquid.push_back(must_reflow.pop_front());
	updateLighting(lighting_modified_blocks, modified_blocks);
}

NodeMetadata* Map::getNodeMetadata(v3s16 p)
{
	v3s16 blockpos = getNodeBlockPos(p);
	v3s16 p_rel = p - blockpos*MAP_BLOCKSIZE;
	MapBlock* block = getBlockNoCreateNoEx(blockpos);
	if (!block)
	{
	    infostream<<"Map::getNodeMetadata(): Need to emerge "
		      <<PP(blockpos)<<std::endl;
	    block = emergeBlock(blockpos, false);
	}
	
	if (!block)
	{
	    infostream<<"WARNING: Map::getNodeMetadata(): Block not found"
		      <<std::endl;
	    return NULL;
	}
	NodeMetadata* const meta = block->m_node_metadata.get(p_rel);
	
	block->ResetCurrent();

	return meta;
}

NodeMetadata* Map::getNodeMetadataClone(v3s16 p)
{
	v3s16 blockpos = getNodeBlockPos(p);
	v3s16 p_rel = p - blockpos*MAP_BLOCKSIZE;
	MapBlock* block = getBlockNoCreateNoEx(blockpos);
	if (!block)
	{
	    infostream<<"Map::getNodeMetadata(): Need to emerge "
		      <<PP(blockpos)<<std::endl;
	    block = emergeBlock(blockpos, false);
	}
	
	if (!block)
	{
	    infostream<<"WARNING: Map::getNodeMetadata(): Block not found"
		      <<std::endl;
	    return NULL;
	}
	NodeMetadata* const meta = block->m_node_metadata.getClone(p_rel);

	block->ResetCurrent();
	return meta;
}

void Map::setNodeMetadata(v3s16 p, NodeMetadata *meta)
{
	v3s16 blockpos = getNodeBlockPos(p);
	v3s16 p_rel = p - blockpos*MAP_BLOCKSIZE;
	MapBlock* block = getBlockNoCreateNoEx(blockpos);
	if(!block)
	{
	    infostream<<"Map::setNodeMetadata(): Need to emerge "
		      <<PP(blockpos)<<std::endl;
	    block = emergeBlock(blockpos, false);
	}
	
	if(!block)
	{
	    infostream<<"WARNING: Map::setNodeMetadata(): Block not found"
		      <<std::endl;
	    return;
	}
	
	block->m_node_metadata.set(p_rel, meta);
	block->ResetCurrent();
}

void Map::removeNodeMetadata(v3s16 p)
{
	v3s16 blockpos = getNodeBlockPos(p);
	v3s16 p_rel = p - blockpos*MAP_BLOCKSIZE;
	MapBlock* const block = getBlockNoCreateNoEx(blockpos);
	if(!block)
	{
	    infostream<<"WARNING: Map::removeNodeMetadata(): Block not found"
		      <<std::endl;
	    return;
	}
	
	block->m_node_metadata.remove(p_rel);
	block->ResetCurrent();
}

void Map::nodeMetadataStep(float dtime,core::map<v3s16,MapBlock*> &changed_blocks,
		ServerEnvironment *env)
{
	/*
		NOTE:
		Currently there is no way to ensure that all the necessary
		blocks are loaded when this is run. (They might get unloaded)
		NOTE: ^- Actually, that might not be so. In a quick test it
		reloaded a block with a furnace when I walked back to it from
		a distance.
	*/
	JMutexAutoLock lock(m_sectors_mutex);
	core::map<v2s16, MapSector*>::Iterator si = m_sectors.getIterator();
	
	for(; si.atEnd() == false; si++)
	{
	    MapSector* const sector = si.getNode()->getValue();
	    core::list<MapBlock*> sectorblocks;
		
	    sector->getBlocks(sectorblocks);
		
	    core::list<MapBlock*>::Iterator i;
	    for(i=sectorblocks.begin(); i!=sectorblocks.end(); i++)
	    {
		MapBlock* const block = *i;
		const bool changed = block->m_node_metadata.step(dtime,
				block->getPosRelative(),env);
		if(changed)
		    changed_blocks[block->getPos()] = block;
	    }
	}
}

/*
	ServerMap
*/

ServerMap::ServerMap() : Map(dout_server),
			 m_seed(0),
			 m_database(NULL),
			 m_database_read(NULL),
			 m_database_write(NULL)
{
	char b[1024];
	infostream<<__FUNCTION_NAME<<std::endl;

	config_load("world","world.cfg");

	loadMapMeta();

	/*
		Try to load map; if not found, create a new one.
	*/

	if (path_get("world","map.sqlite",1,b,1024))
		return;

	vlprintf(CN_ACTION,"Initializing new map");

	// Create zero sector
	emergeSector(v2s16(0,0));

	// Initially write whole map
	save(false);
}

ServerMap::~ServerMap()
{
	infostream<<__FUNCTION_NAME<<std::endl;

	try{
	    save(true);
	    infostream<<"Server: saved map"<<std::endl;
	}
	catch(std::exception &e)
	{
	    infostream<<"Server: Failed to save map, exception: "<<e.what()<<std::endl;
	}

	/*
		Close database if it was opened
	*/
	if(m_database_read)
	    sqlite3_finalize(m_database_read);
	if(m_database_write)
	    sqlite3_finalize(m_database_write);
	if(m_database_list)
	    sqlite3_finalize(m_database_list);
	if(m_database)
	    sqlite3_close(m_database);
}

void ServerMap::initBlockMake(mapgen::BlockMakeData *data, v3s16 blockpos)
{
	// Do nothing if not inside limits (+-1 because of neighbors)
	if (blockpos_over_limit(blockpos - v3s16(1,1,1))
			|| blockpos_over_limit(blockpos + v3s16(1,1,1)))
	{
	    data->no_op = true;
	    return;
	}

	data->no_op = false;
	data->seed = m_seed;
	data->type = m_type;
	data->blockpos = blockpos;

    /*
      Create the whole area of this and the neighboring blocks
    */
	{
	//TimeTaker timer("initBlockMake() create area");

	    for (s16 x=-1; x<=1; x++)
		for (s16 z=-1; z<=1; z++) {
		    v2s16 sectorpos(blockpos.X+x, blockpos.Z+z);
		// Sector metadata is loaded from disk if not already loaded.
		    ServerMapSector* const sector = createSector(sectorpos);
		    assert(sector);

		    for (s16 y=-1; y<=1; y++)
		    {
			v3s16 p(blockpos.X+x, blockpos.Y+y, blockpos.Z+z);
		    //MapBlock *block = createBlock(p);
		    // 1) get from memory, 2) load from disk
			MapBlock* block = emergeBlock(p, false);
		    // 3) create a blank one
			if (!block)
			{
			    block = createBlock(p);

			/*
			  Block gets sunlight if this is true.

			  Refer to the map generator heuristics.
			*/
			    bool ug = mapgen::block_is_underground(data, p);
			    block->setIsUnderground(ug);
			}

		    // Lighting will not be valid after make_chunk is called
			block->setLightingExpired(true);
			block->ResetCurrent();
		    }
		}
	}

    /*
      Now we have a big empty area.

      Make a ManualMapVoxelManipulator that contains this and the
      neighboring blocks
    */

    // The area that contains this block and it's neighbors
	v3s16 bigarea_blocks_min = blockpos - v3s16(1,1,1);
	v3s16 bigarea_blocks_max = blockpos + v3s16(1,1,1);

	data->vmanip = new ManualMapVoxelManipulator(this);

    // Add the area
	{
	//TimeTaker timer("initBlockMake() initialEmerge");
	    data->vmanip->initialEmerge(bigarea_blocks_min, bigarea_blocks_max);
	}

    // Data is ready now.
}

MapBlock* ServerMap::finishBlockMake(mapgen::BlockMakeData *data,
		core::map<v3s16, MapBlock*> &changed_blocks)
{
	if (data->no_op)
		return NULL;

	/*
		Blit generated stuff to map
		NOTE: blitBackAll adds nearly everything to changed_blocks
	*/
	{
		// 70ms @cs=8
		//TimeTaker timer("finishBlockMake() blitBackAll");
		data->vmanip->blitBackAllWithMeta(&changed_blocks);
	}

	/*
		Copy transforming liquid information
	*/
	while (data->transforming_liquid.size() > 0)
	{
		v3s16 p = data->transforming_liquid.pop_front();
		m_transforming_liquid.push_back(p);
	}

	/*
		Get central block
	*/
	MapBlock* const block = getBlockNoCreateNoEx(data->blockpos);
	assert(block);

	block->setBiome(data->biome);

	{
	    v3s16 p0;
	    for (p0.X=0; p0.X<MAP_BLOCKSIZE; p0.X++) {
	    for (p0.Y=0; p0.Y<MAP_BLOCKSIZE; p0.Y++) {
		for (p0.Z=0; p0.Z<MAP_BLOCKSIZE; p0.Z++)
		{
		    const MapNode n = block->getNodeNoEx(p0);
		    if (n.getContent() == CONTENT_CHEST)
		    {
		    // chest? give it metadata and put shit in it
			NodeMetadata* const f = block->m_node_metadata.get(p0);
			Inventory* const inv = f->getInventory();
			if (inv)
			{
			    InventoryList* const ilist = inv->getList("main");
			    if (ilist)
			    {
				if (myrand_range(0,2) == 0)
				    ilist->addItem(new CraftItem(CONTENT_CRAFTITEM_GRAPE,10,0));
				if (myrand_range(0,2) == 0)
				    ilist->addItem(new CraftItem(CONTENT_CRAFTITEM_OERKKI_DUST,6,0));
				if (myrand_range(0,3) == 0)
				    ilist->addItem(new ClothesItem(CONTENT_CLOTHESITEM_QUARTZ_MEDALLION,0,0));
				if (myrand_range(0,3) == 0)
				    ilist->addItem(new ClothesItem(CONTENT_CLOTHESITEM_SPACESUIT_PANTS,0,0));
				if (myrand_range(0,4) == 0)
				{
				    content_t c = CONTENT_CRAFTITEM_IRON_INGOT;
				    switch (myrand_range(0,4)) {
				      case 0:
					c = CONTENT_CRAFTITEM_MITHRIL_UNBOUND;
					break;
				      case 1:
					c = CONTENT_CRAFTITEM_FLINT;
					break;
				      case 2:
					c = CONTENT_CRAFTITEM_TIN_INGOT;
					break;
				      case 3:
					c = CONTENT_CRAFTITEM_QUARTZ;
					break;
				      default:;
				    }
				    ilist->addItem(new CraftItem(c,50,0));
				}
				if (myrand_range(0,4) == 0)
				{
				    content_t c = CONTENT_CRAFTITEM_RUBY;
				    switch (myrand_range(0,4)) {
				      case 0:
					c = CONTENT_CRAFTITEM_TURQUOISE;
					break;
				      case 1:
					c = CONTENT_CRAFTITEM_AMETHYST;
					break;
				      case 2:
					c = CONTENT_CRAFTITEM_SAPPHIRE;
					break;
				      case 3:
					c = CONTENT_CRAFTITEM_SUNSTONE;
					break;
				      default:;
				    }
				    ilist->addItem(new CraftItem(c,20,0));
				}
				if (myrand_range(0,5) == 0)
				{
				    uint16_t en = enchantment_create(ENCHANTMENT_LONGLASTING,3);
				    ilist->addItem(new ToolItem(CONTENT_TOOLITEM_MITHRIL_PICK,0,en));
				}
				if (ilist->getUsedSlots() == 0)
				    ilist->addItem(new CraftItem(CONTENT_CRAFTITEM_COAL,10,0));
			    }
			}
			continue;
		    }
		}
	    }
	    }
	}

	/*
		Update lighting
		NOTE: This takes ~60ms, TODO: Investigate why
	*/
	{
	    core::map<v3s16, MapBlock*> lighting_update_blocks;
		// Center block
	    lighting_update_blocks.insert(block->getPos(), block);
	    updateLighting(lighting_update_blocks, changed_blocks);

	/*
	  Set lighting to non-expired state in all of them.
	  This is cheating, but it is not fast enough if all of them
	  would actually be updated.
	*/
	    for(s16 x=-1; x<=1; x++)
	    for(s16 y=-1; y<=1; y++)
	    for(s16 z=-1; z<=1; z++)
	    {
		const v3s16 p = block->getPos()+v3s16(x,y,z);
		MapBlock* const b = getBlockNoCreateNoEx(p);

		if(b)
		{
		    b->setLightingExpired(false);
		    b->ResetCurrent();
		}
	    }
	}

	/*
		Go through changed blocks
	*/
	for(core::map<v3s16, MapBlock*>::Iterator i = changed_blocks.getIterator();
			i.atEnd() == false; i++)
	{
		MapBlock* const block = i.getNode()->getValue();
		assert(block);
		/*
			Update day/night difference cache of the MapBlocks
		*/
		block->updateDayNightDiff();
		/*
			Set block as modified
		*/
		block->raiseModified(MOD_STATE_WRITE_NEEDED);
	}

	/*
		Set central block as generated
	*/
	block->setGenerated(true);
	block->ResetCurrent();

	return block;
}

ServerMapSector* ServerMap::createSector(v2s16 p2d)
{
	DSTACKF("%s: p2d=(%d,%d)",
			__FUNCTION_NAME,
			p2d.X, p2d.Y);

	/*
		Check if it exists already in memory
	*/
	ServerMapSector* sector =
	    (ServerMapSector*) getSectorNoGenerateNoEx(p2d);
	if(sector != NULL)
		return sector;

	/*
		Do not create over-limit
	*/
	if(p2d.X < -MAP_GENERATION_LIMIT / MAP_BLOCKSIZE
			|| p2d.X > MAP_GENERATION_LIMIT / MAP_BLOCKSIZE
			|| p2d.Y < -MAP_GENERATION_LIMIT / MAP_BLOCKSIZE
			|| p2d.Y > MAP_GENERATION_LIMIT / MAP_BLOCKSIZE)
	        throw InvalidPositionException("createSector(): pos. over limit");

	/*
		Generate blank sector
	*/

	sector = new ServerMapSector(this, p2d);

	/*
		Insert to container
	*/
	m_sectors_mutex.Lock();
	m_sectors.insert(p2d, sector);
	m_sectors_mutex.Unlock();

	return sector;
}

/*
	This is a quick-hand function for calling makeBlock().
*/
MapBlock* ServerMap::generateBlock(v3s16 p,
		core::map<v3s16, MapBlock*> &modified_blocks)
{
	DSTACKF("%s: p=(%d,%d,%d)", __FUNCTION_NAME, p.X, p.Y, p.Z);

	const v2s16 p2d(p.X, p.Z);

	/*
		Do not generate over-limit
	*/
	if (blockpos_over_limit(p))
	{
	    infostream<<__FUNCTION_NAME<<": Block position over limit"<<std::endl;
	    throw InvalidPositionException("generateBlock(): pos. over limit");
	}

	/*
		Create block make data
	*/
	mapgen::BlockMakeData data;
	initBlockMake(&data, p);

	/*
		Generate block
	*/
	mapgen::make_block(&data);

	/*
		Blit data back on map, update lighting, add mobs and whatever this does
	*/
	finishBlockMake(&data, modified_blocks);

	/*
		Get central block
	*/
	MapBlock* const block = getBlockNoCreateNoEx(p);

	return block;
}

MapBlock* ServerMap::createBlock(v3s16 p)
{
	DSTACKF("%s: p=(%d,%d,%d)",
			__FUNCTION_NAME, p.X, p.Y, p.Z);

	/*
		Do not create over-limit
	*/
	if(p.X < -MAP_GENERATION_LIMIT / MAP_BLOCKSIZE
			|| p.X > MAP_GENERATION_LIMIT / MAP_BLOCKSIZE
			|| p.Y < -MAP_GENERATION_LIMIT / MAP_BLOCKSIZE
			|| p.Y > MAP_GENERATION_LIMIT / MAP_BLOCKSIZE
			|| p.Z < -MAP_GENERATION_LIMIT / MAP_BLOCKSIZE
			|| p.Z > MAP_GENERATION_LIMIT / MAP_BLOCKSIZE)
	    throw InvalidPositionException("createBlock(): pos. over limit");

	const v2s16 p2d(p.X, p.Z);
	const s16 block_y = p.Y;
    /*
      This will create or load a sector if not found in memory.
      If block exists on disk, it will be loaded.

      NOTE: On old save formats, this will be slow, as it generates
      lighting on blocks for them.
    */
	ServerMapSector* sector;
	try
	{
	    sector = (ServerMapSector*) createSector(p2d);
	    assert(sector->getId() == MAPSECTOR_SERVER);
	}
	catch(InvalidPositionException &e)
	{
	    infostream<<"createBlock: createSector() failed"<<std::endl;
	    throw e;
	}
	/*
		NOTE: This should not be done, or at least the exception
		should not be passed on as std::exception, because it
		won't be catched at all.
	*/
	/*catch(std::exception &e)
	{
		infostream<<"createBlock: createSector() failed: "
				<<e.what()<<std::endl;
		throw e;
	}*/

	/*
		Try to get a block from the sector
	*/

	MapBlock* const block = sector->getBlockNoCreateNoEx(block_y);
	if(block)
	{
	    if(block->isDummy())
		block->unDummify();
	    return block;
	}
	
	// Create blank
	// PB A voir allocation.
	MapBlock* const block2 = sector->createBlankBlock(block_y);
	return block2;
}

MapBlock* ServerMap::emergeBlock(v3s16 p, bool allow_generate,
		bool *was_generated)
{
	DSTACKF("%s: p=(%d,%d,%d), allow_generate=%d",
			__FUNCTION_NAME,
			p.X, p.Y, p.Z, allow_generate);
	if (was_generated)
	    *was_generated = false;

	{
	    MapBlock* const block = getBlockNoCreateNoEx(p);
	    if(block && block->isDummy() == false)
		return block;
	}

	{
	    MapBlock* const block = loadBlock(p);
	    if(block)
		return block;
	}

	if(allow_generate)
	{
	    core::map<v3s16, MapBlock*> modified_blocks;
	    MapBlock* const block = generateBlock(p, modified_blocks);
	    if(block)
	    {
		MapEditEvent event;
		event.type = MEET_OTHER;
		event.p = p;

	    // Copy modified_blocks to event
		for(core::map<v3s16, MapBlock*>::Iterator
			i = modified_blocks.getIterator();i.atEnd()==false; i++)
		{
		    event.modified_blocks.insert(i.getNode()->getKey(), false);
		}

	    // Queue event
		dispatchEvent(&event);

		if (was_generated)
		    *was_generated = true;
		return block;
	    }
	}

	return NULL;
}

s16 ServerMap::findGroundLevel(v2s16 p2d)
{
	/* determine from map generator noise functions */
	mapgen::BlockMakeData d;
	d.type = m_type;
	d.seed = m_seed;

	const s16 level = mapgen::find_ground_level_from_noise(&d, p2d, 1);
	return level;
}

void ServerMap::createDatabase()
{
	assert(m_database);
	const int e = sqlite3_exec(m_database,
			"CREATE TABLE IF NOT EXISTS `blocks` ("
			"`pos` INT NOT NULL PRIMARY KEY,"
			"`data` BLOB"
			");"
			, NULL, NULL, NULL);
	if(e == SQLITE_ABORT)
	    throw FileNotGoodException("Could not create database structure");
	else
	    infostream<<"Server: Database structure was created";
}

void ServerMap::verifyDatabase()
{
	if(m_database)
	    return;

	{
	    char buff[1024];
	    bool needs_create = false;
	    int d;

	/*
	  Open the database connection
	*/

	    if (!path_get((char*)"world",(char*)"map.sqlite",0,buff,1024))
		throw FileNotGoodException("map.sqlite: Cannot find database file path");

	    if (path_create((char*)"world",NULL))
		throw FileNotGoodException("map.sqlite: Cannot create database file path");

	    if (!path_exists(buff))
		needs_create = true;

	    d = sqlite3_open_v2(buff, &m_database, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
	    
	    if(d != SQLITE_OK)
	    {
		infostream<<"WARNING: Database failed to open: "<<sqlite3_errmsg(m_database)<<std::endl;
		throw FileNotGoodException("map.sqlite: Cannot open database file");
	    }

	    if(needs_create)
		createDatabase();

	    d = sqlite3_prepare(m_database, "SELECT `data` FROM `blocks` WHERE `pos`=? LIMIT 1", -1, &m_database_read, NULL);
	    
	    if(d != SQLITE_OK)
	    {
		infostream<<"WARNING: Database read statment failed to prepare: "<<sqlite3_errmsg(m_database)<<std::endl;
		throw FileNotGoodException("map.sqlite: Cannot prepare read statement");
	    }

	    d = sqlite3_prepare(m_database, "REPLACE INTO `blocks` VALUES(?, ?)", -1, &m_database_write, NULL);
	    if(d != SQLITE_OK)
	    {
		infostream<<"WARNING: Database write statment failed to prepare: "<<sqlite3_errmsg(m_database)<<std::endl;
		throw FileNotGoodException("map.sqlite: Cannot prepare write statement");
	    }

	    d = sqlite3_prepare(m_database, "SELECT `pos` FROM `blocks`", -1, &m_database_list, NULL);
	    if(d != SQLITE_OK)
	    {
		infostream<<"WARNING: Database list statment failed to prepare: "<<sqlite3_errmsg(m_database)<<std::endl;
		throw FileNotGoodException("map.sqlite: Cannot prepare read statement");
	    }

	    infostream<<"Server: Database opened"<<std::endl;
	}
}

sqlite3_int64 ServerMap::getBlockAsInteger(const v3s16 pos)
{
	return (sqlite3_int64)pos.Z*16777216 +
		(sqlite3_int64)pos.Y*4096 + (sqlite3_int64)pos.X;
}

void ServerMap::save(bool only_changed)
{
	DSTACK(__FUNCTION_NAME);

	if (only_changed == false)
		infostream<<"ServerMap: Saving whole map, this can take time."
				<<std::endl;

	u32 block_count = 0;
	u32 block_count_all = 0; // Number of blocks in memory

    // Don't do anything with sqlite unless something is really saved
	bool save_started = false;
	JMutexAutoLock lock(m_sectors_mutex);

	for (core::map<v2s16, MapSector*>::Iterator i = m_sectors.getIterator();
	     i.atEnd() == false; i++)
	{
	    ServerMapSector* const sector =
		(ServerMapSector*)i.getNode()->getValue();
	    assert(sector->getId() == MAPSECTOR_SERVER);
		
	    core::list<MapBlock*> blocks;
	    sector->getBlocks(blocks);
		
	    core::list<MapBlock*>::Iterator j;

	    for (j=blocks.begin(); j!=blocks.end(); j++)
	    {
		MapBlock* const block = *j;

		block_count_all++;

		if (block->getModified() >= MOD_STATE_WRITE_NEEDED
				|| only_changed == false)
		{
		// Lazy beginSave()
		    if (!save_started)
		    {
			beginSave();
			save_started = true;
		    }

		    saveBlock(block);
		    block_count++;
		}
	    }
	}
	if (save_started)
	    endSave();

	/*
		Only print if something happened or saved whole map
	*/
	if (only_changed == false || block_count != 0)
	{
		infostream<<"ServerMap: Written: "
				<<block_count<<" block files"
				<<", "<<block_count_all<<" blocks in memory."
				<<std::endl;
	}
}

static s32 unsignedToSigned(s32 i, s32 max_positive)
{
	if (i < max_positive)
		return i;
	return i - 2*max_positive;
}

// modulo of a negative number does not work consistently in C
static sqlite3_int64 pythonmodulo(sqlite3_int64 i, sqlite3_int64 mod)
{
	if (i >= 0)
		return i % mod;
	return mod - ((-i) % mod);
}

v3s16 ServerMap::getIntegerAsBlock(sqlite3_int64 i)
{
	const s32 x = unsignedToSigned(pythonmodulo(i, 4096), 2048);
	i = (i - x) / 4096;
	const s32 y = unsignedToSigned(pythonmodulo(i, 4096), 2048);
	i = (i - y) / 4096;
	const s32 z = unsignedToSigned(pythonmodulo(i, 4096), 2048);
	return v3s16(x,y,z);
}

void ServerMap::listAllLoadableBlocks(core::list<v3s16> &dst)
{
	verifyDatabase();

	while (sqlite3_step(m_database_list) == SQLITE_ROW)
	{
		sqlite3_int64 block_i = sqlite3_column_int64(m_database_list, 0);
		v3s16 p = getIntegerAsBlock(block_i);
		dst.push_back(p);
	}
}

void ServerMap::loadMapMeta()
{
	if (!config_get("world.seed"))
	{
	    m_seed = (((uint64_t)(myrand()%0xffff)<<0)
			    + ((uint64_t)(myrand()%0xffff)<<16)
			    + ((uint64_t)(myrand()%0xffff)<<32)
			    + ((uint64_t)(myrand()%0xffff)<<48));
	    config_set_int64("world.seed",m_seed);
	}
	else
	    m_seed = config_get_int64("world.seed");
	m_type = MGT_DEFAULT;
	
	if (config_get("world.map.type"))
	{
	    const char* type = config_get("world.map.type");
	    if (!strcmp(type,"flat"))
		m_type = MGT_FLAT;
	    else
		config_set("world.map.type","default");
	}
	else
	    config_set("world.map.type","default");
}

void ServerMap::beginSave()
{
	verifyDatabase();
	if(sqlite3_exec(m_database, "BEGIN;", NULL, NULL, NULL) != SQLITE_OK)
	    infostream<<"WARNING: beginSave() failed, saving might be slow.";
}

void ServerMap::endSave()
{
	verifyDatabase();
	if(sqlite3_exec(m_database, "COMMIT;", NULL, NULL, NULL) != SQLITE_OK)
	    infostream<<"WARNING: endSave() failed, map might not have saved.";
}

void ServerMap::saveBlock(MapBlock* const block)
{
	DSTACK(__FUNCTION_NAME);
	/*
		Dummy blocks are not written
	*/
	if(block->isDummy())
	{
		/*v3s16 p = block->getPos();
		infostream<<"ServerMap::saveBlock(): WARNING: Not writing dummy block "
				<<"("<<p.X<<","<<p.Y<<","<<p.Z<<")"<<std::endl;*/
		return;
	}

	// Format used for writing
	u8 version = SER_FMT_VER_HIGHEST;
	// Get destination
	v3s16 p3d = block->getPos();

	/*
		[0] u8 serialization version
		[1] data
	*/

	verifyDatabase();

	std::ostringstream o(std::ios_base::binary);

	o.write((char*)&version, 1);

	// Write basic data
	block->serialize(o, version);

	// Write extra data stored on disk
	block->serializeDiskExtra(o, version);

	// Write block to database

	std::string tmp = o.str();
	const char *bytes = tmp.c_str();

	if(sqlite3_bind_int64(m_database_write, 1,
					getBlockAsInteger(p3d)) != SQLITE_OK)
	    infostream<<"WARNING: Block position failed to bind: "
		      <<sqlite3_errmsg(m_database)<<std::endl;
	if(sqlite3_bind_blob(m_database_write, 2,(void *)bytes, o.tellp(),
					NULL) != SQLITE_OK) // TODO this mught not be the right length
		infostream<<"WARNING: Block data failed to bind: "
			  <<sqlite3_errmsg(m_database)<<std::endl;
	int written = sqlite3_step(m_database_write);
	if(written != SQLITE_DONE)
		infostream<<"WARNING: Block failed to save ("<<p3d.X
			  <<", "<<p3d.Y<<", "<<p3d.Z<<") "
		<<sqlite3_errmsg(m_database)<<std::endl;
	// Make ready for later reuse
	sqlite3_reset(m_database_write);

	// We just wrote it to the disk so clear modified flag
	block->resetModified();
}

void ServerMap::loadBlock(std::string *blob, v3s16 p3d, MapSector* sector,
		bool save_after_load)
{
	DSTACK(__FUNCTION_NAME);

	try
	{
	    std::istringstream is(*blob, std::ios_base::binary);

	    u8 version = SER_FMT_VER_INVALID;
	    is.read((char*)&version, 1);

	    if (is.fail())
		throw SerializationError("ServerMap::loadBlock(): Failed"
				" to read MapBlock version");

	/*u32 block_size = MapBlock::serializedLength(version);
	  SharedBuffer<u8> data(block_size);
	  is.read((char*)*data, block_size);*/

	// This will always return a sector because we're the server
	//MapSector *sector = emergeSector(p2d);

	    bool created_new = false;
	    MapBlock* block = sector->getBlockNoCreateNoEx(p3d.Y);
		
	    if (!block)
	    {
		block = sector->createBlankBlockNoInsert(p3d.Y);
		created_new = true;
	    }

	// Read basic data
	    block->deSerialize(is, version);

	// Read extra data stored on disk
	    block->deSerializeDiskExtra(is, version);

	// If it's a new block, insert it to the map
	    if (created_new)
		sector->insertBlock(block);

	/*
	  Save blocks loaded in old format in new format
	*/

	    if (version < SER_FMT_VER_HIGHEST || save_after_load)
		saveBlock(block);

	// We just loaded it from, so it's up-to-date.
	    block->resetModified();
	    block->ResetCurrent();
	}
	catch(SerializationError &e)
	{
	    infostream<<"WARNING: Invalid block data in database "
		      <<" (SerializationError). "
		      <<"what()="<<e.what()
		      <<std::endl;
	//" Ignoring. A new one will be generated.
	}
}

MapBlock* ServerMap::loadBlock(v3s16 blockpos)
{
	DSTACK(__FUNCTION_NAME);

	v2s16 p2d(blockpos.X, blockpos.Z);

	verifyDatabase();

	if (sqlite3_bind_int64(m_database_read, 1,
					getBlockAsInteger(blockpos)) != SQLITE_OK)
	    infostream<<"WARNING: Could not bind block position for load: "
		      <<sqlite3_errmsg(m_database)<<std::endl;
	
	if (sqlite3_step(m_database_read) == SQLITE_ROW)
	{
	/*
	  Make sure sector is loaded
	*/
	    MapSector* sector = createSector(p2d);

	/*
	  Load block
	*/
	    const char* data =
		(const char*) sqlite3_column_blob(m_database_read, 0);
	    size_t len = sqlite3_column_bytes(m_database_read, 0);

	    std::string datastr(data, len);

	    loadBlock(&datastr, blockpos, sector, false);

	    sqlite3_step(m_database_read);
	// We should never get more than 1 row, so ok to reset
	    sqlite3_reset(m_database_read);

	    return getBlockNoCreateNoEx(blockpos);
	}
	sqlite3_reset(m_database_read);

	return getBlockNoCreateNoEx(blockpos);
}

void ServerMap::PrintInfo(std::ostream &out)
{
	out<<"ServerMap: ";
}

#ifndef SERVER

/*
	ClientMap
*/

ClientMap::ClientMap(Client* const client,
		MapDrawControl& control,
		scene::ISceneNode* const parent,
		scene::ISceneManager* const mgr,
		s32 id) : Map(dout_client),
	scene::ISceneNode(parent, mgr, id),
	m_client(client),
	m_box(-BS*1000000,-BS*1000000,-BS*1000000,
			BS*1000000,BS*1000000,BS*1000000),
	m_control(control),
	m_camera_position(0,0,0),
	m_camera_direction(0,0,1),
	m_camera_fov(PI),
	m_render_trilinear(config_get_bool("client.video.trilinear")),
	m_render_bilinear(config_get_bool("client.video.bilinear")),
	m_render_anisotropic(config_get_bool("client.video.anisotropic")),
	m_last_drawn_sectors()
{
	m_camera_mutex.Init();
	assert(m_camera_mutex.IsInitialized());
}

ClientMap::~ClientMap()
{
}

MapSector* ClientMap::emergeSector(v2s16 p2d)
{
	DSTACK(__FUNCTION_NAME);
	// Check that it doesn't exist already
	try{
	    return getSectorNoGenerate(p2d);
	}
	catch(InvalidPositionException &e)
	{
	}

	// Create a sector
	ClientMapSector* const sector = new ClientMapSector(this, p2d);

	{
	    JMutexAutoLock lock(m_sectors_mutex);
	    m_sectors.insert(p2d, sector);
	}

	return sector;
}

void ClientMap::OnRegisterSceneNode()
{
	if(IsVisible)
	{
	    SceneManager->registerNodeForRendering(this, scene::ESNRP_SOLID);
	    SceneManager->registerNodeForRendering(this, scene::ESNRP_TRANSPARENT);
	}

	ISceneNode::OnRegisterSceneNode();
}

static bool isOccluded(Map *map, v3s16 p0, v3s16 p1, float step, float stepfac,
		float start_off, float end_off, u32 needed_count)
{
	float d0 = (float)BS * p0.getDistanceFrom(p1);
	v3s16 u0 = p1 - p0;
	v3f uf = v3f(u0.X, u0.Y, u0.Z) * BS;
	uf.normalize();
	v3f p0f = v3f(p0.X, p0.Y, p0.Z) * BS;
	u32 count = 0;
	
	for (float s=start_off; s<d0+end_off; s+=step)
	{
		const v3f pf = p0f + uf * s;
		const v3s16 p = floatToInt(pf, BS);
		const MapNode n = map->getNodeNoEx(p);
		const ContentFeatures& f = content_features(n);
		
		if (f.draw_type == CDT_CUBELIKE
				|| f.draw_type == CDT_DIRTLIKE
				|| f.draw_type == CDT_MELONLIKE)
		{
			count++;
			if (count >= needed_count)
				return true;
		}
		step *= stepfac;
	}
	return false;
}

void ClientMap::renderMap(video::IVideoDriver* driver, s32 pass)
{
	//m_dout<<DTIME<<"Rendering map..."<<std::endl;
	DSTACK(__FUNCTION_NAME);

	bool is_transparent_pass = pass == scene::ESNRP_TRANSPARENT;

	std::string prefix;
	if (pass == scene::ESNRP_SOLID)
	    prefix = "CM: solid: ";
	else
	    prefix = "CM: transparent: ";

	/*
		This is called two times per frame, reset on the non-transparent one
	*/
	if (pass == scene::ESNRP_SOLID)
	    m_last_drawn_sectors.clear();

	/*
		Get time for measuring timeout.

		Measuring time is very useful for long delays when the
		machine is swapping a lot.
	*/
	int time1 = time(0);

	m_camera_mutex.Lock();
	
	v3f camera_position = m_camera_position;
	v3f camera_direction = m_camera_direction;
	v3s16 camera_offset = m_camera_offset;
	f32 camera_fov = m_camera_fov;
	
	m_camera_mutex.Unlock();

	/*
		Get all blocks and draw all visible ones
	*/

	v3s16 cam_pos_nodes = floatToInt(camera_position, BS);

	v3s16 box_nodes_d = m_control.wanted_range * v3s16(1,1,1);

	v3s16 p_nodes_min = cam_pos_nodes - box_nodes_d;
	v3s16 p_nodes_max = cam_pos_nodes + box_nodes_d;

	// Take a fair amount as we will be dropping more out later
	// Umm... these additions are a bit strange but they are needed.
	v3s16 p_blocks_min(p_nodes_min.X / MAP_BLOCKSIZE - 3,
			p_nodes_min.Y / MAP_BLOCKSIZE - 3,
			p_nodes_min.Z / MAP_BLOCKSIZE - 3);
	v3s16 p_blocks_max(p_nodes_max.X / MAP_BLOCKSIZE + 1,
			p_nodes_max.Y / MAP_BLOCKSIZE + 1,
			p_nodes_max.Z / MAP_BLOCKSIZE + 1);

	u32 vertex_count = 0;
	u32 meshbuffer_count = 0;

	// For limiting number of mesh updates per frame
	u32 mesh_update_count = 0;

	// Number of blocks in rendering range
	u32 blocks_in_range = 0;
	// Number of blocks occlusion culled
	u32 blocks_occlusion_culled = 0;
	// Number of blocks in rendering range but don't have a mesh
	u32 blocks_in_range_without_mesh = 0;
	// Blocks that had mesh that would have been drawn according to
	// rendering range (if max blocks limit didn't kick in)
	u32 blocks_would_have_drawn = 0;
	// Blocks that were drawn and had a mesh
	u32 blocks_drawn = 0;
	// Blocks which had a corresponding meshbuffer for this pass
	u32 blocks_had_pass_meshbuf = 0;
	// Blocks from which stuff was actually drawn
	u32 blocks_without_stuff = 0;

	bool anim_textures = config_get_bool("client.graphics.texture.animations");
	float anim_time = m_client->getAnimationTime();

	/*
		Collect a set of blocks for drawing
	*/

	core::map<v3s16, MapBlock*> drawset;

	{
	ScopeProfiler sp(g_profiler, prefix+"collecting blocks for drawing", SPT_AVG);

	for(core::map<v2s16, MapSector*>::Iterator si = m_sectors.getIterator();
	    si.atEnd() == false; si++)
	{
	    m_sectors_mutex.Lock();

	    MapSector* const sector = si.getNode()->getValue();

	    if(!sector)
	    {
		m_sectors_mutex.Unlock();
		continue;
	    }
		
	    v2s16 sp = sector->getPos();

	    if(m_control.range_all == false)
	    {
		if(sp.X < p_blocks_min.X || sp.X > p_blocks_max.X
				|| sp.Y < p_blocks_min.Z
				|| sp.Y > p_blocks_max.Z)
		    continue;
	    }

	    core::list<MapBlock*> sectorblocks;
		
	    sector->getBlocks(sectorblocks);
	    m_sectors_mutex.Unlock();

	/*
	  Loop through blocks in sector
	*/

	    u32 sector_blocks_drawn = 0;
	    core::list<MapBlock*>::Iterator i;
	    
	    for(i=sectorblocks.begin(); i!=sectorblocks.end(); i++)
	    {
		MapBlock* const block = *i;

		if(!block)
		    continue;
			
	    /*
	      Compare block position to camera position, skip
	      if not seen on display
	    */

		{
		    JMutexAutoLock lock(block->mesh_mutex);

		    if (block->mesh != NULL)
			block->mesh->updateCameraOffset(camera_offset);
		}
			
		float range = 100000 * BS;
		if(m_control.range_all == false)
		    range = m_control.wanted_range * BS;

		float d = 0.0;
		if(isBlockInSight(block->getPos(), camera_position,
						camera_direction, camera_fov,
						range, &d) == false)
		    continue;

	    // This is ugly (spherical distance limit?)
	    /*if(m_control.range_all == false &&
	      d - 0.5*BS*MAP_BLOCKSIZE > range)
	      continue;*/

		blocks_in_range++;

#if 1
	    /*
	      Update expired mesh (used for day/night change)

	      It doesn't work exactly like it should now with the
	      tasked mesh update but whatever.
	    */

		bool mesh_expired = false;

		{
		    JMutexAutoLock lock(block->mesh_mutex);

		    mesh_expired = block->getMeshExpired();

		// Mesh has not been expired and there is no mesh:
		// block has no content
		    if(block->mesh == NULL && mesh_expired == false)
		    {
			blocks_in_range_without_mesh++;
			continue;
		    }
		}

		const f32 faraway = BS*100;
	    //f32 faraway = m_control.wanted_range * BS;

	    /*
	      This has to be done with the mesh_mutex unlocked
	    */
	    // Pretty random but this should work somewhat nicely
		if (mesh_expired && (d < faraway || mesh_update_count < 20
						|| (m_control.range_all && mesh_update_count < 100)))
		{
		    mesh_update_count++;

		    JMutexAutoLock lock(block->mesh_mutex);

		// Mesh has been expired: generate new mesh
		    if (block->mesh)
		    {
			m_client->addUpdateMeshTask(block->getPos(),
					false,true);
			block->setMeshExpired(false);
		    }

		    mesh_expired = false;
		}
#endif

	    /*
	      Occlusion culling
	    */

		v3s16 cpn = block->getPos() * MAP_BLOCKSIZE;
		cpn += v3s16(MAP_BLOCKSIZE/2, MAP_BLOCKSIZE/2, MAP_BLOCKSIZE/2);
			
		float step = BS*1;
		float stepfac = 1.1;
		float startoff = BS*1;
		float endoff = -BS*MAP_BLOCKSIZE*1.42*1.42;
		v3s16 spn = cam_pos_nodes + v3s16(0,0,0);
		s16 bs2 = MAP_BLOCKSIZE/2 + 1;
		u32 needed_count = 1;
			
		if(isOccluded(this, spn, cpn + v3s16(0,0,0),
						step, stepfac, startoff, endoff,
						needed_count) &&
				isOccluded(this, spn, cpn + v3s16(bs2,bs2,bs2),
						step, stepfac, startoff, endoff,
						needed_count) &&
				isOccluded(this, spn, cpn + v3s16(bs2,bs2,-bs2),
						step, stepfac, startoff,
						endoff, needed_count) &&
				isOccluded(this, spn, cpn + v3s16(bs2,-bs2,bs2),
						step, stepfac, startoff, endoff,
						needed_count) &&
				isOccluded(this, spn, cpn + v3s16(bs2,-bs2,-bs2),
						step, stepfac, startoff, endoff,
						needed_count) &&
				isOccluded(this, spn, cpn + v3s16(-bs2,bs2,bs2),
						step, stepfac, startoff, endoff,
						needed_count) &&
				isOccluded(this, spn, cpn + v3s16(-bs2,bs2,-bs2),
						step, stepfac, startoff, endoff,
						needed_count) &&
				isOccluded(this, spn, cpn + v3s16(-bs2,-bs2,bs2),
						step, stepfac, startoff, endoff,
						needed_count) &&
				isOccluded(this, spn, cpn + v3s16(-bs2,-bs2,-bs2),
						step, stepfac, startoff, endoff,
						needed_count))
		{
		    blocks_occlusion_culled++;
		    continue;
		}

	    // This block is in range. Reset usage timer.
		block->resetUsageTimer();

	    /*
	      Ignore if mesh doesn't exist
	    */
		{
		    JMutexAutoLock lock(block->mesh_mutex);
		    MapBlockMesh* const mesh = block->mesh;

		    if (mesh == NULL)
		    {
			blocks_in_range_without_mesh++;
			continue;
		    }

		    mesh->isfar = d > faraway;
		}

	    // Limit block count in case of a sudden increase
		blocks_would_have_drawn++;
		if (blocks_drawn >= m_control.wanted_max_blocks
				&& m_control.range_all == false
				&& d > m_control.wanted_min_range * BS)
		    continue;

	    // Animate textures in block mesh
		if (anim_textures)
		{
		    JMutexAutoLock lock(block->mesh_mutex);

		    if(block->mesh->isAnimated())
			block->mesh->animate(anim_time);
		}

	    // Add to set
		drawset[block->getPos()] = block;

		sector_blocks_drawn++;
		blocks_drawn++;
	    } // foreach sectorblocks

	    if (sector_blocks_drawn != 0)
		m_last_drawn_sectors[sp] = true;
	}
	} // ScopeProfiler

	/*
		Draw the selected MapBlocks
	*/

	{
	const ScopeProfiler sp(g_profiler, prefix+"drawing blocks", SPT_AVG);
	int timecheck_counter = 0;
	
	for (core::map<v3s16, MapBlock*>::Iterator i = drawset.getIterator();
	     i.atEnd() == false; i++)
	{
	    timecheck_counter++;
	    if (timecheck_counter > 50)
	    {
		timecheck_counter = 0;
		int time2 = time(0);
		if (time2 > time1 + 4)
		{
		    infostream<<"ClientMap::renderMap(): Rendering takes ages, returning."
			      <<std::endl;
		    return;
		}
	    }

	    MapBlock* const block = i.getNode()->getValue();

	/*
	  Draw the faces of the block
	*/
	    {
		JMutexAutoLock lock(block->mesh_mutex);
		MapBlockMesh* const mesh = block->mesh;
			
		if (!mesh || !mesh->getMesh())
		    continue;
			
		scene::SMesh* m = NULL;
		if (mesh->isfar)
		    m = mesh->getFarMesh();
		else
		    m = mesh->getMesh();
		if (!m)
		    continue;

		const u32 c = m->getMeshBufferCount();
		bool stuff_actually_drawn = false;
			
		for (u32 i=0; i<c; i++)
		{
		    scene::IMeshBuffer* const buf = m->getMeshBuffer(i);
		    if (buf == NULL)
			continue;

		    buf->getMaterial().setFlag(video::EMF_TRILINEAR_FILTER,
				    m_render_trilinear);
		    buf->getMaterial().setFlag(video::EMF_BILINEAR_FILTER,
				    m_render_bilinear);
		    buf->getMaterial().setFlag(video::EMF_ANISOTROPIC_FILTER,
				    m_render_anisotropic);

		    const video::SMaterial& material = buf->getMaterial();
		    video::IMaterialRenderer* rnd =
			driver->getMaterialRenderer(material.MaterialType);
		    bool transparent = (rnd && rnd->isTransparent());
		// Render transparent on transparent pass and likewise.
		    if (transparent == is_transparent_pass)
		    {
			if (buf->getVertexCount() == 0)
			    errorstream<<"Block ["<<analyze_block(block)
				       <<"] contains an empty meshbuf"<<std::endl;
		    /*
		      This *shouldn't* hurt too much because Irrlicht
		      doesn't change opengl textures if the old
		      material has the same texture.
		    */
			driver->setMaterial(buf->getMaterial());
			driver->drawMeshBuffer(buf);
			vertex_count += buf->getVertexCount();
			meshbuffer_count++;
			stuff_actually_drawn = true;
		    }
		}
		if (stuff_actually_drawn)
		    blocks_had_pass_meshbuf++;
		else
		    blocks_without_stuff++;
	    }
	}
	} // ScopeProfiler

    // Log only on solid pass because values are the same
	if(pass == scene::ESNRP_SOLID)
	{
	    g_profiler->avg("CM: blocks in range", blocks_in_range);
	    g_profiler->avg("CM: blocks occlusion culled",
			    blocks_occlusion_culled);
	    if(blocks_in_range != 0)
		g_profiler->avg("CM: blocks in range without mesh (frac)",
				(float)blocks_in_range_without_mesh/blocks_in_range);
	    g_profiler->avg("CM: blocks drawn", blocks_drawn);
	}

	g_profiler->avg(prefix+"vertices drawn", vertex_count);
	
	if(blocks_had_pass_meshbuf != 0)
	    g_profiler->avg(prefix+"meshbuffers per block",
			    (float)meshbuffer_count / (float)blocks_had_pass_meshbuf);
	if(blocks_drawn != 0)
	    g_profiler->avg(prefix+"empty blocks (frac)",
			    (float)blocks_without_stuff / blocks_drawn);

	m_control.blocks_drawn = blocks_drawn;
	m_control.blocks_would_have_drawn = blocks_would_have_drawn;

    /*infostream<<"renderMap(): is_transparent_pass="<<is_transparent_pass
      <<", rendered "<<vertex_count<<" vertices."<<std::endl;*/
}

static bool getVisibleBrightness(Map *map,v3f p0,v3f dir,
	float step,float step_multiplier,float start_distance,
	float end_distance,u32 daylight_factor,float sunlight_min_d,
	int *result,bool *sunlight_seen)
{
	int brightness_sum = 0;
	int brightness_count = 0;
	float distance = start_distance;
	
	dir.normalize();
	
	v3f pf = p0;
	pf += dir * distance;
	
	int noncount = 0;
	bool nonlight_seen = false;
	bool allow_allowing_non_sunlight_propagates = false;
	bool allow_non_sunlight_propagates = false;

    // Check content nearly at camera position
	{
	    const v3s16 p = floatToInt(p0 /*+ dir * 3*BS*/, BS);
	    const MapNode n = map->getNodeNoEx(p);
	    const ContentFeatures& nf = content_features(n.getContent());
		
	    if (nf.param_type == CPT_LIGHT && !nf.sunlight_propagates)
		allow_allowing_non_sunlight_propagates = true;
	}
    // If would start at CONTENT_IGNORE, start closer
	{
	    const v3s16 p = floatToInt(pf, BS);
	    const MapNode n = map->getNodeNoEx(p);
		
	    if (n.getContent() == CONTENT_IGNORE)
	    {
		const float newd = 2*BS;
			
		pf = p0 + dir * 2*newd;
		distance = newd;
	    }
	}

	for (int i=0; distance < end_distance; i++)
	{
	    pf += dir * step;
	    distance += step;
	    step *= step_multiplier;
		
	    const v3s16 p = floatToInt(pf, BS);
	    const MapNode n = map->getNodeNoEx(p);
	    const ContentFeatures& nf = content_features(n.getContent());
		
	    if (allow_allowing_non_sunlight_propagates
			    && i == 0
			    && nf.param_type == CPT_LIGHT
			    && !nf.sunlight_propagates)
		allow_non_sunlight_propagates = true;

	    if (nf.param_type != CPT_LIGHT
			    || (!nf.sunlight_propagates
					    && !allow_non_sunlight_propagates))
	    {
		nonlight_seen = true;
		noncount++;
		if (noncount >= 4)
		    break;
		continue;
	    }
		
	    if (distance >= sunlight_min_d
			    && *sunlight_seen == false
			    && nonlight_seen == false
			    && n.getLight(LIGHTBANK_DAY) == LIGHT_SUN)
		*sunlight_seen = true;
		
	    noncount = 0;
	    brightness_sum += decode_light(n.getLightBlend(daylight_factor));
	    brightness_count++;
	}

	*result = 0;

	if (brightness_count == 0)
	    return false;
	*result = brightness_sum / brightness_count;
	return true;
}

int ClientMap::getBackgroundBrightness(float max_d,u32 daylight_factor,
	int oldvalue,bool *sunlight_seen_result)
{
	static v3f z_directions[50] = {
	    v3f(-100, 0, 0)
	};
	static f32 z_offsets[sizeof(z_directions)/sizeof(*z_directions)] = {
	    -1000,
	};
	
	if (z_directions[0].X < -99)
	{
	    for (u32 i=0; i<sizeof(z_directions)/sizeof(*z_directions); i++)
	    {
		z_directions[i] = v3f(0.01 * myrand_range(-80, 80),
				1.0,
				0.01 * myrand_range(-80, 80));
		z_offsets[i] = 0.01 * myrand_range(0,100);
	    }
	}

	//std::cerr<<"In goes "<<PP(m_camera_direction)<<", out comes ";

	int sunlight_seen_count = 0;
	float sunlight_min_d = max_d * 0.8;
	if (sunlight_min_d > 35*BS)
		sunlight_min_d = 35*BS;
	
	m_camera_mutex.Lock();
	const v3f camera_position = m_camera_position;
	const v3f camera_direction = m_camera_direction;
	m_camera_mutex.Unlock();
	
	core::array<int> values;

	for (u32 i=0; i<sizeof(z_directions)/sizeof(*z_directions); i++)
	{
	    v3f z_dir = z_directions[i];
	    z_dir.normalize();
	    core::CMatrix4<f32> a;
	    a.buildRotateFromTo(v3f(0,1,0), z_dir);
	    v3f dir = camera_direction;
	    a.rotateVect(dir);
	    int br = 0;
	    float step = BS*1.5;
	    if (max_d > 35*BS)
		step = max_d / 35 * 1.5;
	    float off = step * z_offsets[i];
	    bool sunlight_seen_now = false;
	    bool ok = getVisibleBrightness(this,camera_position,dir,
			    step,1.0,max_d*0.6+off,max_d,daylight_factor,
			    sunlight_min_d,&br,&sunlight_seen_now);
	    if (sunlight_seen_now)
		sunlight_seen_count++;
	    if (!ok)
		continue;
	    values.push_back(br);
	// Don't try too much if being in the sun is clear
	    if (sunlight_seen_count >= 20)
		break;
	}

	int brightness_sum = 0;
	int brightness_count = 0;
	
	values.sort();
	
	u32 num_values_to_use = values.size();
	
	if (num_values_to_use >= 10)
	    num_values_to_use -= num_values_to_use/2;
	else if (num_values_to_use >= 7)
	    num_values_to_use -= num_values_to_use/3;

	u32 first_value_i = (values.size() - num_values_to_use) / 2;
	for (u32 i=first_value_i; i < first_value_i+num_values_to_use; i++)
	{
	    brightness_sum += values[i];
	    brightness_count++;
	}

	int ret = 0;
	if (brightness_count == 0)
	{
	    const MapNode n = getNodeNoEx(floatToInt(camera_position, BS));
		
	    if (content_features(n).param_type == CPT_LIGHT)
		ret = decode_light(n.getLightBlend(daylight_factor));
	    else
		ret = oldvalue;
	}
	else
	    ret = brightness_sum / brightness_count;

	*sunlight_seen_result = (sunlight_seen_count > 0);
	return ret;
}

void ClientMap::renderPostFx()
{
    // Sadly ISceneManager has no "post effects" render pass, in that case we
    // could just register for that and handle it in renderMap().

	m_camera_mutex.Lock();
	const v3f camera_position = m_camera_position;
	m_camera_mutex.Unlock();
	video::SColor post_effect_color(0,0,0,0);

	if (m_client->getSleepAlpha() > 0)
	    post_effect_color = video::SColor(m_client->getSleepAlpha(),0,0,0);
	else
	{
	    const MapNode n = getNodeNoEx(floatToInt(camera_position, BS));
	// - If the player is in a solid node, make everything black.
	// - If the player is in liquid, draw a semi-transparent overlay.
	    const ContentFeatures& f = content_features(n);
		
	    post_effect_color = f.post_effect_color;
		
	    if (f.draw_type == CDT_CUBELIKE
			    || f.draw_type == CDT_DIRTLIKE
			    || f.draw_type == CDT_MELONLIKE)
		post_effect_color = video::SColor(255, 0, 0, 0);
	}
	
	if (post_effect_color.getAlpha() != 0)
	{
	// Draw a full-screen rectangle
	    video::IVideoDriver* const driver = SceneManager->getVideoDriver();
	    const v2u32 ss = driver->getScreenSize();
	    const core::rect<s32> rect(0,0, ss.X, ss.Y);
		
	    driver->draw2DRectangle(post_effect_color, rect);
	}

	if (m_client->getServerSuffocation())
	{
	    const u16 a = m_client->getAir();
	    if (a < 50)
	    {
		char buff[1024];
		const u8 c = 255 - (a*5);
		const video::SColor color(c,255,255,255);
		const video::SColor colors[] = {color,color,color,color};

		if (path_get((char*)"texture",(char*)"low_air.png",1,buff,1024))
		{
		    video::IVideoDriver* const driver = SceneManager->getVideoDriver();
		    const v2u32 ss = driver->getScreenSize();
		    video::ITexture* const texture = driver->getTexture(buff);
		    const core::rect<s32> rect(0,0,ss.X,ss.Y);
				
		    driver->draw2DImage(texture,rect,
				    core::rect<s32>(core::position2d<s32>(0,0),
						    core::dimension2di(texture->getOriginalSize())),
				    NULL,colors,true);
		}
	    }
	}
}

void ClientMap::expireMeshes(bool only_daynight_diffed)
{
	TimeTaker timer("expireMeshes()");
	core::map<v2s16, MapSector*>::Iterator si = m_sectors.getIterator();
    // PB Lock ?
	
	m_sectors_mutex.Lock();

	for (; si.atEnd() == false; si++)
	{
	    MapSector* const sector = si.getNode()->getValue();
	    core::list<MapBlock*> sectorblocks;
		
	    sector->getBlocks(sectorblocks);

	    core::list<MapBlock*>::Iterator i;
	    for (i=sectorblocks.begin(); i!=sectorblocks.end(); i++)
	    {
		MapBlock* const block = *i;

		if (only_daynight_diffed
				&& block->dayNightDiffed() == false)
		    continue;

		{
		    JMutexAutoLock lock(block->mesh_mutex);
		    if (block->mesh != NULL)
			block->setMeshExpired(true);
		}
	    }
	}
	
	m_sectors_mutex.Unlock();
}

void ClientMap::PrintInfo(std::ostream &out)
{
	out<<"ClientMap: ";
}

#endif // !SERVER

/*
	MapVoxelManipulator
*/

MapVoxelManipulator::MapVoxelManipulator(Map *map)
{
	m_map = map;
}

MapVoxelManipulator::~MapVoxelManipulator()
{
	/*infostream<<"MapVoxelManipulator: blocks: "<<m_loaded_blocks.size()
			<<std::endl;*/
}

void MapVoxelManipulator::emerge(VoxelArea a, s32 caller_id)
{
	TimeTaker timer1("emerge", &emerge_time);

	// Units of these are MapBlocks
	const v3s16 p_min = getNodeBlockPos(a.MinEdge);
	const v3s16 p_max = getNodeBlockPos(a.MaxEdge);
	const VoxelArea block_area_nodes(p_min * MAP_BLOCKSIZE,
			(p_max + 1) * MAP_BLOCKSIZE-v3s16(1,1,1));

	addArea(block_area_nodes);

	for(s32 z=p_min.Z; z<=p_max.Z; z++)
	for(s32 y=p_min.Y; y<=p_max.Y; y++)
	for(s32 x=p_min.X; x<=p_max.X; x++)
	{
	    const v3s16 p(x,y,z);
	    core::map<v3s16, bool>::Node* const n = m_loaded_blocks.find(p);
	    if(n != NULL)
		continue;

	    bool block_data_inexistent = false;
	    try
	    {
		TimeTaker timer1("emerge load", &emerge_load_time);

	    /*infostream<<"Loading block (caller_id="<<caller_id<<")"
	      <<" ("<<p.X<<","<<p.Y<<","<<p.Z<<")"
	      <<" wanted area: ";
	      a.print(infostream);
	      infostream<<std::endl;*/

		MapBlock* const block = m_map->getBlockNoCreate(p);
		if(block->isDummy())
		    block_data_inexistent = true;
		else
		    block->copyTo(*this);
		block->ResetCurrent();
	    }
	    catch(InvalidPositionException &e)
	    {
		block_data_inexistent = true;
	    }

	    if(block_data_inexistent)
	    {
		const VoxelArea a(p*MAP_BLOCKSIZE, (p+1)*MAP_BLOCKSIZE-v3s16(1,1,1));
			
	    // Fill with VOXELFLAG_INEXISTENT
		for(s32 z=a.MinEdge.Z; z<=a.MaxEdge.Z; z++)
		    for(s32 y=a.MinEdge.Y; y<=a.MaxEdge.Y; y++)
		    {
			const s32 i = m_area.index(a.MinEdge.X,y,z);
			memset(&m_flags[i], VOXELFLAG_INEXISTENT, MAP_BLOCKSIZE);
		    }
	    }

	    m_loaded_blocks.insert(p, !block_data_inexistent);
	}

	//infostream<<"emerge done"<<std::endl;
}

/*
	SUGG: Add an option to only update eg. water and air nodes.
	      This will make it interfere less with important stuff if
		  run on background.
*/
void MapVoxelManipulator::blitBack(core::map<v3s16, MapBlock*> & modified_blocks)
{
	if (m_area.getExtent() == v3s16(0,0,0))
		return;

	//TimeTaker timer1("blitBack");

	/*infostream<<"blitBack(): m_loaded_blocks.size()="
			<<m_loaded_blocks.size()<<std::endl;*/

	/*
		Initialize block cache
	*/
	v3s16 blockpos_last;
	MapBlock* block = NULL;
	bool block_checked_in_modified = false;

	for (s32 z=m_area.MinEdge.Z; z<=m_area.MaxEdge.Z; z++)
	for (s32 y=m_area.MinEdge.Y; y<=m_area.MaxEdge.Y; y++)
	for (s32 x=m_area.MinEdge.X; x<=m_area.MaxEdge.X; x++)
	{
	    const v3s16 p(x,y,z);

	    u8 f = m_flags[m_area.index(p)];
	    if (f & (VOXELFLAG_NOT_LOADED|VOXELFLAG_INEXISTENT))
		continue;

	    MapNode& n = m_data[m_area.index(p)];
	    const v3s16 blockpos = getNodeBlockPos(p);

	    if (block == NULL || blockpos != blockpos_last)
	    {
		    
		if(block)
		    block->ResetCurrent();
		block = m_map->getBlockNoCreate(blockpos);
		blockpos_last = blockpos;
		block_checked_in_modified = false;
	    }

	// Calculate relative position in block
	    const v3s16 relpos = p - blockpos * MAP_BLOCKSIZE;

	// Don't continue if nothing has changed here
	    bool is_valid_position;
	    if (block->getNode(relpos,&is_valid_position) == n
			    || !is_valid_position)
		continue;

	//m_map->setNode(m_area.MinEdge + p, n);
	    block->setNode(relpos, n);

	/*
	  Make sure block is in modified_blocks
	*/
	    if (block_checked_in_modified == false)
	    {
		modified_blocks[blockpos] = block;
		block_checked_in_modified = true;
	    }
	}
	
	if(block)
	    block->ResetCurrent();
}

ManualMapVoxelManipulator::ManualMapVoxelManipulator(Map *map):
		MapVoxelManipulator(map),
		m_create_area(false)
{
}

ManualMapVoxelManipulator::~ManualMapVoxelManipulator()
{
}

void ManualMapVoxelManipulator::emerge(VoxelArea a, s32 caller_id)
{
	// Just create the area so that it can be pointed to
	VoxelManipulator::emerge(a, caller_id);
}

void ManualMapVoxelManipulator::initialEmerge(
		v3s16 blockpos_min, v3s16 blockpos_max)
{
	TimeTaker timer1("initialEmerge", &emerge_time);

	// Units of these are MapBlocks
	const v3s16 p_min = blockpos_min;
	const v3s16 p_max = blockpos_max;
	const VoxelArea block_area_nodes
			(p_min*MAP_BLOCKSIZE,
					(p_max+1)*MAP_BLOCKSIZE-v3s16(1,1,1));
	const u32 size_MB = block_area_nodes.getVolume()*4/1000000;
	
	if(size_MB >= 1)
	{
		infostream<<"initialEmerge: area: ";
		block_area_nodes.print(infostream);
		infostream<<" ("<<size_MB<<"MB)";
		infostream<<std::endl;
	}

	addArea(block_area_nodes);

	for(s32 z=p_min.Z; z<=p_max.Z; z++)
	for(s32 y=p_min.Y; y<=p_max.Y; y++)
	for(s32 x=p_min.X; x<=p_max.X; x++)
	{
	    const v3s16 p(x,y,z);
	    core::map<v3s16, bool>::Node* const n = m_loaded_blocks.find(p);
	    if(n != NULL)
		continue;

	    bool block_data_inexistent = false;
	    try
	    {
		TimeTaker timer1("emerge load", &emerge_load_time);
		MapBlock* const block = m_map->getBlockNoCreate(p);
		if(block->isDummy())
		    block_data_inexistent = true;
		else
		    block->copyTo(*this);
		block->ResetCurrent();
	    }
	    catch(InvalidPositionException &e)
	    {
		block_data_inexistent = true;
	    }

	    if(block_data_inexistent)
	    {
	    /*
	      Mark area inexistent
	    */
		const VoxelArea a(p*MAP_BLOCKSIZE,
				(p+1)*MAP_BLOCKSIZE-v3s16(1,1,1));
	    // Fill with VOXELFLAG_INEXISTENT
		for(s32 z=a.MinEdge.Z; z<=a.MaxEdge.Z; z++)
		    for(s32 y=a.MinEdge.Y; y<=a.MaxEdge.Y; y++)
		    {
			s32 i = m_area.index(a.MinEdge.X,y,z);
			memset(&m_flags[i], VOXELFLAG_INEXISTENT, MAP_BLOCKSIZE);
		    }
	    }

	    m_loaded_blocks.insert(p, !block_data_inexistent);
	}
}

void ManualMapVoxelManipulator::blitBackAll(
		core::map<v3s16, MapBlock*> * modified_blocks)
{
	if (m_area.getExtent() == v3s16(0,0,0))
	    return;

    /*
      Copy data of all blocks
    */
	for (core::map<v3s16, bool>::Iterator i = m_loaded_blocks.getIterator();
	     i.atEnd() == false; i++)
	{
	    const v3s16 p = i.getNode()->getKey();
	    const bool existed = i.getNode()->getValue();
		
	    if (existed == false)
	    {
	    // The Great Bug was found using this
	    /*infostream << "ManualMapVoxelManipulator::blitBackAll: "
	      << "Inexistent (" << p.X << "," << p.Y << "," << p.Z<<")"
	      << std::endl;*/
		continue;
	    }
		
	    MapBlock* const block = m_map->getBlockNoCreateNoEx(p);
	    if (!block)
	    {
		infostream<<"WARNING: "<<__FUNCTION_NAME
			  <<": got NULL block "
			  <<"("<<p.X<<","<<p.Y<<","<<p.Z<<")"
			  <<std::endl;
		continue;
	    }

	    block->copyFrom(*this);

	    if (modified_blocks)
		modified_blocks->insert(p, block);
	    block->ResetCurrent();
	}
}

void ManualMapVoxelManipulator::blitBackAllWithMeta(
		core::map<v3s16, MapBlock*> * modified_blocks)
{
	if (m_area.getExtent() == v3s16(0,0,0))
		return;

	/*
	  Copy data of all blocks
	*/
	for (core::map<v3s16, bool>::Iterator i = m_loaded_blocks.getIterator();
	     i.atEnd() == false; i++)
	{
	    const v3s16 p = i.getNode()->getKey();
	    const bool existed = i.getNode()->getValue();
		
	    if (existed == false)
	    {
	    // The Great Bug was found using this
	    /*infostream<<"ManualMapVoxelManipulator::blitBackAll: "
	      <<"Inexistent ("<<p.X<<","<<p.Y<<","<<p.Z<<")"
	      <<std::endl;*/
		continue;
	    }
		
	    MapBlock* const block = m_map->getBlockNoCreateNoEx(p);
	    if (!block)
	    {
		infostream<<"WARNING: "<<__FUNCTION_NAME
			  <<": got NULL block "
			  <<"("<<p.X<<","<<p.Y<<","<<p.Z<<")"
			  <<std::endl;
		continue;
	    }

	    block->copyFrom(*this);

	    if (modified_blocks)
		modified_blocks->insert(p, block);
	    block->ResetCurrent();
	}
	if (!modified_blocks)
	    return;
	
	// iterate over the modified blocks search for
	// nodes that have metadata that shouldn't
	// nodes that don't have metadata that should
	// nodes that have the wrong metadata
	for (core::map<v3s16,MapBlock*>::Iterator i = modified_blocks->getIterator(); i.atEnd() == false; i++)
	{
	    MapBlock* const block = i.getNode()->getValue();
	    if (block == NULL)
		continue;
		
	    v3s16 p0;
	    for(p0.X=0; p0.X<MAP_BLOCKSIZE; p0.X++)
	    for(p0.Y=0; p0.Y<MAP_BLOCKSIZE; p0.Y++)
	    for(p0.Z=0; p0.Z<MAP_BLOCKSIZE; p0.Z++)
	    {
		const MapNode n = block->getNodeNoEx(p0);
			
		if (content_features(n).initial_metadata != NULL)
		{
		    NodeMetadata* const f =
			content_features(n).initial_metadata;
		    NodeMetadata* const a = block->m_node_metadata.get(p0);
		    if (!a || f->typeId() != a->typeId())
			block->m_node_metadata.set(p0,f->clone());
		}
		else if (block->m_node_metadata.get(p0) != NULL)
		    block->m_node_metadata.remove(p0);
	    }
	}
}

//END
