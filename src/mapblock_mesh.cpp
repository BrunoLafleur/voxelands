/************************************************************************
* Minetest-c55
* Copyright (C) 2010-2011 celeron55, Perttu Ahola <celeron55@gmail.com>
*
* mapblock_mesh.cpp
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

#include "common.h"

#include "config.h"

#include "mapblock_mesh.h"
#include "light.h"
#include "mapblock.h"
#include "map.h"
#include "main.h" // For g_texturesource
#include "content_mapblock.h"
#include "content_nodemeta.h"
#include "profiler.h"
#include "mesh.h"
#include "base64.h"
#include "sound.h"

void MeshMakeData::fill(u32 daynight_ratio, MapBlock *block)
{
	m_daynight_ratio = daynight_ratio;
	if (!block)
		return;
	m_blockpos = block->getPos();
	if (m_env)
		m_vmanip.m_env = m_env;

	m_blockpos_nodes = m_blockpos*MAP_BLOCKSIZE;

	/*
		Copy data
	*/

	// Allocate this block + neighbors
	m_vmanip.clear();
	m_vmanip.addArea(VoxelArea(m_blockpos_nodes-v3s16(1,1,1)*MAP_BLOCKSIZE,
			m_blockpos_nodes+v3s16(1,1,1)*MAP_BLOCKSIZE*2-v3s16(1,1,1)));

	{
		//TimeTaker timer("copy central block data");
		// 0ms

		// Copy our data
		block->copyTo(m_vmanip);
	}
	{
		//TimeTaker timer("copy neighbor block data");
		// 0ms

		/*
			Copy neighbors. This is lightning fast.
			Copying only the borders would be *very* slow.
		*/

		// Get map
		Map *map = block->getParent();

		for(u16 i=0; i<6; i++)
		{
			const v3s16 &dir = g_6dirs[i];
			v3s16 bp = m_blockpos + dir;
			MapBlock *b = map->getBlockNoCreateNoEx(bp);
			if(b)
				b->copyTo(m_vmanip);
		}
	}
}

video::SColor blend_light(u32 data, u32 daylight_factor)
{
	u8 type = (data>>24)&0xFF;
	u8 a = 255;
	u8 r = 0;
	u8 g = 0;
	u8 b = 0;

	if (type < 2) {
		u8 light = data&0xFF;
		if (type == 1)
			a = (data>>8)&0xFF;
		u8 d = light&0x0F;
		u8 n = (light>>4)&0x0F;
		u8 l = ((daylight_factor * d + (1000-daylight_factor) * n) )/1000;
		u8 max = LIGHT_MAX;
		if (d == LIGHT_SUN)
			max = LIGHT_SUN;
		if (l > max)
			l = max;
		l = decode_light(l);
		r = l;
		g = l;
		b = l;
		if (l <= 80)
			b = MYMAX(0, pow((float)l/80.0, 0.8)*80.0);
	}else{
		a = type;
		r = (data>>16)&0xFF;
		g = (data>>8)&0xFF;
		b = data&0xFF;
	}

	return video::SColor(a,r,g,b);
}

std::string getGrassTile(u8 p2, std::string base, std::string overlay)
{
	std::string tex = base+"^"+overlay;
	if (p2) {
		u8 p = p2&0xF0;
		u8 g = p2&0x0F;
		if (g < 15) {
			std::string texture_name = base;
			if (p == 0) {
				float v = (float)(g+1)*0.03125;
				texture_name += "^[blit:";
				texture_name += ftos(0.5-v);
				texture_name += ",";
				texture_name += ftos(0.5-v);
				texture_name += ",";
				texture_name += ftos(0.5+v);
				texture_name += ",";
				texture_name += ftos(0.5+v);
				texture_name += ",";
				texture_name += overlay;
			}else{
				float v = (float)g*0.0625;
				if ((p&(1<<7)) != 0) { // -Z
					texture_name += "^[blit:0,";
					texture_name += ftos(1.0-v);
					texture_name += ",1,1,";
					texture_name += overlay;
				}
				if ((p&(1<<6)) != 0) { // +Z
					texture_name += "^[blit:0,0,1,";
					texture_name += ftos(v);
					texture_name += ",";
					texture_name += overlay;
				}
				if ((p&(1<<5)) != 0) { // -X
					texture_name += "^[blit:0,0,";
					texture_name += ftos(v);
					texture_name += ",1,";
					texture_name += overlay;
				}
				if ((p&(1<<4)) != 0) { // +X
					texture_name += "^[blit:";
					texture_name += ftos(1.0-v);
					texture_name += ",0,1,1,";
					texture_name += overlay;
				}
			}
			tex = texture_name;
		}
	}
	return tex;
}

