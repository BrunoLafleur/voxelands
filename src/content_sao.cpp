/************************************************************************
* Minetest-c55
* Copyright (C) 2010-2011 celeron55, Perttu Ahola <celeron55@gmail.com>
*
* content_sao.cpp
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
#include "content_sao.h"
#include "content_mob.h"
#include "collision.h"
#include "environment.h"
#include "profiler.h"
#include "nodemetadata.h"
#include "mapblock.h"

#include "array.h"

core::map<u16, ServerActiveObject::Factory> ServerActiveObject::m_types;

/* Some helper functions */

// Y is copied, X and Z change is limited
void accelerate_xz(v3f &speed, v3f target_speed, f32 max_increase)
{
	v3f d_wanted = target_speed - speed;
	d_wanted.Y = 0;
	f32 dl_wanted = d_wanted.getLength();
	f32 dl = dl_wanted;
	if(dl > max_increase)
		dl = max_increase;

	v3f d = d_wanted.normalize() * dl;

	speed.X += d.X;
	speed.Z += d.Z;
	speed.Y = target_speed.Y;
}

static void get_random_u32_array(u32 a[], u32 len)
{
	u32 i, n;
	for(i=0; i<len; i++)
		a[i] = i;
	n = len;
	while(n > 1){
		u32 k = myrand() % n;
		n--;
		u32 temp = a[n];
		a[n] = a[k];
		a[k] = temp;
	}
}

#define PP(x) "("<<(x).X<<","<<(x).Y<<","<<(x).Z<<")"



/*
	MobSAO
*/

// Prototype
MobSAO proto_MobSAO(NULL, 0, v3f(0,0,0), CONTENT_IGNORE);

