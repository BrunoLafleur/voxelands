/************************************************************************
* Minetest-c55
* Copyright (C) 2010-2011 celeron55, Perttu Ahola <celeron55@gmail.com>
*
* player.h
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

#ifndef PLAYER_HEADER
#define PLAYER_HEADER

#include "common_irrlicht.h"
#ifndef SERVER
#include <IAnimatedMesh.h>
#endif
#include "inventory.h"
#include "collision.h"

#define PLAYERNAME_SIZE 20

#define PLAYERNAME_ALLOWED_CHARS "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_"
#define PLAYER_DEFAULT_CHARDEF "M:10:10:fair:blue:brown:medium:human:green:blue:leather"

#define PLAYERANIM_STAND 0
#define PLAYERANIM_WALK 1
#define PLAYERANIM_DIG 2
#define PLAYERANIM_WALKDIG 3
#define PLAYERANIM_DIE 4
#define PLAYERANIM_SIT 5

/*
	Animation Indices
	Stand		0-79
	Sit		80-161
	Die		162-167
	Walk		168-187
	Dig		189-198
	Dig/Walk	200-219
*/

#define PLAYERFLAG_HOME -1
#define PLAYERFLAG_WHITE 0
#define PLAYERFLAG_BLUE 1
#define PLAYERFLAG_GREEN 2
#define PLAYERFLAG_ORANGE 3
#define PLAYERFLAG_PURPLE 4
#define PLAYERFLAG_RED 5
#define PLAYERFLAG_YELLOW 6
#define PLAYERFLAG_BLACK 7
#define PLAYERFLAG_COUNT 8
#define PLAYERFLAG_JUMP PLAYERFLAG_COUNT

#define PLAYER_FEET	0x01
#define PLAYER_LLEG	0x02
#define PLAYER_RLEG	0x04
#define PLAYER_TORSO	0x08
#define PLAYER_HANDS	0x10
#define PLAYER_LARM	0x20
#define PLAYER_RARM	0x40
#define PLAYER_HEAD	0x80
#define PLAYER_ALL	0xFF

#define DAMAGE_NONE	0x00
#define DAMAGE_UNKNOWN	0x01
#define DAMAGE_FALL	0x02
#define DAMAGE_EXPOSURE	0x03
#define DAMAGE_COLD	0x04
#define DAMAGE_ATTACK	0x05
#define DAMAGE_SPACE	0x06
#define DAMAGE_HUNGER	0x07
#define DAMAGE_AIR	0x08
#define DAMAGE_LAVA	0x09
#define DAMAGE_CACTUS	0x0A
#define DAMAGE_FIRE	0x0B
#define DAMAGE_TNT	0x0C
#define DAMAGE_STEAM	0x0D
#define DAMAGE_POISON	0x0E
#define DAMAGE_SHADOW	0x0F

class Map;

class Player
{
public:


	Player();
	virtual ~Player();

	void resetInventory(bool include_clothes = true);
	void checkInventory();

	virtual void move(f32 dtime, Map &map, f32 pos_max_d) = 0;

	v3f getSpeed()
	{
		return m_speed;
	}

	void setSpeed(v3f speed)
	{
		m_speed = speed;
	}

	// Y direction is ignored
	void accelerate(v3f target_speed, f32 max_increase);

	v3f getPosition()
	{
		if (m_position.X > 30000*BS || m_position.X < -30000*BS)
			m_position.X = ((s32)m_position.X)%(30000*BS);
		if (m_position.Y > 30000*BS || m_position.Y < -30000*BS)
			m_position.Y = ((s32)m_position.Y)%(30000*BS);
		if (m_position.Z > 30000*BS || m_position.Z < -30000*BS)
			m_position.Z = ((s32)m_position.Z)%(30000*BS);
		return m_position;
	}

	v3s16 getLightPosition() const
	{
		return floatToInt(m_position + v3f(0,BS+BS/2,0), BS);
	}

	v3f getEyeOffset()
	{
		// This is at the height of the eyes of the current figure
		v3f s = getScale();
		return v3f(0,(BS+((5*BS)/8))*s.Y,0);
	}

	v3f getEyePosition()
	{
		return m_position + getEyeOffset();
	}