TileSpec getCrackTile(TileSpec spec, SelectedNode &select)
{

	/*
		apply crack to this node
	*/
	if (select.has_crack) {
		/*
			Get texture id, translate it to name, append stuff to
			name, get texture id
		*/

		// Get original texture name
		u32 orig_id = spec.texture.id;
		std::string orig_name = g_texturesource->getTextureName(orig_id);

		// Create new texture name
		std::ostringstream os;
		os<<orig_name<<"^[crack"<<select.crack;

		// Get new texture
		u32 new_id = g_texturesource->getTextureId(os.str());

		spec.texture = g_texturesource->getTexture(new_id);
	}

	return spec;
}

/*
	Gets node tile from any place relative to block.
	Returns TILE_NODE if doesn't exist or should not be drawn.
*/
TileSpec getNodeTile(MapNode mn, v3s16 p, v3s16 face_dir, SelectedNode &select, NodeMetadata *meta)
{
	TileSpec spec;
	spec = mn.getTile(face_dir,false);
	ContentFeatures *f = &content_features(mn);

	if (meta) {
		FaceText ft = mn.getFaceText(face_dir);
		if (ft.m_hastext) {
			std::string txt("");
			switch (ft.m_type) {
			case FTT_BOOKCONTENT:
				txt = ((BookNodeMetadata*)meta)->getContent();
				break;
			case FTT_OWNER:
				txt = meta->getOwner();
				break;
			case FTT_INVOWNER:
				txt = meta->getInventoryOwner();
				break;
			default:
				txt = meta->getText();
				break;
			}
			if (txt != "") {
				// Get original texture name
				u32 orig_id = spec.texture.id;
				std::string orig_name = g_texturesource->getTextureName(orig_id);
				// Create new texture name
				std::ostringstream os;
				os<<orig_name<<"^[text:";
				os<<ft.m_pos.UpperLeftCorner.X;
				os<<",";
				os<<ft.m_pos.UpperLeftCorner.Y;
				os<<",";
				os<<ft.m_pos.LowerRightCorner.X;
				os<<",";
				os<<ft.m_pos.LowerRightCorner.Y;
				os<<",";
				os<<base64_encode((const unsigned char*)txt.c_str(),txt.size());

				// Get new texture
				u32 new_id = g_texturesource->getTextureId(os.str());

				spec.texture = g_texturesource->getTexture(new_id);
			}
		}
	}

	if (f->draw_type == CDT_PLANTLIKE && f->plantgrowth_on_trellis) {
		if (!select.is_coloured && !select.has_crack) {
			// Get original texture name
			u32 orig_id = spec.texture.id;
			std::string orig_name = g_texturesource->getTextureName(orig_id);
			std::string texture_name("trellis.png");

			if (f->param2_type != CPT_PLANTGROWTH || !mn.param2) {
				texture_name += "^"+orig_name;
			}else{
				std::string bs("^[blit:0,");
				bs += ftos(1.0-(0.0625*(float)mn.param2));
				bs += ",1,1,";
				// new name
				texture_name += bs+orig_name;
			}

			// Get new texture
			u32 new_id = g_texturesource->getTextureId(texture_name);
			spec.texture = g_texturesource->getTexture(new_id);
		}
	}else if (f->draw_type == CDT_CUBELIKE && f->param2_type == CPT_PLANTGROWTH && face_dir.Y == 1) {
		TileSpec bspec = spec;
		spec = mn.getTile(v3s16(0,-1,0),false);
		if (mn.param2) {
			u8 p = mn.param2&0xF0;
			u8 g = mn.param2&0x0F;
			if (g < 15) {
				u32 orig_id = bspec.texture.id;
				std::string blit_name = g_texturesource->getTextureName(orig_id);
				std::string orig_name = g_texturesource->getTextureName(spec.texture.id);
				std::string texture_name = orig_name;
				if (p == 0) {
					float v = (float)(g+1)*0.03125;
					texture_name += "^[blit:";
					texture_name += ftos(0.5-v);
					texture_name += ",";
					texture_name += ftos(0.5-v);
					texture_name += ",";
					texture_name += ftos(0.5+v);
					texture_name += ",";
					texture_name += ftos(0.5+v);
					texture_name += ",";
					texture_name += blit_name;
				}else{
					float v = (float)g*0.0625;
					if ((p&(1<<7)) != 0) { // -Z
						texture_name += "^[blit:0,";
						texture_name += ftos(1.0-v);
						texture_name += ",1,1,";
						texture_name += blit_name;
					}
					if ((p&(1<<6)) != 0) { // +Z
						texture_name += "^[blit:0,0,1,";
						texture_name += ftos(v);
						texture_name += ",";
						texture_name += blit_name;
					}
					if ((p&(1<<5)) != 0) { // -X
						texture_name += "^[blit:0,0,";
						texture_name += ftos(v);
						texture_name += ",1,";
						texture_name += blit_name;
					}
					if ((p&(1<<4)) != 0) { // +X
						texture_name += "^[blit:";
						texture_name += ftos(1.0-v);
						texture_name += ",0,1,1,";
						texture_name += blit_name;
					}
				}
				// Get new texture
				u32 new_id = g_texturesource->getTextureId(texture_name);
				spec.texture = g_texturesource->getTexture(new_id);
			}else{
				spec = bspec;
			}
		}
	}

	std::string rot = mn.getTileRotationString(face_dir);
	if (rot != "") {
		// Get original texture name
		u32 orig_id = spec.texture.id;
		std::string orig_name = g_texturesource->getTextureName(orig_id);
		// new name
		std::string texture_name = orig_name + rot;
		// Get new texture
		u32 new_id = g_texturesource->getTextureId(texture_name);
		spec.texture = g_texturesource->getTexture(new_id);
	}

	/*
		apply crack to this node
	*/
	if (select.has_crack) {
		/*
			Get texture id, translate it to name, append stuff to
			name, get texture id
		*/

		// Get original texture name
		u32 orig_id = spec.texture.id;
		std::string orig_name = g_texturesource->getTextureName(orig_id);

		// Create new texture name
		std::ostringstream os;
		os<<orig_name<<"^[crack"<<select.crack;

		// Get new texture
		u32 new_id = g_texturesource->getTextureId(os.str());

		spec.texture = g_texturesource->getTexture(new_id);
	}

	return spec;
}

