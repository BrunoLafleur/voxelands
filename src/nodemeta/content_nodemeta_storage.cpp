/************************************************************************
* Minetest-c55
* Copyright (C) 2010-2011 celeron55, Perttu Ahola <celeron55@gmail.com>
*
* content_nodemeta.cpp
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
#include "content_nodemeta.h"
#include "inventory.h"
#include "content_mapnode.h"
#include "content_craftitem.h"
#include "content_toolitem.h"
#include "player.h"


/*
	BookShelfNodeMetadata
*/

// Prototype
BookShelfNodeMetadata proto_BookShelfNodeMetadata;

BookShelfNodeMetadata::BookShelfNodeMetadata()
{
	NodeMetadata::registerType(typeId(), create);

	m_inventory = new Inventory();
	m_inventory->addList("0", 14);
	InventoryList *l = m_inventory->getList("0");
	l->setStackable(false);
	l->addAllowed(CONTENT_BOOK);
	l->addAllowed(CONTENT_COOK_BOOK);
	l->addAllowed(CONTENT_DECRAFT_BOOK);
	l->addAllowed(CONTENT_DIARY_BOOK);
	l->addAllowed(CONTENT_CRAFT_BOOK);
	l->addAllowed(CONTENT_RCRAFT_BOOK);
}
BookShelfNodeMetadata::~BookShelfNodeMetadata()
{
	delete m_inventory;
}
u16 BookShelfNodeMetadata::typeId() const
{
	return CONTENT_BOOKSHELF;
}
NodeMetadata* BookShelfNodeMetadata::create(std::istream &is)
{
	BookShelfNodeMetadata *d = new BookShelfNodeMetadata();
	d->m_inventory->deSerialize(is);
	return d;
}
NodeMetadata* BookShelfNodeMetadata::clone()
{
	BookShelfNodeMetadata *d = new BookShelfNodeMetadata();
	*d->m_inventory = *m_inventory;
	return d;
}
void BookShelfNodeMetadata::serializeBody(std::ostream &os)
{
	m_inventory->serialize(os);
}
bool BookShelfNodeMetadata::nodeRemovalDisabled()
{
	/*
		Disable removal if chest contains something
	*/
	InventoryList *list = m_inventory->getList("0");
	if(list == NULL)
		return false;
	if(list->getUsedSlots() == 0)
		return false;
	return true;
}
std::string BookShelfNodeMetadata::getDrawSpecString(Player *player)
{
	return
		"size[8,7]"
		"list[current_name;0;0.5,0;7,2;]"
		"list[current_player;main;0,3;8,4;]";
}
std::vector<NodeBox> BookShelfNodeMetadata::getNodeBoxes(MapNode &n)
{
	std::vector<NodeBox> boxes;
	boxes.clear();

	InventoryList *list = m_inventory->getList("0");
	if(list == NULL)
		return boxes;
	if(list->getUsedSlots() == 0)
		return boxes;

	f32 x = 0;
	f32 y = 0;
	f32 h = 0;

	for (s16 i=0; i<14; i++) {
		if (list->getItem(i) == NULL)
			continue;
		x = (i%7)*0.125;
		y = (i/7)*-0.5;
		h = ((i%7)%2)*0.0625;

		boxes.push_back(NodeBox(
			(-0.4375+x)*BS,(0.0625+y)*BS,-0.4375*BS,(-0.3125+x)*BS,(0.375+y+h)*BS,-0.0625*BS
		));
		boxes.push_back(NodeBox(
			(0.3125-x)*BS,(0.0625+y)*BS,0.0625*BS,(0.4375-x)*BS,(0.375+y+h)*BS,0.4375*BS
		));
	}

	return transformNodeBox(n,boxes);
}

/*
	ChestNodeMetadata
*/

// Prototype
ChestNodeMetadata proto_ChestNodeMetadata;

