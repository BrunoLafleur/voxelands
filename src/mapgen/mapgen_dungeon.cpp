/************************************************************************
* Minetest-c55
* Copyright (C) 2010-2011 celeron55, Perttu Ahola <celeron55@gmail.com>
*
* mapgen.cpp
* voxelands - 3d voxel world sandbox game
* Copyright (C) Lisa 'darkrose' Milne 2014-2017 <lisa@ltmnet.com>
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

#include "mapgen.h"
#include "voxel.h"
#include "content_mapnode.h"
#include "map.h"
#include "noise.h"

namespace mapgen
{


static void make_room1(VoxelManipulator &vmanip, v3s16 roomsize, v3s16 roomplace)
{
	// Make +-X walls
	for (s16 z=0; z<roomsize.Z; z++)
	for (s16 y=0; y<roomsize.Y; y++) {
		{
			v3s16 p = roomplace + v3s16(0,y,z);
			if (vmanip.m_area.contains(p) == false)
				continue;
			u32 vi = vmanip.m_area.index(p);
			if (vmanip.m_flags[vi] & VMANIP_FLAG_DUNGEON_UNTOUCHABLE)
				continue;
			vmanip.m_data[vi] = MapNode(CONTENT_COBBLE);
		}
		{
			v3s16 p = roomplace + v3s16(roomsize.X-1,y,z);
			if (vmanip.m_area.contains(p) == false)
				continue;
			u32 vi = vmanip.m_area.index(p);
			if (vmanip.m_flags[vi] & VMANIP_FLAG_DUNGEON_UNTOUCHABLE)
				continue;
			vmanip.m_data[vi] = MapNode(CONTENT_COBBLE);
		}
	}

	// Make +-Z walls
	for (s16 x=0; x<roomsize.X; x++)
	for (s16 y=0; y<roomsize.Y; y++) {
		{
			v3s16 p = roomplace + v3s16(x,y,0);
			if (vmanip.m_area.contains(p) == false)
				continue;
			u32 vi = vmanip.m_area.index(p);
			if (vmanip.m_flags[vi] & VMANIP_FLAG_DUNGEON_UNTOUCHABLE)
				continue;
			vmanip.m_data[vi] = MapNode(CONTENT_COBBLE);
		}
		{
			v3s16 p = roomplace + v3s16(x,y,roomsize.Z-1);
			if (vmanip.m_area.contains(p) == false)
				continue;
			u32 vi = vmanip.m_area.index(p);
			if (vmanip.m_flags[vi] & VMANIP_FLAG_DUNGEON_UNTOUCHABLE)
				continue;
			vmanip.m_data[vi] = MapNode(CONTENT_COBBLE);
		}
	}

	// Make +-Y walls (floor and ceiling)
	for (s16 z=0; z<roomsize.Z; z++)
	for (s16 x=0; x<roomsize.X; x++) {
		{
			v3s16 p = roomplace + v3s16(x,0,z);
			if (vmanip.m_area.contains(p) == false)
				continue;
			u32 vi = vmanip.m_area.index(p);
			if (vmanip.m_flags[vi] & VMANIP_FLAG_DUNGEON_UNTOUCHABLE)
				continue;
			vmanip.m_data[vi] = MapNode(CONTENT_COBBLE);
		}
		{
			v3s16 p = roomplace + v3s16(x,roomsize.Y-1,z);
			if (vmanip.m_area.contains(p) == false)
				continue;
			u32 vi = vmanip.m_area.index(p);
			if (vmanip.m_flags[vi] & VMANIP_FLAG_DUNGEON_UNTOUCHABLE)
				continue;
			vmanip.m_data[vi] = MapNode(CONTENT_COBBLE);
		}
	}

	// Fill with air
	for (s16 z=1; z<roomsize.Z-1; z++)
	for (s16 y=1; y<roomsize.Y-1; y++)
	for (s16 x=1; x<roomsize.X-1; x++) {
		v3s16 p = roomplace + v3s16(x,y,z);
		if (vmanip.m_area.contains(p) == false)
			continue;
		u32 vi = vmanip.m_area.index(p);
		vmanip.m_flags[vi] |= VMANIP_FLAG_DUNGEON_UNTOUCHABLE;
		vmanip.m_data[vi] = MapNode(CONTENT_AIR);
	}

	//if (myrand_range(0,4) == 0) {
	{
		v3s16 p = roomplace + v3s16(2,1,2);
		if (vmanip.m_area.contains(p) == false)
			return;
		u32 vi = vmanip.m_area.index(p);
		if (vmanip.m_data[vi].getContent() == CONTENT_AIR) {
			vmanip.m_flags[vi] |= VMANIP_FLAG_DUNGEON_UNTOUCHABLE;
			vmanip.m_data[vi] = MapNode(CONTENT_CHEST);
		}
	}
}

static void make_fill(VoxelManipulator &vmanip, v3s16 place, v3s16 size,
		u8 avoid_flags, MapNode n, u8 or_flags)
{
	for (s16 z=0; z<size.Z; z++)
	for (s16 y=0; y<size.Y; y++)
	for (s16 x=0; x<size.X; x++) {
		v3s16 p = place + v3s16(x,y,z);
		if (vmanip.m_area.contains(p) == false)
			continue;
		u32 vi = vmanip.m_area.index(p);
		if (vmanip.m_flags[vi] & avoid_flags)
			continue;
		if (
			vmanip.m_data[vi].getContent() != CONTENT_STONE
			&& vmanip.m_data[vi].getContent() != CONTENT_SAND
			&& vmanip.m_data[vi].getContent() != CONTENT_GRAVEL
			&& vmanip.m_data[vi].getContent() != CONTENT_COBBLE
			&& vmanip.m_data[vi].getContent() != CONTENT_IGNORE
		)
			continue;
		vmanip.m_flags[vi] |= or_flags;
		vmanip.m_data[vi] = n;
	}
}

static void make_hole1(VoxelManipulator &vmanip, v3s16 place)
{
	make_fill(vmanip, place, v3s16(1,2,1), 0, MapNode(CONTENT_AIR), VMANIP_FLAG_DUNGEON_INSIDE);
}

static void make_door1(VoxelManipulator &vmanip, v3s16 doorplace, v3s16 doordir)
{
	make_hole1(vmanip, doorplace);
}

static v3s16 rand_ortho_dir(PseudoRandom &random)
{
	if (random.next()%2==0) {
		return random.next()%2 ? v3s16(-1,0,0) : v3s16(1,0,0);
	}else{
		return random.next()%2 ? v3s16(0,0,-1) : v3s16(0,0,1);
	}
}

static v3s16 turn_xz(v3s16 olddir, int t)
{
	v3s16 dir;
	if (t == 0) {
		// Turn right
		dir.X = olddir.Z;
		dir.Z = -olddir.X;
		dir.Y = olddir.Y;
	}else{
		// Turn left
		dir.X = -olddir.Z;
		dir.Z = olddir.X;
		dir.Y = olddir.Y;
	}
	return dir;
}

static v3s16 random_turn(PseudoRandom &random, v3s16 olddir)
{
	int turn = random.range(0,2);
	v3s16 dir;
	if (turn == 0) {
		// Go straight
		dir = olddir;
	}else if (turn == 1) {
		// Turn right
		dir = turn_xz(olddir, 0);
	}else{
		// Turn left
		dir = turn_xz(olddir, 1);
	}
	return dir;
}

static void make_corridor(VoxelManipulator &vmanip, v3s16 doorplace,
		v3s16 doordir, v3s16 &result_place, v3s16 &result_dir,
		PseudoRandom &random)
{
	make_hole1(vmanip, doorplace);
	v3s16 p0 = doorplace;
	v3s16 dir = doordir;
	u32 length;
	if (random.next()%2) {
		length = random.range(1,13);
	}else{
		length = random.range(1,6);
	}
	length = random.range(1,13);
	u32 partlength = random.range(1,13);
	u32 partcount = 0;
	s16 make_stairs = 0;
	if (random.next()%2 == 0 && partlength >= 3)
		make_stairs = random.next()%2 ? 1 : -1;

	for (u32 i=0; i<length; i++) {
		v3s16 p = p0 + dir;
		if (partcount != 0)
			p.Y += make_stairs;

		if (vmanip.m_area.contains(p) == true && vmanip.m_area.contains(p+v3s16(0,1,0)) == true) {
			if (make_stairs) {
				make_fill(vmanip, p+v3s16(-1,-1,-1), v3s16(3,5,3), VMANIP_FLAG_DUNGEON_UNTOUCHABLE, MapNode(CONTENT_COBBLE), 0);
				make_fill(vmanip, p, v3s16(1,2,1), 0, MapNode(CONTENT_AIR), VMANIP_FLAG_DUNGEON_INSIDE);
				make_fill(vmanip, p-dir, v3s16(1,2,1), 0, MapNode(CONTENT_AIR), VMANIP_FLAG_DUNGEON_INSIDE);
			}else{
				make_fill(vmanip, p+v3s16(-1,-1,-1), v3s16(3,4,3), VMANIP_FLAG_DUNGEON_UNTOUCHABLE, MapNode(CONTENT_COBBLE), 0);
				make_hole1(vmanip, p);
			}

			p0 = p;
		}else{
			// Can't go here, turn away
			dir = turn_xz(dir, random.range(0,1));
			make_stairs = -make_stairs;
			partcount = 0;
			partlength = random.range(1,length);
			continue;
		}

		partcount++;
		if (partcount >= partlength) {
			partcount = 0;

			dir = random_turn(random, dir);

			partlength = random.range(1,length);

			make_stairs = 0;
			if (random.next()%2 == 0 && partlength >= 3)
				make_stairs = random.next()%2 ? 1 : -1;
		}
	}
	result_place = p0;
	result_dir = dir;
}

class RoomWalker
{
public:

	RoomWalker(VoxelManipulator &vmanip_, v3s16 pos, PseudoRandom &random):
			vmanip(vmanip_),
			m_pos(pos),
			m_random(random)
	{
		randomizeDir();
	}

	void randomizeDir()
	{
		m_dir = rand_ortho_dir(m_random);
	}

	void setPos(v3s16 pos)
	{
		m_pos = pos;
	}

	void setDir(v3s16 dir)
	{
		m_dir = dir;
	}

	bool findPlaceForDoor(v3s16 &result_place, v3s16 &result_dir)
	{
		for (u32 i=0; i<100; i++) {
			v3s16 p = m_pos + m_dir;
			v3s16 p1 = p + v3s16(0,1,0);
			if (
				vmanip.m_area.contains(p) == false
				|| vmanip.m_area.contains(p1) == false
				|| i % 4 == 0
			) {
				randomizeDir();
				continue;
			}
			if (
				vmanip.getNodeNoExNoEmerge(p).getContent() == CONTENT_COBBLE
				&& vmanip.getNodeNoExNoEmerge(p1).getContent() == CONTENT_COBBLE
			) {
				// Found wall, this is a good place!
				result_place = p;
				result_dir = m_dir;
				// Randomize next direction
				randomizeDir();
				return true;
			}
			/*
				Determine where to move next
			*/
			// Jump one up if the actual space is there
			if (
				vmanip.getNodeNoExNoEmerge(p+v3s16(0,0,0)).getContent() == CONTENT_COBBLE
				&& vmanip.getNodeNoExNoEmerge(p+v3s16(0,1,0)).getContent() == CONTENT_AIR
				&& vmanip.getNodeNoExNoEmerge(p+v3s16(0,2,0)).getContent() == CONTENT_AIR
			)
				p += v3s16(0,1,0);
			// Jump one down if the actual space is there
			if (
				vmanip.getNodeNoExNoEmerge(p+v3s16(0,1,0)).getContent() == CONTENT_COBBLE
				&& vmanip.getNodeNoExNoEmerge(p+v3s16(0,0,0)).getContent() == CONTENT_AIR
				&& vmanip.getNodeNoExNoEmerge(p+v3s16(0,-1,0)).getContent() == CONTENT_AIR
			)
				p += v3s16(0,-1,0);
			// Check if walking is now possible
			if (
				vmanip.getNodeNoExNoEmerge(p).getContent() != CONTENT_AIR
				|| vmanip.getNodeNoExNoEmerge(p+v3s16(0,1,0)).getContent() != CONTENT_AIR
			) {
				// Cannot continue walking here
				randomizeDir();
				continue;
			}
			// Move there
			m_pos = p;
		}
		return false;
	}

	bool findPlaceForRoomDoor(v3s16 roomsize, v3s16 &result_doorplace, v3s16 &result_doordir, v3s16 &result_roomplace)
	{
		for (s16 trycount=0; trycount<30; trycount++) {
			v3s16 doorplace;
			v3s16 doordir;
			bool r = findPlaceForDoor(doorplace, doordir);
			if (r == false)
				continue;
			v3s16 roomplace;
			// X east, Z north, Y up
			if (doordir == v3s16(1,0,0)) // X+
				roomplace = doorplace + v3s16(0,-1,m_random.range(-roomsize.Z+2,-2));
			if (doordir == v3s16(-1,0,0)) // X-
				roomplace = doorplace + v3s16(-roomsize.X+1,-1,m_random.range(-roomsize.Z+2,-2));
			if (doordir == v3s16(0,0,1)) // Z+
				roomplace = doorplace + v3s16(m_random.range(-roomsize.X+2,-2),-1,0);
			if (doordir == v3s16(0,0,-1)) // Z-
				roomplace = doorplace + v3s16(m_random.range(-roomsize.X+2,-2),-1,-roomsize.Z+1);

			// Check fit
			bool fits = true;
			for (s16 z=1; z<roomsize.Z-1; z++)
			for (s16 y=1; y<roomsize.Y-1; y++)
			for (s16 x=1; x<roomsize.X-1; x++) {
				v3s16 p = roomplace + v3s16(x,y,z);
				if (vmanip.m_area.contains(p) == false) {
					fits = false;
					break;
				}
				if (vmanip.m_flags[vmanip.m_area.index(p)] & VMANIP_FLAG_DUNGEON_INSIDE) {
					fits = false;
					break;
				}
			}
			// Find new place
			if (fits == false)
				continue;
			result_doorplace = doorplace;
			result_doordir = doordir;
			result_roomplace = roomplace;
			return true;
		}
		return false;
	}

