/************************************************************************
* Minetest-c55
* Copyright (C) 2010-2011 celeron55, Perttu Ahola <celeron55@gmail.com>
*
* content_nodemeta.cpp
* voxelands - 3d voxel world sandbox game
* Copyright (C) Lisa 'darkrose' Milne 2013-2017 <lisa@ltmnet.com>
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
#include "content_nodemeta.h"
#include "inventory.h"
#include "content_mapnode.h"
#include "content_craft.h"
#include "environment.h"

/*
	FurnaceNodeMetadata
*/

// Prototype
FurnaceNodeMetadata proto_FurnaceNodeMetadata;

FurnaceNodeMetadata::FurnaceNodeMetadata()
{
	NodeMetadata::registerType(typeId(), create);

	m_inventory = new Inventory();
	m_inventory->addList("fuel", 1);
	m_inventory->addList("src", 1);
	m_inventory->addList("main", 4);

	m_active_timer = 0.0;
	m_burn_counter = 0.0;
	m_burn_timer = 0.0;
	m_cook_timer = 0.0;
	m_step_interval = 1.0;
}
FurnaceNodeMetadata::~FurnaceNodeMetadata()
{
	delete m_inventory;
}
u16 FurnaceNodeMetadata::typeId() const
{
	return CONTENT_FURNACE;
}
NodeMetadata* FurnaceNodeMetadata::clone()
{
	FurnaceNodeMetadata *d = new FurnaceNodeMetadata();
	*d->m_inventory = *m_inventory;

	d->m_active_timer = m_active_timer;
	d->m_burn_counter = m_burn_counter;
	d->m_burn_timer = m_burn_timer;
	d->m_cook_timer = m_cook_timer;
	d->m_step_interval = m_step_interval;

	return d;
}
NodeMetadata* FurnaceNodeMetadata::create(std::istream &is)
{
	std::string s;
	FurnaceNodeMetadata *d = new FurnaceNodeMetadata();

	s = deSerializeString(is);
	d->m_active_timer = mystof(s);

	s = deSerializeString(is);
	d->m_burn_counter = mystoi(s);

	s = deSerializeString(is);
	d->m_burn_timer = mystoi(s);

	s = deSerializeString(is);
	d->m_cook_timer = mystof(s);

	s = deSerializeString(is);
	d->m_step_interval = mystof(s);

	d->m_inventory->deSerialize(is);

	return d;
}
void FurnaceNodeMetadata::serializeBody(std::ostream &os)
{
	os<<serializeString(ftos(m_active_timer));
	os<<serializeString(itos(m_burn_counter));
	os<<serializeString(itos(m_burn_timer));
	os<<serializeString(ftos(m_cook_timer));
	os<<serializeString(ftos(m_step_interval));
	m_inventory->serialize(os);
}
std::wstring FurnaceNodeMetadata::infoText()
{
	return narrow_to_wide(gettext("Furnace"));
}
bool FurnaceNodeMetadata::nodeRemovalDisabled()
{
	/*
		Disable removal if not empty
	*/
	InventoryList *list[3] = {
		m_inventory->getList("src"),
		m_inventory->getList("main"),
		m_inventory->getList("fuel")
	};

	for (int i = 0; i < 3; i++) {
		if (list[i] == NULL)
			continue;
		if (list[i]->getUsedSlots() == 0)
			continue;
		return true;
	}
	return false;
}
void FurnaceNodeMetadata::inventoryModified()
{
	vlprintf(CN_INFO,"Furnace inventory modification callback");
}
bool FurnaceNodeMetadata::step(float dtime, v3s16 pos, ServerEnvironment *env)
{
	float cook_time = 10.0;
	bool changed = false;
	bool is_cooking;
	bool cook_ongoing;
	bool room_available;
	InventoryList *dst_list;
	InventoryList *src_list;
	InventoryItem *src_item;
	InventoryList *fuel_list;
	InventoryItem *fuel_item;

	if (dtime > 60.0)
		vlprintf(CN_INFO,"Furnace stepping a long time (%f)",dtime);

	dst_list = m_inventory->getList("main");
	if (!dst_list)
		return false;

	src_list = m_inventory->getList("src");
	if (!src_list)
		return false;

	m_active_timer += dtime;

	while (m_active_timer > m_step_interval) {
		m_active_timer -= m_step_interval;

		is_cooking = false;
		cook_ongoing = false;
		room_available = false;

		src_item = src_list->getItem(0);
		if (src_item && src_item->isCookable(COOK_FURNACE)) {
			is_cooking = true;
			room_available = dst_list->roomForCookedItem(src_item);
			if (room_available && src_item->getCount() > 1)
				cook_ongoing = true;
		}else{
			m_cook_timer = 0.0;
		}

		if (m_cook_timer > cook_time)
			m_cook_timer = cook_time;

		if (m_burn_counter < 1.0 && is_cooking) {
			if (m_cook_timer+m_step_interval < cook_time || cook_ongoing) {
				fuel_list = m_inventory->getList("fuel");
				if (!fuel_list)
					break;
				fuel_item = fuel_list->getItem(0);
				if (fuel_item && fuel_item->isFuel()) {
					content_t c = fuel_item->getContent();
					float v = 0.0;
					if ((c&CONTENT_CRAFTITEM_MASK) == CONTENT_CRAFTITEM_MASK) {
						v = ((CraftItem*)fuel_item)->getFuelTime();
					}else if ((c&CONTENT_TOOLITEM_MASK) == CONTENT_TOOLITEM_MASK) {
						v = ((ToolItem*)fuel_item)->getFuelTime();
					}else{
						v = ((MaterialItem*)fuel_item)->getFuelTime();
					}
					fuel_list->decrementMaterials(1);
					if (c == CONTENT_TOOLITEM_IRON_BUCKET_LAVA) {
						fuel_list->addItem(0,new ToolItem(CONTENT_TOOLITEM_IRON_BUCKET,0,0));
					}
					m_burn_counter += v;
					changed = true;
				}
			}
		}

		if (m_burn_counter <= 0.0) {
			m_active_timer = 0.0;
			m_burn_counter = 0.0;
			m_burn_timer = 0.0;
			break;
		}

		if (m_burn_counter < 1.0)
			continue;

		m_burn_timer += m_step_interval;
		changed = true;
		if (m_burn_timer >= cook_time) {
			m_burn_counter -= 1.0;
			m_burn_timer -= cook_time;
		}

		if (!is_cooking) {
			m_cook_timer = 0.0;
			continue;
		}

		m_cook_timer += m_step_interval;

		if (m_cook_timer >= cook_time) {
			m_cook_timer -= cook_time;
			if (src_item && src_item->isCookable(COOK_FURNACE)) {
				InventoryItem *cookresult = src_item->createCookResult();
				dst_list->addItem(cookresult);
				src_list->decrementMaterials(1);
			}
		}
	}

	return changed;
}
std::string FurnaceNodeMetadata::getDrawSpecString(Player *player)
{
	float v = 0;
	if (m_burn_counter > 0.0 && m_burn_timer > 0.0)
		v = (10.0*m_burn_timer);

	std::string spec("size[9,10]");

	spec += "list[current_name;fuel;1.5,3.5;1,1;]";
	spec += "ring[1.5,3.5;1;#FF0000;";
	spec += itos((int)v);
	spec += "]";
	spec += "list[current_name;src;1.5,1.5;1,1;]";

	spec += "list[current_name;main;5,1.5;2,2;]";

	spec += "list[current_player;main;0.5,5.8;8,1;0,8;]";
	spec += "list[current_player;main;0.5,7;8,3;8,-1;]";

	return spec;
}
std::vector<NodeBox> FurnaceNodeMetadata::getNodeBoxes(MapNode &n)
{
	std::vector<NodeBox> boxes;

	if (m_burn_counter > 0.0) {
		boxes.push_back(NodeBox(
			-0.3125*BS,-0.3125*BS,-0.3125*BS,0.3125*BS,-0.0625*BS,0.3125*BS
		));
	}

	return boxes;
}

/*
	SmelteryNodeMetadata
*/

// Prototype
SmelteryNodeMetadata proto_SmelteryNodeMetadata;

