/************************************************************************
* Minetest-c55
* Copyright (C) 2010-2011 celeron55, Perttu Ahola <celeron55@gmail.com>
*
* mapgen.h
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

#ifndef MAPGEN_HEADER
#define MAPGEN_HEADER

#include "common_irrlicht.h"
#include "utility.h" // UniqueQueue
#include "mapnode.h"

class MapBlock;
class ManualMapVoxelManipulator;
class VoxelManipulator;
struct NoiseParams;

enum MapGenType {
	MGT_FLAT = 0,
	MGT_DEFAULT
};

#define SURBIOME_X_MINUS 0
#define SURBIOME_X_PLUS 1
#define SURBIOME_Z_MINUS 2
#define SURBIOME_Z_PLUS 3
#define SURBIOME_XZ_MINUS 4
#define SURBIOME_XZ_PLUS 5
#define SURBIOME_X_MINUS_Z 6
#define SURBIOME_Z_MINUS_X 7


#define VMANIP_FLAG_DUNGEON_INSIDE VOXELFLAG_CHECKED1
#define VMANIP_FLAG_DUNGEON_PRESERVE VOXELFLAG_CHECKED2
#define VMANIP_FLAG_DUNGEON_UNTOUCHABLE (VMANIP_FLAG_DUNGEON_INSIDE|VMANIP_FLAG_DUNGEON_PRESERVE)

/*
	Scaling the output of the noise function affects the overdrive of the
	contour function, which affects the shape of the output considerably.
*/
#define CAVE_NOISE_SCALE 12.0
#define CAVE_NOISE_THRESHOLD (1.5/CAVE_NOISE_SCALE)

namespace mapgen
{
	struct BlockMakeData
	{
		bool no_op;
		ManualMapVoxelManipulator *vmanip;
		uint64_t seed;
		MapGenType type;
		uint8_t biome;
		uint8_t surrounding_biomes[8];
		v3s16 blockpos;
		UniqueQueue<v3s16> transforming_liquid;

		BlockMakeData();
		~BlockMakeData();
	};

	// Finds precise ground level at any position
	s16 find_ground_level_from_noise(BlockMakeData *data, v2s16 p2d, s16 precision);

	// Find out if block is completely underground
	bool block_is_underground(BlockMakeData *data, v3s16 blockpos);

	// Main map generation routine
	void make_block(BlockMakeData *data);

	/* defined in mapgen_plants.cpp */
	void make_papyrus(VoxelManipulator &vmanip, v3s16 p0);
	void make_cactus(VoxelManipulator &vmanip, v3s16 p0);

	/* defined in mapgen_trees.cpp */
	void make_tree(ManualMapVoxelManipulator &vmanip, v3s16 p0);
	void make_appletree(ManualMapVoxelManipulator &vmanip, v3s16 p0);
	void make_conifertree(ManualMapVoxelManipulator &vmanip, v3s16 p0);
	void make_largetree(ManualMapVoxelManipulator &vmanip, v3s16 p0);
	void make_jungletree(ManualMapVoxelManipulator &vmanip, v3s16 p0);

	/* defined in mapgen_rocks.cpp */
	void make_boulder(ManualMapVoxelManipulator &vmanip, v3s16 pos, uint16_t size, content_t inner, content_t outer, content_t replace);

	/* defined in mapgen_dungeon.cpp */
	void make_dungeon(BlockMakeData *data, uint32_t blockseed);

	/* defined in mapgen_util.cpp */
	NoiseParams get_cave_noise1_params(uint64_t seed);
	NoiseParams get_cave_noise2_params(uint64_t seed);
	NoiseParams get_ground_noise1_params(uint64_t seed);
	NoiseParams get_ground_crumbleness_params(uint64_t seed);
	NoiseParams get_ground_wetness_params(uint64_t seed);
	float get_humidity(uint64_t seed, v2s16 p);
	int16_t get_ground_height(uint64_t seed, v2s16 p);
	uint32_t get_tree_density(BlockMakeData *data, v2s16 p);
	uint32_t get_grass_density(BlockMakeData *data, v2s16 p);
	uint32_t get_boulder_density(BlockMakeData *data, v2s16 p);

	bool is_cave(uint64_t seed, v3s16 p);
	double debris_amount_2d(uint64_t seed, v2s16 p);
	s16 find_ground_level_from_noise(BlockMakeData *data, v2s16 p2d, s16 precision);
	double get_sector_average_ground_level(BlockMakeData *data, v2s16 sectorpos);
	double get_sector_maximum_ground_level(BlockMakeData *data, v2s16 sectorpos);
	double get_sector_minimum_ground_level(BlockMakeData *data, v2s16 sectorpos);
	bool block_is_underground(BlockMakeData *data, v3s16 blockpos);
	bool get_have_sand(uint64_t seed, v2s16 p2d);
	uint8_t get_block_biome(BlockMakeData *data, v3s16 blockpos);
	void calc_biome(BlockMakeData *data);

	/* defined in mapgen_space.cpp */
	void make_space(BlockMakeData *data);

	/* defined in mapgen_sky.cpp */
	void make_sky(BlockMakeData *data);

	/* defined in mapgen_thedeep.cpp */
	void make_thedeep(BlockMakeData *data);

	/* defined in mapgen_flat.cpp */
	void make_flat(BlockMakeData *data);

}; // namespace mapgen

#endif
