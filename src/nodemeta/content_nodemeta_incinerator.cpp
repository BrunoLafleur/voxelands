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
#include "environment.h"

/*
	IncineratorNodeMetadata
*/

// Prototype
IncineratorNodeMetadata proto_IncineratorNodeMetadata;

IncineratorNodeMetadata::IncineratorNodeMetadata()
{
	NodeMetadata::registerType(typeId(), create);

	m_inventory = new Inventory();
	m_inventory->addList("fuel", 1);

	m_should_fire = false;
	m_step_accumulator = 0;
	m_fuel_totaltime = 0;
	m_fuel_time = 0;
}
IncineratorNodeMetadata::~IncineratorNodeMetadata()
{
	delete m_inventory;
}
u16 IncineratorNodeMetadata::typeId() const
{
	return CONTENT_INCINERATOR;
}
NodeMetadata* IncineratorNodeMetadata::clone()
{
	IncineratorNodeMetadata *d = new IncineratorNodeMetadata();
	*d->m_inventory = *m_inventory;
	d->m_fuel_totaltime = m_fuel_totaltime;
	d->m_fuel_time = m_fuel_time;
	return d;
}
NodeMetadata* IncineratorNodeMetadata::create(std::istream &is)
{
	IncineratorNodeMetadata *d = new IncineratorNodeMetadata();

	d->m_inventory->deSerialize(is);
	int temp;
	is>>temp;
	d->m_fuel_totaltime = (float)temp/10;
	is>>temp;
	d->m_fuel_time = (float)temp/10;

	return d;
}
void IncineratorNodeMetadata::serializeBody(std::ostream &os)
{
	m_inventory->serialize(os);
	os<<itos(m_fuel_totaltime*10)<<" ";
	os<<itos(m_fuel_time*10)<<" ";
}
std::wstring IncineratorNodeMetadata::infoText()
{
	if (m_fuel_time < m_fuel_totaltime)
		return narrow_to_wide(gettext("Incinerator is active"));
	InventoryList *fuel_list = m_inventory->getList("fuel");
	if (fuel_list) {
		InventoryItem *fuel_item = fuel_list->getItem(0);
		if (fuel_item && fuel_item->isFuel())
			return narrow_to_wide(gettext("Incinerator is active"));
	}
	return narrow_to_wide(gettext("Incinerator is inactive"));
}
bool IncineratorNodeMetadata::nodeRemovalDisabled()
{
	/*
		Disable removal if not empty
	*/
	InventoryList *list = m_inventory->getList("fuel");

	if (list && list->getUsedSlots() > 0)
		return true;
	return false;

}
void IncineratorNodeMetadata::inventoryModified()
{
	vlprintf(CN_INFO,"Incinerator inventory modification callback");
}
bool IncineratorNodeMetadata::step(float dtime, v3s16 pos, ServerEnvironment *env)
{
	MapNode n = env->getMap().getNodeNoEx(pos);
	if (n.getContent() == CONTENT_INCINERATOR_ACTIVE) {
		n.param2 = n.param1;
		n.setContent(CONTENT_INCINERATOR);
		env->setPostStepNodeSwap(pos,n);
	}
	if (dtime > 60.0)
		vlprintf(CN_INFO,"Incinerator stepping a long time (%f)",dtime);
	// Update at a fixed frequency
	const float interval = 2.0;
	m_step_accumulator += dtime;
	bool changed = false;
	while(m_step_accumulator > interval) {
		m_step_accumulator -= interval;
		dtime = interval;

		/*
			If fuel is burning, increment the burn counters.
		*/
		if (m_fuel_time < m_fuel_totaltime) {
			m_fuel_time += dtime;
			changed = true;

			// If the fuel was not used up this step, just keep burning it
			if (m_fuel_time < m_fuel_totaltime)
				continue;
		}

		/*
			If the furnace is still cooking, stop loop.
		*/
		if (m_fuel_time < m_fuel_totaltime) {
			m_step_accumulator = 0;
			break;
		}

		if (!m_should_fire) {
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
			m_should_fire = false;
			changed = true;
		}else{
			m_step_accumulator = 0;
		}
	}
	return changed;
}
std::string IncineratorNodeMetadata::getDrawSpecString(Player *player)
{
	std::string spec("size[8,7]");
	spec += "label[1,0.5;";
	spec += gettext("Add fuel, then punch to incinerate wielded item");;
	spec += "]";
	spec += "label[3,1.5;";
	spec += gettext("Fuel");;
	spec += "]";
	spec += "list[current_name;fuel;4,1;1,1;]";
	spec += "ring[4,1;1;#FF0000;";
	float v = 0;
	if (m_fuel_totaltime > 0.0)
		v = 100.0-((100.0/m_fuel_totaltime)*m_fuel_time);
	spec += itos((int)v);
	spec += "]";
	spec += "list[current_player;main;0,3;8,4;]";
	return spec;
}
std::vector<NodeBox> IncineratorNodeMetadata::getNodeBoxes(MapNode &n)
{
	std::vector<NodeBox> boxes;
	boxes.clear();
	InventoryList *list = m_inventory->getList("fuel");
	InventoryItem *fitem;

	if (
		(
			m_fuel_time < m_fuel_totaltime
		) || (
			list
			&& list->getUsedSlots() > 0
			&& (fitem = list->getItem(0)) != NULL
			&& fitem->isFuel()
		)
	) {
		boxes.push_back(NodeBox(
			-0.3125*BS,-0.25*BS,-0.4*BS,0.3125*BS,0.125*BS,-0.3*BS
		));
	}

	return transformNodeBox(n,boxes);
}