	virtual void setPosition(const v3f &position)
	{
		m_position = position;
		if (m_position.X > 30000*BS || m_position.X < -30000*BS)
			m_position.X = ((s32)m_position.X)%(30000*BS);
		if (m_position.Y > 30000*BS || m_position.Y < -30000*BS)
			m_position.Y = ((s32)m_position.Y)%(30000*BS);
		if (m_position.Z > 30000*BS || m_position.Z < -30000*BS)
			m_position.Z = ((s32)m_position.Z)%(30000*BS);
	}

	void setPitch(f32 pitch)
	{
		m_pitch = pitch;
	}

	virtual void setYaw(f32 yaw)
	{
		m_yaw = yaw;
	}

	f32 getPitch()
	{
		return m_pitch;
	}

	f32 getYaw()
	{
		return m_yaw;
	}

	virtual void updateName(const char *name)
	{
		snprintf(m_name, PLAYERNAME_SIZE, "%s", name);
	}

	virtual void wieldItem(u16 item);
	virtual const InventoryItem *getWieldItem() const
	{
		const InventoryList* const list = inventory.getList("main");
		if (list)
			return list->getItem(m_selected_item);
		return NULL;
	}
	virtual u16 getSelectedItem() {return m_selected_item;}

	const char * getName()
	{
		return m_name;
	}

	virtual bool isLocal() const = 0;

	virtual void updateLight(u8 light_at_pos)
	{
		light = light_at_pos;
	}

	virtual void updateAnim(u8 anim_id, content_t pointed)
	{}

	// NOTE: Use peer_id == 0 for disconnected
	/*virtual bool isClientConnected() { return false; }
	virtual void setClientConnected(bool) {}*/

	/*
		serialize() writes a bunch of text that can contain
		any characters except a '\0', and such an ending that
		deSerialize stops reading exactly at the right point.
	*/
	void serialize(std::ostream &os);
	void deSerialize(std::istream &is);

	bool touching_ground;
	// This oscillates so that the player jumps a bit above the surface
	bool in_water;
	// This is more stable and defines the maximum speed of the player
	bool in_water_stable;
	bool is_climbing;
	bool swimming_up;

	// whether the player is in bed
	bool in_bed;
	// set for 10 hours (10000) after the player last woke up
	f32 wake_timeout;

	u8 light;

	Inventory inventory;
	bool inventory_modified;

	bool craftresult_is_preview;

	u16 health;
	u16 air;
	u16 hunger;
	u16 energy_effect;
	u16 cold_effect;

	u8 dirt;
	u8 wet;
	float wet_dtime;
	u8 blood;

	void addDirt(u8 area)
	{
		dirt |= area;
	}
	void addBlood(u8 area)
	{
		blood |= area;
	}
	void addWater(u8 area)
	{
		wet |= area;
		dirt &= ~area;
		blood &= ~area;
	}

	u16 last_damage;
	void addDamage(u8 area, u8 type, u16 amount)
	{
		last_damage = (type<<8)|area;
		if (amount > health) {
			health = 0;
		}else{
			health -= amount;
		}
	}
	void addHealth(u16 amount)
	{
		last_damage = 0;
		health += amount;
		if (health > 100)
			health = 100;
	}

	u16 peer_id;

	bool getHome(s8 i, v3f &h);
	void setHome(s8 i, v3f h);
	void unsetHome(s8 i);