private:
	VoxelManipulator &vmanip;
	v3s16 m_pos;
	v3s16 m_dir;
	PseudoRandom &m_random;
};

void make_dungeon1(VoxelManipulator &vmanip, PseudoRandom &random)
{
	v3s16 areasize = vmanip.m_area.getExtent();
	v3s16 roomsize;
	v3s16 roomplace;

	/*
		Find place for first room
	*/
	bool fits = false;
	for (u32 i=0; i<100; i++) {
		roomsize = v3s16(random.range(4,8),random.range(4,6),random.range(4,8));
		roomplace = vmanip.m_area.MinEdge + v3s16(
			random.range(0,areasize.X-roomsize.X-1),
			random.range(0,areasize.Y-roomsize.Y-1),
			random.range(0,areasize.Z-roomsize.Z-1)
		);
		/*
			Check that we're not putting the room to an unknown place,
			otherwise it might end up floating in the air
		*/
		fits = true;
		for (s16 z=1; z<roomsize.Z-1; z++)
		for (s16 y=1; y<roomsize.Y-1; y++)
		for (s16 x=1; x<roomsize.X-1; x++) {
			v3s16 p = roomplace + v3s16(x,y,z);
			u32 vi = vmanip.m_area.index(p);
			if (vmanip.m_flags[vi] & VMANIP_FLAG_DUNGEON_INSIDE) {
				fits = false;
				break;
			}
			if (vmanip.m_data[vi].getContent() == CONTENT_IGNORE) {
				fits = false;
				break;
			}
		}
		if (fits)
			break;
	}
	// No place found
	if (fits == false)
		return;

	/*
		Stores the center position of the last room made, so that
		a new corridor can be started from the last room instead of
		the new room, if chosen so.
	*/
	v3s16 last_room_center = roomplace+v3s16(roomsize.X/2,1,roomsize.Z/2);

	u32 room_count = random.range(2,7);
	for (u32 i=0; i<room_count; i++) {

		v3s16 room_center = roomplace + v3s16(roomsize.X/2,1,roomsize.Z/2);

		// dungeons should only be made in stone, so if room_center isn't stone
		// stop making a dungeon
		if (vmanip.m_data[vmanip.m_area.index(room_center)].getContent() != CONTENT_STONE)
			break;

		// Make a room to the determined place
		make_room1(vmanip, roomsize, roomplace);

		// Place torch at room center (for testing)
		//vmanip.m_data[vmanip.m_area.index(room_center)] = MapNode(CONTENT_TORCH);

		// Quit if last room
		if (i == room_count-1)
			break;

		// Determine walker start position

		bool start_in_last_room = (random.range(0,2)!=0);
		//bool start_in_last_room = true;

		v3s16 walker_start_place;

		if (start_in_last_room) {
			walker_start_place = last_room_center;
		}else{
			walker_start_place = room_center;
			// Store center of current room as the last one
			last_room_center = room_center;
		}

		// Create walker and find a place for a door
		RoomWalker walker(vmanip, walker_start_place, random);
		v3s16 doorplace;
		v3s16 doordir;
		bool r = walker.findPlaceForDoor(doorplace, doordir);
		if (r == false)
			return;

		if (random.range(0,1)==0) {
			// Make the door
			make_door1(vmanip, doorplace, doordir);
		}else{
			// Don't actually make a door
			doorplace -= doordir;
		}

		// Make a random corridor starting from the door
		v3s16 corridor_end;
		v3s16 corridor_end_dir;
		make_corridor(vmanip, doorplace, doordir, corridor_end, corridor_end_dir, random);

		// Find a place for a random sized room
		roomsize = v3s16(random.range(4,8),random.range(4,6),random.range(4,8));
		walker.setPos(corridor_end);
		walker.setDir(corridor_end_dir);
		r = walker.findPlaceForRoomDoor(roomsize, doorplace, doordir, roomplace);
		if (r == false)
			return;

		if (random.range(0,1)==0) {
			// Make the door
			make_door1(vmanip, doorplace, doordir);
		}else{
			// Don't actually make a door
			roomplace -= doordir;
		}
	}
}