SmelteryNodeMetadata::SmelteryNodeMetadata()
{
	NodeMetadata::registerType(typeId(), create);

	m_inventory = new Inventory();
	m_inventory->addList("fuel", 1);
	m_inventory->addList("src", 2);
	m_inventory->addList("main", 4);
	m_inventory->addList("upgrades", 4);

	m_active_timer = 0.0;
	m_burn_counter = 0.0;
	m_burn_timer = 0.0;
	m_cook_timer = 0.0;
	m_step_interval = 1.0;
	m_is_locked = false;
	m_is_expanded = false;
	m_is_exo = false;
	m_cook_upgrade = 1.0;
	m_burn_upgrade = 1.0;
	m_expanded_slot_id = 0;

	inventoryModified();
}
SmelteryNodeMetadata::~SmelteryNodeMetadata()
{
	delete m_inventory;
}
u16 SmelteryNodeMetadata::typeId() const
{
	return CONTENT_SMELTERY;
}
NodeMetadata* SmelteryNodeMetadata::clone()
{
	SmelteryNodeMetadata *d = new SmelteryNodeMetadata();
	*d->m_inventory = *m_inventory;

	d->m_active_timer = m_active_timer;
	d->m_burn_counter = m_burn_counter;
	d->m_burn_timer = m_burn_timer;
	d->m_cook_timer = m_cook_timer;
	d->m_step_interval = m_step_interval;
	d->m_is_locked = m_is_locked;
	d->m_is_expanded = m_is_expanded;
	d->m_is_exo = m_is_exo;
	d->m_cook_upgrade = m_cook_upgrade;
	d->m_burn_upgrade = m_burn_upgrade;
	d->m_expanded_slot_id = m_expanded_slot_id;

	return d;
}
NodeMetadata* SmelteryNodeMetadata::create(std::istream &is)
{
	std::string s;
	SmelteryNodeMetadata *d = new SmelteryNodeMetadata();

	d->setOwner(deSerializeString(is));

	s = deSerializeString(is);
	d->m_active_timer = mystof(s);

	s = deSerializeString(is);
	d->m_burn_counter = mystoi(s);

	s = deSerializeString(is);
	d->m_burn_timer = mystoi(s);

	s = deSerializeString(is);
	d->m_cook_timer = mystof(s);

	s = deSerializeString(is);
	d->m_step_interval = mystof(s);

	s = deSerializeString(is);
	d->m_is_locked = !!mystoi(s);

	s = deSerializeString(is);
	d->m_is_expanded = !!mystoi(s);

	s = deSerializeString(is);
	d->m_is_exo = !!mystoi(s);

	s = deSerializeString(is);
	d->m_cook_upgrade = mystof(s);

	s = deSerializeString(is);
	d->m_burn_upgrade = mystof(s);

	s = deSerializeString(is);
	d->m_expanded_slot_id = mystoi(s);

	if (d->m_is_expanded) {
		delete d->m_inventory;
		d->m_inventory = new Inventory();
		d->m_inventory->addList("fuel", 1);
		d->m_inventory->addList("src", 2);
		d->m_inventory->addList("main", 16);
		d->m_inventory->addList("upgrades", 4);
	}
	d->m_inventory->deSerialize(is);
	d->inventoryModified();

	return d;
}
void SmelteryNodeMetadata::serializeBody(std::ostream &os)
{
	os<<serializeString(m_owner);
	os<<serializeString(ftos(m_active_timer));
	os<<serializeString(itos(m_burn_counter));
	os<<serializeString(itos(m_burn_timer));
	os<<serializeString(ftos(m_cook_timer));
	os<<serializeString(ftos(m_step_interval));
	os<<serializeString(itos(m_is_locked ? 1 : 0));
	os<<serializeString(itos(m_is_expanded ? 1 : 0));
	os<<serializeString(itos(m_is_exo ? 1 : 0));
	os<<serializeString(ftos(m_cook_upgrade));
	os<<serializeString(ftos(m_burn_upgrade));
	os<<serializeString(itos(m_expanded_slot_id));
	m_inventory->serialize(os);
}
std::wstring SmelteryNodeMetadata::infoText()
{
	char buff[256];
	if (m_is_locked) {
		snprintf(buff, 256, gettext("Locked Smeltery owned by '%s'"), m_owner.c_str());
	}else if (m_is_exo) {
		snprintf(buff, 256, gettext("Exo Smeltery owned by '%s'"), m_owner.c_str());
	}else{
		snprintf(buff, 256, gettext("Smeltery"));
	}
	return narrow_to_wide(buff);
}
bool SmelteryNodeMetadata::nodeRemovalDisabled()
{
	/*
		Disable removal if crusher is not empty
	*/
	InventoryList *list[4] = {
		m_inventory->getList("src"),
		m_inventory->getList("upgrades"),
		m_inventory->getList("main"),
		m_inventory->getList("fuel")
	};

	for (int i = 0; i < 4; i++) {
		if (list[i] == NULL)
			continue;
		if (list[i]->getUsedSlots() == 0)
			continue;
		return true;
	}
	return false;

}
void SmelteryNodeMetadata::inventoryModified()
{
	int i;
	int k;
	int a[3] = {1,1,1};
	int b[3] = {0,0,0};
	int f = -1;
	Inventory *inv;
	InventoryList *il;
	InventoryList *im;
	InventoryItem *itm;
	InventoryList *l = m_inventory->getList("upgrades");
	InventoryList *m = m_inventory->getList("main");
	if (!l || !m)
		return;

	for (i=0; i<4; i++) {
		itm = l->getItem(i);
		if (!itm)
			continue;
		if (itm->getContent() == CONTENT_CRAFTITEM_UPGRADE_STORAGE) {
			f = i;
			if (m_is_expanded) {
				b[0] = 1;
				continue;
			}
			if (m_is_exo)
				continue;
			inv = new Inventory();
			inv->addList("fuel", 1);
			inv->addList("src", 2);
			inv->addList("upgrades", 4);
			inv->addList("main", 16);
			il = inv->getList("upgrades");
			im = inv->getList("main");
			if (!il || !im) {
				delete inv;
				continue;
			}
			InventoryList *ol1 = inv->getList("fuel");
			InventoryList *ol2 = m_inventory->getList("fuel");
			if (ol1 && ol2) {
				itm = ol2->changeItem(0,NULL);
				if (itm)
					ol1->addItem(0,itm);
			}
			ol1 = inv->getList("src");
			ol2 = m_inventory->getList("src");
			if (ol1 && ol2) {
				for (k=0; k<2; k++) {
					itm = ol2->changeItem(k,NULL);
					if (itm)
						ol1->addItem(k,itm);
				}
			}
			for (k=0; k<4; k++) {
				itm = l->changeItem(k,NULL);
				if (itm)
					il->addItem(k,itm);
			}
			for (k=0; k<4; k++) {
				itm = m->changeItem(k,NULL);
				if (itm)
					im->addItem(k,itm);
			}
			delete m_inventory;
			m_inventory = inv;
			l = il;
			m = im;
			a[2] = 0;
			b[0] = 1;
			m_is_expanded = true;
			m_expanded_slot_id = i;
		}else if (itm->getContent() == CONTENT_CRAFTITEM_PADLOCK) {
			if (m_is_exo)
				continue;
			a[2] = 0;
			b[1] = 1;
			m_is_locked = true;
		}else if (itm->getContent() == CONTENT_CRAFTITEM_UPGRADE_EXO) {
			if (m_is_exo) {
				b[2] = 1;
				continue;
			}
			if (m->getUsedSlots() != 0)
				continue;
			if (l->getUsedSlots() != 1)
				continue;
			if (m_is_locked)
				continue;
			if (m_is_expanded)
				continue;
			m_is_exo = true;
			a[0] = 0;
			a[1] = 0;
			b[2] = 1;
		}else if (itm->getContent() == CONTENT_CRAFTITEM_UPGRADE_BURNING) {
			m_burn_upgrade = 1+itm->getCount();
		}else if (itm->getContent() == CONTENT_CRAFTITEM_UPGRADE_COOKING) {
			m_cook_upgrade = 1+itm->getCount();
		}
	}

	if (m_is_expanded && f > -1 && f != m_expanded_slot_id)
		m_expanded_slot_id = f;

	if (m_is_expanded && !b[0]) {
		inv = new Inventory();
		inv->addList("upgrades", 4);
		inv->addList("main", 4);
		inv->addList("fuel", 1);
		inv->addList("src", 2);
		il = inv->getList("upgrades");
		im = inv->getList("main");
		if (!il || !im) {
			delete inv;
		}else{
			InventoryList *ol1 = inv->getList("fuel");
			InventoryList *ol2 = m_inventory->getList("fuel");
			if (ol1 && ol2) {
				itm = ol2->changeItem(0,NULL);
				if (itm)
					ol1->addItem(0,itm);
			}
			ol1 = inv->getList("src");
			ol2 = m_inventory->getList("src");
			if (ol1 && ol2) {
				for (k=0; k<2; k++) {
					itm = ol2->changeItem(k,NULL);
					if (itm)
						ol1->addItem(k,itm);
				}
			}
			for (k=0; k<3; k++) {
				itm = l->changeItem(k,NULL);
				if (itm)
					il->addItem(k,itm);
			}
			for (k=0; k<16; k++) {
				itm = m->changeItem(k,NULL);
				if (itm) {
					if (k > 3) {
						im->addItem(itm);
					}else{
						im->addItem(k,itm);
					}
				}
			}
			delete m_inventory;
			m_inventory = inv;
			l = il;
			m = im;
			m_is_expanded = false;
		}
	}

	if (m_is_locked && !b[1])
		m_is_locked = false;

	if (m_is_exo && !b[2])
		m_is_exo = false;

	if (m_is_expanded || m_is_locked)
		a[2] = 0;

	l->clearAllowed();
	l->addAllowed(CONTENT_CRAFTITEM_UPGRADE_COOKING);
	l->addAllowed(CONTENT_CRAFTITEM_UPGRADE_BURNING);
	if (a[0])
		l->addAllowed(CONTENT_CRAFTITEM_UPGRADE_STORAGE);
	if (a[1])
		l->addAllowed(CONTENT_CRAFTITEM_PADLOCK);
	if (a[2])
		l->addAllowed(CONTENT_CRAFTITEM_UPGRADE_EXO);
}
bool SmelteryNodeMetadata::step(float dtime, v3s16 pos, ServerEnvironment *env)
{
	float cook_time;
	bool changed = false;
	bool is_cooking;
	bool cook_ongoing;
	bool room_available;
	uint8_t cook_mode = 0;
	InventoryList *dst_list;
	InventoryList *src_list;
	InventoryItem *src_item;
	InventoryItem *src_item1 = NULL;
	InventoryList *fuel_list;
	InventoryItem *fuel_item;
	Player *player = NULL;

	if (dtime > 60.0)
		vlprintf(CN_INFO,"Smeltery stepping a long time (%f)",dtime);

	if (m_is_exo) {
		player = env->getPlayer(m_owner.c_str());
		if (!player)
			return false;
		dst_list = player->inventory.getList("exo");
	}else{
		dst_list = m_inventory->getList("main");
	}
	if (!dst_list)
		return false;

	src_list = m_inventory->getList("src");
	if (!src_list)
		return false;

	m_active_timer += dtime;

	if (m_cook_upgrade < 1.0)
		m_cook_upgrade = 1.0;
	if (m_burn_upgrade < 1.0)
		m_burn_upgrade = 1.0;

	/* cook_upgrade/m_cook_time determines time to cook one item */
	/* burn_upgrade*m_burn_counter determines number of items that fuel can cook */

	cook_time = 10.0/m_cook_upgrade;
	if (cook_time < 0.1)
		cook_time = 0.1;

	if (cook_time < m_step_interval)
		m_step_interval = cook_time;

	while (m_active_timer > m_step_interval) {
		m_active_timer -= m_step_interval;

		is_cooking = false;
		cook_ongoing = false;
		room_available = false;

		src_item = src_list->getItem(0);
		src_item1 = src_list->getItem(1);
		if (src_item && src_item1) {
			if (src_item->getContent() == src_item1->getContent()) {
				if (src_item->isCookable(COOK_SMELTERY)) {
					cook_mode = 1;
					is_cooking = true;
					room_available = dst_list->roomForCookedItem(src_item);
					if (room_available)
						cook_ongoing = true;
				}else{
					m_cook_timer = 0.0;
				}
			}else{
				cook_mode = 0;
				InventoryItem *alloy = crafting::getAlloy(src_item->getContent(),src_item1->getContent());
				if (alloy) {
					is_cooking = true;
					room_available = dst_list->roomForItem(alloy);
					if (room_available && src_item->getCount() > 1 && src_item1->getCount() > 1)
						cook_ongoing = true;
					delete alloy;
				}else{
					m_cook_timer = 0.0;
				}
			}
		}else if (src_item) {
			if (src_item->isCookable(COOK_SMELTERY)) {
				cook_mode = 1;
				is_cooking = true;
				room_available = dst_list->roomForCookedItem(src_item);
				if (room_available && src_item->getCount() > 1)
					cook_ongoing = true;
			}else{
				m_cook_timer = 0.0;
			}
		}else if (src_item1) {
			if (src_item1->isCookable(COOK_SMELTERY)) {
				cook_mode = 2;
				is_cooking = true;
				room_available = dst_list->roomForCookedItem(src_item1);
				if (room_available && src_item1->getCount() > 1)
					cook_ongoing = true;
			}else{
				m_cook_timer = 0.0;
			}
		}else{
			m_cook_timer = 0.0;
		}

		if (m_cook_timer > cook_time)
			m_cook_timer = cook_time;

		if (m_burn_counter < 1.0 && is_cooking) {
			if (m_cook_timer+m_step_interval < cook_time || cook_ongoing) {
				fuel_list = m_inventory->getList("fuel");
				if (!fuel_list)
					break;
				fuel_item = fuel_list->getItem(0);
				if (fuel_item && fuel_item->isFuel()) {
					content_t c = fuel_item->getContent();
					float v = 0.0;
					if ((c&CONTENT_CRAFTITEM_MASK) == CONTENT_CRAFTITEM_MASK) {
						v = ((CraftItem*)fuel_item)->getFuelTime();
					}else if ((c&CONTENT_TOOLITEM_MASK) == CONTENT_TOOLITEM_MASK) {
						v = ((ToolItem*)fuel_item)->getFuelTime();
					}else{
						v = ((MaterialItem*)fuel_item)->getFuelTime();
					}
					fuel_list->decrementMaterials(1);
					if (c == CONTENT_TOOLITEM_IRON_BUCKET_LAVA) {
						fuel_list->addItem(0,new ToolItem(CONTENT_TOOLITEM_IRON_BUCKET,0,0));
					}
					m_burn_counter += v*m_burn_upgrade;
					changed = true;
				}
			}
		}

		if (m_burn_counter <= 0.0) {
			m_active_timer = 0.0;
			m_burn_counter = 0.0;
			m_burn_timer = 0.0;
			break;
		}

		if (m_burn_counter < 1.0)
			continue;

		m_burn_timer += m_step_interval;
		changed = true;
		if (m_burn_timer >= cook_time) {
			m_burn_counter -= 1.0;
			m_burn_timer -= cook_time;
		}

		if (!is_cooking) {
			m_cook_timer = 0.0;
			continue;
		}

		m_cook_timer += m_step_interval;

		if (m_cook_timer >= cook_time) {
			m_cook_timer -= cook_time;
			if (cook_mode == 1) {
				if (src_item && src_item->isCookable(COOK_SMELTERY)) {
					InventoryItem *result = src_item->createCookResult();
					dst_list->addItem(result);
					InventoryItem *itm = src_list->takeItem(0,1);
					if (itm)
						delete itm;
					src_list->addDiff(0,src_item);
				}
			}else if (cook_mode == 2) {
				if (src_item1 && src_item1->isCookable(COOK_SMELTERY)) {
					InventoryItem *result = src_item1->createCookResult();
					dst_list->addItem(result);
					InventoryItem *itm = src_list->takeItem(1,1);
					if (itm)
						delete itm;
					src_list->addDiff(1,src_item1);
				}
			}else if (src_item && src_item1) {
				InventoryItem *alloy = crafting::getAlloy(src_item->getContent(),src_item1->getContent());
				dst_list->addItem(alloy);
				{
					InventoryItem *itm = src_list->takeItem(0,1);
					if (itm)
						delete itm;
					src_list->addDiff(0,src_item);
				}
				{
					InventoryItem *itm = src_list->takeItem(1,1);
					if (itm)
						delete itm;
					src_list->addDiff(1,src_item1);
				}
			}
		}
	}

	return changed;
}
std::string SmelteryNodeMetadata::getDrawSpecString(Player *player)
{
	float cook_time;
	float v = 0;

	if (m_cook_upgrade < 1.0)
		m_cook_upgrade = 1.0;
	cook_time = 10.0/m_cook_upgrade;
	if (cook_time < 0.1)
		cook_time = 0.1;
	if (m_burn_counter > 0.0 && m_burn_timer > 0.0)
		v = ((100.0/cook_time)*m_burn_timer);

	std::string spec("size[9,10]");

	InventoryList *l = m_inventory->getList("main");
	if (m_is_expanded && l && l->getUsedSlots() > 4) {
		char buff[10];
		snprintf(buff,10,"%u",CONTENT_CRAFTITEM_UPGRADE_STORAGE);
		if (m_expanded_slot_id == 0) {
			spec += "image[0,0;1,1;inventory:";
			spec += buff;
			spec +="]";
			spec += "list[current_name;upgrades;1,0;3,1;1,4;]";
		}else if (m_expanded_slot_id == 1) {
			spec += "list[current_name;upgrades;0,0;1,1;0,1;]";
			spec += "image[1,0;1,1;inventory:";
			spec += buff;
			spec +="]";
			spec += "list[current_name;upgrades;2,0;2,1;2,4;]";
		}else if (m_expanded_slot_id == 2) {
			spec += "list[current_name;upgrades;0,0;2,1;0,2;]";
			spec += "image[2,0;1,1;inventory:";
			spec += buff;
			spec +="]";
			spec += "list[current_name;upgrades;3,0;1,1;3,4;]";
		}else{
			spec += "list[current_name;upgrades;0,0;3,1;0,3;]";
			spec += "image[3,0;1,1;inventory:";
			spec += buff;
			spec +="]";
		}
	}else{
		spec += "list[current_name;upgrades;0,0;4,1;]";
	}

	spec += "list[current_name;fuel;1.5,3.5;1,1;]";
	spec += "ring[1.5,3.5;1;#FF0000;";
	spec += itos((int)v);
	spec += "]";
	spec += "list[current_name;src;1,1.5;2,1;]";

	if (m_is_expanded) {
		spec += "list[current_name;main;4,1;4,4;]";
	}else if (m_is_exo) {
		spec += "list[current_player;exo;3,1;6,3;]";
	}else{
		spec += "list[current_name;main;5,1.5;2,2;]";
	}

	spec += "list[current_player;main;0.5,5.8;8,1;0,8;]";
	spec += "list[current_player;main;0.5,7;8,3;8,-1;]";

	return spec;
}
std::vector<NodeBox> SmelteryNodeMetadata::getNodeBoxes(MapNode &n)
{
	std::vector<NodeBox> boxes;

	if (m_burn_counter > 0.0) {
		boxes.push_back(NodeBox(
			-0.375*BS,-0.375*BS,-0.375*BS,0.375*BS,0.375*BS,0.4375*BS
		));
	}
	if (m_is_locked) {
		boxes.push_back(NodeBox(
			v3s16(0,90,0),aabb3f(-0.5*BS,-0.375*BS,-0.4375*BS,-0.4375*BS,0.375*BS,-0.375*BS)
		));
		boxes.push_back(NodeBox(
			v3s16(0,90,0),aabb3f(-0.5*BS,0.3125*BS,-0.375*BS,-0.4375*BS,0.375*BS,0.375*BS)
		));
		boxes.push_back(NodeBox(
			v3s16(0,90,0),aabb3f(-0.5*BS,-0.375*BS,-0.375*BS,-0.4375*BS,-0.3125*BS,0.375*BS)
		));
		boxes.push_back(NodeBox(
			v3s16(0,90,0),aabb3f(-0.5*BS,-0.375*BS,0.375*BS,-0.4375*BS,0.375*BS,0.4375*BS)
		));
		boxes.push_back(NodeBox(
			v3s16(0,90,0),aabb3f(-0.5*BS,-0.3125*BS,0.125*BS,-0.4375*BS,0.3125*BS,0.1875*BS)
		));
		boxes.push_back(NodeBox(
			v3s16(0,90,0),aabb3f(-0.5*BS,-0.3125*BS,-0.1875*BS,-0.4375*BS,0.3125*BS,-0.125*BS)
		));
	}
	if (m_is_exo) {
		boxes.push_back(NodeBox(
			v3s16(0,180,0),aabb3f(-0.125*BS,-0.125*BS,0.4375*BS,0.125*BS,0.125*BS,0.5*BS)
		));
	}

	return boxes;
}
std::string SmelteryNodeMetadata::getOwner()
{
	if (m_is_locked || m_is_exo)
		return m_owner;
	return "";
}
std::string SmelteryNodeMetadata::getInventoryOwner()
{
	if (m_is_locked)
		return m_owner;
	return "";
}