ChestNodeMetadata::ChestNodeMetadata()
{
	NodeMetadata::registerType(typeId(), create);

	m_is_locked = false;
	m_is_expanded = false;
	m_is_exo = false;
	m_expanded_slot_id = 0;

	m_inventory = new Inventory();
	m_inventory->addList("upgrades", 2);
	m_inventory->addList("main", 18);
	inventoryModified();
}
ChestNodeMetadata::~ChestNodeMetadata()
{
	delete m_inventory;
}
u16 ChestNodeMetadata::typeId() const
{
	return CONTENT_CHEST;
}
NodeMetadata* ChestNodeMetadata::create(std::istream &is)
{
	std::string s;
	ChestNodeMetadata *d = new ChestNodeMetadata();

	d->setOwner(deSerializeString(is));

	s = deSerializeString(is);
	d->m_is_exo = !!mystoi(s);

	s = deSerializeString(is);
	d->m_is_expanded = !!mystoi(s);

	s = deSerializeString(is);
	d->m_is_locked = !!mystoi(s);

	s = deSerializeString(is);
	d->m_expanded_slot_id = mystoi(s);

	if (d->m_is_expanded) {
		delete d->m_inventory;
		d->m_inventory = new Inventory();
		d->m_inventory->addList("upgrades", 2);
		d->m_inventory->addList("main", 36);
	}
	d->m_inventory->deSerialize(is);
	d->inventoryModified();

	return d;
}
NodeMetadata* ChestNodeMetadata::clone()
{
	ChestNodeMetadata *d = new ChestNodeMetadata();
	d->m_is_exo = m_is_exo;
	d->m_is_expanded = m_is_expanded;
	d->m_is_locked = m_is_locked;
	d->m_expanded_slot_id = m_expanded_slot_id;
	*d->m_inventory = *m_inventory;
	return d;
}
void ChestNodeMetadata::serializeBody(std::ostream &os)
{
	os<<serializeString(m_owner);
	os<<serializeString(itos(m_is_exo ? 1 : 0));
	os<<serializeString(itos(m_is_expanded ? 1 : 0));
	os<<serializeString(itos(m_is_locked ? 1 : 0));
	os<<serializeString(itos(m_expanded_slot_id));
	m_inventory->serialize(os);
}
std::wstring ChestNodeMetadata::infoText()
{
	char buff[256];
	if (m_is_locked) {
		snprintf(buff, 256, gettext("Locked Chest owned by '%s'"), m_owner.c_str());
	}else if (m_is_exo) {
		snprintf(buff, 256, gettext("Exo Chest"));
	}else{
		snprintf(buff, 256, gettext("Chest"));
	}
	return narrow_to_wide(buff);
}
Inventory* ChestNodeMetadata::getInventory()
{
	return m_inventory;
}
void ChestNodeMetadata::inventoryModified()
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

	for (i=0; i<2; i++) {
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
			inv->addList("upgrades", 2);
			inv->addList("main", 36);
			il = inv->getList("upgrades");
			im = inv->getList("main");
			if (!il || !im) {
				delete inv;
				continue;
			}
			for (k=0; k<2; k++) {
				itm = l->changeItem(k,NULL);
				if (itm)
					il->addItem(k,itm);
			}
			for (k=0; k<18; k++) {
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

	if (m_is_expanded && !b[0]) {
		inv = new Inventory();
		inv->addList("upgrades", 2);
		inv->addList("main", 18);
		il = inv->getList("upgrades");
		im = inv->getList("main");
		if (!il || !im) {
			delete inv;
		}else{
			for (k=0; k<2; k++) {
				itm = l->changeItem(k,NULL);
				if (itm)
					il->addItem(k,itm);
			}
			for (k=0; k<36; k++) {
				itm = m->changeItem(k,NULL);
				if (itm) {
					if (k > 17) {
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

	if (m_is_expanded && f > -1 && f != m_expanded_slot_id)
		m_expanded_slot_id = f;

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
bool ChestNodeMetadata::nodeRemovalDisabled()
{
	/*
		Disable removal if chest contains something
	*/
	InventoryList *list = m_inventory->getList("main");
	if(list == NULL)
		return false;
	if(list->getUsedSlots() == 0)
		return false;
	return true;
}
std::string ChestNodeMetadata::getDrawSpecString(Player *player)
{
	std::string spec("size[9,10]");

	if (!m_is_exo) {
		InventoryList *l = m_inventory->getList("main");
		if (m_is_expanded && l && l->getUsedSlots() > 18) {
			char buff[10];
			snprintf(buff,10,"%u",CONTENT_CRAFTITEM_UPGRADE_STORAGE);
			if (m_expanded_slot_id == 0) {
				spec += "image[0,0;1,1;inventory:";
				spec += buff;
				spec +="]";
				spec += "list[current_name;upgrades;1,0;1,1;1,1;]";
			}else{
				spec += "list[current_name;upgrades;0,0;1,1;0,1;]";
				spec += "image[1,0;1,1;inventory:";
				spec += buff;
				spec +="]";
			}
		}else{
			spec += "list[current_name;upgrades;0,0;2,1;]";
		}
	}

	if (m_is_expanded) {
		spec += "list[current_name;main;0,1;9,4;]";
	}else if (m_is_exo) {
		spec += "list[current_player;exo;1.5,1.5;6,3;]";
	}else{
		spec += "list[current_name;main;1.5,1.5;6,3;]";
	}

	spec += "list[current_player;main;0.5,5.8;8,1;0,8;]";
	spec += "list[current_player;main;0.5,7;8,3;8,-1;]";

	return spec;
}
std::vector<NodeBox> ChestNodeMetadata::getNodeBoxes(MapNode &n)
{
	std::vector<NodeBox> boxes;

	if (m_is_locked) {
		boxes.push_back(NodeBox(
			aabb3f(-0.125*BS,-0.3125*BS,-0.5*BS,0.125*BS,-0.125*BS,-0.4375*BS)
		));
		boxes.push_back(NodeBox(
			aabb3f(0.0625*BS,-0.125*BS,-0.5*BS,0.09375*BS,0.0,-0.4375*BS)
		));
		boxes.push_back(NodeBox(
			aabb3f(-0.09375*BS,-0.125*BS,-0.5*BS,-0.0625*BS,0.0,-0.4375*BS)
		));
	}else if (m_is_exo) {
		boxes.push_back(NodeBox(
			v3s16(0,180,0),aabb3f(-0.125*BS,-0.3125*BS,0.4375*BS,0.125*BS,-0.1875*BS,0.5*BS)
		));
		boxes.push_back(NodeBox(
			v3s16(0,180,0),aabb3f(-0.0625*BS,-0.375*BS,0.4375*BS,0.0625*BS,-0.3125*BS,0.5*BS)
		));
		boxes.push_back(NodeBox(
			v3s16(0,180,0),aabb3f(-0.0625*BS,-0.1875*BS,0.4375*BS,0.0625*BS,-0.125*BS,0.5*BS)
		));
	}

	return boxes;
}
std::string ChestNodeMetadata::getOwner()
{
	if (m_is_locked)
		return m_owner;
	return "";
}
std::string ChestNodeMetadata::getInventoryOwner()
{
	if (m_is_locked)
		return m_owner;
	return "";
}

/*
	BarrelNodeMetadata
*/

// Prototype
BarrelNodeMetadata proto_BarrelNodeMetadata;

BarrelNodeMetadata::BarrelNodeMetadata()
{
	NodeMetadata::registerType(typeId(), create);

	m_water_level = 0;
}
BarrelNodeMetadata::~BarrelNodeMetadata()
{
}
u16 BarrelNodeMetadata::typeId() const
{
	return CONTENT_WOOD_BARREL;
}
NodeMetadata* BarrelNodeMetadata::create(std::istream &is)
{
	std::string s;
	BarrelNodeMetadata *d = new BarrelNodeMetadata();

	s = deSerializeString(is);
	d->m_water_level = mystoi(s);

	return d;
}
NodeMetadata* BarrelNodeMetadata::clone()
{
	BarrelNodeMetadata *d = new BarrelNodeMetadata();
	d->m_water_level = m_water_level;
	return d;
}
void BarrelNodeMetadata::serializeBody(std::ostream &os)
{
	os<<serializeString(itos(m_water_level));
}
std::wstring BarrelNodeMetadata::infoText()
{
	char buff[1024];
	if (!m_water_level)
		return narrow_to_wide(gettext("Barrel is empty"));

	if (m_water_level > 39)
		return narrow_to_wide(gettext("Barrel is full"));

	if (snprintf(buff,1024,gettext("Barrel is %u%% full"),(m_water_level*5)/2) < 1024)
		return narrow_to_wide(buff);

	return narrow_to_wide(gettext("Barrel"));
}
std::vector<NodeBox> BarrelNodeMetadata::getNodeBoxes(MapNode &n)
{
	std::vector<NodeBox> boxes;

	if (m_water_level) {
		float h = -0.375+(0.015625*(float)m_water_level);
		boxes.push_back(NodeBox(
			-0.3125*BS,-0.375*BS,-0.3125*BS,0.3125*BS,h*BS,0.3125*BS
		));
	}

	return boxes;
}
bool BarrelNodeMetadata::import(NodeMetadata *meta)
{
	if (meta->typeId() != CONTENT_WOOD_BARREL_SEALED)
		return false;
	SealedBarrelNodeMetadata *l = (SealedBarrelNodeMetadata*)meta;
	m_water_level = l->m_water_level;
	return true;
}

/*
	SealedBarrelNodeMetadata
*/

// Prototype
SealedBarrelNodeMetadata proto_SealedBarrelNodeMetadata;

SealedBarrelNodeMetadata::SealedBarrelNodeMetadata()
{
	NodeMetadata::registerType(typeId(), create);

	m_water_level = 0;
}
SealedBarrelNodeMetadata::~SealedBarrelNodeMetadata()
{
}
u16 SealedBarrelNodeMetadata::typeId() const
{
	return CONTENT_WOOD_BARREL_SEALED;
}
NodeMetadata* SealedBarrelNodeMetadata::create(std::istream &is)
{
	std::string s;
	SealedBarrelNodeMetadata *d = new SealedBarrelNodeMetadata();

	s = deSerializeString(is);
	d->m_water_level = mystoi(s);

	return d;
}
NodeMetadata* SealedBarrelNodeMetadata::clone()
{
	SealedBarrelNodeMetadata *d = new SealedBarrelNodeMetadata();
	d->m_water_level = m_water_level;
	return d;
}
void SealedBarrelNodeMetadata::serializeBody(std::ostream &os)
{
	os<<serializeString(itos(m_water_level));
}
std::wstring SealedBarrelNodeMetadata::infoText()
{
	char buff[1024];
	if (!m_water_level)
		return narrow_to_wide(gettext("Barrel is empty"));

	if (m_water_level > 39)
		return narrow_to_wide(gettext("Barrel is full"));

	if (snprintf(buff,1024,gettext("Barrel is %u%% full"),(m_water_level*5)/2) < 1024)
		return narrow_to_wide(buff);

	return narrow_to_wide(gettext("Barrel"));
}
bool SealedBarrelNodeMetadata::import(NodeMetadata *meta)
{
	if (meta->typeId() != CONTENT_WOOD_BARREL)
		return false;
	BarrelNodeMetadata *l = (BarrelNodeMetadata*)meta;
	m_water_level = l->m_water_level;
	return true;
}

/*
	ClayVesselNodeMetadata
*/

// Prototype
ClayVesselNodeMetadata proto_ClayVesselNodeMetadata;

ClayVesselNodeMetadata::ClayVesselNodeMetadata()
{
	NodeMetadata::registerType(typeId(), create);

	m_inventory = new Inventory();
	m_inventory->addList("main", 9);
	is_sealed = false;
	InventoryList *l = m_inventory->getList("main");
	l->addAllowed(CONTENT_CRAFTITEM_APPLE);
	l->addAllowed(CONTENT_CRAFTITEM_APPLE_BLOSSOM);
	l->addAllowed(CONTENT_CRAFTITEM_APPLE_IRON);
	l->addAllowed(CONTENT_CRAFTITEM_APPLE_JUICE);
	l->addAllowed(CONTENT_CRAFTITEM_APPLE_PIE_SLICE);
	l->addAllowed(CONTENT_CRAFTITEM_BEETROOT);
	l->addAllowed(CONTENT_CRAFTITEM_BLUEBERRY);
	l->addAllowed(CONTENT_CRAFTITEM_BREAD);
	l->addAllowed(CONTENT_CRAFTITEM_CACTUS_FRUIT);
	l->addAllowed(CONTENT_CRAFTITEM_CARROT);
	l->addAllowed(CONTENT_CRAFTITEM_CARROT_CAKE);
	l->addAllowed(CONTENT_CRAFTITEM_CARROT_CAKE_RAW);
	l->addAllowed(CONTENT_CRAFTITEM_COFFEE);
	l->addAllowed(CONTENT_CRAFTITEM_COFFEE_BEANS);
	l->addAllowed(CONTENT_CRAFTITEM_COOKED_FISH);
	l->addAllowed(CONTENT_CRAFTITEM_COOKED_MEAT);
	l->addAllowed(CONTENT_CRAFTITEM_COOKED_RAT);
	l->addAllowed(CONTENT_CRAFTITEM_DOUGH);
	l->addAllowed(CONTENT_CRAFTITEM_FISH);
	l->addAllowed(CONTENT_CRAFTITEM_FLOUR);
	l->addAllowed(CONTENT_CRAFTITEM_GLASS_BOTTLE_WATER);
	l->addAllowed(CONTENT_CRAFTITEM_GRAPE);
	l->addAllowed(CONTENT_CRAFTITEM_GRAPE_JUICE);
	l->addAllowed(CONTENT_CRAFTITEM_IRON_BOTTLE_WATER);
	l->addAllowed(CONTENT_CRAFTITEM_MEAT);
	l->addAllowed(CONTENT_CRAFTITEM_MELONSLICE);
	l->addAllowed(CONTENT_CRAFTITEM_MUSH);
	l->addAllowed(CONTENT_CRAFTITEM_POTATO);
	l->addAllowed(CONTENT_CRAFTITEM_PUMPKINSLICE);
	l->addAllowed(CONTENT_CRAFTITEM_PUMPKIN_PIE_SLICE);
	l->addAllowed(CONTENT_CRAFTITEM_RASPBERRY);
	l->addAllowed(CONTENT_CRAFTITEM_ROASTPOTATO);
	l->addAllowed(CONTENT_CRAFTITEM_TEA);
	l->addAllowed(CONTENT_CRAFTITEM_TEA_LEAVES);
	l->addAllowed(CONTENT_CRAFTITEM_WHEAT);
}
ClayVesselNodeMetadata::~ClayVesselNodeMetadata()
{
	delete m_inventory;
}
u16 ClayVesselNodeMetadata::typeId() const
{
	return CONTENT_CLAY_VESSEL;
}
NodeMetadata* ClayVesselNodeMetadata::create(std::istream &is)
{
	std::string s;
	ClayVesselNodeMetadata *d = new ClayVesselNodeMetadata();
	d->m_inventory->deSerialize(is);

	s = deSerializeString(is);
	d->is_sealed = !!mystoi(s);

	return d;
}
NodeMetadata* ClayVesselNodeMetadata::clone()
{
	ClayVesselNodeMetadata *d = new ClayVesselNodeMetadata();
	*d->m_inventory = *m_inventory;
	d->is_sealed = is_sealed;
	return d;
}
void ClayVesselNodeMetadata::serializeBody(std::ostream &os)
{
	m_inventory->serialize(os);
	os<<serializeString(itos(is_sealed ? 1 : 0));
}
bool ClayVesselNodeMetadata::nodeRemovalDisabled()
{
	/*
		Disable removal if chest contains something
	*/
	InventoryList *list = m_inventory->getList("main");
	if (list == NULL)
		return false;
	if (list->getUsedSlots() == 0)
		return false;
	return true;
}
std::string ClayVesselNodeMetadata::getDrawSpecString(Player *player)
{
	std::string spec("size[8,8]");

	if (is_sealed) {
		spec += "button[0.5,2;3,1;unseal;";
		spec += gettext("Unseal Vessel");
		spec += "]";
	}else{
		spec += "button[0.5,2;3,1;seal;";
		spec += gettext("Seal Vessel");
		spec += "]";

		spec += "list[current_name;main;3.5,0.5;3,3;]";
	}

	spec += "list[current_player;main;0,3.8;8,1;0,8;]";
	spec += "list[current_player;main;0,5;8,3;8,-1;]";

	return spec;
}

bool ClayVesselNodeMetadata::receiveFields(std::string formname, std::map<std::string, std::string> fields, Player *player)
{
	if (fields["seal"] != "") {
		if (is_sealed)
			return false;
		is_sealed = true;
		return true;
	}else if (fields["unseal"] != "") {
		if (!is_sealed)
			return false;
		is_sealed = false;
		return true;
	}
	return false;
}

std::vector<NodeBox> ClayVesselNodeMetadata::getNodeBoxes(MapNode &n)
{
	std::vector<NodeBox> boxes;

	if (is_sealed) {
		boxes.push_back(NodeBox(
			-0.34375*BS,0.1875*BS,-0.34375*BS,0.34375*BS,0.3125*BS,0.34375*BS
		));
		boxes.push_back(NodeBox(
			-0.125*BS,0.3125*BS,-0.125*BS,0.125*BS,0.375*BS,0.125*BS
		));
	}

	return boxes;
}
