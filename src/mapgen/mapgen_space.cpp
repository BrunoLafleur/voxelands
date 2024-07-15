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

void make_space(BlockMakeData *data)
{
	v3s16 blockpos = data->blockpos;

	ManualMapVoxelManipulator &vmanip = *(data->vmanip);
	// Area of center block
	v3s16 node_min = blockpos*MAP_BLOCKSIZE;
	v3s16 node_max = (blockpos+v3s16(1,1,1))*MAP_BLOCKSIZE-v3s16(1,1,1);
	// Full allocated area
	v3s16 full_node_min = (blockpos-1)*MAP_BLOCKSIZE;
	// Area of a block
	double block_area_nodes = MAP_BLOCKSIZE*MAP_BLOCKSIZE;

	v2s16 p2d_center(node_min.X+MAP_BLOCKSIZE/2, node_min.Z+MAP_BLOCKSIZE/2);
	u32 blockseed = (u32)(data->seed%0x100000000ULL) + full_node_min.Z*38134234 + full_node_min.Y*42123 + full_node_min.X*23;

	for (s16 x=node_min.X; x<=node_max.X; x++)
	for (s16 z=node_min.Z; z<=node_max.Z; z++) {
		// Node position
		v2s16 p2d(x,z);
		{
			// Use fast index incrementing
			v3s16 em = vmanip.m_area.getExtent();
			u32 i = vmanip.m_area.index(v3s16(p2d.X, node_min.Y, p2d.Y));
			for (s16 y=node_min.Y; y<=node_max.Y; y++) {
				vmanip.m_data[i] = MapNode(CONTENT_VACUUM);
				data->vmanip->m_area.add_y(em, i, 1);
			}
		}
	}

	u32 debris_amount = block_area_nodes*debris_amount_2d(blockseed,p2d_center);

	if (debris_amount < 10)
		return;

	NoiseBuffer noisebuf_mineral;
	{
		v3f minpos_f(node_min.X, node_min.Y, node_min.Z);
		v3f maxpos_f(node_max.X, node_max.Y, node_max.Z);


		// Sample length
		v3f sl(2.5, 2.5, 2.5);
		noisebuf_mineral.create(
			get_ground_wetness_params(data->seed),
			minpos_f.X, minpos_f.Y, minpos_f.Z,
			maxpos_f.X, maxpos_f.Y+5, maxpos_f.Z,
			sl.X, sl.Y, sl.Z
		);
	}

	{
		PseudoRandom debrisrandom(blockseed);
		u32 count = debris_amount/6;
		for (u32 i=0; i<count; i++) {
			s16 x = debrisrandom.range(node_min.X, node_max.X);
			s16 y = debrisrandom.range(node_min.Y, node_max.Y);
			s16 z = debrisrandom.range(node_min.Z, node_max.Z);

			MapNode new_content(CONTENT_VACUUM);

			float mineral_noise = noisebuf_mineral.get(x,y,z);

			if (mineral_noise < -0.1) {
				new_content = MapNode(CONTENT_SPACEROCK, MINERAL_IRON);
			}else if (mineral_noise > 0.4) {
				new_content = MapNode(CONTENT_SPACEROCK, MINERAL_SILVER);
			}else if (mineral_noise > 0.3) {
				new_content = MapNode(CONTENT_SPACEROCK, MINERAL_QUARTZ);
			}else if (mineral_noise > 0.2) {
				new_content = MapNode(CONTENT_SPACEROCK, MINERAL_COPPER);
			}else if (mineral_noise > 0.1) {
				new_content = MapNode(CONTENT_SPACEROCK, MINERAL_TIN);
			}else if (mineral_noise > 0.0) {
				new_content = MapNode(CONTENT_SPACEROCK, MINERAL_GOLD);
			}

			u32 vi = vmanip.m_area.index(v3s16(x,y,z));
			if (vmanip.m_data[vi].getContent() == CONTENT_VACUUM)
				vmanip.m_data[vi] = new_content;
		}
	}

	if (debris_amount < 13)
		return;

	{
		v3s16 pos = node_min+(MAP_BLOCKSIZE/2);
		u16 comet_size = debris_amount-11;
		if (comet_size > 5)
			comet_size = 5;
		content_t inner = CONTENT_SPACEROCK;

		float mineral_noise = noisebuf_mineral.get(pos.X,pos.Y,pos.Z);

		if (mineral_noise < -0.5) {
			inner = CONTENT_LAVASOURCE;;
		}else if (mineral_noise < -0.3) {
			inner = CONTENT_ICE;;
		}else if (mineral_noise < -0.1) {
			inner = CONTENT_IRON;;
		}else if (mineral_noise > 0.4) {
			inner = CONTENT_SILVER;;
		}else if (mineral_noise > 0.3) {
			inner = CONTENT_MITHRIL_BLOCK;;
		}else if (mineral_noise > 0.2) {
			inner = CONTENT_COPPER;;
		}else if (mineral_noise > 0.1) {
			inner = CONTENT_TIN;;
		}else if (mineral_noise > 0.0) {
			inner = CONTENT_GOLD;;
		}

		make_boulder(vmanip,pos,comet_size,inner,CONTENT_SPACEROCK,CONTENT_VACUUM);
	}
}

}
