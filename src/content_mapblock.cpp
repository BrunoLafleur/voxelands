/************************************************************************
* Minetest-c55
* Copyright (C) 2010-2011 celeron55, Perttu Ahola <celeron55@gmail.com>
*
* content_mapblock.cpp
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

#include "content_mapblock.h"
#include "content_mapnode.h"
#include "main.h" // For g_settings and g_texturesource
#include "mineral.h"
#include "mapblock_mesh.h" // For MapBlock_LightColor()
#include "environment.h"
#include "nodemetadata.h"
#include "content_nodemeta.h"
#include "sound.h"

#ifndef SERVER

// For all MSVC versions before Visual Studio 2013
#if defined(_MSC_VER) && _MSC_VER < 1800
	float roundf(float x)
	{
		return float((x < 0) ? (ceil((x) - 0.5)) : (floor((x) + 0.5)));
	}
#endif

static const v3s16 corners[8] = {
	v3s16(-1, 1, 1),
	v3s16( 1, 1, 1),
	v3s16( 1, 1,-1),
	v3s16(-1, 1,-1),
	v3s16(-1,-1,-1),
	v3s16( 1,-1,-1),
	v3s16( 1,-1, 1),
	v3s16(-1,-1, 1)
};
static u8 smooth_lights[8];

#if 0
static void meshgen_fullbright_lights(std::vector<u32> &colours, u8 alpha, u16 count)
{
	u32 c = 0x0000000F;
	if (alpha != 255) {
		c |= 0x01<<24;
		c |= alpha<<8;
	}
	for (u16 i=0; i<count; i++) {
		colours.push_back(c);
	}
}
#endif

static void meshgen_custom_lights(std::vector<u32> &colours, u8 alpha, u8 red, u8 green, u8 blue, u16 count)
{
	if (alpha < 2)
		alpha = 3;
	u32 c = (alpha<<24)|(red<<16)|(green<<8)|blue;
	for (u16 i=0; i<count; i++) {
		colours.push_back(c);
	}
}

static void meshgen_selected_lights(std::vector<u32> &colours, u8 alpha, u16 count)
{
	if (alpha < 2)
		alpha = 3;
	u32 c = (alpha<<24)|(128<<16)|(128<<8)|255;
	for (u16 i=0; i<count; i++) {
		colours.push_back(c);
	}
}

static u8 meshgen_interpolate_lights(u8 l1, u8 l2, float v, s16 bias)
{
	if (l1 == l2)
		return l1;
	v /= (float)BS;
	if (v > -0.4 && v < 0.4 && bias) {
		if (bias > 0) {
			if (l1 > l2) {
				v -= 0.1;
			}else if (l2 > l1) {
				v += 0.1;
			}
		}else{
			if (l1 > l2) {
				v += 0.1;
			}else if (l2 > l1) {
				v -= 0.1;
			}
		}
	}
	if (v < -0.49)
		return l1;
	if (v > 0.49)
		return l2;
	float f1 = l1;
	float f2 = l2;

	float f = 0.0-(v-0.5);
	f1 *= f;
	f = v+0.5;
	f2 *= f;

	float l = f1+f2;
	if (l >= LIGHT_SUN)
		return LIGHT_SUN;
	if (bias > 0)
		return ceilf(l);
	if (bias < 0)
		return floorf(l);
	return roundf(l);
}

/*
 * what this should do:
 * interpolate to the requested vertex position
 */
static void meshgen_lights_vertex(
	MeshMakeData *data,
	MapNode &n,
	v3s16 p,
	std::vector<u32> &colours,
	u8 alpha,
	v3s16 face,
	video::S3DVertex &vertex
)
{
	u32 dl = 0;
	u32 nl = 0;
	if (face.X > 0) {
		if (face.Y > 0) {
			if (face.Z > 0) {
				// x+ y+ z+ light
				dl = smooth_lights[1]&0x0F;
				nl = (smooth_lights[1]>>4)&0x0F;
			}else if (face.Z < 0) {
				// x+ y+ z- light
				dl = smooth_lights[2]&0x0F;
				nl = (smooth_lights[2]>>4)&0x0F;
			}else{
				// x+ y+ interpolate z light
				dl = meshgen_interpolate_lights(smooth_lights[2]&0x0F,smooth_lights[1]&0x0F,vertex.Pos.Z,face.Y);
				nl = meshgen_interpolate_lights(smooth_lights[2]>>4,smooth_lights[1]>>4,vertex.Pos.Z,face.Y);
			}
		}else if (face.Y < 0) {
			if (face.Z > 0) {
				// x+ y- z+ light
				dl = smooth_lights[6]&0x0F;
				nl = (smooth_lights[6]>>4)&0x0F;
			}else if (face.Z < 0) {
				// x+ y- z- light
				dl = smooth_lights[5]&0x0F;
				nl = (smooth_lights[5]>>4)&0x0F;
			}else{
				// x+ y- interpolate z light
				dl = meshgen_interpolate_lights(smooth_lights[5]&0x0F,smooth_lights[6]&0x0F,vertex.Pos.Z,face.Y);
				nl = meshgen_interpolate_lights(smooth_lights[5]>>4,smooth_lights[6]>>4,vertex.Pos.Z,face.Y);
			}
		}else{
			if (face.Z > 0) {
				// x+ z+ interpolate y light
				dl = meshgen_interpolate_lights(smooth_lights[6]&0x0F,smooth_lights[1]&0x0F,vertex.Pos.Y,face.Y);
				nl = meshgen_interpolate_lights(smooth_lights[6]>>4,smooth_lights[1]>>4,vertex.Pos.Y,face.Y);
			}else if (face.Z < 0) {
				// x+ z- interpolate y light
				dl = meshgen_interpolate_lights(smooth_lights[5]&0x0F,smooth_lights[2]&0x0F,vertex.Pos.Y,face.Y);
				nl = meshgen_interpolate_lights(smooth_lights[5]>>4,smooth_lights[2]>>4,vertex.Pos.Y,face.Y);
			}else{
				// x+ interpolate y z light
				u8 dl1 = meshgen_interpolate_lights(smooth_lights[6]&0x0F,smooth_lights[1]&0x0F,vertex.Pos.Y,face.Y);
				u8 dl2 = meshgen_interpolate_lights(smooth_lights[5]&0x0F,smooth_lights[2]&0x0F,vertex.Pos.Y,face.Y);
				u8 nl1 = meshgen_interpolate_lights(smooth_lights[6]>>4,smooth_lights[1]>>4,vertex.Pos.Y,face.Y);
				u8 nl2 = meshgen_interpolate_lights(smooth_lights[5]>>4,smooth_lights[2]>>4,vertex.Pos.Y,face.Y);
				dl = meshgen_interpolate_lights(dl2,dl1,vertex.Pos.Z,face.Y);
				nl = meshgen_interpolate_lights(nl2,nl1,vertex.Pos.Z,face.Y);
			}
		}
	}else if (face.X < 0) {
		if (face.Y > 0) {
			if (face.Z > 0) {
				// x- y+ z+ light
				dl = smooth_lights[0]&0x0F;
				nl = (smooth_lights[0]>>4)&0x0F;
			}else if (face.Z < 0) {
				// x- y+ z- light
				dl = smooth_lights[3]&0x0F;
				nl = (smooth_lights[3]>>4)&0x0F;
			}else{
				// x- y+ interpolate z light
				dl = meshgen_interpolate_lights(smooth_lights[3]&0x0F,smooth_lights[0]&0x0F,vertex.Pos.Z,face.Y);
				nl = meshgen_interpolate_lights(smooth_lights[3]>>4,smooth_lights[0]>>4,vertex.Pos.Z,face.Y);
			}
		}else if (face.Y < 0) {
			if (face.Z > 0) {
				// x- y- z+ light
				dl = smooth_lights[7]&0x0F;
				nl = (smooth_lights[7]>>4)&0x0F;
			}else if (face.Z < 0) {
				// x- y- z- light
				dl = smooth_lights[4]&0x0F;
				nl = (smooth_lights[4]>>4)&0x0F;
			}else{
				// x- y- interpolate z light
				dl = meshgen_interpolate_lights(smooth_lights[4]&0x0F,smooth_lights[7]&0x0F,vertex.Pos.Z,face.Y);
				nl = meshgen_interpolate_lights(smooth_lights[4]>>4,smooth_lights[7]>>4,vertex.Pos.Z,face.Y);
			}
		}else{
			if (face.Z > 0) {
				// x- z+ interpolate y light
				dl = meshgen_interpolate_lights(smooth_lights[7]&0x0F,smooth_lights[0]&0x0F,vertex.Pos.Y,face.Y);
				nl = meshgen_interpolate_lights(smooth_lights[7]>>4,smooth_lights[0]>>4,vertex.Pos.Y,face.Y);
			}else if (face.Z < 0) {
				// x- z- interpolate y light
				dl = meshgen_interpolate_lights(smooth_lights[4]&0x0F,smooth_lights[3]&0x0F,vertex.Pos.Y,face.Y);
				nl = meshgen_interpolate_lights(smooth_lights[4]>>4,smooth_lights[3]>>4,vertex.Pos.Y,face.Y);
			}else{
				// x- interpolate y z light
				u8 dl1 = meshgen_interpolate_lights(smooth_lights[7]&0x0F,smooth_lights[0]&0x0F,vertex.Pos.Y,face.Y);
				u8 dl2 = meshgen_interpolate_lights(smooth_lights[4]&0x0F,smooth_lights[3]&0x0F,vertex.Pos.Y,face.Y);
				u8 nl1 = meshgen_interpolate_lights(smooth_lights[7]>>4,smooth_lights[0]>>4,vertex.Pos.Y,face.Y);
				u8 nl2 = meshgen_interpolate_lights(smooth_lights[4]>>4,smooth_lights[3]>>4,vertex.Pos.Y,face.Y);
				dl = meshgen_interpolate_lights(dl2,dl1,vertex.Pos.Z,face.Y);
				nl = meshgen_interpolate_lights(nl2,nl1,vertex.Pos.Z,face.Y);
			}
		}
	}else{
		if (face.Y > 0) {
			if (face.Z > 0) {
				// y+ z+ interpolate x light
				dl = meshgen_interpolate_lights(smooth_lights[0]&0x0F,smooth_lights[1]&0x0F,vertex.Pos.X,face.Y);
				nl = meshgen_interpolate_lights(smooth_lights[0]>>4,smooth_lights[1]>>4,vertex.Pos.X,face.Y);
			}else if (face.Z < 0) {
				// y+ z- interpolate x light
				dl = meshgen_interpolate_lights(smooth_lights[3]&0x0F,smooth_lights[2]&0x0F,vertex.Pos.X,face.Y);
				nl = meshgen_interpolate_lights(smooth_lights[3]>>4,smooth_lights[2]>>4,vertex.Pos.X,face.Y);
			}else{
				// y+ interpolate x z light
				u8 dl1 = meshgen_interpolate_lights(smooth_lights[0]&0x0F,smooth_lights[1]&0x0F,vertex.Pos.X,face.Y);
				u8 dl2 = meshgen_interpolate_lights(smooth_lights[3]&0x0F,smooth_lights[2]&0x0F,vertex.Pos.X,face.Y);
				u8 nl1 = meshgen_interpolate_lights(smooth_lights[0]>>4,smooth_lights[1]>>4,vertex.Pos.X,face.Y);
				u8 nl2 = meshgen_interpolate_lights(smooth_lights[3]>>4,smooth_lights[2]>>4,vertex.Pos.X,face.Y);
				dl = meshgen_interpolate_lights(dl2,dl1,vertex.Pos.Z,face.Y);
				nl = meshgen_interpolate_lights(nl2,nl1,vertex.Pos.Z,face.Y);
			}
		}else if (face.Y < 0) {
			if (face.Z > 0) {
				// y- z+ interpolate x light
				dl = meshgen_interpolate_lights(smooth_lights[7]&0x0F,smooth_lights[6]&0x0F,vertex.Pos.X,face.Y);
				nl = meshgen_interpolate_lights(smooth_lights[7]>>4,smooth_lights[6]>>4,vertex.Pos.X,face.Y);
			}else if (face.Z < 0) {
				// y- z- interpolate x light
				dl = meshgen_interpolate_lights(smooth_lights[4]&0x0F,smooth_lights[5]&0x0F,vertex.Pos.X,face.Y);
				nl = meshgen_interpolate_lights(smooth_lights[4]>>4,smooth_lights[5]>>4,vertex.Pos.X,face.Y);
			}else{
				// y- interpolate x z light
				u8 dl1 = meshgen_interpolate_lights(smooth_lights[7]&0x0F,smooth_lights[6]&0x0F,vertex.Pos.X,face.Y);
				u8 dl2 = meshgen_interpolate_lights(smooth_lights[4]&0x0F,smooth_lights[5]&0x0F,vertex.Pos.X,face.Y);
				u8 nl1 = meshgen_interpolate_lights(smooth_lights[7]>>4,smooth_lights[6]>>4,vertex.Pos.X,face.Y);
				u8 nl2 = meshgen_interpolate_lights(smooth_lights[4]>>4,smooth_lights[5]>>4,vertex.Pos.X,face.Y);
				dl = meshgen_interpolate_lights(dl2,dl1,vertex.Pos.Z,face.Y);
				nl = meshgen_interpolate_lights(nl2,nl1,vertex.Pos.Z,face.Y);
			}
		}else{
			if (face.Z > 0) {
				// z+ interpolate x y light
				u8 dl1 = meshgen_interpolate_lights(smooth_lights[7]&0x0F,smooth_lights[6]&0x0F,vertex.Pos.X,face.Y);
				u8 dl2 = meshgen_interpolate_lights(smooth_lights[0]&0x0F,smooth_lights[1]&0x0F,vertex.Pos.X,face.Y);
				u8 nl1 = meshgen_interpolate_lights(smooth_lights[7]>>4,smooth_lights[6]>>4,vertex.Pos.X,face.Y);
				u8 nl2 = meshgen_interpolate_lights(smooth_lights[0]>>4,smooth_lights[1]>>4,vertex.Pos.X,face.Y);
				dl = meshgen_interpolate_lights(dl1,dl2,vertex.Pos.Y,face.Y);
				nl = meshgen_interpolate_lights(nl1,nl2,vertex.Pos.Y,face.Y);
			}else if (face.Z < 0) {
				// z- interpolate x y light
				u8 dl1 = meshgen_interpolate_lights(smooth_lights[4]&0x0F,smooth_lights[5]&0x0F,vertex.Pos.X,face.Y);
				u8 dl2 = meshgen_interpolate_lights(smooth_lights[3]&0x0F,smooth_lights[2]&0x0F,vertex.Pos.X,face.Y);
				u8 nl1 = meshgen_interpolate_lights(smooth_lights[4]>>4,smooth_lights[5]>>4,vertex.Pos.X,face.Y);
				u8 nl2 = meshgen_interpolate_lights(smooth_lights[3]>>4,smooth_lights[2]>>4,vertex.Pos.X,face.Y);
				dl = meshgen_interpolate_lights(dl1,dl2,vertex.Pos.Y,face.Y);
				nl = meshgen_interpolate_lights(nl1,nl2,vertex.Pos.Y,face.Y);
			}else{
				// interpolate x y z light
				// z+ interpolate x y
				u8 dl1 = meshgen_interpolate_lights(smooth_lights[7]&0x0F,smooth_lights[6]&0x0F,vertex.Pos.X,face.Y);
				u8 dl2 = meshgen_interpolate_lights(smooth_lights[0]&0x0F,smooth_lights[1]&0x0F,vertex.Pos.X,face.Y);
				u8 nl1 = meshgen_interpolate_lights(smooth_lights[7]>>4,smooth_lights[6]>>4,vertex.Pos.X,face.Y);
				u8 nl2 = meshgen_interpolate_lights(smooth_lights[0]>>4,smooth_lights[1]>>4,vertex.Pos.X,face.Y);
				dl1 = meshgen_interpolate_lights(dl1,dl2,vertex.Pos.Y,face.Y);
				nl2 = meshgen_interpolate_lights(nl1,nl2,vertex.Pos.Y,face.Y);
				// z- interpolate x y
				u8 dl3 = meshgen_interpolate_lights(smooth_lights[4]&0x0F,smooth_lights[5]&0x0F,vertex.Pos.X,face.Y);
				u8 dl4 = meshgen_interpolate_lights(smooth_lights[3]&0x0F,smooth_lights[2]&0x0F,vertex.Pos.X,face.Y);
				u8 nl3 = meshgen_interpolate_lights(smooth_lights[4]>>4,smooth_lights[5]>>4,vertex.Pos.X,face.Y);
				u8 nl4 = meshgen_interpolate_lights(smooth_lights[3]>>4,smooth_lights[2]>>4,vertex.Pos.X,face.Y);
				dl2 = meshgen_interpolate_lights(dl3,dl4,vertex.Pos.Y,face.Y);
				nl2 = meshgen_interpolate_lights(nl3,nl4,vertex.Pos.Y,face.Y);
				// x y interpolate z
				dl = meshgen_interpolate_lights(dl2,dl1,vertex.Pos.Z,face.Y);
				nl = meshgen_interpolate_lights(nl2,nl1,vertex.Pos.Z,face.Y);
			}
		}
	}
	u32 c = ((nl&0x0F)<<4)|(dl&0x0F);
	if (alpha != 255) {
		c |= 0x01<<24;
		c |= alpha<<8;
	}
	colours.push_back(c);
}

/*
 * what this should do:
 * return face lighting
 */
static void meshgen_lights_face(
	MeshMakeData *data,
	MapNode &n,
	v3s16 p,
	std::vector<u32> &colours,
	u8 alpha,
	v3s16 face,
	u16 count,
	video::S3DVertex *vertexes
)
{
	MapNode n1 = data->m_vmanip.getNodeRO(data->m_blockpos_nodes+p+face);
	u8 light = face_light(n, n1, face);
	if ((face.X && face.Y) || (face.X && face.Z) || (face.Y && face.Z)) {
		u8 l;
		u32 dl = light&0x0F;
		u32 nl = (light&0xF0)>>4;
		u16 nc = 1;
		if (face.X) {
			n1 = data->m_vmanip.getNodeRO(data->m_blockpos_nodes+p+v3s16(face.X,0,0));
			l = face_light(n, n1, face);
			dl += (l&0x0F);
			nl += (l>>4)&0x0F;
			nc++;
		}
		if (face.Y) {
			n1 = data->m_vmanip.getNodeRO(data->m_blockpos_nodes+p+v3s16(0,face.Y,0));
			l = face_light(n, n1, face);
			dl += (l&0x0F);
			nl += (l>>4)&0x0F;
			nc++;
		}
		if (face.Z) {
			n1 = data->m_vmanip.getNodeRO(data->m_blockpos_nodes+p+v3s16(0,0,face.Z));
			l = face_light(n, n1, face);
			dl += (l&0x0F);
			nl += (l>>4)&0x0F;
			nc++;
		}
		if (nc > 1) {
			dl /= nc;
			nl /= nc;
		}
		light = (nl<<4)|dl;
	}
	u32 c = light;
	if (alpha != 255) {
		c |= 0x01<<24;
		c |= alpha<<8;
	}
	for (u16 i=0; i<count; i++) {
		colours.push_back(c);
	}
}

/*
 * what this should do:
 * MeshMakeData has a light_detail value in it, don't check config for every vertex!
 * for each vertex:
 *	call meshgen_lights_vertex
 */
static void meshgen_lights(
	MeshMakeData *data,
	MapNode &n,
	v3s16 p,
	std::vector<u32> &colours,
	u8 alpha,
	v3s16 face,
	u16 count,
	video::S3DVertex *vertexes
)
{
	if (data->light_detail > 1) {
		for (u16 i=0; i<count; i++) {
			meshgen_lights_vertex(data,n,p,colours,alpha,face,vertexes[i]);
		}
	}else{
		meshgen_lights_face(data,n,p,colours,alpha,face,count,vertexes);
	}
}

/* TODO: there may be other cases that should return false */
static bool meshgen_hardface(MeshMakeData *data, v3s16 p, MapNode &n, v3s16 pos)
{
	MapNode nn = data->m_vmanip.getNodeRO(data->m_blockpos_nodes+p+pos);
	ContentFeatures *ff = &content_features(nn.getContent());
	if (ff->draw_type == CDT_CUBELIKE || ff->draw_type == CDT_DIRTLIKE)
		return false;
	v3s16 ipos(0,0,0);
	if (pos.X)
		ipos.X = -pos.X;
	if (pos.Y)
		ipos.Y = -pos.Y;
	if (pos.Z)
		ipos.Z = -pos.Z;
	ContentFeatures *f = &content_features(n.getContent());
	if (ff->draw_type == CDT_STAIRLIKE) {
		bool urot = (nn.getContent() >= CONTENT_SLAB_STAIR_UD_MIN && nn.getContent() <= CONTENT_SLAB_STAIR_UD_MAX);
		if ((urot && pos.Y == -1) || (!urot && pos.Y == 1))
			return false;
		if (unpackDir(nn.param1) == ipos)
			return false;
		bool furot = (n.getContent() >= CONTENT_SLAB_STAIR_UD_MIN && n.getContent() <= CONTENT_SLAB_STAIR_UD_MAX);
		if (f->draw_type == CDT_STAIRLIKE && furot == urot && nn.param1 == n.param1)
			return false;
		if (f->draw_type == CDT_SLABLIKE && furot == urot && unpackDir(nn.param1) == pos)
			return false;
	}
	if (ff->draw_type == CDT_SLABLIKE && f->draw_type == CDT_SLABLIKE) {
		bool urot = (nn.getContent() >= CONTENT_SLAB_STAIR_UD_MIN && nn.getContent() <= CONTENT_SLAB_STAIR_UD_MAX);
		if ((urot && pos.Y == 1) || (!urot && pos.Y == -1))
			return false;
		if (!pos.Y && n.getContent() == nn.getContent())
			return false;
	}
	if (ff->draw_type == CDT_MELONLIKE) {
		if (ff->param2_type != CPT_PLANTGROWTH || nn.param2 == 0)
			return false;
	}
	return true;
}

static bool meshgen_farface(MeshMakeData *data, v3s16 p, MapNode &n, v3s16 pos)
{
	MapNode nn = data->m_vmanip.getNodeRO(data->m_blockpos_nodes+p+pos);
	ContentFeatures *ff = &content_features(nn.getContent());
	if (content_features(n).draw_type != CDT_LIQUID_SOURCE && ff->draw_type == CDT_LIQUID_SOURCE)
		return true;
	if (content_features(n).draw_type == CDT_LIQUID_SOURCE && nn.getContent() == CONTENT_IGNORE)
		return false;
	if (ff->draw_type == CDT_CUBELIKE)
		return false;
	if (ff->draw_type == CDT_TRUNKLIKE)
		return false;
	if (ff->draw_type == CDT_LEAFLIKE)
		return false;
	if (ff->draw_type == CDT_ROOFLIKE)
		return false;
	if (ff->draw_type == CDT_WALLLIKE)
		return false;
	if (ff->draw_type == CDT_LIQUID_SOURCE)
		return false;
	return true;
}

static int meshgen_check_walllike(MeshMakeData *data, MapNode n, v3s16 p, u8 d[8])
{
	static const v3s16 fence_dirs[8] = {
		v3s16(1,0,0),
		v3s16(-1,0,0),
		v3s16(0,0,1),
		v3s16(0,0,-1),
		v3s16(1,0,1),
		v3s16(1,0,-1),
		v3s16(-1,0,1),
		v3s16(-1,0,-1)
	};
	static const int showcheck[4][2] = {
		{0,2},
		{0,3},
		{1,2},
		{1,3}
	};
	v3s16 p2;
	MapNode n2;
	content_t c2;
	const ContentFeatures *f2;
	for (s16 i=0; i<8; i++) {
		d[i] = 0;
	}
	for (int k=0; k<8; k++) {
		if (k > 3 && (d[showcheck[k-4][0]] || d[showcheck[k-4][1]]))
					continue;
		p2 = p+fence_dirs[k];
		n2 = data->m_vmanip.getNodeRO(p2);
		c2 = n2.getContent();
		f2 = &content_features(c2);
		if (
			f2->draw_type == CDT_FENCELIKE
			|| f2->draw_type == CDT_WALLLIKE
			|| c2 == CONTENT_WOOD_GATE
			|| c2 == CONTENT_WOOD_GATE_OPEN
			|| c2 == CONTENT_IRON_GATE
			|| c2 == CONTENT_IRON_GATE_OPEN
			|| (
				c2 != CONTENT_IGNORE
				&& c2 == content_features(n).special_alternate_node
			)
		) {
			d[k] = 1;
		}
	}
	u8 ps = d[0]+d[1]+d[2]+d[3]+d[4]+d[5]+d[6]+d[7];
	p2 = p;
	p2.Y++;
	n2 = data->m_vmanip.getNodeRO(p2);
	c2 = n2.getContent();
	f2 = &content_features(c2);
	if (
		f2->draw_type != CDT_WALLLIKE
		&& f2->draw_type != CDT_AIRLIKE
	) {
		if (
			f2->draw_type == CDT_TORCHLIKE
			|| f2->draw_type == CDT_FENCELIKE
		)
			return 0;
		return 1;
	}
	if (f2->draw_type == CDT_WALLLIKE) {
		u8 ad[8];
		int ap = meshgen_check_walllike(data,n2,p2,ad);
		if ((ad[0]+ad[1]+ad[2]+ad[3]+ad[4]+ad[5]+ad[6]+ad[7]) == 2) {
			if (ap != 2)
				return 1;
		}else{
			return 1;
		}
	}
	if (ps == 2) {
		if (
			d[4]
			|| d[5]
			|| d[6]
			|| d[7]
			|| (d[0] && d[2])
			|| (d[1] && d[3])
			|| (d[0] && d[3])
			|| (d[1] && d[2])
		) {
			return 0;
		}
	}else{
		return 0;
	}
	return 2;
}

static int meshgen_check_plantlike_water(MeshMakeData *data, MapNode n, v3s16 p, bool *ignore)
{
	int level = 0;
	int ignore_count = 0;
	bool would_ignore;
	bool my_ignore[4] = {
		false,
		false,
		false,
		false
	};
	v3s16 around[4] = {
		v3s16(-1,0,0),
		v3s16(1,0,0),
		v3s16(0,0,-1),
		v3s16(0,0,1)
	};
	MapNode nn;
	ContentFeatures *f = &content_features(n.getContent());
	ContentFeatures *ff;
	if (f->draw_type != CDT_PLANTLIKE && f->draw_type != CDT_PLANTLIKE_FERN)
		return 0;

	if (!ignore)
		ignore = my_ignore;

	for (int i=0; i<4; i++) {
		if (ignore[i]) {
			level++;
			ignore_count++;
			continue;
		}
		nn = data->m_vmanip.getNodeRO(data->m_blockpos_nodes+p+around[i]);
		would_ignore = ignore[i];
		ignore[i] = true;
		ff = &content_features(nn.getContent());
		if (
			nn.getContent() == CONTENT_WATERSOURCE
			|| ff->draw_type == CDT_DIRTLIKE
			|| ff->draw_type == CDT_CUBELIKE
			|| (
				(
					ff->draw_type == CDT_PLANTLIKE
					|| ff->draw_type == CDT_PLANTLIKE_FERN
				) && meshgen_check_plantlike_water(data,nn,p+around[i],ignore) > 0
			)
		) {
			level++;
		}
		ignore[i] = would_ignore;
	}

	if (level != 4 || ignore_count == 4)
		return 0;

	nn = data->m_vmanip.getNodeRO(data->m_blockpos_nodes+p+v3s16(0,1,0));
	ff = &content_features(nn.getContent());
	if (
		nn.getContent() == CONTENT_WATERSOURCE
		|| (
			(
				ff->draw_type == CDT_PLANTLIKE
				|| ff->draw_type == CDT_PLANTLIKE_FERN
			) && meshgen_check_plantlike_water(data,nn,p+v3s16(0,1,0),NULL) > 0
		)
	) {
		return 2;
	}

	return 1;
}

static void meshgen_cuboid(
	MeshMakeData *data,
	MapNode &n,
	v3s16 p,
	v3f pos,
	const aabb3f &box,
	TileSpec *tiles,
	int tilecount,
	SelectedNode &selected,
	const f32* txc,
	v3s16 angle,
	v3f centre,
	u8 *cols=NULL
)
{
	if (tilecount < 1 || tilecount > 6)
		return;

	v3f min = box.MinEdge;
	v3f max = box.MaxEdge;

	if (txc == NULL) {
		static const f32 txc_default[24] = {
			0,0,1,1,
			0,0,1,1,
			0,0,1,1,
			0,0,1,1,
			0,0,1,1,
			0,0,1,1
		};
		txc = txc_default;
	}

	static v3s16 faces[6] = {
		v3s16( 0, 1, 0),
		v3s16( 0,-1, 0),
		v3s16( 1, 0, 0),
		v3s16(-1, 0, 0),
		v3s16( 0, 0, 1),
		v3s16( 0, 0,-1)
	};

	video::S3DVertex vertices[6][4] = {
		{	// up
			video::S3DVertex(min.X,max.Y,max.Z, 0,0,0, video::SColor(255,255,255,255), txc[0],txc[1]),
			video::S3DVertex(max.X,max.Y,max.Z, 0,0,0, video::SColor(255,255,255,255), txc[2],txc[1]),
			video::S3DVertex(max.X,max.Y,min.Z, 0,0,0, video::SColor(255,255,255,255), txc[2],txc[3]),
			video::S3DVertex(min.X,max.Y,min.Z, 0,0,0, video::SColor(255,255,255,255), txc[0],txc[3])
		},{	// down
			video::S3DVertex(min.X,min.Y,min.Z, 0,0,0, video::SColor(255,255,255,255), txc[4],txc[5]),
			video::S3DVertex(max.X,min.Y,min.Z, 0,0,0, video::SColor(255,255,255,255), txc[6],txc[5]),
			video::S3DVertex(max.X,min.Y,max.Z, 0,0,0, video::SColor(255,255,255,255), txc[6],txc[7]),
			video::S3DVertex(min.X,min.Y,max.Z, 0,0,0, video::SColor(255,255,255,255), txc[4],txc[7])
		},{	// right
			video::S3DVertex(max.X,max.Y,min.Z, 0,0,0, video::SColor(255,255,255,255), txc[ 8],txc[9]),
			video::S3DVertex(max.X,max.Y,max.Z, 0,0,0, video::SColor(255,255,255,255), txc[10],txc[9]),
			video::S3DVertex(max.X,min.Y,max.Z, 0,0,0, video::SColor(255,255,255,255), txc[10],txc[11]),
			video::S3DVertex(max.X,min.Y,min.Z, 0,0,0, video::SColor(255,255,255,255), txc[ 8],txc[11])
		},{	// left
			video::S3DVertex(min.X,max.Y,max.Z, 0,0,0, video::SColor(255,255,255,255), txc[12],txc[13]),
			video::S3DVertex(min.X,max.Y,min.Z, 0,0,0, video::SColor(255,255,255,255), txc[14],txc[13]),
			video::S3DVertex(min.X,min.Y,min.Z, 0,0,0, video::SColor(255,255,255,255), txc[14],txc[15]),
			video::S3DVertex(min.X,min.Y,max.Z, 0,0,0, video::SColor(255,255,255,255), txc[12],txc[15])
		},{	// back
			video::S3DVertex(max.X,max.Y,max.Z, 0,0,0, video::SColor(255,255,255,255), txc[16],txc[17]),
			video::S3DVertex(min.X,max.Y,max.Z, 0,0,0, video::SColor(255,255,255,255), txc[18],txc[17]),
			video::S3DVertex(min.X,min.Y,max.Z, 0,0,0, video::SColor(255,255,255,255), txc[18],txc[19]),
			video::S3DVertex(max.X,min.Y,max.Z, 0,0,0, video::SColor(255,255,255,255), txc[16],txc[19])
		},{	// front
			video::S3DVertex(min.X,max.Y,min.Z, 0,0,0, video::SColor(255,255,255,255), txc[20],txc[21]),
			video::S3DVertex(max.X,max.Y,min.Z, 0,0,0, video::SColor(255,255,255,255), txc[22],txc[21]),
			video::S3DVertex(max.X,min.Y,min.Z, 0,0,0, video::SColor(255,255,255,255), txc[22],txc[23]),
			video::S3DVertex(min.X,min.Y,min.Z, 0,0,0, video::SColor(255,255,255,255), txc[20],txc[23])
		}
	};

	u16 indices[] = {0,1,2,2,3,0};
	for (u16 i=0; i<6; i++) {
		int tileindex = MYMIN(i, tilecount-1);
		for (s32 j=0; j<4; j++) {
			vertices[i][j].Pos -= centre;
			if (angle.Y)
				vertices[i][j].Pos.rotateXZBy(angle.Y);
			if (angle.X)
				vertices[i][j].Pos.rotateYZBy(angle.X);
			if (angle.Z)
				vertices[i][j].Pos.rotateXYBy(angle.Z);
			vertices[i][j].Pos += centre;
			vertices[i][j].TCoords *= tiles[tileindex].texture.size;
			vertices[i][j].TCoords += tiles[tileindex].texture.pos;
		}
		// don't draw unseen faces
		bool skip = false;
		switch (i) {
		case 0:
		{
			bool edge = true;
			for (s32 j=0; edge && j<4; j++) {
				if (vertices[i][j].Pos.Y < 0.49*data->m_BS)
					edge = false;
			}
			if (edge && !meshgen_hardface(data,p,n,v3s16(0,1,0)))
				skip = true;
		}
			break;
		case 1:
		{
			bool edge = true;
			for (s32 j=0; edge && j<4; j++) {
				if (vertices[i][j].Pos.Y > -0.49*data->m_BS)
					edge = false;
			}
			if (edge && !meshgen_hardface(data,p,n,v3s16(0,-1,0)))
				skip = true;
		}
			break;
		case 2:
		{
			bool edge = true;
			for (s32 j=0; edge && j<4; j++) {
				if (vertices[i][j].Pos.X < 0.49*data->m_BS)
					edge = false;
			}
			if (edge && !meshgen_hardface(data,p,n,v3s16(1,0,0)))
				skip = true;
		}
			break;
		case 3:
		{
			bool edge = true;
			for (s32 j=0; edge && j<4; j++) {
				if (vertices[i][j].Pos.X > -0.49*data->m_BS)
					edge = false;
			}
			if (edge && !meshgen_hardface(data,p,n,v3s16(-1,0,0)))
				skip = true;
		}
			break;
		case 4:
		{
			bool edge = true;
			for (s32 j=0; edge && j<4; j++) {
				if (vertices[i][j].Pos.Z < 0.49*data->m_BS)
					edge = false;
			}
			if (edge && !meshgen_hardface(data,p,n,v3s16(0,0,1)))
				skip = true;
		}
			break;
		case 5:
		{
			bool edge = true;
			for (s32 j=0; edge && j<4; j++) {
				if (vertices[i][j].Pos.Z > -0.49*data->m_BS)
					edge = false;
			}
			if (edge && !meshgen_hardface(data,p,n,v3s16(0,0,-1)))
				skip = true;
		}
			break;
		}
		if (skip)
			continue;
		std::vector<u32> colours;
		if (cols) {
			meshgen_custom_lights(colours,cols[0],cols[1],cols[2],cols[3],4);
		}else if (selected.is_coloured) {
			meshgen_selected_lights(colours,255,4);
		}else{
			meshgen_lights(data,n,p,colours,255,faces[i],4,vertices[i]);
		}
		for (s32 j=0; j<4; j++) {
			vertices[i][j].Pos += pos;
		}
		data->append(tiles[tileindex], vertices[i], 4, indices, 6, colours);
	}
}

/* TODO: this can also have the fuck optimised out of it, make less faces where possible */
static void meshgen_build_nodebox(MeshMakeData *data, v3s16 p, MapNode &n, SelectedNode &selected, std::vector<NodeBox> &boxes, TileSpec *tiles)
{
	v3f pos = intToFloat(p,BS);
	for (std::vector<NodeBox>::iterator i = boxes.begin(); i != boxes.end(); i++) {
		NodeBox box = *i;

		// Compute texture coords
		f32 tx1 = (box.m_box.MinEdge.X/data->m_BS)+0.5;
		f32 ty1 = (box.m_box.MinEdge.Y/data->m_BS)+0.5;
		f32 tz1 = (box.m_box.MinEdge.Z/data->m_BS)+0.5;
		f32 tx2 = (box.m_box.MaxEdge.X/data->m_BS)+0.5;
		f32 ty2 = (box.m_box.MaxEdge.Y/data->m_BS)+0.5;
		f32 tz2 = (box.m_box.MaxEdge.Z/data->m_BS)+0.5;
		f32 txc[24] = {
			// up
			tx1, 1-tz2, tx2, 1-tz1,
			// down
			tx1, tz1, tx2, tz2,
			// right
			tz1, 1-ty2, tz2, 1-ty1,
			// left
			1-tz2, 1-ty2, 1-tz1, 1-ty1,
			// back
			1-tx2, 1-ty2, 1-tx1, 1-ty1,
			// front
			tx1, 1-ty2, tx2, 1-ty1,
		};
		meshgen_cuboid(data,n,p, pos, box.m_box, tiles, 6, selected, txc, box.m_angle, box.m_centre);
	}
}

static void meshgen_rooftri(MeshMakeData *data, MapNode &n, v3s16 p, v3f corners[3], v3f pos, TileSpec &tile, SelectedNode &selected, s16 rot, v3s16 face)
{
	// vertices for top and bottom tri
	v3f top_v[3];
	v3f btm_v[3];
	// tex coords for top and bottom tri
	v2f top_t[3];
	v2f btm_t[3];
	// faces for top and bottom tri
	v3s16 upface = face;
	v3s16 downface = face;
	if (downface.X)
		downface.X *= -1;
	if (downface.Y)
		downface.Y *= -1;
	if (downface.Z)
		downface.Z *= -1;
	for (int i=0; i<3; i++) {
		top_v[i].X = (corners[i].X*BS);
		top_v[i].Y = ((corners[i].Y+0.01)*BS)+data->m_BSd;
		top_v[i].Z = (corners[i].Z*BS);
		top_t[i].X = (corners[i].X+0.5);
		top_t[i].Y = (corners[i].Z+0.5);
		if (rot)
			top_t[i] = top_t[i].rotateBy(rot,v2f(0.5,0.5));
		top_t[i].X = (top_t[i].X*tile.texture.size.X)+tile.texture.pos.X;
		top_t[i].Y = (top_t[i].Y*tile.texture.size.Y)+tile.texture.pos.Y;

		// reverse winding for bottom
		btm_v[2-i].X = (corners[i].X*BS);
		btm_v[2-i].Y = ((corners[i].Y-0.01)*BS)-data->m_BSd;
		btm_v[2-i].Z = (corners[i].Z*BS);
		btm_t[2-i].X = top_t[i].X;
		btm_t[2-i].Y = top_t[i].Y;
	}

	{
		video::S3DVertex tri_v[3] = {
			video::S3DVertex(btm_v[0].X, btm_v[0].Y, btm_v[0].Z, 0,0,0, video::SColor(255,255,255,255), btm_t[0].X, btm_t[0].Y),
			video::S3DVertex(btm_v[1].X, btm_v[1].Y, btm_v[1].Z, 0,0,0, video::SColor(255,255,255,255), btm_t[1].X, btm_t[1].Y),
			video::S3DVertex(btm_v[2].X, btm_v[2].Y, btm_v[2].Z, 0,0,0, video::SColor(255,255,255,255), btm_t[2].X, btm_t[2].Y),
		};
		u16 indices[] = {0,1,2};
		std::vector<u32> colours;
		if (selected.is_coloured) {
			meshgen_selected_lights(colours,255,3);
		}else{
			meshgen_lights(data,n,p,colours,255,downface,3,tri_v);
		}
		tri_v[0].Pos += pos;
		tri_v[1].Pos += pos;
		tri_v[2].Pos += pos;
		data->append(tile, tri_v, 3, indices, 3, colours);
	}
	{
		video::S3DVertex tri_v[3] = {
			video::S3DVertex(top_v[0].X, top_v[0].Y, top_v[0].Z, 0,0,0, video::SColor(255,255,255,255), top_t[0].X, top_t[0].Y),
			video::S3DVertex(top_v[1].X, top_v[1].Y, top_v[1].Z, 0,0,0, video::SColor(255,255,255,255), top_t[1].X, top_t[1].Y),
			video::S3DVertex(top_v[2].X, top_v[2].Y, top_v[2].Z, 0,0,0, video::SColor(255,255,255,255), top_t[2].X, top_t[2].Y),
		};
		u16 indices[] = {0,1,2};
		std::vector<u32> colours;
		if (selected.is_coloured) {
			meshgen_selected_lights(colours,255,3);
		}else{
			meshgen_lights(data,n,p,colours,255,upface,3,tri_v);
		}
		tri_v[0].Pos += pos;
		tri_v[1].Pos += pos;
		tri_v[2].Pos += pos;
		data->append(tile, tri_v, 3, indices, 3, colours);
	}
}

