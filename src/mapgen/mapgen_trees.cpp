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

void make_tree(ManualMapVoxelManipulator &vmanip, v3s16 p0)
{
	MapNode treenode(CONTENT_TREE);
	MapNode leavesnode(CONTENT_LEAVES);
	uint8_t b = 0xE0;

	s16 trunk_h = myrand_range(5,6);
	v3s16 p1 = p0;
	for (s16 ii=0; ii<trunk_h; ii++) {
		treenode.param1 = b|ii;
		if (vmanip.m_area.contains(p1))
			vmanip.m_data[vmanip.m_area.index(p1)] = treenode;
		b = 0;
		p1.Y++;
	}

	// p1 is now the last piece of the trunk
	p1.Y -= 1;

	VoxelArea leaves_a(v3s16(-3,-1,-3), v3s16(3,2,3));
	Buffer<u8> leaves_d(leaves_a.getVolume());
	for (s32 i=0; i<leaves_a.getVolume(); i++) {
		leaves_d[i] = 0;
	}

	// Force leaves at near the end of the trunk
	s16 rad = 3;
	for (s16 y=-1; y<=1; y++) {
	for (s16 z=-rad; z<=rad; z++) {
	for (s16 x=-rad; x<=rad; x++) {
		if (rad < 3 || (z > -rad && z < rad) || (x > -rad && x < rad))
			leaves_d[leaves_a.index(v3s16(x,y,z))] = 1;
	}
	}
		rad--;
	}

	// Add leaves randomly
	for (u32 iii=0; iii<7; iii++) {
		s16 d = 1;

		v3s16 p(
			myrand_range(leaves_a.MinEdge.X, leaves_a.MaxEdge.X-d),
			myrand_range(leaves_a.MinEdge.Y, leaves_a.MaxEdge.Y-d),
			myrand_range(leaves_a.MinEdge.Z, leaves_a.MaxEdge.Z-d)
		);

		for (s16 z=0; z<=d; z++) {
		for (s16 y=0; y<=d; y++) {
		for (s16 x=0; x<=d; x++) {
			leaves_d[leaves_a.index(p+v3s16(x,y,z))] = 1;
		}
		}
		}
	}

	// Blit leaves to vmanip
	for (s16 z=leaves_a.MinEdge.Z; z<=leaves_a.MaxEdge.Z; z++) {
	for (s16 y=leaves_a.MinEdge.Y; y<=leaves_a.MaxEdge.Y; y++) {
	for (s16 x=leaves_a.MinEdge.X; x<=leaves_a.MaxEdge.X; x++) {
		v3s16 p(x,y,z);
		p += p1;
		if (vmanip.m_area.contains(p) == false)
			continue;
		u32 vi = vmanip.m_area.index(p);
		if (vmanip.m_data[vi].getContent() != CONTENT_AIR
				&& vmanip.m_data[vi].getContent() != CONTENT_IGNORE)
			continue;
		u32 i = leaves_a.index(x,y,z);
		if (leaves_d[i] == 1)
			vmanip.m_data[vi] = leavesnode;
	}
	}
	}
}