/*
	DeprecatedFurnaceNodeMetadata
*/

// Prototype
DeprecatedFurnaceNodeMetadata proto_DeprecatedFurnaceNodeMetadata;

DeprecatedFurnaceNodeMetadata::DeprecatedFurnaceNodeMetadata()
{
	NodeMetadata::registerType(typeId(), create);

	m_inventory = new Inventory();
	m_inventory->addList("fuel", 1);
	m_inventory->addList("src", 1);
	m_inventory->addList("dst", 4);

	m_step_accumulator = 0;
	m_fuel_totaltime = 0;
	m_fuel_time = 0;
	m_src_totaltime = 0;
	m_src_time = 0;
}
DeprecatedFurnaceNodeMetadata::~DeprecatedFurnaceNodeMetadata()
{
	delete m_inventory;
}
u16 DeprecatedFurnaceNodeMetadata::typeId() const
{
	return CONTENT_FURNACE_DEPRECATED;
}
NodeMetadata* DeprecatedFurnaceNodeMetadata::clone()
{
	DeprecatedFurnaceNodeMetadata *d = new DeprecatedFurnaceNodeMetadata();
	*d->m_inventory = *m_inventory;
	d->m_fuel_totaltime = m_fuel_totaltime;
	d->m_fuel_time = m_fuel_time;
	d->m_src_totaltime = m_src_totaltime;
	d->m_src_time = m_src_time;
	return d;
}
NodeMetadata* DeprecatedFurnaceNodeMetadata::create(std::istream &is)
{
	DeprecatedFurnaceNodeMetadata *d = new DeprecatedFurnaceNodeMetadata();

	d->m_inventory->deSerialize(is);

	int temp;
	is>>temp;
	d->m_fuel_totaltime = (float)temp/10;
	is>>temp;
	d->m_fuel_time = (float)temp/10;

	return d;
}
void DeprecatedFurnaceNodeMetadata::serializeBody(std::ostream &os)
{
	m_inventory->serialize(os);
	os<<itos(m_fuel_totaltime*10)<<" ";
	os<<itos(m_fuel_time*10)<<" ";
}
std::wstring DeprecatedFurnaceNodeMetadata::infoText()
{
	//return "DeprecatedFurnace";
	if (m_fuel_time >= m_fuel_totaltime) {
		const InventoryList *src_list = m_inventory->getList("src");
		assert(src_list);
		const InventoryItem *src_item = src_list->getItem(0);

		if (src_item && src_item->isCookable(COOK_FURNACE)) {
			InventoryList *dst_list = m_inventory->getList("dst");
			if(!dst_list->roomForCookedItem(src_item))
				return narrow_to_wide(gettext("Furnace is overloaded"));
			return narrow_to_wide(gettext("Furnace is out of fuel"));
		}else{
			return narrow_to_wide(gettext("Furnace is inactive"));
		}
	}else{
		std::wstring s = narrow_to_wide(gettext("Furnace is active"));
		// Do this so it doesn't always show (0%) for weak fuel
		if (m_fuel_totaltime > 3) {
			s += L" (";
			s += itows(m_fuel_time/m_fuel_totaltime*100);
			s += L"%)";
		}
		return s;
	}
}
bool DeprecatedFurnaceNodeMetadata::nodeRemovalDisabled()
{
	/*
		Disable removal if furnace is not empty
	*/
	InventoryList *list[3] = {m_inventory->getList("src"),
	m_inventory->getList("dst"), m_inventory->getList("fuel")};

	for (int i = 0; i < 3; i++) {
		if (list[i] == NULL)
			continue;
		if (list[i]->getUsedSlots() == 0)
			continue;
		return true;
	}
	return false;

}
void DeprecatedFurnaceNodeMetadata::inventoryModified()
{
	vlprintf(CN_INFO,"Furnace inventory modification callback");
}
bool DeprecatedFurnaceNodeMetadata::step(float dtime, v3s16 pos, ServerEnvironment *env)
{
	if (dtime > 60.0)
		vlprintf(CN_INFO,"Furnace stepping a long time (%f)",dtime);
	// Update at a fixed frequency
	const float interval = 2.0;
	m_step_accumulator += dtime;
	bool changed = false;
	while (m_step_accumulator > interval) {
		m_step_accumulator -= interval;
		dtime = interval;

		InventoryList *dst_list = m_inventory->getList("dst");
		assert(dst_list);

		InventoryList *src_list = m_inventory->getList("src");
		assert(src_list);
		InventoryItem *src_item = src_list->getItem(0);

		bool room_available = false;

		if (src_item && src_item->isCookable(COOK_FURNACE))
			room_available = dst_list->roomForCookedItem(src_item);

		// Start only if there are free slots in dst, so that it can
		// accomodate any result item
		if (room_available) {
			m_src_totaltime = 3;
		}else{
			m_src_time = 0;
			m_src_totaltime = 0;
		}

		/*
			If fuel is burning, increment the burn counters.
			If item finishes cooking, move it to result.
		*/
		if (m_fuel_time < m_fuel_totaltime) {
			m_fuel_time += dtime;
			m_src_time += dtime;
			if (m_src_time >= m_src_totaltime && m_src_totaltime > 0.001 && src_item) {
				InventoryItem *cookresult = src_item->createCookResult();
				dst_list->addItem(cookresult);
				src_list->decrementMaterials(1);
				m_src_time = 0;
				m_src_totaltime = 0;
			}
			changed = true;

			// If the fuel was not used up this step, just keep burning it
			if (m_fuel_time < m_fuel_totaltime)
				continue;
		}

		/*
			Get the source again in case it has all burned
		*/
		src_item = src_list->getItem(0);

		/*
			If there is no source item, or the source item is not cookable,
			or the furnace is still cooking, or the furnace became overloaded, stop loop.
		*/
		if (
			src_item == NULL
			|| !room_available
			|| m_fuel_time < m_fuel_totaltime
			|| dst_list->roomForCookedItem(src_item) == false
		) {
			m_step_accumulator = 0;
			break;
		}

		//vlprintf(CN_INFO,"Furnace is out of fuel");

		InventoryList *fuel_list = m_inventory->getList("fuel");
		assert(fuel_list);
		InventoryItem *fuel_item = fuel_list->getItem(0);
		if (fuel_item && fuel_item->isFuel()) {
			if ((fuel_item->getContent()&CONTENT_CRAFTITEM_MASK) == CONTENT_CRAFTITEM_MASK) {
				m_fuel_totaltime = ((CraftItem*)fuel_item)->getFuelTime();
			}else if ((fuel_item->getContent()&CONTENT_TOOLITEM_MASK) == CONTENT_TOOLITEM_MASK) {
				m_fuel_totaltime = ((ToolItem*)fuel_item)->getFuelTime();
			}else{
				m_fuel_totaltime = ((MaterialItem*)fuel_item)->getFuelTime();
			}
			m_fuel_time = 0;
			content_t c = fuel_item->getContent();
			fuel_list->decrementMaterials(1);
			if (c == CONTENT_TOOLITEM_IRON_BUCKET_LAVA) {
				fuel_list->addItem(0,new ToolItem(CONTENT_TOOLITEM_IRON_BUCKET,0,0));
			}
			changed = true;
		}else{
			m_step_accumulator = 0;
		}
	}
	return changed;
}
std::string DeprecatedFurnaceNodeMetadata::getDrawSpecString(Player *player)
{
	std::string spec("size[8,9]");
	spec += "list[current_name;fuel;2,3;1,1;]";
	spec += "ring[2,3;1;#FF0000;";
	float v = 0;
	if (m_fuel_totaltime > 0.0)
		v = 100.0-((100.0/m_fuel_totaltime)*m_fuel_time);
	spec += itos((int)v);
	spec += "]";
	spec += "list[current_name;src;2,1;1,1;]";
	spec += "list[current_name;dst;5,1;2,2;]";
	spec += "list[current_player;main;0,5;8,4;]";
	return spec;
}
std::vector<NodeBox> DeprecatedFurnaceNodeMetadata::getNodeBoxes(MapNode &n)
{
	std::vector<NodeBox> boxes;
	boxes.clear();

	if (m_fuel_time < m_fuel_totaltime) {
		boxes.push_back(NodeBox(
			-0.3125*BS,-0.25*BS,-0.4*BS,0.3125*BS,0.125*BS,-0.3*BS
		));
	}

	return transformNodeBox(n,boxes);
}
bool DeprecatedFurnaceNodeMetadata::import(NodeMetadata *meta)
{
	if (meta->typeId() != CONTENT_LOCKABLE_FURNACE_DEPRECATED)
		return false;
	LockingDeprecatedFurnaceNodeMetadata *l = (LockingDeprecatedFurnaceNodeMetadata*)meta;
	*m_inventory = *l->getInventory();
	return true;
}