void meshgen_preset_smooth_lights(MeshMakeData *data, v3s16 p)
{
	v3s16 pos = data->m_blockpos_nodes+p;
	for (u16 i=0; i<8; i++) {
		smooth_lights[i] = getSmoothLight(pos,corners[i],data->m_vmanip);
	}
}

void meshgen_cubelike(MeshMakeData *data, v3s16 p, MapNode &n, SelectedNode &selected)
{
	v3f pos = intToFloat(p, BS);
	if (meshgen_hardface(data,p,n,v3s16(-1,0,0))) {
		TileSpec tile = getNodeTile(n,p,v3s16(-1,0,0),selected,NULL);
		video::S3DVertex vertices[4] = {
			video::S3DVertex(-0.5*data->m_BS, 0.5*data->m_BS, 0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tile.texture.x0(), tile.texture.y0()),
			video::S3DVertex(-0.5*data->m_BS, 0.5*data->m_BS,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tile.texture.x1(), tile.texture.y0()),
			video::S3DVertex(-0.5*data->m_BS,-0.5*data->m_BS,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tile.texture.x1(), tile.texture.y1()),
			video::S3DVertex(-0.5*data->m_BS,-0.5*data->m_BS, 0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tile.texture.x0(), tile.texture.y1())
		};

		u16 indices[6] = {0,1,2,2,3,0};
		std::vector<u32> colours;
		if (selected.is_coloured) {
			meshgen_selected_lights(colours,255,4);
		}else{
			meshgen_lights(data,n,p,colours,255,v3s16(-1,0,0),4,vertices);
		}

		for (u16 i=0; i<4; i++) {
			vertices[i].Pos += pos;
		}

		data->append(tile, vertices, 4, indices, 6, colours);
	}
	if (meshgen_hardface(data,p,n,v3s16(1,0,0))) {
		TileSpec tile = getNodeTile(n,p,v3s16(1,0,0),selected,NULL);
		video::S3DVertex vertices[4] = {
			video::S3DVertex(0.5*data->m_BS,-0.5*data->m_BS, 0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tile.texture.x1(), tile.texture.y1()),
			video::S3DVertex(0.5*data->m_BS,-0.5*data->m_BS,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tile.texture.x0(), tile.texture.y1()),
			video::S3DVertex(0.5*data->m_BS, 0.5*data->m_BS,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tile.texture.x0(), tile.texture.y0()),
			video::S3DVertex(0.5*data->m_BS, 0.5*data->m_BS, 0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tile.texture.x1(), tile.texture.y0())
		};

		u16 indices[6] = {0,1,2,2,3,0};
		std::vector<u32> colours;
		if (selected.is_coloured) {
			meshgen_selected_lights(colours,255,4);
		}else{
			meshgen_lights(data,n,p,colours,255,v3s16(1,0,0),4,vertices);
		}

		for (u16 i=0; i<4; i++) {
			vertices[i].Pos += pos;
		}

		data->append(tile, vertices, 4, indices, 6, colours);
	}
	if (meshgen_hardface(data,p,n,v3s16(0,-1,0))) {
		TileSpec tile = getNodeTile(n,p,v3s16(0,-1,0),selected,NULL);
		video::S3DVertex vertices[4] = {
			video::S3DVertex( 0.5*data->m_BS,-0.5*data->m_BS, 0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tile.texture.x0(), tile.texture.y0()),
			video::S3DVertex(-0.5*data->m_BS,-0.5*data->m_BS, 0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tile.texture.x1(), tile.texture.y0()),
			video::S3DVertex(-0.5*data->m_BS,-0.5*data->m_BS,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tile.texture.x1(), tile.texture.y1()),
			video::S3DVertex( 0.5*data->m_BS,-0.5*data->m_BS,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tile.texture.x0(), tile.texture.y1())
		};

		u16 indices[6] = {0,1,2,2,3,0};
		std::vector<u32> colours;
		if (selected.is_coloured) {
			meshgen_selected_lights(colours,255,4);
		}else{
			meshgen_lights(data,n,p,colours,255,v3s16(0,-1,0),4,vertices);
		}

		for (u16 i=0; i<4; i++) {
			vertices[i].Pos += pos;
		}

		data->append(tile, vertices, 4, indices, 6, colours);
	}
	if (meshgen_hardface(data,p,n,v3s16(0,1,0))) {
		TileSpec tile = getNodeTile(n,p,v3s16(0,1,0),selected,NULL);
		video::S3DVertex vertices[4] = {
			video::S3DVertex( 0.5*data->m_BS, 0.5*data->m_BS,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tile.texture.x1(), tile.texture.y1()),
			video::S3DVertex(-0.5*data->m_BS, 0.5*data->m_BS,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tile.texture.x0(), tile.texture.y1()),
			video::S3DVertex(-0.5*data->m_BS, 0.5*data->m_BS, 0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tile.texture.x0(), tile.texture.y0()),
			video::S3DVertex( 0.5*data->m_BS, 0.5*data->m_BS, 0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tile.texture.x1(), tile.texture.y0())
		};

		u16 indices[6] = {0,1,2,2,3,0};
		std::vector<u32> colours;
		if (selected.is_coloured) {
			meshgen_selected_lights(colours,255,4);
		}else{
			meshgen_lights(data,n,p,colours,255,v3s16(0,1,0),4,vertices);
		}

		for (u16 i=0; i<4; i++) {
			vertices[i].Pos += pos;
		}

		data->append(tile, vertices, 4, indices, 6, colours);
	}
	if (meshgen_hardface(data,p,n,v3s16(0,0,-1))) {
		TileSpec tile = getNodeTile(n,p,v3s16(0,0,-1),selected,NULL);
		video::S3DVertex vertices[4] = {
			video::S3DVertex(-0.5*data->m_BS, 0.5*data->m_BS,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tile.texture.x0(), tile.texture.y0()),
			video::S3DVertex( 0.5*data->m_BS, 0.5*data->m_BS,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tile.texture.x1(), tile.texture.y0()),
			video::S3DVertex( 0.5*data->m_BS,-0.5*data->m_BS,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tile.texture.x1(), tile.texture.y1()),
			video::S3DVertex(-0.5*data->m_BS,-0.5*data->m_BS,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tile.texture.x0(), tile.texture.y1())
		};

		u16 indices[6] = {0,1,2,2,3,0};
		std::vector<u32> colours;
		if (selected.is_coloured) {
			meshgen_selected_lights(colours,255,4);
		}else{
			meshgen_lights(data,n,p,colours,255,v3s16(0,0,-1),4,vertices);
		}

		for (u16 i=0; i<4; i++) {
			vertices[i].Pos += pos;
		}

		data->append(tile, vertices, 4, indices, 6, colours);
	}
	if (meshgen_hardface(data,p,n,v3s16(0,0,1))) {
		TileSpec tile = getNodeTile(n,p,v3s16(0,0,1),selected,NULL);
		video::S3DVertex vertices[4] = {
			video::S3DVertex( 0.5*data->m_BS, 0.5*data->m_BS, 0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tile.texture.x0(), tile.texture.y0()),
			video::S3DVertex(-0.5*data->m_BS, 0.5*data->m_BS, 0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tile.texture.x1(), tile.texture.y0()),
			video::S3DVertex(-0.5*data->m_BS,-0.5*data->m_BS, 0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tile.texture.x1(), tile.texture.y1()),
			video::S3DVertex( 0.5*data->m_BS,-0.5*data->m_BS, 0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tile.texture.x0(), tile.texture.y1())
		};

		u16 indices[6] = {0,1,2,2,3,0};
		std::vector<u32> colours;
		if (selected.is_coloured) {
			meshgen_selected_lights(colours,255,4);
		}else{
			meshgen_lights(data,n,p,colours,255,v3s16(0,0,1),4,vertices);
		}

		for (u16 i=0; i<4; i++) {
			vertices[i].Pos += pos;
		}

		data->append(tile, vertices, 4, indices, 6, colours);
	}
}

void meshgen_dirtlike(MeshMakeData *data, v3s16 p, MapNode &n, SelectedNode &selected)
{
	/*
	 * param1:
	 * 	top nibble:
	 * 		0 - no effect
	 * 		1 - footsteps
	 * 	bottom nibble:
	 *		0 - no overlay
	 * 		1 - spring grass
	 * 		2 - autumn grass
	 * 		4 - snow
	 *		6 - polluted grass
	 * 		8 - jungle grass
	 *
	 * param2:
	 *  plantgrowth, only valid if param1 bottom nibble is 1 or 2
	 *	top nibble:
	 * 		bitwise OR direction of growth
	 *	bottom nibble:
	 *		0 - fully grown
	 *		1-15 - growth stages
	 */

	u8 effect = (n.param1&0xF0)>>4;
	u8 overlay = (n.param1&0x0F);

	TileSpec basetile = content_features(n.getContent()).tiles[1];
	TileSpec toptile;
	TileSpec sidetile;
	TileSpec upstile;
	upstile.material_flags = 0;
	upstile.texture = g_texturesource->getTexture("grass_corner.png");

	/*
	 * 0: top
	 * 1: bottom
	 * 2: right
	 * 3: left
	 * 4: back
	 * 5: front
	*/
	bool faces[6] = {
		false,
		false,
		false,
		false,
		false,
		false
	};
	bool o_faces[6] = {
		false,
		false,
		false,
		false,
		false,
		false
	};
	if (data->mesh_detail > 1) {
		faces[0] = meshgen_hardface(data,p,n,v3s16(0,1,0));
		faces[1] = meshgen_hardface(data,p,n,v3s16(0,-1,0));
		faces[2] = meshgen_hardface(data,p,n,v3s16(1,0,0));
		faces[3] = meshgen_hardface(data,p,n,v3s16(-1,0,0));
		faces[4] = meshgen_hardface(data,p,n,v3s16(0,0,1));
		faces[5] = meshgen_hardface(data,p,n,v3s16(0,0,-1));
	}

	if (!effect && !overlay) {
		toptile = basetile;
	}else{
		std::string btex = g_texturesource->getTextureName(basetile.texture.id);
		std::string tex = btex;
		switch (overlay) {
		case 8:
			if (n.param2 == 0) {
				tex = "grass_jungle.png";
				if (data->mesh_detail > 2) {
					for (int i=0; i<6; i++) {
						o_faces[i] = faces[i];
					}
				}
			}else{
				tex = getGrassTile(n.param2,btex,"grass_growing_jungle.png");
				if (data->mesh_detail > 2) {
					u8 pg = n.param2&0xF0;
					if ((pg&(1<<7)) != 0) { // -Z
						o_faces[5] = faces[5];
					}
					if ((pg&(1<<6)) != 0) { // +Z
						o_faces[4] = faces[4];
					}
					if ((pg&(1<<5)) != 0) { // -X
						o_faces[3] = faces[3];
					}
					if ((pg&(1<<4)) != 0) { // +X
						o_faces[2] = faces[2];
					}
				}
			}
			sidetile.texture = g_texturesource->getTexture("grass_side_jungle.png");
			upstile.texture = g_texturesource->getTexture("grass_corner_jungle.png");
			break;
		case 6:
			tex = "grass_polluted.png";
			if (data->mesh_detail > 2) {
				for (int i=0; i<6; i++) {
					o_faces[i] = faces[i];
				}
			}
			sidetile.texture = g_texturesource->getTexture("grass_side_polluted.png");
			break;
		case 4:
			tex = "snow.png";
			if (data->mesh_detail > 2) {
				for (int i=0; i<6; i++) {
					o_faces[i] = faces[i];
				}
			}
			sidetile.texture = g_texturesource->getTexture("snow_side.png");
			break;
		case 2:
			if (n.param2 == 0) {
				tex = "grass_autumn.png";
				if (data->mesh_detail > 2) {
					for (int i=0; i<6; i++) {
						o_faces[i] = faces[i];
					}
				}
			}else{
				tex = getGrassTile(n.param2,btex,"grass_growing_autumn.png");
				if (data->mesh_detail > 2) {
					u8 pg = n.param2&0xF0;
					if ((pg&(1<<7)) != 0) { // -Z
						o_faces[5] = faces[5];
					}
					if ((pg&(1<<6)) != 0) { // +Z
						o_faces[4] = faces[4];
					}
					if ((pg&(1<<5)) != 0) { // -X
						o_faces[3] = faces[3];
					}
					if ((pg&(1<<4)) != 0) { // +X
						o_faces[2] = faces[2];
					}
				}
			}
			sidetile.texture = g_texturesource->getTexture("grass_side_autumn.png");
			break;
		case 1:
			if (n.param2 == 0) {
				tex = "grass.png";
				if (data->mesh_detail > 2) {
					for (int i=0; i<6; i++) {
						o_faces[i] = faces[i];
					}
				}
			}else{
				tex = getGrassTile(n.param2,btex,"grass_growing.png");
				if (data->mesh_detail > 2) {
					u8 pg = n.param2&0xF0;
					if ((pg&(1<<7)) != 0) { // -Z
						o_faces[5] = faces[5];
					}
					if ((pg&(1<<6)) != 0) { // +Z
						o_faces[4] = faces[4];
					}
					if ((pg&(1<<5)) != 0) { // -X
						o_faces[3] = faces[3];
					}
					if ((pg&(1<<4)) != 0) { // +X
						o_faces[2] = faces[2];
					}
				}
			}
			sidetile.texture = g_texturesource->getTexture("grass_side.png");
			upstile.texture = g_texturesource->getTexture("grass_corner_spring.png");
			break;
		case 0:
		default:;
		}
		if ((effect&0x01) == 0x01)
			tex += "^footsteps.png";
		if ((effect&0x02) == 0x02)
			tex += "^mineral_salt.png";
		toptile.texture = g_texturesource->getTexture(tex);
	}

	if (selected.has_crack) {
		toptile = getCrackTile(toptile,selected);
		basetile = getCrackTile(basetile,selected);
	}

	if (data->mesh_detail == 1) {
		v3f pos = intToFloat(p, BS);
		TileSpec tiles[6];
		tiles[0] = toptile;
		tiles[1] = content_features(n.getContent()).tiles[1];
		{
			std::string tex1 = g_texturesource->getTextureName(basetile.texture.id);
			std::string tex2 = g_texturesource->getTextureName(sidetile.texture.id);
			tiles[2].texture = g_texturesource->getTexture(tex1+"^"+tex2);
			for (u16 i=3; i<6; i++) {
				tiles[i] = tiles[2];
			}
		}
		aabb3f box(-0.5*data->m_BS,-0.5*data->m_BS,-0.5*data->m_BS,0.5*data->m_BS,0.5*data->m_BS,0.5*data->m_BS);
		meshgen_cuboid(data,n,p,pos,box,tiles,6,selected,NULL,v3s16(0,0,0),v3f(0,0,0),NULL);
		return;
	}

	bool ups[6] = {
		false,
		false,
		false,
		false
	};

	float heights[4] = {
		0.0, // x+,z+
		0.0, // x+,z-
		0.0, // x-,z-
		0.0  // x-,z+
	};

	if (data->mesh_detail > 1) {
		v3s16 np = data->m_blockpos_nodes + p;
		v3s16 nearby_p[9] = {
			v3s16(-1,0,1),  // 0 x-,z+
			v3s16(0,0,1),   // 1 x ,z+
			v3s16(1,0,1),   // 2 x+,z+
			v3s16(-1,0,0),  // 3 x-,z
			v3s16(0,0,0),   // 4 x ,z
			v3s16(1,0,0),   // 5 x+,z
			v3s16(-1,0,-1), // 6 x-,z-
			v3s16(0,0,-1),  // 7 x ,z-
			v3s16(1,0,-1)   // 8 x+,z-
		};
		u16 corners[4][4] = {
			{4,5,2,1},
			{4,5,8,7},
			{4,3,6,7},
			{4,3,0,1}
		};
		u16 upsi[9] = {
			0,
			2,
			0,
			1,
			0,
			0,
			0,
			3,
			0
		};
		v3s16 corners_p[4] = {
			v3s16(1,0,1),   // 0 x+,z+
			v3s16(1,0,0),   // 1 x+,z
			v3s16(0,0,0),   // 2 x ,z
			v3s16(0,0,1),   // 3 x ,z+
		};
		content_t nearby[9][2];
		for (int i=0; i<9; i++) {
			nearby[i][0] = data->m_vmanip.getNodeRO(np+nearby_p[i]).getContent();
			nearby[i][1] = data->m_vmanip.getNodeRO(np+nearby_p[i]+v3s16(0,1,0)).getContent();
			if (i%2 && content_features(nearby[i][0]).draw_type == CDT_DIRTLIKE) {
				ContentFeatures *f = &content_features(nearby[i][1]);
				if (
					f->draw_type == CDT_CUBELIKE
					|| f->draw_type == CDT_DIRTLIKE
					|| f->draw_type == CDT_TRUNKLIKE
					|| f->draw_type == CDT_GLASSLIKE
				)
					ups[upsi[i]] = true;
			}
		}

		for (int i=0; i<4; i++) {
			v3s16 cp = np+corners_p[i];
			bool candown = true;
			bool canup = true;
			bool change = true;
			for (int k=0; change && k<4; k++) {
				if (content_features(nearby[corners[i][k]][0]).draw_type != CDT_DIRTLIKE) {
					if (nearby[corners[i][k]][0] != CONTENT_AIR)
						change = false;
					canup = false;
				}
				if (nearby[corners[i][k]][1] != CONTENT_AIR) {
					if (content_features(nearby[corners[i][k]][1]).draw_type != CDT_DIRTLIKE)
						change = false;
					candown = false;
				}
			}
			if (!change || (!canup && !candown))
				continue;

			u8 v = (cp.X%7)+(cp.Y%6)+(cp.Z%9);
			switch (v) {
			case 1:
			case 10:
				if (canup)
					heights[i] = 0.0625;
				break;
			case 2:
			case 15:
				if (candown)
					heights[i] = -0.03125;
				break;
			case 17:
				if (candown)
					heights[i] = -0.125;
				break;
			case 4:
			case 11:
				if (canup)
					heights[i] = 0.09375;
				break;
			case 12:
			case 20:
				if (candown)
					heights[i] = -0.0625;
				break;
			case 6:
			case 13:
				if (canup)
					heights[i] = 0.03125;
				break;
			case 9:
				if (canup)
					heights[i] = 0.125;
				break;
			case 8:
			case 14:
				if (candown)
					heights[i] = -0.09375;
				break;
			default:;
			}
		}
	}

	v3f pos = intToFloat(p, BS);

	if (faces[0]) {
		video::S3DVertex v[4] = {
			video::S3DVertex( 0.5*data->m_BS, (0.5+heights[1])*data->m_BS,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), toptile.texture.x1(), toptile.texture.y1()),
			video::S3DVertex(-0.5*data->m_BS, (0.5+heights[2])*data->m_BS,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), toptile.texture.x0(), toptile.texture.y1()),
			video::S3DVertex(-0.5*data->m_BS, (0.5+heights[3])*data->m_BS, 0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), toptile.texture.x0(), toptile.texture.y0()),
			video::S3DVertex( 0.5*data->m_BS, (0.5+heights[0])*data->m_BS, 0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), toptile.texture.x1(), toptile.texture.y0())
		};

		if (o_faces[2] && o_faces[3] && o_faces[4] && o_faces[5]) {
			v[0].Pos.X += 0.03125*data->m_BS;
			v[0].Pos.Z -= 0.03125*data->m_BS;
			v[1].Pos.X -= 0.03125*data->m_BS;
			v[1].Pos.Z -= 0.03125*data->m_BS;
			v[2].Pos.X -= 0.03125*data->m_BS;
			v[2].Pos.Z += 0.03125*data->m_BS;
			v[3].Pos.X += 0.03125*data->m_BS;
			v[3].Pos.Z += 0.03125*data->m_BS;
		}

		u16 indices[6] = {0,1,2,2,3,0};
		std::vector<u32> colours;
		if (selected.is_coloured) {
			meshgen_selected_lights(colours,255,4);
		}else{
			meshgen_lights(data,n,p,colours,255,v3s16(0,1,0),4,v);
		}

		for (u16 i=0; i<4; i++) {
			v[i].Pos += pos;
		}

		data->append(toptile, v, 4, indices, 6, colours);

		if (!o_faces[2] || !o_faces[3] || !o_faces[4] || !o_faces[5]) {
			if (o_faces[2]) {
				video::S3DVertex v2[4] = {
					video::S3DVertex(0.53125*data->m_BS, (0.5001+heights[1])*data->m_BS,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 0.03125, 1.),
					video::S3DVertex(0.5*data->m_BS, (0.5001+heights[1])*data->m_BS,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 0., 1.),
					video::S3DVertex(0.5*data->m_BS, (0.5001+heights[0])*data->m_BS, 0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 0., 0.),
					video::S3DVertex(0.53125*data->m_BS, (0.5001+heights[0])*data->m_BS, 0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 0.03125, 0.)
				};
				if (o_faces[4]) {
					v2[2].Pos.Z += 0.03125*data->m_BS;
					v2[3].Pos.Z += 0.03125*data->m_BS;
				}
				colours.clear();
				if (selected.is_coloured) {
					meshgen_selected_lights(colours,255,4);
				}else{
					meshgen_lights(data,n,p,colours,255,v3s16(0,1,0),4,v2);
				}

				for (u16 i=0; i<4; i++) {
					v2[i].Pos += pos;
					v2[i].TCoords *= toptile.texture.size;
					v2[i].TCoords += toptile.texture.pos;
				}

				data->append(toptile, v2, 4, indices, 6, colours);
			}
			if (o_faces[3]) {
				video::S3DVertex v2[4] = {
					video::S3DVertex(-0.5*data->m_BS, (0.5001+heights[2])*data->m_BS,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 1., 1.),
					video::S3DVertex(-0.53125*data->m_BS, (0.5001+heights[2])*data->m_BS,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 0.96875, 1.),
					video::S3DVertex(-0.53125*data->m_BS, (0.5001+heights[3])*data->m_BS, 0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 0.96875, 0.),
					video::S3DVertex(-0.5*data->m_BS, (0.5001+heights[3])*data->m_BS, 0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 1., 0.)
				};
				if (o_faces[5]) {
					v2[0].Pos.Z -= 0.03125*data->m_BS;
					v2[1].Pos.Z -= 0.03125*data->m_BS;
				}
				//v[1].Pos.X -= 0.03125*data->m_BS;
				//v[2].Pos.X -= 0.03125*data->m_BS;
				colours.clear();
				if (selected.is_coloured) {
					meshgen_selected_lights(colours,255,4);
				}else{
					meshgen_lights(data,n,p,colours,255,v3s16(0,1,0),4,v2);
				}

				for (u16 i=0; i<4; i++) {
					v2[i].Pos += pos;
					v2[i].TCoords *= toptile.texture.size;
					v2[i].TCoords += toptile.texture.pos;
				}

				data->append(toptile, v2, 4, indices, 6, colours);
			}
			if (o_faces[4]) {
				video::S3DVertex v2[4] = {
					video::S3DVertex( 0.5*data->m_BS, (0.5001+heights[0])*data->m_BS, 0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 1., 1.),
					video::S3DVertex(-0.5*data->m_BS, (0.5001+heights[3])*data->m_BS, 0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 0., 1.),
					video::S3DVertex(-0.5*data->m_BS, (0.5001+heights[3])*data->m_BS, 0.53125*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 0., 0.96875),
					video::S3DVertex( 0.5*data->m_BS, (0.5001+heights[0])*data->m_BS, 0.53125*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 1., 0.96875)
				};
				if (o_faces[3]) {
					v2[1].Pos.X -= 0.03125*data->m_BS;
					v2[2].Pos.X -= 0.03125*data->m_BS;
				}
				//v[2].Pos.Z += 0.03125*data->m_BS;
				//v[3].Pos.Z += 0.03125*data->m_BS;
				colours.clear();
				if (selected.is_coloured) {
					meshgen_selected_lights(colours,255,4);
				}else{
					meshgen_lights(data,n,p,colours,255,v3s16(0,1,0),4,v2);
				}

				for (u16 i=0; i<4; i++) {
					v2[i].Pos += pos;
					v2[i].TCoords *= toptile.texture.size;
					v2[i].TCoords += toptile.texture.pos;
				}

				data->append(toptile, v2, 4, indices, 6, colours);
			}
			if (o_faces[5]) {
				video::S3DVertex v2[4] = {
					video::S3DVertex( 0.5*data->m_BS, (0.5001+heights[1])*data->m_BS,-0.53125*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 1., 0.03125),
					video::S3DVertex(-0.5*data->m_BS, (0.5001+heights[2])*data->m_BS,-0.53125*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 0., 0.03125),
					video::S3DVertex(-0.5*data->m_BS, (0.5001+heights[2])*data->m_BS,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 0., 0.),
					video::S3DVertex( 0.5*data->m_BS, (0.5001+heights[1])*data->m_BS,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 1., 0.)
				};
				if (o_faces[2]) {
					v2[0].Pos.X += 0.03125*data->m_BS;
					v2[3].Pos.X += 0.03125*data->m_BS;
				}
				//v[0].Pos.Z -= 0.03125*data->m_BS;
				//v[1].Pos.Z -= 0.03125*data->m_BS;
				colours.clear();
				if (selected.is_coloured) {
					meshgen_selected_lights(colours,255,4);
				}else{
					meshgen_lights(data,n,p,colours,255,v3s16(0,1,0),4,v2);
				}

				for (u16 i=0; i<4; i++) {
					v2[i].Pos += pos;
					v2[i].TCoords *= toptile.texture.size;
					v2[i].TCoords += toptile.texture.pos;
				}

				data->append(toptile, v2, 4, indices, 6, colours);
			}
		}
	}
	if (faces[1]) {
		video::S3DVertex v[4] = {
			video::S3DVertex( 0.5*data->m_BS,-0.5*data->m_BS, 0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), basetile.texture.x0(), basetile.texture.y0()),
			video::S3DVertex(-0.5*data->m_BS,-0.5*data->m_BS, 0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), basetile.texture.x1(), basetile.texture.y0()),
			video::S3DVertex(-0.5*data->m_BS,-0.5*data->m_BS,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), basetile.texture.x1(), basetile.texture.y1()),
			video::S3DVertex( 0.5*data->m_BS,-0.5*data->m_BS,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), basetile.texture.x0(), basetile.texture.y1())
		};

		u16 indices[6] = {0,1,2,2,3,0};
		std::vector<u32> colours;
		if (selected.is_coloured) {
			meshgen_selected_lights(colours,255,4);
		}else{
			meshgen_lights(data,n,p,colours,255,v3s16(0,-1,0),4,v);
		}

		for (u16 i=0; i<4; i++) {
			v[i].Pos += pos;
		}

		data->append(basetile, v, 4, indices, 6, colours);
	}

	video::S3DVertex vertices[4] = {
		video::S3DVertex(0.5*data->m_BS,-0.5*data->m_BS, 0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), basetile.texture.x1(), basetile.texture.y1()),
		video::S3DVertex(0.5*data->m_BS,-0.5*data->m_BS,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), basetile.texture.x0(), basetile.texture.y1()),
		video::S3DVertex(0.5*data->m_BS, 0.5*data->m_BS,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 0., 0.),
		video::S3DVertex(0.5*data->m_BS, 0.5*data->m_BS, 0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 1., 0.)
	};
	video::S3DVertex u_vertices[4] = {
		video::S3DVertex(0.5*data->m_BS,0.5*data->m_BS, 0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), upstile.texture.x1(), upstile.texture.y1()),
		video::S3DVertex(0.5*data->m_BS,0.5*data->m_BS,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), upstile.texture.x0(), upstile.texture.y1()),
		video::S3DVertex(0.375*data->m_BS,1.5*data->m_BS,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), upstile.texture.x0(), upstile.texture.y0()),
		video::S3DVertex(0.375*data->m_BS,1.5*data->m_BS, 0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), upstile.texture.x1(), upstile.texture.y0())
	};
	video::S3DVertex o_vertices[4] = {
		video::S3DVertex(0.53125*data->m_BS,-0.5*data->m_BS, 0.5002*data->m_BS, 0,0,0, video::SColor(255,255,255,255), sidetile.texture.x1(), sidetile.texture.y1()),
		video::S3DVertex(0.53125*data->m_BS,-0.5*data->m_BS,-0.5002*data->m_BS, 0,0,0, video::SColor(255,255,255,255), sidetile.texture.x0(), sidetile.texture.y1()),
		video::S3DVertex(0.53125*data->m_BS, 0.5002*data->m_BS,-0.5002*data->m_BS, 0,0,0, video::SColor(255,255,255,255), sidetile.texture.x0(), sidetile.texture.y0()),
		video::S3DVertex(0.53125*data->m_BS, 0.5002*data->m_BS, 0.5002*data->m_BS, 0,0,0, video::SColor(255,255,255,255), sidetile.texture.x1(), sidetile.texture.y0())
	};
	u16 angle[6] = {0,0,0,180,90,270};
	u16 fh[6][2] = {
		{0,0},
		{0,0},
		{1,0},
		{3,2},
		{0,3},
		{2,1}
	};
	u16 o_alts[6][2] = {
		{0,0},
		{0,0},
		{4,5},
		{5,4},
		{3,2},
		{2,3}
	};
	v3s16 sdirs[6] = {
		v3s16(0,1,0),
		v3s16(0,-1,0),
		v3s16(1,0,0),
		v3s16(-1,0,0),
		v3s16(0,0,1),
		v3s16(0,0,-1)
	};
	if (data->mesh_detail < 3 && data->texture_detail > 1) {
		std::string base_name;
		std::string o_name;

		{
			u32 orig_id = basetile.texture.id;
			base_name = g_texturesource->getTextureName(orig_id);
		}
		{
			u32 orig_id = sidetile.texture.id;
			o_name = g_texturesource->getTextureName(orig_id);
		}
		// Create new texture name
		std::ostringstream os;
		os<<base_name<<"^"<<o_name;

		// Get new texture
		u32 new_id = g_texturesource->getTextureId(os.str());

		basetile.texture = g_texturesource->getTexture(new_id);
	}
	for (int face=2; face<6; face++) {
		video::S3DVertex v[4];
		u16 indices[6] = {0,1,2,2,3,0};

		if (data->mesh_detail > 2 && overlay && faces[0] && ups[face-2]) {
			for (u16 i=0; i<4; i++) {
				v[i] = u_vertices[i];
				v[i].Pos.rotateXZBy(angle[face]);
			}
			std::vector<u32> colours;
			if (selected.is_coloured) {
				meshgen_selected_lights(colours,255,4);
			}else{
				meshgen_lights(data,n,p,colours,255,sdirs[face],4,v);
			}

			for (u16 i=0; i<4; i++) {
				v[i].Pos += pos;
			}

			data->append(upstile, v, 4, indices, 6, colours);
		}
		if (!faces[face])
			continue;

		for (u16 i=0; i<4; i++) {
			v[i] = vertices[i];
			v[i].Pos.rotateXZBy(angle[face]);
		}
		v[2].Pos.Y += heights[fh[face][0]]*data->m_BS;
		v[3].Pos.Y += heights[fh[face][1]]*data->m_BS;
		v[2].TCoords.Y -= heights[fh[face][0]];
		v[2].TCoords *= basetile.texture.size;
		v[2].TCoords += basetile.texture.pos;
		v[3].TCoords.Y -= heights[fh[face][1]];
		v[3].TCoords *= basetile.texture.size;
		v[3].TCoords += basetile.texture.pos;

		std::vector<u32> colours;
		if (selected.is_coloured) {
			meshgen_selected_lights(colours,255,4);
		}else{
			meshgen_lights(data,n,p,colours,255,sdirs[face],4,v);
		}

		for (u16 i=0; i<4; i++) {
			v[i].Pos += pos;
		}

		data->append(basetile, v, 4, indices, 6, colours);
		if (!o_faces[face])
			continue;

		for (u16 i=0; i<4; i++) {
			v[i] = o_vertices[i];
			if ((i == 0 || i == 3) && o_faces[o_alts[face][0]]) {
				v[i].Pos.Z += 0.03125*data->m_BS;
			}
			if ((i == 1 || i == 2) && o_faces[o_alts[face][1]]) {
				v[i].Pos.Z -= 0.03125*data->m_BS;
			}
			v[i].Pos.rotateXZBy(angle[face]);
		}
		v[2].Pos.Y += heights[fh[face][0]]*data->m_BS;
		v[3].Pos.Y += heights[fh[face][1]]*data->m_BS;

		colours.clear();
		if (selected.is_coloured) {
			meshgen_selected_lights(colours,255,4);
		}else{
			meshgen_lights(data,n,p,colours,255,sdirs[face],4,v);
		}

		for (u16 i=0; i<4; i++) {
			v[i].Pos += pos;
		}

		data->append(sidetile, v, 4, indices, 6, colours);
	}
}

