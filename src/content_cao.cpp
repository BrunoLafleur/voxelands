/************************************************************************
* Minetest-c55
* Copyright (C) 2010-2011 celeron55, Perttu Ahola <celeron55@gmail.com>
*
* content_cao.cpp
* voxelands - 3d voxel world sandbox game
* Copyright (C) Lisa 'darkrose' Milne 2013-2014 <lisa@ltmnet.com>
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

#include "content_cao.h"
#include "content_mob.h"
#include "path.h"
#include "environment.h"
#include "mesh.h"
#include <ICameraSceneNode.h>
#include "sound.h"

core::map<u16, ClientActiveObject::Factory> ClientActiveObject::m_types;

/*
	MobCAO
*/

// Prototype
MobCAO proto_MobCAO;


MobCAO::MobCAO():
	ClientActiveObject(0),
	m_selection_box(-0.4*BS,-0.4*BS,-0.4*BS, 0.4*BS,0.8*BS,0.4*BS),
	m_node(NULL),
	m_position(v3f(0,10*BS,0)),
	m_camera_offset(v3s16(0,0,0)),
	m_yaw(0),
	m_last_light(0),
	m_player_hit_timer(0),
	m_damage_visual_timer(0),
	m_shooting(false),
	m_shooting_unset_timer(0),
	m_walking(false),
	m_walking_unset_timer(0),
	m_last_step(0),
	m_next_foot(0),
	m_draw_type(MDT_AUTO)
{
	ClientActiveObject::registerType(getType(), create);
}
MobCAO::~MobCAO()
{
}
ClientActiveObject* MobCAO::create()
{
	return new MobCAO();
}
void MobCAO::addToScene(scene::ISceneManager *smgr)
{
	if (m_node != NULL)
		return;

	video::IVideoDriver* driver = smgr->getVideoDriver();
	MobFeatures& m = content_mob_features(m_content);
	
	if (m.texture_display == MDT_EXTRUDED) {
		ExtrudedSpriteSceneNode *node = new ExtrudedSpriteSceneNode(smgr->getRootSceneNode(),smgr,-1,v3f(0,0,0),v3f(0,0,0),v3f(5,5,5));
		node->setVisible(true);
		node->setSprite(g_texturesource->getTextureRaw(m.texture));
		m_node = (scene::IMeshSceneNode*)node;
		m_draw_type = MDT_EXTRUDED;
		updateNodePos();
	}else if (m.model != "") {
		scene::IAnimatedMesh* const mesh = createModelMesh(smgr,m.model.c_str(),true);
		if (!mesh)
			return;

		scene::IAnimatedMeshSceneNode* const node = smgr->addAnimatedMeshSceneNode(mesh);

		if (node) {
			int s;
			int e;
			char buff[1024];
			m.getAnimationFrames(MA_STAND,&s,&e);
			node->setFrameLoop(s,e);
			node->setScale(m.model_scale);
			setMeshColor(node->getMesh(), video::SColor(255,255,255,255));
			bool use_trilinear_filter = config_get_bool("client.video.trilinear");
			bool use_bilinear_filter = config_get_bool("client.video.bilinear");
			bool use_anisotropic_filter = config_get_bool("client.video.anisotropic");

			// Set material flags and texture
			if (path_get((char*)"texture",const_cast<char*>(m.texture.c_str()),1,buff,1024))
				node->setMaterialTexture( 0, driver->getTexture(buff));
			video::SMaterial& material = node->getMaterial(0);
			material.setFlag(video::EMF_LIGHTING, false);
			material.setFlag(video::EMF_TRILINEAR_FILTER, use_trilinear_filter);
			material.setFlag(video::EMF_BILINEAR_FILTER, use_bilinear_filter);
			material.setFlag(video::EMF_ANISOTROPIC_FILTER, use_anisotropic_filter);
			material.MaterialType = video::EMT_TRANSPARENT_ALPHA_CHANNEL_REF;

			node->setVisible(true);
			m_draw_type = MDT_MODEL;
		}
		m_node = (scene::IMeshSceneNode*)node;
#if (IRRLICHT_VERSION_MAJOR >= 1 && IRRLICHT_VERSION_MINOR >= 8) || IRRLICHT_VERSION_MAJOR >= 2
		mesh->drop();
#endif
		updateNodePos();
	}else if (m.texture != "") {
		char buff[1024];
		bool use_trilinear_filter = config_get_bool("client.video.trilinear");
		bool use_bilinear_filter = config_get_bool("client.video.bilinear");
		bool use_anisotropic_filter = config_get_bool("client.video.anisotropic");
		scene::IBillboardSceneNode *bill = smgr->addBillboardSceneNode(NULL, v2f(1, 1), v3f(0,0,0), -1);
		if (path_get((char*)"texture",const_cast<char*>(m.texture.c_str()),1,buff,1024))
			bill->setMaterialTexture( 0, driver->getTexture(buff));
		bill->setMaterialFlag(video::EMF_LIGHTING, false);
		bill->setMaterialFlag(video::EMF_TRILINEAR_FILTER, use_trilinear_filter);
		bill->setMaterialFlag(video::EMF_BILINEAR_FILTER, use_bilinear_filter);
		bill->setMaterialFlag(video::EMF_ANISOTROPIC_FILTER, use_anisotropic_filter);
		bill->setMaterialType(video::EMT_TRANSPARENT_ALPHA_CHANNEL_REF);
		bill->setMaterialFlag(video::EMF_FOG_ENABLE, true);
		bill->setColor(video::SColor(255,0,0,0));
		bill->setVisible(true);
#if (IRRLICHT_VERSION_MAJOR >= 1 && IRRLICHT_VERSION_MINOR >= 8) || IRRLICHT_VERSION_MAJOR >= 2
		bill->setSize(m.model_scale.X*BS,m.model_scale.Y*BS,m.model_scale.Z*BS);
#else
		v2f bb_size(m.model_scale.X*BS,m.model_scale.Y*BS);
		bill->setSize(bb_size);
#endif
		m_node = (scene::IMeshSceneNode*)bill;
		m_draw_type = MDT_SPRITE;
		updateNodePos();
	}
}
void MobCAO::removeFromScene()
{
	if (m_node != NULL) {
		m_node->remove();
		m_node = NULL;
	}
}
void MobCAO::updateLight(u8 light_at_pos)
{
	const MobFeatures& m = content_mob_features(m_content);
	
	if (m.glow_light)
		light_at_pos = m.glow_light;
	if (m_shooting && m.attack_glow_light)
		light_at_pos = m.attack_glow_light;

	m_last_light = light_at_pos;

	u8 li = decode_light(light_at_pos);
	video::SColor color(255,li,li,li);

	if (m_node != NULL) {
		if (m_draw_type == MDT_MODEL) {
			setMeshVerticesColor(((scene::IAnimatedMeshSceneNode*)m_node)->getMesh(), color);
		}else if (m_draw_type == MDT_SPRITE) {
			((scene::IBillboardSceneNode*)m_node)->setColor(color);
		}else if (m_draw_type == MDT_EXTRUDED) {
			((ExtrudedSpriteSceneNode*)m_node)->updateLight(li);
		}
	}
}
v3s16 MobCAO::getLightPosition()
{
	return floatToInt(m_position, BS);
}
void MobCAO::updateNodePos()
{
	if (m_node == NULL)
		return;
	v3f offset = content_mob_features(m_content).model_offset+v3f(0,-0.5,0);
	offset.X *= (float)BS;
	offset.Y *= (float)BS;
	offset.Z *= (float)BS;
	offset += pos_translator.vect_show;
	m_node->setPosition(offset-intToFloat(m_camera_offset, BS));

	v3f rot = m_node->getRotation();
	if (m_draw_type == MDT_MODEL) {
		rot.Y = (90-pos_translator.yaw_show)+content_mob_features(m_content).model_rotation.Y;
	}else if (m_draw_type == MDT_SPRITE) {
		rot.Y = pos_translator.yaw_show+content_mob_features(m_content).model_rotation.Y;
	}else if (m_draw_type == MDT_EXTRUDED) {
		rot.Y = (180-pos_translator.yaw_show)+content_mob_features(m_content).model_rotation.Y;
	}
	m_node->setRotation(rot);
}
void MobCAO::step(float dtime, ClientEnvironment *env)
{
	if (!m_node)
		return;

	const MobFeatures& m = content_mob_features(m_content);

	pos_translator.translate(dtime);

	updateNodePos();

	/* Damage local player */
	if (m.attack_player_damage && m_player_hit_timer <= 0.0) {
		LocalPlayer *player = env->getLocalPlayer();
		assert(player);

		v3f playerpos = player->getPosition();

		if (
			fabs(m_position.X - playerpos.X) < m.attack_player_range.X*BS
			&& (
				fabs(m_position.Y - playerpos.Y) < m.attack_player_range.Y*BS
				|| (
					m_position.Y > playerpos.Y
					&& fabs(m_position.Y-(playerpos.Y+BS)) < m.attack_player_range.Y*BS
				)
			) && fabs(m_position.Z - playerpos.Z) < m.attack_player_range.Z*BS
		) {
			env->damageLocalPlayer(PLAYER_TORSO,DAMAGE_ATTACK,m.attack_player_damage);
			m_player_hit_timer = 3.0;
		}
	}

	/* Run timers */
	m_player_hit_timer -= dtime;

	if (m_damage_visual_timer >= 0)
		m_damage_visual_timer -= dtime;

	m_walking_unset_timer += dtime;
	if (m_walking_unset_timer >= 1.0) {
		m_walking = false;
		setAnimation(MA_STAND);
	}

	m_shooting_unset_timer -= dtime;
	if (m_shooting_unset_timer <= 0.0 && m_shooting) {
		if (m.attack_glow_light) {
			u8 li = decode_light(m_last_light);
			video::SColor color(255,li,li,li);
			if (m_draw_type == MDT_MODEL) {
				setMeshVerticesColor(((scene::IAnimatedMeshSceneNode*)m_node)->getMesh(), color);
			}else if (m_draw_type == MDT_SPRITE) {
				((scene::IBillboardSceneNode*)m_node)->setColor(color);
			}
		}
		m_shooting = false;
		if (m_walking) {
			setAnimation(MA_MOVE);
		}else{
			setAnimation(MA_STAND);
		}
	}

	if (!m.moves_silently && m_walking && m_draw_type == MDT_MODEL) {
		m_last_step += dtime;
		/* roughly sort of when a step sound should probably be heard, maybe */
		if (m_last_step > 0.5) {
			m_last_step -= 0.5;
#if USE_AUDIO == 1
			sound_play_step(&env->getMap(),m_position,m_next_foot, 0.3);
#endif
			m_next_foot = !m_next_foot;
		}
	}
}
void MobCAO::processMessage(const std::string &data)
{
	//infostream<<"MobCAO: Got message"<<std::endl;
	std::istringstream is(data, std::ios::binary);
	// command
	u8 cmd = readU8(is);

	// Move
	if (cmd == 0) {
		// pos
		m_position = readV3F1000(is);
		// yaw
		m_yaw = readF1000(is);
		pos_translator.update(m_position,m_yaw);

		if (!m_walking) {
			m_walking = true;
			m_walking_unset_timer = 0;
			setAnimation(MA_MOVE);
		}

		updateNodePos();
	}
	// Trigger shooting
	else if (cmd == 2) {
		// length
		m_shooting_unset_timer = readF1000(is);

		if (content_mob_features(m_content).attack_glow_light){
			u8 li = decode_light(content_mob_features(m_content).attack_glow_light);
			video::SColor color(255,li,li,li);
			if (m_node != NULL) {
				if (m_draw_type == MDT_MODEL) {
					setMeshVerticesColor(((scene::IAnimatedMeshSceneNode*)m_node)->getMesh(), color);
				}else if (m_draw_type == MDT_SPRITE) {
					((scene::IBillboardSceneNode*)m_node)->setColor(color);
				}else if (m_draw_type == MDT_EXTRUDED) {
					((ExtrudedSpriteSceneNode*)m_node)->updateLight(li);
				}
			}
		}

		if (!m_shooting) {
			m_shooting = true;
			setAnimation(MA_ATTACK);
		}
	}
}
void MobCAO::initialize(const std::string &data)
{
	{
		std::istringstream is(data, std::ios::binary);
		// version
		u8 version = readU8(is);
		// check version
		if (version < 0 || version > 1)
			return;
		// pos
		m_position = readV3F1000(is);
		// content
		m_content = readU16(is);
		m_draw_type = content_mob_features(m_content).texture_display;
		// yaw
		m_yaw = readF1000(is);
		pos_translator.init(m_position,m_yaw);
		if (version == 0) {
			// client doesn't use these, but has to read past them
			// speed
			readV3F1000(is);
			// age
			readF1000(is);
			// hp
			readU8(is);
		}
		// shooting
		m_shooting = !!readU8(is);
	}

	updateNodePos();
}
bool MobCAO::directReportPunch(content_t punch_item, v3f dir)
{
	const MobFeatures& m = content_mob_features(m_content);
	if (m.punch_action == MPA_IGNORE)
		return false;

	ToolItemFeatures f = content_toolitem_features(punch_item);
	if (m.special_dropped_item != CONTENT_IGNORE && (m.special_punch_item == TT_NONE || f.type == m.special_punch_item))
		return false;

	video::SColor color(255,255,0,0);

	if (m_node != NULL) {
		if (m_draw_type == MDT_MODEL) {
			setMeshVerticesColor(((scene::IAnimatedMeshSceneNode*)m_node)->getMesh(), color);
		}else if (m_draw_type == MDT_SPRITE) {
			((scene::IBillboardSceneNode*)m_node)->setColor(color);
		}
	}

	m_damage_visual_timer = 0.05;

	m_position += dir * BS;
	pos_translator.sharpen();
	pos_translator.update(m_position,m_yaw);
	updateNodePos();

	return false;
}
void MobCAO::setAnimation(MobAnimation anim)
{
	if (m_draw_type != MDT_MODEL)
		return;
	int s;
	int e;
	content_mob_features(m_content).getAnimationFrames(anim,&s,&e);
	((scene::IAnimatedMeshSceneNode*)m_node)->setFrameLoop(s,e);
}