/*
	LockingDeprecatedFurnaceNodeMetadata
*/

// Prototype
LockingDeprecatedFurnaceNodeMetadata proto_LockingDeprecatedFurnaceNodeMetadata;

LockingDeprecatedFurnaceNodeMetadata::LockingDeprecatedFurnaceNodeMetadata()
{
	NodeMetadata::registerType(typeId(), create);

	m_inventory = new Inventory();
	m_inventory->addList("fuel", 1);
	m_inventory->addList("src", 1);
	m_inventory->addList("dst", 4);

	m_step_accumulator = 0;
	m_fuel_totaltime = 0;
	m_fuel_time = 0;
	m_src_totaltime = 0;
	m_src_time = 0;
	m_lock = 0;
}
LockingDeprecatedFurnaceNodeMetadata::~LockingDeprecatedFurnaceNodeMetadata()
{
	delete m_inventory;
}
u16 LockingDeprecatedFurnaceNodeMetadata::typeId() const
{
	return CONTENT_LOCKABLE_FURNACE_DEPRECATED;
}
NodeMetadata* LockingDeprecatedFurnaceNodeMetadata::clone()
{
	LockingDeprecatedFurnaceNodeMetadata *d = new LockingDeprecatedFurnaceNodeMetadata();
	*d->m_inventory = *m_inventory;
	d->m_fuel_totaltime = m_fuel_totaltime;
	d->m_fuel_time = m_fuel_time;
	d->m_src_totaltime = m_src_totaltime;
	d->m_src_time = m_src_time;
	return d;
}
NodeMetadata* LockingDeprecatedFurnaceNodeMetadata::create(std::istream &is)
{
	LockingDeprecatedFurnaceNodeMetadata *d = new LockingDeprecatedFurnaceNodeMetadata();

	d->m_inventory->deSerialize(is);
	d->setOwner(deSerializeString(is));
	d->setInventoryOwner(deSerializeString(is));

	int temp;
	is>>temp;
	d->m_fuel_totaltime = (float)temp/10;
	is>>temp;
	d->m_fuel_time = (float)temp/10;
	is>>temp;
	d->m_lock = (float)temp/10;

	return d;
}
void LockingDeprecatedFurnaceNodeMetadata::serializeBody(std::ostream &os)
{
	m_inventory->serialize(os);
	os<<serializeString(m_owner);
	os<<serializeString(m_inv_owner);
	os<<itos(m_fuel_totaltime*10)<<" ";
	os<<itos(m_fuel_time*10)<<" ";
	os<<itos(m_lock*10)<<" ";
}
std::wstring LockingDeprecatedFurnaceNodeMetadata::infoText()
{
	char buff[256];
	char* s;
	char e[128];
	std::string ostr = m_owner;

	e[0] = 0;

	if (m_inv_owner != "")
		ostr += ","+m_inv_owner;

	if (m_fuel_time >= m_fuel_totaltime) {
		const InventoryList *src_list = m_inventory->getList("src");
		assert(src_list);
		const InventoryItem *src_item = src_list->getItem(0);

		if(src_item && src_item->isCookable(COOK_FURNACE)) {
			InventoryList *dst_list = m_inventory->getList("dst");
			if (!dst_list->roomForCookedItem(src_item)) {
				s = gettext("Locking Furnace is overloaded");
			}else{
				s = gettext("Locking Furnace is out of fuel");
			}
		}else{
			s = gettext("Locking Furnace is inactive");
		}
	}else{
		s = gettext("Locking Furnace is active");
		// Do this so it doesn't always show (0%) for weak fuel
		if (m_fuel_totaltime > 3) {
			uint32_t tt = m_fuel_time/m_fuel_totaltime*100;
			snprintf(e,128, " (%d%%)",tt);
		}
	}

	snprintf(buff,256,"%s (%s)%s",s,ostr.c_str(),e);
	return narrow_to_wide(buff);
}
bool LockingDeprecatedFurnaceNodeMetadata::nodeRemovalDisabled()
{
	/*
		Disable removal if furnace is not empty
	*/
	InventoryList *list[3] = {m_inventory->getList("src"),
	m_inventory->getList("dst"), m_inventory->getList("fuel")};

	for(int i = 0; i < 3; i++) {
		if(list[i] == NULL)
			continue;
		if(list[i]->getUsedSlots() == 0)
			continue;
		return true;
	}
	return false;

}
void LockingDeprecatedFurnaceNodeMetadata::inventoryModified()
{
	vlprintf(CN_INFO,"LockingFurnace inventory modification callback");
}
bool LockingDeprecatedFurnaceNodeMetadata::step(float dtime, v3s16 pos, ServerEnvironment *env)
{
	if (dtime > 60.0)
		vlprintf(CN_INFO,"LockingFurnace stepping a long time (%f)",dtime);
	// Update at a fixed frequency
	const float interval = 2.0;
	m_step_accumulator += dtime;
	bool changed = false;
	while(m_step_accumulator > interval) {
		m_step_accumulator -= interval;
		dtime = interval;

		//infostream<<"Furnace step dtime="<<dtime<<std::endl;

		InventoryList *dst_list = m_inventory->getList("dst");
		assert(dst_list);

		InventoryList *src_list = m_inventory->getList("src");
		assert(src_list);
		InventoryItem *src_item = src_list->getItem(0);

		bool room_available = false;

		if (src_item && src_item->isCookable(COOK_FURNACE)) {
			room_available = dst_list->roomForCookedItem(src_item);
			m_lock = 300.0;
			changed = true;
		}else if (m_lock < 0.0) {
			setInventoryOwner("");
			changed = true;
		}else{
			m_lock -= dtime;
			changed = true;
		}

		// Start only if there are free slots in dst, so that it can
		// accomodate any result item
		if (room_available) {
			m_src_totaltime = 3;
		}else{
			m_src_time = 0;
			m_src_totaltime = 0;
		}

		/*
			If fuel is burning, increment the burn counters.
			If item finishes cooking, move it to result.
		*/
		if (m_fuel_time < m_fuel_totaltime) {
			m_fuel_time += dtime;
			m_src_time += dtime;
			if (m_src_time >= m_src_totaltime && m_src_totaltime > 0.001 && src_item) {
				InventoryItem *cookresult = src_item->createCookResult();
				dst_list->addItem(cookresult);
				src_list->decrementMaterials(1);
				m_src_time = 0;
				m_src_totaltime = 0;
			}
			changed = true;

			// If the fuel was not used up this step, just keep burning it
			if (m_fuel_time < m_fuel_totaltime)
				continue;
		}

		/*
			Get the source again in case it has all burned
		*/
		src_item = src_list->getItem(0);

		/*
			If there is no source item, or the source item is not cookable,
			or the furnace is still cooking, or the furnace became overloaded, stop loop.
		*/
		if (
			src_item == NULL
			|| !room_available ||
			m_fuel_time < m_fuel_totaltime
			|| dst_list->roomForCookedItem(src_item) == false
		) {
			m_step_accumulator = 0;
			break;
		}

		//vlprintf(CN_INFO,"Furnace is out of fuel");

		InventoryList *fuel_list = m_inventory->getList("fuel");
		assert(fuel_list);
		InventoryItem *fuel_item = fuel_list->getItem(0);
		if (fuel_item && fuel_item->isFuel()) {
			if ((fuel_item->getContent()&CONTENT_CRAFTITEM_MASK) == CONTENT_CRAFTITEM_MASK) {
				m_fuel_totaltime = ((CraftItem*)fuel_item)->getFuelTime();
			}else if ((fuel_item->getContent()&CONTENT_TOOLITEM_MASK) == CONTENT_TOOLITEM_MASK) {
				m_fuel_totaltime = ((ToolItem*)fuel_item)->getFuelTime();
			}else{
				m_fuel_totaltime = ((MaterialItem*)fuel_item)->getFuelTime();
			}
			m_fuel_time = 0;
			content_t c = fuel_item->getContent();
			fuel_list->decrementMaterials(1);
			if (c == CONTENT_TOOLITEM_IRON_BUCKET_LAVA) {
				fuel_list->addItem(0,new ToolItem(CONTENT_TOOLITEM_IRON_BUCKET,0,0));
			}
			changed = true;
		}else{
			m_step_accumulator = 0;
		}
	}
	return changed;
}
std::string LockingDeprecatedFurnaceNodeMetadata::getDrawSpecString(Player *player)
{
	std::string spec("size[8,9]");
	spec += "list[current_name;fuel;2,3;1,1;]";
	spec += "ring[2,3;1;#FF0000;";
	float v = 0;
	if (m_fuel_totaltime > 0.0)
		v = 100.0-((100.0/m_fuel_totaltime)*m_fuel_time);
	spec += itos((int)v);
	spec += "]";
	spec += "list[current_name;src;2,1;1,1;]";
	spec += "list[current_name;dst;5,1;2,2;]";
	spec += "list[current_player;main;0,5;8,4;]";
	return spec;
}
std::vector<NodeBox> LockingDeprecatedFurnaceNodeMetadata::getNodeBoxes(MapNode &n)
{
	std::vector<NodeBox> boxes;
	boxes.clear();

	if (m_fuel_time < m_fuel_totaltime) {
		boxes.push_back(NodeBox(
			-0.3125*BS,-0.25*BS,-0.4*BS,0.3125*BS,0.125*BS,-0.3*BS
		));
	}

	return transformNodeBox(n,boxes);
}
bool LockingDeprecatedFurnaceNodeMetadata::import(NodeMetadata *meta)
{
	if (meta->typeId() != CONTENT_FURNACE_DEPRECATED)
		return false;
	DeprecatedFurnaceNodeMetadata *l = (DeprecatedFurnaceNodeMetadata*)meta;
	*m_inventory = *l->getInventory();
	return true;
}