/* TODO: should use custom vertexes instead of boxes for curved rails */
void meshgen_raillike(MeshMakeData *data, v3s16 p, MapNode &n, SelectedNode &selected)
{
	bool is_rail_x [] = { false, false };  /* x-1, x+1 */
	bool is_rail_z [] = { false, false };  /* z-1, z+1 */

	bool is_rail_z_minus_y [] = { false, false };  /* z-1, z+1; y-1 */
	bool is_rail_x_minus_y [] = { false, false };  /* x-1, z+1; y-1 */
	bool is_rail_z_plus_y [] = { false, false };  /* z-1, z+1; y+1 */
	bool is_rail_x_plus_y [] = { false, false };  /* x-1, x+1; y+1 */

	MapNode n_minus_x = data->m_vmanip.getNodeRO(data->m_blockpos_nodes + p + v3s16(-1,0,0));
	MapNode n_plus_x = data->m_vmanip.getNodeRO(data->m_blockpos_nodes + p + v3s16(1,0,0));
	MapNode n_minus_z = data->m_vmanip.getNodeRO(data->m_blockpos_nodes + p + v3s16(0,0,-1));
	MapNode n_plus_z = data->m_vmanip.getNodeRO(data->m_blockpos_nodes + p + v3s16(0,0,1));
	MapNode n_plus_x_plus_y = data->m_vmanip.getNodeRO(data->m_blockpos_nodes + p + v3s16(1, 1, 0));
	MapNode n_plus_x_minus_y = data->m_vmanip.getNodeRO(data->m_blockpos_nodes + p + v3s16(1, -1, 0));
	MapNode n_minus_x_plus_y = data->m_vmanip.getNodeRO(data->m_blockpos_nodes + p + v3s16(-1, 1, 0));
	MapNode n_minus_x_minus_y = data->m_vmanip.getNodeRO(data->m_blockpos_nodes + p + v3s16(-1, -1, 0));
	MapNode n_plus_z_plus_y = data->m_vmanip.getNodeRO(data->m_blockpos_nodes + p + v3s16(0, 1, 1));
	MapNode n_minus_z_plus_y = data->m_vmanip.getNodeRO(data->m_blockpos_nodes + p + v3s16(0, 1, -1));
	MapNode n_plus_z_minus_y = data->m_vmanip.getNodeRO(data->m_blockpos_nodes + p + v3s16(0, -1, 1));
	MapNode n_minus_z_minus_y = data->m_vmanip.getNodeRO(data->m_blockpos_nodes + p + v3s16(0, -1, -1));

	content_t thiscontent = n.getContent();

	if (n_minus_x.getContent() == thiscontent)
		is_rail_x[0] = true;
	if (n_minus_x_minus_y.getContent() == thiscontent)
		is_rail_x_minus_y[0] = true;
	if (n_minus_x_plus_y.getContent() == thiscontent)
		is_rail_x_plus_y[0] = true;
	if (n_plus_x.getContent() == thiscontent)
		is_rail_x[1] = true;
	if (n_plus_x_minus_y.getContent() == thiscontent)
		is_rail_x_minus_y[1] = true;
	if (n_plus_x_plus_y.getContent() == thiscontent)
		is_rail_x_plus_y[1] = true;
	if (n_minus_z.getContent() == thiscontent)
		is_rail_z[0] = true;
	if (n_minus_z_minus_y.getContent() == thiscontent)
		is_rail_z_minus_y[0] = true;
	if (n_minus_z_plus_y.getContent() == thiscontent)
		is_rail_z_plus_y[0] = true;
	if (n_plus_z.getContent() == thiscontent)
		is_rail_z[1] = true;
	if (n_plus_z_minus_y.getContent() == thiscontent)
		is_rail_z_minus_y[1] = true;
	if (n_plus_z_plus_y.getContent() == thiscontent)
		is_rail_z_plus_y[1] = true;

	bool is_rail_x_all[] = {false, false};
	bool is_rail_z_all[] = {false, false};
	is_rail_x_all[0] = is_rail_x[0] || is_rail_x_minus_y[0] || is_rail_x_plus_y[0];
	is_rail_x_all[1] = is_rail_x[1] || is_rail_x_minus_y[1] || is_rail_x_plus_y[1];
	is_rail_z_all[0] = is_rail_z[0] || is_rail_z_minus_y[0] || is_rail_z_plus_y[0];
	is_rail_z_all[1] = is_rail_z[1] || is_rail_z_minus_y[1] || is_rail_z_plus_y[1];

	// reasonable default, flat straight unrotated rail
	bool is_straight = true;
	int adjacencies = 0;
	int angle = 0;
	u8 type = 0;

	// check for sloped rail
	if (is_rail_x_plus_y[0] || is_rail_x_plus_y[1] || is_rail_z_plus_y[0] || is_rail_z_plus_y[1]) {
		adjacencies = 5; //5 means sloped
		is_straight = true; // sloped is always straight
	}else{
		// is really straight, rails on both sides
		is_straight = (is_rail_x_all[0] && is_rail_x_all[1]) || (is_rail_z_all[0] && is_rail_z_all[1]);
		adjacencies = is_rail_x_all[0] + is_rail_x_all[1] + is_rail_z_all[0] + is_rail_z_all[1];
	}

	switch (adjacencies) {
	case 1:
		if(is_rail_x_all[0] || is_rail_x_all[1])
			angle = 90;
		break;
	case 2:
		if(!is_straight)
			type = 1; // curved
		if(is_rail_x_all[0] && is_rail_x_all[1])
			angle = 90;
		if(is_rail_z_all[0] && is_rail_z_all[1]){
			if (is_rail_z_plus_y[0])
				angle = 180;
		}
		else if(is_rail_x_all[0] && is_rail_z_all[0])
			angle = 270;
		else if(is_rail_x_all[0] && is_rail_z_all[1])
			angle = 180;
		else if(is_rail_x_all[1] && is_rail_z_all[1])
			angle = 90;
		break;
	case 3:
		// here is where the potential to 'switch' a junction is, but not implemented at present
		type = 2; // t-junction
		if(!is_rail_x_all[1])
			angle=180;
		if(!is_rail_z_all[0])
			angle=90;
		if(!is_rail_z_all[1])
			angle=270;
		break;
	case 4:
		type = 3; // crossing
		break;
	case 5: //sloped
		if(is_rail_z_plus_y[0])
			angle = 180;
		if(is_rail_x_plus_y[0])
			angle = 90;
		if(is_rail_x_plus_y[1])
			angle = 270;
		break;
	default:
		break;
	}

	static const v3s16 tile_dirs[6] = {
		v3s16(0, 1, 0),
		v3s16(0, -1, 0),
		v3s16(1, 0, 0),
		v3s16(-1, 0, 0),
		v3s16(0, 0, 1),
		v3s16(0, 0, -1)
	};
	TileSpec tiles[6];
	TileSpec *tile;
	for (int i = 0; i < 6; i++) {
		// Handles facedir rotation for textures
		tiles[i] = getNodeTile(n,p,tile_dirs[i],selected);
	}
	v3f pos = intToFloat(p,BS);

	switch (type) {
	case 0: // straight
	{
		if (is_rail_x_plus_y[0] || is_rail_x_plus_y[1] || is_rail_z_plus_y[0] || is_rail_z_plus_y[1]) {
			aabb3f track[5] = {
				aabb3f(-0.375*data->m_BS,-0.03125*data->m_BS,-0.4375*data->m_BS,0.375*data->m_BS,0.03125*data->m_BS,-0.25*data->m_BS),
				aabb3f(-0.375*data->m_BS,-0.03125*data->m_BS,0.0625*data->m_BS,0.375*data->m_BS,0.03125*data->m_BS,0.25*data->m_BS),
				aabb3f(-0.375*data->m_BS,-0.03125*data->m_BS,0.5*data->m_BS,0.375*data->m_BS,0.03125*data->m_BS,0.6875*data->m_BS),
				aabb3f(0.1875*data->m_BS,0.03125*data->m_BS,-0.64*data->m_BS,0.25*data->m_BS,0.09375*data->m_BS,0.77*data->m_BS),
				aabb3f(-0.25*data->m_BS,0.03125*data->m_BS,-0.64*data->m_BS,-0.1875*data->m_BS,0.09375*data->m_BS,0.77*data->m_BS)
			};
			tile = &tiles[0];
			v3s16 an(0,angle,0);
			switch (angle) {
			case 90:
				an.Z = -45;
				break;
			case 180:
				an.X = 45;
				break;
			case 270:
				an.Z = 45;
				break;
			default:
				an.X = -45;
			}
			for (int bi=0; bi<5; bi++) {
				if (bi == 3)
					tile = &tiles[1];
				f32 tx1 = (track[bi].MinEdge.X/data->m_BS)+0.5;
				f32 ty1 = (track[bi].MinEdge.Y/data->m_BS)+0.5;
				f32 tz1 = (track[bi].MinEdge.Z/data->m_BS)+0.5;
				f32 tx2 = (track[bi].MaxEdge.X/data->m_BS)+0.5;
				f32 ty2 = (track[bi].MaxEdge.Y/data->m_BS)+0.5;
				f32 tz2 = (track[bi].MaxEdge.Z/data->m_BS)+0.5;
				f32 txc[24] = {
					// up
					tx1, 1-tz2, tx2, 1-tz1,
					// down
					tx1, tz1, tx2, tz2,
					// right
					tz1, 1-ty2, tz2, 1-ty1,
					// left
					1-tz2, 1-ty2, 1-tz1, 1-ty1,
					// back
					1-tx2, 1-ty2, 1-tx1, 1-ty1,
					// front
					tx1, 1-ty2, tx2, 1-ty1,
				};
				meshgen_cuboid(data,n,p,pos,track[bi],tile,1, selected,txc,an, v3f(0,0,0));
			}
		}else{
			aabb3f track[4] = {
				aabb3f(-0.375*data->m_BS,-0.5*data->m_BS,-0.375*data->m_BS,0.375*data->m_BS,-0.4375*data->m_BS,-0.1875*data->m_BS),
				aabb3f(-0.375*data->m_BS,-0.5*data->m_BS,0.1875*data->m_BS,0.375*data->m_BS,-0.4375*data->m_BS,0.375*data->m_BS),
				aabb3f(-0.25*data->m_BS,-0.4375*data->m_BS,-0.5*data->m_BS,-0.1875*data->m_BS,-0.375*data->m_BS,0.5*data->m_BS),
				aabb3f(0.1875*data->m_BS,-0.4375*data->m_BS,-0.5*data->m_BS,0.25*data->m_BS,-0.375*data->m_BS,0.5*data->m_BS)
			};
			tile = &tiles[0];
			for (int bi=0; bi<4; bi++) {
				if (bi == 2)
					tile = &tiles[1];
				f32 tx1 = (track[bi].MinEdge.X/data->m_BS)+0.5;
				f32 ty1 = (track[bi].MinEdge.Y/data->m_BS)+0.5;
				f32 tz1 = (track[bi].MinEdge.Z/data->m_BS)+0.5;
				f32 tx2 = (track[bi].MaxEdge.X/data->m_BS)+0.5;
				f32 ty2 = (track[bi].MaxEdge.Y/data->m_BS)+0.5;
				f32 tz2 = (track[bi].MaxEdge.Z/data->m_BS)+0.5;
				f32 txc[24] = {
					// up
					tx1, 1-tz2, tx2, 1-tz1,
					// down
					tx1, tz1, tx2, tz2,
					// right
					tz1, 1-ty2, tz2, 1-ty1,
					// left
					1-tz2, 1-ty2, 1-tz1, 1-ty1,
					// back
					1-tx2, 1-ty2, 1-tx1, 1-ty1,
					// front
					tx1, 1-ty2, tx2, 1-ty1,
				};
				meshgen_cuboid(data,n,p,pos,track[bi],tile,1, selected,txc,v3s16(0,angle,0), v3f(0,0,0));
			}
		}
		break;
	}
	case 1: // curved
	{
		aabb3f track[10] = {
			aabb3f(-0.4375*data->m_BS,-0.5*data->m_BS,-0.375*data->m_BS,0.3125*data->m_BS,-0.4375*data->m_BS,-0.1875*data->m_BS),
			aabb3f(-0.375*data->m_BS,-0.499*data->m_BS,-0.4375*data->m_BS,-0.1875*data->m_BS,-0.4385*data->m_BS,0.3125*data->m_BS),

			aabb3f(0.1875*data->m_BS,-0.4375*data->m_BS,-0.5*data->m_BS,0.25*data->m_BS,-0.375*data->m_BS,-0.0625*data->m_BS),
			aabb3f(0.1575*data->m_BS,-0.4365*data->m_BS,-0.125*data->m_BS,0.22*data->m_BS,-0.374*data->m_BS,0.0),
			aabb3f(0.15*data->m_BS,-0.4375*data->m_BS,-0.0625*data->m_BS,0.2135*data->m_BS,-0.375*data->m_BS,0.0625*data->m_BS),
			aabb3f(-0.125*data->m_BS,-0.4365*data->m_BS,0.1575*data->m_BS,0.0,-0.374*data->m_BS,0.22*data->m_BS),
			aabb3f(-0.5*data->m_BS,-0.4375*data->m_BS,0.1875*data->m_BS,-0.0625*data->m_BS,-0.375*data->m_BS,0.25*data->m_BS),

			aabb3f(-0.25*data->m_BS,-0.4375*data->m_BS,-0.5*data->m_BS,-0.1875*data->m_BS,-0.375*data->m_BS,-0.3125*data->m_BS),
			aabb3f(0.3425*data->m_BS,-0.4385*data->m_BS,-0.0625*data->m_BS,0.415*data->m_BS,-0.374*data->m_BS,0.0625*data->m_BS),
			aabb3f(-0.5*data->m_BS,-0.4375*data->m_BS,-0.25*data->m_BS,-0.3125*data->m_BS,-0.375*data->m_BS,-0.1875*data->m_BS)
		};
		tile = &tiles[0];
		s16 a[10] = {110,70, 90,110,135,70,90, 90,-45,90};
		for (int bi=0; bi<10; bi++) {
			if (bi == 2)
				tile = &tiles[1];
			f32 tx1 = (track[bi].MinEdge.X/data->m_BS)+0.5;
			f32 ty1 = (track[bi].MinEdge.Y/data->m_BS)+0.5;
			f32 tz1 = (track[bi].MinEdge.Z/data->m_BS)+0.5;
			f32 tx2 = (track[bi].MaxEdge.X/data->m_BS)+0.5;
			f32 ty2 = (track[bi].MaxEdge.Y/data->m_BS)+0.5;
			f32 tz2 = (track[bi].MaxEdge.Z/data->m_BS)+0.5;
			f32 txc[24] = {
				// up
				tx1, 1-tz2, tx2, 1-tz1,
				// down
				tx1, tz1, tx2, tz2,
				// right
				tz1, 1-ty2, tz2, 1-ty1,
				// left
				1-tz2, 1-ty2, 1-tz1, 1-ty1,
				// back
				1-tx2, 1-ty2, 1-tx1, 1-ty1,
				// front
				tx1, 1-ty2, tx2, 1-ty1,
			};
			meshgen_cuboid(data,n,p,pos,track[bi],tile,1, selected,txc,v3s16(0,angle+a[bi],0),v3f(0,0,0));
		}
		break;
	}
	case 2: // t-junction
	{
		aabb3f track[13] = {
			aabb3f(-0.375*data->m_BS,-0.5*data->m_BS,-0.375*data->m_BS,0.375*data->m_BS,-0.4375*data->m_BS,-0.1875*data->m_BS),
			aabb3f(-0.375*data->m_BS,-0.5*data->m_BS,0.1875*data->m_BS,0.375*data->m_BS,-0.4375*data->m_BS,0.375*data->m_BS),
			aabb3f(-0.4375*data->m_BS,-0.499*data->m_BS,-0.375*data->m_BS,0.3125*data->m_BS,-0.4385*data->m_BS,-0.1875*data->m_BS),

			aabb3f(-0.25*data->m_BS,-0.4375*data->m_BS,-0.5*data->m_BS,-0.1875*data->m_BS,-0.375*data->m_BS,0.5*data->m_BS),

			aabb3f(0.1875*data->m_BS,-0.4375*data->m_BS,-0.5*data->m_BS,0.25*data->m_BS,-0.375*data->m_BS,0.125*data->m_BS),
			aabb3f(0.1875*data->m_BS,-0.4375*data->m_BS,0.25*data->m_BS,0.25*data->m_BS,-0.375*data->m_BS,0.5*data->m_BS),

			aabb3f(-0.25*data->m_BS,-0.4375*data->m_BS,-0.5*data->m_BS,-0.1875*data->m_BS,-0.375*data->m_BS,-0.3125*data->m_BS),
			aabb3f(0.3425*data->m_BS,-0.4385*data->m_BS,-0.0625*data->m_BS,0.415*data->m_BS,-0.374*data->m_BS,0.0625*data->m_BS),

			aabb3f(0.1875*data->m_BS,-0.4375*data->m_BS,-0.5*data->m_BS,0.25*data->m_BS,-0.375*data->m_BS,-0.1875*data->m_BS),
			aabb3f(0.1875*data->m_BS,-0.4375*data->m_BS,-0.125*data->m_BS,0.25*data->m_BS,-0.375*data->m_BS,-0.0625*data->m_BS),
			aabb3f(0.1575*data->m_BS,-0.4365*data->m_BS,-0.125*data->m_BS,0.22*data->m_BS,-0.374*data->m_BS,0.0),
			aabb3f(0.15*data->m_BS,-0.4375*data->m_BS,-0.0625*data->m_BS,0.2135*data->m_BS,-0.375*data->m_BS,0.0625*data->m_BS),
			aabb3f(-0.125*data->m_BS,-0.4365*data->m_BS,0.1575*data->m_BS,0.0,-0.374*data->m_BS,0.22*data->m_BS)
		};
		tile = &tiles[0];
		s16 a[13] = {0,0,110, 0, 0,0, 90,-45, 90,90,110,135,70};
		for (int bi=0; bi<13; bi++) {
			if (bi == 3)
				tile = &tiles[1];
			f32 tx1 = (track[bi].MinEdge.X/data->m_BS)+0.5;
			f32 ty1 = (track[bi].MinEdge.Y/data->m_BS)+0.5;
			f32 tz1 = (track[bi].MinEdge.Z/data->m_BS)+0.5;
			f32 tx2 = (track[bi].MaxEdge.X/data->m_BS)+0.5;
			f32 ty2 = (track[bi].MaxEdge.Y/data->m_BS)+0.5;
			f32 tz2 = (track[bi].MaxEdge.Z/data->m_BS)+0.5;
			f32 txc[24] = {
				// up
				tx1, 1-tz2, tx2, 1-tz1,
				// down
				tx1, tz1, tx2, tz2,
				// right
				tz1, 1-ty2, tz2, 1-ty1,
				// left
				1-tz2, 1-ty2, 1-tz1, 1-ty1,
				// back
				1-tx2, 1-ty2, 1-tx1, 1-ty1,
				// front
				tx1, 1-ty2, tx2, 1-ty1,
			};
			meshgen_cuboid(data,n,p,pos,track[bi],tile,1, selected,txc,v3s16(0,angle+a[bi],0),v3f(0,0,0));
		}
		break;
	}
	case 3: // crossing
	{
		aabb3f track[20] = {
			aabb3f(-0.375*data->m_BS,-0.5*data->m_BS,-0.3125*data->m_BS,0.375*data->m_BS,-0.4375*data->m_BS,-0.125*data->m_BS),
			aabb3f(-0.375*data->m_BS,-0.5*data->m_BS,0.125*data->m_BS,0.375*data->m_BS,-0.4375*data->m_BS,0.3125*data->m_BS),
			aabb3f(-0.3125*data->m_BS,-0.499*data->m_BS,-0.375*data->m_BS,-0.125*data->m_BS,-0.4385*data->m_BS,0.375*data->m_BS),
			aabb3f( 0.125*data->m_BS,-0.499*data->m_BS,-0.375*data->m_BS,0.3125*data->m_BS,-0.4385*data->m_BS,0.375*data->m_BS),

			aabb3f(-0.25*data->m_BS,-0.4375*data->m_BS,-0.5*data->m_BS,-0.1875*data->m_BS,-0.375*data->m_BS,-0.1875*data->m_BS),
			aabb3f(-0.25*data->m_BS,-0.4375*data->m_BS,0.1875*data->m_BS,-0.1875*data->m_BS,-0.375*data->m_BS,0.5*data->m_BS),
			aabb3f(-0.25*data->m_BS,-0.4375*data->m_BS,-0.125*data->m_BS,-0.1875*data->m_BS,-0.375*data->m_BS,0.125*data->m_BS),

			aabb3f(0.1875*data->m_BS,-0.4375*data->m_BS,-0.5*data->m_BS,0.25*data->m_BS,-0.375*data->m_BS,-0.1875*data->m_BS),
			aabb3f(0.1875*data->m_BS,-0.4375*data->m_BS,0.1875*data->m_BS,0.25*data->m_BS,-0.375*data->m_BS,0.5*data->m_BS),
			aabb3f(0.1875*data->m_BS,-0.4375*data->m_BS,-0.125*data->m_BS,0.25*data->m_BS,-0.375*data->m_BS,0.125*data->m_BS),

			aabb3f(-0.5*data->m_BS,-0.4375*data->m_BS,0.1875*data->m_BS,-0.25*data->m_BS,-0.375*data->m_BS,0.25*data->m_BS),
			aabb3f(0.25*data->m_BS,-0.4375*data->m_BS,0.1875*data->m_BS,0.5*data->m_BS,-0.375*data->m_BS,0.25*data->m_BS),
			aabb3f(-0.125*data->m_BS,-0.4375*data->m_BS,0.1875*data->m_BS,0.125*data->m_BS,-0.375*data->m_BS,0.25*data->m_BS),

			aabb3f(-0.5*data->m_BS,-0.4375*data->m_BS,-0.25*data->m_BS,-0.25*data->m_BS,-0.375*data->m_BS,-0.1875*data->m_BS),
			aabb3f(0.25*data->m_BS,-0.4375*data->m_BS,-0.25*data->m_BS,0.5*data->m_BS,-0.375*data->m_BS,-0.1875*data->m_BS),
			aabb3f(-0.125*data->m_BS,-0.4375*data->m_BS,-0.25*data->m_BS,0.125*data->m_BS,-0.375*data->m_BS,-0.1875*data->m_BS)
		};
		tile = &tiles[0];
		for (int bi=0; bi<16; bi++) {
			if (bi == 4)
				tile = &tiles[1];
			f32 tx1 = (track[bi].MinEdge.X/data->m_BS)+0.5;
			f32 ty1 = (track[bi].MinEdge.Y/data->m_BS)+0.5;
			f32 tz1 = (track[bi].MinEdge.Z/data->m_BS)+0.5;
			f32 tx2 = (track[bi].MaxEdge.X/data->m_BS)+0.5;
			f32 ty2 = (track[bi].MaxEdge.Y/data->m_BS)+0.5;
			f32 tz2 = (track[bi].MaxEdge.Z/data->m_BS)+0.5;
			f32 txc[24] = {
				// up
				tx1, 1-tz2, tx2, 1-tz1,
				// down
				tx1, tz1, tx2, tz2,
				// right
				tz1, 1-ty2, tz2, 1-ty1,
				// left
				1-tz2, 1-ty2, 1-tz1, 1-ty1,
				// back
				1-tx2, 1-ty2, 1-tx1, 1-ty1,
				// front
				tx1, 1-ty2, tx2, 1-ty1,
			};
			meshgen_cuboid(data,n,p,pos,track[bi],tile,1, selected,txc,v3s16(0,angle,0),v3f(0,0,0));
		}
		break;
	}
	default:;
	}
}

void meshgen_plantlike(MeshMakeData *data, v3s16 p, MapNode &n, SelectedNode &selected)
{
	ContentFeatures *f = &content_features(n);
	TileSpec tile = getNodeTile(n,p,v3s16(0,1,0),selected);
	v3f offset(0,0,0);
	if (data->m_vmanip.getNodeRO(data->m_blockpos_nodes + p + v3s16(0,-1,0)).getContent() == CONTENT_FLOWER_POT)
		offset = v3f(0,-0.25*data->m_BS,0);

	v3f pos_inner(0,0,0);
	{
		ContentFeatures *unf = f;
		content_t unc = n.getContent();
		v3s16 up = p;
		while (unf->draw_type == f->draw_type) {
			up.Y--;
			unc = data->m_vmanip.getNodeRO(data->m_blockpos_nodes + up).getContent();
			unf = &content_features(unc);
		}

		if ((unf->draw_type == CDT_CUBELIKE || unf->draw_type == CDT_DIRTLIKE) && unc != CONTENT_FARM_DIRT) {
			if (up.X%2) {
				pos_inner.Z = -0.1*data->m_BS;
			}else{
				pos_inner.Z = 0.1*data->m_BS;
			}
			if (up.Z%2) {
				pos_inner.X = -0.1*data->m_BS;
			}else{
				pos_inner.X = 0.1*data->m_BS;
			}
			if (up.Y%2) {
				pos_inner.X += 0.05*data->m_BS;
				pos_inner.Z -= 0.05*data->m_BS;
			}else{
				pos_inner.X -= 0.05*data->m_BS;
				pos_inner.Z += 0.05*data->m_BS;
			}
		}
	}

	f32 v0 = 0.;
	f32 v1 = 1.;
	f32 h = 0.5;

	if (f->plantlike_tiled) {
		if (f->param2_type == CPT_PLANTGROWTH && n.param2 != 0 && !f->plantgrowth_on_trellis) {
			h = (0.0625*(float)n.param2);
			if (data->m_vmanip.getNodeRO(data->m_blockpos_nodes + p + v3s16(0,-1,0)).getContent() != n.getContent()) {
				v0 = (1.0-h)/2;
			}else{
				v0 = ((1.0-h)/2)+0.25;
				v1 = 0.75;
			}
			h -= 0.5;
		}else if (data->m_vmanip.getNodeRO(data->m_blockpos_nodes + p + v3s16(0,-1,0)).getContent() != n.getContent()) {
			v0 = 0.5;
		}else if (data->m_vmanip.getNodeRO(data->m_blockpos_nodes + p + v3s16(0,1,0)).getContent() != n.getContent()) {
			v1 = 0.5;
		}else{
			v0 = 0.25;
			v1 = 0.75;
		}
	}else if (f->param2_type == CPT_PLANTGROWTH) {
		if (n.param2 != 0 && !f->plantgrowth_on_trellis) {
			h = (0.0625*(float)n.param2);
			v0 = (1.0-h);
			h -= 0.5;
		}
	}

	v3f pos = offset+intToFloat(p,BS)+pos_inner;

	if (selected.is_coloured || selected.has_crack) {
		for (u32 j=0; j<2; j++) {
			video::S3DVertex vertices[4] = {
				video::S3DVertex(-0.5*BS,-0.5*BS,0., 0,0,0, video::SColor(255,255,255,255), 0.,v1),
				video::S3DVertex( 0.5*BS,-0.5*BS,0., 0,0,0, video::SColor(255,255,255,255), 1.,v1),
				video::S3DVertex( 0.5*BS,   h*BS,0., 0,0,0, video::SColor(255,255,255,255), 1.,v0),
				video::S3DVertex(-0.5*BS,   h*BS,0., 0,0,0, video::SColor(255,255,255,255), 0.,v0)
			};

			s16 angle = 45;
			if (j == 1)
				angle = -45;

			for (u16 i=0; i<4; i++) {
				vertices[i].Pos.rotateXZBy(angle);
				vertices[i].TCoords *= tile.texture.size;
				vertices[i].TCoords += tile.texture.pos;
			}

			u16 indices[] = {0,1,2,2,3,0};
			std::vector<u32> colours;
			if (selected.is_coloured) {
				meshgen_selected_lights(colours,255,4);
			}else{
				meshgen_lights(data,n,p,colours,255,v3s16(0,0,0),4,vertices);
			}

			for (u16 i=0; i<4; i++) {
				vertices[i].Pos += pos;
				vertices[i].Pos.X += 0.005;
			}

			data->append(tile, vertices, 4, indices, 6, colours);

			for (u16 i=0; i<4; i++) {
				vertices[i].Pos.X -= 0.01;
			}

			data->append(tile, vertices, 4, indices, 6, colours);
		}
	}else{
		for (u32 j=0; j<2; j++) {
			video::S3DVertex vertices[4] = {
				video::S3DVertex(-0.5*BS,-0.5*BS,0., 0,0,0, video::SColor(255,255,255,255), 0.,v1),
				video::S3DVertex( 0.5*BS,-0.5*BS,0., 0,0,0, video::SColor(255,255,255,255), 1.,v1),
				video::S3DVertex( 0.5*BS,   h*BS,0., 0,0,0, video::SColor(255,255,255,255), 1.,v0),
				video::S3DVertex(-0.5*BS,   h*BS,0., 0,0,0, video::SColor(255,255,255,255), 0.,v0)
			};

			s16 angle = 45;
			if (j == 1)
				angle = -45;

			for (u16 i=0; i<4; i++) {
				vertices[i].Pos.rotateXZBy(angle);
				vertices[i].TCoords *= tile.texture.size;
				vertices[i].TCoords += tile.texture.pos;
			}

			u16 indices[] = {0,1,2,2,3,0};
			std::vector<u32> colours;
			if (selected.is_coloured) {
				meshgen_selected_lights(colours,255,4);
			}else{
				meshgen_lights(data,n,p,colours,255,v3s16(0,0,0),4,vertices);
			}

			for (u16 i=0; i<4; i++) {
				vertices[i].Pos += pos;
			}

			data->append(tile, vertices, 4, indices, 6, colours);
		}
	}

	if (selected.is_coloured || selected.has_crack)
		return;

	if (meshgen_check_plantlike_water(data, n, p, NULL) == 1) {
		tile.texture = g_texturesource->getTexture("water.png");
		tile.material_flags = 0;
		tile.material_type = MATERIAL_ALPHA_VERTEX;
		pos = intToFloat(p,BS);
		video::S3DVertex vertices[4] = {
			video::S3DVertex(-0.5*BS, 0.375*BS,-0.5*BS, 0,0,0, video::SColor(255,255,255,255), tile.texture.x0(),tile.texture.y0()),
			video::S3DVertex( 0.5*BS, 0.375*BS,-0.5*BS, 0,0,0, video::SColor(255,255,255,255), tile.texture.x1(),tile.texture.y0()),
			video::S3DVertex( 0.5*BS, 0.375*BS, 0.5*BS, 0,0,0, video::SColor(255,255,255,255), tile.texture.x1(),tile.texture.y1()),
			video::S3DVertex(-0.5*BS, 0.375*BS, 0.5*BS, 0,0,0, video::SColor(255,255,255,255), tile.texture.x0(),tile.texture.y1())
		};

		u16 indices[] = {0,1,2,2,3,0};
		std::vector<u32> colours;
		meshgen_lights(data,n,p,colours,160,v3s16(0,1,0),4,vertices);

		for (u16 i=0; i<4; i++) {
			vertices[i].Pos += pos;
		}

		data->append(tile, vertices, 4, indices, 6, colours);
	}
}

void meshgen_plantlike_fern(MeshMakeData *data, v3s16 p, MapNode &n, SelectedNode &selected)
{
	if (data->mesh_detail < 2) {
		meshgen_plantlike(data,p,n,selected);
		return;
	}
	ContentFeatures *f = &content_features(n);
	TileSpec tile = getNodeTile(n,p,v3s16(0,-1,0),selected);
	v3f offset(0,0,0);
	int rot = 0;
	bool is_dropped = false;
	if (data->m_vmanip.getNodeRO(data->m_blockpos_nodes + p + v3s16(0,-1,0)).getContent() == CONTENT_FLOWER_POT) {
		offset = v3f(0,-0.25*data->m_BS,0);
		is_dropped = true;
	}

	v3f pos_inner(0,0,0);
	{
		ContentFeatures *unf = f;
		content_t unc = n.getContent();
		v3s16 up = p;
		while (unf->draw_type == f->draw_type) {
			up.Y--;
			unc = data->m_vmanip.getNodeRO(data->m_blockpos_nodes + up).getContent();
			unf = &content_features(unc);
		}

		if ((unf->draw_type == CDT_CUBELIKE || unf->draw_type == CDT_DIRTLIKE) && unc != CONTENT_FARM_DIRT) {
			if (up.X%2) {
				pos_inner.Z = -0.1*data->m_BS;
				rot = 20;
			}else{
				pos_inner.Z = 0.1*data->m_BS;
				rot = -20;
			}
			if (up.Z%2) {
				pos_inner.X = -0.1*data->m_BS;
				rot += 10;
			}else{
				pos_inner.X = 0.1*data->m_BS;
				rot += -10;
			}
			if (up.Y%2) {
				pos_inner.X += 0.05*data->m_BS;
				pos_inner.Z -= 0.05*data->m_BS;
				rot += 5;
			}else{
				pos_inner.X -= 0.05*data->m_BS;
				pos_inner.Z += 0.05*data->m_BS;
				rot += -5;
			}
		}
	}

	v3f pos = offset+intToFloat(p,BS)+pos_inner;

	video::S3DVertex vb[4] = {
		video::S3DVertex(-0.5*BS,-0.5*BS,0., 0,0,0, video::SColor(255,255,255,255), 0.,1.),
		video::S3DVertex( 0.5*BS,-0.5*BS,0., 0,0,0, video::SColor(255,255,255,255), 1.,1.),
		video::S3DVertex( 0.5*BS, 1.0*BS,0., 0,0,0, video::SColor(255,255,255,255), 1.,0.),
		video::S3DVertex(-0.5*BS, 1.0*BS,0., 0,0,0, video::SColor(255,255,255,255), 0.,0.)
	};
	video::S3DVertex vl[8] = {
		// stalk
		video::S3DVertex( 0.5*data->m_BS, 0. *data->m_BS,0.*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 1.,1.),
		video::S3DVertex(-0.5*data->m_BS, 0. *data->m_BS,0.*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 0.,1.),
		video::S3DVertex(-0.5*data->m_BS, 0.5  *data->m_BS,1.*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 0.,0.3),
		video::S3DVertex( 0.5*data->m_BS, 0.5  *data->m_BS,1.*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 1.,0.3),
		// end
		video::S3DVertex( 0.5*data->m_BS, 0.5*data->m_BS, 1.*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 1.,0.3),
		video::S3DVertex(-0.5*data->m_BS, 0.5*data->m_BS, 1.*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 0.,0.3),
		video::S3DVertex(-0.5*data->m_BS, 0.28125*data->m_BS,1.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 0.,0.),
		video::S3DVertex( 0.5*data->m_BS, 0.28125*data->m_BS,1.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 1.,0.)
	};
	int angle[8] = {
		  45+rot,
		 -45+rot,
		 135+rot,
		-135+rot,
		  90+rot,
		   0+rot,
		 180+rot,
		 -90+rot
	};
	float vo[8] = {
		 0.0,
		 0.0,
		 0.0,
		 0.0,
		-0.25*BS,
		-0.25*BS,
		-0.25*BS,
		-0.25*BS
	};

	{
			MapNode n2 = data->m_vmanip.getNodeRO(data->m_blockpos_nodes + p + v3s16(0,1,0));
			if (
				content_features(n2).draw_type == CDT_PLANTLIKE
				|| content_features(n2).draw_type == CDT_PLANTLIKE_FERN
			) {
				if (is_dropped) {
					vb[2].Pos.Y = 0.75*BS;
					vb[2].TCoords.Y = 0.25;
					vb[3].Pos.Y = 0.75*BS;
					vb[3].TCoords.Y = 0.25;
				}else{
					vb[2].Pos.Y = 0.5*BS;
					vb[2].TCoords.Y = 0.375;
					vb[3].Pos.Y = 0.5*BS;
					vb[3].TCoords.Y = 0.375;
				}
			}
	}

	if (selected.is_coloured || selected.has_crack) {
		for (int i=0; i<8; i++) {
			vo[i] += 0.005;
		}
	}
	for (u32 j=0; j<2; j++) {
		video::S3DVertex v[4];

		for (u16 i=0; i<4; i++) {
			v[i] = vb[i];
			v[i].Pos.rotateXZBy(angle[j]);
			v[i].Pos.X += vo[j];
			v[i].TCoords *= tile.texture.size;
			v[i].TCoords += tile.texture.pos;
		}

		u16 indices[] = {0,1,2,2,3,0};
		std::vector<u32> colours;
		if (selected.is_coloured) {
			meshgen_selected_lights(colours,255,4);
		}else{
			meshgen_lights(data,n,p,colours,255,v3s16(0,0,0),4,v);
		}

		for (u16 i=0; i<4; i++) {
			v[i].Pos += pos;
		}

		data->append(tile, v, 4, indices, 6, colours);

		if (!selected.is_coloured && !selected.has_crack)
			continue;

		for (u16 i=0; i<4; i++) {
			v[i].Pos.X -= 0.01;
		}

		data->append(tile, v, 4, indices, 6, colours);
	}
	u32 max = (data->mesh_detail == 3 ? 8 : 4);
	for (u32 j=0; j<max; j++) {
		video::S3DVertex v[8];

		for (u16 i=0; i<8; i++) {
			v[i] = vl[i];
			v[i].Pos.rotateXZBy(angle[j]);
			v[i].Pos.Y += vo[j];
			v[i].TCoords *= tile.texture.size;
			v[i].TCoords += tile.texture.pos;
		}

		u16 indices[] = {0,1,2,2,3,0,4,5,6,6,7,4};
		std::vector<u32> colours;
		if (selected.is_coloured) {
			meshgen_selected_lights(colours,255,8);
		}else{
			meshgen_lights(data,n,p,colours,255,v3s16(0,0,0),8,v);
		}

		for (u16 i=0; i<8; i++) {
			v[i].Pos += pos;
		}

		data->append(tile, v, 8, indices, 12, colours);

		if (!selected.is_coloured && !selected.has_crack)
			continue;

		for (u16 i=0; i<8; i++) {
			v[i].Pos.Y -= 0.01;
		}

		data->append(tile, v, 8, indices, 12, colours);
	}

	if (selected.is_coloured || selected.has_crack)
		return;

	if (meshgen_check_plantlike_water(data, n, p, NULL) == 1) {
		tile.texture = g_texturesource->getTexture("water.png");
		tile.material_flags = 0;
		tile.material_type = MATERIAL_ALPHA_VERTEX;
		pos = intToFloat(p,BS);
		video::S3DVertex vertices[4] = {
			video::S3DVertex(-0.5*BS, 0.375*BS,-0.5*BS, 0,0,0, video::SColor(255,255,255,255), tile.texture.x0(),tile.texture.y0()),
			video::S3DVertex( 0.5*BS, 0.375*BS,-0.5*BS, 0,0,0, video::SColor(255,255,255,255), tile.texture.x1(),tile.texture.y0()),
			video::S3DVertex( 0.5*BS, 0.375*BS, 0.5*BS, 0,0,0, video::SColor(255,255,255,255), tile.texture.x1(),tile.texture.y1()),
			video::S3DVertex(-0.5*BS, 0.375*BS, 0.5*BS, 0,0,0, video::SColor(255,255,255,255), tile.texture.x0(),tile.texture.y1())
		};

		u16 indices[] = {0,1,2,2,3,0};
		std::vector<u32> colours;
		meshgen_lights(data,n,p,colours,160,v3s16(0,1,0),4,vertices);

		for (u16 i=0; i<4; i++) {
			vertices[i].Pos += pos;
		}

		data->append(tile, vertices, 4, indices, 6, colours);
	}
}

void meshgen_croplike(MeshMakeData *data, v3s16 p, MapNode &n, SelectedNode &selected)
{
	ContentFeatures *f = &content_features(n);
	TileSpec tile = getNodeTile(n,p,v3s16(0,1,0),selected);
	v3f offset(0,0,0);
	if (data->m_vmanip.getNodeRO(data->m_blockpos_nodes + p + v3s16(0,-1,0)).getContent() == CONTENT_FLOWER_POT)
		offset = v3f(0,-0.25*data->m_BS,0);

	f32 v0 = 0.;
	f32 v1 = 1.;
	f32 h = 0.5;

	if (f->param2_type == CPT_PLANTGROWTH) {
		if (n.param2 != 0 && !f->plantgrowth_on_trellis) {
			h = (0.0625*(float)n.param2);
			v0 = (1.0-h);
			h -= 0.5;
		}
	}

	v3f pos = offset+intToFloat(p,BS);

	for (u32 j=0; j<4; j++) {
		video::S3DVertex vertices[4] = {
			video::S3DVertex(-0.5*BS,-0.5*BS,-0.25*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 0.,v1),
			video::S3DVertex( 0.5*BS,-0.5*BS,-0.25*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 1.,v1),
			video::S3DVertex( 0.5*BS,   h*BS,-0.25*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 1.,v0),
			video::S3DVertex(-0.5*BS,   h*BS,-0.25*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 0.,v0)
		};

		s16 angle = j*90;

		for (u16 i=0; i<4; i++) {
			vertices[i].Pos.rotateXZBy(angle);
			vertices[i].TCoords *= tile.texture.size;
			vertices[i].TCoords += tile.texture.pos;
		}

		u16 indices[] = {0,1,2,2,3,0};
		std::vector<u32> colours;
		if (selected.is_coloured) {
			meshgen_selected_lights(colours,255,4);
		}else{
			meshgen_lights(data,n,p,colours,255,v3s16(0,0,0),4,vertices);
		}

		for (u16 i=0; i<4; i++) {
			vertices[i].Pos += pos;
		}

		data->append(tile, vertices, 4, indices, 6, colours);

		if (!selected.is_coloured && !selected.has_crack)
			continue;

		v3f hpos(0.0,0.0,data->m_BSd*2.0);
		hpos.rotateXZBy(angle);

		for (u16 i=0; i<4; i++) {
			vertices[i].Pos += hpos;
		}

		data->append(tile, vertices, 4, indices, 6, colours);
	}
}

