/************************************************************************
* Minetest-c55
* Copyright (C) 2010 celeron55, Perttu Ahola <celeron55@gmail.com>
*
* voxel.h
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

#ifndef VOXEL_HEADER
#define VOXEL_HEADER

#include "common_irrlicht.h"
#include <iostream>
#include "debug.h"
#include "mapnode.h"

// For VC++
#undef min
#undef max

/*
	A fast voxel manipulator class.

	In normal operation, it fetches more map when it is requested.
	It can also be used so that all allowed area is fetched at the
	start, using ManualMapVoxelManipulator.

	Not thread-safe.
*/

/*
	Debug stuff
*/
extern u32 emerge_time;
extern u32 emerge_load_time;

/*
	This class resembles aabbox3d<s16> a lot, but has inclusive
	edges for saner handling of integer sizes
*/
class VoxelArea
{
public:
	// Starts as zero sized
	VoxelArea():
		MinEdge(1,1,1),
		MaxEdge(0,0,0)
	{
	}
	VoxelArea(v3s16 min_edge, v3s16 max_edge):
		MinEdge(min_edge),
		MaxEdge(max_edge)
	{
	}
	VoxelArea(v3s16 p):
		MinEdge(p),
		MaxEdge(p)
	{
	}

	/*
		Modifying methods
	*/

	void addArea(const VoxelArea& a)
	{
		if(getExtent() == v3s16(0,0,0))
		{
			*this = a;
			return;
		}
		if(a.MinEdge.X < MinEdge.X) MinEdge.X = a.MinEdge.X;
		if(a.MinEdge.Y < MinEdge.Y) MinEdge.Y = a.MinEdge.Y;
		if(a.MinEdge.Z < MinEdge.Z) MinEdge.Z = a.MinEdge.Z;
		if(a.MaxEdge.X > MaxEdge.X) MaxEdge.X = a.MaxEdge.X;
		if(a.MaxEdge.Y > MaxEdge.Y) MaxEdge.Y = a.MaxEdge.Y;
		if(a.MaxEdge.Z > MaxEdge.Z) MaxEdge.Z = a.MaxEdge.Z;
	}
	void addPoint(v3s16 p)
	{
		if(getExtent() == v3s16(0,0,0))
		{
			MinEdge = p;
			MaxEdge = p;
			return;
		}
		if(p.X < MinEdge.X) MinEdge.X = p.X;
		if(p.Y < MinEdge.Y) MinEdge.Y = p.Y;
		if(p.Z < MinEdge.Z) MinEdge.Z = p.Z;
		if(p.X > MaxEdge.X) MaxEdge.X = p.X;
		if(p.Y > MaxEdge.Y) MaxEdge.Y = p.Y;
		if(p.Z > MaxEdge.Z) MaxEdge.Z = p.Z;
	}

	// Pad with d nodes
	void pad(v3s16 d)
	{
		MinEdge -= d;
		MaxEdge += d;
	}

	/*void operator+=(v3s16 off)
	{
		MinEdge += off;
		MaxEdge += off;
	}

	void operator-=(v3s16 off)
	{
		MinEdge -= off;
		MaxEdge -= off;
	}*/

	/*
		const methods
	*/

	v3s16 getExtent() const
	{
		return MaxEdge - MinEdge + v3s16(1,1,1);
	}
	s32 getVolume() const
	{
		v3s16 e = getExtent();
		return (s32)e.X * (s32)e.Y * (s32)e.Z;
	}
	bool contains(const VoxelArea &a) const
	{
		// No area contains an empty area
		// NOTE: Algorithms depend on this, so do not change.
		if(a.getExtent() == v3s16(0,0,0))
			return false;

		return(
			a.MinEdge.X >= MinEdge.X && a.MaxEdge.X <= MaxEdge.X &&
			a.MinEdge.Y >= MinEdge.Y && a.MaxEdge.Y <= MaxEdge.Y &&
			a.MinEdge.Z >= MinEdge.Z && a.MaxEdge.Z <= MaxEdge.Z
		);
	}
	bool contains(v3s16 p) const
	{
		return(
			p.X >= MinEdge.X && p.X <= MaxEdge.X &&
			p.Y >= MinEdge.Y && p.Y <= MaxEdge.Y &&
			p.Z >= MinEdge.Z && p.Z <= MaxEdge.Z
		);
	}
	bool contains(s32 i) const
	{
		return (i >= 0 && i < getVolume());
	}
	bool operator==(const VoxelArea &other) const
	{
		return (MinEdge == other.MinEdge
				&& MaxEdge == other.MaxEdge);
	}