MobSAO::MobSAO(ServerEnvironment *env, u16 id, v3f pos, content_t type):
	ServerActiveObject(env, id, pos),
	m_content(type),
	m_speed(0,0,0),
	m_last_sent_position(0,0,0),
	m_oldpos(0,0,0),
	m_initial_pos(pos),
	m_yaw(0),
	m_falling(false),
	m_next_pos_exists(false),
	m_hp(10),
	m_angry(false),
	m_special_count(0),
	m_tamed_chance(0),
	m_disturb_timer(100000),
	m_disturbing_player(),
	m_random_disturb_timer(0),
	m_walk_around(false),
	m_walk_around_timer(0),
	m_shoot_reload_timer(0),
	m_shooting(false),
	m_shooting_timer(0),
	m_shoot_y(0),
	m_last_sound(0)
{
	ServerActiveObject::registerType(getType(), create);
	if ((type&CONTENT_MOB_MASK) == CONTENT_MOB_MASK) {
		m_hp = content_mob_features(type).hp;
		m_special_count = content_mob_features(type).special_dropped_max;
		m_tamed_chance = content_mob_features(type).level*5;
	}
}
MobSAO::MobSAO(ServerEnvironment *env, u16 id, v3f pos, v3f speed, content_t type):
	ServerActiveObject(env, id, pos),
	m_content(type),
	m_speed(speed),
	m_last_sent_position(0,0,0),
	m_oldpos(0,0,0),
	m_initial_pos(pos),
	m_yaw(0),
	m_falling(false),
	m_next_pos_exists(false),
	m_hp(10),
	m_angry(false),
	m_special_count(0),
	m_tamed_chance(0),
	m_disturb_timer(100000),
	m_disturbing_player(),
	m_random_disturb_timer(0),
	m_walk_around(false),
	m_walk_around_timer(0),
	m_shoot_reload_timer(0),
	m_shooting(false),
	m_shooting_timer(0),
	m_shoot_y(0),
	m_last_sound(0)
{
	ServerActiveObject::registerType(getType(), create);
	if ((type&CONTENT_MOB_MASK) == CONTENT_MOB_MASK) {
		m_hp = content_mob_features(type).hp;
		m_special_count = content_mob_features(type).special_dropped_max;
		m_tamed_chance = content_mob_features(type).level*5;
	}
}
MobSAO::~MobSAO()
{
}
ServerActiveObject* MobSAO::create(ServerEnvironment *env, u16 id, v3f pos, const std::string &data)
{
	std::istringstream is(data, std::ios::binary);
	char buf[1];
	// read version
	is.read(buf, 1);
	u8 version = buf[0];
	// check if version is supported
	if (version > 1)
		return NULL;
	v3f p = readV3F1000(is);
	content_t c = readU16(is);
	c = content_mob_features(c).content;
	if (c == CONTENT_IGNORE)
		return NULL;
	MobSAO *o = new MobSAO(env,id,pos,c);
	o->m_base_position = p;
	o->m_yaw = readF1000(is);
	o->m_speed = readV3F1000(is);
	// this was age
	if (version == 0)
		readF1000(is);
	o->m_hp = readU8(is);
	return o;
}
std::string MobSAO::getStaticData()
{
	std::ostringstream os(std::ios::binary);
	// version
	writeU8(os, 1);
	// pos
	writeV3F1000(os, m_base_position);
	// content
	writeU16(os,m_content);
	// yaw
	writeF1000(os,m_yaw);
	// speed
	writeV3F1000(os, m_speed);
	// hp
	writeU8(os,m_hp);
	// shooting
	writeU8(os,(u8)m_shooting);
	return os.str();
}
std::string MobSAO::getClientInitializationData()
{
	std::ostringstream os(std::ios::binary);
	// version
	writeU8(os, 1);
	// pos
	writeV3F1000(os, m_base_position);
	// content
	writeU16(os,m_content);
	// yaw
	writeF1000(os,m_yaw);
	// shooting
	writeU8(os,(u8)m_shooting);
	return os.str();
}
void MobSAO::step(float dtime, bool send_recommended)
{
	const MobFeatures& m = content_mob_features(m_content);
	Player *disturbing_player = NULL;
	v3f disturbing_player_off = v3f(0,1,0);
	v3f disturbing_player_norm = v3f(0,1,0);
	float disturbing_player_distance = 1000000;
	float disturbing_player_dir = 0;
	bool dont_move = false;
	bool notices_player = false;
	bool following = false;

	/* don't do anything if there's no nearby player */
	if (m_disturbing_player == "") {
		float distance = 40*BS;
		array_t *players = m_env->getPlayers(true);
		Player *player;
		uint32_t i;
		for (i=0; i<players->length; i++) {
			player = (Player*)array_get_ptr(players,i);
			if (!player)
				continue;
			v3f playerpos = player->getPosition();
			f32 dist = m_base_position.getDistanceFrom(playerpos);
			if (dist < distance)
				distance = dist;
		}
		array_free(players,1);
		if (distance > 32*BS) {
			/* kill of anything that shouldn't be on its own */
			if (
				m.level == MOB_AGGRESSIVE
				|| (m.motion == MM_THROWN || m.motion == MM_CONSTANT || m.motion == MM_STATIC)
			)
				m_removed = true;
			return;
		}
		if (distance > 16*BS && myrand_range(0,10) != 0)
			dont_move = true;
	}

	if (m.follow_item != CONTENT_IGNORE || m.motion == MM_SEEKER || m.angry_motion == MM_SEEKER || m.angry_motion == MM_FLEE)
		notices_player = true;

	/* if it isn't a swimmer, kill it in liquid */
	if (m.motion_type != MMT_SWIM) {
		v3s16 p = floatToInt(m_base_position,BS);
		MapNode n = m_env->getMap().getNodeNoEx(p);
		if (content_features(n).liquid_type != LIQUID_NONE) {
			m_hp--;
			if (m_hp < 1) {
				m_removed = true;
				return;
			}
		}
	}else{
		v3s16 p = floatToInt(m_base_position,BS);
		MapNode n = m_env->getMap().getNodeNoEx(p);
		if (content_features(n).liquid_type == LIQUID_NONE) {
			m_hp--;
			if (m_hp < 1) {
				m_removed = true;
				return;
			}
		}
	}

	m_last_sound += dtime;

	if (m_last_sound > 30.0) {
		m_last_sound -= 5.0;
		if (m.sound_random != "" && myrand_range(0,10) == 0) {
			if (m.sound_random_extra != "" && myrand_range(0,100) == 0) {
				m_env->addEnvEvent(ENV_EVENT_SOUND,m_base_position,m.sound_random_extra);
			}else{
				m_env->addEnvEvent(ENV_EVENT_SOUND,m_base_position,m.sound_random);
			}
			m_last_sound -= 30.0;
		}
	}

	if (m.special_dropped_max > 0 && m_special_count < m.special_dropped_max && myrand_range(0,5000) == 0)
		m_special_count++;

	m_random_disturb_timer += dtime;
	if (notices_player) {
		if (m_random_disturb_timer >= 5.0) {
			m_random_disturb_timer = 0;
			if (
				m_disturbing_player == ""
				|| m_base_position.getDistanceFrom(m_env->getPlayer(m_disturbing_player.c_str())->getPosition()) > BS*16
			) {
				m_disturbing_player = "";
				// Check connected players
				array_t *players = m_env->getPlayers(true);
				Player *player;
				uint32_t i;
				for (i=0; i<players->length; i++) {
					player = (Player*)array_get_ptr(players,i);
					if (!player)
						continue;
					v3f playerpos = player->getPosition();
					f32 dist = m_base_position.getDistanceFrom(playerpos);
					if (dist < BS*16) {
						if (dist < BS*8 || myrand_range(0,2) == 0) {
							vlprintf(
								CN_ACTION,
								(char*)"mob (%u) at (%f,%f,%f) was randomly disturbed by %s",
								m_id,
								m_base_position.X/BS,
								m_base_position.Y/BS,
								m_base_position.Z/BS,
								player->getName()
							);
							m_disturbing_player = player->getName();
							m_disturb_timer = 0;
							break;
						}
					}
				}
				array_free(players,1);
			}
		}
		if (m_disturbing_player != "") {
			disturbing_player = m_env->getPlayer(m_disturbing_player.c_str());
			if (disturbing_player) {
				disturbing_player_off = disturbing_player->getPosition() - m_base_position;
				disturbing_player_distance = disturbing_player_off.getLength();
				disturbing_player_norm = disturbing_player_off;
				disturbing_player_norm.normalize();
				disturbing_player_dir = 180./PI*atan2(disturbing_player_norm.Z,disturbing_player_norm.X);
				if (m.follow_item != CONTENT_IGNORE && !dont_move) {
					u16 item_i = disturbing_player->getSelectedItem();
					InventoryList *ilist = disturbing_player->inventory.getList("main");
					if (ilist != NULL) {
						InventoryItem *item = ilist->getItem(item_i);
						if (item != NULL && item->getContent() == m.follow_item)
							following = true;
					}
				}
			}
		}else if (m_angry) {
			m_angry = false;
		}else if (m.follow_item != CONTENT_IGNORE && !dont_move && myrand_range(0,100) == 0) {
			core::array<DistanceSortedActiveObject> objects;
			f32 range = 32*BS;

			m_env->getActiveObjects(m_base_position, range, objects);

			// Sort them.
			// After this, the closest object is the first in the array.
			objects.sort();
			if (objects.size() < 3) {
				for (u32 i=0; i<objects.size(); i++) {
					ServerActiveObject *obj = (ServerActiveObject*)objects[i].obj;
					if (obj->getId() == m_id)
						continue;
					if (obj->getType() == ACTIVEOBJECT_TYPE_MOB) {
						MobSAO *mob = (MobSAO*)obj;
						if (mob->getContent() == m_content) {
							v3s16 p = floatToInt(m_base_position,BS);
							mob_spawn(p,m_content,m_env);
						}
					}
				}
			}
		}
		m_disturb_timer += dtime;

		if (!m_falling && m.attack_throw_object != CONTENT_IGNORE) {
			m_shooting_timer -= dtime;
			if (m_shooting_timer <= 0.0 && m_shooting) {
				m_shooting = false;
				v3f shoot_pos = m.attack_throw_offset * BS;
				v3f dir(cos(m_yaw/180*PI),0,sin(m_yaw/180*PI));
				dir.Y = m_shoot_y;
				dir.normalize();
				v3f speed = dir * BS * 10.0;
				v3f pos = m_base_position + shoot_pos;
				ServerActiveObject *obj = new MobSAO(m_env, 0, pos, speed, m.attack_throw_object);
				m_env->addActiveObject(obj);
			}

			m_shoot_reload_timer += dtime;

			float reload_time = 15.0;
			if (m_disturb_timer <= 15.0)
				reload_time = 3.0;

			if (
				!m_shooting
				&& m_shoot_reload_timer >= reload_time
				&& !m_next_pos_exists
				&& m_disturb_timer <= 60.0
			) {
				m_shoot_y = 0;
				if (
					m_disturb_timer < 60.0
					&& disturbing_player
					&& disturbing_player_distance < 16*BS
					&& fabs(disturbing_player_norm.Y) < 0.8
				) {
					m_yaw = disturbing_player_dir;
					sendPosition();
					m_shoot_y += disturbing_player_norm.Y;
				}else{
					m_shoot_y = 0.01 * myrand_range(-30,10);
				}
				m_shoot_reload_timer = 0.0;
				m_shooting = true;
				m_shooting_timer = 1.5;
				{
					std::ostringstream os(std::ios::binary);
					// command (2 = shooting)
					writeU8(os, 2);
					// time
					writeF1000(os, m_shooting_timer + 0.1);
					// create message and add to list
					ActiveObjectMessage aom(getId(), false, os.str());
					m_messages_out.push_back(aom);
				}
			}
		}
	}

	if (m.attack_mob_damage > 0) {
		core::array<DistanceSortedActiveObject> objects;
		f32 range = m.attack_mob_range.X;
		if (m.attack_mob_range.Y > range)
			range = m.attack_mob_range.Y;
		if (m.attack_mob_range.Z > range)
			range = m.attack_mob_range.Z;

		m_env->getActiveObjects(m_base_position, range*BS, objects);

		// Sort them.
		// After this, the closest object is the first in the array.
		objects.sort();
		bool hit = false;

		for (u32 i=0; i<objects.size() && !hit; i++) {
			ServerActiveObject *obj = (ServerActiveObject*)objects[i].obj;
			if (obj->getId() == m_id)
				continue;
			if (obj->getType() == ACTIVEOBJECT_TYPE_MOB) {
				((MobSAO*)obj)->doDamage(m.attack_mob_damage);
				hit = true;
			}
		}
		if (hit)
			m_removed = true;
	}

	if (!dont_move) {
		MobMotion mot = getMotion();

		if (mot != MM_CONSTANT && mot != MM_STATIC) {
			m_walk_around_timer -= dtime;
			if (m_walk_around_timer <= 0.0) {
				if (m.motion_type == MMT_FLY || (disturbing_player && (mot == MM_SEEKER || mot == MM_FLEE))) {
					if (!m_walk_around) {
						m_walk_around_timer = 0.2;
						m_walk_around = true;
					}
				}else{
					m_walk_around = !m_walk_around;
					if (m_walk_around) {
						if ((!notices_player && !disturbing_player) || mot != MM_SEEKER)
							m_walk_around_timer = myrand_range(10,20);
					}else{
						m_walk_around_timer = myrand_range(10,20);
					}
				}
			}else if (m_walk_around_timer > 10.0) {
				m_walk_around_timer = 0.2;
				m_walk_around = true;
			}
			if (m_next_pos_exists) {
				v3f pos_f = m_base_position;
				v3f next_pos_f = intToFloat(m_next_pos_i, BS);
				v3f diff = next_pos_f - pos_f;
				v3f dir = diff;
				dir.normalize();
				float speed = BS;
				if (((mot == MM_SEEKER && m.level == MOB_AGGRESSIVE) || mot == MM_FLEE) && disturbing_player)
					speed = BS * 3.0;
				if (m_falling)
					speed = BS * 3.0;

				dir *= dtime * speed;
				bool arrived = false;
				if (dir.getLength() > diff.getLength()) {
					dir = diff;
					arrived = true;
				}
				pos_f += dir;
				m_yaw = wrapDegrees_180(180./PI*atan2(dir.Z, dir.X));
				m_base_position = pos_f;

				if ((pos_f - next_pos_f).getLength() < 0.1 || arrived)
					m_next_pos_exists = false;
			}
		}

		mot = getMotion();

		if (mot == MM_WANDER) {
			if (following) {
				stepMotionSeeker(dtime,1.5);
			}else{
				stepMotionWander(dtime);
			}
		}else if (mot == MM_FLEE) {
			stepMotionFlee(dtime);
		}else if (mot == MM_SEEKER) {
			if (!disturbing_player) {
				stepMotionWander(dtime);
			}else{
				stepMotionSeeker(dtime,0.5);
			}
		}else if (mot == MM_SENTRY) {
			stepMotionSentry(dtime);
		}else if (mot == MM_THROWN) {
			stepMotionThrown(dtime);
		}else if (mot == MM_CONSTANT) {
			stepMotionConstant(dtime);
		}
	}

	if (send_recommended == false)
		return;

	if (m_base_position.getDistanceFrom(m_last_sent_position) > 0.5*BS)
		sendPosition();
}
void MobSAO::stepMotionWander(float dtime)
{
	const MobFeatures& m = content_mob_features(m_content);
	v3s16 pos_i = floatToInt(m_base_position, BS);
	v3s16 pos_size_off(0,0,0);

	if (m.motion_type == MMT_WALK) {
		if (!m_next_pos_exists) {
			/* Check whether to drop down */
			if (checkFreePosition(pos_i + pos_size_off + v3s16(0,-1,0))) {
				m_next_pos_i = pos_i + v3s16(0,-1,0);
				m_next_pos_exists = true;
				m_falling = true;
			}else{
				m_falling = false;
			}
		}

		if (m_walk_around && !m_next_pos_exists) {
			/* Find some position where to go next */
			v3s16 dps[3*3*3];
			int num_dps = 0;
			for (int dx=-1; dx<=1; dx++)
			for (int dy=-1; dy<=1; dy++)
			for (int dz=-1; dz<=1; dz++) {
				if (dx == 0 && dz == 0)
					continue;
				if (dx != 0 && dz != 0 && dy != 0)
					continue;
				dps[num_dps++] = v3s16(dx,dy,dz);
			}
			u32 order[3*3*3];
			get_random_u32_array(order, num_dps);
			v3s16 op = floatToInt(m_oldpos,BS);
			for (int i=0; i<num_dps; i++) {
				v3s16 p = dps[order[i]] + pos_i;
				if (p == op)
					continue;
				if (!checkFreeAndWalkablePosition(p + pos_size_off))
					continue;
				m_next_pos_i = p;
				m_next_pos_exists = true;
				break;
			}
		}
	}else if (m.motion_type == MMT_FLY) {
		bool falling = false;
		bool raising = false;
		if (!m_next_pos_exists) {
			u16 above;
			v3s16 p = pos_i + pos_size_off;
			for (above=0; above < 14; above++) {
				p.Y--;
				if (!checkFreePosition(p))
					break;
			}
			if (above > 12) {
				/* Check whether to drop down */
				if (checkFreePosition(pos_i + pos_size_off + v3s16(0,-1,0))) {
					m_next_pos_i = pos_i + v3s16(0,-1,0);
					falling = true;
				}
			}else if (above < 8) {
				/* Check whether to rise up */
				if (checkFreePosition(pos_i + pos_size_off + v3s16(0,1,0))) {
					m_next_pos_i = pos_i + v3s16(0,1,0);
					raising = true;
				}
			}
		}

		if (m_walk_around && !m_next_pos_exists) {
			/* Find some position where to go next */
			v3s16 dps[3*3*3];
			int num_dps = 0;
			for (int dx=-1; dx<=1; dx++)
			for (int dy=-1; dy<=1; dy++)
			for (int dz=-1; dz<=1; dz++) {
				if (dx == 0 && dy == 0)
					continue;
				if (dx != 0 && dz != 0 && dy != 0)
					continue;
				if (falling && dy > 0)
					continue;
				if (raising && dy < 0)
					continue;
				dps[num_dps++] = v3s16(dx,dy,dz);
			}
			u32 order[3*3*3];
			get_random_u32_array(order, num_dps);
			v3s16 op = floatToInt(m_oldpos,BS);
			for (int i=0; i<num_dps; i++) {
				v3s16 p = dps[order[i]] + pos_i;
				if (p == op)
					continue;
				if (!checkFreePosition(p + pos_size_off))
					continue;
				m_next_pos_i = p;
				m_next_pos_exists = true;
				break;
			}
		}
	}else if (m.motion_type == MMT_FLYLOW || m.motion_type == MMT_SWIM) {
		bool falling = false;
		bool raising = false;
		if (!m_next_pos_exists) {
			u16 above;
			v3s16 p = pos_i + pos_size_off;
			for (above=0; above < 6; above++) {
				p.Y--;
				if (!checkFreePosition(p))
					break;
			}
			if (!above) {
				m_next_pos_i = pos_i + v3s16(0,1,0);
				raising = true;
			}else if (above > 5) {
				/* Check whether to drop down */
				if (checkFreePosition(pos_i + pos_size_off + v3s16(0,-1,0))) {
					m_next_pos_i = pos_i + v3s16(0,-1,0);
					falling = true;
				}
			}else if (above < 2) {
				/* Check whether to rise up */
				if (checkFreePosition(pos_i + pos_size_off + v3s16(0,1,0))) {
					m_next_pos_i = pos_i + v3s16(0,1,0);
					raising = true;
				}
			}
		}

		if (m_walk_around && !m_next_pos_exists) {
			/* Find some position where to go next */
			v3s16 dps[3*3*3];
			int num_dps = 0;
			for (int dx=-1; dx<=1; dx++)
			for (int dy=-1; dy<=1; dy++)
			for (int dz=-1; dz<=1; dz++) {
				if (dx == 0 && dy == 0)
					continue;
				if (dx != 0 && dz != 0 && dy != 0)
					continue;
				if (falling && dy > 0)
					continue;
				if (raising && dy < 0)
					continue;
				dps[num_dps++] = v3s16(dx,dy,dz);
			}
			u32 order[3*3*3];
			get_random_u32_array(order, num_dps);
			v3s16 op = floatToInt(m_oldpos,BS);
			for (int i=0; i<num_dps; i++) {
				v3s16 p = dps[order[i]] + pos_i;
				if (p == op)
					continue;
				if (!checkFreePosition(p + pos_size_off))
					continue;
				m_next_pos_i = p;
				m_next_pos_exists = true;
				break;
			}
		}
	}
}
void MobSAO::stepMotionSeeker(float dtime, float offset)
{
	const MobFeatures& m = content_mob_features(m_content);
	v3s16 pos_i = floatToInt(m_base_position, BS);
	Player *disturbing_player = m_env->getPlayer(m_disturbing_player.c_str());
	if (!disturbing_player) {
		m_next_pos_exists = false;
		return;
	}
	v3f player_pos = disturbing_player->getPosition();
	float distance = m_base_position.getDistanceFrom(player_pos);
	float d;
	float min;
	float max;

	offset = offset*(float)BS;

	min = offset;
	max = offset+6.0;

	if (m.motion_type == MMT_WALK) {
		if (!m_next_pos_exists) {
			/* Check whether to drop down */
			if (checkFreePosition(pos_i + v3s16(0,-1,0))) {
				m_next_pos_i = pos_i + v3s16(0,-1,0);
				m_next_pos_exists = true;
				m_falling = true;
			}else{
				m_falling = false;
			}
		}

		if (m_walk_around && !m_next_pos_exists) {
			/* Find some position where to go next */
			v3s16 dps[3*3*3];
			int num_dps = 0;
			for (int dx=-1; dx<=1; dx++)
			for (int dy=-1; dy<=1; dy++)
			for (int dz=-1; dz<=1; dz++) {
				if (dx == 0 && dz == 0)
					continue;
				if (dx != 0 && dz != 0 && dy != 0)
					continue;
				d = (m_base_position+intToFloat(v3s16(dx,dy,dz),BS)).getDistanceFrom(player_pos);
				if (distance < min) {
					if (d > max)
						continue;
				}else if (d > distance) {
					continue;
				}
				dps[num_dps++] = v3s16(dx,dy,dz);
			}
			u32 order[3*3*3];
			get_random_u32_array(order, num_dps);
			v3s16 op = floatToInt(m_oldpos,BS);
			for (int i=0; i<num_dps; i++) {
				v3s16 p = dps[order[i]] + pos_i;
				if (p == op)
					continue;
				if (!checkFreeAndWalkablePosition(p))
					continue;
				m_next_pos_i = p;
				m_next_pos_exists = true;
				break;
			}
		}
	}else if (m.motion_type == MMT_FLY) {
		bool falling = false;
		bool raising = false;
		if (!m_next_pos_exists) {
			u16 above;
			v3s16 p = pos_i;
			for (above=0; above < 14; above++) {
				p.Y--;
				if (!checkFreePosition(p))
					break;
			}
			if (above > 12) {
				/* Check whether to drop down */
				if (checkFreePosition(pos_i + v3s16(0,-1,0))) {
					m_next_pos_i = pos_i + v3s16(0,-1,0);
					falling = true;
				}
			}else if (above < 8) {
				/* Check whether to rise up */
				if (checkFreePosition(pos_i + v3s16(0,1,0))) {
					m_next_pos_i = pos_i + v3s16(0,1,0);
					raising = true;
				}
			}
		}

		if (m_walk_around && !m_next_pos_exists) {
			/* Find some position where to go next */
			v3s16 dps[3*3*3];
			int num_dps = 0;
			for (int dx=-1; dx<=1; dx++)
			for (int dy=-1; dy<=1; dy++)
			for (int dz=-1; dz<=1; dz++) {
				if (dx == 0 && dy == 0)
					continue;
				if (dx != 0 && dz != 0 && dy != 0)
					continue;
				if (falling && dy > 0)
					continue;
				if (raising && dy < 0)
					continue;
				d = (m_base_position+intToFloat(v3s16(dx,dy,dz),BS)).getDistanceFrom(player_pos);
				if (distance < min) {
					if (d > max)
						continue;
				}else if (d > distance) {
					continue;
				}
				dps[num_dps++] = v3s16(dx,dy,dz);
			}
			u32 order[3*3*3];
			get_random_u32_array(order, num_dps);
			v3s16 op = floatToInt(m_oldpos,BS);
			for (int i=0; i<num_dps; i++) {
				v3s16 p = dps[order[i]] + pos_i;
				if (p == op)
					continue;
				if (!checkFreePosition(p))
					continue;
				m_next_pos_i = p;
				m_next_pos_exists = true;
				break;
			}
		}
	}else if (m.motion_type == MMT_FLYLOW || m.motion_type == MMT_SWIM) {
		bool falling = false;
		bool raising = false;
		if (!m_next_pos_exists) {
			u16 above;
			v3s16 p = pos_i;
			for (above=0; above < 6; above++) {
				p.Y--;
				if (!checkFreePosition(p))
					break;
			}
			if (above > 5) {
				/* Check whether to drop down */
				if (checkFreePosition(pos_i + v3s16(0,-1,0))) {
					m_next_pos_i = pos_i + v3s16(0,-1,0);
					falling = true;
				}
			}else if (above < 2) {
				/* Check whether to rise up */
				if (checkFreePosition(pos_i + v3s16(0,1,0))) {
					m_next_pos_i = pos_i + v3s16(0,1,0);
					raising = true;
				}
			}
		}

		if (m_walk_around && !m_next_pos_exists) {
			/* Find some position where to go next */
			v3s16 dps[3*3*3];
			int num_dps = 0;
			for (int dx=-1; dx<=1; dx++)
			for (int dy=-1; dy<=1; dy++)
			for (int dz=-1; dz<=1; dz++) {
				if (dx == 0 && dy == 0)
					continue;
				if (dx != 0 && dz != 0 && dy != 0)
					continue;
				if (falling && dy > 0)
					continue;
				if (raising && dy < 0)
					continue;
				d = (m_base_position+intToFloat(v3s16(dx,dy,dz),BS)).getDistanceFrom(player_pos);
				if (distance < min) {
					if (d > max)
						continue;
				}else if (d > distance) {
					continue;
				}
				dps[num_dps++] = v3s16(dx,dy,dz);
			}
			u32 order[3*3*3];
			get_random_u32_array(order, num_dps);
			v3s16 op = floatToInt(m_oldpos,BS);
			for (int i=0; i<num_dps; i++) {
				v3s16 p = dps[order[i]] + pos_i;
				if (p == op)
					continue;
				if (!checkFreePosition(p))
					continue;
				m_next_pos_i = p;
				m_next_pos_exists = true;
				break;
			}
		}
	}
}
void MobSAO::stepMotionFlee(float dtime)
{
	const MobFeatures& m = content_mob_features(m_content);
	v3s16 pos_i = floatToInt(m_base_position, BS);
	Player *disturbing_player = m_env->getPlayer(m_disturbing_player.c_str());
	if (!disturbing_player) {
		m_next_pos_exists = false;
		return;
	}
	v3f player_pos = disturbing_player->getPosition();
	float distance = m_base_position.getDistanceFrom(player_pos);

	if (m.motion_type == MMT_WALK) {
		if (!m_next_pos_exists) {
			/* Check whether to drop down */
			if (checkFreePosition(pos_i + v3s16(0,-1,0))) {
				m_next_pos_i = pos_i + v3s16(0,-1,0);
				m_next_pos_exists = true;
				m_falling = true;
			}else{
				m_falling = false;
			}
		}

		if (m_walk_around && !m_next_pos_exists) {
			/* Find some position where to go next */
			v3s16 dps[3*3*3];
			int num_dps = 0;
			for (int dx=-1; dx<=1; dx++)
			for (int dy=-1; dy<=1; dy++)
			for (int dz=-1; dz<=1; dz++) {
				if (dx == 0 && dz == 0)
					continue;
				if (dx != 0 && dz != 0 && dy != 0)
					continue;
				if ((m_base_position+intToFloat(v3s16(dx,dy,dz),BS)).getDistanceFrom(player_pos) < distance)
					continue;
				dps[num_dps++] = v3s16(dx,dy,dz);
			}
			u32 order[3*3*3];
			get_random_u32_array(order, num_dps);
			v3s16 op = floatToInt(m_oldpos,BS);
			for (int i=0; i<num_dps; i++) {
				v3s16 p = dps[order[i]] + pos_i;
				if (p == op)
					continue;
				if (!checkFreeAndWalkablePosition(p))
					continue;
				m_next_pos_i = p;
				m_next_pos_exists = true;
				break;
			}
		}
	}else if (m.motion_type == MMT_FLY) {
		bool falling = false;
		bool raising = false;
		if (!m_next_pos_exists) {
			u16 above;
			v3s16 p = pos_i;
			for (above=0; above < 14; above++) {
				p.Y--;
				if (!checkFreePosition(p))
					break;
			}
			if (above > 12) {
				/* Check whether to drop down */
				if (checkFreePosition(pos_i + v3s16(0,-1,0))) {
					m_next_pos_i = pos_i + v3s16(0,-1,0);
					falling = true;
				}
			}else if (above < 8) {
				/* Check whether to rise up */
				if (checkFreePosition(pos_i + v3s16(0,1,0))) {
					m_next_pos_i = pos_i + v3s16(0,1,0);
					raising = true;
				}
			}
		}

		if (m_walk_around && !m_next_pos_exists) {
			/* Find some position where to go next */
			v3s16 dps[3*3*3];
			int num_dps = 0;
			for (int dx=-1; dx<=1; dx++)
			for (int dy=-1; dy<=1; dy++)
			for (int dz=-1; dz<=1; dz++) {
				if (dx == 0 && dy == 0)
					continue;
				if (dx != 0 && dz != 0 && dy != 0)
					continue;
				if (falling && dy > 0)
					continue;
				if (raising && dy < 0)
					continue;
				if ((m_base_position+intToFloat(v3s16(dx,dy,dz),BS)).getDistanceFrom(player_pos) < distance)
					continue;
				dps[num_dps++] = v3s16(dx,dy,dz);
			}
			u32 order[3*3*3];
			get_random_u32_array(order, num_dps);
			v3s16 op = floatToInt(m_oldpos,BS);
			for (int i=0; i<num_dps; i++) {
				v3s16 p = dps[order[i]] + pos_i;
				if (p == op)
					continue;
				if (!checkFreePosition(p))
					continue;
				m_next_pos_i = p;
				m_next_pos_exists = true;
				break;
			}
		}
	}else if (m.motion_type == MMT_FLYLOW || m.motion_type == MMT_SWIM) {
		bool falling = false;
		bool raising = false;
		if (!m_next_pos_exists) {
			u16 above;
			v3s16 p = pos_i;
			for (above=0; above < 6; above++) {
				p.Y--;
				if (!checkFreePosition(p))
					break;
			}
			if (above > 5) {
				/* Check whether to drop down */
				if (checkFreePosition(pos_i + v3s16(0,-1,0))) {
					m_next_pos_i = pos_i + v3s16(0,-1,0);
					falling = true;
				}
			}else if (above < 2) {
				/* Check whether to rise up */
				if (checkFreePosition(pos_i + v3s16(0,1,0))) {
					m_next_pos_i = pos_i + v3s16(0,1,0);
					raising = true;
				}
			}
		}

		if (m_walk_around && !m_next_pos_exists) {
			/* Find some position where to go next */
			v3s16 dps[3*3*3];
			int num_dps = 0;
			for (int dx=-1; dx<=1; dx++)
			for (int dy=-1; dy<=1; dy++)
			for (int dz=-1; dz<=1; dz++) {
				if (dx == 0 && dy == 0)
					continue;
				if (dx != 0 && dz != 0 && dy != 0)
					continue;
				if (falling && dy > 0)
					continue;
				if (raising && dy < 0)
					continue;
				if ((m_base_position+intToFloat(v3s16(dx,dy,dz),BS)).getDistanceFrom(player_pos) < distance)
					continue;
				dps[num_dps++] = v3s16(dx,dy,dz);
			}
			u32 order[3*3*3];
			get_random_u32_array(order, num_dps);
			v3s16 op = floatToInt(m_oldpos,BS);
			for (int i=0; i<num_dps; i++) {
				v3s16 p = dps[order[i]] + pos_i;
				if (p == op)
					continue;
				if (!checkFreePosition(p))
					continue;
				m_next_pos_i = p;
				m_next_pos_exists = true;
				break;
			}
		}
	}
}
void MobSAO::stepMotionSentry(float dtime)
{
	const MobFeatures& m = content_mob_features(m_content);
	v3s16 pos_i = floatToInt(m_base_position, BS);

	if (m.motion_type == MMT_WALK) {
		if (!m_next_pos_exists) {
			/* Check whether to drop down */
			if (checkFreePosition(pos_i + v3s16(0,-1,0))) {
				m_next_pos_i = pos_i + v3s16(0,-1,0);
				m_next_pos_exists = true;
				m_falling = true;
			}else{
				m_falling = false;
			}
		}

		if (m_walk_around && !m_next_pos_exists) {
			/* Find some position where to go next */
			v3s16 dps[3*3*3];
			int num_dps = 0;
			for (int dx=-1; dx<=1; dx++)
			for (int dy=-1; dy<=1; dy++)
			for (int dz=-1; dz<=1; dz++) {
				if (dx == 0 && dz == 0)
					continue;
				if (dx != 0 && dz != 0 && dy != 0)
					continue;
				if ((m_base_position+intToFloat(v3s16(dx,dy,dz),BS)).getDistanceFrom(m_initial_pos) > (10.0*BS))
					continue;
				dps[num_dps++] = v3s16(dx,dy,dz);
			}
			u32 order[3*3*3];
			get_random_u32_array(order, num_dps);
			for (int i=0; i<num_dps; i++) {
				v3s16 p = dps[order[i]] + pos_i;
				if (!checkFreeAndWalkablePosition(p))
					continue;
				m_next_pos_i = p;
				m_next_pos_exists = true;
				break;
			}
		}
	}else if (m.motion_type == MMT_FLY) {
		bool falling = false;
		bool raising = false;
		if (!m_next_pos_exists) {
			u16 above;
			v3s16 p = pos_i;
			for (above=0; above < 14; above++) {
				p.Y--;
				if (!checkFreePosition(p))
					break;
			}
			if (above > 12) {
				/* Check whether to drop down */
				if (checkFreePosition(pos_i + v3s16(0,-1,0))) {
					m_next_pos_i = pos_i + v3s16(0,-1,0);
					falling = true;
				}
			}else if (above < 8) {
				/* Check whether to rise up */
				if (checkFreePosition(pos_i + v3s16(0,1,0))) {
					m_next_pos_i = pos_i + v3s16(0,1,0);
					raising = true;
				}
			}
		}

		if (m_walk_around && !m_next_pos_exists) {
			/* Find some position where to go next */
			v3s16 dps[3*3*3];
			int num_dps = 0;
			for (int dx=-1; dx<=1; dx++)
			for (int dy=-1; dy<=1; dy++)
			for (int dz=-1; dz<=1; dz++) {
				if (dx == 0 && dy == 0)
					continue;
				if (dx != 0 && dz != 0 && dy != 0)
					continue;
				if (falling && dy > 0)
					continue;
				if (raising && dy < 0)
					continue;
				if ((m_base_position+intToFloat(v3s16(dx,dy,dz),BS)).getDistanceFrom(m_initial_pos) > (10.0*BS))
					continue;
				dps[num_dps++] = v3s16(dx,dy,dz);
			}
			u32 order[3*3*3];
			get_random_u32_array(order, num_dps);
			for (int i=0; i<num_dps; i++) {
				v3s16 p = dps[order[i]] + pos_i;
				if (!checkFreePosition(p))
					continue;
				m_next_pos_i = p;
				m_next_pos_exists = true;
				break;
			}
		}
	}else if (m.motion_type == MMT_FLYLOW || m.motion_type == MMT_SWIM) {
		bool falling = false;
		if (!m_next_pos_exists) {
			/* Check whether to drop down */
			if (checkFreePosition(pos_i + v3s16(0,-1,0))) {
				m_next_pos_i = pos_i + v3s16(0,-1,0);
				falling = true;
			}
		}

		if (m_walk_around && !m_next_pos_exists) {
			/* Find some position where to go next */
			v3s16 dps[3*3*3];
			int num_dps = 0;
			for (int dx=-1; dx<=1; dx++)
			for (int dy=-1; dy<=1; dy++)
			for (int dz=-1; dz<=1; dz++) {
				if (dx == 0 && dy == 0)
					continue;
				if (dx != 0 && dz != 0 && dy != 0)
					continue;
				if (falling && dy > 0)
					continue;
				if ((m_base_position+intToFloat(v3s16(dx,dy,dz),BS)).getDistanceFrom(m_initial_pos) > (10.0*BS))
					continue;
				dps[num_dps++] = v3s16(dx,dy,dz);
			}
			u32 order[3*3*3];
			get_random_u32_array(order, num_dps);
			for (int i=0; i<num_dps; i++) {
				v3s16 p = dps[order[i]] + pos_i;
				if (!checkFreePosition(p))
					continue;
				m_next_pos_i = p;
				m_next_pos_exists = true;
				break;
			}
		}
	}
}
void MobSAO::stepMotionThrown(float dtime)
{
	const MobFeatures& m = content_mob_features(m_content);
	m_base_position += m_speed * dtime;
	m_speed.Y -= 10.0*BS*dtime;

	m_yaw = wrapDegrees_180(180./PI*atan2(m_speed.Z, m_speed.X));

	v3s16 pos_i = floatToInt(m_base_position, BS);
	if (!checkFreePosition(pos_i) || m_removed) {
		if (m.contact_explosion_diameter > 0)
			explodeSquare(pos_i, v3s16(m.contact_explosion_diameter,m.contact_explosion_diameter,m.contact_explosion_diameter));
		if (m.contact_place_node != CONTENT_IGNORE && checkFreeAndWalkablePosition(pos_i+v3s16(0,1,0))) {
			v3s16 pos = pos_i+v3s16(0,1,0);
			m_env->getMap().addNodeWithEvent(pos,MapNode(m.contact_place_node));
		}else if (m.contact_drop_item != CONTENT_IGNORE) {
			InventoryItem *i = InventoryItem::create(m.contact_drop_item,1);
			if (i) {
				ServerActiveObject *obj = i->createSAO(m_env,0,m_base_position);
				if (obj)
					m_env->addActiveObject(obj);
			}
		}
		m_removed = true;
		return;
	}
}
void MobSAO::stepMotionConstant(float dtime)
{
	const MobFeatures& m = content_mob_features(m_content);
	
	m_base_position += m_speed * dtime;

	m_yaw = wrapDegrees_180(180./PI*atan2(m_speed.Z, m_speed.X));

	v3s16 pos_i = floatToInt(m_base_position, BS);
	if (!checkFreePosition(pos_i)) {
		if (m.contact_explosion_diameter > 0)
			explodeSquare(pos_i, v3s16(m.contact_explosion_diameter,m.contact_explosion_diameter,m.contact_explosion_diameter));
		m_removed = true;
		return;
	}
}
bool MobSAO::checkFreePosition(v3s16 p0)
{
	assert(m_env);
	Map *map = &m_env->getMap();
	v3s16 size = content_mob_features(m_content).getSizeBlocks();
	if (content_mob_features(m_content).motion_type == MMT_SWIM) {
		for (int dx=0; dx<size.X; dx++)
		for (int dy=0; dy<size.Y; dy++)
		for (int dz=0; dz<size.Z; dz++) {
			v3s16 dp(dx, dy, dz);
			v3s16 p = p0 + dp;
			MapNode n = map->getNodeNoEx(p);
			if (n.getContent() != CONTENT_WATERSOURCE)
				return false;
		}
	}else{
		for (int dx=0; dx<size.X; dx++)
		for (int dy=0; dy<size.Y; dy++)
		for (int dz=0; dz<size.Z; dz++) {
			v3s16 dp(dx, dy, dz);
			v3s16 p = p0 + dp;
			MapNode n = map->getNodeNoEx(p);
			if (n.getContent() != CONTENT_AIR && content_features(n).walkable)
				return false;
			if (content_features(n).liquid_type == LIQUID_SOURCE)
				return false;
		}
	}
	MapNode n = map->getNodeNoEx(p0+v3s16(0,-1,0));
	if (!content_features(n).jumpable)
		return false;
	return true;
}
bool MobSAO::checkWalkablePosition(v3s16 p0)
{
	assert(m_env);
	v3s16 p = p0 + v3s16(0,-1,0);
	MapNode n = m_env->getMap().getNodeNoEx(p);
	if (n.getContent() != CONTENT_AIR) {
		if (content_features(n).liquid_type == LIQUID_NONE && content_features(n).walkable)
			return true;
	}
	return false;
}
bool MobSAO::checkFreeAndWalkablePosition(v3s16 p0)
{
	if (!checkFreePosition(p0))
		return false;
	if (!checkWalkablePosition(p0))
		return false;
	return true;
}
void MobSAO::explodeSquare(v3s16 p0, v3s16 size)
{
	assert(m_env);
	Map *map = &m_env->getMap();
	core::map<v3s16, MapBlock*> modified_blocks;

	MapNode fire(CONTENT_FIRE);
	std::string blame("damned mobs");

	if (content_mob_features(m_content).level != MOB_DESTRUCTIVE) {
		if (m_env->searchNear(p0,size+v3s16(5,5,5),CONTENT_BORDERSTONE,NULL))
			return;
	}

	for (int dx=0; dx<size.X; dx++)
	for (int dy=0; dy<size.Y; dy++)
	for (int dz=0; dz<size.Z; dz++) {
		v3s16 dp(dx - size.X/2, dy - size.Y/2, dz - size.Z/2);
		v3s16 p = p0 + dp;
		MapNode n = map->getNodeNoEx(p);
		if (n.getContent() == CONTENT_IGNORE)
			continue;
		if (content_features(n).flammable)
			map->addNodeAndUpdate(p,fire,modified_blocks,blame);
	}

	// Send a MEET_OTHER event
	MapEditEvent event;
	event.type = MEET_OTHER;
	for (core::map<v3s16, MapBlock*>::Iterator i = modified_blocks.getIterator(); i.atEnd() == false; i++) {
		v3s16 p = i.getNode()->getKey();
		event.modified_blocks.insert(p, true);
	}
	map->dispatchEvent(&event);
}
InventoryItem* MobSAO::createPickedUpItem(content_t punch_item)
{
	const MobFeatures& m = content_mob_features(m_content);
	ToolItemFeatures f = content_toolitem_features(punch_item);
	
	if (m.punch_action != MPA_PICKUP) {
		if (!m_removed) {
			if (m.special_dropped_item != CONTENT_IGNORE && (m.special_punch_item == TT_NONE || f.type == m.special_punch_item)) {
				if (m.special_dropped_max > 0) {
					if (m_special_count < m.special_dropped_count)
						return NULL;
					m_special_count -= m.special_dropped_count;
					if (m_special_count < 0) {
						m_special_count = 0;
						return NULL;
					}
				}else{
					m_removed = true;
				}
				return InventoryItem::create(m.special_dropped_item,m.special_dropped_count);
			}
			return NULL;
		}
	}
	if (m.dropped_item == "")
		return NULL;
	std::istringstream is(m.dropped_item, std::ios_base::binary);
	InventoryItem *item = InventoryItem::deSerialize(is);
	if (!m_removed)
		m_removed = true;
	return item;
}
u16 MobSAO::punch(content_t punch_item, v3f dir, const std::string &playername)
{
	const MobFeatures& m = content_mob_features(m_content);
	
	if (m.sound_punch != "")
		m_env->addEnvEvent(ENV_EVENT_SOUND,m_base_position,m.sound_punch);
	if (m.punch_action == MPA_IGNORE)
		return 0;
	ToolItemFeatures f = content_toolitem_features(punch_item);
	u16 wear = 655;

	vlprintf(
		CN_ACTION,
		(char*)"%s punches mod (%u) with a '%s' at (%f,%f,%f)",
		playername.c_str(),
		m_id,
		f.description,
		m_base_position.X/BS,
		m_base_position.Y/BS,
		m_base_position.Z/BS
	);

	if (m.special_dropped_item != CONTENT_IGNORE && (m.special_punch_item == TT_NONE || f.type == m.special_punch_item))
		return 0;

	if (m.punch_action == MPA_HARM) {
		m_disturb_timer = 0;
		m_disturbing_player = playername;
		m_next_pos_exists = false; // Cancel moving immediately
		m_angry = true;
		m_walk_around_timer = 0.2;
		m_walk_around = true;

		m_yaw = wrapDegrees_180(180./PI*atan2(dir.Z, dir.X) + 180.);
		v3f new_base_position = m_base_position + dir * BS;
		{
			v3s16 pos_i = floatToInt(new_base_position, BS);
			v3s16 pos_size_off(0,0,0);
			if (m.getSize().X >= 2.5) {
				pos_size_off.X = -1;
				pos_size_off.Y = -1;
			}
			bool free = checkFreePosition(pos_i + pos_size_off);
			if (free)
				m_base_position = new_base_position;
		}
		sendPosition();

		tooluse_t usage;

		if (!get_tool_use(&usage,m_content,0,punch_item,0)) {
			if (usage.diggable)
				doDamage(usage.data);
			wear = usage.wear;
		}
	}else if (m.punch_action == MPA_DIE) {
		m_hp = 0;
		m_removed = true;
	}

	return wear;
}
bool MobSAO::rightClick(Player *player)
{
	// so get the player
	if (!player)
		return false;
	// see if mob is tamable
	const MobFeatures& m = content_mob_features(m_content);
	if (m.tamed_mob == CONTENT_IGNORE)
		return false;
	// get the wielded item
	u16 item_i = player->getSelectedItem();
	InventoryList *ilist = player->inventory.getList("main");
	if (ilist == NULL)
		return false;
	InventoryItem *item = ilist->getItem(item_i);
	if (!item)
		return false;
	// check if it's a craft item
	content_t c = item->getContent();
	if ((c&CONTENT_CRAFTITEM_MASK) != CONTENT_CRAFTITEM_MASK)
		return false;
	CraftItemFeatures *f = content_craftitem_features(c);
	if (f->content != c)
		return false;
	// and edible
	if (!f->consumable || !f->hunger_effect)
		return false;
	// feed the mob
	// after this always return true as inventory has been modified
	if (!config_get_bool((char*)"world.player.inventory.creative") && ilist) {
		// Remove from inventory
		if (item->getCount() == 1) {
			ilist->deleteItem(item_i);
		}else{
			item->remove(1);
			ilist->addDiff(item_i,item);
		}
	}
	if (m_tamed_chance < 1)
		m_tamed_chance = 1;
	// tame it maybe
	if (m.level > MOB_PASSIVE && myrand_range(0,m_tamed_chance) != 0) {
		if (m_tamed_chance > 1)
			m_tamed_chance--;
		return true;
	}

	// add new tamed mob
	ServerActiveObject *obj = new MobSAO(m_env, 0, m_base_position, m.tamed_mob);
	if (obj)
		m_env->addActiveObject(obj);
	// delete this one
	m_removed = true;

	return true;
}
u8 MobSAO::level()
{
	return content_mob_features(m_content).level;
}
void MobSAO::sendPosition()
{
	m_last_sent_position = m_base_position;

	std::ostringstream os(std::ios::binary);
	// command (0 = update position)
	writeU8(os, 0);
	// pos
	writeV3F1000(os, m_base_position);
	// yaw
	writeF1000(os, m_yaw);
	// create message and add to list
	ActiveObjectMessage aom(getId(), false, os.str());
	m_messages_out.push_back(aom);
}
void MobSAO::doDamage(u16 d)
{
	if (d >= m_hp) {
		vlprintf(
			CN_ACTION,
			(char*)"mob (%u) dies at (%f,%f,%f)",
			m_id,
			m_base_position.X/BS,
			m_base_position.Y/BS,
			m_base_position.Z/BS
		);

		// Die
		m_hp = 0;
		m_removed = true;
		return;
	}

	m_hp -= d;

	{
		std::ostringstream os(std::ios::binary);
		// command (1 = damage)
		writeU8(os, 1);
		// amount
		writeU16(os, d);
		// create message and add to list
		ActiveObjectMessage aom(getId(), false, os.str());
		m_messages_out.push_back(aom);
	}
}