void meshgen_liquid(MeshMakeData *data, v3s16 p, MapNode &n, SelectedNode &selected)
{
	ContentFeatures *f = &content_features(n);
	TileSpec *tiles = f->tiles;
	bool top_is_same_liquid = false;
	MapNode ntop = data->m_vmanip.getNodeRO(data->m_blockpos_nodes + p + v3s16(0,1,0));
	if (ntop.getContent() == f->liquid_alternative_flowing || ntop.getContent() == f->liquid_alternative_source)
		top_is_same_liquid = true;

	float node_liquid_level = 0.875;

	// Neighbor liquid levels (key = relative position)
	// Includes current node
	core::map<v3s16, f32> neighbor_levels;
	core::map<v3s16, content_t> neighbor_contents;
	core::map<v3s16, u8> neighbor_flags;
	const u8 neighborflag_top_is_same_liquid = 0x01;
	v3s16 neighbor_dirs[9] = {
		v3s16(0,0,0),
		v3s16(0,0,1),
		v3s16(0,0,-1),
		v3s16(1,0,0),
		v3s16(-1,0,0),
		v3s16(1,0,1),
		v3s16(-1,0,-1),
		v3s16(1,0,-1),
		v3s16(-1,0,1),
	};
	for (u32 i=0; i<9; i++) {
		content_t content = CONTENT_AIR;
		float level = -0.5 * data->m_BS;
		u8 flags = 0;
		// Check neighbor
		v3s16 p2 = p + neighbor_dirs[i];
		MapNode n2 = data->m_vmanip.getNodeRO(data->m_blockpos_nodes + p2);
		if (n2.getContent() != CONTENT_IGNORE) {
			content = n2.getContent();

			if (n2.getContent() == f->liquid_alternative_source) {
				p2.Y += 1;
				n2 = data->m_vmanip.getNodeRO(data->m_blockpos_nodes + p2);
				if (content_features(n2).liquid_type == LIQUID_NONE) {
					level = 0.5*data->m_BS;
				}else{
					level = (-0.5+node_liquid_level) * data->m_BS;
				}
				p2.Y -= 1;
			}else if (n2.getContent() == f->liquid_alternative_flowing) {
				level = (-0.5 + ((float)(n2.param2&LIQUID_LEVEL_MASK)
						+ 0.5) / 8.0 * node_liquid_level) * data->m_BS;
			}

			// Check node above neighbor.
			// NOTE: This doesn't get executed if neighbor
			//       doesn't exist
			p2.Y += 1;
			n2 = data->m_vmanip.getNodeRO(data->m_blockpos_nodes + p2);
			if (
				n2.getContent() == f->liquid_alternative_source
				|| n2.getContent() == f->liquid_alternative_flowing
			)
				flags |= neighborflag_top_is_same_liquid;
		}

		neighbor_levels.insert(neighbor_dirs[i], level);
		neighbor_contents.insert(neighbor_dirs[i], content);
		neighbor_flags.insert(neighbor_dirs[i], flags);
	}

	// Corner heights (average between four liquids)
	f32 corner_levels[4];

	v3s16 halfdirs[4] = {
		v3s16(0,0,0),
		v3s16(1,0,0),
		v3s16(1,0,1),
		v3s16(0,0,1),
	};
	for (u32 i=0; i<4; i++) {
		v3s16 cornerdir = halfdirs[i];
		float cornerlevel = 0;
		u32 valid_count = 0;
		u32 air_count = 0;
		for (u32 j=0; j<4; j++) {
			v3s16 neighbordir = cornerdir - halfdirs[j];
			content_t content = neighbor_contents[neighbordir];
			// If top is liquid, draw starting from top of node
			if ((neighbor_flags[neighbordir]&neighborflag_top_is_same_liquid) != 0) {
				cornerlevel = 0.5*data->m_BS;
				valid_count = 1;
				break;
			// Source is always the same height
			}else if (content == f->liquid_alternative_source) {
				cornerlevel = (-0.5+node_liquid_level)*data->m_BS;
				valid_count = 1;
				break;
			// Flowing liquid has level information
			}else if (content == f->liquid_alternative_flowing) {
				cornerlevel += neighbor_levels[neighbordir];
				valid_count++;
			}else if (content == CONTENT_AIR) {
				air_count++;
			}
		}
		if (air_count >= 2) {
			cornerlevel = -0.5*data->m_BS;
		}else if (valid_count > 0) {
			cornerlevel /= valid_count;
		}
		corner_levels[i] = cornerlevel;
	}

	/*
		Generate sides
	*/

	v3s16 side_dirs[4] = {
		v3s16(1,0,0),
		v3s16(-1,0,0),
		v3s16(0,0,1),
		v3s16(0,0,-1),
	};
	s16 side_corners[4][2] = {
		{1, 2},
		{3, 0},
		{2, 3},
		{0, 1},
	};
	v3f pos = intToFloat(p,BS);
	for (u32 i=0; i<4; i++) {
		v3s16 dir = side_dirs[i];

		/*
			If our topside is liquid and neighbor's topside
			is liquid, don't draw side face
		*/
		if (top_is_same_liquid && (neighbor_flags[dir]&neighborflag_top_is_same_liquid) != 0)
			continue;

		content_t neighbor_content = neighbor_contents[dir];
		ContentFeatures &n_feat = content_features(neighbor_content);

		// Don't draw face if neighbor is blocking the view
		if (
			n_feat.draw_type == CDT_CUBELIKE
			|| n_feat.draw_type == CDT_DIRTLIKE
			|| n_feat.draw_type == CDT_MELONLIKE
		)
			continue;

		bool neighbor_is_same_liquid = false;
		if (
			neighbor_content == f->liquid_alternative_source
			|| neighbor_content == f->liquid_alternative_flowing
		)
			neighbor_is_same_liquid = true;

		// Don't draw any faces if neighbor same is liquid and top is
		// same liquid
		if (neighbor_is_same_liquid == true && top_is_same_liquid == false)
			continue;

		video::S3DVertex vertices[4] = {
			video::S3DVertex(-0.5*data->m_BS,0,0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tiles[i].texture.x0(), tiles[i].texture.y1()),
			video::S3DVertex( 0.5*data->m_BS,0,0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tiles[i].texture.x1(), tiles[i].texture.y1()),
			video::S3DVertex( 0.5*data->m_BS,0,0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tiles[i].texture.x1(), tiles[i].texture.y0()),
			video::S3DVertex(-0.5*data->m_BS,0,0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tiles[i].texture.x0(), tiles[i].texture.y0()),
		};

		// If our topside is liquid, set upper border of face at upper border of node
		if (top_is_same_liquid) {
			vertices[2].Pos.Y = 0.5*data->m_BS;
			vertices[3].Pos.Y = 0.5*data->m_BS;
		// Otherwise upper position of face is corner levels
		}else{
			vertices[2].Pos.Y = corner_levels[side_corners[i][0]];
			vertices[3].Pos.Y = corner_levels[side_corners[i][1]];
		}

		// If neighbor is liquid, lower border of face is corner liquid levels
		if (neighbor_is_same_liquid) {
			vertices[0].Pos.Y = corner_levels[side_corners[i][1]];
			vertices[1].Pos.Y = corner_levels[side_corners[i][0]];
		// If neighbor is not liquid, lower border of face is lower border of node
		}else{
			vertices[0].Pos.Y = -0.5*data->m_BS;
			vertices[1].Pos.Y = -0.5*data->m_BS;
		}

		s16 angle = 0;
		switch (i) {
		case 0:
			angle = -90;
			break;
		case 1:
			angle = 90;
			break;
		case 3:
			angle = 180;
			break;
		default:;
		}

		for (s32 j=0; j<4; j++) {
			if (angle)
				vertices[j].Pos.rotateXZBy(angle);
		}

		u16 indices[] = {0,1,2,2,3,0};
		std::vector<u32> colours;
		if (selected.is_coloured) {
			meshgen_selected_lights(colours,f->vertex_alpha,4);
		}else{
			meshgen_lights(data,n,p,colours,f->vertex_alpha,dir,4,vertices);
		}

		for (s32 j=0; j<4; j++) {
			vertices[j].Pos += pos;
		}

		data->append(tiles[i], vertices, 4, indices, 6, colours);
	}

	/*
		Generate top side, if appropriate
	*/

	if (top_is_same_liquid == false) {
		video::S3DVertex vertices[4] = {
			video::S3DVertex(-0.5*data->m_BS,0, 0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tiles[0].texture.x0(), tiles[0].texture.y1()),
			video::S3DVertex( 0.5*data->m_BS,0, 0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tiles[0].texture.x1(), tiles[0].texture.y1()),
			video::S3DVertex( 0.5*data->m_BS,0,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tiles[0].texture.x1(), tiles[0].texture.y0()),
			video::S3DVertex(-0.5*data->m_BS,0,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tiles[0].texture.x0(), tiles[0].texture.y0())
		};

		// To get backface culling right, the vertices need to go
		// clockwise around the front of the face. And we happened to
		// calculate corner levels in exact reverse order.
		s32 corner_resolve[4] = {3,2,1,0};

		for (s32 i=0; i<4; i++) {
			s32 j = corner_resolve[i];
			vertices[i].Pos.Y += corner_levels[j];
			vertices[i].Pos += pos;
		}

		// Default downwards-flowing texture animation goes from
		// -Z towards +Z, thus the direction is +Z.
		// Rotate texture to make animation go in flow direction
		// Positive if liquid moves towards +Z
		int dz = (corner_levels[side_corners[3][0]] +
				corner_levels[side_corners[3][1]]) -
				(corner_levels[side_corners[2][0]] +
				corner_levels[side_corners[2][1]]);
		// Positive if liquid moves towards +X
		int dx = (corner_levels[side_corners[1][0]] +
				corner_levels[side_corners[1][1]]) -
				(corner_levels[side_corners[0][0]] +
				corner_levels[side_corners[0][1]]);
		// -X
		if (-dx >= abs(dz)) {
			v2f t = vertices[0].TCoords;
			vertices[0].TCoords = vertices[1].TCoords;
			vertices[1].TCoords = vertices[2].TCoords;
			vertices[2].TCoords = vertices[3].TCoords;
			vertices[3].TCoords = t;
		}
		// +X
		if (dx >= abs(dz)) {
			v2f t = vertices[0].TCoords;
			vertices[0].TCoords = vertices[3].TCoords;
			vertices[3].TCoords = vertices[2].TCoords;
			vertices[2].TCoords = vertices[1].TCoords;
			vertices[1].TCoords = t;
		}
		// -Z
		if (-dz >= abs(dx)) {
			v2f t = vertices[0].TCoords;
			vertices[0].TCoords = vertices[3].TCoords;
			vertices[3].TCoords = vertices[2].TCoords;
			vertices[2].TCoords = vertices[1].TCoords;
			vertices[1].TCoords = t;
			t = vertices[0].TCoords;
			vertices[0].TCoords = vertices[3].TCoords;
			vertices[3].TCoords = vertices[2].TCoords;
			vertices[2].TCoords = vertices[1].TCoords;
			vertices[1].TCoords = t;
		}

		u16 indices[] = {0,1,2,2,3,0};
		std::vector<u32> colours;
		if (selected.is_coloured) {
			meshgen_selected_lights(colours,f->vertex_alpha,4);
		}else{
			meshgen_lights(data,n,p,colours,f->vertex_alpha,v3s16(0,1,0),4,vertices);
		}

		data->append(tiles[0], vertices, 4, indices, 6, colours);
	}
}

void meshgen_liquid_source(MeshMakeData *data, v3s16 p, MapNode &n, SelectedNode &selected)
{
	ContentFeatures *f = &content_features(n);
	TileSpec *tiles = f->tiles;
	//    x,y       -,-  +,-  +,+  -,+
	bool drop[4] = {true,true,true,true};
	v3s16 n2p = data->m_blockpos_nodes + p + v3s16(0,1,0);
	MapNode n2 = data->m_vmanip.getNodeRO(n2p);
	ContentFeatures *f2 = &content_features(n2);
	if (f2->liquid_type != LIQUID_NONE) {
		drop[0] = false;
		drop[1] = false;
		drop[2] = false;
		drop[3] = false;
	}else{
		v3s16 dirs[8] = {
			v3s16(-1,1,-1),
			v3s16(0,1,-1),
			v3s16(1,1,-1),
			v3s16(1,1,0),
			v3s16(1,1,1),
			v3s16(0,1,1),
			v3s16(-1,1,1),
			v3s16(-1,1,0),
		};
		for (u32 i=0; i<8; i++) {
			n2p = data->m_blockpos_nodes + p + dirs[i];
			n2 = data->m_vmanip.getNodeRO(n2p);
			f2 = &content_features(n2);
			if (f2->liquid_type == LIQUID_NONE)
				continue;
			switch (i) {
			case 0:
				drop[0] = false;
				break;
			case 1:
				drop[0] = false;
				drop[1] = false;
				break;
			case 2:
				drop[1] = false;
				break;
			case 3:
				drop[1] = false;
				drop[2] = false;
				break;
			case 4:
				drop[2] = false;
				break;
			case 5:
				drop[2] = false;
				drop[3] = false;
				break;
			case 6:
				drop[3] = false;
				break;
			case 7:
				drop[3] = false;
				drop[0] = false;
				break;
			default:;
			}
		}
	}

	v3f pos = intToFloat(p,BS);

	for (u32 j=0; j<6; j++) {
		// Check this neighbor
		n2p = data->m_blockpos_nodes + p + g_6dirs[j];
		n2 = data->m_vmanip.getNodeRO(n2p);
		f2 = &content_features(n2);
		if (meshgen_check_plantlike_water(data,n2,p+g_6dirs[j],NULL)) {
			continue;
		}else if (f2->liquid_type != LIQUID_NONE) {
			if (n2.getContent() == f->liquid_alternative_flowing)
				continue;
			if (n2.getContent() == f->liquid_alternative_source)
				continue;
		}else if (!meshgen_hardface(data,p,n,g_6dirs[j])) {
			if (g_6dirs[j].Y != 1)
				continue;
			if (!drop[0] && !drop[1] && !drop[2] && !drop[3])
				continue;
		}else if (n2.getContent() == CONTENT_IGNORE) {
			continue;
		}

		// The face at Z+
		video::S3DVertex vertices[4] = {
			video::S3DVertex(-0.5*data->m_BS,-0.5*data->m_BS,0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tiles[j].texture.x0(), tiles[j].texture.y1()),
			video::S3DVertex( 0.5*data->m_BS,-0.5*data->m_BS,0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tiles[j].texture.x1(), tiles[j].texture.y1()),
			video::S3DVertex( 0.5*data->m_BS, 0.5*data->m_BS,0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tiles[j].texture.x1(), tiles[j].texture.y0()),
			video::S3DVertex(-0.5*data->m_BS, 0.5*data->m_BS,0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tiles[j].texture.x0(), tiles[j].texture.y0()),
		};
		switch (j) {
		case 0: // Z+
			if (drop[0])
				vertices[2].Pos.Y = 0.375*data->m_BS;
			if (drop[1])
				vertices[3].Pos.Y = 0.375*data->m_BS;
			break;
		case 1: // Y+
			if (!drop[1] && !drop[2] && !drop[3] && drop[0]) {
				for(u16 i=0; i<4; i++) {
					vertices[i].Pos.rotateXYBy(90);
				}
				vertices[2].Pos.Z = 0.375*data->m_BS;
			}else if (!drop[0] && !drop[1] && !drop[3] && drop[2]) {
				for(u16 i=0; i<4; i++) {
					vertices[i].Pos.rotateXYBy(90);
				}
				vertices[0].Pos.Z = 0.375*data->m_BS;
			}else{
				if (drop[0])
					vertices[3].Pos.Z = 0.375*data->m_BS;
				if (drop[1])
					vertices[2].Pos.Z = 0.375*data->m_BS;
				if (drop[2])
					vertices[1].Pos.Z = 0.375*data->m_BS;
				if (drop[3])
					vertices[0].Pos.Z = 0.375*data->m_BS;
			}
			for(u16 i=0; i<4; i++) {
				vertices[i].Pos.rotateYZBy(-90);
			}
			break;
		case 2: // X+
			if (drop[1])
				vertices[2].Pos.Y = 0.375*data->m_BS;
			if (drop[2])
				vertices[3].Pos.Y = 0.375*data->m_BS;
			for(u16 i=0; i<4; i++) {
				vertices[i].Pos.rotateXZBy(-90);
			}
			break;
		case 3: // Z-
			if (drop[2])
				vertices[2].Pos.Y = 0.375*data->m_BS;
			if (drop[3])
				vertices[3].Pos.Y = 0.375*data->m_BS;
			for(u16 i=0; i<4; i++) {
				vertices[i].Pos.rotateXZBy(180);
			}
			break;
		case 4: // Y-
			for(u16 i=0; i<4; i++) {
				vertices[i].Pos.rotateYZBy(90);
			}
			break;
		case 5: // X-
			if (drop[3])
				vertices[2].Pos.Y = 0.375*data->m_BS;
			if (drop[0])
				vertices[3].Pos.Y = 0.375*data->m_BS;
			for(u16 i=0; i<4; i++) {
				vertices[i].Pos.rotateXZBy(90);
			}
			break;
		default:;
		}

		u16 indices[] = {0,1,2,2,3,0};
		std::vector<u32> colours;
		if (selected.is_coloured) {
			meshgen_selected_lights(colours,f->vertex_alpha,4);
		}else{
			meshgen_lights(data,n,p,colours,f->vertex_alpha,g_6dirs[j],4,vertices);
		}

		for(u16 i=0; i<4; i++) {
			vertices[i].Pos += pos;
		}
		data->append(tiles[j], vertices, 4, indices, 6, colours);
	}
}

void meshgen_nodebox(MeshMakeData *data, v3s16 p, MapNode &n, SelectedNode &selected, bool has_meta)
{
	static const v3s16 tile_dirs[6] = {
		v3s16(0, 1, 0),
		v3s16(0, -1, 0),
		v3s16(1, 0, 0),
		v3s16(-1, 0, 0),
		v3s16(0, 0, 1),
		v3s16(0, 0, -1)
	};

	TileSpec tiles[6];
	NodeMetadata *meta = data->m_env->getMap().getNodeMetadataClone(p+data->m_blockpos_nodes);
	for (int i = 0; i < 6; i++) {
		// Handles facedir rotation for textures
		tiles[i] = getNodeTile(n,p,tile_dirs[i],selected,meta);
	}

	std::vector<NodeBox> boxes = content_features(n).getNodeBoxes(n);
	meshgen_build_nodebox(data,p,n,selected,boxes,tiles);
	if (!meta)
		return;
	if (!has_meta) {
		delete meta;
		return;
	}

	boxes = meta->getNodeBoxes(n);
	if (boxes.size() > 0) {

		boxes = transformNodeBox(n, boxes);

		for (int i = 0; i < 6; i++) {
			// Handles facedir rotation for textures
			tiles[i] = getMetaTile(n,p,tile_dirs[i],selected);
		}

		meshgen_build_nodebox(data,p,n,selected,boxes,tiles);
	}

	delete meta;
}

void meshgen_glasslike(MeshMakeData *data, v3s16 p, MapNode &n, SelectedNode &selected)
{
	static const v3s16 tile_dirs[6] = {
		v3s16(0, 1, 0),
		v3s16(0, -1, 0),
		v3s16(1, 0, 0),
		v3s16(-1, 0, 0),
		v3s16(0, 0, 1),
		v3s16(0, 0, -1)
	};

	TileSpec tiles[6];
	for (int i = 0; i < 6; i++) {
		// Handles facedir rotation for textures
		tiles[i] = getNodeTile(n,p,tile_dirs[i],selected);
	}

	v3f pos = intToFloat(p,BS);

	for (u32 j=0; j<6; j++) {
		// Check this neighbor
		v3s16 n2p = data->m_blockpos_nodes + p + g_6dirs[j];
		MapNode n2 = data->m_vmanip.getNodeRO(n2p);
		// Don't make face if neighbor is of same type
		if (n2.getContent() == n.getContent())
			continue;

		// The face at Z+
		video::S3DVertex vertices[4] = {
			video::S3DVertex(-0.5*data->m_BS,-0.5*data->m_BS,0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tiles[j].texture.x0(), tiles[j].texture.y1()),
			video::S3DVertex( 0.5*data->m_BS,-0.5*data->m_BS,0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tiles[j].texture.x1(), tiles[j].texture.y1()),
			video::S3DVertex( 0.5*data->m_BS, 0.5*data->m_BS,0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tiles[j].texture.x1(), tiles[j].texture.y0()),
			video::S3DVertex(-0.5*data->m_BS, 0.5*data->m_BS,0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tiles[j].texture.x0(), tiles[j].texture.y0()),
		};

		s16 yrot = 0;
		s16 xrot = 0;

		// Rotations in the g_6dirs format
		switch (j) {
		case 1: // Y+
			xrot = -90;
			break;
		case 2: // X+
			yrot = -90;
			break;
		case 3: // Z-
			yrot = 180;
			break;
		case 4: // Y-
			xrot = 90;
			break;
		case 5: // X-
			yrot = 90;
			break;
		default:;
		}

		if (yrot){
			for (u16 i=0; i<4; i++) {
				vertices[i].Pos.rotateXZBy(yrot);
			}
		}else if (xrot) {
			for (u16 i=0; i<4; i++) {
				vertices[i].Pos.rotateYZBy(xrot);
			}
		}

		u16 indices[] = {0,1,2,2,3,0};
		std::vector<u32> colours;
		if (selected.is_coloured) {
			meshgen_selected_lights(colours,255,4);
		}else{
			meshgen_lights(data,n,p,colours,255,g_6dirs[j],4,vertices);
		}

		for (u16 i=0; i<4; i++) {
			vertices[i].Pos += pos;
		}

		data->append(tiles[j], vertices, 4, indices, 6, colours);
	}

	if (data->mesh_detail > 2 && data->light_detail > 2 && content_features(n.getContent()).light_source > 0) {
		TileSpec tile;
		tile.texture = g_texturesource->getTexture("flare.png");
		tile.material_flags = 0;
		tile.material_type = MATERIAL_ALPHA_BLEND;
		s16 angle[4] = {
			45,
			-45,
			45,
			-45
		};
		for (u32 j=0; j<4; j++) {
			video::S3DVertex vertices[4] = {
				video::S3DVertex(-1.125*BS,-1.125*BS,0., 0,0,0, video::SColor(255,255,255,255), 0.,1.),
				video::S3DVertex( 1.125*BS,-1.125*BS,0., 0,0,0, video::SColor(255,255,255,255), 1.,1.),
				video::S3DVertex( 1.125*BS, 1.125*BS,0., 0,0,0, video::SColor(255,255,255,255), 1.,0.),
				video::S3DVertex(-1.125*BS, 1.125*BS,0., 0,0,0, video::SColor(255,255,255,255), 0.,0.)
			};

			for (u16 i=0; i<4; i++) {
				if (j > 1) {
					vertices[i].Pos.rotateYZBy(angle[j]);
				}else{
					vertices[i].Pos.rotateXZBy(angle[j]);
				}
				vertices[i].Pos += pos;
				vertices[i].TCoords *= tile.texture.size;
				vertices[i].TCoords += tile.texture.pos;
			}

			u16 indices[] = {0,1,2,2,3,0};
			std::vector<u32> colours;
			meshgen_custom_lights(colours,255,255,255,255,4);

			data->append(tile, vertices, 4, indices, 6, colours);
		}
	}
}

void meshgen_torchlike(MeshMakeData *data, v3s16 p, MapNode &n, SelectedNode &selected)
{
	static const f32 txc[24] = {
		0.625,0.125,0.75,0.25,
		0.625,0.625,0.625,0.75,
		0,0,0.125,1,
		0,0,0.125,1,
		0,0,0.125,1,
		0,0,0.125,1
	};
	static v3s16 faces[6] = {
		v3s16( 0, 1, 0),
		v3s16( 0,-1, 0),
		v3s16( 1, 0, 0),
		v3s16(-1, 0, 0),
		v3s16( 0, 0, 1),
		v3s16( 0, 0,-1),
	};
	v3f os(0,0,0);
	v3s16 dir = unpackDir(n.param2);
	video::S3DVertex vertices[6][4] = {
		{	// up
			video::S3DVertex(-0.0625*data->m_BS,0.125*data->m_BS,0.0625*data->m_BS, 0,1,0, video::SColor(255,255,255,255), txc[0],txc[1]),
			video::S3DVertex(0.0625*data->m_BS,0.125*data->m_BS,0.0625*data->m_BS, 0,1,0, video::SColor(255,255,255,255), txc[2],txc[1]),
			video::S3DVertex(0.0625*data->m_BS,0.125*data->m_BS,-0.0625*data->m_BS, 0,1,0, video::SColor(255,255,255,255), txc[2],txc[3]),
			video::S3DVertex(-0.0625*data->m_BS,0.125*data->m_BS,-0.0625*data->m_BS, 0,1,0, video::SColor(255,255,255,255), txc[0],txc[3])
		},{	// down
			video::S3DVertex(-0.0625*data->m_BS,-0.5*data->m_BS,-0.0625*data->m_BS, 0,-1,0, video::SColor(255,255,255,255), txc[4],txc[5]),
			video::S3DVertex(0.0625*data->m_BS,-0.5*data->m_BS,-0.0625*data->m_BS, 0,-1,0, video::SColor(255,255,255,255), txc[6],txc[5]),
			video::S3DVertex(0.0625*data->m_BS,-0.5*data->m_BS,0.0625*data->m_BS, 0,-1,0, video::SColor(255,255,255,255), txc[6],txc[7]),
			video::S3DVertex(-0.0625*data->m_BS,-0.5*data->m_BS,0.0625*data->m_BS, 0,-1,0, video::SColor(255,255,255,255), txc[4],txc[7])
		},{	// right
			video::S3DVertex(0.0625*data->m_BS,0.125*data->m_BS,-0.0625*data->m_BS, 1,0,0, video::SColor(255,255,255,255), txc[ 8],txc[9]),
			video::S3DVertex(0.0625*data->m_BS,0.125*data->m_BS,0.0625*data->m_BS, 1,0,0, video::SColor(255,255,255,255), txc[10],txc[9]),
			video::S3DVertex(0.0625*data->m_BS,-0.5*data->m_BS,0.0625*data->m_BS, 1,0,0, video::SColor(255,255,255,255), txc[10],txc[11]),
			video::S3DVertex(0.0625*data->m_BS,-0.5*data->m_BS,-0.0625*data->m_BS, 1,0,0, video::SColor(255,255,255,255), txc[ 8],txc[11])
		},{	// left
			video::S3DVertex(-0.0625*data->m_BS,0.125*data->m_BS,0.0625*data->m_BS, -1,0,0, video::SColor(255,255,255,255), txc[12],txc[13]),
			video::S3DVertex(-0.0625*data->m_BS,0.125*data->m_BS,-0.0625*data->m_BS, -1,0,0, video::SColor(255,255,255,255), txc[14],txc[13]),
			video::S3DVertex(-0.0625*data->m_BS,-0.5*data->m_BS,-0.0625*data->m_BS, -1,0,0, video::SColor(255,255,255,255), txc[14],txc[15]),
			video::S3DVertex(-0.0625*data->m_BS,-0.5*data->m_BS,0.0625*data->m_BS, -1,0,0, video::SColor(255,255,255,255), txc[12],txc[15])
		},{	// back
			video::S3DVertex(0.0625*data->m_BS,0.125*data->m_BS,0.0625*data->m_BS, 0,0,1, video::SColor(255,255,255,255), txc[16],txc[17]),
			video::S3DVertex(-0.0625*data->m_BS,0.125*data->m_BS,0.0625*data->m_BS, 0,0,1, video::SColor(255,255,255,255), txc[18],txc[17]),
			video::S3DVertex(-0.0625*data->m_BS,-0.5*data->m_BS,0.0625*data->m_BS, 0,0,1, video::SColor(255,255,255,255), txc[18],txc[19]),
			video::S3DVertex(0.0625*data->m_BS,-0.5*data->m_BS,0.0625*data->m_BS, 0,0,1, video::SColor(255,255,255,255), txc[16],txc[19])
		},{	// front
			video::S3DVertex(-0.0625*data->m_BS,0.125*data->m_BS,-0.0625*data->m_BS, 0,0,-1, video::SColor(255,255,255,255), txc[20],txc[21]),
			video::S3DVertex(0.0625*data->m_BS,0.125*data->m_BS,-0.0625*data->m_BS, 0,0,-1, video::SColor(255,255,255,255), txc[22],txc[21]),
			video::S3DVertex(0.0625*data->m_BS,-0.5*data->m_BS,-0.0625*data->m_BS, 0,0,-1, video::SColor(255,255,255,255), txc[22],txc[23]),
			video::S3DVertex(-0.0625*data->m_BS,-0.5*data->m_BS,-0.0625*data->m_BS, 0,0,-1, video::SColor(255,255,255,255), txc[20],txc[23])
		}
	};

	TileSpec tile = content_features(n).tiles[0];
	f32 sx = tile.texture.x1()-tile.texture.x0();
	f32 sy = tile.texture.y1()-tile.texture.y0();

	if (dir.Y == 1) { // roof
		os.Y = -0.125*BS;
		for (s32 i=0; i<6; i++) {
			for (int j=0; j<4; j++) {
				vertices[i][j].Pos.rotateXYBy(175);
				vertices[i][j].Pos.rotateYZBy(5);
				vertices[i][j].TCoords *= v2f(sx,sy);
				vertices[i][j].TCoords += v2f(
					tile.texture.x0(),
					tile.texture.y0()
				);
			}
		}
	}else if (dir.Y == -1) { // floor
		os.Y = 0.125*BS;
		for (s32 i=0; i<6; i++) {
			for (int j=0; j<4; j++) {
				vertices[i][j].TCoords *= v2f(sx,sy);
				vertices[i][j].TCoords += v2f(
					tile.texture.x0(),
					tile.texture.y0()
				);
			}
		}
	}else{ // wall
		os.Y = 0.375*BS;
		os.Z = 0.375*BS;
		if (dir.X == 1) {
			os.rotateXZBy(-90);
		}else if (dir.X == -1) {
			os.rotateXZBy(90);
		}else if (dir.Z == 1) {
			os.rotateXZBy(0);
		}else if (dir.Z == -1) {
			os.rotateXZBy(180);
		}
		for (s32 i=0; i<6; i++) {
			for (int j=0; j<4; j++) {
				vertices[i][j].Pos.Y += 0.25*BS;
				vertices[i][j].Pos.rotateYZBy(-5);
				vertices[i][j].Pos += v3f(0.,0.,0.4*BS);
				if (dir.X == 1) {
					vertices[i][j].Pos.rotateXZBy(-90);
				}else if (dir.X == -1) {
					vertices[i][j].Pos.rotateXZBy(90);
				}else if (dir.Z == -1) {
					vertices[i][j].Pos.rotateXZBy(180);
				}

				vertices[i][j].TCoords *= v2f(sx,sy);
				vertices[i][j].TCoords += v2f(
					tile.texture.x0(),
					tile.texture.y0()
				);
			}
		}
	}

	u16 indices[] = {0,1,2,2,3,0};


	v3f pos = intToFloat(p,BS);
	// Add to mesh collector
	for (s32 j=0; j<6; j++) {
		std::vector<u32> colours;
		if (selected.is_coloured) {
			meshgen_selected_lights(colours,255,4);
		}else{
			meshgen_lights(data,n,p,colours,255,faces[j],4,vertices[j]);
		}

		for (u16 i=0; i<4; i++) {
			vertices[j][i].Pos += pos;
		}

		data->append(tile, vertices[j], 4, indices, 6, colours);
	}

	if (data->mesh_detail > 2 && data->light_detail > 2) {
		tile.texture = g_texturesource->getTexture("flare.png");
		tile.material_flags = 0;
		tile.material_type = MATERIAL_ALPHA_BLEND;
		s16 angle[4] = {
			45,
			-45,
			45,
			-45
		};
		for (u32 j=0; j<4; j++) {
			video::S3DVertex vertices[4] = {
				video::S3DVertex(-0.5*BS,-0.5*BS,0., 0,0,0, video::SColor(255,255,255,255), 0.,1.),
				video::S3DVertex( 0.5*BS,-0.5*BS,0., 0,0,0, video::SColor(255,255,255,255), 1.,1.),
				video::S3DVertex( 0.5*BS, 0.5*BS,0., 0,0,0, video::SColor(255,255,255,255), 1.,0.),
				video::S3DVertex(-0.5*BS, 0.5*BS,0., 0,0,0, video::SColor(255,255,255,255), 0.,0.)
			};

			for (u16 i=0; i<4; i++) {
				if (j > 1) {
					vertices[i].Pos.rotateYZBy(angle[j]);
				}else{
					vertices[i].Pos.rotateXZBy(angle[j]);
				}
				vertices[i].Pos += pos+os;
				vertices[i].TCoords *= tile.texture.size;
				vertices[i].TCoords += tile.texture.pos;
			}

			u16 indices[] = {0,1,2,2,3,0};
			std::vector<u32> colours;
			meshgen_custom_lights(colours,255,255,255,255,4);

			data->append(tile, vertices, 4, indices, 6, colours);
		}
	}
}

void meshgen_fencelike(MeshMakeData *data, v3s16 p, MapNode &n, SelectedNode &selected)
{
	static const v3s16 tile_dirs[6] = {
		v3s16( 0, 1, 0),
		v3s16( 0,-1, 0),
		v3s16( 1, 0, 0),
		v3s16(-1, 0, 0),
		v3s16( 0, 0, 1),
		v3s16( 0, 0,-1)
	};
	static const v3s16 fence_dirs[8] = {
		v3s16(1,0,0),
		v3s16(-1,0,0),
		v3s16(0,0,1),
		v3s16(0,0,-1),
		v3s16(1,0,1),
		v3s16(1,0,-1),
		v3s16(-1,0,1),
		v3s16(-1,0,-1)
	};
	static const int showcheck[4][2] = {
		{0,2},
		{0,3},
		{1,2},
		{1,3}
	};
	static const int shown_angles[8] = {0,0,0,0,45,135,45,315};
	bool shown_dirs[8] = {false,false,false,false,false,false,false,false};
	n.param2 = 0;

	TileSpec tiles[6];
	for (int i = 0; i < 6; i++) {
		// Handles facedir rotation for textures
		tiles[i] = getNodeTile(n,p,tile_dirs[i],selected);
	}

	v3f pos = intToFloat(p, BS);
	std::vector<NodeBox> boxes = content_features(n).getNodeBoxes(n);
	int bi = 1;
	v3s16 p2 = p;
	p2.Y++;
	MapNode n2 = data->m_vmanip.getNodeRO(data->m_blockpos_nodes + p2);
	const ContentFeatures *f2 = &content_features(n2);
	aabb3f box;
	if (f2->draw_type == CDT_AIRLIKE || f2->draw_type == CDT_TORCHLIKE)
		bi = 0;
	{
		NodeBox box = boxes[bi];

		// Compute texture coords
		f32 tx1 = (box.m_box.MinEdge.X/data->m_BS)+0.5;
		f32 ty1 = (box.m_box.MinEdge.Y/data->m_BS)+0.5;
		f32 tz1 = (box.m_box.MinEdge.Z/data->m_BS)+0.5;
		f32 tx2 = (box.m_box.MaxEdge.X/data->m_BS)+0.5;
		f32 ty2 = (box.m_box.MaxEdge.Y/data->m_BS)+0.5;
		f32 tz2 = (box.m_box.MaxEdge.Z/data->m_BS)+0.5;
		f32 txc[24] = {
			// up
			tx1, 1-tz2, tx2, 1-tz1,
			// down
			tx1, tz1, tx2, tz2,
			// right
			tz1, 1-ty2, tz2, 1-ty1,
			// left
			1-tz2, 1-ty2, 1-tz1, 1-ty1,
			// back
			1-tx2, 1-ty2, 1-tx1, 1-ty1,
			// front
			tx1, 1-ty2, tx2, 1-ty1,
		};
		meshgen_cuboid(data,n,p, pos, box.m_box, tiles, 6,  selected, txc, v3s16(0,0,0),v3f(0,0,0));
	}

	int bps = ((boxes.size()-2)/4); // boxes per section
	u8 np = 1;

	for (int k=0; k<8; k++) {
		if (k > 3 && (shown_dirs[showcheck[k-4][0]] || shown_dirs[showcheck[k-4][1]]))
					continue;
		p2 = data->m_blockpos_nodes+p+fence_dirs[k];
		n2 = data->m_vmanip.getNodeRO(p2);
		f2 = &content_features(n2);
		if (
			f2->draw_type == CDT_FENCELIKE
			|| f2->draw_type == CDT_WALLLIKE
			|| n2.getContent() == CONTENT_WOOD_GATE
			|| n2.getContent() == CONTENT_WOOD_GATE_OPEN
			|| n2.getContent() == CONTENT_IRON_GATE
			|| n2.getContent() == CONTENT_IRON_GATE_OPEN
			|| (
				n2.getContent() != CONTENT_IGNORE
				&& n2.getContent() == content_features(n).special_alternate_node
			)
		) {
			shown_dirs[k] = true;
			n.param2 |= (np<<k);
			for (int i=0; i<bps; i++) {
				NodeBox box = boxes[i+2+(bps*(k%4))];

				// Compute texture coords
				f32 tx1 = (box.m_box.MinEdge.X/data->m_BS)+0.5;
				f32 ty1 = (box.m_box.MinEdge.Y/data->m_BS)+0.5;
				f32 tz1 = (box.m_box.MinEdge.Z/data->m_BS)+0.5;
				f32 tx2 = (box.m_box.MaxEdge.X/data->m_BS)+0.5;
				f32 ty2 = (box.m_box.MaxEdge.Y/data->m_BS)+0.5;
				f32 tz2 = (box.m_box.MaxEdge.Z/data->m_BS)+0.5;
				f32 txc[24] = {
					// up
					tx1, 1-tz2, tx2, 1-tz1,
					// down
					tx1, tz1, tx2, tz2,
					// right
					tz1, 1-ty2, tz2, 1-ty1,
					// left
					1-tz2, 1-ty2, 1-tz1, 1-ty1,
					// back
					1-tx2, 1-ty2, 1-tx1, 1-ty1,
					// front
					tx1, 1-ty2, tx2, 1-ty1,
				};
				if (k > 3) {
					switch (k) {
					case 4:
						box.m_box.MaxEdge.X *= 1.414;
						break;
					case 5:
						box.m_box.MinEdge.X *= 1.414;
						break;
					case 6:
						box.m_box.MaxEdge.Z *= 1.414;
						break;
					case 7:
						box.m_box.MinEdge.Z *= 1.414;
						break;
					default:;
					}
				}
				meshgen_cuboid(data,n,p, pos, box.m_box, tiles, 6,  selected, txc, v3s16(0,shown_angles[k],0),v3f(0,0,0));
			}
		}
	}
}

void meshgen_firelike(MeshMakeData *data, v3s16 p, MapNode &n, SelectedNode &selected)
{
	TileSpec tile = getNodeTile(n,p,v3s16(0,1,0),selected);
	content_t current = n.getContent();
	content_t n2c;
	MapNode n2;
	v3s16 n2p;
	static const v3s16 dirs[6] = {
		v3s16( 0, 1, 0),
		v3s16( 0,-1, 0),
		v3s16( 1, 0, 0),
		v3s16(-1, 0, 0),
		v3s16( 0, 0, 1),
		v3s16( 0, 0,-1)
	};
	int doDraw[6] = {0,0,0,0,0,0};
	int i;
	// Draw the full flame even if there are no surrounding nodes
	bool drawAllFaces = true;
	// Check for adjacent nodes
	for (i = 0; i < 6; i++) {
		n2p = data->m_blockpos_nodes + p + dirs[i];
		n2 = data->m_vmanip.getNodeRO(n2p);
		n2c = n2.getContent();
		if (n2c != CONTENT_IGNORE && n2c != CONTENT_AIR && n2c != current) {
			doDraw[i] = 1;
			drawAllFaces = false;
		}
	}
	v3f pos = intToFloat(p,BS);
	for (u32 j=0; j<4; j++) {
		video::S3DVertex vertices[4] = {
			video::S3DVertex(-0.5*data->m_BS,-0.5*data->m_BS,0.369*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tile.texture.x0(), tile.texture.y1()),
			video::S3DVertex( 0.5*data->m_BS,-0.5*data->m_BS,0.369*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tile.texture.x1(), tile.texture.y1()),
			video::S3DVertex( 0.5*data->m_BS, 0.5*data->m_BS,0.369*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tile.texture.x1(), tile.texture.y0()),
			video::S3DVertex(-0.5*data->m_BS, 0.5*data->m_BS,0.369*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tile.texture.x0(), tile.texture.y0())
		};
		int vOffset = 1; // Vertical offset of faces after rotation
		// Calculate which faces should be drawn
		if(j == 0 && (drawAllFaces || (doDraw[3] == 1 || doDraw[1] == 1))) {
			for(u16 i=0; i<4; i++) {
				vertices[i].Pos.rotateXZBy(90);
				vertices[i].Pos.rotateXYBy(-15);
				vertices[i].Pos.Y -= vOffset;
			}
		}else if(j == 1 && (drawAllFaces || (doDraw[5] == 1 || doDraw[1] == 1))) {
			for(u16 i=0; i<4; i++) {
				vertices[i].Pos.rotateXZBy(180);
				vertices[i].Pos.rotateYZBy(15);
				vertices[i].Pos.Y -= vOffset;
			}
		}else if(j == 2 && (drawAllFaces || (doDraw[2] == 1 || doDraw[1] == 1))) {
			for(u16 i=0; i<4; i++) {
				vertices[i].Pos.rotateXZBy(270);
				vertices[i].Pos.rotateXYBy(15);
				vertices[i].Pos.Y -= vOffset;
			}
		}else if(j == 3 && (drawAllFaces || (doDraw[4] == 1 || doDraw[1] == 1))) {
			for(u16 i=0; i<4; i++) {
				vertices[i].Pos.rotateYZBy(-15);
				vertices[i].Pos.Y -= vOffset;
			}
		}else if(j == 3 && (drawAllFaces || (doDraw[0] == 1 && doDraw[1] == 0))) {
			for(u16 i=0; i<4; i++) {
				vertices[i].Pos.rotateYZBy(-90);
				vertices[i].Pos.Y += vOffset;
			}
		}else{
			// Skip faces that aren't adjacent to a node
			continue;
		}
		u16 indices[] = {0,1,2,2,3,0};
		std::vector<u32> colours;
		if (selected.is_coloured) {
			meshgen_selected_lights(colours,255,4);
		}else{
			meshgen_lights(data,n,p,colours,255,v3s16(0,0,0),4,vertices);
		}

		for (u16 i=0; i<4; i++) {
			vertices[i].Pos += pos;
		}

		data->append(tile, vertices, 4, indices, 6, colours);
	}
}