void make_appletree(ManualMapVoxelManipulator &vmanip, v3s16 p0)
{
	MapNode treenode(CONTENT_APPLE_TREE);
	MapNode leavesnode(CONTENT_APPLE_LEAVES);
	MapNode applenode(CONTENT_APPLE);
	uint8_t b = 0xE0;

	s16 trunk_h = myrand_range(4, 5);
	v3s16 p1 = p0;
	for (s16 ii=0; ii<trunk_h; ii++) {
		treenode.param1 = b|ii;
		if(vmanip.m_area.contains(p1))
			vmanip.m_data[vmanip.m_area.index(p1)] = treenode;
		b = 0;
		p1.Y++;
	}

	// p1 is now the last piece of the trunk
	p1.Y -= 1;

	VoxelArea leaves_a(v3s16(-2,-1,-2), v3s16(2,2,2));
	Buffer<u8> leaves_d(leaves_a.getVolume());
	for(s32 i=0; i<leaves_a.getVolume(); i++)
		leaves_d[i] = 0;

	// Force leaves at near the end of the trunk
	{
		s16 d = 1;
		for(s16 z=-d; z<=d; z++)
		for(s16 y=-d; y<=d; y++)
		for(s16 x=-d; x<=d; x++)
		{
			leaves_d[leaves_a.index(v3s16(x,y,z))] = 1;
		}
	}

	// Add leaves randomly
	for(u32 iii=0; iii<7; iii++)
	{
		s16 d = 1;

		v3s16 p(
			myrand_range(leaves_a.MinEdge.X, leaves_a.MaxEdge.X-d),
			myrand_range(leaves_a.MinEdge.Y, leaves_a.MaxEdge.Y-d),
			myrand_range(leaves_a.MinEdge.Z, leaves_a.MaxEdge.Z-d)
		);

		for(s16 z=0; z<=d; z++)
		for(s16 y=0; y<=d; y++)
		for(s16 x=0; x<=d; x++)
		{
			leaves_d[leaves_a.index(p+v3s16(x,y,z))] = 1;
		}
	}

	// not all apple trees have apples
	bool have_fruit = (myrand_range(0,4) == 0);

	// Blit leaves to vmanip
	for(s16 z=leaves_a.MinEdge.Z; z<=leaves_a.MaxEdge.Z; z++)
	for(s16 y=leaves_a.MinEdge.Y; y<=leaves_a.MaxEdge.Y; y++)
	for(s16 x=leaves_a.MinEdge.X; x<=leaves_a.MaxEdge.X; x++)
	{
		v3s16 p(x,y,z);
		p += p1;
		if (vmanip.m_area.contains(p) == false)
			continue;
		u32 vi = vmanip.m_area.index(p);
		if (vmanip.m_data[vi].getContent() != CONTENT_AIR && vmanip.m_data[vi].getContent() != CONTENT_IGNORE)
			continue;
		u32 i = leaves_a.index(x,y,z);
		if (leaves_d[i] == 1) {
			bool is_apple = myrand_range(0,99) < 10;
			if (have_fruit && is_apple) {
				vmanip.m_data[vi] = applenode;
			}else{
				vmanip.m_data[vi] = leavesnode;
			}
		}
	}
}

void make_conifertree(ManualMapVoxelManipulator &vmanip, v3s16 p0)
{
	MapNode treenode(CONTENT_CONIFER_TREE);
	MapNode leavesnode(CONTENT_CONIFER_LEAVES);
	uint8_t b = 0xE0;

	s16 trunk_h = myrand_range(8, 11);
	v3s16 p1 = p0;
	for (s16 ii=0; ii<trunk_h; ii++) {
		treenode.param1 = b|ii;
		if(vmanip.m_area.contains(p1))
			vmanip.m_data[vmanip.m_area.index(p1)] = treenode;
		b = 0;
		p1.Y++;
	}

	// p1 is now the last piece of the trunk
	if (vmanip.m_area.contains(p1+v3s16(0,1,0)))
		vmanip.m_data[vmanip.m_area.index(p1+v3s16(0,1,0))] = leavesnode;
	for(s16 z=-1; z<=1; z++)
	for(s16 y=-2; y<=0; y++)
	for(s16 x=-1; x<=1; x++)
	{
		if (!x && !z)
			continue;
		if (vmanip.m_area.contains(p1+v3s16(x,y,z)))
			vmanip.m_data[vmanip.m_area.index(p1+v3s16(x,y,z))] = leavesnode;
	}
	for(s16 z=-2; z<=2; z++)
	for(s16 y=-5; y<-2; y++)
	for(s16 x=-2; x<=2; x++)
	{
		if (!x && !z)
			continue;
		if (vmanip.m_area.contains(p1+v3s16(x,y,z)))
			vmanip.m_data[vmanip.m_area.index(p1+v3s16(x,y,z))] = leavesnode;
	}

}

