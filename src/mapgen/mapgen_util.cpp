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
#include "mapblock.h"
#include "noise.h"

namespace mapgen
{

/*
	Noise functions. Make sure seed is mangled differently in each one.
*/

NoiseParams get_cave_noise1_params(uint64_t seed)
{
	return NoiseParams(NOISE_PERLIN_CONTOUR, seed+52534, 4, 0.5,
			50, CAVE_NOISE_SCALE);
}

NoiseParams get_cave_noise2_params(uint64_t seed)
{
	return NoiseParams(NOISE_PERLIN_CONTOUR_FLIP_YZ, seed+10325, 4, 0.5,
			50, CAVE_NOISE_SCALE);
}

NoiseParams get_ground_noise1_params(uint64_t seed)
{
	return NoiseParams(NOISE_PERLIN, seed+983240, 4,
			0.55, 80.0, 40.0);
}

NoiseParams get_ground_crumbleness_params(uint64_t seed)
{
	return NoiseParams(NOISE_PERLIN, seed+34413, 3,
			1.3, 20.0, 1.0);
}

NoiseParams get_ground_wetness_params(uint64_t seed)
{
	return NoiseParams(NOISE_PERLIN, seed+32474, 4,
			1.1, 40.0, 1.0);
}

float get_humidity(uint64_t seed, v2s16 p)
{
	double noise = noise2d_perlin((float)p.X/500.0, (float)p.Y/500.0, seed+72384, 4, 0.5);
	noise = (noise + 1.0)/2.0;
	if (noise < 0.0)
		return 0.0;
	if (noise > 1.0)
		return 1.0;
	return noise;
}

int16_t get_ground_height(uint64_t seed, v2s16 p)
{
	double e = noise2d_perlin((float)p.X/200.0, (float)p.Y/200.0, seed, 4, 0.5);

	if (e > 0.0)
		e = pow(e,1.9);

	return (WATER_LEVEL+1)+(25.0*e);
}

bool is_cave(uint64_t seed, v3s16 p)
{
	double d1 = noise3d_param(get_cave_noise1_params(seed), p.X,p.Y,p.Z);
	double d2 = noise3d_param(get_cave_noise2_params(seed), p.X,p.Y,p.Z);
	return d1*d2 > CAVE_NOISE_THRESHOLD;
}
// Amount of trees per area in nodes
uint32_t get_tree_density(BlockMakeData *data, v2s16 p)
{

	double zeroval = -0.39;
	double density = 0.0;
	double noise = 0.0;
	uint32_t r = 0;

	noise = noise2d_perlin(
			0.5+(float)p.X/125,
			0.5+(float)p.Y/125,
			data->seed+2,
			4,
			0.66
		);

	if (noise >= zeroval) {
		density = 0.04 * (noise-zeroval) / (1.0-zeroval);
		r = density*(double)(MAP_BLOCKSIZE*MAP_BLOCKSIZE);
	}

	if (data->biome == BIOME_JUNGLE || data->biome == BIOME_FOREST) {
		if (r < 1) {
			r = 20;
		}else{
			r *= 5;
		}
	}else if (data->biome == BIOME_LAKE || data->biome == BIOME_SNOWCAP || data->biome == BIOME_WOODLANDS) {
		if (r < 1)
			r = 5;
	}else if (data->biome == BIOME_PLAINS) {
		if (r)
			r /= 5;
	}

	return r;
}
// Amount of grasses per area in nodes
uint32_t get_grass_density(BlockMakeData *data, v2s16 p)
{
	double zeroval = -0.39;
	double density = 0.0;
	double noise = 0.0;
	uint32_t r = 0;

	if (data->biome == BIOME_DESERT || data->biome == BIOME_SNOWCAP || data->biome == BIOME_WASTELANDS)
		return 0;

	noise = noise2d_perlin(
			0.5+(float)p.X/125,
			0.5+(float)p.Y/125,
			data->seed+21335,
			4,
			0.66
		);

	if (noise >= zeroval) {
		density = 0.04 * (noise-zeroval) / (1.0-zeroval);
		r = density*(double)(MAP_BLOCKSIZE*MAP_BLOCKSIZE);
	}

	if (data->biome == BIOME_JUNGLE || data->biome == BIOME_PLAINS) {
		if (r < 1) {
			r = 50;
		}else{
			r *= 15;
		}
	}else if (data->biome == BIOME_LAKE || data->biome == BIOME_WOODLANDS) {
		if (r < 1)
			r = 5;
	}

	return r*3;
}

uint32_t get_boulder_density(BlockMakeData *data, v2s16 p)
{
	double zeroval = 0.3;
	double density = 0.0;
	double factor = 500.0;
	double noise = 0.0;
	uint32_t r = 0;

	if (data->biome == BIOME_DESERT || data->biome == BIOME_SNOWCAP || data->biome == BIOME_OCEAN || data->biome == BIOME_BEACH)
		return 0;
	if (data->biome == BIOME_WASTELANDS)
		factor = 200.0;
	if (data->biome == BIOME_WOODLANDS)
		factor = 250.0;

	noise = noise2d_perlin(
			0.5+(float)p.X/factor,
			0.5+(float)p.Y/factor,
			data->seed+14143242,
			4,
			0.66
		);

	if (noise >= zeroval) {
		density = 0.005 * (noise-zeroval) / (1.0-zeroval);
		r = density*(double)(MAP_BLOCKSIZE*MAP_BLOCKSIZE);
	}

	return !!r;
}

// used in space
double debris_amount_2d(uint64_t seed, v2s16 p)
{
	double noise = noise2d_perlin(
		0.5+(float)p.X/125,
		0.5+(float)p.Y/125,
		seed+2,
		4,
		0.7
	);
	double zeroval = -0.41;
	if (noise < zeroval)
		return 0;

	return 0.037 * (noise-zeroval) / (1.0-zeroval);
}

/*
	Incrementally find ground level from 3d noise
*/
s16 find_ground_level_from_noise(BlockMakeData *data, v2s16 p2d, s16 precision)
{
	if (data->type == MGT_FLAT)
		return 2;
	return get_ground_height(data->seed,p2d);
}

double get_sector_average_ground_level(BlockMakeData *data, v2s16 sectorpos)
{
	v2s16 node_min = sectorpos*MAP_BLOCKSIZE;
	v2s16 node_max = (sectorpos+v2s16(1,1))*MAP_BLOCKSIZE-v2s16(1,1);
	double a = 0;
	a += get_ground_height(data->seed, v2s16(node_min.X, node_min.Y));
	a += get_ground_height(data->seed, v2s16(node_min.X, node_max.Y));
	a += get_ground_height(data->seed, v2s16(node_max.X, node_max.Y));
	a += get_ground_height(data->seed, v2s16(node_max.X, node_min.Y));
	a += get_ground_height(data->seed, v2s16(node_min.X+MAP_BLOCKSIZE/2, node_min.Y+MAP_BLOCKSIZE/2));
	a /= 5.0;
	return a;
}

double get_sector_maximum_ground_level(BlockMakeData *data, v2s16 sectorpos)
{
	v2s16 node_min = sectorpos*MAP_BLOCKSIZE;
	v2s16 node_max = (sectorpos+v2s16(1,1))*MAP_BLOCKSIZE-v2s16(1,1);
	double a = -31000;
	// Corners
	a = MYMAX(a, get_ground_height(data->seed, v2s16(node_min.X, node_min.Y)));
	a = MYMAX(a, get_ground_height(data->seed, v2s16(node_min.X, node_max.Y)));
	a = MYMAX(a, get_ground_height(data->seed, v2s16(node_max.X, node_max.Y)));
	a = MYMAX(a, get_ground_height(data->seed, v2s16(node_min.X, node_min.Y)));
	// Center
	a = MYMAX(a, get_ground_height(data->seed, v2s16(node_min.X+MAP_BLOCKSIZE/2, node_min.Y+MAP_BLOCKSIZE/2)));
	// Side middle points
	a = MYMAX(a, get_ground_height(data->seed, v2s16(node_min.X+MAP_BLOCKSIZE/2, node_min.Y)));
	a = MYMAX(a, get_ground_height(data->seed, v2s16(node_min.X+MAP_BLOCKSIZE/2, node_max.Y)));
	a = MYMAX(a, get_ground_height(data->seed, v2s16(node_min.X, node_min.Y+MAP_BLOCKSIZE/2)));
	a = MYMAX(a, get_ground_height(data->seed, v2s16(node_max.X, node_min.Y+MAP_BLOCKSIZE/2)));
	return a;
}

double get_sector_minimum_ground_level(BlockMakeData *data, v2s16 sectorpos)
{
	v2s16 node_min = sectorpos*MAP_BLOCKSIZE;
	v2s16 node_max = (sectorpos+v2s16(1,1))*MAP_BLOCKSIZE-v2s16(1,1);
	double a = 31000;
	// Corners
	a = MYMIN(a, get_ground_height(data->seed, v2s16(node_min.X, node_min.Y)));
	a = MYMIN(a, get_ground_height(data->seed, v2s16(node_min.X, node_max.Y)));
	a = MYMIN(a, get_ground_height(data->seed, v2s16(node_max.X, node_max.Y)));
	a = MYMIN(a, get_ground_height(data->seed, v2s16(node_min.X, node_min.Y)));
	// Center
	a = MYMIN(a, get_ground_height(data->seed, v2s16(node_min.X+MAP_BLOCKSIZE/2, node_min.Y+MAP_BLOCKSIZE/2)));
	// Side middle points
	a = MYMIN(a, get_ground_height(data->seed, v2s16(node_min.X+MAP_BLOCKSIZE/2, node_min.Y)));
	a = MYMIN(a, get_ground_height(data->seed, v2s16(node_min.X+MAP_BLOCKSIZE/2, node_max.Y)));
	a = MYMIN(a, get_ground_height(data->seed, v2s16(node_min.X, node_min.Y+MAP_BLOCKSIZE/2)));
	a = MYMIN(a, get_ground_height(data->seed, v2s16(node_max.X, node_min.Y+MAP_BLOCKSIZE/2)));
	return a;
}

bool block_is_underground(BlockMakeData *data, v3s16 blockpos)
{
	s16 minimum_groundlevel = (s16)get_sector_minimum_ground_level(data, v2s16(blockpos.X, blockpos.Z));

	if(blockpos.Y*MAP_BLOCKSIZE + MAP_BLOCKSIZE <= minimum_groundlevel)
		return true;
	else
		return false;
}

bool get_have_sand(uint64_t seed, v2s16 p2d)
{
	// Determine whether to have sand here
	double sandnoise = noise2d_perlin(
			0.5+(float)p2d.X/500, 0.5+(float)p2d.Y/500,
			seed+59420, 3, 0.50);

	return (sandnoise > -0.15);
}

uint8_t get_block_biome(BlockMakeData *data, v3s16 blockpos)
{
	v3s16 relpos = blockpos - data->blockpos*MAP_BLOCKSIZE;
	uint16_t xv = 0;
	uint16_t zv = 0;
	uint16_t v;
	uint8_t xb = SURBIOME_X_PLUS;
	uint8_t zb = SURBIOME_Z_PLUS;

	if (
		data->surrounding_biomes[SURBIOME_X_MINUS] == data->biome
		&& data->surrounding_biomes[SURBIOME_X_PLUS] == data->biome
		&& data->surrounding_biomes[SURBIOME_Z_MINUS] == data->biome
		&& data->surrounding_biomes[SURBIOME_Z_PLUS] == data->biome
	)
		return data->biome;

	if (relpos.X > 11) {
		if (relpos.Z > 11) {
			xv = (relpos.X-11);
			zv = (relpos.Z-11);
		}else if (relpos.Z < 5) {
			zb = SURBIOME_Z_MINUS;
			xv = (relpos.X-11);
			zv = 3-relpos.Z;
		}
	}else if (relpos.X < 5) {
		xb = SURBIOME_X_MINUS;
		if (relpos.Z > 11) {
			xv = 3-relpos.X;
			zv = (relpos.Z-11);
		}else if (relpos.Z < 5) {
			zb = SURBIOME_Z_MINUS;
			xv = 3-relpos.X;
			zv = 3-relpos.Z;
		}
	}

	v = xv+zv;
	if (v > 4) {
		if (xv > zv) {
			return data->surrounding_biomes[xb];
		}else{
			return data->surrounding_biomes[zb];
		}
	}

	return data->biome;
}

uint8_t get_chunk_biome(uint64_t seed, v3s16 blockpos)
{
	v3s16 node_min = blockpos*MAP_BLOCKSIZE;
	v3s16 node_max = (blockpos+v3s16(1,1,1))*MAP_BLOCKSIZE-v3s16(1,1,1);
	v2s16 p2d_center(node_min.X+MAP_BLOCKSIZE/2, node_min.Z+MAP_BLOCKSIZE/2);
	v2s16 p2d(blockpos.X, blockpos.Z);
	int16_t average_ground_height;
	float surface_humidity = 0;
	BlockMakeData data;

	if (node_min.Y >= 1024) {
		return BIOME_SPACE;
	}else if (node_min.Y >= 256) {
		return BIOME_SKY;
	}else if (node_max.Y <= -128) {
		return BIOME_THEDEEP;
	}

	data.seed = seed;
	data.blockpos = blockpos;

	average_ground_height = (int16_t)get_sector_average_ground_level(&data,p2d);

	if (average_ground_height <= -10) {
		return BIOME_OCEAN;
	}
	if (average_ground_height >= 40) {
		return BIOME_SNOWCAP;
	}

	surface_humidity = get_humidity(seed, p2d_center);

	if (average_ground_height <= 2) {
		if (surface_humidity < 0.5) {
			return BIOME_BEACH;
		}
		return BIOME_LAKE;
	}

	if (average_ground_height > 30) {
		if (surface_humidity < 0.25) {
			return BIOME_WOODLANDS;
		}
		if (surface_humidity < 0.5) {
			return BIOME_FOREST;
		}
		return BIOME_JUNGLE;
	}

	if (average_ground_height > 10) {
		if (surface_humidity < 0.05) {
			return BIOME_WASTELANDS;
		}
		if (surface_humidity < 0.25) {
			return BIOME_DESERT;
		}
		if (surface_humidity < 0.5) {
			return BIOME_WOODLANDS;
		}
		if (surface_humidity < 0.75) {
			return BIOME_FOREST;
		}
		return BIOME_JUNGLE;
	}

	if (surface_humidity < 0.25) {
		return BIOME_PLAINS;
	}
	if (surface_humidity < 0.75) {
		return BIOME_WOODLANDS;
	}

	return BIOME_FOREST;
}

void calc_biome(BlockMakeData *data)
{
	data->biome = get_chunk_biome(data->seed,data->blockpos);

	data->surrounding_biomes[SURBIOME_X_MINUS] = get_chunk_biome(data->seed,data->blockpos+v3s16(-1,0,0));
	data->surrounding_biomes[SURBIOME_X_PLUS] = get_chunk_biome(data->seed,data->blockpos+v3s16(1,0,0));
	data->surrounding_biomes[SURBIOME_Z_MINUS] = get_chunk_biome(data->seed,data->blockpos+v3s16(0,0,-1));
	data->surrounding_biomes[SURBIOME_Z_PLUS] = get_chunk_biome(data->seed,data->blockpos+v3s16(0,0,1));
	data->surrounding_biomes[SURBIOME_XZ_MINUS] = get_chunk_biome(data->seed,data->blockpos+v3s16(-1,0,-1));
	data->surrounding_biomes[SURBIOME_XZ_PLUS] = get_chunk_biome(data->seed,data->blockpos+v3s16(1,0,1));
	data->surrounding_biomes[SURBIOME_X_MINUS_Z] = get_chunk_biome(data->seed,data->blockpos+v3s16(-1,0,1));
	data->surrounding_biomes[SURBIOME_Z_MINUS_X] = get_chunk_biome(data->seed,data->blockpos+v3s16(1,0,-1));
}

}