/*
	Gets node meta tile from any place relative to block.
	Returns TILE_NODE if doesn't exist or should not be drawn.
*/
TileSpec getMetaTile(MapNode mn, v3s16 p, v3s16 face_dir, SelectedNode &select)
{
	TileSpec spec;
	spec = mn.getMetaTile(face_dir);

	return spec;
}

static const v3s16 dirs8[8] = {
	v3s16(0,0,0),
	v3s16(0,0,1),
	v3s16(0,1,0),
	v3s16(0,1,1),
	v3s16(1,0,0),
	v3s16(1,1,0),
	v3s16(1,0,1),
	v3s16(1,1,1),
};

// Calculate lighting at the given corner of p
u8 getSmoothLight(v3s16 p, v3s16 corner, VoxelManipulator &vmanip)
{
	float ambient_occlusion = 0;
	float dl = 0;
	float nl = 0;
	u16 light_count = 0;

	if (corner.X == 1)
		p.X += 1;
	if (corner.Y == 1)
		p.Y += 1;
	if (corner.Z == 1)
		p.Z += 1;

		for (u8 i = 0; i < 8; i++) {
		MapNode n = vmanip.getNodeRO(p - dirs8[i]);
		ContentFeatures &f = content_features(n);
		if (f.param_type == CPT_LIGHT) {
			dl += n.getLight(LIGHTBANK_DAY);
			nl += n.getLight(LIGHTBANK_NIGHT);
			light_count++;
			if (f.light_source > 0)
				ambient_occlusion -= 1.0;
		} else if (f.draw_type == CDT_CUBELIKE || f.draw_type == CDT_DIRTLIKE) {
			ambient_occlusion += 0.5;
		} else if (n.getContent() != CONTENT_IGNORE) {
			ambient_occlusion += 0.25;
		}
	}

	if (light_count == 0)
		return 0;

	dl /= light_count;
	nl /= light_count;

	if (ambient_occlusion > 2.0) {
		ambient_occlusion = (ambient_occlusion-2) * 0.4 + 1.0;
		dl /= ambient_occlusion;
		nl /= ambient_occlusion;
	}

	u8 idl;
	u8 inl;

	if (dl >= LIGHT_SUN) {
		idl = LIGHT_SUN;
	}else{
		idl = ceilf(dl);
	}
	inl = ceilf(nl);

	return ((inl<<4)&0xF0)|(idl&0x0F);;
}