void meshgen_walllike(MeshMakeData *data, v3s16 p, MapNode &n, SelectedNode &selected)
{
	static const v3s16 tile_dirs[6] = {
		v3s16(0, 1, 0),
		v3s16(0, -1, 0),
		v3s16(1, 0, 0),
		v3s16(-1, 0, 0),
		v3s16(0, 0, 1),
		v3s16(0, 0, -1)
	};
	static const int shown_angles[8] = {0,0,0,0,45,135,45,315};
	n.param2 = 0;

	TileSpec tiles[6];
	for (int i = 0; i < 6; i++) {
		// Handles facedir rotation for textures
		tiles[i] = getNodeTile(n,p,tile_dirs[i],selected);
	}

	v3f pos = intToFloat(p, BS);
	std::vector<NodeBox> boxes = content_features(n).getNodeBoxes(n);
	v3s16 p2 = p;
	p2.Y++;
	NodeBox box;
	u8 d[8];
	int bi = meshgen_check_walllike(data,n,p+data->m_blockpos_nodes,d);
	{
		box = boxes[bi];

		// Compute texture coords
		f32 tx1 = (box.m_box.MinEdge.X/data->m_BS)+0.5;
		f32 ty1 = (box.m_box.MinEdge.Y/data->m_BS)+0.5;
		f32 tz1 = (box.m_box.MinEdge.Z/data->m_BS)+0.5;
		f32 tx2 = (box.m_box.MaxEdge.X/data->m_BS)+0.5;
		f32 ty2 = (box.m_box.MaxEdge.Y/data->m_BS)+0.5;
		f32 tz2 = (box.m_box.MaxEdge.Z/data->m_BS)+0.5;
		f32 txc[24] = {
			// up
			tx1, 1-tz2, tx2, 1-tz1,
			// down
			tx1, tz1, tx2, tz2,
			// right
			tz1, 1-ty2, tz2, 1-ty1,
			// left
			1-tz2, 1-ty2, 1-tz1, 1-ty1,
			// back
			1-tx2, 1-ty2, 1-tx1, 1-ty1,
			// front
			tx1, 1-ty2, tx2, 1-ty1,
		};
		meshgen_cuboid(data,n,p,pos,box.m_box,tiles,6,selected,txc,v3s16(0,0,0),v3f(0,0,0));
	}

	int bps = ((boxes.size()-3)/4); // boxes per section
	u8 np = 1;

	for (int k=0; k<8; k++) {
		if (d[k]) {
			n.param2 |= (np<<k);
			for (int i=0; i<bps; i++) {
				box = boxes[i+3+(bps*(k%4))];

				// Compute texture coords
				f32 tx1 = (box.m_box.MinEdge.X/data->m_BS)+0.5;
				f32 ty1 = (box.m_box.MinEdge.Y/data->m_BS)+0.5;
				f32 tz1 = (box.m_box.MinEdge.Z/data->m_BS)+0.5;
				f32 tx2 = (box.m_box.MaxEdge.X/data->m_BS)+0.5;
				f32 ty2 = (box.m_box.MaxEdge.Y/data->m_BS)+0.5;
				f32 tz2 = (box.m_box.MaxEdge.Z/data->m_BS)+0.5;
				f32 txc[24] = {
					// up
					tx1, 1-tz2, tx2, 1-tz1,
					// down
					tx1, tz1, tx2, tz2,
					// right
					tz1, 1-ty2, tz2, 1-ty1,
					// left
					1-tz2, 1-ty2, 1-tz1, 1-ty1,
					// back
					1-tx2, 1-ty2, 1-tx1, 1-ty1,
					// front
					tx1, 1-ty2, tx2, 1-ty1,
				};
				if (k > 3) {
					switch (k) {
					case 4:
						box.m_box.MaxEdge.X *= 1.414;
						break;
					case 5:
						box.m_box.MinEdge.X *= 1.414;
						break;
					case 6:
						box.m_box.MaxEdge.Z *= 1.414;
						break;
					case 7:
						box.m_box.MinEdge.Z *= 1.414;
						break;
					default:;
					}
				}
				meshgen_cuboid(data,n,p, pos, box.m_box, tiles, 6,  selected, txc, v3s16(0,shown_angles[k],0),v3f(0,0,0));
			}
		}
	}
}

void meshgen_rooflike(MeshMakeData *data, v3s16 p, MapNode &n, SelectedNode &selected)
{
	bool is_roof_x [] = { false, false };  /* x-1, x+1 */
	bool is_roof_z [] = { false, false };  /* z-1, z+1 */

	bool is_roof_z_minus_y [] = { false, false };  /* z-1, z+1; y-1 */
	bool is_roof_x_minus_y [] = { false, false };  /* x-1, z+1; y-1 */
	bool is_roof_z_plus_y [] = { false, false };  /* z-1, z+1; y+1 */
	bool is_roof_x_plus_y [] = { false, false };  /* x-1, x+1; y+1 */

	MapNode n_minus_x = data->m_vmanip.getNodeRO(data->m_blockpos_nodes + p + v3s16(-1,0,0));
	MapNode n_plus_x = data->m_vmanip.getNodeRO(data->m_blockpos_nodes + p + v3s16(1,0,0));
	MapNode n_minus_z = data->m_vmanip.getNodeRO(data->m_blockpos_nodes + p + v3s16(0,0,-1));
	MapNode n_plus_z = data->m_vmanip.getNodeRO(data->m_blockpos_nodes + p + v3s16(0,0,1));
	MapNode n_plus_x_plus_y = data->m_vmanip.getNodeRO(data->m_blockpos_nodes + p + v3s16(1, 1, 0));
	MapNode n_plus_x_minus_y = data->m_vmanip.getNodeRO(data->m_blockpos_nodes + p + v3s16(1, -1, 0));
	MapNode n_minus_x_plus_y = data->m_vmanip.getNodeRO(data->m_blockpos_nodes + p + v3s16(-1, 1, 0));
	MapNode n_minus_x_minus_y = data->m_vmanip.getNodeRO(data->m_blockpos_nodes + p + v3s16(-1, -1, 0));
	MapNode n_plus_z_plus_y = data->m_vmanip.getNodeRO(data->m_blockpos_nodes + p + v3s16(0, 1, 1));
	MapNode n_minus_z_plus_y = data->m_vmanip.getNodeRO(data->m_blockpos_nodes + p + v3s16(0, 1, -1));
	MapNode n_plus_z_minus_y = data->m_vmanip.getNodeRO(data->m_blockpos_nodes + p + v3s16(0, -1, 1));
	MapNode n_minus_z_minus_y = data->m_vmanip.getNodeRO(data->m_blockpos_nodes + p + v3s16(0, -1, -1));

	if (content_features(n_minus_x).draw_type == CDT_ROOFLIKE)
		is_roof_x[0] = true;
	if (content_features(n_minus_x_minus_y).draw_type == CDT_ROOFLIKE)
		is_roof_x_minus_y[0] = true;
	if (content_features(n_minus_x_plus_y).draw_type == CDT_ROOFLIKE)
		is_roof_x_plus_y[0] = true;
	if (content_features(n_plus_x).draw_type == CDT_ROOFLIKE)
		is_roof_x[1] = true;
	if (content_features(n_plus_x_minus_y).draw_type == CDT_ROOFLIKE)
		is_roof_x_minus_y[1] = true;
	if (content_features(n_plus_x_plus_y).draw_type == CDT_ROOFLIKE)
		is_roof_x_plus_y[1] = true;
	if (content_features(n_minus_z).draw_type == CDT_ROOFLIKE)
		is_roof_z[0] = true;
	if (content_features(n_minus_z_minus_y).draw_type == CDT_ROOFLIKE)
		is_roof_z_minus_y[0] = true;
	if (content_features(n_minus_z_plus_y).draw_type == CDT_ROOFLIKE)
		is_roof_z_plus_y[0] = true;
	if (content_features(n_plus_z).draw_type == CDT_ROOFLIKE)
		is_roof_z[1] = true;
	if (content_features(n_plus_z_minus_y).draw_type == CDT_ROOFLIKE)
		is_roof_z_minus_y[1] = true;
	if (content_features(n_plus_z_plus_y).draw_type == CDT_ROOFLIKE)
		is_roof_z_plus_y[1] = true;

	u8 adjacencies = is_roof_x[0] + is_roof_x[1] + is_roof_z[0] + is_roof_z[1];

	// get the tile, with crack if being dug
	TileSpec tile = getNodeTile(n,p,v3s16(0,1,0),selected);
	TileSpec toptile = getNodeTile(n,p,v3s16(0,-1,0),selected);

	u8 type = 0;
	s16 angle = 0;

	MapNode abv;

	v3f pos = intToFloat(p, BS);

	if (adjacencies == 1) {
		// cross X
		if (is_roof_x[0] || is_roof_x[1]) {
			if (is_roof_z_plus_y[0]) {
				type = 0;
				angle = 180;
			}else if (is_roof_z_plus_y[1]) {
				type = 0;
				angle = 0;
			}else if (is_roof_x[0] && is_roof_x_minus_y[1]) {
				type = 9;
				angle = 0;
			}else if (is_roof_x[1] && is_roof_x_minus_y[0]) {
				type = 9;
				angle = 180;
			}else{
				type = 1;
				angle = 0;
			}
		}
		// cross Z
		else if (is_roof_z[0] || is_roof_z[1]) {
			if (is_roof_x_plus_y[1]) {
				type = 0;
				angle = 270;
			}else if (is_roof_x_plus_y[0]) {
				type = 0;
				angle = 90;
			}else if (is_roof_z[0] && is_roof_z_minus_y[1]) {
				type = 9;
				angle = 90;
			}else if (is_roof_z[1] && is_roof_z_minus_y[0]) {
				type = 9;
				angle = 270;
			}else{
				type = 1;
				angle = 90;
			}
		}
	}else if (adjacencies == 2) {
		// cross X
		if (is_roof_x[0] && is_roof_x[1]) {
			if (is_roof_z_plus_y[0]) {
				type = 0;
				angle = 180;
			}else if (is_roof_z_plus_y[1]) {
				type = 0;
				angle = 0;
			}else{
				type = 1;
				angle = 0;
			}
		}
		// cross Z
		else if (is_roof_z[0] && is_roof_z[1]) {
			if (is_roof_x_plus_y[1]) {
				type = 0;
				angle = 270;
			}else if (is_roof_x_plus_y[0]) {
				type = 0;
				angle = 90;
			}else{
				type = 1;
				angle = 90;
			}
		}else if (is_roof_x[0] && is_roof_z[0]) {
			if (is_roof_x_plus_y[1] && is_roof_z_plus_y[1]) {
				type = 7;
				angle = 90;
			}else if (is_roof_x_plus_y[1]) {
				type = 2;
				angle = 0;
			}else if (is_roof_z_plus_y[1]) {
				type = 2;
				angle = 90;
			}else{
				abv = data->m_vmanip.getNodeRO(data->m_blockpos_nodes + p + v3s16(-1, 1,-1));
				if (content_features(abv).draw_type == CDT_ROOFLIKE) {
					type = 4;
					angle = 90;
				}else{
					type = 3;
					angle = 0;
				}
			}
		}else if (is_roof_x[0] && is_roof_z[1]) {
			if (is_roof_x_plus_y[1] && is_roof_z_plus_y[0]) {
				type = 7;
				angle = 0;
			}else if (is_roof_x_plus_y[1]) {
				type = 2;
				angle = 0;
			}else if (is_roof_z_plus_y[0]) {
				type = 2;
				angle = 270;
			}else{
				abv = data->m_vmanip.getNodeRO(data->m_blockpos_nodes + p + v3s16(-1, 1, 1));
				if (content_features(abv).draw_type == CDT_ROOFLIKE) {
					type = 4;
					angle = 0;
				}else{
					type = 3;
					angle = 270;
				}
			}
		}else if (is_roof_x[1] && is_roof_z[0]) {
			if (is_roof_x_plus_y[0] && is_roof_z_plus_y[1]) {
				type = 7;
				angle = 180;
			}else if (is_roof_x_plus_y[0]) {
				type = 2;
				angle = 180;
			}else if (is_roof_z_plus_y[1]) {
				type = 2;
				angle = 90;
			}else{
				abv = data->m_vmanip.getNodeRO(data->m_blockpos_nodes + p + v3s16( 1, 1,-1));
				if (content_features(abv).draw_type == CDT_ROOFLIKE) {
					type = 4;
					angle = 180;
				}else{
					type = 3;
					angle = 90;
				}
			}
		}else if (is_roof_x[1] && is_roof_z[1]) {
			if (is_roof_x_plus_y[0] && is_roof_z_plus_y[0]) {
				type = 7;
				angle = 270;
			}else if (is_roof_x_plus_y[0]) {
				type = 2;
				angle = 180;
			}else if (is_roof_z_plus_y[0]) {
				type = 2;
				angle = 270;
			}else{
				abv = data->m_vmanip.getNodeRO(data->m_blockpos_nodes + p + v3s16( 1, 1, 1));
				if (content_features(abv).draw_type == CDT_ROOFLIKE) {
					type = 4;
					angle = 270;
				}else{
					type = 3;
					angle = 180;
				}
			}
		}
	}else if (adjacencies == 3) {
		if (is_roof_x[0] && is_roof_x[1] && is_roof_z[0]) {
			if (is_roof_z_plus_y[1]) {
				type = 2;
				angle = 90;
			}else{
				type = 6;
				angle = 0;
			}
		}else if (is_roof_x[0] && is_roof_x[1] && is_roof_z[1]) {
			if (is_roof_z_plus_y[0]) {
				type = 2;
				angle = 270;
			}else{
				type = 6;
				angle = 180;
			}
		}else if (is_roof_x[0] && is_roof_z[0] && is_roof_z[1]) {
			if (is_roof_x_plus_y[1]) {
				type = 2;
				angle = 0;
			}else{
				type = 6;
				angle = 270;
			}
		}else if (is_roof_x[1] && is_roof_z[0] && is_roof_z[1]) {
			if (is_roof_x_plus_y[0]) {
				type = 2;
				angle = 180;
			}else{
				type = 6;
				angle = 90;
			}
		}
	}else if (adjacencies == 4) {
		type = 5;
		angle = 0;
	}else{
		if (is_roof_z_plus_y[0]) {
			type = 0;
			angle = 180;
		}else if (is_roof_z_plus_y[1]) {
			type = 0;
		}else if (is_roof_x_plus_y[1]) {
			type = 0;
			angle = 270;
		}else if (is_roof_x_plus_y[0]) {
			type = 0;
			angle = 90;
		}else{
			type = 8;
			angle = 0;
		}
	}

	v3s16 face(0,1,0);
	/*
		0: slope
		1: top
		2: top butting to slope
		3: top corner
		4: outer corner
		5: top X
		6: top T
		7: inner corner
		8: top cap
		9: top end cap
	*/
	switch (type) {
	case 0:
	{
		v3f cnr[2][3];
		if (angle == 0) {
			cnr[0][0] = v3f(-0.5,-0.5,-0.5);
			cnr[0][1] = v3f(0.5,-0.5,-0.5);
			cnr[0][2] = v3f(0.5,0.5,0.5);
			cnr[1][0] = v3f(0.5,0.5,0.5);
			cnr[1][1] = v3f(-0.5,0.5,0.5);
			cnr[1][2] = v3f(-0.5,-0.5,-0.5);
			face = v3s16(0,1,-1);
		}else if (angle == 90) {
			cnr[0][0] = v3f(-0.5,0.5,-0.5);
			cnr[0][1] = v3f(0.5,-0.5,-0.5);
			cnr[0][2] = v3f(0.5,-0.5,0.5);
			cnr[1][0] = v3f(0.5,-0.5,0.5);
			cnr[1][1] = v3f(-0.5,0.5,0.5);
			cnr[1][2] = v3f(-0.5,0.5,-0.5);
			face = v3s16(1,1,0);
		}else if (angle == 180) {
			cnr[0][0] = v3f(-0.5,0.5,-0.5);
			cnr[0][1] = v3f(0.5,0.5,-0.5);
			cnr[0][2] = v3f(0.5,-0.5,0.5);
			cnr[1][0] = v3f(0.5,-0.5,0.5);
			cnr[1][1] = v3f(-0.5,-0.5,0.5);
			cnr[1][2] = v3f(-0.5,0.5,-0.5);
			face = v3s16(0,1,1);
		}else if (angle == 270) {
			cnr[0][0] = v3f(-0.5,-0.5,-0.5);
			cnr[0][1] = v3f(0.5,0.5,-0.5);
			cnr[0][2] = v3f(0.5,0.5,0.5);
			cnr[1][0] = v3f(0.5,0.5,0.5);
			cnr[1][1] = v3f(-0.5,-0.5,0.5);
			cnr[1][2] = v3f(-0.5,-0.5,-0.5);
			face = v3s16(-1,1,0);
		}
		s16 a = 180-angle;
		if (a < 0)
			a += 360;
		for (int s=0; s<2; s++) {
			meshgen_rooftri(data,n,p,cnr[s],pos,tile,selected,a,face);
		}
	}
	break;
	case 1:
	{
		v3f cnr[4][3];
		if (angle == 0 || angle == 180) {
			cnr[0][0] = v3f(-0.5,-0.5,-0.5);
			cnr[0][1] = v3f(0.5,-0.5,-0.5);
			cnr[0][2] = v3f(0.5,0.,0.);
			cnr[1][0] = v3f(0.5,0.,0.);
			cnr[1][1] = v3f(-0.5,0.,0.);
			cnr[1][2] = v3f(-0.5,-0.5,-0.5);
			cnr[2][0] = v3f(-0.5,0.,0.);
			cnr[2][1] = v3f(0.5,0.,0.);
			cnr[2][2] = v3f(0.5,-0.5,0.5);
			cnr[3][0] = v3f(0.5,-0.5,0.5);
			cnr[3][1] = v3f(-0.5,-0.5,0.5);
			cnr[3][2] = v3f(-0.5,0.,0.);
		}else if (angle == 90 || angle == 270) {
			cnr[0][0] = v3f(-0.5,-0.5,-0.5);
			cnr[0][1] = v3f(-0.5,-0.5,0.5);
			cnr[0][2] = v3f(0.,0.,0.5);
			cnr[1][0] = v3f(0.,0.,0.5);
			cnr[1][1] = v3f(0.,0.,-0.5);
			cnr[1][2] = v3f(-0.5,-0.5,-0.5);
			cnr[2][0] = v3f(0.,0.,-0.5);
			cnr[2][1] = v3f(0.,0.,0.5);
			cnr[2][2] = v3f(0.5,-0.5,0.5);
			cnr[3][0] = v3f(0.5,-0.5,0.5);
			cnr[3][1] = v3f(0.5,-0.5,-0.5);
			cnr[3][2] = v3f(0.,0.,-0.5);
		}
		s16 a = angle;
		if (a < 180)
			a += 180;
		for (int s=0; s<4; s++) {
			if (s == 2)
				a -= 180;
			meshgen_rooftri(data,n,p,cnr[s],pos,toptile,selected,a,face);
		}
	}
	break;
	case 2:
	{
		v3f cnr[2][3];
		if (angle == 90) {
			cnr[0][0] = v3f(-0.5,-0.5,-0.5);
			cnr[0][1] = v3f(0.5,-0.5,-0.5);
			cnr[0][2] = v3f(0.5,0.5,0.5);
			cnr[1][0] = v3f(0.5,0.5,0.5);
			cnr[1][1] = v3f(-0.5,0.5,0.5);
			cnr[1][2] = v3f(-0.5,-0.5,-0.5);
			face = v3s16(0,1,-1);
		}else if (angle == 180) {
			cnr[0][0] = v3f(-0.5,0.5,-0.5);
			cnr[0][1] = v3f(0.5,-0.5,-0.5);
			cnr[0][2] = v3f(0.5,-0.5,0.5);
			cnr[1][0] = v3f(0.5,-0.5,0.5);
			cnr[1][1] = v3f(-0.5,0.5,0.5);
			cnr[1][2] = v3f(-0.5,0.5,-0.5);
			face = v3s16(1,1,0);
		}else if (angle == 270) {
			cnr[0][0] = v3f(-0.5,0.5,-0.5);
			cnr[0][1] = v3f(0.5,0.5,-0.5);
			cnr[0][2] = v3f(0.5,-0.5,0.5);
			cnr[1][0] = v3f(0.5,-0.5,0.5);
			cnr[1][1] = v3f(-0.5,-0.5,0.5);
			cnr[1][2] = v3f(-0.5,0.5,-0.5);
			face = v3s16(0,1,1);
		}else if (angle == 0) {
			cnr[0][0] = v3f(-0.5,-0.5,-0.5);
			cnr[0][1] = v3f(0.5,0.5,-0.5);
			cnr[0][2] = v3f(0.5,0.5,0.5);
			cnr[1][0] = v3f(0.5,0.5,0.5);
			cnr[1][1] = v3f(-0.5,-0.5,0.5);
			cnr[1][2] = v3f(-0.5,-0.5,-0.5);
			face = v3s16(-1,1,0);
		}
		s16 a = 270-angle;
		if (a < 0)
			a += 360;
		for (int s=0; s<2; s++) {
			meshgen_rooftri(data,n,p,cnr[s],pos,tile,selected,a,face);
		}
	}
	{
		v3f cnr[2][3];
		if (angle == 0) {
			cnr[0][0] = v3f(0.,0.,0.);
			cnr[0][1] = v3f(-0.5,0.,0.);
			cnr[0][2] = v3f(-0.5,-0.5,-0.5);
			cnr[1][0] = v3f(0.,0.,0.);
			cnr[1][1] = v3f(-0.5,-0.5,0.5);
			cnr[1][2] = v3f(-0.5,0.,0.);
			face = v3s16(-1,1,0);
		}else if (angle == 90) {
			cnr[0][0] = v3f(0.,0.,0.);
			cnr[0][1] = v3f(0.,0.,-0.5);
			cnr[0][2] = v3f(-0.5,-0.5,-0.5);
			cnr[1][0] = v3f(0.,0.,0.);
			cnr[1][1] = v3f(0.5,-0.5,-0.5);
			cnr[1][2] = v3f(0.,0.,-0.5);
			face = v3s16(0,1,-1);
		}else if (angle == 180) {
			cnr[0][0] = v3f(0.,0.,0.);
			cnr[0][1] = v3f(0.5,0.,0.);
			cnr[0][2] = v3f(0.5,-0.5,-0.5);
			cnr[1][0] = v3f(0.,0.,0.);
			cnr[1][1] = v3f(0.5,-0.5,0.5);
			cnr[1][2] = v3f(0.5,0.,0.);
			face = v3s16(1,1,0);
		}else if (angle == 270) {
			cnr[0][0] = v3f(0.,0.,0.);
			cnr[0][1] = v3f(0.,0.,0.5);
			cnr[0][2] = v3f(-0.5,-0.5,0.5);
			cnr[1][0] = v3f(0.,0.,0.);
			cnr[1][1] = v3f(0.5,-0.5,0.5);
			cnr[1][2] = v3f(0.,0.,0.5);
			face = v3s16(0,1,1);
		}
		s16 a = angle;
		if (a < 180)
			a += 180;
		for (int s=0; s<2; s++) {
			meshgen_rooftri(data,n,p,cnr[s],pos,toptile,selected,a,face);
			a -= 180;
		}
	}
	break;
	case 3:
	{
		v3f cnr[2][3];
		s16 a1 = angle;
		s16 a2 = angle + 90;
		if (angle == 0) {
			cnr[0][0] = v3f(0.,0.,0.);
			cnr[0][1] = v3f(-0.5,0.,0.);
			cnr[0][2] = v3f(-0.5,-0.5,-0.5);
			cnr[1][0] = v3f(0.,0.,0.);
			cnr[1][1] = v3f(0.,0.,-0.5);
			cnr[1][2] = v3f(-0.5,-0.5,-0.5);
			a1 = 180;
			a2 = 270;
		}else if (angle == 90) {
			cnr[0][0] = v3f(0.,0.,0.);
			cnr[0][1] = v3f(0.,0.,-0.5);
			cnr[0][2] = v3f(0.5,-0.5,-0.5);
			cnr[1][0] = v3f(0.,0.,0.);
			cnr[1][1] = v3f(0.5,0.,0.);
			cnr[1][2] = v3f(0.5,-0.5,-0.5);
		}else if (angle == 180) {
			cnr[0][0] = v3f(0.,0.,0.);
			cnr[0][1] = v3f(0.,0.,0.5);
			cnr[0][2] = v3f(0.5,-0.5,0.5);
			cnr[1][0] = v3f(0.,0.,0.);
			cnr[1][1] = v3f(0.5,0.,0.);
			cnr[1][2] = v3f(0.5,-0.5,0.5);
			a1 = 90;
			a2 = 0;
		}else if (angle == 270) {
			cnr[0][0] = v3f(0.,0.,0.);
			cnr[0][1] = v3f(0.,0.,0.5);
			cnr[0][2] = v3f(-0.5,-0.5,0.5);
			cnr[1][0] = v3f(0.,0.,0.);
			cnr[1][1] = v3f(-0.5,0.,0.);
			cnr[1][2] = v3f(-0.5,-0.5,0.5);
		}
		s16 a = a1;
		for (int s=0; s<2; s++) {
			meshgen_rooftri(data,n,p,cnr[s],pos,toptile,selected,a,face);
			a = a2;
		}
	}
	{
		v3f cnr[4][3];
		s16 a1 = angle;
		s16 a2 = angle + 90;
		if (angle == 0) {
			cnr[0][0] = v3f(0.,0.,0.);
			cnr[0][1] = v3f(-0.5,-0.5,0.5);
			cnr[0][2] = v3f(-0.5,0.,0.);
			cnr[1][0] = v3f(0.,0.,0.);
			cnr[1][1] = v3f(0.5,-0.5,0.5);
			cnr[1][2] = v3f(-0.5,-0.5,0.5);
			cnr[2][0] = v3f(0.,0.,0.);
			cnr[2][1] = v3f(0.,0.,-0.5);
			cnr[2][2] = v3f(0.5,-0.5,-0.5);
			cnr[3][0] = v3f(0.,0.,0.);
			cnr[3][1] = v3f(0.5,-0.5,-0.5);
			cnr[3][2] = v3f(0.5,-0.5,0.5);
		}else if (angle == 90) {
			cnr[0][0] = v3f(0.,0.,0.);
			cnr[0][1] = v3f(-0.5,-0.5,-0.5);
			cnr[0][2] = v3f(0.,0.,-0.5);
			cnr[1][0] = v3f(0.,0.,0.);
			cnr[1][1] = v3f(-0.5,-0.5,0.5);
			cnr[1][2] = v3f(-0.5,-0.5,-0.5);
			cnr[2][0] = v3f(0.,0.,0.);
			cnr[2][1] = v3f(0.5,0.,0.);
			cnr[2][2] = v3f(0.5,-0.5,0.5);
			cnr[3][0] = v3f(0.,0.,0.);
			cnr[3][1] = v3f(0.5,-0.5,0.5);
			cnr[3][2] = v3f(-0.5,-0.5,0.5);
			a1 = 270;
			a2 = 0;
		}else if (angle == 180) {
			cnr[0][0] = v3f(0.,0.,0.);
			cnr[0][1] = v3f(0.5,-0.5,-0.5);
			cnr[0][2] = v3f(0.5,0.,0.);
			cnr[1][0] = v3f(0.,0.,0.);
			cnr[1][1] = v3f(-0.5,-0.5,-0.5);
			cnr[1][2] = v3f(0.5,-0.5,-0.5);
			cnr[2][0] = v3f(0.,0.,0.);
			cnr[2][1] = v3f(0.,0.,0.5);
			cnr[2][2] = v3f(-0.5,-0.5,0.5);
			cnr[3][0] = v3f(0.,0.,0.);
			cnr[3][1] = v3f(-0.5,-0.5,0.5);
			cnr[3][2] = v3f(-0.5,-0.5,-0.5);
		}else if (angle == 270) {
			cnr[0][0] = v3f(0.,0.,0.);
			cnr[0][1] = v3f(0.5,-0.5,0.5);
			cnr[0][2] = v3f(0.,0.,0.5);
			cnr[1][0] = v3f(0.,0.,0.);
			cnr[1][1] = v3f(0.5,-0.5,-0.5);
			cnr[1][2] = v3f(0.5,-0.5,0.5);
			cnr[2][0] = v3f(0.,0.,0.);
			cnr[2][1] = v3f(-0.5,0.,0.);
			cnr[2][2] = v3f(-0.5,-0.5,-0.5);
			cnr[3][0] = v3f(0.,0.,0.);
			cnr[3][1] = v3f(-0.5,-0.5,-0.5);
			cnr[3][2] = v3f(0.5,-0.5,-0.5);
			a1 = 90;
			a2 = 180;
		}
		s16 a = a1;
		for (int s=0; s<4; s++) {
			if (s == 2)
				a = a2;
			meshgen_rooftri(data,n,p,cnr[s],pos,toptile,selected,a,face);
		}
	}
	break;
	case 4:
	{
		v3f cnr[2][3];
		s16 a1 = angle;
		s16 a2 = angle - 90;
		if (angle == 0) {
			cnr[0][0] = v3f(-0.5,-0.5,-0.5);
			cnr[0][1] = v3f(0.5,-0.5,-0.5);
			cnr[0][2] = v3f(-0.5,0.5,0.5);
			cnr[1][0] = v3f(-0.5,0.5,0.5);
			cnr[1][1] = v3f(0.5,-0.5,-0.5);
			cnr[1][2] = v3f(0.5,-0.5,0.5);
			a1 = 180;
			a2 = 90;
			face = v3s16(1,1,-1);
		}else if (angle == 90) {
			cnr[0][0] = v3f(-0.5,-0.5,0.5);
			cnr[0][1] = v3f(0.5,-0.5,0.5);
			cnr[0][2] = v3f(-0.5,0.5,-0.5);
			cnr[1][0] = v3f(-0.5,0.5,-0.5);
			cnr[1][1] = v3f(0.5,-0.5,0.5);
			cnr[1][2] = v3f(0.5,-0.5,-0.5);
			a1 = 0;
			a2 = 90;
			face = v3s16(1,1,1);
		}else if (angle == 180) {
			cnr[0][0] = v3f(-0.5,-0.5,-0.5);
			cnr[0][1] = v3f(0.5,0.5,-0.5);
			cnr[0][2] = v3f(-0.5,-0.5,0.5);
			cnr[1][0] = v3f(-0.5,-0.5,0.5);
			cnr[1][1] = v3f(0.5,0.5,-0.5);
			cnr[1][2] = v3f(0.5,-0.5,0.5);
			a1 = 270;
			a2 = 0;
			face = v3s16(-1,1,1);
		}else if (angle == 270) {
			cnr[0][0] = v3f(-0.5,-0.5,0.5);
			cnr[0][1] = v3f(0.5,0.5,0.5);
			cnr[0][2] = v3f(-0.5,-0.5,-0.5);
			cnr[1][0] = v3f(-0.5,-0.5,-0.5);
			cnr[1][1] = v3f(0.5,0.5,0.5);
			cnr[1][2] = v3f(0.5,-0.5,-0.5);
			face = v3s16(-1,1,-1);
		}
		s16 a = a1;
		for (int s=0; s<2; s++) {
			meshgen_rooftri(data,n,p,cnr[s],pos,tile,selected,a,face);
			a = a2;
		}
	}
	break;
	case 5:
	{
		v3f cnr[8][3] = {
			{
				v3f(0.,0.,0.),
				v3f(-0.5,0.,0.),
				v3f(-0.5,-0.5,-0.5)
			},{
				v3f(-0.5,0.,0.),
				v3f(-0.5,-0.5,0.5),
				v3f(0.,0.,0.)
			},{
				v3f(0.,0.,0.),
				v3f(0.5,0.,0.),
				v3f(0.5,-0.5,-0.5)
			},{
				v3f(0.5,0.,0.),
				v3f(0.5,-0.5,0.5),
				v3f(0.,0.,0.)
			},{
				v3f(0.,0.,0.),
				v3f(0.,0.,-0.5),
				v3f(-0.5,-0.5,-0.5)
			},{
				v3f(0.,0.,-0.5),
				v3f(0.5,-0.5,-0.5),
				v3f(0.,0.,0.)
			},{
				v3f(0.,0.,0.),
				v3f(0.,0.,0.5),
				v3f(-0.5,-0.5,0.5)
			},{
				v3f(0.,0.,0.5),
				v3f(0.5,-0.5,0.5),
				v3f(0.,0.,0.)
			}
		};
		s16 a[8] = {
			180,
			0,
			180,
			0,
			270,
			90,
			270,
			90,
		};
		for (int s=0; s<8; s++) {
			meshgen_rooftri(data,n,p,cnr[s],pos,toptile,selected,a[s],face);
		}
	}
	break;
	case 6:
	{
		v3f cnr[6][3];
		s16 a[6] = {0,0,0,0,0,0};
		if (angle == 0) {
			cnr[0][0] = v3f(-0.5,-0.5,0.5);
			cnr[0][1] = v3f(0.5,-0.5,0.5);
			cnr[0][2] = v3f(0.5,0.,0.);
			cnr[1][0] = v3f(0.5,0.,0.);
			cnr[1][1] = v3f(-0.5,0.,0.);
			cnr[1][2] = v3f(-0.5,-0.5,0.5);
			cnr[2][0] = v3f(-0.5,-0.5,-0.5);
			cnr[2][1] = v3f(0.5,-0.5,-0.5);
			cnr[2][2] = v3f(0.5,0.,0.);
			cnr[3][0] = v3f(0.5,0.,0.);
			cnr[3][1] = v3f(-0.5,0.,0.);
			cnr[3][2] = v3f(-0.5,-0.5,-0.5);
			cnr[4][0] = v3f(0.,0.,-0.5);
			cnr[4][1] = v3f(-0.5,-0.5,-0.5);
			cnr[4][2] = v3f(0.,0.,0.);
			cnr[5][0] = v3f(0.,0.,0.);
			cnr[5][1] = v3f(0.5,-0.5,-0.5);
			cnr[5][2] = v3f(0.,0.,-0.5);
			a[2] = 180;
			a[3] = 180;
			a[4] = 270;
			a[5] = 90;
		}else if (angle == 90) {
			cnr[0][0] = v3f(-0.5,-0.5,-0.5);
			cnr[0][1] = v3f(-0.5,-0.5,0.5);
			cnr[0][2] = v3f(0.,0.,0.5);
			cnr[1][0] = v3f(0.,0.,0.5);
			cnr[1][1] = v3f(0.,0.,-0.5);
			cnr[1][2] = v3f(-0.5,-0.5,-0.5);
			cnr[2][0] = v3f(0.5,-0.5,-0.5);
			cnr[2][1] = v3f(0.5,-0.5,0.5);
			cnr[2][2] = v3f(0.,0.,0.5);
			cnr[3][0] = v3f(0.,0.,0.5);
			cnr[3][1] = v3f(0.,0.,-0.5);
			cnr[3][2] = v3f(0.5,-0.5,-0.5);
			cnr[4][0] = v3f(0.5,0.,0.);
			cnr[4][1] = v3f(0.5,-0.5,-0.5);
			cnr[4][2] = v3f(0.,0.,0.);
			cnr[5][0] = v3f(0.,0.,0.);
			cnr[5][1] = v3f(0.5,-0.5,0.5);
			cnr[5][2] = v3f(0.5,0.,0.);
			a[0] = 270;
			a[1] = 270;
			a[2] = 90;
			a[3] = 90;
			a[4] = 180;
		}else if (angle == 180) {
			cnr[0][0] = v3f(-0.5,-0.5,-0.5);
			cnr[0][1] = v3f(0.5,-0.5,-0.5);
			cnr[0][2] = v3f(0.5,0.,0.);
			cnr[1][0] = v3f(0.5,0.,0.);
			cnr[1][1] = v3f(-0.5,0.,0.);
			cnr[1][2] = v3f(-0.5,-0.5,-0.5);
			cnr[2][0] = v3f(-0.5,-0.5,0.5);
			cnr[2][1] = v3f(0.5,-0.5,0.5);
			cnr[2][2] = v3f(0.5,0.,0.);
			cnr[3][0] = v3f(0.5,0.,0.);
			cnr[3][1] = v3f(-0.5,0.,0.);
			cnr[3][2] = v3f(-0.5,-0.5,0.5);
			cnr[4][0] = v3f(0.,0.,0.5);
			cnr[4][1] = v3f(-0.5,-0.5,0.5);
			cnr[4][2] = v3f(0.,0.,0.);
			cnr[5][0] = v3f(0.,0.,0.);
			cnr[5][1] = v3f(0.5,-0.5,0.5);
			cnr[5][2] = v3f(0.,0.,0.5);
			a[0] = 180;
			a[1] = 180;
			a[4] = 270;
			a[5] = 90;
		}else if (angle == 270) {
			cnr[0][0] = v3f(0.5,-0.5,-0.5);
			cnr[0][1] = v3f(0.5,-0.5,0.5);
			cnr[0][2] = v3f(0.,0.,0.5);
			cnr[1][0] = v3f(0.,0.,0.5);
			cnr[1][1] = v3f(0.,0.,-0.5);
			cnr[1][2] = v3f(0.5,-0.5,-0.5);
			cnr[2][0] = v3f(-0.5,-0.5,-0.5);
			cnr[2][1] = v3f(-0.5,-0.5,0.5);
			cnr[2][2] = v3f(0.,0.,0.5);
			cnr[3][0] = v3f(0.,0.,0.5);
			cnr[3][1] = v3f(0.,0.,-0.5);
			cnr[3][2] = v3f(-0.5,-0.5,-0.5);
			cnr[4][0] = v3f(-0.5,0.,0.);
			cnr[4][1] = v3f(-0.5,-0.5,-0.5);
			cnr[4][2] = v3f(0.,0.,0.);
			cnr[5][0] = v3f(0.,0.,0.);
			cnr[5][1] = v3f(-0.5,-0.5,0.5);
			cnr[5][2] = v3f(-0.5,0.,0.);
			a[0] = 90;
			a[1] = 90;
			a[2] = 270;
			a[3] = 270;
			a[4] = 180;
		}
		for (int s=0; s<6; s++) {
			meshgen_rooftri(data,n,p,cnr[s],pos,toptile,selected,a[s],face);
		}
	}
	break;
	case 7:
	{
		v3f cnr[2][3];
		s16 a1 = angle;
		s16 a2 = angle - 90;
		if (angle == 0) {
			cnr[0][0] = v3f(0.5,0.5,-0.5);
			cnr[0][1] = v3f(-0.5,0.5,-0.5);
			cnr[0][2] = v3f(-0.5,-0.5,0.5);
			cnr[1][0] = v3f(0.5,0.5,-0.5);
			cnr[1][1] = v3f(0.5,0.5,0.5);
			cnr[1][2] = v3f(-0.5,-0.5,0.5);
			face = v3s16(-1,1,1);
		}else if (angle == 90) {
			cnr[0][0] = v3f(0.5,0.5,0.5);
			cnr[0][1] = v3f(-0.5,0.5,0.5);
			cnr[0][2] = v3f(-0.5,-0.5,-0.5);
			cnr[1][0] = v3f(0.5,0.5,0.5);
			cnr[1][1] = v3f(0.5,0.5,-0.5);
			cnr[1][2] = v3f(-0.5,-0.5,-0.5);
			a1 = 180;
			a2 = 270;
			face = v3s16(-1,1,-1);
		}else if (angle == 180) {
			cnr[0][0] = v3f(0.5,-0.5,-0.5);
			cnr[0][1] = v3f(-0.5,0.5,-0.5);
			cnr[0][2] = v3f(-0.5,0.5,0.5);
			cnr[1][0] = v3f(0.5,-0.5,-0.5);
			cnr[1][1] = v3f(0.5,0.5,0.5);
			cnr[1][2] = v3f(-0.5,0.5,0.5);
			a1 = 90;
			a2 = 180;
			face = v3s16(1,1,-1);
		}else if (angle == 270) {
			cnr[0][0] = v3f(0.5,-0.5,0.5);
			cnr[0][1] = v3f(-0.5,0.5,0.5);
			cnr[0][2] = v3f(-0.5,0.5,-0.5);
			cnr[1][0] = v3f(0.5,-0.5,0.5);
			cnr[1][1] = v3f(0.5,0.5,-0.5);
			cnr[1][2] = v3f(-0.5,0.5,-0.5);
			a1 = 90;
			a2 = 0;
			face = v3s16(1,1,1);
		}
		s16 a = a1;
		for (int s=0; s<2; s++) {
			meshgen_rooftri(data,n,p,cnr[s],pos,tile,selected,a,face);
			a = a2;
		}
	}
	break;
	case 8:
	{
		v3f cnr[4][3];
		cnr[0][0] = v3f(0.,0.,0.);
		cnr[0][1] = v3f(-0.5,-0.5,-0.5);
		cnr[0][2] = v3f(-0.5,-0.5,0.5);
		cnr[1][0] = v3f(0.,0.,0.);
		cnr[1][1] = v3f(-0.5,-0.5,-0.5);
		cnr[1][2] = v3f(0.5,-0.5,-0.5);
		cnr[2][0] = v3f(0.,0.,0.);
		cnr[2][1] = v3f(0.5,-0.5,-0.5);
		cnr[2][2] = v3f(0.5,-0.5,0.5);
		cnr[3][0] = v3f(0.,0.,0.);
		cnr[3][1] = v3f(-0.5,-0.5,0.5);
		cnr[3][2] = v3f(0.5,-0.5,0.5);
		for (int s=0; s<4; s++) {
			meshgen_rooftri(data,n,p,cnr[s],pos,toptile,selected,(90*s)+90+(180*(!(s%2))),face);
		}
	}
	break;
	case 9:
	{
		v3f cnr[5][3];
		s16 a[5] = {0,0,0,0,0};
		if (angle == 0) {
			cnr[0][0] = v3f(-0.5,-0.5,-0.5);
			cnr[0][1] = v3f(0.5,-0.5,-0.5);
			cnr[0][2] = v3f(0.,0.,0.);
			cnr[1][0] = v3f(-0.5,-0.5,-0.5);
			cnr[1][1] = v3f(0.,0.,0.);
			cnr[1][2] = v3f(-0.5,0.,0.);
			cnr[2][0] = v3f(0.5,-0.5,0.5);
			cnr[2][1] = v3f(-0.5,-0.5,0.5);
			cnr[2][2] = v3f(0.,0.,0.);
			cnr[3][0] = v3f(0.,0.,0.);
			cnr[3][1] = v3f(-0.5,-0.5,0.5);
			cnr[3][2] = v3f(-0.5,0.,0.);
			cnr[4][0] = v3f(0.5,-0.5,-0.5);
			cnr[4][1] = v3f(0.5,-0.5,0.5);
			cnr[4][2] = v3f(0.,0.,0.);
			a[0] = 180;
			a[1] = 180;
			a[4] = 90;
			face = v3s16(1,1,0);
		}else if (angle == 90) {
			cnr[0][0] = v3f(-0.5,-0.5,0.5);
			cnr[0][1] = v3f(-0.5,-0.5,-0.5);
			cnr[0][2] = v3f(0.,0.,0.);
			cnr[1][0] = v3f(0.,0.,0.);
			cnr[1][1] = v3f(-0.5,-0.5,-0.5);
			cnr[1][2] = v3f(0.,0.,-0.5);
			cnr[2][0] = v3f(0.5,-0.5,-0.5);
			cnr[2][1] = v3f(0.5,-0.5,0.5);
			cnr[2][2] = v3f(0.,0.,0.);
			cnr[3][0] = v3f(0.5,-0.5,-0.5);
			cnr[3][1] = v3f(0.,0.,0.);
			cnr[3][2] = v3f(0.,0.,-0.5);
			cnr[4][0] = v3f(-0.5,-0.5,0.5);
			cnr[4][1] = v3f(0.5,-0.5,0.5);
			cnr[4][2] = v3f(0.,0.,0.);
			a[0] = 270;
			a[1] = 270;
			a[2] = 90;
			a[3] = 90;
			face = v3s16(0,1,1);
		}else if (angle == 180) {
			cnr[0][0] = v3f(-0.5,-0.5,-0.5);
			cnr[0][1] = v3f(0.5,-0.5,-0.5);
			cnr[0][2] = v3f(0.,0.,0.);
			cnr[1][0] = v3f(0.5,-0.5,-0.5);
			cnr[1][1] = v3f(0.,0.,0.);
			cnr[1][2] = v3f(0.5,0.,0.);
			cnr[2][0] = v3f(0.5,-0.5,0.5);
			cnr[2][1] = v3f(-0.5,-0.5,0.5);
			cnr[2][2] = v3f(0.,0.,0.);
			cnr[3][0] = v3f(0.,0.,0.);
			cnr[3][1] = v3f(0.5,-0.5,0.5);
			cnr[3][2] = v3f(0.5,0.,0.);
			cnr[4][0] = v3f(-0.5,-0.5,0.5);
			cnr[4][1] = v3f(-0.5,-0.5,-0.5);
			cnr[4][2] = v3f(0.,0.,0.);
			a[0] = 180;
			a[1] = 180;
			a[4] = 270;
			face = v3s16(-1,1,0);
		}else if (angle == 270) {
			cnr[0][0] = v3f(-0.5,-0.5,0.5);
			cnr[0][1] = v3f(-0.5,-0.5,-0.5);
			cnr[0][2] = v3f(0.,0.,0.);
			cnr[1][0] = v3f(0.,0.,0.);
			cnr[1][1] = v3f(0.,0.,0.5);
			cnr[1][2] = v3f(-0.5,-0.5,0.5);
			cnr[2][0] = v3f(0.5,-0.5,-0.5);
			cnr[2][1] = v3f(0.5,-0.5,0.5);
			cnr[2][2] = v3f(0.,0.,0.);
			cnr[3][0] = v3f(0.,0.,0.);
			cnr[3][1] = v3f(0.,0.,0.5);
			cnr[3][2] = v3f(0.5,-0.5,0.5);
			cnr[4][0] = v3f(0.5,-0.5,-0.5);
			cnr[4][1] = v3f(-0.5,-0.5,-0.5);
			cnr[4][2] = v3f(0.,0.,0.);
			a[0] = 270;
			a[1] = 270;
			a[2] = 90;
			a[3] = 90;
			a[4] = 180;
			face = v3s16(0,1,-1);
		}
		for (int s=0; s<5; s++) {
			meshgen_rooftri(data,n,p,cnr[s],pos,toptile,selected,a[s],face);
		}
	}
	break;
	default:
		break;
	}
}

