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
#include "noise.h"
#include "mapblock.h"
#include "map.h"
#include "mineral.h"

namespace mapgen
{

void make_flat(BlockMakeData *data)
{
	v3s16 blockpos = data->blockpos;

	ManualMapVoxelManipulator &vmanip = *(data->vmanip);
	// Area of center block
	v3s16 node_min = blockpos*MAP_BLOCKSIZE;
	v3s16 node_max = (blockpos+v3s16(1,1,1))*MAP_BLOCKSIZE-v3s16(1,1,1);

	v2s16 p2d_center(node_min.X+MAP_BLOCKSIZE/2, node_min.Z+MAP_BLOCKSIZE/2);

	data->biome = BIOME_WOODLANDS;

	for (s16 x=node_min.X; x<=node_max.X; x++)
	for (s16 z=node_min.Z; z<=node_max.Z; z++) {
		// Node position
		v2s16 p2d(x,z);
		{
			// Use fast index incrementing
			v3s16 em = vmanip.m_area.getExtent();
			u32 i = vmanip.m_area.index(v3s16(p2d.X, node_min.Y, p2d.Y));
			for (s16 y=node_min.Y; y<=node_max.Y; y++) {
				if (y > 2) {
					vmanip.m_data[i] = MapNode(CONTENT_AIR);
				}else if (y == 2) {
					vmanip.m_data[i] = MapNode(CONTENT_MUD,0x01);
				}else{
					vmanip.m_data[i] = MapNode(CONTENT_MUD);
				}
				data->vmanip->m_area.add_y(em, i, 1);
			}
		}
	}
}

}
