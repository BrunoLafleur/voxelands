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
#include "environment.h"

/*
	ClockNodeMetadata
*/

// Prototype
ClockNodeMetadata proto_ClockNodeMetadata;

ClockNodeMetadata::ClockNodeMetadata():
	m_time(0)
{
	NodeMetadata::registerType(typeId(), create);
}
u16 ClockNodeMetadata::typeId() const
{
	return CONTENT_CLOCK;
}
NodeMetadata* ClockNodeMetadata::create(std::istream &is)
{
	ClockNodeMetadata *d = new ClockNodeMetadata();
	int temp;
	is>>temp;
	d->m_time = temp;
	return d;
}
NodeMetadata* ClockNodeMetadata::clone()
{
	ClockNodeMetadata *d = new ClockNodeMetadata();
	d->m_time = m_time;
	return d;
}
void ClockNodeMetadata::serializeBody(std::ostream &os)
{
	os<<itos(m_time) << " ";
}
std::vector<NodeBox> ClockNodeMetadata::getNodeBoxes(MapNode &n) {
	std::vector<NodeBox> boxes;
	boxes.clear();

	u16 h = m_time/100;
	u16 m = (u16)((float)(m_time%100)/1.6667);

	u16 v[4];
	v[0] = h/10;
	v[1] = h%10;
	v[2] = m/10;
	v[3] = m%10;

	f32 x[4] = {-0.125,0.0625,0.3125,0.5};

	u8 b[10] = {0xFC,0x0C,0xB6,0x9E,0x4E,0xDA,0xFA,0x8C,0xFE,0xDE};

	for (int i=0; i<4; i++) {
		if ((b[v[i]]&0x80))
			boxes.push_back(NodeBox((-0.25+x[i])*BS,0.0625*BS,-0.125*BS,(-0.0625+x[i])*BS,0.125*BS,-0.0625*BS));
		if ((b[v[i]]&0x04))
			boxes.push_back(NodeBox((-0.125+x[i])*BS,-0.0625*BS,-0.125*BS,(-0.0625+x[i])*BS,0.0625*BS,-0.0625*BS));
		if ((b[v[i]]&0x08))
			boxes.push_back(NodeBox((-0.125+x[i])*BS,-0.25*BS,-0.125*BS,(-0.0625+x[i])*BS,-0.125*BS,-0.0625*BS));
		if ((b[v[i]]&0x10))
			boxes.push_back(NodeBox((-0.25+x[i])*BS,-0.3125*BS,-0.125*BS,(-0.0625+x[i])*BS,-0.25*BS,-0.0625*BS));
		if ((b[v[i]]&0x20))
			boxes.push_back(NodeBox((-0.25+x[i])*BS,-0.25*BS,-0.125*BS,(-0.1875+x[i])*BS,-0.125*BS,-0.0625*BS));
		if ((b[v[i]]&0x40))
			boxes.push_back(NodeBox((-0.25+x[i])*BS,-0.0625*BS,-0.125*BS,(-0.1875+x[i])*BS,0.0625*BS,-0.0625*BS));
		if ((b[v[i]]&0x02))
			boxes.push_back(NodeBox((-0.1875+x[i])*BS,-0.125*BS,-0.125*BS,(-0.125+x[i])*BS,-0.0625*BS,-0.0625*BS));
	}

	return transformNodeBox(n,boxes);
}
bool ClockNodeMetadata::step(float dtime, v3s16 pos, ServerEnvironment *env)
{
	u32 t = env->getTimeOfDay();
	t /= 10;
	if (t == m_time)
		return false;
	m_time = t;
	return true;
}

/*
	CauldronNodeMetadata
*/

// Prototype
CauldronNodeMetadata proto_CauldronNodeMetadata;