/*
	CampFireNodeMetadata
*/

// Prototype
CampFireNodeMetadata proto_CampFireNodeMetadata;

CampFireNodeMetadata::CampFireNodeMetadata()
{
	NodeMetadata::registerType(typeId(), create);

	m_inventory = new Inventory();
	m_inventory->addList("fuel", 1);
	m_inventory->addList("src", 3);
	m_inventory->addList("dst", 1);

	m_active_timer = 0.0;
	m_burn_counter = 0.0;
	m_burn_timer = 0.0;
	m_cook_timer = 0.0;
	m_has_pots = false;

	inventoryModified();
}
CampFireNodeMetadata::~CampFireNodeMetadata()
{
	delete m_inventory;
}
u16 CampFireNodeMetadata::typeId() const
{
	return CONTENT_CAMPFIRE;
}
NodeMetadata* CampFireNodeMetadata::clone()
{
	CampFireNodeMetadata *d = new CampFireNodeMetadata();
	*d->m_inventory = *m_inventory;

	d->m_active_timer = m_active_timer;
	d->m_burn_counter = m_burn_counter;
	d->m_burn_timer = m_burn_timer;
	d->m_cook_timer = m_cook_timer;
	d->m_has_pots = m_has_pots;

	return d;
}
NodeMetadata* CampFireNodeMetadata::create(std::istream &is)
{
	std::string s;
	CampFireNodeMetadata *d = new CampFireNodeMetadata();

	d->m_inventory->deSerialize(is);

	s = deSerializeString(is);
	d->m_active_timer = mystof(s);

	s = deSerializeString(is);
	d->m_burn_counter = mystoi(s);

	s = deSerializeString(is);
	d->m_burn_timer = mystoi(s);

	s = deSerializeString(is);
	d->m_cook_timer = mystof(s);

	s = deSerializeString(is);
	d->m_has_pots = !!mystoi(s);

	d->inventoryModified();

	return d;
}
void CampFireNodeMetadata::serializeBody(std::ostream &os)
{
	m_inventory->serialize(os);
	os<<serializeString(ftos(m_active_timer));
	os<<serializeString(itos(m_burn_counter));
	os<<serializeString(itos(m_burn_timer));
	os<<serializeString(ftos(m_cook_timer));
	os<<serializeString(itos(m_has_pots ? 1 : 0));
}
std::wstring CampFireNodeMetadata::infoText()
{
	char buff[256];
	char* s;
	char e[128];
	e[0] = 0;

	if (m_burn_counter > 0.0) {
		s = gettext("CampFire is active");
		if (m_cook_timer > 0.0) {
			uint32_t tt = m_cook_timer/4.0*100;
			snprintf(e,128, " (%d%%)",tt);
		}
	}else{
		s = gettext("CampFire is inactive");
	}

	snprintf(buff,256,"%s%s",s,e);
	return narrow_to_wide(buff);
}
bool CampFireNodeMetadata::nodeRemovalDisabled()
{
	/*
		Disable removal if furnace is not empty
	*/
	InventoryList *list[3] = {
		m_inventory->getList("src"),
		m_inventory->getList("dst"),
		m_inventory->getList("fuel")
	};

	for (int i = 0; i < 3; i++) {
		if (list[i] == NULL)
			continue;
		if (list[i]->getUsedSlots() == 0)
			continue;
		return true;
	}
	return false;
}
void CampFireNodeMetadata::inventoryModified()
{
	InventoryList *src_list;
	InventoryItem *src_item = NULL;

	src_list = m_inventory->getList("src");
	if (!src_list)
		return;

	src_item = src_list->getItem(0);
	if (!src_item) {
		m_has_pots = false;
		return;
	}

	if (src_item->getContent() != CONTENT_CLAY_VESSEL) {
		m_has_pots = false;
		return;
	}

	m_has_pots = true;
}
bool CampFireNodeMetadata::step(float dtime, v3s16 pos, ServerEnvironment *env)
{
	float cook_time = 15.0;
	bool changed = false;
	bool is_cooking;
	bool cook_ongoing;
	bool room_available;
	uint8_t pots_mode = 0;
	InventoryList *dst_list;
	InventoryList *src_list;
	InventoryItem *src_item = NULL;
	InventoryItem *src_item1 = NULL;
	InventoryList *fuel_list;
	InventoryItem *fuel_item;
	uint16_t mode = COOK_FIRE;

	/* requires air above it (because why not?) */
	if (content_features(env->getMap().getNodeNoEx(pos+v3s16(0,1,0),NULL).getContent()).air_equivalent == false)
		return false;

	if (dtime > 60.0)
		vlprintf(CN_INFO,"Campfire stepping a long time (%f)",dtime);

	dst_list = m_inventory->getList("dst");
	if (!dst_list)
		return false;

	src_list = m_inventory->getList("src");
	if (!src_list)
		return false;

	if (m_has_pots)
		mode = COOK_FIRE_POT;

	m_active_timer += dtime;

	while (m_active_timer > 1.0) {
		m_active_timer -= 1.0;

		is_cooking = false;
		cook_ongoing = false;
		room_available = false;

		if (m_has_pots) {
			src_item = src_list->getItem(1);
			src_item1 = src_list->getItem(2);
			if (src_item && src_item1) {
				if (src_item->getContent() == src_item1->getContent()) {
					if (src_item->isCookable(mode)) {
						pots_mode = 1;
						is_cooking = true;
						room_available = dst_list->roomForCookedItem(src_item);
						if (room_available)
							cook_ongoing = true;
					}else{
						m_cook_timer = 0.0;
					}
				}else{
					pots_mode = 0;
					InventoryItem *alloy = crafting::getAlloy(src_item->getContent(),src_item1->getContent());
					if (alloy) {
						is_cooking = true;
						room_available = dst_list->roomForItem(alloy);
						if (room_available && src_item->getCount() > 1 && src_item1->getCount() > 1)
							cook_ongoing = true;
						delete alloy;
					}else{
						m_cook_timer = 0.0;
					}
				}
			}else if (src_item) {
				if (src_item->isCookable(mode)) {
					pots_mode = 1;
					is_cooking = true;
					room_available = dst_list->roomForCookedItem(src_item);
					if (room_available && src_item->getCount() > 1)
						cook_ongoing = true;
				}else{
					m_cook_timer = 0.0;
				}
			}else if (src_item1) {
				if (src_item1->isCookable(mode)) {
					pots_mode = 2;
					is_cooking = true;
					room_available = dst_list->roomForCookedItem(src_item1);
					if (room_available && src_item1->getCount() > 1)
						cook_ongoing = true;
				}else{
					m_cook_timer = 0.0;
				}
			}else{
				m_cook_timer = 0.0;
			}
		}else{
			src_item = src_list->getItem(0);
			if (src_item && src_item->isCookable(mode)) {
				is_cooking = true;
				room_available = dst_list->roomForCookedItem(src_item);
				if (room_available && src_item->getCount() > 1)
					cook_ongoing = true;
			}else{
				m_cook_timer = 0.0;
			}
		}

		if (m_cook_timer > cook_time)
			m_cook_timer = cook_time;

		if (m_burn_counter < 1.0 && is_cooking) {
			if (m_cook_timer+1.0 < cook_time || cook_ongoing) {
				fuel_list = m_inventory->getList("fuel");
				if (!fuel_list)
					break;
				fuel_item = fuel_list->getItem(0);
				if (fuel_item && fuel_item->isFuel()) {
					content_t c = fuel_item->getContent();
					float v = 0.0;
					if ((c&CONTENT_CRAFTITEM_MASK) == CONTENT_CRAFTITEM_MASK) {
						v = ((CraftItem*)fuel_item)->getFuelTime();
					}else if ((c&CONTENT_TOOLITEM_MASK) == CONTENT_TOOLITEM_MASK) {
						v = ((ToolItem*)fuel_item)->getFuelTime();
					}else{
						v = ((MaterialItem*)fuel_item)->getFuelTime();
					}
					fuel_list->decrementMaterials(1);
					if (c == CONTENT_TOOLITEM_IRON_BUCKET_LAVA) {
						fuel_list->addItem(0,new ToolItem(CONTENT_TOOLITEM_IRON_BUCKET,0,0));
					}
					m_burn_counter += v;
					changed = true;
				}
			}
		}

		if (m_burn_counter <= 0.0) {
			m_active_timer = 0.0;
			m_burn_counter = 0.0;
			m_burn_timer = 0.0;
			break;
		}

		if (m_burn_counter < 1.0)
			continue;

		m_burn_timer += 1.0;
		changed = true;
		if (m_burn_timer >= cook_time) {
			m_burn_counter -= 1.0;
			m_burn_timer -= cook_time;
		}

		if (!is_cooking) {
			m_cook_timer = 0.0;
			continue;
		}

		m_cook_timer += 1.0;

		if (m_cook_timer >= cook_time) {
			m_cook_timer -= cook_time;
			if (m_has_pots) {
				if (pots_mode == 1) {
					if (src_item && src_item->isCookable(mode)) {
						InventoryItem *result = src_item->createCookResult();
						dst_list->addItem(result);
						InventoryItem *itm = src_list->takeItem(1,1);
						if (itm)
							delete itm;
						src_list->addDiff(1,src_item);
					}
				}else if (pots_mode == 2) {
					if (src_item1 && src_item1->isCookable(mode)) {
						InventoryItem *result = src_item1->createCookResult();
						dst_list->addItem(result);
						InventoryItem *itm = src_list->takeItem(2,1);
						if (itm)
							delete itm;
						src_list->addDiff(2,src_item1);
					}
				}else if (src_item && src_item1) {
					InventoryItem *alloy = crafting::getAlloy(src_item->getContent(),src_item1->getContent());
					dst_list->addItem(alloy);
					{
						InventoryItem *itm = src_list->takeItem(1,1);
						if (itm)
							delete itm;
						src_list->addDiff(1,src_item);
					}
					{
						InventoryItem *itm = src_list->takeItem(2,1);
						if (itm)
							delete itm;
						src_list->addDiff(2,src_item1);
					}
				}
			}else{
				if (src_item && src_item->isCookable(mode)) {
					InventoryItem *crushresult = src_item->createCookResult();
					dst_list->addItem(crushresult);
					src_list->decrementMaterials(1);
				}
			}
		}
	}

	return changed;
}
std::string CampFireNodeMetadata::getDrawSpecString(Player *player)
{
	std::string spec("size[8,9]");
	float v = 0;
	if (m_burn_counter > 0.0 && m_burn_timer > 0.0)
		v = ((100.0/15)*m_burn_timer);

	spec += "list[current_name;fuel;2,2;1,1;]";
	spec += "ring[2,2;1;#FF0000;";
	spec += itos((int)v);
	spec += "]";


	if (m_has_pots) {
		InventoryList *src_list = m_inventory->getList("src");
		if (src_list && (src_list->getItem(1) || src_list->getItem(2))) {
			char buff[10];
			snprintf(buff,10,"%u",CONTENT_CLAY_VESSEL);
			spec += "image[5,0.5;1,1;inventory:";
			spec += buff;
			spec += "]";
		}else{
			spec += "list[current_name;src;5,0.5;1,1;0,1;]";
		}
		spec += "list[current_name;src;4.5,1.5;2,1;1,-1;]";
	}else{
		spec += "list[current_name;src;5,1;1,1;]";
	}
	spec += "list[current_name;dst;5,3;1,1;]";

	spec += "list[current_player;main;0,4.8;8,1;0,8;]";
	spec += "list[current_player;main;0,6;8,3;8,-1;]";

	return spec;
}
std::vector<NodeBox> CampFireNodeMetadata::getNodeBoxes(MapNode &n)
{
	std::vector<NodeBox> boxes;

	if (m_burn_counter > 0.0) {
		boxes.push_back(NodeBox(
			-0.3125*BS,-0.25*BS,-0.4*BS,0.3125*BS,0.125*BS,-0.3*BS
		));
	}

	return boxes;
}