MapBlockMesh::MapBlockMesh(MeshMakeData *data, v3s16 camera_offset):
	m_mesh(NULL),
	m_farmesh(NULL),
	m_camera_offset(camera_offset)
{
	generate(data,camera_offset,NULL);
}

MapBlockMesh::~MapBlockMesh()
{
	m_mesh->drop();
	m_mesh = NULL;
	m_farmesh->drop();
	m_farmesh = NULL;
	if (!m_animation_data.empty())
		m_animation_data.clear();
}

void MapBlockMesh::animate(float time)
{
	if (!m_mesh)
		return;

	for (std::map<u32, AnimationData>::iterator it = m_animation_data.begin();
				it != m_animation_data.end(); ++it) {

		 // Make sure we don't cause an overflow
		if (it->first >= m_mesh->getMeshBufferCount())
			return;

		const TileSpec &tile = it->second.tile;

		// Figure out current frame
		int frame = 0;
		if (time > 0.0)
			frame = (int)(time * 1000 / tile.animation_frame_length_ms) % tile.animation_frame_count;

		// If frame doesn't change, skip
		if (frame == it->second.frame)
			continue;

		m_animation_data[it->first].frame = frame;

		u16 mc = m_mesh->getMeshBufferCount();
		if (mc <= it->first)
			continue;
		scene::IMeshBuffer *buf = m_mesh->getMeshBuffer(it->first);

		// Create new texture name from original
		if (g_texturesource && frame >= 0) {
			std::ostringstream os(std::ios::binary);
			os << g_texturesource->getTextureName(tile.texture.id);
			os << "^[verticalframe:" << (int)tile.animation_frame_count << ":" << frame;
			// Set the texture
			AtlasPointer ap = g_texturesource->getTexture(os.str());
			buf->getMaterial().setTexture(0, ap.atlas);
		}
	}
}