void make_largetree(ManualMapVoxelManipulator &vmanip, v3s16 p0)
{
	MapNode treenode(CONTENT_TREE);
	MapNode leavesnode(CONTENT_LEAVES);
	uint8_t b = 0xE0;

	s16 trunk_h = myrand_range(10, 12);
	v3s16 p1 = p0;
	for (s16 ii=0; ii<trunk_h; ii++) {
		treenode.param1 = b|ii;
		if(vmanip.m_area.contains(p1))
			vmanip.m_data[vmanip.m_area.index(p1)] = treenode;
		b = 0;
		p1.Y++;
	}

	// p1 is now the last piece of the trunk
	p1.Y -= 1;

	VoxelArea leaves_a(v3s16(-3,-3,-3), v3s16(3,3,3));
	Buffer<u8> leaves_d(leaves_a.getVolume());
	for(s32 i=0; i<leaves_a.getVolume(); i++)
		leaves_d[i] = 0;

	for (s16 k=0; k<5; k++) {
		if (k == 1) {
			p1.Y -= 3;
			b = k<<5;
			for (s16 ki=0; ki<4; ki++) {
				p1.X++;
				treenode.param1 = b|((trunk_h-3)+ki);
				vmanip.m_data[vmanip.m_area.index(p1)] = treenode;
			}
			p1.X--;
		}else if (k == 2) {
			p1.X -= 3;
			b = k<<5;
			for (s16 ki=0; ki<4; ki++) {
				p1.X--;
				treenode.param1 = b|((trunk_h-3)+ki);
				vmanip.m_data[vmanip.m_area.index(p1)] = treenode;
			}
			p1.X++;
		}else if (k == 3) {
			p1.X += 3;
			b = k<<5;
			for (s16 ki=0; ki<4; ki++) {
				p1.Z++;
				treenode.param1 = b|((trunk_h-3)+ki);
				vmanip.m_data[vmanip.m_area.index(p1)] = treenode;
			}
			p1.Z--;
		}else if (k == 4) {
			p1.Z -= 3;
			b = k<<5;
			for (s16 ki=0; ki<4; ki++) {
				p1.Z--;
				treenode.param1 = b|((trunk_h-3)+ki);
				vmanip.m_data[vmanip.m_area.index(p1)] = treenode;
			}
			p1.Z++;
		}else{
			treenode.param1 = trunk_h;
			vmanip.m_data[vmanip.m_area.index(p1)] = treenode;
		}
		// Force leaves at near the end of the trunk
		{
			s16 d = 1;
			for(s16 z=-d; z<=d; z++)
			for(s16 y=-d; y<=d; y++)
			for(s16 x=-d; x<=d; x++)
			{
				leaves_d[leaves_a.index(v3s16(x,y,z))] = 1;
			}
		}

		if (k) {
			// Add leaves randomly
			for (u32 iii=0; iii<10; iii++) {
				s16 d = 1;

				v3s16 p(
					myrand_range(leaves_a.MinEdge.X, leaves_a.MaxEdge.X-d),
					myrand_range(leaves_a.MinEdge.Y, leaves_a.MaxEdge.Y-d),
					myrand_range(leaves_a.MinEdge.Z, leaves_a.MaxEdge.Z-d)
				);

				for(s16 z=0; z<=d; z++)
				for(s16 y=0; y<=d; y++)
				for(s16 x=0; x<=d; x++)
				{
					leaves_d[leaves_a.index(p+v3s16(x,y,z))] = 1;
				}
			}
		}

		// Blit leaves to vmanip
		for(s16 z=leaves_a.MinEdge.Z; z<=leaves_a.MaxEdge.Z; z++)
		for(s16 y=leaves_a.MinEdge.Y; y<=leaves_a.MaxEdge.Y; y++)
		for(s16 x=leaves_a.MinEdge.X; x<=leaves_a.MaxEdge.X; x++)
		{
			v3s16 p(x,y,z);
			p += p1;
			if(vmanip.m_area.contains(p) == false)
				continue;
			u32 vi = vmanip.m_area.index(p);
			if(vmanip.m_data[vi].getContent() != CONTENT_AIR
					&& vmanip.m_data[vi].getContent() != CONTENT_IGNORE)
				continue;
			u32 i = leaves_a.index(x,y,z);
			if(leaves_d[i] == 1)
				vmanip.m_data[vi] = leavesnode;
		}
	}
}