void meshgen_leaflike(MeshMakeData *data, v3s16 p, MapNode &n, SelectedNode &selected)
{
	if (data->mesh_detail == 1) {
		meshgen_glasslike(data,p,n,selected);
		return;
	}

	s16 angle = 0;
	s16 dir = 0;
	s16 twist = 0;
	v3f offset(0,0,0);
	bool ground = true;

	{
		s16 distance = 1;
		bool inv = false;
		v3s16 tp;
		for (; ground && distance<4; distance++) {
			for (s16 x=-distance; ground && x<=distance; x++) {
				for (s16 z=-distance; ground && z<=distance; z++) {
					if (x == -distance || x == distance || z == -distance || z == distance) {
						MapNode nn = data->m_vmanip.getNodeRO(data->m_blockpos_nodes+p+v3s16(x,0,z));
						if (
							nn.getContent() == CONTENT_TREE
							|| nn.getContent() == CONTENT_JUNGLETREE
							|| nn.getContent() == CONTENT_APPLE_TREE
							|| nn.getContent() == CONTENT_CONIFER_TREE
							|| nn.getContent() == CONTENT_YOUNG_TREE
							|| nn.getContent() == CONTENT_YOUNG_JUNGLETREE
							|| nn.getContent() == CONTENT_YOUNG_APPLE_TREE
							|| nn.getContent() == CONTENT_YOUNG_CONIFER_TREE
						) {
							ground = false;
							tp = v3s16(x,0,z);
						}
					}
				}
			}
		}

		if (ground) {
			inv = true;
			for (distance=0; ground && distance<4; distance++) {
				for (s16 x=-distance; ground && x<=distance; x++) {
					for (s16 z=-distance; ground && z<=distance; z++) {
						if (x == -distance || x == distance || z == -distance || z == distance) {
							for (s16 y=4; ground && y>-5; y--) {
								if (!y)
									continue;
								MapNode nn = data->m_vmanip.getNodeRO(data->m_blockpos_nodes+p+v3s16(x,y,z));
								if (
									nn.getContent() == CONTENT_TREE
									|| nn.getContent() == CONTENT_JUNGLETREE
									|| nn.getContent() == CONTENT_APPLE_TREE
									|| nn.getContent() == CONTENT_CONIFER_TREE
									|| nn.getContent() == CONTENT_YOUNG_TREE
									|| nn.getContent() == CONTENT_YOUNG_JUNGLETREE
									|| nn.getContent() == CONTENT_YOUNG_APPLE_TREE
									|| nn.getContent() == CONTENT_YOUNG_CONIFER_TREE
								) {
									if (y<0)
										inv = false;
									ground = false;
									tp = v3s16(x,0,z);
								}
							}
						}
					}
				}
			}
		}

		if (!ground) {
			if (inv) {
				offset.Y = (-0.2*BS)*(float)distance;
				angle = 12*(5-distance);
			}else{
				angle = 12*distance;
			}
			v3f tv = intToFloat(tp,-1);
			tv.normalize();
			dir = 180./PI*atan2(tv.Z,tv.X);
			dir -= 90;

			twist = (tp.X*5)+(tp.Y*2);
		}
	}


	v3f pos = intToFloat(p,BS);
	if (selected.is_coloured || selected.has_crack) {
		TileSpec tile = getNodeTile(n,p,v3s16(1,0,0),selected,NULL);
		for (u16 k=0; k<2; k++) {
			video::S3DVertex vertices[4] = {
				video::S3DVertex( 0.75*data->m_BS, 0.,-0.75*data->m_BS, 0,1,0, video::SColor(255,255,255,255), tile.texture.x1(), tile.texture.y1()),
				video::S3DVertex(-0.75*data->m_BS, 0.,-0.75*data->m_BS, 0,1,0, video::SColor(255,255,255,255), tile.texture.x0(), tile.texture.y1()),
				video::S3DVertex(-0.75*data->m_BS, 0., 0.75*data->m_BS, 0,1,0, video::SColor(255,255,255,255), tile.texture.x0(), tile.texture.y0()),
				video::S3DVertex( 0.75*data->m_BS, 0., 0.75*data->m_BS, 0,1,0, video::SColor(255,255,255,255), tile.texture.x1(), tile.texture.y0())
			};
			if (ground) {
				for (u16 i=0; i<4; i++) {
					vertices[i].Pos.Y = -0.49*data->m_BS;
					vertices[i].Pos += offset;
				}
			}else{
				for (u16 i=0; i<4; i++) {
					vertices[i].Pos.rotateYZBy(angle);
					vertices[i].Pos.rotateXYBy(twist);
					vertices[i].Pos.rotateXZBy(dir);
					vertices[i].Pos += offset;
				}
			}

			u16 indices[6] = {0,1,2,2,3,0};
			std::vector<u32> colours;
			if (selected.is_coloured) {
				meshgen_selected_lights(colours,255,4);
			}else{
				meshgen_lights(data,n,p,colours,255,v3s16(0,1,0),4,vertices);
			}

			for (u16 i=0; i<4; i++) {
				vertices[i].Pos += pos;
				if (k == 0) {
					vertices[i].Pos += data->m_BSd;
				}else{
					vertices[i].Pos -= data->m_BSd;
				}
			}

			data->append(tile, vertices, 4, indices, 6, colours);
		}
	}else{
		TileSpec tile = getNodeTile(n,p,v3s16(1,0,0),selected,NULL);
		video::S3DVertex vertices[4] = {
			video::S3DVertex( 0.75*data->m_BS, 0.,-0.75*data->m_BS, 0,1,0, video::SColor(255,255,255,255), tile.texture.x1(), tile.texture.y1()),
			video::S3DVertex(-0.75*data->m_BS, 0.,-0.75*data->m_BS, 0,1,0, video::SColor(255,255,255,255), tile.texture.x0(), tile.texture.y1()),
			video::S3DVertex(-0.75*data->m_BS, 0., 0.75*data->m_BS, 0,1,0, video::SColor(255,255,255,255), tile.texture.x0(), tile.texture.y0()),
			video::S3DVertex( 0.75*data->m_BS, 0., 0.75*data->m_BS, 0,1,0, video::SColor(255,255,255,255), tile.texture.x1(), tile.texture.y0())
		};
		if (ground) {
			for (u16 i=0; i<4; i++) {
				vertices[i].Pos.Y = -0.49*data->m_BS;
				vertices[i].Pos += offset;
			}
		}else{
			for (u16 i=0; i<4; i++) {
				vertices[i].Pos.rotateYZBy(angle);
				vertices[i].Pos.rotateXYBy(twist);
				vertices[i].Pos.rotateXZBy(dir);
				vertices[i].Pos += offset;
			}
		}

		u16 indices[6] = {0,1,2,2,3,0};
		std::vector<u32> colours;
		meshgen_lights(data,n,p,colours,255,v3s16(0,1,0),4,vertices);

		for (u16 i=0; i<4; i++) {
			vertices[i].Pos += pos;
		}

		data->append(tile, vertices, 4, indices, 6, colours);
	}
}

void meshgen_wirelike(MeshMakeData *data, v3s16 p, MapNode &n, SelectedNode &selected, bool is3d)
{
	MapNode n_plus_y = data->m_vmanip.getNodeNoEx(data->m_blockpos_nodes + p + v3s16(0,1,0));
	MapNode n_minus_x = data->m_vmanip.getNodeNoEx(data->m_blockpos_nodes + p + v3s16(-1,0,0));
	MapNode n_plus_x = data->m_vmanip.getNodeNoEx(data->m_blockpos_nodes + p + v3s16(1,0,0));
	MapNode n_minus_z = data->m_vmanip.getNodeNoEx(data->m_blockpos_nodes + p + v3s16(0,0,-1));
	MapNode n_plus_z = data->m_vmanip.getNodeNoEx(data->m_blockpos_nodes + p + v3s16(0,0,1));
	MapNode n_minus_xy = data->m_vmanip.getNodeNoEx(data->m_blockpos_nodes + p + v3s16(-1,1,0));
	MapNode n_plus_xy = data->m_vmanip.getNodeNoEx(data->m_blockpos_nodes + p + v3s16(1,1,0));
	MapNode n_minus_zy = data->m_vmanip.getNodeNoEx(data->m_blockpos_nodes + p + v3s16(0,1,-1));
	MapNode n_plus_zy = data->m_vmanip.getNodeNoEx(data->m_blockpos_nodes + p + v3s16(0,1,1));
	MapNode n_minus_x_y = data->m_vmanip.getNodeNoEx(data->m_blockpos_nodes + p + v3s16(-1,-1,0));
	MapNode n_plus_x_y = data->m_vmanip.getNodeNoEx(data->m_blockpos_nodes + p + v3s16(1,-1,0));
	MapNode n_minus_z_y = data->m_vmanip.getNodeNoEx(data->m_blockpos_nodes + p + v3s16(0,-1,-1));
	MapNode n_plus_z_y = data->m_vmanip.getNodeNoEx(data->m_blockpos_nodes + p + v3s16(0,-1,1));
	bool x_plus = false;
	bool x_plus_y = false;
	bool x_minus = false;
	bool x_minus_y = false;
	bool z_plus = false;
	bool z_plus_y = false;
	bool z_minus = false;
	bool z_minus_y = false;
	bool y_plus = false;
	// +Y
	if (n_plus_y.getContent() == CONTENT_AIR || content_features(n_plus_y).energy_type != CET_NONE)
		y_plus = true;
	// +X
	if (
		content_features(n_plus_x).energy_type == CET_NONE
		&& content_features(n_plus_x).flammable != 2
	) {
		if (
			y_plus
			&& (
				content_features(n_plus_x).draw_type == CDT_CUBELIKE
				|| content_features(n_plus_x).draw_type == CDT_GLASSLIKE
			)
		) {
			if (content_features(n_plus_xy).energy_type != CET_NONE) {
				x_plus_y = true;
				x_plus = true;
			}
		}else if (
			n_plus_x.getContent() == CONTENT_AIR
			&& content_features(n_plus_x_y).energy_type != CET_NONE
		) {
			x_plus = true;
		}
	}else{
		x_plus = true;
	}
	// -X
	if (content_features(n_minus_x).energy_type == CET_NONE && content_features(n_minus_x).flammable != 2) {
		if (
			y_plus
			&& (
				content_features(n_minus_x).draw_type == CDT_CUBELIKE
				|| content_features(n_minus_x).draw_type == CDT_GLASSLIKE
			)
		) {
			if (content_features(n_minus_xy).energy_type != CET_NONE) {
				x_minus_y = true;
				x_minus = true;
			}
		}else if (
			n_minus_x.getContent() == CONTENT_AIR
			&& content_features(n_minus_x_y).energy_type != CET_NONE
		) {
			x_minus = true;
		}
	}else{
		x_minus = true;
	}
	// +Z
	if (
		content_features(n_plus_z).energy_type == CET_NONE
		&& content_features(n_plus_z).flammable != 2
	) {
		if (
			y_plus
			&& (
				content_features(n_plus_z).draw_type == CDT_CUBELIKE
				|| content_features(n_plus_z).draw_type == CDT_GLASSLIKE
			)
		) {
			if (content_features(n_plus_zy).energy_type != CET_NONE) {
				z_plus_y = true;
				z_plus = true;
			}
		}else if (
			n_plus_z.getContent() == CONTENT_AIR
			&& content_features(n_plus_z_y).energy_type != CET_NONE
		) {
			z_plus = true;
		}
	}else{
		z_plus = true;
	}
	// -Z
	if (
		content_features(n_minus_z).energy_type == CET_NONE
		&& content_features(n_minus_z).flammable != 2
	) {
		if (
			y_plus
			&& (
				content_features(n_minus_z).draw_type == CDT_CUBELIKE
				|| content_features(n_minus_z).draw_type == CDT_GLASSLIKE
			)
		) {
			if (content_features(n_minus_zy).energy_type != CET_NONE) {
				z_minus_y = true;
				z_minus = true;
			}
		}else if (
			n_minus_z.getContent() == CONTENT_AIR
			&& content_features(n_minus_z_y).energy_type != CET_NONE
		) {
			z_minus = true;
		}
	}else{
		z_minus = true;
	}

	static const v3s16 tile_dirs[6] = {
		v3s16(0, 1, 0),
		v3s16(0, -1, 0),
		v3s16(1, 0, 0),
		v3s16(-1, 0, 0),
		v3s16(0, 0, 1),
		v3s16(0, 0, -1)
	};

	TileSpec tiles[6];
	for (int i = 0; i < 6; i++) {
		// Handles facedir rotation for textures
		tiles[i] = getNodeTile(n,p,tile_dirs[i],selected);
	}

	v3f pos = intToFloat(p, BS);
	u8 cols[4] = {250,64,64,64};
	if (selected.is_coloured) {
		cols[0] = 255;
		cols[1] = 64;
		cols[2] = 64;
		cols[3] = 255;
	}else{
		NodeMetadata *meta = data->m_env->getMap().getNodeMetadataClone(p+data->m_blockpos_nodes);
		if (meta) {
			if (meta->getEnergy()) {
				u8 e = meta->getEnergy();
				e = (e*16)-1;
				if (e < 80)
					e = 80;
				cols[0] = 255;
				cols[1] = e;
				cols[2] = e;
				cols[3] = e;
			}
			delete meta;
		}
	}

	if (is3d == false) {
		f32 sy = tiles[0].texture.y1()-tiles[0].texture.y0();
		if (!x_plus && !x_minus && !z_plus && !z_minus) {
			{
				video::S3DVertex vertices[4] = {
					video::S3DVertex(-data->m_BS/2,-0.49*data->m_BS,data->m_BS/2, 0,0,0, video::SColor(255,255,255,255), tiles[0].texture.x0(), tiles[0].texture.y0()),
					video::S3DVertex(data->m_BS/2,-0.49*data->m_BS,data->m_BS/2, 0,0,0, video::SColor(255,255,255,255), tiles[0].texture.x0(), tiles[0].texture.y1()),
					video::S3DVertex(data->m_BS/2,-0.49*data->m_BS,-data->m_BS/2, 0,0,0, video::SColor(255,255,255,255), tiles[0].texture.x1(), tiles[0].texture.y1()),
					video::S3DVertex(-data->m_BS/2,-0.49*data->m_BS,-data->m_BS/2, 0,0,0, video::SColor(255,255,255,255), tiles[0].texture.x1(), tiles[0].texture.y0()),
				};
				for (u16 i=0; i<4; i++) {
					vertices[i].Pos += pos;
				}
				u16 indices[] = {0,1,2,2,3,0};
				std::vector<u32> colours;
				meshgen_custom_lights(colours,cols[0],cols[1],cols[2],cols[3],4);
				data->append(tiles[0], vertices, 4, indices, 6, colours);
			}
			{
				video::S3DVertex vertices[4] = {
					video::S3DVertex(-data->m_BS/2,-0.49*data->m_BS,data->m_BS/2, 0,0,0, video::SColor(255,255,255,255), tiles[0].texture.x0(), tiles[0].texture.y1()),
					video::S3DVertex(data->m_BS/2,-0.49*data->m_BS,data->m_BS/2, 0,0,0, video::SColor(255,255,255,255), tiles[0].texture.x1(), tiles[0].texture.y1()),
					video::S3DVertex(data->m_BS/2,-0.49*data->m_BS,-data->m_BS/2, 0,0,0, video::SColor(255,255,255,255), tiles[0].texture.x1(), tiles[0].texture.y0()),
					video::S3DVertex(-data->m_BS/2,-0.49*data->m_BS,-data->m_BS/2, 0,0,0, video::SColor(255,255,255,255), tiles[0].texture.x0(), tiles[0].texture.y0()),
				};
				for (u16 i=0; i<4; i++) {
					vertices[i].Pos += pos;
				}
				u16 indices[] = {0,1,2,2,3,0};
				std::vector<u32> colours;
				meshgen_custom_lights(colours,cols[0],cols[1],cols[2],cols[3],4);
				data->append(tiles[0], vertices, 4, indices, 6, colours);
			}
		}else{
			if (x_plus) {
				video::S3DVertex vertices[4] = {
					video::S3DVertex(0,-0.49*data->m_BS,data->m_BS/2, 0,0,0, video::SColor(255,255,255,255), tiles[0].texture.x0(), tiles[0].texture.y0()+(sy/2)),
					video::S3DVertex(data->m_BS/2,-0.49*data->m_BS,data->m_BS/2, 0,0,0, video::SColor(255,255,255,255), tiles[0].texture.x0(), tiles[0].texture.y1()),
					video::S3DVertex(data->m_BS/2,-0.49*data->m_BS,-data->m_BS/2, 0,0,0, video::SColor(255,255,255,255), tiles[0].texture.x1(), tiles[0].texture.y1()),
					video::S3DVertex(0,-0.49*data->m_BS,-data->m_BS/2, 0,0,0, video::SColor(255,255,255,255), tiles[0].texture.x1(), tiles[0].texture.y0()+(sy/2)),
				};
				for (u16 i=0; i<4; i++) {
					vertices[i].Pos += pos;
				}
				u16 indices[] = {0,1,2,2,3,0};
				std::vector<u32> colours;
				meshgen_custom_lights(colours,cols[0],cols[1],cols[2],cols[3],4);
				data->append(tiles[0], vertices, 4, indices, 6, colours);
			}
			if (x_minus) {
				video::S3DVertex vertices[4] = {
					video::S3DVertex(-data->m_BS/2,-0.49*data->m_BS,data->m_BS/2, 0,0,0, video::SColor(255,255,255,255), tiles[0].texture.x0(), tiles[0].texture.y0()),
					video::S3DVertex(0,-0.49*data->m_BS,data->m_BS/2, 0,0,0, video::SColor(255,255,255,255), tiles[0].texture.x0(), tiles[0].texture.y1()-(sy/2)),
					video::S3DVertex(0,-0.49*data->m_BS,-data->m_BS/2, 0,0,0, video::SColor(255,255,255,255), tiles[0].texture.x1(), tiles[0].texture.y1()-(sy/2)),
					video::S3DVertex(-data->m_BS/2,-0.49*data->m_BS,-data->m_BS/2, 0,0,0, video::SColor(255,255,255,255), tiles[0].texture.x1(), tiles[0].texture.y0()),
				};
				for (u16 i=0; i<4; i++) {
					vertices[i].Pos += pos;
				}
				u16 indices[] = {0,1,2,2,3,0};
				std::vector<u32> colours;
				meshgen_custom_lights(colours,cols[0],cols[1],cols[2],cols[3],4);
				data->append(tiles[0], vertices, 4, indices, 6, colours);
			}
			if (z_plus) {
				video::S3DVertex vertices[4] = {
					video::S3DVertex(-data->m_BS/2,-0.49*data->m_BS,data->m_BS/2, 0,0,0, video::SColor(255,255,255,255), tiles[0].texture.x0(), tiles[0].texture.y1()),
					video::S3DVertex(data->m_BS/2,-0.49*data->m_BS,data->m_BS/2, 0,0,0, video::SColor(255,255,255,255), tiles[0].texture.x1(), tiles[0].texture.y1()),
					video::S3DVertex(data->m_BS/2,-0.49*data->m_BS,0, 0,0,0, video::SColor(255,255,255,255), tiles[0].texture.x1(), tiles[0].texture.y0()+(sy/2)),
					video::S3DVertex(-data->m_BS/2,-0.49*data->m_BS,0, 0,0,0, video::SColor(255,255,255,255), tiles[0].texture.x0(), tiles[0].texture.y0()+(sy/2)),
				};
				for (u16 i=0; i<4; i++) {
					vertices[i].Pos += pos;
				}
				u16 indices[] = {0,1,2,2,3,0};
				std::vector<u32> colours;
				meshgen_custom_lights(colours,cols[0],cols[1],cols[2],cols[3],4);
				data->append(tiles[0], vertices, 4, indices, 6, colours);
			}
			if (z_minus) {
				video::S3DVertex vertices[4] = {
					video::S3DVertex(-data->m_BS/2,-0.49*data->m_BS,0, 0,0,0, video::SColor(255,255,255,255), tiles[0].texture.x0(), tiles[0].texture.y1()-(sy/2)),
					video::S3DVertex(data->m_BS/2,-0.49*data->m_BS,0, 0,0,0, video::SColor(255,255,255,255), tiles[0].texture.x1(), tiles[0].texture.y1()-(sy/2)),
					video::S3DVertex(data->m_BS/2,-0.49*data->m_BS,-data->m_BS/2, 0,0,0, video::SColor(255,255,255,255), tiles[0].texture.x1(), tiles[0].texture.y0()),
					video::S3DVertex(-data->m_BS/2,-0.49*data->m_BS,-data->m_BS/2, 0,0,0, video::SColor(255,255,255,255), tiles[0].texture.x0(), tiles[0].texture.y0()),
				};
				for (u16 i=0; i<4; i++) {
					vertices[i].Pos += pos;
				}
				u16 indices[] = {0,1,2,2,3,0};
				std::vector<u32> colours;
				meshgen_custom_lights(colours,cols[0],cols[1],cols[2],cols[3],4);
				data->append(tiles[0], vertices, 4, indices, 6, colours);
			}
			if (x_plus_y) {
				video::S3DVertex vertices[4] = {
					video::S3DVertex(0.49*data->m_BS,-data->m_BS/2,-data->m_BS/2, 0,0,0, video::SColor(255,255,255,255), tiles[0].texture.x0(), tiles[0].texture.y1()),
					video::S3DVertex(0.49*data->m_BS,-data->m_BS/2,data->m_BS/2, 0,0,0, video::SColor(255,255,255,255), tiles[0].texture.x1(), tiles[0].texture.y1()),
					video::S3DVertex(0.49*data->m_BS,data->m_BS/2,data->m_BS/2, 0,0,0, video::SColor(255,255,255,255), tiles[0].texture.x1(), tiles[0].texture.y0()),
					video::S3DVertex(0.49*data->m_BS,data->m_BS/2,-data->m_BS/2, 0,0,0, video::SColor(255,255,255,255), tiles[0].texture.x0(), tiles[0].texture.y0()),
				};
				for (u16 i=0; i<4; i++) {
					vertices[i].Pos += pos;
				}
				u16 indices[] = {0,1,2,2,3,0};
				std::vector<u32> colours;
				meshgen_custom_lights(colours,cols[0],cols[1],cols[2],cols[3],4);
				data->append(tiles[0], vertices, 4, indices, 6, colours);
			}
			if (x_minus_y) {
				video::S3DVertex vertices[4] = {
					video::S3DVertex(-0.49*data->m_BS,-data->m_BS/2,data->m_BS/2, 0,0,0, video::SColor(255,255,255,255), tiles[0].texture.x0(), tiles[0].texture.y1()),
					video::S3DVertex(-0.49*data->m_BS,-data->m_BS/2,-data->m_BS/2, 0,0,0, video::SColor(255,255,255,255), tiles[0].texture.x1(), tiles[0].texture.y1()),
					video::S3DVertex(-0.49*data->m_BS,data->m_BS/2,-data->m_BS/2, 0,0,0, video::SColor(255,255,255,255), tiles[0].texture.x1(), tiles[0].texture.y0()),
					video::S3DVertex(-0.49*data->m_BS,data->m_BS/2,data->m_BS/2, 0,0,0, video::SColor(255,255,255,255), tiles[0].texture.x0(), tiles[0].texture.y0()),
				};
				for (u16 i=0; i<4; i++) {
					vertices[i].Pos += pos;
				}
				u16 indices[] = {0,1,2,2,3,0};
				std::vector<u32> colours;
				meshgen_custom_lights(colours,cols[0],cols[1],cols[2],cols[3],4);
				data->append(tiles[0], vertices, 4, indices, 6, colours);
			}
			if (z_plus_y) {
				video::S3DVertex vertices[4] = {
					video::S3DVertex(data->m_BS/2,-data->m_BS/2,0.49*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tiles[0].texture.x0(), tiles[0].texture.y1()),
					video::S3DVertex(-data->m_BS/2,-data->m_BS/2,0.49*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tiles[0].texture.x1(), tiles[0].texture.y1()),
					video::S3DVertex(-data->m_BS/2,data->m_BS/2,0.49*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tiles[0].texture.x1(), tiles[0].texture.y0()),
					video::S3DVertex(data->m_BS/2,data->m_BS/2,0.49*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tiles[0].texture.x0(), tiles[0].texture.y0()),
				};
				for (u16 i=0; i<4; i++) {
					vertices[i].Pos += pos;
				}
				u16 indices[] = {0,1,2,2,3,0};
				std::vector<u32> colours;
				meshgen_custom_lights(colours,cols[0],cols[1],cols[2],cols[3],4);
				data->append(tiles[0], vertices, 4, indices, 6, colours);
			}
			if (z_minus_y) {
				video::S3DVertex vertices[4] = {
					video::S3DVertex(-data->m_BS/2,-data->m_BS/2,-0.49*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tiles[0].texture.x0(), tiles[0].texture.y1()),
					video::S3DVertex(data->m_BS/2,-data->m_BS/2,-0.49*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tiles[0].texture.x1(), tiles[0].texture.y1()),
					video::S3DVertex(data->m_BS/2,data->m_BS/2,-0.49*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tiles[0].texture.x1(), tiles[0].texture.y0()),
					video::S3DVertex(-data->m_BS/2,data->m_BS/2,-0.49*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tiles[0].texture.x0(), tiles[0].texture.y0()),
				};
				for (u16 i=0; i<4; i++) {
					vertices[i].Pos += pos;
				}
				u16 indices[] = {0,1,2,2,3,0};
				std::vector<u32> colours;
				meshgen_custom_lights(colours,cols[0],cols[1],cols[2],cols[3],4);
				data->append(tiles[0], vertices, 4, indices, 6, colours);
			}
		}
		return;
	}

	std::vector<aabb3f> boxes;
	if (!x_plus && !x_minus && !z_plus && !z_minus) {
		boxes.push_back(aabb3f(-0.125*data->m_BS,-0.5*data->m_BS,-0.125*data->m_BS,0.125*data->m_BS,-0.4375*data->m_BS,0.125*data->m_BS));
	}else{
		if (x_plus) {
			boxes.push_back(aabb3f(0.,-0.5*data->m_BS,-0.0625*data->m_BS,0.5*data->m_BS,-0.4375*data->m_BS,0.0625*data->m_BS));
		}
		if (x_minus) {
			boxes.push_back(aabb3f(-0.5*data->m_BS,-0.5*data->m_BS,-0.0625*data->m_BS,0.,-0.4375*data->m_BS,0.0625*data->m_BS));
		}
		if (z_plus) {
			boxes.push_back(aabb3f(-0.0625*data->m_BS,-0.5*data->m_BS,0.,0.0625*data->m_BS,-0.4375*data->m_BS,0.5*data->m_BS));
		}
		if (z_minus) {
			boxes.push_back(aabb3f(-0.0625*data->m_BS,-0.5*data->m_BS,-0.5*data->m_BS,0.0625*data->m_BS,-0.4375*data->m_BS,0.));
		}
		if (x_plus_y) {
			boxes.push_back(aabb3f(0.4375*data->m_BS,-0.4375*data->m_BS,-0.0625*data->m_BS,0.5*data->m_BS,0.5625*data->m_BS,0.0625*data->m_BS));
		}
		if (x_minus_y) {
			boxes.push_back(aabb3f(-0.5*data->m_BS,-0.4375*data->m_BS,-0.0625*data->m_BS,-0.4375*data->m_BS,0.5625*data->m_BS,0.0625*data->m_BS));
		}
		if (z_plus_y) {
			boxes.push_back(aabb3f(-0.0625*data->m_BS,-0.4375*data->m_BS,0.4375*data->m_BS,0.0625*data->m_BS,0.5625*data->m_BS,0.5*data->m_BS));
		}
		if (z_minus_y) {
			boxes.push_back(aabb3f(-0.0625*data->m_BS,-0.4375*data->m_BS,-0.5*data->m_BS,0.0625*data->m_BS,0.5625*data->m_BS,-0.4375*data->m_BS));
		}
		u8 cnt = x_plus+x_minus+z_plus+z_minus;
		if (
			cnt > 2
			|| (
				cnt == 2
				&& (
					(x_plus && z_plus)
					|| (x_minus && z_plus)
					|| (x_plus && z_minus)
					|| (x_minus && z_minus)
				)
			)
		) {
			boxes.push_back(aabb3f(-0.125*data->m_BS,-0.5*data->m_BS,-0.125*data->m_BS,0.125*data->m_BS,-0.375*data->m_BS,0.125*data->m_BS));
		}
	}
	for (std::vector<aabb3f>::iterator i = boxes.begin(); i != boxes.end(); i++) {
		aabb3f box = *i;

		// Compute texture coords
		f32 tx1 = (i->MinEdge.X/data->m_BS)+0.5;
		f32 ty1 = (i->MinEdge.Y/data->m_BS)+0.5;
		f32 tz1 = (i->MinEdge.Z/data->m_BS)+0.5;
		f32 tx2 = (i->MaxEdge.X/data->m_BS)+0.5;
		f32 ty2 = (i->MaxEdge.Y/data->m_BS)+0.5;
		f32 tz2 = (i->MaxEdge.Z/data->m_BS)+0.5;
		f32 txc[24] = {
			// up
			tx1, 1-tz2, tx2, 1-tz1,
			// down
			tx1, tz1, tx2, tz2,
			// right
			tz1, 1-ty2, tz2, 1-ty1,
			// left
			1-tz2, 1-ty2, 1-tz1, 1-ty1,
			// back
			1-tx2, 1-ty2, 1-tx1, 1-ty1,
			// front
			tx1, 1-ty2, tx2, 1-ty1,
		};
		meshgen_cuboid(data,n,p, pos, box, tiles, 6, selected, txc,v3s16(0,0,0),v3f(0,0,0), cols);
	}
}