void MapBlockMesh::generate(MeshMakeData *data, v3s16 camera_offset, JMutex *mutex)
{
	DSTACK(__FUNCTION_NAME);

	data->mesh_detail = config_get_int("client.graphics.mesh.lod");
	data->texture_detail = config_get_int("client.graphics.texture.lod");
	data->light_detail = config_get_int("client.graphics.light.lod");
	m_pos = data->m_blockpos;
	SelectedNode selected;
	if (!m_animation_data.empty())
		m_animation_data.clear();

	for(s16 z=0; z<MAP_BLOCKSIZE; z++)
	for(s16 y=0; y<MAP_BLOCKSIZE; y++)
	for(s16 x=0; x<MAP_BLOCKSIZE; x++)
	{
		v3s16 p(x,y,z);

		MapNode n = data->m_vmanip.getNodeNoEx(data->m_blockpos_nodes+p);


#if USE_AUDIO == 1
		if (data->m_sounds != NULL) {
			std::string snd = content_features(n).sound_ambient;
			std::map<v3s16,MapBlockSound>::iterator i = data->m_sounds->find(p);
			if (snd != "") {
				bool add_sound = true;
				if (i != data->m_sounds->end()) {
					if (i->second.name == snd && sound_exists(i->second.id)) {
						add_sound = false;
					}else{
						sound_stop_single(i->second.id);
					}
				}
				if (add_sound && content_features(n).liquid_type != LIQUID_NONE) {
					if (data->m_vmanip.getNodeRO(data->m_blockpos_nodes+p+v3s16(0,1,0)).getContent() != CONTENT_AIR) {
						add_sound = false;
					}else if (content_features(n).param2_type != CPT_LIQUID || n.param2 < 4 || n.param2 > 7) {
						add_sound = false;
					}else{
						int adj = 0;
						for (s16 x=-1; x<2; x++) {
							for (s16 z=-1; z<2; z++) {
								if (!x && !z)
									continue;
								content_t ac = data->m_vmanip.getNodeRO(data->m_blockpos_nodes+p+v3s16(x,0,z)).getContent();
								if (
									ac == content_features(n).liquid_alternative_flowing
									|| ac == content_features(n).liquid_alternative_source
								)
									adj++;
							}
						}
						if (adj > 3)
							add_sound = false;
					}
				}
				if (add_sound) {
					v3f pf = intToFloat(p+data->m_blockpos_nodes,BS);
					v3_t vp = {pf.X,pf.Y,pf.Z};
					MapBlockSound bsnd;
					bsnd.id = sound_play_effect((char*)snd.c_str(),1.0,1,&vp);
					bsnd.name = snd;
					if (bsnd.id > 0)
						(*data->m_sounds)[p] = bsnd;
				}
			}else if (i != data->m_sounds->end()) {
				sound_stop_single(i->second.id);
				data->m_sounds->erase(i);
			}
		}
#endif
		if (data->light_detail > 1 && !selected.is_coloured)
			meshgen_preset_smooth_lights(data,p);
		switch (content_features(n).draw_type) {
		case CDT_AIRLIKE:
			break;
		case CDT_CUBELIKE:
			meshgen_cubelike(data,p,n,selected);
			meshgen_farnode(data,p,n);
			break;
		case CDT_DIRTLIKE:
			meshgen_dirtlike(data,p,n,selected);
			meshgen_farnode(data,p,n);
			break;
		case CDT_RAILLIKE:
			meshgen_raillike(data,p,n,selected);
			break;
		case CDT_PLANTLIKE:
			meshgen_plantlike(data,p,n,selected);
			break;
		case CDT_PLANTLIKE_FERN:
			meshgen_plantlike_fern(data,p,n,selected);
			break;
		case CDT_CROPLIKE:
			meshgen_croplike(data,p,n,selected);
			break;
		case CDT_LIQUID:
			meshgen_liquid(data,p,n,selected);
			break;
		case CDT_LIQUID_SOURCE:
			meshgen_liquid_source(data,p,n,selected);
			meshgen_farnode(data,p,n);
			break;
		case CDT_NODEBOX:
			meshgen_nodebox(data,p,n,selected,false);
			break;
		case CDT_GLASSLIKE:
			meshgen_glasslike(data,p,n,selected);
			break;
		case CDT_TORCHLIKE:
			meshgen_torchlike(data,p,n,selected);
			break;
		case CDT_FENCELIKE:
			meshgen_fencelike(data,p,n,selected);
			break;
		case CDT_FIRELIKE:
			meshgen_firelike(data,p,n,selected);
			break;
		case CDT_WALLLIKE:
			meshgen_walllike(data,p,n,selected);
			meshgen_farnode(data,p,n);
			break;
		case CDT_ROOFLIKE:
			meshgen_rooflike(data,p,n,selected);
			meshgen_farnode(data,p,n);
			break;
		case CDT_LEAFLIKE:
			meshgen_leaflike(data,p,n,selected);
			meshgen_farnode(data,p,n);
			break;
		case CDT_NODEBOX_META:
			meshgen_nodebox(data,p,n,selected,true);
			break;
		case CDT_WIRELIKE:
			meshgen_wirelike(data,p,n,selected,false);
			break;
		case CDT_3DWIRELIKE:
			meshgen_wirelike(data,p,n,selected,true);
			break;
		case CDT_STAIRLIKE:
			meshgen_stairlike(data,p,n,selected);
			break;
		case CDT_SLABLIKE:
			meshgen_slablike(data,p,n,selected);
			break;
		case CDT_TRUNKLIKE:
			meshgen_trunklike(data,p,n,selected);
			meshgen_farnode(data,p,n);
			break;
		case CDT_FLAGLIKE:
			meshgen_flaglike(data,p,n,selected);
			break;
		case CDT_MELONLIKE:
			meshgen_melonlike(data,p,n,selected);
			meshgen_farnode(data,p,n);
			break;
		case CDT_CAMPFIRELIKE:
			meshgen_campfirelike(data,p,n,selected);
			break;
		case CDT_BUSHLIKE:
			meshgen_bushlike(data,p,n,selected);
			break;
		default:;
		}
	}

	scene::SMesh *mesh = new scene::SMesh();
	scene::SMesh *fmesh = new scene::SMesh();
	for (u32 i=0; i<data->m_meshdata.size(); i++) {
		MeshData &d = data->m_meshdata[i];

		// - Texture animation
		if (d.tile.material_flags & MATERIAL_FLAG_ANIMATION_VERTICAL_FRAMES) {
			// Add to MapBlockMesh in order to animate these tiles
			AnimationData anim_data;
			anim_data.tile = d.tile;
			anim_data.frame = -1;
			m_animation_data[i] = anim_data;
		}

		// Create meshbuffer
		// This is a "Standard MeshBuffer",
		// it's a typedeffed CMeshBuffer<video::S3DVertex>
		scene::SMeshBuffer *buf = new scene::SMeshBuffer();
		// Set material
		buf->Material = d.tile.getMaterial();
		// Add to mesh
		mesh->addMeshBuffer(buf);
		// Mesh grabbed it
		buf->drop();

		buf->append(d.vertices.data(), d.vertices.size(), d.indices.data(), d.indices.size());
	}
	for (u32 i=0; i<data->m_fardata.size(); i++) {
		MeshData &d = data->m_fardata[i];
		// Create meshbuffer
		// This is a "Standard MeshBuffer",
		// it's a typedeffed CMeshBuffer<video::S3DVertex>
		scene::SMeshBuffer *buf = new scene::SMeshBuffer();
		// Set material
		buf->Material = d.tile.getMaterial();
		// Add to mesh
		fmesh->addMeshBuffer(buf);
		// Mesh grabbed it
		buf->drop();

		buf->append(d.vertices.data(), d.vertices.size(), d.indices.data(), d.indices.size());
	}

	translateMesh(mesh, intToFloat(data->m_blockpos * MAP_BLOCKSIZE - camera_offset, BS));
	translateMesh(fmesh, intToFloat(data->m_blockpos * MAP_BLOCKSIZE - camera_offset, BS));

	if (mutex != NULL)
		mutex->Lock();

	if (m_mesh != NULL)
		m_mesh->drop();
	if (m_farmesh != NULL)
		m_farmesh->drop();
	m_mesh = mesh;
	m_farmesh = fmesh;
	m_meshdata.swap(data->m_meshdata);
	m_fardata.swap(data->m_fardata);
	refresh(data->m_daynight_ratio);
	m_mesh->recalculateBoundingBox();

	if (mutex != NULL)
		mutex->Unlock();

	// Get frist frame of animation AFTER the mutex is unlocked
	animate(0.0);
}

