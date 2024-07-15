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

namespace mapgen
{

void make_boulder(ManualMapVoxelManipulator &vmanip, v3s16 pos, uint16_t size, content_t inner, content_t outer, content_t replace)
{
	int edge;
	int16_t min = -size;
	int16_t max = size;

	for (s16 x=min; x<=max; x++) {
	for (s16 y=min; y<=max; y++) {
	for (s16 z=min; z<=max; z++) {
		edge = 0;
		if (x == min || x == max)
			edge++;
		if (y == min || y == max)
			edge++;
		if (z == min || z == max)
			edge++;

		if (edge > 1)
			continue;
		u32 vi = vmanip.m_area.index(pos+v3s16(x,y,z));
		if (replace != CONTENT_IGNORE && vmanip.m_data[vi].getContent() != replace)
			continue;
		if (edge) {
			vmanip.m_data[vi] = MapNode(outer);
			continue;
		}
		vmanip.m_data[vi] = MapNode(inner);
	}
	}
	}
}

}