void make_dungeon(BlockMakeData *data, uint32_t blockseed)
{
	v3s16 full_node_min = (data->blockpos-1)*MAP_BLOCKSIZE;
	v3s16 full_node_max = (data->blockpos+2)*MAP_BLOCKSIZE-v3s16(1,1,1);
	ManualMapVoxelManipulator &vmanip = *(data->vmanip);

	// Dungeon generator doesn't modify places which have this set
	data->vmanip->clearFlag(VMANIP_FLAG_DUNGEON_INSIDE | VMANIP_FLAG_DUNGEON_PRESERVE);

	// Set all air and water to be untouchable to make dungeons open
	// to caves and open air
	for (s16 x=full_node_min.X; x<=full_node_max.X; x++)
	for (s16 z=full_node_min.Z; z<=full_node_max.Z; z++) {
		// Node position
		v2s16 p2d(x,z);
		{
			// Use fast index incrementing
			v3s16 em = vmanip.m_area.getExtent();
			u32 i = vmanip.m_area.index(v3s16(p2d.X, full_node_max.Y, p2d.Y));
			for (s16 y=full_node_max.Y; y>=full_node_min.Y; y--) {
				if (vmanip.m_data[i].getContent() == CONTENT_AIR) {
					vmanip.m_flags[i] |= VMANIP_FLAG_DUNGEON_PRESERVE;
				}else if (vmanip.m_data[i].getContent() == CONTENT_WATERSOURCE) {
					vmanip.m_flags[i] |= VMANIP_FLAG_DUNGEON_PRESERVE;
				}
				data->vmanip->m_area.add_y(em, i, -1);
			}
		}
	}

	PseudoRandom random(blockseed+2);

	// Add it
	make_dungeon1(vmanip, random);

	// Convert some cobble to mossy cobble
	for (s16 x=full_node_min.X; x<=full_node_max.X; x++)
	for (s16 z=full_node_min.Z; z<=full_node_max.Z; z++) {
		// Node position
		v2s16 p2d(x,z);
		{
			// Use fast index incrementing
			v3s16 em = vmanip.m_area.getExtent();
			u32 i = vmanip.m_area.index(v3s16(p2d.X, full_node_max.Y, p2d.Y));
			for (s16 y=full_node_max.Y; y>=full_node_min.Y; y--) {
				// (noisebuf not used because it doesn't contain the
				//  full area)
				double wetness = noise3d_param(get_ground_wetness_params(data->seed), x,y,z);
				double d = noise3d_perlin((float)x/2.5, (float)y/2.5,(float)z/2.5, blockseed, 2, 1.4);
				if (vmanip.m_data[i].getContent() == CONTENT_COBBLE && d < wetness/3.0)
					vmanip.m_data[i].setContent(CONTENT_MOSSYCOBBLE);
				data->vmanip->m_area.add_y(em, i, -1);
			}
		}
	}
}

}