	VoxelArea operator+(v3s16 off) const
	{
		return VoxelArea(MinEdge+off, MaxEdge+off);
	}

	VoxelArea operator-(v3s16 off) const
	{
		return VoxelArea(MinEdge-off, MaxEdge-off);
	}

	/*
		Returns 0-6 non-overlapping areas that can be added to
		a to make up this area.

		a: area inside *this
	*/
	void diff(const VoxelArea &a, core::list<VoxelArea> &result)
	{
		/*
			This can result in a maximum of 6 areas
		*/

		// If a is an empty area, return the current area as a whole
		if(a.getExtent() == v3s16(0,0,0))
		{
			VoxelArea b = *this;
			if(b.getVolume() != 0)
				result.push_back(b);
			return;
		}

		assert(contains(a));

		// Take back area, XY inclusive
		{
			v3s16 min(MinEdge.X, MinEdge.Y, a.MaxEdge.Z+1);
			v3s16 max(MaxEdge.X, MaxEdge.Y, MaxEdge.Z);
			VoxelArea b(min, max);
			if(b.getVolume() != 0)
				result.push_back(b);
		}

		// Take front area, XY inclusive
		{
			v3s16 min(MinEdge.X, MinEdge.Y, MinEdge.Z);
			v3s16 max(MaxEdge.X, MaxEdge.Y, a.MinEdge.Z-1);
			VoxelArea b(min, max);
			if(b.getVolume() != 0)
				result.push_back(b);
		}

		// Take top area, X inclusive
		{
			v3s16 min(MinEdge.X, a.MaxEdge.Y+1, a.MinEdge.Z);
			v3s16 max(MaxEdge.X, MaxEdge.Y, a.MaxEdge.Z);
			VoxelArea b(min, max);
			if(b.getVolume() != 0)
				result.push_back(b);
		}

		// Take bottom area, X inclusive
		{
			v3s16 min(MinEdge.X, MinEdge.Y, a.MinEdge.Z);
			v3s16 max(MaxEdge.X, a.MinEdge.Y-1, a.MaxEdge.Z);
			VoxelArea b(min, max);
			if(b.getVolume() != 0)
				result.push_back(b);
		}

		// Take left area, non-inclusive
		{
			v3s16 min(MinEdge.X, a.MinEdge.Y, a.MinEdge.Z);
			v3s16 max(a.MinEdge.X-1, a.MaxEdge.Y, a.MaxEdge.Z);
			VoxelArea b(min, max);
			if(b.getVolume() != 0)
				result.push_back(b);
		}

		// Take right area, non-inclusive
		{
			v3s16 min(a.MaxEdge.X+1, a.MinEdge.Y, a.MinEdge.Z);
			v3s16 max(MaxEdge.X, a.MaxEdge.Y, a.MaxEdge.Z);
			VoxelArea b(min, max);
			if(b.getVolume() != 0)
				result.push_back(b);
		}

	}

	/*
		Translates position from virtual coordinates to array index
	*/
	s32 index(s16 x, s16 y, s16 z) const
	{
		v3s16 em = getExtent();
		v3s16 off = MinEdge;
		s32 i = (s32)(z-off.Z)*em.Y*em.X + (y-off.Y)*em.X + (x-off.X);
		//dstream<<" i("<<x<<","<<y<<","<<z<<")="<<i<<" ";
		return i;
	}
	s32 index(v3s16 p) const
	{
		return index(p.X, p.Y, p.Z);
	}

	// Translate index in the X coordinate
	void add_x(const v3s16 &extent, u32 &i, s16 a)
	{
		i += a;
	}
	// Translate index in the Y coordinate
	void add_y(const v3s16 &extent, u32 &i, s16 a)
	{
		i += a * extent.X;
	}
	// Translate index in the Z coordinate
	void add_z(const v3s16 &extent, u32 &i, s16 a)
	{
		i += a * extent.X*extent.Y;
	}
	// Translate index in space
	void add_p(const v3s16 &extent, u32 &i, v3s16 a)
	{
		i += a.Z*extent.X*extent.Y + a.Y*extent.X + a.X;
	}

