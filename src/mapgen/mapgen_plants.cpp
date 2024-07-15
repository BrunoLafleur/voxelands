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

void make_papyrus(VoxelManipulator &vmanip, v3s16 p0)
{
	MapNode papyrusnode(CONTENT_PAPYRUS);

	s16 trunk_h = myrand_range(2, 3);
	v3s16 p1 = p0;
	for (s16 ii=0; ii<trunk_h; ii++) {
		if (vmanip.m_area.contains(p1))
			vmanip.m_data[vmanip.m_area.index(p1)] = papyrusnode;
		p1.Y++;
	}
}

void make_cactus(VoxelManipulator &vmanip, v3s16 p0)
{
	MapNode cactusnode(CONTENT_CACTUS);

	s16 trunk_h = 3;
	if (myrand()%5000 == 0)
		trunk_h = 4;
	v3s16 p1 = p0;
	for (s16 ii=0; ii<trunk_h; ii++) {
		if (vmanip.m_area.contains(p1))
			vmanip.m_data[vmanip.m_area.index(p1)] = cactusnode;
		p1.Y++;
	}
}

}