	f32 getProtection(u8 area, u8 type)
	{
		InventoryList *l;
		InventoryItem *i;
		f32 v = 0;
		const char* list[7] = {"hat","shirt","jacket","decorative","belt","pants","boots"};

		switch (type) {
		case DAMAGE_FALL:
			l = inventory.getList("boots");
			if (l == NULL)
				return 0;
			i = l->getItem(0);
			if (i == NULL)
				return 0;
			return content_clothesitem_features(i->getContent())->armour;
			break;
		case DAMAGE_AIR:
			l = inventory.getList("hat");
			if (l == NULL)
				return 0;
			i = l->getItem(0);
			if (i == NULL)
				return 0;
			return content_clothesitem_features(i->getContent())->suffocate;
			break;
		default:;
		}

		for (int j=0; j<7; j++) {
			if (area && area != PLAYER_ALL) {
				switch (area) {
				case PLAYER_FEET:
					if (j != 6)
						continue;
					break;
				case PLAYER_LLEG:
				case PLAYER_RLEG:
					if (j != 5 && j != 6)
						continue;
					break;
				case PLAYER_TORSO:
					if (j != 1 && j != 2)
						continue;
					break;
				case PLAYER_HANDS:
				case PLAYER_LARM:
				case PLAYER_RARM:
					if (j != 2)
						continue;
					break;
				case PLAYER_HEAD:
					if (j != 0)
						continue;
					break;
				default:;
				}
			}

			l = inventory.getList(list[j]);
			if (l == NULL)
				continue;
			i = l->getItem(0);
			if (i == NULL)
				continue;

			switch (type) {
			case DAMAGE_TNT:
			case DAMAGE_FIRE:
			case DAMAGE_ATTACK:
			case DAMAGE_CACTUS:
				v += content_clothesitem_features(i->getContent())->armour;
				break;
			case DAMAGE_COLD:
				v += content_clothesitem_features(i->getContent())->warmth;
				break;
			case DAMAGE_POISON:
			case DAMAGE_SPACE:
				v += content_clothesitem_features(i->getContent())->vacuum;
				break;
			default:;
			}
		}
		if (v > 1.0)
			return 1.0;
		return v;
	}

	// character def used for skin creation and model scaling
	virtual void setCharDef(std::string d) {m_character = d;}
	std::string getCharDef() {return m_character;}
	void setClothesGiven(bool v) {m_given_clothes = v;}

	v3f getScale();

	// the unclothed player skin parts
	void getSkin(std::vector<std::string> &parts);

protected:
	char m_name[PLAYERNAME_SIZE];
	u16 m_selected_item;
	f32 m_pitch;
	f32 m_yaw;
	v3f m_speed;
	v3f m_position;
	v3f m_home;
	bool m_hashome;
	v3f m_flag[PLAYERFLAG_COUNT];
	bool m_hasflag[PLAYERFLAG_COUNT];
	std::string m_character;
	bool m_given_clothes;

public:

};

/*
	Player on the server
*/

class ServerRemotePlayer : public Player
{
public:
	ServerRemotePlayer():
		animation_id(PLAYERANIM_STAND),
		pointed_id(CONTENT_AIR),
		m_addr("")
	{
	}
	virtual ~ServerRemotePlayer()
	{
	}

	virtual bool isLocal() const
	{
		return false;
	}

	virtual void move(f32 dtime, Map &map, f32 pos_max_d)
	{
	}

	void setAddress(std::string addr) {m_addr = addr;}
	std::string getAddress() {return m_addr;}

	virtual void setCharDef(std::string d);

	u8 animation_id;
	content_t pointed_id;
	virtual void updateAnim(u8 anim_id, content_t pointed)
	{
		animation_id = anim_id;
		pointed_id = pointed;
	}

private:
	std::string m_addr;
};

#ifndef SERVER
#include "path.h"
#include "mesh.h"

/*
	All the other players on the client are these
*/

class RemotePlayer : public Player, public scene::ISceneNode
{
public:
	RemotePlayer(
		scene::ISceneNode* parent=NULL,
		IrrlichtDevice *device=NULL,
		s32 id=0);

	virtual ~RemotePlayer();

	/*
		ISceneNode methods
	*/

	virtual void OnRegisterSceneNode()
	{
		if (IsVisible)
			SceneManager->registerNodeForRendering(this);

		ISceneNode::OnRegisterSceneNode();
	}

	virtual void render()
	{
		// Do nothing
	}

	virtual const core::aabbox3d<f32>& getBoundingBox() const
	{
		return m_box;
	}

	void setPosition(const v3f &position)
	{
		m_oldpos = m_showpos;

		if(m_pos_animation_time < 0.001 || m_pos_animation_time > 1.0)
			m_pos_animation_time = m_pos_animation_time_counter;
		else
			m_pos_animation_time = m_pos_animation_time * 0.9
					+ m_pos_animation_time_counter * 0.1;
		m_pos_animation_time_counter = 0;
		m_pos_animation_counter = 0;

		Player::setPosition(position);
	}