bool CampFireNodeMetadata::isActive()
{
	if (m_burn_counter > 0.0)
		return false;
	return true;
}

/*
	CrusherNodeMetadata
*/

// Prototype
CrusherNodeMetadata proto_CrusherNodeMetadata;

CrusherNodeMetadata::CrusherNodeMetadata()
{
	NodeMetadata::registerType(typeId(), create);

	m_inventory = new Inventory();
	m_inventory->addList("fuel", 1);
	m_inventory->addList("src", 1);
	m_inventory->addList("main", 4);
	m_inventory->addList("upgrades", 3);

	m_active_timer = 0.0;
	m_burn_counter = 0.0;
	m_burn_timer = 0.0;
	m_cook_timer = 0.0;
	m_step_interval = 1.0;
	m_is_locked = false;
	m_is_expanded = false;
	m_is_exo = false;
	m_cook_upgrade = 1.0;
	m_burn_upgrade = 1.0;
	m_expanded_slot_id = 0;

	inventoryModified();
}
CrusherNodeMetadata::~CrusherNodeMetadata()
{
	delete m_inventory;
}
u16 CrusherNodeMetadata::typeId() const
{
	return CONTENT_CRUSHER;
}
NodeMetadata* CrusherNodeMetadata::clone()
{
	CrusherNodeMetadata *d = new CrusherNodeMetadata();
	*d->m_inventory = *m_inventory;

	d->m_active_timer = m_active_timer;
	d->m_burn_counter = m_burn_counter;
	d->m_burn_timer = m_burn_timer;
	d->m_cook_timer = m_cook_timer;
	d->m_step_interval = m_step_interval;
	d->m_is_locked = m_is_locked;
	d->m_is_expanded = m_is_expanded;
	d->m_is_exo = m_is_exo;
	d->m_cook_upgrade = m_cook_upgrade;
	d->m_burn_upgrade = m_burn_upgrade;
	d->m_expanded_slot_id = m_expanded_slot_id;

	return d;
}
NodeMetadata* CrusherNodeMetadata::create(std::istream &is)
{
	std::string s;
	CrusherNodeMetadata *d = new CrusherNodeMetadata();

	d->setOwner(deSerializeString(is));

	s = deSerializeString(is);
	d->m_active_timer = mystof(s);

	s = deSerializeString(is);
	d->m_burn_counter = mystoi(s);

	s = deSerializeString(is);
	d->m_burn_timer = mystoi(s);

	s = deSerializeString(is);
	d->m_cook_timer = mystof(s);

	s = deSerializeString(is);
	d->m_step_interval = mystof(s);

	s = deSerializeString(is);
	d->m_is_locked = !!mystoi(s);

	s = deSerializeString(is);
	d->m_is_expanded = !!mystoi(s);

	s = deSerializeString(is);
	d->m_is_exo = !!mystoi(s);

	s = deSerializeString(is);
	d->m_cook_upgrade = mystof(s);

	s = deSerializeString(is);
	d->m_burn_upgrade = mystof(s);

	s = deSerializeString(is);
	d->m_expanded_slot_id = mystoi(s);

	if (d->m_is_expanded) {
		delete d->m_inventory;
		d->m_inventory = new Inventory();
		d->m_inventory->addList("fuel", 1);
		d->m_inventory->addList("src", 1);
		d->m_inventory->addList("main", 16);
		d->m_inventory->addList("upgrades", 3);
	}
	d->m_inventory->deSerialize(is);
	d->inventoryModified();

	return d;
}
void CrusherNodeMetadata::serializeBody(std::ostream &os)
{
	os<<serializeString(m_owner);
	os<<serializeString(ftos(m_active_timer));
	os<<serializeString(itos(m_burn_counter));
	os<<serializeString(itos(m_burn_timer));
	os<<serializeString(ftos(m_cook_timer));
	os<<serializeString(ftos(m_step_interval));
	os<<serializeString(itos(m_is_locked ? 1 : 0));
	os<<serializeString(itos(m_is_expanded ? 1 : 0));
	os<<serializeString(itos(m_is_exo ? 1 : 0));
	os<<serializeString(ftos(m_cook_upgrade));
	os<<serializeString(ftos(m_burn_upgrade));
	os<<serializeString(itos(m_expanded_slot_id));
	m_inventory->serialize(os);
}
std::wstring CrusherNodeMetadata::infoText()
{
	char buff[256];
	if (m_is_locked) {
		snprintf(buff, 256, gettext("Locked Crusher owned by '%s'"), m_owner.c_str());
	}else if (m_is_exo) {
		snprintf(buff, 256, gettext("Exo Crusher owned by '%s'"), m_owner.c_str());
	}else{
		snprintf(buff, 256, gettext("Crusher"));
	}
	return narrow_to_wide(buff);
}
bool CrusherNodeMetadata::nodeRemovalDisabled()
{
	/*
		Disable removal if crusher is not empty
	*/
	InventoryList *list[3] = {
		m_inventory->getList("src"),
		m_inventory->getList("main"),
		m_inventory->getList("fuel")
	};

	for (int i = 0; i < 3; i++) {
		if (list[i] == NULL)
			continue;
		if (list[i]->getUsedSlots() == 0)
			continue;
		return true;
	}
	return false;

}
void CrusherNodeMetadata::inventoryModified()
{
	int i;
	int k;
	int a[3] = {1,1,1};
	int b[3] = {0,0,0};
	int f = -1;
	Inventory *inv;
	InventoryList *il;
	InventoryList *im;
	InventoryItem *itm;
	InventoryList *l = m_inventory->getList("upgrades");
	InventoryList *m = m_inventory->getList("main");
	if (!l || !m)
		return;

	for (i=0; i<3; i++) {
		itm = l->getItem(i);
		if (!itm)
			continue;
		if (itm->getContent() == CONTENT_CRAFTITEM_UPGRADE_STORAGE) {
			f = i;
			if (m_is_expanded) {
				b[0] = 1;
				continue;
			}
			if (m_is_exo)
				continue;
			inv = new Inventory();
			inv->addList("upgrades", 3);
			inv->addList("main", 16);
			il = inv->getList("upgrades");
			im = inv->getList("main");
			if (!il || !im) {
				delete inv;
				continue;
			}
			vlprintf(CN_INFO,"inv size: '%u' '%u'",l->getSize(),m->getSize());
			for (k=0; k<3; k++) {
				itm = l->changeItem(k,NULL);
				if (itm)
					il->addItem(k,itm);
			}
			for (k=0; k<4; k++) {
				itm = m->changeItem(k,NULL);
				if (itm)
					im->addItem(k,itm);
			}
			delete m_inventory;
			m_inventory = inv;
			l = il;
			m = im;
			a[2] = 0;
			b[0] = 1;
			m_is_expanded = true;
			m_expanded_slot_id = i;
		}else if (itm->getContent() == CONTENT_CRAFTITEM_PADLOCK) {
			if (m_is_exo)
				continue;
			a[2] = 0;
			b[1] = 1;
			m_is_locked = true;
		}else if (itm->getContent() == CONTENT_CRAFTITEM_UPGRADE_EXO) {
			if (m_is_exo) {
				b[2] = 1;
				continue;
			}
			if (m->getUsedSlots() != 0)
				continue;
			if (l->getUsedSlots() != 1)
				continue;
			if (m_is_locked)
				continue;
			if (m_is_expanded)
				continue;
			m_is_exo = true;
			a[0] = 0;
			a[1] = 0;
			b[2] = 1;
		}
	}

	if (m_is_expanded && f > -1 && f != m_expanded_slot_id)
		m_expanded_slot_id = f;

	if (m_is_expanded && !b[0]) {
		inv = new Inventory();
		inv->addList("upgrades", 3);
		inv->addList("main", 4);
		il = inv->getList("upgrades");
		im = inv->getList("main");
		if (!il || !im) {
			delete inv;
		}else{
			for (k=0; k<3; k++) {
				itm = l->changeItem(k,NULL);
				if (itm)
					il->addItem(k,itm);
			}
			for (k=0; k<16; k++) {
				itm = m->changeItem(k,NULL);
				if (itm) {
					if (k > 3) {
						im->addItem(itm);
					}else{
						im->addItem(k,itm);
					}
				}
			}
			delete m_inventory;
			m_inventory = inv;
			l = il;
			m = im;
			m_is_expanded = false;
		}
	}

	if (m_is_locked && !b[1])
		m_is_locked = false;

	if (m_is_exo && !b[2])
		m_is_exo = false;

	if (m_is_expanded || m_is_locked || m->getUsedSlots() != 0)
		a[2] = 0;

	l->clearAllowed();
	if (a[0])
		l->addAllowed(CONTENT_CRAFTITEM_UPGRADE_STORAGE);
	if (a[1])
		l->addAllowed(CONTENT_CRAFTITEM_PADLOCK);
	if (a[2])
		l->addAllowed(CONTENT_CRAFTITEM_UPGRADE_EXO);
}
bool CrusherNodeMetadata::step(float dtime, v3s16 pos, ServerEnvironment *env)
{
	float cook_time;
	bool changed = false;
	bool is_cooking;
	bool cook_ongoing;
	bool room_available;
	InventoryList *dst_list;
	InventoryList *src_list;
	InventoryItem *src_item;
	InventoryList *fuel_list;
	InventoryItem *fuel_item;
	Player *player = NULL;

	/* requires air above it (because that's where the imput slot on the nodebox is) */
	if (content_features(env->getMap().getNodeNoEx(pos+v3s16(0,1,0),NULL).getContent()).air_equivalent == false)
		return false;

	if (dtime > 60.0)
		vlprintf(CN_INFO,"Crusher stepping a long time (%f)",dtime);

	if (m_is_exo) {
		player = env->getPlayer(m_owner.c_str());
		if (!player)
			return false;
		dst_list = player->inventory.getList("exo");
	}else{
		dst_list = m_inventory->getList("main");
	}
	if (!dst_list)
		return false;

	src_list = m_inventory->getList("src");
	if (!src_list)
		return false;

	m_active_timer += dtime;

	if (m_cook_upgrade < 1.0)
		m_cook_upgrade = 1.0;
	if (m_burn_upgrade < 1.0)
		m_burn_upgrade = 1.0;

	/* cook_upgrade/m_cook_time determines time to cook one item */
	/* burn_upgrade/m_burn_time determines number of items that fuel can cook */

	cook_time = 4.0/m_cook_upgrade;
	if (cook_time < 0.1)
		cook_time = 0.1;

	if (cook_time < m_step_interval)
		m_step_interval = cook_time;

	while (m_active_timer > m_step_interval) {
		m_active_timer -= m_step_interval;

		is_cooking = false;
		cook_ongoing = false;
		room_available = false;

		src_item = src_list->getItem(0);
		if (src_item && src_item->isCrushable(CRUSH_CRUSHER)) {
			is_cooking = true;
			room_available = dst_list->roomForCrushedItem(src_item);
			if (room_available && src_item->getCount() > 1)
				cook_ongoing = true;
		}else{
			m_cook_timer = 0.0;
		}

		if (m_cook_timer > cook_time)
			m_cook_timer = cook_time;

		if (m_burn_counter < 1.0 && is_cooking) {
			if (m_cook_timer+m_step_interval < cook_time || cook_ongoing) {
				fuel_list = m_inventory->getList("fuel");
				if (!fuel_list)
					break;
				fuel_item = fuel_list->getItem(0);
				if (fuel_item && fuel_item->isFuel()) {
					content_t c = fuel_item->getContent();
					float v = 0.0;
					if ((c&CONTENT_CRAFTITEM_MASK) == CONTENT_CRAFTITEM_MASK) {
						v = ((CraftItem*)fuel_item)->getFuelTime();
					}else if ((c&CONTENT_TOOLITEM_MASK) == CONTENT_TOOLITEM_MASK) {
						v = ((ToolItem*)fuel_item)->getFuelTime();
					}else{
						v = ((MaterialItem*)fuel_item)->getFuelTime();
					}
					fuel_list->decrementMaterials(1);
					if (c == CONTENT_TOOLITEM_IRON_BUCKET_LAVA) {
						fuel_list->addItem(0,new ToolItem(CONTENT_TOOLITEM_IRON_BUCKET,0,0));
					}
					m_burn_counter += v*m_burn_upgrade;
					changed = true;
				}
			}
		}

		if (m_burn_counter <= 0.0) {
			m_active_timer = 0.0;
			m_burn_counter = 0.0;
			m_burn_timer = 0.0;
			break;
		}

		if (m_burn_counter < 1.0)
			continue;

		m_burn_timer += m_step_interval;
		changed = true;
		if (m_burn_timer >= cook_time) {
			m_burn_counter -= 1.0;
			m_burn_timer -= cook_time;
		}

		if (!is_cooking) {
			m_cook_timer = 0.0;
			continue;
		}

		m_cook_timer += m_step_interval;

		if (m_cook_timer >= cook_time) {
			m_cook_timer -= cook_time;
			if (src_item && src_item->isCrushable(CRUSH_CRUSHER)) {
				InventoryItem *crushresult = src_item->createCrushResult();
				dst_list->addItem(crushresult);
				src_list->decrementMaterials(1);
				if (m_is_exo && player)
					player->inventory_modified = true;
			}
		}
	}

	return changed;
}
std::string CrusherNodeMetadata::getDrawSpecString(Player *player)
{
	float cook_time;
	float v = 0;

	if (m_cook_upgrade < 1.0)
		m_cook_upgrade = 1.0;
	cook_time = 4.0/m_cook_upgrade;
	if (cook_time < 0.1)
		cook_time = 0.1;
	if (m_burn_counter > 0.0 && m_burn_timer > 0.0)
		v = ((100.0/cook_time)*m_burn_timer);

	std::string spec("size[9,10]");

	if (!m_is_exo) {
		InventoryList *l = m_inventory->getList("main");
		if (m_is_expanded && l && l->getUsedSlots() > 4) {
			char buff[10];
			snprintf(buff,10,"%u",CONTENT_CRAFTITEM_UPGRADE_STORAGE);
			if (m_expanded_slot_id == 0) {
				spec += "image[0,0;1,1;inventory:";
				spec += buff;
				spec +="]";
				spec += "list[current_name;upgrades;1,0;2,1;1,3;]";
			}else if (m_expanded_slot_id == 1) {
				spec += "list[current_name;upgrades;0,0;1,1;0,1;]";
				spec += "image[1,0;1,1;inventory:";
				spec += buff;
				spec +="]";
				spec += "list[current_name;upgrades;2,0;2,1;2,3;]";
			}else{
				spec += "list[current_name;upgrades;0,0;2,1;0,2;]";
				spec += "image[2,0;1,1;inventory:";
				spec += buff;
				spec +="]";
			}
		}else{
			spec += "list[current_name;upgrades;0,0;3,1;]";
		}
	}

	spec += "list[current_name;fuel;1.5,3.5;1,1;]";
	spec += "ring[1.5,3.5;1;#FF0000;";
	spec += itos((int)v);
	spec += "]";
	spec += "list[current_name;src;1.5,1.5;1,1;]";

	if (m_is_expanded) {
		spec += "list[current_name;main;4,1;4,4;]";
	}else if (m_is_exo) {
		spec += "list[current_player;exo;3,1;6,3;]";
	}else{
		spec += "list[current_name;main;5,1.5;2,2;]";
	}

	spec += "list[current_player;main;0.5,5.8;8,1;0,8;]";
	spec += "list[current_player;main;0.5,7;8,3;8,-1;]";

	return spec;
}
std::vector<NodeBox> CrusherNodeMetadata::getNodeBoxes(MapNode &n)
{
	std::vector<NodeBox> boxes;
	int v = 0;

	if (m_burn_counter > 0.0) {
		float cook_time;
		if (m_cook_upgrade < 1.0)
			m_cook_upgrade = 1.0;
		cook_time = 4.0/m_cook_upgrade;
		if (cook_time < 0.1)
			cook_time = 0.1;
		if (m_burn_counter > 0.0 && m_burn_timer > 0.0)
			v = ((2.0/cook_time)*m_burn_timer);
	}

	if (v) {
		boxes.push_back(NodeBox(
			-0.125*BS,-0.375*BS,-0.375*BS,-0.0625*BS,0.375*BS,0.375*BS
		));
		boxes.push_back(NodeBox(
			0.0625*BS,-0.375*BS,-0.375*BS,0.125*BS,0.375*BS,0.375*BS
		));
	}else{
		boxes.push_back(NodeBox(
			-0.375*BS,-0.375*BS,-0.375*BS,-0.3125*BS,0.375*BS,0.375*BS
		));
		boxes.push_back(NodeBox(
			0.3125*BS,-0.375*BS,-0.375*BS,0.375*BS,0.375*BS,0.375*BS
		));
	}

	return boxes;
}
std::string CrusherNodeMetadata::getOwner()
{
	if (m_is_locked || m_is_exo)
		return m_owner;
	return "";
}
std::string CrusherNodeMetadata::getInventoryOwner()
{
	if (m_is_locked)
		return m_owner;
	return "";
}