CauldronNodeMetadata::CauldronNodeMetadata():
	m_water_level(0),
	m_water_heated(false),
	m_water_hot(false),
	m_fuel_time(0.0),
	m_src_time(0.0),
	m_cool_time(0.0)
{
	NodeMetadata::registerType(typeId(), create);

	m_inventory = new Inventory();
	m_inventory->addList("fuel", 1);
}
CauldronNodeMetadata::~CauldronNodeMetadata()
{
	delete m_inventory;
}
u16 CauldronNodeMetadata::typeId() const
{
	return CONTENT_CAULDRON;
}
NodeMetadata* CauldronNodeMetadata::clone()
{
	CauldronNodeMetadata *d = new CauldronNodeMetadata();
	d->m_fuel_time = m_fuel_time;
	d->m_src_time = m_src_time;
	d->m_water_level = m_water_level;
	d->m_water_heated = m_water_heated;
	d->m_water_hot = m_water_hot;
	*d->m_inventory = *m_inventory;
	return d;
}
NodeMetadata* CauldronNodeMetadata::create(std::istream &is)
{
	CauldronNodeMetadata *d = new CauldronNodeMetadata();

	d->m_inventory->deSerialize(is);
	int temp;
	is>>temp;
	d->m_fuel_time = (float)temp/10;
	is>>temp;
	d->m_src_time = (float)temp/10;
	is>>temp;
	d->m_cool_time = (float)temp/10;
	is>>temp;
	d->m_water_level = temp;
	is>>temp;
	d->m_water_heated = !!temp;
	is>>temp;
	d->m_water_hot = !!temp;

	return d;
}
void CauldronNodeMetadata::serializeBody(std::ostream &os)
{
	m_inventory->serialize(os);
	os<<itos(m_fuel_time*10)<<" ";
	os<<itos(m_src_time*10)<<" ";
	os<<itos(m_cool_time*10)<<" ";
	os<<itos(m_water_level)<<" ";
	os<<itos(m_water_heated ? 1 : 0)<<" ";
	os<<itos(m_water_hot ? 1 : 0)<<" ";
}
std::wstring CauldronNodeMetadata::infoText()
{
	if (m_fuel_time)
		return narrow_to_wide(gettext("Cauldron is active"));
	if (m_water_level) {
		if (m_water_hot)
			return narrow_to_wide(gettext("Cauldron is hot"));
		if (m_water_heated)
			return narrow_to_wide(gettext("Cauldron is cool"));
	}else{
		return narrow_to_wide(gettext("Cauldron is empty"));
	}
	InventoryList *list = m_inventory->getList("fuel");
	if (list && list->getUsedSlots() > 0)
		return narrow_to_wide(gettext("Cauldron is inactive"));
	return narrow_to_wide(gettext("Cauldron is out of fuel"));
}
bool CauldronNodeMetadata::nodeRemovalDisabled()
{
	/*
		Disable removal if not empty
	*/
	InventoryList *list = m_inventory->getList("fuel");

	if (list && list->getUsedSlots() > 0)
		return true;
	if (m_water_level)
		return true;
	return false;

}
void CauldronNodeMetadata::inventoryModified()
{
	vlprintf(CN_INFO,"Cauldron inventory modification callback");
}
bool CauldronNodeMetadata::step(float dtime, v3s16 pos, ServerEnvironment *env)
{
	if (m_fuel_time > 0.0) {
		if (!m_water_heated)
			m_src_time += dtime;
		m_fuel_time -= dtime;
	}

	InventoryList *list = m_inventory->getList("fuel");
	bool should_heat = false;

	if (m_water_level) {
		if (m_water_hot) {
			m_cool_time -= dtime;
			if (m_cool_time < 20.0)
				m_water_hot = false;
		}else if (!m_water_heated) {
			m_cool_time = 120.0;
			if (m_src_time < 2.0) {
				should_heat = true;
			}else{
				m_water_hot = true;
				m_water_heated = true;
				m_src_time = 0.0;
			}
		}else if (list && list->getUsedSlots() > 0) {
			m_cool_time -= dtime;
			if (m_cool_time < 0.0)
				m_water_heated = false;
		}
	}else{
		m_water_hot = false;
		m_water_heated = false;
	}

	if (should_heat && m_fuel_time <= 0.0) {
		InventoryList *list = m_inventory->getList("fuel");
		InventoryItem *fitem;
		if (list && list->getUsedSlots() > 0 && (fitem = list->getItem(0)) != NULL && fitem->isFuel()) {
			if ((fitem->getContent()&CONTENT_CRAFTITEM_MASK) == CONTENT_CRAFTITEM_MASK) {
				m_fuel_time = ((CraftItem*)fitem)->getFuelTime();
			}else if ((fitem->getContent()&CONTENT_TOOLITEM_MASK) == CONTENT_TOOLITEM_MASK) {
				m_fuel_time = ((ToolItem*)fitem)->getFuelTime();
			}else{
				m_fuel_time = ((MaterialItem*)fitem)->getFuelTime();
			}
			content_t c = fitem->getContent();
			list->decrementMaterials(1);
			if (c == CONTENT_TOOLITEM_IRON_BUCKET_LAVA)
				list->addItem(0,new ToolItem(CONTENT_TOOLITEM_IRON_BUCKET,0,0));
			return true;
		}
	}
	return false;
}
std::string CauldronNodeMetadata::getDrawSpecString(Player *player)
{
	std::string spec("size[8,7]");

	spec += "label[1,0.5;";
	spec += gettext("Add fuel, then punch to add or remove water");;
	spec += "]";
	spec += "label[3.5,1.5;";
	spec += gettext("Fuel");;
	spec += "]";
	spec += "list[current_name;fuel;4,1;1,1;]";
	spec += "list[current_player;main;0,3;8,4;]";

	return spec;
}
std::vector<NodeBox> CauldronNodeMetadata::getNodeBoxes(MapNode &n) {
	std::vector<NodeBox> boxes;
	boxes.clear();

	if (m_fuel_time)
		boxes.push_back(NodeBox(-0.125*BS,-0.5*BS,-0.125*BS,0.125*BS,-0.25*BS,0.125*BS));

	if (m_water_level) {
		switch (m_water_level) {
		case 1:
			boxes.push_back(NodeBox(-0.375*BS,-0.0625*BS,-0.375*BS,0.375*BS,0.0625*BS,0.375*BS));
			break;
		case 2:
			boxes.push_back(NodeBox(-0.375*BS,-0.0625*BS,-0.375*BS,0.375*BS,0.1875*BS,0.375*BS));
			break;
		case 3:
			boxes.push_back(NodeBox(-0.375*BS,-0.0625*BS,-0.375*BS,0.375*BS,0.3125*BS,0.375*BS));
			break;
		default:
			boxes.push_back(NodeBox(-0.375*BS,-0.0625*BS,-0.375*BS,0.375*BS,0.4375*BS,0.375*BS));
			break;
		}
	}

	return transformNodeBox(n,boxes);
}