void meshgen_stairlike(MeshMakeData *data, v3s16 p, MapNode &n, SelectedNode &selected)
{
	static v3s16 faces[6] = {
		v3s16( 0, 1, 0),
		v3s16( 0,-1, 0),
		v3s16( 1, 0, 0),
		v3s16(-1, 0, 0),
		v3s16( 0, 0, 1),
		v3s16( 0, 0,-1),
	};
	v3f pos = intToFloat(p, BS);
	s16 rot = n.getRotationAngle();

	// remove rotation from the node, we'll do it ourselves
	{
		content_t c = n.getContent();
		n.setContent(c);
	}

	TileSpec tiles[6];
	for (int i=0; i<6; i++) {
		// Handles facedir rotation for textures
		tiles[i] = getNodeTile(n,p,faces[i],selected);
	}

	bool urot = (n.getContent() >= CONTENT_SLAB_STAIR_UD_MIN && n.getContent() <= CONTENT_SLAB_STAIR_UD_MAX);

	video::S3DVertex vertices[6][16] = {
		{ // up
			video::S3DVertex(-0.5*data->m_BS, 0.5*data->m_BS , 0.5*data->m_BS , 0,0,0, video::SColor(255,255,255,255), 0.,0.),
			video::S3DVertex( 0.5*data->m_BS, 0.5*data->m_BS , 0.5*data->m_BS , 0,0,0, video::SColor(255,255,255,255), 1.,0.),
			video::S3DVertex( 0.5*data->m_BS, 0.5*data->m_BS , 0.25*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 1.,0.25),
			video::S3DVertex(-0.5*data->m_BS, 0.5*data->m_BS , 0.25*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 0.,0.25),

			video::S3DVertex(-0.5*data->m_BS, 0.25*data->m_BS, 0.25*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 0.,0.25),
			video::S3DVertex( 0.5*data->m_BS, 0.25*data->m_BS, 0.25*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 1.,0.25),
			video::S3DVertex( 0.5*data->m_BS, 0.25*data->m_BS, 0.     , 0,0,0, video::SColor(255,255,255,255), 1.,0.5),
			video::S3DVertex(-0.5*data->m_BS, 0.25*data->m_BS, 0.     , 0,0,0, video::SColor(255,255,255,255), 0.,0.5),

			video::S3DVertex(-0.5*data->m_BS, data->m_BSd, data->m_BSd, 0,0,0, video::SColor(255,255,255,255), 0.,0.5),
			video::S3DVertex( 0.5*data->m_BS, data->m_BSd, data->m_BSd, 0,0,0, video::SColor(255,255,255,255), 1.,0.5),
			video::S3DVertex( 0.5*data->m_BS, data->m_BSd,-0.25*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 1.,0.75),
			video::S3DVertex(-0.5*data->m_BS, data->m_BSd,-0.25*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 0.,0.75),

			video::S3DVertex(-0.5*data->m_BS,-0.25*BS+data->m_BSd,-0.25*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 0.,0.75),
			video::S3DVertex( 0.5*data->m_BS,-0.25*BS+data->m_BSd,-0.25*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 1.,0.75),
			video::S3DVertex( 0.5*data->m_BS,-0.25*BS+data->m_BSd,-0.5*data->m_BS , 0,0,0, video::SColor(255,255,255,255), 1.,1.),
			video::S3DVertex(-0.5*data->m_BS,-0.25*BS+data->m_BSd,-0.5*data->m_BS , 0,0,0, video::SColor(255,255,255,255), 0.,1.)
		},{ // down
			video::S3DVertex(-0.5*data->m_BS,-0.5*data->m_BS ,-0.5*data->m_BS , 0,0,0, video::SColor(255,255,255,255), 0.,0.),
			video::S3DVertex( 0.5*data->m_BS,-0.5*data->m_BS ,-0.5*data->m_BS , 0,0,0, video::SColor(255,255,255,255), 1.,0.),
			video::S3DVertex( 0.5*data->m_BS,-0.5*data->m_BS , 0.5*data->m_BS , 0,0,0, video::SColor(255,255,255,255), 1.,1.),
			video::S3DVertex(-0.5*data->m_BS,-0.5*data->m_BS , 0.5*data->m_BS , 0,0,0, video::SColor(255,255,255,255), 0.,1.)
		},{ // right
			video::S3DVertex( 0.5*data->m_BS, 0.5*data->m_BS , 0.5*data->m_BS , 0,0,0, video::SColor(255,255,255,255), 1.,0.),
			video::S3DVertex( 0.5*data->m_BS,-0.5*data->m_BS , 0.5*data->m_BS , 0,0,0, video::SColor(255,255,255,255), 1.,1.),
			video::S3DVertex( 0.5*data->m_BS,-0.5*data->m_BS ,-0.5*data->m_BS , 0,0,0, video::SColor(255,255,255,255), 0.,1.),

			video::S3DVertex( 0.5*data->m_BS, 0.5*data->m_BS , 0.5*data->m_BS , 0,0,0, video::SColor(255,255,255,255), 1.,0.),
			video::S3DVertex( 0.5*data->m_BS, 0.25*data->m_BS, 0.25*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 0.75,0.25),
			video::S3DVertex( 0.5*data->m_BS, 0.5*data->m_BS , 0.25*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 0.75,0.),

			video::S3DVertex( 0.5*data->m_BS, 0.25*data->m_BS, 0.25*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 0.75,0.25),
			video::S3DVertex( 0.5*data->m_BS, data->m_BSd, data->m_BSd, 0,0,0, video::SColor(255,255,255,255), 0.5,0.5),
			video::S3DVertex( 0.5*data->m_BS, 0.25*data->m_BS, data->m_BSd, 0,0,0, video::SColor(255,255,255,255), 0.5,0.25),

			video::S3DVertex( 0.5*data->m_BS, data->m_BSd, data->m_BSd, 0,0,0, video::SColor(255,255,255,255), 0.5,0.5),
			video::S3DVertex( 0.5*data->m_BS,-0.25*data->m_BS,-0.25*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 0.25,0.75),
			video::S3DVertex( 0.5*data->m_BS, data->m_BSd,-0.25*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 0.5,0.75),

			video::S3DVertex( 0.5*data->m_BS,-0.25*data->m_BS,-0.25*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 0.25,0.75),
			video::S3DVertex( 0.5*data->m_BS,-0.5*data->m_BS ,-0.5*data->m_BS , 0,0,0, video::SColor(255,255,255,255), 0.,1.),
			video::S3DVertex( 0.5*data->m_BS,-0.25*data->m_BS,-0.5*data->m_BS , 0,0,0, video::SColor(255,255,255,255), 0.,0.75)
		},{ // left
			video::S3DVertex(-0.5*data->m_BS, 0.5*data->m_BS , 0.5*data->m_BS , 0,0,0, video::SColor(255,255,255,255), 0.,0.),
			video::S3DVertex(-0.5*data->m_BS,-0.5*data->m_BS ,-0.5*data->m_BS , 0,0,0, video::SColor(255,255,255,255), 1.,1.),
			video::S3DVertex(-0.5*data->m_BS,-0.5*data->m_BS , 0.5*data->m_BS , 0,0,0, video::SColor(255,255,255,255), 0.,1.),

			video::S3DVertex(-0.5*data->m_BS, 0.5*data->m_BS , 0.5*data->m_BS , 0,0,0, video::SColor(255,255,255,255), 0.,0.),
			video::S3DVertex(-0.5*data->m_BS, 0.5*data->m_BS , 0.25*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 0.25,0.),
			video::S3DVertex(-0.5*data->m_BS, 0.25*data->m_BS, 0.25*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 0.25,0.25),

			video::S3DVertex(-0.5*data->m_BS, 0.25*data->m_BS, 0.25*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 0.25,0.25),
			video::S3DVertex(-0.5*data->m_BS, 0.25*data->m_BS, data->m_BSd, 0,0,0, video::SColor(255,255,255,255), 0.5,0.25),
			video::S3DVertex(-0.5*data->m_BS, data->m_BSd, data->m_BSd, 0,0,0, video::SColor(255,255,255,255), 0.5,0.5),

			video::S3DVertex(-0.5*data->m_BS, data->m_BSd, data->m_BSd, 0,0,0, video::SColor(255,255,255,255), 0.5,0.5),
			video::S3DVertex(-0.5*data->m_BS, data->m_BSd,-0.25*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 0.75,0.5),
			video::S3DVertex(-0.5*data->m_BS,-0.25*data->m_BS,-0.25*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 0.75,0.75),

			video::S3DVertex(-0.5*data->m_BS,-0.25*data->m_BS,-0.25*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 0.75,0.75),
			video::S3DVertex(-0.5*data->m_BS,-0.25*data->m_BS,-0.5*data->m_BS , 0,0,0, video::SColor(255,255,255,255), 1.,0.75),
			video::S3DVertex(-0.5*data->m_BS,-0.5*data->m_BS ,-0.5*data->m_BS , 0,0,0, video::SColor(255,255,255,255), 1.,1.)
		},{ // back
			video::S3DVertex( 0.5*data->m_BS, 0.5*data->m_BS , 0.5*data->m_BS , 0,0,0, video::SColor(255,255,255,255), 0.,0.),
			video::S3DVertex(-0.5*data->m_BS, 0.5*data->m_BS , 0.5*data->m_BS , 0,0,0, video::SColor(255,255,255,255), 1.,0.),
			video::S3DVertex(-0.5*data->m_BS,-0.5*data->m_BS , 0.5*data->m_BS , 0,0,0, video::SColor(255,255,255,255), 1.,1.),
			video::S3DVertex( 0.5*data->m_BS,-0.5*data->m_BS , 0.5*data->m_BS , 0,0,0, video::SColor(255,255,255,255), 0.,1.)
		},{ // front
			video::S3DVertex(-0.5*data->m_BS, 0.5*data->m_BS , 0.25*BS-data->m_BSd, 0,0,0, video::SColor(255,255,255,255), 0.,0.),
			video::S3DVertex( 0.5*data->m_BS, 0.5*data->m_BS , 0.25*BS-data->m_BSd, 0,0,0, video::SColor(255,255,255,255), 1.,0.),
			video::S3DVertex( 0.5*data->m_BS, 0.25*data->m_BS, 0.25*BS-data->m_BSd, 0,0,0, video::SColor(255,255,255,255), 1.,0.25),
			video::S3DVertex(-0.5*data->m_BS, 0.25*data->m_BS, 0.25*BS-data->m_BSd, 0,0,0, video::SColor(255,255,255,255), 0.,0.25),

			video::S3DVertex(-0.5*data->m_BS, 0.25*data->m_BS, -data->m_BSd, 0,0,0, video::SColor(255,255,255,255), 0.,0.25),
			video::S3DVertex( 0.5*data->m_BS, 0.25*data->m_BS, -data->m_BSd, 0,0,0, video::SColor(255,255,255,255), 1.,0.25),
			video::S3DVertex( 0.5*data->m_BS, data->m_BSd, -data->m_BSd, 0,0,0, video::SColor(255,255,255,255), 1.,0.5),
			video::S3DVertex(-0.5*data->m_BS, data->m_BSd, -data->m_BSd, 0,0,0, video::SColor(255,255,255,255), 0.,0.5),

			video::S3DVertex(-0.5*data->m_BS, data->m_BSd,-0.25*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 0.,0.5),
			video::S3DVertex( 0.5*data->m_BS, data->m_BSd,-0.25*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 1.,0.5),
			video::S3DVertex( 0.5*data->m_BS,-0.25*data->m_BS,-0.25*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 1.,0.75),
			video::S3DVertex(-0.5*data->m_BS,-0.25*data->m_BS,-0.25*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 0.,0.75),

			video::S3DVertex(-0.5*data->m_BS,-0.25*BS+data->m_BSd,-0.5*data->m_BS , 0,0,0, video::SColor(255,255,255,255), 0.,0.75),
			video::S3DVertex( 0.5*data->m_BS,-0.25*BS+data->m_BSd,-0.5*data->m_BS , 0,0,0, video::SColor(255,255,255,255), 1.,0.75),
			video::S3DVertex( 0.5*data->m_BS,-0.5*data->m_BS ,-0.5*data->m_BS , 0,0,0, video::SColor(255,255,255,255), 1.,1.),
			video::S3DVertex(-0.5*data->m_BS,-0.5*data->m_BS ,-0.5*data->m_BS , 0,0,0, video::SColor(255,255,255,255), 0.,1.)
		}
	};

	u16 indices[6][24] = {
		{ // up
			0,1,2,2,3,0,
			4,5,6,6,7,4,
			8,9,10,10,11,8,
			12,13,14,14,15,12
		},{ // down
			0,1,2,2,3,0
		},{ // right
			0,1,2,3,4,5,6,7,8,9,10,11,12,13,14
		},{ // left
			0,1,2,3,4,5,6,7,8,9,10,11,12,13,14
		},{ // back
			0,1,2,2,3,0
		},{ // front
			0,1,2,2,3,0,
			4,5,6,6,7,4,
			8,9,10,10,11,8,
			12,13,14,14,15,12
		}
	};
	v3s16 back(0,0,1);
	v3s16 front(0,0,-1);
	v3s16 left(-1,0,0);
	v3s16 right(1,0,0);
	back.rotateXZBy(rot);
	front.rotateXZBy(rot);
	left.rotateXZBy(rot);
	right.rotateXZBy(rot);
	if (urot) {
		v3s16 r = left;
		left = right;
		right = r;
	}
	MapNode nf = data->m_vmanip.getNodeRO(data->m_blockpos_nodes + p + front);
	MapNode nl = data->m_vmanip.getNodeRO(data->m_blockpos_nodes + p + left);
	MapNode nr = data->m_vmanip.getNodeRO(data->m_blockpos_nodes + p + right);
	ContentFeatures *ff = &content_features(nf);
	ContentFeatures *fl = &content_features(nl);
	ContentFeatures *fr = &content_features(nr);
	s16 vcounts[6] = {16,4,15,15,4,16};
	s16 icounts[6] = {24,6,15,15,6,24};
	// don't draw unseen faces
	bool skips[6] = {false,false,false,false,false,false};
	if (
		ff->draw_type == CDT_SLABLIKE
		|| (
			ff->draw_type == CDT_STAIRLIKE
			&& (
				fl->draw_type == CDT_SLABLIKE
				|| fl->draw_type == CDT_STAIRLIKE
			) && (
				fr->draw_type == CDT_SLABLIKE
				|| fr->draw_type == CDT_STAIRLIKE
			)
		)
	) {
		// slab connection
		vcounts[0] = 12;
		icounts[0] = 18;
		vertices[0][8] =  video::S3DVertex(-0.5*data->m_BS, data->m_BSd, data->m_BSd,     0,0,0, video::SColor(255,255,255,255), 0.,0.5);
		vertices[0][9] =  video::S3DVertex( 0.5*data->m_BS, data->m_BSd, data->m_BSd,     0,0,0, video::SColor(255,255,255,255), 1.,0.5),
		vertices[0][10] = video::S3DVertex( 0.5*data->m_BS, data->m_BSd,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 1.,1.0),
		vertices[0][11] = video::S3DVertex(-0.5*data->m_BS, data->m_BSd,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 0.,1.0),

		vcounts[2] = 12;
		icounts[2] = 12;
		vertices[2][9] =  video::S3DVertex( 0.5*data->m_BS, data->m_BSd, data->m_BSd,     0,0,0, video::SColor(255,255,255,255), 0.5,0.5);
		vertices[2][10] = video::S3DVertex( 0.5*data->m_BS,-0.5*data->m_BS,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 0.,1.);
		vertices[2][11] = video::S3DVertex( 0.5*data->m_BS, data->m_BSd,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 0.,0.5);

		vcounts[3] = 12;
		icounts[3] = 12;
		vertices[3][9] =  video::S3DVertex(-0.5*data->m_BS, data->m_BSd, data->m_BSd,     0,0,0, video::SColor(255,255,255,255), 0.5,0.5);
		vertices[3][10] = video::S3DVertex(-0.5*data->m_BS, data->m_BSd,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 1.0,0.5);
		vertices[3][11] = video::S3DVertex(-0.5*data->m_BS,-0.5*data->m_BS,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 1.0,1.0);

		vcounts[5] = 12;
		icounts[5] = 18;
		vertices[5][8] =  video::S3DVertex(-0.5*data->m_BS, data->m_BSd,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 0.,0.5);
		vertices[5][9] =  video::S3DVertex( 0.5*data->m_BS, data->m_BSd,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 1.,0.5);
		vertices[5][10] = video::S3DVertex( 0.5*data->m_BS,-0.5*data->m_BS,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 1.,1.);
		vertices[5][11] = video::S3DVertex(-0.5*data->m_BS,-0.5*data->m_BS,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 0.,1.);
	}else{
		skips[2] = !meshgen_hardface(data,p,n,right);
		skips[3] = !meshgen_hardface(data,p,n,left);
	}

	skips[4] = !meshgen_hardface(data,p,n,back);

	if (urot) {
		if (rot) {
			for (int i=0; i<6; i++) {
				if (skips[i])
					continue;
				v3s16 f = faces[i];
				f.rotateXYBy(180);
				f.rotateXZBy(rot);
				for (int j=0; j<vcounts[i]; j++) {
					vertices[i][j].Pos.rotateXYBy(180);
					vertices[i][j].Pos.rotateXZBy(rot);
					vertices[i][j].TCoords *= tiles[i].texture.size;
					vertices[i][j].TCoords += tiles[i].texture.pos;
				}
				std::vector<u32> colours;
				if (selected.is_coloured) {
					meshgen_selected_lights(colours,255,vcounts[i]);
				}else{
					meshgen_lights(data,n,p,colours,255,f,vcounts[i],vertices[i]);
				}

				for (int j=0; j<vcounts[i]; j++) {
					vertices[i][j].Pos += pos;
				}

				data->append(tiles[i], vertices[i], vcounts[i], indices[i], icounts[i], colours);
			}
		}else{
			for (int i=0; i<6; i++) {
				if (skips[i])
					continue;
				v3s16 f = faces[i];
				f.rotateXYBy(180);
				for (int j=0; j<vcounts[i]; j++) {
					vertices[i][j].Pos.rotateXYBy(180);
					vertices[i][j].TCoords *= tiles[i].texture.size;
					vertices[i][j].TCoords += tiles[i].texture.pos;
				}
				std::vector<u32> colours;
				if (selected.is_coloured) {
					meshgen_selected_lights(colours,255,vcounts[i]);
				}else{
					meshgen_lights(data,n,p,colours,255,f,vcounts[i],vertices[i]);
				}

				for (int j=0; j<vcounts[i]; j++) {
					vertices[i][j].Pos += pos;
				}

				data->append(tiles[i], vertices[i], vcounts[i], indices[i], icounts[i], colours);
			}
		}
	}else if (rot) {
		for (int i=0; i<6; i++) {
			if (skips[i])
				continue;
			v3s16 f = faces[i];
			f.rotateXZBy(rot);
			for (int j=0; j<vcounts[i]; j++) {
				vertices[i][j].Pos.rotateXZBy(rot);
				vertices[i][j].TCoords *= tiles[i].texture.size;
				vertices[i][j].TCoords += tiles[i].texture.pos;
			}
			std::vector<u32> colours;
			if (selected.is_coloured) {
				meshgen_selected_lights(colours,255,vcounts[i]);
			}else{
				meshgen_lights(data,n,p,colours,255,f,vcounts[i],vertices[i]);
			}

			for (int j=0; j<vcounts[i]; j++) {
				vertices[i][j].Pos += pos;
			}

			data->append(tiles[i], vertices[i], vcounts[i], indices[i], icounts[i], colours);
		}
	}else{
		for (int i=0; i<6; i++) {
			if (skips[i])
				continue;
			for (int j=0; j<vcounts[i]; j++) {
				vertices[i][j].TCoords *= tiles[i].texture.size;
				vertices[i][j].TCoords += tiles[i].texture.pos;
			}
			std::vector<u32> colours;
			if (selected.is_coloured) {
				meshgen_selected_lights(colours,255,vcounts[i]);
			}else{
				meshgen_lights(data,n,p,colours,255,faces[i],vcounts[i],vertices[i]);
			}

			for (int j=0; j<vcounts[i]; j++) {
				vertices[i][j].Pos += pos;
			}

			data->append(tiles[i], vertices[i], vcounts[i], indices[i], icounts[i], colours);
		}
	}
}

void meshgen_slablike(MeshMakeData *data, v3s16 p, MapNode &n, SelectedNode &selected)
{
	static v3s16 faces[6] = {
		v3s16( 0, 1, 0),
		v3s16( 0,-1, 0),
		v3s16( 1, 0, 0),
		v3s16(-1, 0, 0),
		v3s16( 0, 0, 1),
		v3s16( 0, 0,-1),
	};
	static v3s16 ufaces[6] = {
		v3s16( 0,-1, 0),
		v3s16( 0, 1, 0),
		v3s16(-1, 0, 0),
		v3s16( 1, 0, 0),
		v3s16( 0, 0, 1),
		v3s16( 0, 0,-1),
	};

	TileSpec tiles[6];
	for (int i = 0; i < 6; i++) {
		// Handles facedir rotation for textures
		tiles[i] = getNodeTile(n,p,faces[i],selected);
	}
	v3f pos = intToFloat(p, BS);

	bool urot = (n.getContent() >= CONTENT_SLAB_STAIR_UD_MIN && n.getContent() <= CONTENT_SLAB_STAIR_UD_MAX);
	float tex_v = 0.;
	if (urot)
		tex_v = 0.5;
	// flip lighting
	video::S3DVertex vertices[6][4] = {
		{ // up
			video::S3DVertex(-0.5*data->m_BS, data->m_BSd, 0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 0.,0.),
			video::S3DVertex( 0.5*data->m_BS, data->m_BSd, 0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 1.,0.),
			video::S3DVertex( 0.5*data->m_BS, data->m_BSd,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 1.,1.),
			video::S3DVertex(-0.5*data->m_BS, data->m_BSd,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 0.,1.)
		},{ // down
			video::S3DVertex(-0.5*data->m_BS,-0.5*data->m_BS,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 0.,0.),
			video::S3DVertex( 0.5*data->m_BS,-0.5*data->m_BS,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 1.,0.),
			video::S3DVertex( 0.5*data->m_BS,-0.5*data->m_BS, 0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 1.,1.),
			video::S3DVertex(-0.5*data->m_BS,-0.5*data->m_BS, 0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 0.,1.)
		},{ // right
			video::S3DVertex( 0.5*data->m_BS, data->m_BSd,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 0.,0.+tex_v),
			video::S3DVertex( 0.5*data->m_BS, data->m_BSd, 0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 1.,0.+tex_v),
			video::S3DVertex( 0.5*data->m_BS,-0.5*data->m_BS, 0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 1.,.5+tex_v),
			video::S3DVertex( 0.5*data->m_BS,-0.5*data->m_BS,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 0.,.5+tex_v)
		},{ // left
			video::S3DVertex(-0.5*data->m_BS, data->m_BSd, 0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 0.,0.+tex_v),
			video::S3DVertex(-0.5*data->m_BS, data->m_BSd,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 1.,0.+tex_v),
			video::S3DVertex(-0.5*data->m_BS,-0.5*data->m_BS,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 1.,.5+tex_v),
			video::S3DVertex(-0.5*data->m_BS,-0.5*data->m_BS, 0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 0.,.5+tex_v)
		},{ // back
			video::S3DVertex( 0.5*data->m_BS, data->m_BSd, 0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 0.,0.+tex_v),
			video::S3DVertex(-0.5*data->m_BS, data->m_BSd, 0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 1.,0.+tex_v),
			video::S3DVertex(-0.5*data->m_BS,-0.5*data->m_BS, 0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 1.,.5+tex_v),
			video::S3DVertex( 0.5*data->m_BS,-0.5*data->m_BS, 0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 0.,.5+tex_v)
		},{ // front
			video::S3DVertex(-0.5*data->m_BS, data->m_BSd,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 0.,0.+tex_v),
			video::S3DVertex( 0.5*data->m_BS, data->m_BSd,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 1.,0.+tex_v),
			video::S3DVertex( 0.5*data->m_BS,-0.5*data->m_BS,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 1.,.5+tex_v),
			video::S3DVertex(-0.5*data->m_BS,-0.5*data->m_BS,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), 0.,.5+tex_v)
		}
	};
	u16 indices[6] = {0,1,2,2,3,0};
	// don't draw unseen faces
	bool skips[6] = {false,false,false,false,false,false};

	if (urot) {
		for (u16 i=0; i<6; i++) {
			skips[i] = !meshgen_hardface(data,p,n,ufaces[i]);
		}
		for (int i=0; i<6; i++) {
			if (skips[i])
				continue;
			for (int j=0; j<4; j++) {
				vertices[i][j].Pos.rotateXYBy(180);
				vertices[i][j].TCoords *= tiles[i].texture.size;
				vertices[i][j].TCoords += tiles[i].texture.pos;
			}
			std::vector<u32> colours;
			if (selected.is_coloured) {
				meshgen_selected_lights(colours,255,4);
			}else{
				meshgen_lights(data,n,p,colours,255,ufaces[i],4,vertices[i]);
			}

			for (int j=0; j<4; j++) {
				vertices[i][j].Pos += pos;
			}

			data->append(tiles[i], vertices[i], 4, indices, 6, colours);
		}
	}else{
		for (u16 i=0; i<6; i++) {
			skips[i] = !meshgen_hardface(data,p,n,faces[i]);
		}
		for (int i=0; i<6; i++) {
			if (skips[i])
				continue;
			for (int j=0; j<4; j++) {
				vertices[i][j].TCoords *= tiles[i].texture.size;
				vertices[i][j].TCoords += tiles[i].texture.pos;
			}
			std::vector<u32> colours;
			if (selected.is_coloured) {
				meshgen_selected_lights(colours,255,4);
			}else{
				meshgen_lights(data,n,p,colours,255,faces[i],4,vertices[i]);
			}

			for (int j=0; j<4; j++) {
				vertices[i][j].Pos += pos;
			}

			data->append(tiles[i], vertices[i], 4, indices, 6, colours);
		}
	}
}

void meshgen_trunklike(MeshMakeData *data, v3s16 p, MapNode &n, SelectedNode &selected)
{
	bool x_plus = false;
	bool x_plus_any = false;
	bool y_plus = false;
	bool y_plus_any = false;
	bool z_plus = false;
	bool z_plus_any = false;
	bool x_minus = false;
	bool x_minus_any = false;
	bool y_minus = false;
	bool y_minus_any = false;
	bool z_minus = false;
	bool z_minus_any = false;
	content_t n2;
	bool mud_under = false;

	float bottom_scale = 1.0;
	float top_scale = 1.0;
	int height = 0;
	int dir = 0;

	content_t thiscontent = n.getContent();

	if (data->mesh_detail == 1) {
		meshgen_cubelike(data,p,n,selected);
		return;
	}

	n2 = data->m_vmanip.getNodeRO(data->m_blockpos_nodes + p + v3s16(1,0,0)).getContent();
	if (n2 == thiscontent) {
		x_plus = true;
		x_plus_any = true;
	}else if (!meshgen_hardface(data,p,n,v3s16(1,0,0))) {
		x_plus_any = true;
	}

	n2 = data->m_vmanip.getNodeRO(data->m_blockpos_nodes + p + v3s16(0,1,0)).getContent();
	if (n2 == thiscontent) {
		y_plus = true;
		y_plus_any = true;
	}else if (!meshgen_hardface(data,p,n,v3s16(0,1,0))) {
		y_plus_any = true;
	}

	n2 = data->m_vmanip.getNodeRO(data->m_blockpos_nodes + p + v3s16(0,0,1)).getContent();
	if (n2 == thiscontent) {
		z_plus = true;
		z_plus_any = true;
	}else if (!meshgen_hardface(data,p,n,v3s16(0,0,1))) {
		z_plus_any = true;
	}

	n2 = data->m_vmanip.getNodeRO(data->m_blockpos_nodes + p + v3s16(-1,0,0)).getContent();
	if (n2 == thiscontent) {
		x_minus = true;
		x_minus_any = true;
	}else if (!meshgen_hardface(data,p,n,v3s16(-1,0,0))) {
		x_minus_any = true;
	}

	n2 = data->m_vmanip.getNodeRO(data->m_blockpos_nodes + p + v3s16(0,-1,0)).getContent();
	if (n2 == thiscontent) {
		y_minus = true;
		y_minus_any = true;
	}else if (!meshgen_hardface(data,p,n,v3s16(0,-1,0))) {
		y_minus_any = true;
		if (n2 == CONTENT_MUD)
			mud_under = true;
	}

	n2 = data->m_vmanip.getNodeRO(data->m_blockpos_nodes + p + v3s16(0,0,-1)).getContent();
	if (n2 == thiscontent) {
		z_minus = true;
		z_minus_any = true;
	}else if (!meshgen_hardface(data,p,n,v3s16(0,0,-1))) {
		z_minus_any = true;
	}

	if (content_features(thiscontent).param_type == CPT_BLOCKDATA) {
		height = n.param1&0x1F;
		dir = (n.param1&0xE0)>>5;
	}

	if (height || dir) {
		int top_height = height+1;
		int bottom_height = height;
		if (top_height > 16)
			top_height = 16;
		if (bottom_height > 16)
			bottom_height = 16;
		if (dir == 1 || dir == 4) {
			bottom_scale = (0.0625*(16.0-((float)top_height)));
			top_scale = (0.0625*(16.0-((float)bottom_height)));
		}else{
			bottom_scale = (0.0625*(16.0-((float)bottom_height)));
			top_scale = (0.0625*(16.0-((float)top_height)));
		}
		if (dir) {
			if (dir == 1 || dir == 2) {
				x_plus = true;
				x_minus = true;
			}else if (dir == 3 || dir == 4) {
				z_plus = true;
				z_minus = true;
			}
		}
	}

	TileSpec tile = getNodeTile(n,p,v3s16(1,0,0),selected);
	TileSpec endtile = getNodeTile(n,p,v3s16(0,1,0),selected);
	video::S3DVertex vertices[10] = {
		video::S3DVertex(0               ,-data->m_BS*0.5,data->m_BS*0.499*(bottom_scale), 0,0,0, video::SColor(255,255,255,255), 0.125, 0.),
		video::S3DVertex(data->m_BS*0.125*(bottom_scale),-data->m_BS*0.5,data->m_BS*0.499*(bottom_scale), 0,0,0, video::SColor(255,255,255,255), 0.25, 0.),
		video::S3DVertex(data->m_BS*0.375*(bottom_scale),-data->m_BS*0.5,data->m_BS*0.375*(bottom_scale), 0,0,0, video::SColor(255,255,255,255), 0.5, 0.),
		video::S3DVertex(data->m_BS*0.499*(bottom_scale),-data->m_BS*0.5,data->m_BS*0.125*(bottom_scale), 0,0,0, video::SColor(255,255,255,255), 0.75, 0.),
		video::S3DVertex(data->m_BS*0.499*(bottom_scale),-data->m_BS*0.5,0       , 0,0,0, video::SColor(255,255,255,255), 0.625, 0.),

		video::S3DVertex(0               , data->m_BS*0.5,data->m_BS*0.499*top_scale, 0,0,0, video::SColor(255,255,255,255), 0.125, 1.0),
		video::S3DVertex(data->m_BS*0.125*top_scale, data->m_BS*0.5,data->m_BS*0.499*top_scale, 0,0,0, video::SColor(255,255,255,255), 0.25, 1.0),
		video::S3DVertex(data->m_BS*0.375*top_scale, data->m_BS*0.5,data->m_BS*0.375*top_scale, 0,0,0, video::SColor(255,255,255,255), 0.5, 1.0),
		video::S3DVertex(data->m_BS*0.499*top_scale, data->m_BS*0.5,data->m_BS*0.125*top_scale, 0,0,0, video::SColor(255,255,255,255), 0.75, 1.0),
		video::S3DVertex(data->m_BS*0.499*top_scale, data->m_BS*0.5,0       , 0,0,0, video::SColor(255,255,255,255), 0.625, 1.0)
	};
	video::S3DVertex branch_vertices[10] = {
		video::S3DVertex(0       ,data->m_BS*0.125,data->m_BS*0.499*top_scale, 0,0,0, video::SColor(255,255,255,255), 0.125, 0.625),
		video::S3DVertex(data->m_BS*0.125*top_scale,data->m_BS*0.125,data->m_BS*0.499*top_scale, 0,0,0, video::SColor(255,255,255,255), 0.25, 0.625),
		video::S3DVertex(data->m_BS*0.375*top_scale,data->m_BS*0.125,data->m_BS*0.375*top_scale, 0,0,0, video::SColor(255,255,255,255), 0.5, 0.625),
		video::S3DVertex(data->m_BS*0.499*top_scale,data->m_BS*0.125,data->m_BS*0.125*top_scale, 0,0,0, video::SColor(255,255,255,255), 0.75, 0.625),
		video::S3DVertex(data->m_BS*0.499*top_scale,data->m_BS*0.125,0       , 0,0,0, video::SColor(255,255,255,255), 0.625, 0.625),

		video::S3DVertex(0       ,data->m_BS*0.5  ,data->m_BS*0.499*top_scale, 0,0,0, video::SColor(255,255,255,255), 0.125, 1.0),
		video::S3DVertex(data->m_BS*0.125*top_scale,data->m_BS*0.5  ,data->m_BS*0.499*top_scale, 0,0,0, video::SColor(255,255,255,255), 0.25, 1.0),
		video::S3DVertex(data->m_BS*0.375*top_scale,data->m_BS*0.5  ,data->m_BS*0.375*top_scale, 0,0,0, video::SColor(255,255,255,255), 0.5, 1.0),
		video::S3DVertex(data->m_BS*0.499*top_scale,data->m_BS*0.5  ,data->m_BS*0.125*top_scale, 0,0,0, video::SColor(255,255,255,255), 0.75, 1.0),
		video::S3DVertex(data->m_BS*0.499*top_scale,data->m_BS*0.5  ,0       , 0,0,0, video::SColor(255,255,255,255), 0.625, 1.0)
	};
	u16 indices[24] = {0,1,6,0,6,5,1,2,7,1,7,6,2,3,8,2,8,7,3,4,9,3,9,8};
	video::S3DVertex end_vertices[6] = {
		video::S3DVertex(data->m_BS*0.5  ,data->m_BS*0.5,0       , 0,0,0, video::SColor(255,255,255,255), 0.5, 0.),
		video::S3DVertex(data->m_BS*0.5  ,data->m_BS*0.5,data->m_BS*0.125, 0,0,0, video::SColor(255,255,255,255), 0.625, 0.),
		video::S3DVertex(data->m_BS*0.375,data->m_BS*0.5,data->m_BS*0.375, 0,0,0, video::SColor(255,255,255,255), 0.875, 0.125),
		video::S3DVertex(data->m_BS*0.125,data->m_BS*0.5,data->m_BS*0.5  , 0,0,0, video::SColor(255,255,255,255), 1.0, 0.375),
		video::S3DVertex(0       ,data->m_BS*0.5,data->m_BS*0.5  , 0,0,0, video::SColor(255,255,255,255), 1.0, 0.5),
		video::S3DVertex(0       ,data->m_BS*0.5,0       , 0,0,0, video::SColor(255,255,255,255), 0.5, 0.5)
	};
	u16 end_indices[12] = {5,1,0,5,2,1,5,3,2,5,4,3};
	video::S3DVertex base_vertices[6] = {
		video::S3DVertex(data->m_BS*0.498,-data->m_BS*0.5 ,data->m_BS*0.498, 0,0,0, video::SColor(255,255,255,255), 0., 0.),
		video::S3DVertex(data->m_BS*0.498,-data->m_BS*0.5 ,data->m_BS*0.125, 0,0,0, video::SColor(255,255,255,255), 0.375, 0.),
		video::S3DVertex(data->m_BS*0.125,-data->m_BS*0.5 ,data->m_BS*0.498, 0,0,0, video::SColor(255,255,255,255), 0.375, 0.),
		video::S3DVertex(data->m_BS*0.498*top_scale, data->m_BS*0.25,data->m_BS*0.125*top_scale, 0,0,0, video::SColor(255,255,255,255), 0.375, 0.75),
		video::S3DVertex(data->m_BS*0.375*top_scale, data->m_BS*0.25,data->m_BS*0.375*top_scale, 0,0,0, video::SColor(255,255,255,255), 0.125, 0.75),
		video::S3DVertex(data->m_BS*0.125*top_scale, data->m_BS*0.25,data->m_BS*0.498*top_scale, 0,0,0, video::SColor(255,255,255,255), 0.375, 0.75),
	};
	u16 base_indices[12] = {0,1,3,0,5,2,0,3,4,0,4,5};
	u16 rots[4] = {0,90,180,270};
	v3s16 faces[3] = {
		v3s16(0,0,1),
		v3s16(1,0,1),
		v3s16(1,0,0)
	};

	v3f pos = intToFloat(p,BS);

	if (y_plus || y_minus || (!x_plus && !x_minus && !z_plus && !z_minus && mud_under)) { /* vertical trunk */
		for (u16 j=0; j<4; j++) {
			video::S3DVertex v[10];
			for (u16 i=0; i<10; i++) {
				v[i] = vertices[i];
				v[i].Pos.rotateXZBy(rots[j]);
				v[i].TCoords *= tile.texture.size;
				v[i].TCoords += tile.texture.pos;
			}
			std::vector<u32> colours;
			if (selected.is_coloured) {
				meshgen_selected_lights(colours,255,10);
			}else{
				v3s16 f[3];
				for (u16 i=0; i<3; i++) {
					f[i] = faces[i];
					f[i].rotateXZBy(rots[j]);
				}
				meshgen_lights(data,n,p,colours,255,f[0],2,v);
				meshgen_lights(data,n,p,colours,255,f[1],1,&v[2]);
				meshgen_lights(data,n,p,colours,255,f[2],2,&v[3]);
				meshgen_lights(data,n,p,colours,255,f[0],2,&v[5]);
				meshgen_lights(data,n,p,colours,255,f[1],1,&v[7]);
				meshgen_lights(data,n,p,colours,255,f[2],2,&v[8]);
			}

			for (int k=0; k<10; k++) {
				v[k].Pos += pos;
			}

			data->append(tile, v, 10, indices, 24, colours);
		}
		if (!y_plus_any) {
			for (u16 j=0; j<4; j++) {
				video::S3DVertex v[6];
				for (u16 i=0; i<6; i++) {
					v[i] = end_vertices[i];
					v[i].Pos.X *= top_scale;
					v[i].Pos.Z *= top_scale;
					v[i].Pos.rotateXZBy(rots[j]);
					v[i].TCoords *= endtile.texture.size;
					v[i].TCoords += endtile.texture.pos;
				}
				std::vector<u32> colours;
				if (selected.is_coloured) {
					meshgen_selected_lights(colours,255,6);
				}else{
					meshgen_lights(data,n,p,colours,255,v3s16(0,1,0),6,v);
				}

				for (int k=0; k<6; k++) {
					v[k].Pos += pos;
				}

				data->append(endtile, v, 6, end_indices, 12, colours);
			}
		}
		if (!y_minus_any) {
			for (u16 j=0; j<4; j++) {
				video::S3DVertex v[6];
				for (u16 i=0; i<6; i++) {
					v[i] = end_vertices[i];
					v[i].Pos.X *= bottom_scale;
					v[i].Pos.Z *= bottom_scale;
					v[i].Pos.rotateXYBy(180);
					v[i].Pos.rotateXZBy(rots[j]);
					v[i].TCoords *= endtile.texture.size;
					v[i].TCoords += endtile.texture.pos;
				}
				std::vector<u32> colours;
				if (selected.is_coloured) {
					meshgen_selected_lights(colours,255,6);
				}else{
					meshgen_lights(data,n,p,colours,255,v3s16(0,-1,0),6,v);
				}

				for (int k=0; k<6; k++) {
					v[k].Pos += pos;
				}

				data->append(endtile, v, 6, end_indices, 12, colours);
			}
		}else if (mud_under) {
			for (u16 j=0; j<4; j++) {
				video::S3DVertex v[6];
				for (u16 i=0; i<6; i++) {
					v[i] = base_vertices[i];
					v[i].Pos.rotateXZBy(rots[j]);
					v[i].TCoords *= tile.texture.size;
					v[i].TCoords += tile.texture.pos;
				}
				std::vector<u32> colours;
				if (selected.is_coloured) {
					meshgen_selected_lights(colours,255,6);
				}else{
					meshgen_lights(data,n,p,colours,255,v3s16(0,1,0),6,v);
				}

				for (int k=0; k<6; k++) {
					v[k].Pos += pos;
				}

				data->append(tile, v, 6, base_indices, 12, colours);
			}
		}
		if (x_plus) {
			for (u16 j=0; j<4; j++) {
				video::S3DVertex v[10];
				for (u16 i=0; i<10; i++) {
					v[i] = branch_vertices[i];
					v[i].Pos += v3f(0,-data->m_BS*0.625,0);
					v[i].Pos.rotateXYBy(90);
					v[i].Pos.rotateYZBy(rots[j]);
					v[i].TCoords *= tile.texture.size;
					v[i].TCoords += tile.texture.pos;
				}
				std::vector<u32> colours;
				if (selected.is_coloured) {
					meshgen_selected_lights(colours,255,10);
				}else{
					v3s16 f[3];
					for (u16 i=0; i<3; i++) {
						f[i] = faces[i];
						f[i].rotateXYBy(90);
						f[i].rotateYZBy(rots[j]);
					}
					meshgen_lights(data,n,p,colours,255,f[0],2,v);
					meshgen_lights(data,n,p,colours,255,f[1],1,&v[2]);
					meshgen_lights(data,n,p,colours,255,f[2],2,&v[3]);
					meshgen_lights(data,n,p,colours,255,f[0],2,&v[5]);
					meshgen_lights(data,n,p,colours,255,f[1],1,&v[7]);
					meshgen_lights(data,n,p,colours,255,f[2],2,&v[8]);
				}

				for (int k=0; k<10; k++) {
					v[k].Pos += pos;
				}

				data->append(tile, v, 10, indices, 24, colours);
			}
		}
		if (x_minus) {
			for (u16 j=0; j<4; j++) {
				video::S3DVertex v[10];
				for (u16 i=0; i<10; i++) {
					v[i] = branch_vertices[i];
					v[i].Pos.rotateXYBy(90);
					v[i].Pos.rotateYZBy(rots[j]);
					v[i].TCoords *= tile.texture.size;
					v[i].TCoords += tile.texture.pos;
				}
				std::vector<u32> colours;
				if (selected.is_coloured) {
					meshgen_selected_lights(colours,255,10);
				}else{
					v3s16 f[3];
					for (u16 i=0; i<3; i++) {
						f[i] = faces[i];
						f[i].rotateXYBy(90);
						f[i].rotateYZBy(rots[j]);
					}
					meshgen_lights(data,n,p,colours,255,f[0],2,v);
					meshgen_lights(data,n,p,colours,255,f[1],1,&v[2]);
					meshgen_lights(data,n,p,colours,255,f[2],2,&v[3]);
					meshgen_lights(data,n,p,colours,255,f[0],2,&v[5]);
					meshgen_lights(data,n,p,colours,255,f[1],1,&v[7]);
					meshgen_lights(data,n,p,colours,255,f[2],2,&v[8]);
				}

				for (int k=0; k<10; k++) {
					v[k].Pos += pos;
				}

				data->append(tile, v, 10, indices, 24, colours);
			}
		}
		if (z_plus) {
			for (u16 j=0; j<4; j++) {
				video::S3DVertex v[10];
				for (u16 i=0; i<10; i++) {
					v[i] = branch_vertices[i];
					v[i].Pos.rotateYZBy(90);
					v[i].Pos.rotateXYBy(rots[j]);
					v[i].TCoords *= tile.texture.size;
					v[i].TCoords += tile.texture.pos;
				}
				std::vector<u32> colours;
				if (selected.is_coloured) {
					meshgen_selected_lights(colours,255,10);
				}else{
					v3s16 f[3];
					for (u16 i=0; i<3; i++) {
						f[i] = faces[i];
						f[i].rotateYZBy(90);
						f[i].rotateXYBy(rots[j]);
					}
					meshgen_lights(data,n,p,colours,255,f[0],2,v);
					meshgen_lights(data,n,p,colours,255,f[1],1,&v[2]);
					meshgen_lights(data,n,p,colours,255,f[2],2,&v[3]);
					meshgen_lights(data,n,p,colours,255,f[0],2,&v[5]);
					meshgen_lights(data,n,p,colours,255,f[1],1,&v[7]);
					meshgen_lights(data,n,p,colours,255,f[2],2,&v[8]);
				}

				for (int k=0; k<10; k++) {
					v[k].Pos += pos;
				}

				data->append(tile, v, 10, indices, 24, colours);
			}
		}
		if (z_minus) {
			for (u16 j=0; j<4; j++) {
				video::S3DVertex v[10];
				for (u16 i=0; i<10; i++) {
					v[i] = branch_vertices[i];
					v[i].Pos += v3f(0,-data->m_BS*0.625,0);
					v[i].Pos.rotateYZBy(90);
					v[i].Pos.rotateXYBy(rots[j]);
					v[i].TCoords *= tile.texture.size;
					v[i].TCoords += tile.texture.pos;
				}
				std::vector<u32> colours;
				if (selected.is_coloured) {
					meshgen_selected_lights(colours,255,10);
				}else{
					v3s16 f[3];
					for (u16 i=0; i<3; i++) {
						f[i] = faces[i];
						f[i].rotateYZBy(90);
						f[i].rotateXYBy(rots[j]);
					}
					meshgen_lights(data,n,p,colours,255,f[0],2,v);
					meshgen_lights(data,n,p,colours,255,f[1],1,&v[2]);
					meshgen_lights(data,n,p,colours,255,f[2],2,&v[3]);
					meshgen_lights(data,n,p,colours,255,f[0],2,&v[5]);
					meshgen_lights(data,n,p,colours,255,f[1],1,&v[7]);
					meshgen_lights(data,n,p,colours,255,f[2],2,&v[8]);
				}

				for (int k=0; k<10; k++) {
					v[k].Pos += pos;
				}

				data->append(tile, v, 10, indices, 24, colours);
			}
		}
	}else{ /* horizontal trunk */
		if ((x_plus && x_minus) || ((x_plus || x_minus) && (!z_plus || !z_minus))) { // centred along X
			for (u16 j=0; j<4; j++) {
				video::S3DVertex v[10];
				for (u16 i=0; i<10; i++) {
					v[i] = vertices[i];
					v[i].Pos.rotateXYBy(90);
					v[i].Pos.rotateYZBy(rots[j]);
					v[i].TCoords *= tile.texture.size;
					v[i].TCoords += tile.texture.pos;
				}
				std::vector<u32> colours;
				if (selected.is_coloured) {
					meshgen_selected_lights(colours,255,10);
				}else{
					v3s16 f[3];
					for (u16 i=0; i<3; i++) {
						f[i] = faces[i];
						f[i].rotateXYBy(90);
						f[i].rotateYZBy(rots[j]);
					}
					meshgen_lights(data,n,p,colours,255,f[0],2,v);
					meshgen_lights(data,n,p,colours,255,f[1],1,&v[2]);
					meshgen_lights(data,n,p,colours,255,f[2],2,&v[3]);
					meshgen_lights(data,n,p,colours,255,f[0],2,&v[5]);
					meshgen_lights(data,n,p,colours,255,f[1],1,&v[7]);
					meshgen_lights(data,n,p,colours,255,f[2],2,&v[8]);
				}

				for (int k=0; k<10; k++) {
					v[k].Pos += pos;
				}

				data->append(tile, v, 10, indices, 24, colours);
			}
			if (!x_plus_any) {
				for (u16 j=0; j<4; j++) {
					video::S3DVertex v[6];
					for (u16 i=0; i<6; i++) {
						v[i] = end_vertices[i];
						if (dir) {
							v[i].Pos.X *= bottom_scale;
							v[i].Pos.Z *= bottom_scale;
						}
						v[i].Pos.rotateXYBy(-90);
						v[i].Pos.rotateYZBy(rots[j]);
						v[i].TCoords *= endtile.texture.size;
						v[i].TCoords += endtile.texture.pos;
					}
					std::vector<u32> colours;
					if (selected.is_coloured) {
						meshgen_selected_lights(colours,255,6);
					}else{
						meshgen_lights(data,n,p,colours,255,v3s16(1,0,0),6,v);
					}

					for (int k=0; k<6; k++) {
						v[k].Pos += pos;
					}

					data->append(endtile, v, 6, end_indices, 12, colours);
				}
			}
			if (!x_minus_any) {
				for (u16 j=0; j<4; j++) {
					video::S3DVertex v[6];
					for (u16 i=0; i<6; i++) {
						v[i] = end_vertices[i];
						if (dir) {
							v[i].Pos.X *= top_scale;
							v[i].Pos.Z *= top_scale;
						}
						v[i].Pos.rotateXYBy(90);
						v[i].Pos.rotateYZBy(rots[j]);
						v[i].TCoords *= endtile.texture.size;
						v[i].TCoords += endtile.texture.pos;
					}
					std::vector<u32> colours;
					if (selected.is_coloured) {
						meshgen_selected_lights(colours,255,6);
					}else{
						meshgen_lights(data,n,p,colours,255,v3s16(-1,0,0),6,v);
					}

					for (int k=0; k<6; k++) {
						v[k].Pos += pos;
					}

					data->append(endtile, v, 6, end_indices, 12, colours);
				}
			}
			if (z_plus) {
				for (u16 j=0; j<4; j++) {
					video::S3DVertex v[10];
					for (u16 i=0; i<10; i++) {
						v[i] = branch_vertices[i];
						v[i].Pos.rotateYZBy(90);
						v[i].Pos.rotateXYBy(rots[j]);
						v[i].TCoords *= tile.texture.size;
						v[i].TCoords += tile.texture.pos;
					}
					std::vector<u32> colours;
					if (selected.is_coloured) {
						meshgen_selected_lights(colours,255,10);
					}else{
						v3s16 f[3];
						for (u16 i=0; i<3; i++) {
							f[i] = faces[i];
							f[i].rotateYZBy(90);
							f[i].rotateXYBy(rots[j]);
						}
						meshgen_lights(data,n,p,colours,255,f[0],2,v);
						meshgen_lights(data,n,p,colours,255,f[1],1,&v[2]);
						meshgen_lights(data,n,p,colours,255,f[2],2,&v[3]);
						meshgen_lights(data,n,p,colours,255,f[0],2,&v[5]);
						meshgen_lights(data,n,p,colours,255,f[1],1,&v[7]);
						meshgen_lights(data,n,p,colours,255,f[2],2,&v[8]);
					}

					for (int k=0; k<10; k++) {
						v[k].Pos += pos;
					}

					data->append(tile, v, 10, indices, 24, colours);
				}
			}
			if (z_minus) {
				for (u16 j=0; j<4; j++) {
					video::S3DVertex v[10];
					for (u16 i=0; i<10; i++) {
						v[i] = branch_vertices[i];
						v[i].Pos += v3f(0,-data->m_BS*0.625,0);
						v[i].Pos.rotateYZBy(90);
						v[i].Pos.rotateXYBy(rots[j]);
						v[i].TCoords *= tile.texture.size;
						v[i].TCoords += tile.texture.pos;
					}
					std::vector<u32> colours;
					if (selected.is_coloured) {
						meshgen_selected_lights(colours,255,10);
					}else{
						v3s16 f[3];
						for (u16 i=0; i<3; i++) {
							f[i] = faces[i];
							f[i].rotateYZBy(90);
							f[i].rotateXYBy(rots[j]);
						}
						meshgen_lights(data,n,p,colours,255,f[0],2,v);
						meshgen_lights(data,n,p,colours,255,f[1],1,&v[2]);
						meshgen_lights(data,n,p,colours,255,f[2],2,&v[3]);
						meshgen_lights(data,n,p,colours,255,f[0],2,&v[5]);
						meshgen_lights(data,n,p,colours,255,f[1],1,&v[7]);
						meshgen_lights(data,n,p,colours,255,f[2],2,&v[8]);
					}

					for (int k=0; k<10; k++) {
						v[k].Pos += pos;
					}

					data->append(tile, v, 10, indices, 24, colours);
				}
			}
		}else{ // centred along Z
			for (u16 j=0; j<4; j++) {
				video::S3DVertex v[10];
				for (u16 i=0; i<10; i++) {
					v[i] = vertices[i];
					v[i].Pos.rotateYZBy(90);
					v[i].Pos.rotateXYBy(rots[j]);
					v[i].TCoords *= tile.texture.size;
					v[i].TCoords += tile.texture.pos;
				}
				std::vector<u32> colours;
				if (selected.is_coloured) {
					meshgen_selected_lights(colours,255,10);
				}else{
					v3s16 f[3];
					for (u16 i=0; i<3; i++) {
						f[i] = faces[i];
						f[i].rotateYZBy(90);
						f[i].rotateXYBy(rots[j]);
					}
					meshgen_lights(data,n,p,colours,255,f[0],2,v);
					meshgen_lights(data,n,p,colours,255,f[1],1,&v[2]);
					meshgen_lights(data,n,p,colours,255,f[2],2,&v[3]);
					meshgen_lights(data,n,p,colours,255,f[0],2,&v[5]);
					meshgen_lights(data,n,p,colours,255,f[1],1,&v[7]);
					meshgen_lights(data,n,p,colours,255,f[2],2,&v[8]);
				}

				for (int k=0; k<10; k++) {
					v[k].Pos += pos;
				}

				data->append(tile, v, 10, indices, 24, colours);
			}
			if (!z_plus_any) {
				for (u16 j=0; j<4; j++) {
					video::S3DVertex v[6];
					for (u16 i=0; i<6; i++) {
						v[i] = end_vertices[i];
						if (dir) {
							v[i].Pos.X *= top_scale;
							v[i].Pos.Z *= top_scale;
						}
						v[i].Pos.rotateYZBy(90);
						v[i].Pos.rotateXYBy(rots[j]);
						v[i].TCoords *= endtile.texture.size;
						v[i].TCoords += endtile.texture.pos;
					}
					std::vector<u32> colours;
					if (selected.is_coloured) {
						meshgen_selected_lights(colours,255,6);
					}else{
						meshgen_lights(data,n,p,colours,255,v3s16(0,0,1),6,v);
					}

					for (int k=0; k<6; k++) {
						v[k].Pos += pos;
					}

					data->append(endtile, v, 6, end_indices, 12, colours);
				}
			}
			if (!z_minus_any) {
				for (u16 j=0; j<4; j++) {
					video::S3DVertex v[6];
					for (u16 i=0; i<6; i++) {
						v[i] = end_vertices[i];
						if (dir) {
							v[i].Pos.X *= bottom_scale;
							v[i].Pos.Z *= bottom_scale;
						}
						v[i].Pos.rotateYZBy(-90);
						v[i].Pos.rotateXYBy(rots[j]);
						v[i].TCoords *= endtile.texture.size;
						v[i].TCoords += endtile.texture.pos;
					}
					std::vector<u32> colours;
					if (selected.is_coloured) {
						meshgen_selected_lights(colours,255,6);
					}else{
						meshgen_lights(data,n,p,colours,255,v3s16(0,0,-1),6,v);
					}

					for (int k=0; k<6; k++) {
						v[k].Pos += pos;
					}

					data->append(endtile, v, 6, end_indices, 12, colours);
				}
			}
			if (x_plus) {
				for (u16 j=0; j<4; j++) {
					video::S3DVertex v[10];
					for (u16 i=0; i<10; i++) {
						v[i] = branch_vertices[i];
						v[i].Pos += v3f(0,-data->m_BS*0.625,0);
						v[i].Pos.rotateXYBy(90);
						v[i].Pos.rotateYZBy(rots[j]);
						v[i].TCoords *= tile.texture.size;
						v[i].TCoords += tile.texture.pos;
					}
					std::vector<u32> colours;
					if (selected.is_coloured) {
						meshgen_selected_lights(colours,255,10);
					}else{
						v3s16 f[3];
						for (u16 i=0; i<3; i++) {
							f[i] = faces[i];
							f[i].rotateXYBy(90);
							f[i].rotateYZBy(rots[j]);
						}
						meshgen_lights(data,n,p,colours,255,f[0],2,v);
						meshgen_lights(data,n,p,colours,255,f[1],1,&v[2]);
						meshgen_lights(data,n,p,colours,255,f[2],2,&v[3]);
						meshgen_lights(data,n,p,colours,255,f[0],2,&v[5]);
						meshgen_lights(data,n,p,colours,255,f[1],1,&v[7]);
						meshgen_lights(data,n,p,colours,255,f[2],2,&v[8]);
					}

					for (int k=0; k<10; k++) {
						v[k].Pos += pos;
					}

					data->append(tile, v, 10, indices, 24, colours);
				}
			}
			if (x_minus) {
				for (u16 j=0; j<4; j++) {
					video::S3DVertex v[10];
					for (u16 i=0; i<10; i++) {
						v[i] = branch_vertices[i];
						v[i].Pos.rotateXYBy(90);
						v[i].Pos.rotateYZBy(rots[j]);
						v[i].TCoords *= tile.texture.size;
						v[i].TCoords += tile.texture.pos;
					}
					std::vector<u32> colours;
					if (selected.is_coloured) {
						meshgen_selected_lights(colours,255,10);
					}else{
						v3s16 f[3];
						for (u16 i=0; i<3; i++) {
							f[i] = faces[i];
							f[i].rotateXYBy(90);
							f[i].rotateYZBy(rots[j]);
						}
						meshgen_lights(data,n,p,colours,255,f[0],2,v);
						meshgen_lights(data,n,p,colours,255,f[1],1,&v[2]);
						meshgen_lights(data,n,p,colours,255,f[2],2,&v[3]);
						meshgen_lights(data,n,p,colours,255,f[0],2,&v[5]);
						meshgen_lights(data,n,p,colours,255,f[1],1,&v[7]);
						meshgen_lights(data,n,p,colours,255,f[2],2,&v[8]);
					}

					for (int k=0; k<10; k++) {
						v[k].Pos += pos;
					}

					data->append(tile, v, 10, indices, 24, colours);
				}
			}
		}
	}
}