	virtual void setYaw(f32 yaw)
	{
		Player::setYaw(yaw);
		ISceneNode::setRotation(v3f(0, -yaw, 0));
	}

	bool isLocal() const
	{
		return false;
	}

	void updateName(const char *name);

	virtual void wieldItem(u16 item);

	virtual void updateAnim(u8 anim_id, content_t pointed)
	{
		m_anim_id = anim_id;
		if (pointed != CONTENT_IGNORE)
			m_pointed = pointed;
	}

	virtual void updateLight(u8 light_at_pos)
	{
		Player::updateLight(light_at_pos);

		if(m_node == NULL)
			return;

		u8 li = decode_light(light_at_pos);
		video::SColor color(255,li,li,li);
		setMeshVerticesColor(m_node->getMesh(), color);
		m_wield->updateLight(li);
	}

	void updateCameraOffset(v3s16 camera_offset)
	{
		m_camera_offset = intToFloat(camera_offset,BS);
	}

	void move(f32 dtime, Map &map, f32 pos_max_d);

	video::ITexture* getTexture();

private:
	scene::IAnimatedMeshSceneNode *m_node;
	scene::ITextSceneNode* m_text;
	class ExtrudedSpriteSceneNode *m_wield;
	core::aabbox3d<f32> m_box;

	v3f m_oldpos;
	f32 m_pos_animation_counter;
	f32 m_pos_animation_time;
	f32 m_pos_animation_time_counter;
	v3f m_showpos;
	v3f m_camera_offset;
	u8 m_anim_id;
	content_t m_pointed;
	int m_next_foot;
};

struct PlayerControl
{
	PlayerControl()
	{
		forward = false;
		backward = false;
		left = false;
		right = false;
		jump = false;
		sneak = false;
		up = false;
		down = false;
		fast = false;
		free = false;
		digging = false;
		pitch = 0;
		yaw = 0;
	}
	PlayerControl(
		bool a_fwd,
		bool a_back,
		bool a_left,
		bool a_right,
		bool a_jump,
		bool a_sneak,
		bool a_up,
		bool a_down,
		bool a_fast,
		bool a_free,
		bool a_digging,
		float a_pitch,
		float a_yaw
	)
	{
		forward = a_fwd;
		backward = a_back;
		left = a_left;
		right = a_right;
		jump = a_jump;
		sneak = a_sneak;
		up = a_up;
		down = a_down;
		fast = a_fast;
		free = a_free;
		pitch = a_pitch;
		yaw = a_yaw;
		digging = a_digging;
	}
	bool forward;
	bool backward;
	bool left;
	bool right;
	bool jump;
	bool sneak;
	bool up;
	bool down;
	bool fast;
	bool free;
	bool digging;
	float pitch;
	float yaw;
};

class LocalPlayer : public Player
{
public:
	LocalPlayer();
	virtual ~LocalPlayer();

	bool isLocal() const
	{
		return true;
	}

	void move(f32 dtime, Map &map, f32 pos_max_d,
			core::list<CollisionInfo> *collision_info);
	void move(f32 dtime, Map &map, f32 pos_max_d);

	void applyControl(float dtime);

	void setEnergy(float e) {m_energy = e;}
	float getEnergy() {return m_energy;}

	void setIgnoreEnergy(bool v) {m_ignore_energy = v;}

	video::ITexture* getTexture();

	PlayerControl control;

	float energy_effectf;
	float cold_effectf;
	// Used to check if anything changed and prevent sending packets if not
	v3f last_position;
	v3f last_speed;
	float last_pitch;
	float last_yaw;
	float shadow_timer;

private:
	// This is used for determining the sneaking range
	v3s16 m_sneak_node;
	bool m_refresh_sneak_node;
	// Whether the player is allowed to sneak
	bool m_sneak_node_exists;
	v3s16 m_old_node_below;
	content_t m_old_node_below_type;
	float m_energy;
	bool m_can_use_energy;
	bool m_ignore_energy;
	int m_low_energy_effect;

};
#endif // !SERVER

#endif

