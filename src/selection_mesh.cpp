/************************************************************************
* selection_mesh.cpp
* voxelands - 3d voxel world sandbox game
* Copyright (C) Lisa 'darkrose' Milne 2013-2015 <lisa@ltmnet.com>
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
************************************************************************/

#include "common.h"

#include "common_irrlicht.h"
#include "selection_mesh.h"
#include "content_mapblock.h"
#include "main.h" // For g_settings and g_texturesource
#include "mesh.h"
#include "mapblock.h"
#include "client.h"

static std::map<v3s16,SelectedNode> selected;
static std::vector<SelectionMesh*> meshes;
static v3s16 o_camera_offset(0,0,0);

static void selection_clear()
{
	selected.clear();

	/* delete meshes */
	std::vector<SelectionMesh*> omeshes;
	omeshes.swap(meshes);
	for (std::vector<SelectionMesh*>::iterator i = omeshes.begin(); i != omeshes.end(); i++) {
		SelectionMesh *mesh = *i;
		if (mesh)
			delete mesh;
	}
}

static void selection_generate(Client &client)
{
	std::map<v3s16, std::map<v3s16,SelectedNode> > selected_blocks;

	Map &map = client.getEnv().getMap();
	u32 dn_ratio = client.getEnv().getDayNightRatio();

	/* split into mapblocks */
	for (std::map<v3s16,SelectedNode>::iterator i = selected.begin(); i != selected.end(); i++) {
		v3s16 bp = getNodeBlockPos(i->first);
		selected_blocks[bp][i->first] = i->second;
	}

	/* iterate over mapblocks */
	for (std::map<v3s16, std::map<v3s16,SelectedNode> >::iterator i = selected_blocks.begin(); i != selected_blocks.end(); i++) {
		/* create selection meshes */
		v3s16 bp = i->first;
		std::map<v3s16,SelectedNode> bselected = i->second;
		MeshMakeData data;
		MapBlock* const block = map.getBlockNoCreateNoEx(bp);
		data.m_selected = bselected;
		data.m_env = &client.getEnv();
		data.fill(dn_ratio,block);
		SelectionMesh *mesh = new SelectionMesh(&data);
		meshes.push_back(mesh);
		if(block)
		    block->ResetCurrent();
	}
}

void selection_draw(video::IVideoDriver* driver, Client &client, v3s16 camera_offset, std::vector<SelectedNode> &select)
{
	if (!select.size()) {
		if (selected.size())
			selection_clear();
		return;
	}

	bool match = true;
	std::map<v3s16,SelectedNode> nselected;

	for (std::vector<SelectedNode>::iterator i = select.begin(); i != select.end(); i++) {
		std::map<v3s16,SelectedNode>::iterator n = selected.find(i->pos);
		if (
			n == selected.end()
			|| n->second.crack != i->crack
			|| n->second.has_crack != i->has_crack
			|| n->second.is_coloured != i->is_coloured
			|| n->second.content != i->content
		)
			match = false;
		nselected[i->pos] = *i;
	}

	if (!match || nselected.size() != selected.size()) {
		selection_clear();
		selected.swap(nselected);
		o_camera_offset = camera_offset;
		selection_generate(client);
	}

	if (!meshes.size())
		return;

	v3f cos_diff(0,0,0);
	bool cos_changed = false;

	if (camera_offset != o_camera_offset) {
		cos_diff = intToFloat(o_camera_offset-camera_offset, BS);
		o_camera_offset = camera_offset;
		cos_changed = true;
	}

	bool render_trilinear = config_get_bool("client.video.trilinear");
	bool render_bilinear = config_get_bool("client.video.bilinear");
	bool render_anisotropic = config_get_bool("client.video.anisotropic");
	bool anim_textures = config_get_bool("client.graphics.texture.animations");

	for (std::vector<SelectionMesh*>::iterator i = meshes.begin(); i != meshes.end(); i++) {
			SelectionMesh* const mesh = *i;
		if (!mesh || !mesh->getMesh())
			continue;
		scene::SMesh* const m = mesh->getMesh();
		if (!m)
			continue;
		if (cos_changed)
			translateMesh(m, cos_diff);

		if (anim_textures && mesh->isAnimated())
			mesh->animate(client.getAnimationTime());

		u32 c = m->getMeshBufferCount();

		for (u32 i=0; i<c; i++) {
			scene::IMeshBuffer *buf = m->getMeshBuffer(i);
			if (buf == NULL)
				continue;
			if (buf->getVertexCount() == 0)
				continue;

			buf->getMaterial().setFlag(video::EMF_TRILINEAR_FILTER, render_trilinear);
			buf->getMaterial().setFlag(video::EMF_BILINEAR_FILTER, render_bilinear);
			buf->getMaterial().setFlag(video::EMF_ANISOTROPIC_FILTER, render_anisotropic);

			driver->setMaterial(buf->getMaterial());
			driver->drawMeshBuffer(buf);
		}
	}
}