	/*
		Print method for debugging
	*/
	void print(std::ostream &o) const
	{
		v3s16 e = getExtent();
		o<<"("<<MinEdge.X
		 <<","<<MinEdge.Y
		 <<","<<MinEdge.Z
		 <<")("<<MaxEdge.X
		 <<","<<MaxEdge.Y
		 <<","<<MaxEdge.Z
		 <<")"
		 <<"="<<e.X<<"x"<<e.Y<<"x"<<e.Z<<"="<<getVolume();
	}

	// Edges are inclusive
	v3s16 MinEdge;
	v3s16 MaxEdge;
};

// Hasn't been copied from source (emerged)
#define VOXELFLAG_NOT_LOADED (1<<0)
// Checked as being inexistent in source
#define VOXELFLAG_INEXISTENT (1<<1)
// Algorithm-dependent
#define VOXELFLAG_CHECKED1 (1<<2)
// Algorithm-dependent
#define VOXELFLAG_CHECKED2 (1<<3)
// Algorithm-dependent
#define VOXELFLAG_CHECKED3 (1<<4)
// Algorithm-dependent
#define VOXELFLAG_CHECKED4 (1<<5)

enum VoxelPrintMode
{
	VOXELPRINT_NOTHING,
	VOXELPRINT_MATERIAL,
	VOXELPRINT_WATERPRESSURE,
};

class Environment;

class VoxelManipulator /*: public NodeContainer*/
{
    public:
	
	VoxelManipulator();
	virtual ~VoxelManipulator();

	/*
		Virtuals from NodeContainer
	*/
	/*virtual u16 nodeContainerId() const
	{
		return NODECONTAINER_ID_VOXELMANIPULATOR;
	}
	bool isValidPosition(v3s16 p)
	{
		emerge(p);
		return !(m_flags[m_area.index(p)] & VOXELFLAG_INEXISTENT);
	}*/

	/*
		These are a bit slow and shouldn't be used internally.
		Use m_data[m_area.index(p)] instead.
	*/
	MapNode getNode(v3s16 p)
	{
		emerge(p);

		if(m_flags[m_area.index(p)] & VOXELFLAG_INEXISTENT)
		{
			dstream<<"EXCEPT: VoxelManipulator::getNode(): "
					<<"p=("<<p.X<<","<<p.Y<<","<<p.Z<<")"
					<<", index="<<m_area.index(p)
					<<", flags="<<(int)m_flags[m_area.index(p)]
					<<" is inexistent"<<std::endl;
			throw InvalidPositionException
			("VoxelManipulator: getNode: inexistent");
		}

		return m_data[m_area.index(p)];
	}
	MapNode getNodeRO(v3s16 p);
	MapNode getNodeNoEx(v3s16 p)
	{
		emerge(p);

		if(m_flags[m_area.index(p)] & VOXELFLAG_INEXISTENT)
		{
			return MapNode(CONTENT_IGNORE);
		}

		return m_data[m_area.index(p)];
	}
	MapNode getNodeNoExNoEmerge(v3s16 p) const
	{
		if(m_area.contains(p) == false)
			return MapNode(CONTENT_IGNORE);
		if(m_flags[m_area.index(p)] & VOXELFLAG_INEXISTENT)
			return MapNode(CONTENT_IGNORE);
		return m_data[m_area.index(p)];
	}
	MapNode & getNodeRef(v3s16 p)
	{
		emerge(p);

		if(m_flags[m_area.index(p)] & VOXELFLAG_INEXISTENT)
		{
			dstream<<"EXCEPT: VoxelManipulator::getNode(): "
					<<"p=("<<p.X<<","<<p.Y<<","<<p.Z<<")"
					<<", index="<<m_area.index(p)
					<<", flags="<<(int)m_flags[m_area.index(p)]
					<<" is inexistent"<<std::endl;
			throw InvalidPositionException
			("VoxelManipulator: getNode: inexistent");
		}

		return m_data[m_area.index(p)];
	}
	void setNode(v3s16 p, MapNode &n)
	{
		emerge(p);

		m_data[m_area.index(p)] = n;
		m_flags[m_area.index(p)] &= ~VOXELFLAG_INEXISTENT;
		m_flags[m_area.index(p)] &= ~VOXELFLAG_NOT_LOADED;
	}
	void setNodeNoRef(v3s16 p, MapNode n)
	{
		setNode(p, n);
	}