void meshgen_flaglike(MeshMakeData *data, v3s16 p, MapNode &n, SelectedNode &selected)
{
	static const v3s16 tile_dirs[6] = {
		v3s16(0, 1, 0),
		v3s16(0, -1, 0),
		v3s16(1, 0, 0),
		v3s16(-1, 0, 0),
		v3s16(0, 0, 1),
		v3s16(0, 0, -1)
	};

	TileSpec tiles[6];
	TileSpec flag;
	MapNode pn(CONTENT_FENCE);
	NodeMetadata *meta = data->m_env->getMap().getNodeMetadataClone(p+data->m_blockpos_nodes);
	for (u16 i=0; i<6; i++) {
		// Handles facedir rotation for textures
		tiles[i] = getNodeTile(pn,p,tile_dirs[i],selected);
	}

	flag = getNodeTile(n,p,v3s16(0,0,0),selected,meta);

	std::vector<NodeBox> boxes = content_features(n).getNodeBoxes(n);
	meshgen_build_nodebox(data,p,n,selected,boxes,tiles);

	float tuv[8] = {
		0.25, 0.5,
		1.0 , 0.5,
		1.0 , 0.0625,
		0.25, 0.0625
	};

	for (u16 i=0; i<8; i++) {
		tuv[i] *= flag.texture.size.X;
		tuv[i] += flag.texture.pos.X;
		i++;
		tuv[i] *= flag.texture.size.Y;
		tuv[i] += flag.texture.pos.Y;
	}

	video::S3DVertex vertices[4] = {
		video::S3DVertex( 0.125*BS,       0.,0. , 0,0,0, video::SColor(255,255,255,255), tuv[0], tuv[1]),
		video::S3DVertex( 0.875*BS,       0.,0. , 0,0,0, video::SColor(255,255,255,255), tuv[2], tuv[3]),
		video::S3DVertex( 0.875*BS,0.4375*BS,0.5, 0,0,0, video::SColor(255,255,255,255), tuv[4], tuv[5]),
		video::S3DVertex( 0.125*BS,0.4375*BS,0.2, 0,0,0, video::SColor(255,255,255,255), tuv[6], tuv[7])
	};

	s16 angle = 5+n.getRotationAngle();

	if (selected.is_coloured || selected.has_crack) {
		float os[2] = {0.01,-0.01};
		for (u16 k=0; k<2; k++) {
			video::S3DVertex v[4];
			for (u16 i=0; i<4; i++) {
				v[i] = vertices[i];
				v[i].Pos.Z += os[k];
				v[i].Pos.rotateXZBy(angle);
			}

			u16 indices[] = {0,1,2,2,3,0};
			std::vector<u32> colours;
			if (selected.is_coloured) {
				meshgen_selected_lights(colours,255,4);
			}else{
				meshgen_lights(data,n,p,colours,255,v3s16(0,0,0),4,v);
			}

			v3f pos = intToFloat(p,BS);
			for (u16 i=0; i<4; i++) {
				v[i].Pos += pos;
			}

			data->append(flag, v, 4, indices, 6, colours);
		}
	}else{
		for (u16 i=0; i<4; i++) {
			vertices[i].Pos.rotateXZBy(angle);
		}

		u16 indices[] = {0,1,2,2,3,0};
		std::vector<u32> colours;
		if (selected.is_coloured) {
			meshgen_selected_lights(colours,255,4);
		}else{
			meshgen_lights(data,n,p,colours,255,v3s16(0,0,0),4,vertices);
		}

		v3f pos = intToFloat(p,BS);
		for (u16 i=0; i<4; i++) {
			vertices[i].Pos += pos;
		}

		data->append(flag, vertices, 4, indices, 6, colours);
	}
	delete meta;
}

void meshgen_melonlike(MeshMakeData *data, v3s16 p, MapNode &n, SelectedNode &selected)
{
	ContentFeatures *f = &content_features(n.getContent());
	if (f->param2_type != CPT_PLANTGROWTH || n.param2 == 0) {
		meshgen_cubelike(data,p,n,selected);
		return;
	}

	static const v3s16 tile_dirs[6] = {
		v3s16(0, 1, 0),
		v3s16(0, -1, 0),
		v3s16(1, 0, 0),
		v3s16(-1, 0, 0),
		v3s16(0, 0, 1),
		v3s16(0, 0, -1)
	};

	TileSpec tiles[6];
	for (u16 i=0; i<6; i++) {
		// Handles facedir rotation for textures
		tiles[i] = getNodeTile(n,p,tile_dirs[i],selected);
	}

	float v = (float)n.param2*0.0625;
	float hv = v/2;
	std::vector<NodeBox> boxes;
	boxes.push_back(NodeBox(-hv*data->m_BS,-0.5*data->m_BS,-hv*data->m_BS,hv*data->m_BS,(v-0.5)*data->m_BS,hv*data->m_BS));
	meshgen_build_nodebox(data,p,n,selected,boxes,tiles);
}

void meshgen_campfirelike(MeshMakeData *data, v3s16 p, MapNode &n, SelectedNode &selected)
{
	TileSpec stone_tiles[6];
	TileSpec wood_tiles[6];
	TileSpec ember_tile;
	TileSpec fire_tile;
	ContentFeatures *f;
	std::vector<NodeBox> boxes;

	f = &content_features(n.getContent());

	ember_tile = f->tiles[2];
	fire_tile = f->tiles[3];

	for (u16 i=0; i<6; i++) {
		stone_tiles[i] = f->tiles[0];
		wood_tiles[i] = f->tiles[1];
	}

	boxes.push_back(NodeBox(
		-0.5*data->m_BS,-0.5*data->m_BS,-0.125*data->m_BS,-0.3125*data->m_BS,-0.375*data->m_BS,0.125*data->m_BS
	));
	boxes.push_back(NodeBox(
		v3s16(0,45,0),v3f(0,0,0),-0.5*data->m_BS,-0.5*data->m_BS,-0.125*data->m_BS,-0.3125*data->m_BS,-0.375*data->m_BS,0.125*data->m_BS
	));
	boxes.push_back(NodeBox(
		-0.125*data->m_BS,-0.5*data->m_BS,0.3125*data->m_BS,0.125*data->m_BS,-0.375*data->m_BS,0.5*data->m_BS
	));
	boxes.push_back(NodeBox(
		v3s16(0,45,0),v3f(0,0,0),-0.125*data->m_BS,-0.5*data->m_BS,0.3125*data->m_BS,0.125*data->m_BS,-0.375*data->m_BS,0.5*data->m_BS
	));
	boxes.push_back(NodeBox(
		0.3125*data->m_BS,-0.5*data->m_BS,-0.125*data->m_BS,0.5*data->m_BS,-0.375*data->m_BS,0.125*data->m_BS
	));
	boxes.push_back(NodeBox(
		v3s16(0,45,0),v3f(0,0,0),0.3125*data->m_BS,-0.5*data->m_BS,-0.125*data->m_BS,0.5*data->m_BS,-0.375*data->m_BS,0.125*data->m_BS
	));
	boxes.push_back(NodeBox(
		-0.125*data->m_BS,-0.5*data->m_BS,-0.5*data->m_BS,0.125*data->m_BS,-0.375*data->m_BS,-0.3125*data->m_BS
	));
	boxes.push_back(NodeBox(
		v3s16(0,45,0),v3f(0,0,0),-0.125*data->m_BS,-0.5*data->m_BS,-0.5*data->m_BS,0.125*data->m_BS,-0.375*data->m_BS,-0.3125*data->m_BS
	));
	meshgen_build_nodebox(data,p,n,selected,boxes,stone_tiles);

	boxes.clear();

	boxes.push_back(NodeBox(
		v3s16(0,0,-20),v3f(0,0,0),-0.125*data->m_BS,-0.5*data->m_BS,-0.0625*data->m_BS,0.0*data->m_BS,-0.0625*data->m_BS,0.0625*data->m_BS
	));
	boxes.push_back(NodeBox(
		v3s16(0,0,20),v3f(0,0,0),0.0*data->m_BS,-0.5*data->m_BS,-0.0625*data->m_BS,0.125*data->m_BS,-0.0625*data->m_BS,0.0625*data->m_BS
	));
	boxes.push_back(NodeBox(
		v3s16(20,0,0),v3f(0,0,0),-0.0625*data->m_BS,-0.5*data->m_BS,-0.125*data->m_BS,0.0625*data->m_BS,-0.0625*data->m_BS,0.0*data->m_BS
	));
	boxes.push_back(NodeBox(
		v3s16(-20,0,0),v3f(0,0,0),-0.0625*data->m_BS,-0.5*data->m_BS,0.0*data->m_BS,0.0625*data->m_BS,-0.0625*data->m_BS,0.125*data->m_BS
	));

	meshgen_build_nodebox(data,p,n,selected,boxes,wood_tiles);

	if (selected.is_coloured || selected.has_crack)
		return;

	v3f pos = intToFloat(p,BS);

	video::S3DVertex end_vertices[4] = {
		video::S3DVertex(data->m_BS*0.0    ,data->m_BS*-0.4375 ,data->m_BS*0.0    ,0,0,0, video::SColor(255,255,255,255), 0.5, 0.5),
		video::S3DVertex(data->m_BS*0.0    ,data->m_BS*-0.4375 ,data->m_BS*0.375  ,0,0,0, video::SColor(255,255,255,255), 0.5, 0.875),
		video::S3DVertex(data->m_BS*0.375  ,data->m_BS*-0.4375 ,data->m_BS*0.0    ,0,0,0, video::SColor(255,255,255,255), 0.875, 0.5),
		video::S3DVertex(data->m_BS*0.25   ,data->m_BS*-0.4375 ,data->m_BS*0.25   ,0,0,0, video::SColor(255,255,255,255), 0.75, 0.75)
	};
	u16 end_indices[6] = {0,1,3,0,3,2};
	u16 rots[4] = {0,90,180,270};

	for (u16 j=0; j<4; j++) {
		video::S3DVertex v[4];
		for (u16 i=0; i<4; i++) {
			v[i] = end_vertices[i];
			v[i].Pos.rotateXZBy(rots[j]);
			v[i].TCoords *= ember_tile.texture.size;
			v[i].TCoords += ember_tile.texture.pos;
		}
		std::vector<u32> colours;
		meshgen_lights(data,n,p,colours,255,v3s16(0,1,0),4,v);

		for (int k=0; k<4; k++) {
			v[k].Pos += pos;
		}

		data->append(ember_tile, v, 4, end_indices, 6, colours);
	}

	NodeMetadata *meta = data->m_env->getMap().getNodeMetadataClone(p+data->m_blockpos_nodes);
	if (!meta || !((CampFireNodeMetadata*)meta)->isActive())
		return;

	for (u32 j=0; j<2; j++) {
		video::S3DVertex vertices[4] = {
			video::S3DVertex(-0.5*BS,-0.5*BS,0., 0,0,0, video::SColor(255,255,255,255), 0.,1.),
			video::S3DVertex( 0.5*BS,-0.5*BS,0., 0,0,0, video::SColor(255,255,255,255), 1.,1.),
			video::S3DVertex( 0.5*BS, 0.5*BS,0., 0,0,0, video::SColor(255,255,255,255), 1.,0.),
			video::S3DVertex(-0.5*BS, 0.5*BS,0., 0,0,0, video::SColor(255,255,255,255), 0.,0.)
		};

		s16 angle = 45;
		if (j == 1)
			angle = -45;

		for (u16 i=0; i<4; i++) {
			vertices[i].Pos.rotateXZBy(angle);
			vertices[i].TCoords *= fire_tile.texture.size;
			vertices[i].TCoords += fire_tile.texture.pos;
		}

		u16 indices[] = {0,1,2,2,3,0};
		std::vector<u32> colours;
		meshgen_lights(data,n,p,colours,255,v3s16(0,0,0),4,vertices);

		for (u16 i=0; i<4; i++) {
			vertices[i].Pos += pos;
		}

		data->append(fire_tile, vertices, 4, indices, 6, colours);
	}
}

void meshgen_bushlike(MeshMakeData *data, v3s16 p, MapNode &n, SelectedNode &selected)
{
	static const v3s16 join_dirs[6] = {
		v3s16( 1, 0, 0),
		v3s16( 0, 0, 1),
		v3s16(-1, 0, 0),
		v3s16( 0, 0,-1),
		v3s16( 0, 1, 0)
	};
	int connections[4][2] = {
		{1,3},
		{2,0},
		{3,1},
		{0,2}
	};
	bool joins[5];

	TileSpec leaf_tile;
	TileSpec berry_tile;
	ContentFeatures *f;
	ContentFeatures *nf;

	f = &content_features(n.getContent());

	leaf_tile = f->tiles[0];
	berry_tile = f->meta_tiles[0];

	v3f pos = intToFloat(p,BS);

	for (int j=0; j<4; j++) {
		v3s16 n2p = data->m_blockpos_nodes + p + join_dirs[j];
		MapNode n2 = data->m_vmanip.getNodeRO(n2p);
		nf = &content_features(n2.getContent());
		joins[j] = (nf->draw_type == CDT_BUSHLIKE);
	}

	NodeMetadata *meta = data->m_env->getMap().getNodeMetadataClone(p+data->m_blockpos_nodes);
	bool show_berries = false;
	if (meta) {
		if (((BushNodeMetadata*)meta)->berryCount() > 0)
			show_berries = true;
		delete meta;
	}

	u16 indices[] = {0,1,2,2,3,0};

	for (int j=0; j<4; j++) {
		float fz = 0.375;
		float nz = -0.375;
		float jn = 0.375;
		float x0 = 0.125;
		float x1 = 0.875;
		if (joins[connections[j][0]]) {
			fz = 0.5;
			x1 = 1.0;
		}
		if (joins[connections[j][1]]) {
			nz = -0.5;
			x0 = 0.0;
		}
		if (joins[j])
			jn = 0.499;
		video::S3DVertex vertices[4] = {
			video::S3DVertex(jn*data->m_BS,-0.5 *data->m_BS,nz*data->m_BS, 0,0,0, video::SColor(255,255,255,255), x0, 1.0),
			video::S3DVertex(jn*data->m_BS,-0.5 *data->m_BS,fz*data->m_BS, 0,0,0, video::SColor(255,255,255,255), x1, 1.0),
			video::S3DVertex(jn*data->m_BS, 0.25*data->m_BS,fz*data->m_BS, 0,0,0, video::SColor(255,255,255,255), x1, 0.25),
			video::S3DVertex(jn*data->m_BS, 0.25*data->m_BS,nz*data->m_BS, 0,0,0, video::SColor(255,255,255,255), x0, 0.25),
		};
		video::S3DVertex bvertices[4] = {
			video::S3DVertex((jn+0.001)*data->m_BS,-0.5 *data->m_BS,nz*data->m_BS, 0,0,0, video::SColor(255,255,255,255), x0, 1.0),
			video::S3DVertex((jn+0.001)*data->m_BS,-0.5 *data->m_BS,fz*data->m_BS, 0,0,0, video::SColor(255,255,255,255), x1, 1.0),
			video::S3DVertex((jn+0.001)*data->m_BS, 0.25*data->m_BS,fz*data->m_BS, 0,0,0, video::SColor(255,255,255,255), x1, 0.25),
			video::S3DVertex((jn+0.001)*data->m_BS, 0.25*data->m_BS,nz*data->m_BS, 0,0,0, video::SColor(255,255,255,255), x0, 0.25),
		};
		for (u16 i=0; i<4; i++) {
			vertices[i].Pos.rotateXZBy(90*j);
			bvertices[i].Pos.rotateXZBy(90*j);
		}
		std::vector<u32> colours;
		if (selected.is_coloured) {
			meshgen_selected_lights(colours,255,4);
		}else{
			meshgen_lights(data,n,p,colours,255,join_dirs[0],4,vertices);
		}

		for (u16 i=0; i<4; i++) {
			vertices[i].Pos += pos;
			vertices[i].TCoords *= leaf_tile.texture.size;
			vertices[i].TCoords += leaf_tile.texture.pos;
			bvertices[i].Pos += pos;
			bvertices[i].TCoords *= berry_tile.texture.size;
			bvertices[i].TCoords += berry_tile.texture.pos;
		}

		data->append(leaf_tile, vertices, 4, indices, 6, colours);
		if (show_berries)
			data->append(berry_tile, bvertices, 4, indices, 6, colours);
	}
	{
		float x_min = -0.375;
		float x_max = 0.375;
		float z_min = -0.375;
		float z_max = 0.375;
		float x0 = 0.125;
		float x1 = 0.875;
		float y0 = 0.125;
		float y1 = 0.875;
		if (joins[0]) {
			x_max = 0.5;
			x1 = 1.0;
		}
		if (joins[1]) {
			z_max = 0.5;
			y0 = 0.0;
		}
		if (joins[2]) {
			x_min = -0.5;
			x0 = 0.0;
		}
		if (joins[3]) {
			z_min = -0.5;
			y1 = 1.0;
		}
		video::S3DVertex vertices[4] = {
			video::S3DVertex(x_max*data->m_BS, 0.25*data->m_BS,z_min*data->m_BS, 0,0,0, video::SColor(255,255,255,255), x1,y1),
			video::S3DVertex(x_min*data->m_BS, 0.25*data->m_BS,z_min*data->m_BS, 0,0,0, video::SColor(255,255,255,255), x0,y1),
			video::S3DVertex(x_min*data->m_BS, 0.25*data->m_BS,z_max*data->m_BS, 0,0,0, video::SColor(255,255,255,255), x0,y0),
			video::S3DVertex(x_max*data->m_BS, 0.25*data->m_BS,z_max*data->m_BS, 0,0,0, video::SColor(255,255,255,255), x1,y0),
		};
		std::vector<u32> colours;
		if (selected.is_coloured) {
			meshgen_selected_lights(colours,255,4);
		}else{
			meshgen_lights(data,n,p,colours,255,join_dirs[0],4,vertices);
		}

		for (u16 i=0; i<4; i++) {
			vertices[i].Pos += pos;
			vertices[i].TCoords *= leaf_tile.texture.size;
			vertices[i].TCoords += leaf_tile.texture.pos;
		}

		data->append(leaf_tile, vertices, 4, indices, 6, colours);
	}
}

void meshgen_farnode(MeshMakeData *data, v3s16 p, MapNode &n)
{
	SelectedNode selected;
	v3f pos = intToFloat(p, data->m_BS);
	if (meshgen_farface(data,p,n,v3s16(-1,0,0))) {
		TileSpec tile = getNodeTile(n,p,v3s16(-1,0,0),selected,NULL);
		video::S3DVertex vertices[4] = {
			video::S3DVertex(-0.5*data->m_BS, 0.5*data->m_BS, 0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tile.texture.x0(), tile.texture.y0()),
			video::S3DVertex(-0.5*data->m_BS, 0.5*data->m_BS,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tile.texture.x1(), tile.texture.y0()),
			video::S3DVertex(-0.5*data->m_BS,-0.5*data->m_BS,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tile.texture.x1(), tile.texture.y1()),
			video::S3DVertex(-0.5*data->m_BS,-0.5*data->m_BS, 0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tile.texture.x0(), tile.texture.y1())
		};

		u16 indices[6] = {0,1,2,2,3,0};

		for (u16 i=0; i<4; i++) {
			vertices[i].Pos += pos;
		}

		data->appendFar(tile, vertices, 4, indices, 6);
	}
	if (meshgen_farface(data,p,n,v3s16(1,0,0))) {
		TileSpec tile = getNodeTile(n,p,v3s16(1,0,0),selected,NULL);
		video::S3DVertex vertices[4] = {
			video::S3DVertex(0.5*data->m_BS,-0.5*data->m_BS, 0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tile.texture.x1(), tile.texture.y1()),
			video::S3DVertex(0.5*data->m_BS,-0.5*data->m_BS,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tile.texture.x0(), tile.texture.y1()),
			video::S3DVertex(0.5*data->m_BS, 0.5*data->m_BS,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tile.texture.x0(), tile.texture.y0()),
			video::S3DVertex(0.5*data->m_BS, 0.5*data->m_BS, 0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tile.texture.x1(), tile.texture.y0())
		};

		u16 indices[6] = {0,1,2,2,3,0};

		for (u16 i=0; i<4; i++) {
			vertices[i].Pos += pos;
		}

		data->appendFar(tile, vertices, 4, indices, 6);
	}
	if (meshgen_farface(data,p,n,v3s16(0,-1,0))) {
		TileSpec tile = getNodeTile(n,p,v3s16(0,-1,0),selected,NULL);
		video::S3DVertex vertices[4] = {
			video::S3DVertex( 0.5*data->m_BS,-0.5*data->m_BS, 0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tile.texture.x0(), tile.texture.y0()),
			video::S3DVertex(-0.5*data->m_BS,-0.5*data->m_BS, 0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tile.texture.x1(), tile.texture.y0()),
			video::S3DVertex(-0.5*data->m_BS,-0.5*data->m_BS,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tile.texture.x1(), tile.texture.y1()),
			video::S3DVertex( 0.5*data->m_BS,-0.5*data->m_BS,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tile.texture.x0(), tile.texture.y1())
		};

		u16 indices[6] = {0,1,2,2,3,0};
		std::vector<u32> colours;

		for (u16 i=0; i<4; i++) {
			vertices[i].Pos += pos;
		}

		data->appendFar(tile, vertices, 4, indices, 6);
	}
	if (meshgen_farface(data,p,n,v3s16(0,1,0))) {
		TileSpec tile = getNodeTile(n,p,v3s16(0,1,0),selected,NULL);
		float h = 0.5;
		if (content_features(n.getContent()).liquid_type == LIQUID_SOURCE)
			h = 0.375;
		video::S3DVertex vertices[4] = {
			video::S3DVertex( 0.5*data->m_BS, h*data->m_BS,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tile.texture.x1(), tile.texture.y1()),
			video::S3DVertex(-0.5*data->m_BS, h*data->m_BS,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tile.texture.x0(), tile.texture.y1()),
			video::S3DVertex(-0.5*data->m_BS, h*data->m_BS, 0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tile.texture.x0(), tile.texture.y0()),
			video::S3DVertex( 0.5*data->m_BS, h*data->m_BS, 0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tile.texture.x1(), tile.texture.y0())
		};

		u16 indices[6] = {0,1,2,2,3,0};

		for (u16 i=0; i<4; i++) {
			vertices[i].Pos += pos;
		}

		data->appendFar(tile, vertices, 4, indices, 6);
	}
	if (meshgen_farface(data,p,n,v3s16(0,0,-1))) {
		TileSpec tile = getNodeTile(n,p,v3s16(0,0,-1),selected,NULL);
		video::S3DVertex vertices[4] = {
			video::S3DVertex(-0.5*data->m_BS, 0.5*data->m_BS,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tile.texture.x0(), tile.texture.y0()),
			video::S3DVertex( 0.5*data->m_BS, 0.5*data->m_BS,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tile.texture.x1(), tile.texture.y0()),
			video::S3DVertex( 0.5*data->m_BS,-0.5*data->m_BS,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tile.texture.x1(), tile.texture.y1()),
			video::S3DVertex(-0.5*data->m_BS,-0.5*data->m_BS,-0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tile.texture.x0(), tile.texture.y1())
		};

		u16 indices[6] = {0,1,2,2,3,0};

		for (u16 i=0; i<4; i++) {
			vertices[i].Pos += pos;
		}

		data->appendFar(tile, vertices, 4, indices, 6);
	}
	if (meshgen_farface(data,p,n,v3s16(0,0,1))) {
		TileSpec tile = getNodeTile(n,p,v3s16(0,0,1),selected,NULL);
		video::S3DVertex vertices[4] = {
			video::S3DVertex( 0.5*data->m_BS, 0.5*data->m_BS, 0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tile.texture.x0(), tile.texture.y0()),
			video::S3DVertex(-0.5*data->m_BS, 0.5*data->m_BS, 0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tile.texture.x1(), tile.texture.y0()),
			video::S3DVertex(-0.5*data->m_BS,-0.5*data->m_BS, 0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tile.texture.x1(), tile.texture.y1()),
			video::S3DVertex( 0.5*data->m_BS,-0.5*data->m_BS, 0.5*data->m_BS, 0,0,0, video::SColor(255,255,255,255), tile.texture.x0(), tile.texture.y1())
		};

		u16 indices[6] = {0,1,2,2,3,0};

		for (u16 i=0; i<4; i++) {
			vertices[i].Pos += pos;
		}

		data->appendFar(tile, vertices, 4, indices, 6);
	}
}

#endif