/*
	BushNodeMetadata
*/

// Prototype
BushNodeMetadata proto_BushNodeMetadata;

BushNodeMetadata::BushNodeMetadata()
{
	NodeMetadata::registerType(typeId(), create);

	m_inventory = new Inventory();
	m_inventory->addList("main", 1);

	m_days_since_growth = -1;
}
BushNodeMetadata::~BushNodeMetadata()
{
}
u16 BushNodeMetadata::typeId() const
{
	return CONTENT_BUSH_RASPBERRY;
}
NodeMetadata* BushNodeMetadata::clone()
{
	BushNodeMetadata *d = new BushNodeMetadata();
	*d->m_inventory = *m_inventory;
	d->m_days_since_growth = m_days_since_growth;
	return d;
}
NodeMetadata* BushNodeMetadata::create(std::istream &is)
{
	BushNodeMetadata *d = new BushNodeMetadata();

	d->m_inventory->deSerialize(is);

	return d;
}
void BushNodeMetadata::serializeBody(std::ostream &os)
{
	m_inventory->serialize(os);
}
void BushNodeMetadata::inventoryModified()
{
	vlprintf(CN_INFO,"Bush inventory modification callback");
}
bool BushNodeMetadata::step(float dtime, v3s16 pos, ServerEnvironment *env)
{
	if (berryCount() > 8)
		return false;
	if (env->getSeason() != ENV_SEASON_SPRING)
		return false;
	u32 day = env->getTime();
	if (m_days_since_growth < 0) {
		m_days_since_growth = day;
		return true;
	}
	if (day-m_days_since_growth < 10) {
		return true;
	}

	InventoryList *list = m_inventory->getList("main");
	if (!list)
		return true;

	InventoryItem *item;

	MapNode n = env->getMap().getNodeNoEx(pos);
	content_t berry = content_features(n.getContent()).special_alternate_node;
	if (berry == CONTENT_IGNORE)
		return false;

	item = new CraftItem(berry,2,0);

	item = list->addItem(item);
	if (item)
		delete item;

	m_days_since_growth = day;

	return true;
}
u16 BushNodeMetadata::berryCount()
{
	InventoryList *list = m_inventory->getList("main");
	if (!list)
		return 0;

	InventoryItem *item = list->getItem(0);
	if (!item)
		return 0;

	return item->getCount();
}