void make_jungletree(ManualMapVoxelManipulator &vmanip, v3s16 p0)
{
	MapNode treenode(CONTENT_JUNGLETREE);
	MapNode leavesnode(CONTENT_JUNGLELEAVES);

	for(s16 x=-1; x<=1; x++)
	for(s16 z=-1; z<=1; z++)
	{
		if(myrand_range(0, 2) == 0)
			continue;
		v3s16 p1 = p0 + v3s16(x,0,z);
		v3s16 p2 = p0 + v3s16(x,-1,z);
		if(vmanip.m_area.contains(p2)
				&& vmanip.m_data[vmanip.m_area.index(p2)] == CONTENT_AIR)
			vmanip.m_data[vmanip.m_area.index(p2)] = treenode;
		else if(vmanip.m_area.contains(p1))
			vmanip.m_data[vmanip.m_area.index(p1)] = treenode;
	}

	s16 trunk_h = myrand_range(8, 12);
	v3s16 p1 = p0;
	for(s16 ii=0; ii<trunk_h; ii++)
	{
		if(vmanip.m_area.contains(p1))
			vmanip.m_data[vmanip.m_area.index(p1)] = treenode;
		p1.Y++;
	}

	// p1 is now the last piece of the trunk
	p1.Y -= 1;

	VoxelArea leaves_a(v3s16(-3,-2,-3), v3s16(3,2,3));
	//SharedPtr<u8> leaves_d(new u8[leaves_a.getVolume()]);
	Buffer<u8> leaves_d(leaves_a.getVolume());
	for(s32 i=0; i<leaves_a.getVolume(); i++)
		leaves_d[i] = 0;

	// Force leaves at near the end of the trunk
	{
		s16 d = 1;
		for(s16 z=-d; z<=d; z++)
		for(s16 y=-d; y<=d; y++)
		for(s16 x=-d; x<=d; x++)
		{
			leaves_d[leaves_a.index(v3s16(x,y,z))] = 1;
		}
	}

	// Add leaves randomly
	for(u32 iii=0; iii<30; iii++)
	{
		s16 d = 1;

		v3s16 p(
			myrand_range(leaves_a.MinEdge.X, leaves_a.MaxEdge.X-d),
			myrand_range(leaves_a.MinEdge.Y, leaves_a.MaxEdge.Y-d),
			myrand_range(leaves_a.MinEdge.Z, leaves_a.MaxEdge.Z-d)
		);

		for(s16 z=0; z<=d; z++)
		for(s16 y=0; y<=d; y++)
		for(s16 x=0; x<=d; x++)
		{
			leaves_d[leaves_a.index(p+v3s16(x,y,z))] = 1;
		}
	}

	// Blit leaves to vmanip
	for(s16 z=leaves_a.MinEdge.Z; z<=leaves_a.MaxEdge.Z; z++)
	for(s16 y=leaves_a.MinEdge.Y; y<=leaves_a.MaxEdge.Y; y++)
	for(s16 x=leaves_a.MinEdge.X; x<=leaves_a.MaxEdge.X; x++)
	{
		v3s16 p(x,y,z);
		p += p1;
		if(vmanip.m_area.contains(p) == false)
			continue;
		u32 vi = vmanip.m_area.index(p);
		if(vmanip.m_data[vi].getContent() != CONTENT_AIR
				&& vmanip.m_data[vi].getContent() != CONTENT_IGNORE)
			continue;
		u32 i = leaves_a.index(x,y,z);
		if(leaves_d[i] == 1)
			vmanip.m_data[vi] = leavesnode;
	}
}

}