void MapBlockMesh::refresh(u32 daynight_ratio)
{
	if (m_mesh == NULL)
		return;

	u16 mc = m_mesh->getMeshBufferCount();
	for (u16 j=0; j<mc; j++) {
		scene::IMeshBuffer *buf = m_mesh->getMeshBuffer(j);
		if (buf == 0)
			continue;
		u16 vc = buf->getVertexCount();
		if (!vc)
			continue;
		video::S3DVertex *vertices = (video::S3DVertex*)buf->getVertices();
		if (vertices == 0)
			continue;
		u32 *c = m_meshdata[j].colours.data();
		for (u16 i=0; i<vc; i++) {
			vertices[i].Color = blend_light(c[i],daynight_ratio);
		}
	}
	mc = m_farmesh->getMeshBufferCount();
	for (u16 j=0; j<mc; j++) {
		scene::IMeshBuffer *buf = m_farmesh->getMeshBuffer(j);
		if (buf == 0)
			continue;
		u16 vc = buf->getVertexCount();
		if (!vc)
			continue;
		video::S3DVertex *vertices = (video::S3DVertex*)buf->getVertices();
		if (vertices == 0)
			continue;
		for (u16 i=0; i<vc; i++) {
			vertices[i].Color = blend_light(0x0F,daynight_ratio);
		}
	}
}

void MapBlockMesh::updateCameraOffset(v3s16 camera_offset)
{
	if (camera_offset != m_camera_offset) {
		translateMesh(m_mesh, intToFloat(m_camera_offset-camera_offset, BS));
		translateMesh(m_farmesh, intToFloat(m_camera_offset-camera_offset, BS));
		m_camera_offset = camera_offset;
	}
}