	/*void setExists(VoxelArea a)
	{
		emerge(a);
		for(s32 z=a.MinEdge.Z; z<=a.MaxEdge.Z; z++)
		for(s32 y=a.MinEdge.Y; y<=a.MaxEdge.Y; y++)
		for(s32 x=a.MinEdge.X; x<=a.MaxEdge.X; x++)
		{
			m_flags[m_area.index(x,y,z)] &= ~VOXELFLAG_INEXISTENT;
		}
	}*/

	/*MapNode & operator[](v3s16 p)
	{
		//dstream<<"operator[] p=("<<p.X<<","<<p.Y<<","<<p.Z<<")"<<std::endl;
		if(isValidPosition(p) == false)
			emerge(VoxelArea(p));

		return m_data[m_area.index(p)];
	}*/

	/*
		Set stuff if available without an emerge.
		Return false if failed.
		This is convenient but slower than playing around directly
		with the m_data table with indices.
	*/
	bool setNodeNoEmerge(v3s16 p, MapNode n)
	{
		if(m_area.contains(p) == false)
			return false;
		m_data[m_area.index(p)] = n;
		return true;
	}
	bool setNodeNoEmerge(s32 i, MapNode n)
	{
		if(m_area.contains(i) == false)
			return false;
		m_data[i] = n;
		return true;
	}
	/*bool setContentNoEmerge(v3s16 p, u8 c)
	{
		if(isValidPosition(p) == false)
			return false;
		m_data[m_area.index(p)].d = c;
	}*/

	/*
		Control
	*/

	void setEnv(Environment* const env) {
		m_env = env;
	}
			
	virtual void clear();

	void print(std::ostream &o, VoxelPrintMode mode=VOXELPRINT_MATERIAL);

	void addArea(const VoxelArea area);

	/*
		Copy data and set flags to 0
		dst_area.getExtent() <= src_area.getExtent()
	*/
	void copyFrom(const MapNode* const src,const VoxelArea src_area,
			const v3s16 from_pos,const v3s16 to_pos,const v3s16 size);

	// Copy data
	void copyTo(MapNode* const dst,const VoxelArea dst_area,
			const v3s16 dst_pos,const v3s16 from_pos,const v3s16 size);

	/*
		Algorithms
	*/

	void clearFlag(u8 flag);

	void unspreadLight(enum LightBank bank, v3s16 p, u8 oldlight,
			core::map<v3s16, bool> & light_sources);
	void unspreadLight(enum LightBank bank,
			core::map<v3s16, u8> & from_nodes,
			core::map<v3s16, bool> & light_sources);

	void spreadLight(enum LightBank bank, v3s16 p);
	void spreadLight(enum LightBank bank,
			core::map<v3s16, bool> & from_nodes);

	/*
		Virtual functions
	*/

	/*
		Get the contents of the requested area from somewhere.
		Shall touch only nodes that have VOXELFLAG_NOT_LOADED
		Shall reset VOXELFLAG_NOT_LOADED

		If not found from source, add with VOXELFLAG_INEXISTENT
	*/
	virtual void emerge(VoxelArea a, s32 caller_id=-1)
	{
		//dstream<<"emerge p=("<<p.X<<","<<p.Y<<","<<p.Z<<")"<<std::endl;
		addArea(a);
	}

    /********************************************
		Member variables
    *********************************************/
    public:
	
	/*
		The area that is stored in m_data.
		addInternalBox should not be used if getExtent() == v3s16(0,0,0)
		MaxEdge is 1 higher than maximum allowed position
	*/
	VoxelArea m_area;

	/*
		NULL if data size is 0 (extent (0,0,0))
		Data is stored as [z*h*w + y*h + x]
	*/
	MapNode *m_data;

	/*
		Flags of all nodes
	*/

	u8 *m_flags;

    private:
	
	Environment *m_env;

	//TODO: Use these or remove them
	//TODO: Would these make any speed improvement?
	//bool m_pressure_route_valid;
	//v3s16 m_pressure_route_surface;

	/*
		Some settings
	*/
	//bool m_disable_water_climb;
};

#endif