SelectionMesh::SelectionMesh(MeshMakeData *data):
	m_mesh(NULL)
{
	generate(data);
}

void SelectionMesh::animate(float time)
{
	for (std::map<u32, AnimationData>::iterator it = m_animation_data.begin();
			it != m_animation_data.end(); ++it) {

		const TileSpec &tile = it->second.tile;

		// Figure out current frame
		int frame = (int)(time * 1000 / tile.animation_frame_length_ms) % tile.animation_frame_count;

		// If frame doesn't change, skip
		if (frame == it->second.frame)// || temp_data.frame < 0)
			continue;

		m_animation_data[it->first].frame = frame;

		// Make sure we don't cause an overflow. Can get removed if future is no problems occuring
		if (it->first >= m_mesh->getMeshBufferCount())
			return;

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

SelectionMesh::~SelectionMesh()
{
	m_mesh->drop();
	m_mesh = NULL;

	if (!m_animation_data.empty())
			m_animation_data.clear();
}

void SelectionMesh::generate(MeshMakeData *data)
{
	DSTACK(__FUNCTION_NAME);

	data->m_BSd = 0.02;
	data->m_BS = (float)BS+data->m_BSd;
	data->mesh_detail = config_get_int("client.graphics.mesh.lod");
	data->texture_detail = config_get_int("client.graphics.texture.lod");
	data->light_detail = config_get_int("client.graphics.light.lod");
	m_pos = data->m_blockpos;

	for (std::map<v3s16,SelectedNode>::iterator i = data->m_selected.begin(); i != data->m_selected.end(); i++) {
		SelectedNode selected = i->second;
		v3s16 p = selected.pos-data->m_blockpos_nodes;

		MapNode n = data->m_vmanip.getNodeNoEx(selected.pos);

		if (data->light_detail > 1 && !selected.is_coloured)
			meshgen_preset_smooth_lights(data,p);
		switch (content_features(n).draw_type) {
		case CDT_AIRLIKE:
			break;
		case CDT_CUBELIKE:
			meshgen_cubelike(data,p,n,selected);
			break;
		case CDT_DIRTLIKE:
			meshgen_dirtlike(data,p,n,selected);
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
			break;
		case CDT_NODEBOX:
		case CDT_NODEBOX_META:
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
			break;
		case CDT_ROOFLIKE:
			meshgen_rooflike(data,p,n,selected);
			break;
		case CDT_LEAFLIKE:
			meshgen_leaflike(data,p,n,selected);
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
			break;
		case CDT_FLAGLIKE:
			meshgen_flaglike(data,p,n,selected);
			break;
		case CDT_MELONLIKE:
			meshgen_melonlike(data,p,n,selected);
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

	scene::SMesh* const mesh = new scene::SMesh();
	
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
		scene::SMeshBuffer* const buf = new scene::SMeshBuffer();
		
		// Set material
		buf->Material = d.tile.getMaterial();
		// Add to mesh
		mesh->addMeshBuffer(buf);
		// Mesh grabbed it
		buf->drop();
		buf->append(d.vertices.data(), d.vertices.size(), d.indices.data(), d.indices.size());
	}

	translateMesh(mesh, intToFloat(data->m_blockpos_nodes - o_camera_offset, BS));

	if (m_mesh)
		m_mesh->drop();
	m_mesh = mesh;
	m_meshdata.swap(data->m_meshdata);
	refresh(data->m_daynight_ratio);
	m_mesh->recalculateBoundingBox();
	animate(0.0); // init first frame
}

void SelectionMesh::refresh(u32 daynight_ratio)
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
}
