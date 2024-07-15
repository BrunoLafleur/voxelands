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
#include "enchantment.h"

/*
	ForgeNodeMetadata
*/

// Prototype
ForgeNodeMetadata proto_ForgeNodeMetadata;

ForgeNodeMetadata::ForgeNodeMetadata():
	m_show_craft(false)
{
	NodeMetadata::registerType(typeId(), create);

	m_inventory = new Inventory();
	m_inventory->addList("mithril", 9);
	m_inventory->addList("gem", 9);
	m_inventory->addList("craft", 9);
	m_inventory->addList("craftresult", 1);
	{
		InventoryList *l = m_inventory->getList("mithril");
		l->addAllowed(CONTENT_CRAFTITEM_MITHRIL_UNBOUND);
	}
	{
		InventoryList *l = m_inventory->getList("gem");
		l->addAllowed(CONTENT_CRAFTITEM_RUBY);
		l->addAllowed(CONTENT_CRAFTITEM_TURQUOISE);
		l->addAllowed(CONTENT_CRAFTITEM_AMETHYST);
		l->addAllowed(CONTENT_CRAFTITEM_SAPPHIRE);
		l->addAllowed(CONTENT_CRAFTITEM_SUNSTONE);
	}
}
ForgeNodeMetadata::~ForgeNodeMetadata()
{
}
u16 ForgeNodeMetadata::typeId() const
{
	return CONTENT_FORGE;
}
NodeMetadata* ForgeNodeMetadata::clone()
{
	ForgeNodeMetadata *d = new ForgeNodeMetadata();
	d->m_show_craft = m_show_craft;
	*d->m_inventory = *m_inventory;
	return d;
}
NodeMetadata* ForgeNodeMetadata::create(std::istream &is)
{
	ForgeNodeMetadata *d = new ForgeNodeMetadata();

	d->m_inventory->deSerialize(is);
	int c;
	is>>c;
	d->m_show_craft = !!c;

	return d;
}
void ForgeNodeMetadata::serializeBody(std::ostream &os)
{
	m_inventory->serialize(os);
	os<<itos(m_show_craft ? 1 : 0)<<" ";
}
std::wstring ForgeNodeMetadata::infoText()
{
	return narrow_to_wide(gettext("Forge"));
}
void ForgeNodeMetadata::inventoryModified()
{
	vlprintf(CN_INFO,"Forge inventory modification callback");
}
bool ForgeNodeMetadata::step(float dtime, v3s16 pos, ServerEnvironment *env)
{
	v3s16 abv = pos+v3s16(0,1,0);
	MapNode n = env->getMap().getNodeNoEx(abv);
	if (n.getContent() == CONTENT_AIR) {
		bool show_flame = false;
		if (m_show_craft) {
			InventoryItem *items[9];
			bool has_enchanted = false;
			InventoryList *clist = m_inventory->getList("craft");
			InventoryList *rlist = m_inventory->getList("craftresult");
			if (!clist || !rlist)
				return false;

			for (u16 i=0; i<9; i++) {
				items[i] = clist->getItem(i);
				if (
					!has_enchanted
					&& items[i]
					&& (items[i]->getContent()&CONTENT_CRAFTITEM_MASK) == CONTENT_CRAFTITEM_MASK
					&& items[i]->getData() > 0
				)
					has_enchanted = true;
			}

			if (!has_enchanted)
				return false;

			// Get result of crafting grid
			/* TODO: player/server args */
			InventoryItem *result = crafting::getResult(items,NULL,NULL);
			if (!result)
				return false;
			if (rlist->itemFits(0,result))
				show_flame = true;
			delete result;
		}else{
			InventoryList *mlist = m_inventory->getList("mithril");
			InventoryList *glist = m_inventory->getList("gem");
			if (!mlist || !glist)
				return false;
			InventoryItem *mithril = mlist->getItem(0);
			InventoryItem *gem = glist->getItem(0);
			if (mithril && gem)
				show_flame = true;
		}
		if (show_flame) {
			n.setContent(CONTENT_FORGE_FIRE);
			env->getMap().addNodeWithEvent(abv,n);
		}
	}else if (n.getContent() == CONTENT_FORGE_FIRE) {
		env->getMap().removeNodeWithEvent(abv);
		if (m_show_craft) {
			InventoryItem *items[9];
			bool has_enchanted = false;
			InventoryList *clist = m_inventory->getList("craft");
			InventoryList *rlist = m_inventory->getList("craftresult");
			if (!clist || !rlist)
				return false;

			for (u16 i=0; i<9; i++) {
				items[i] = clist->getItem(i);
				if (
					!has_enchanted
					&& items[i]
					&& (items[i]->getContent()&CONTENT_CRAFTITEM_MASK) == CONTENT_CRAFTITEM_MASK
					&& items[i]->getData() > 0
				)
					has_enchanted = true;
			}

			if (!has_enchanted)
				return false;

			// Get result of crafting grid
			/* TODO: player/server args */
			InventoryItem *result = crafting::getResult(items,NULL,NULL);
			if (!result)
				return false;

			{
				u16 data = 0;
				for (u16 i=0; i<9; i++) {
					if (
						items[i]
						&& (items[i]->getContent()&CONTENT_CRAFTITEM_MASK) == CONTENT_CRAFTITEM_MASK
						&& items[i]->getData() > 0
					)
						enchantment_set(&data,items[i]->getData());
				}
				result->setData(data);
			}
			if (!rlist->itemFits(0,result)) {
				delete result;
				return false;
			}

			rlist->addItem(result);
			clist->decrementMaterials(1);

			return true;
		}else{
			InventoryList *mlist = m_inventory->getList("mithril");
			InventoryList *glist = m_inventory->getList("gem");
			InventoryList *result = m_inventory->getList("craftresult");
			if (!mlist || !glist || !result)
				return false;
			InventoryItem *mithril = mlist->getItem(0);
			InventoryItem *gem = glist->getItem(0);
			if (!mithril || !gem)
				return false;
			u16 data = 0;
			if (!enchantment_enchant(&data,gem->getContent()))
				return false;
			InventoryItem *newitem = new CraftItem(CONTENT_CRAFTITEM_MITHRIL,1,data);
			if (!newitem)
				return false;
			if (!result->itemFits(0,newitem)) {
				delete newitem;
				return false;
			}
			result->addItem(newitem);
			mlist->decrementMaterials(1);
			glist->decrementMaterials(1);
			return true;
		}
	}
	return false;
}
bool ForgeNodeMetadata::nodeRemovalDisabled()
{
	/*
		Disable removal if not empty
	*/
	InventoryList *list = m_inventory->getList("craft");
	if (list && list->getUsedSlots() > 0)
		return true;

	list = m_inventory->getList("mithril");
	if (list && list->getUsedSlots() > 0)
		return true;

	list = m_inventory->getList("gem");
	if (list && list->getUsedSlots() > 0)
		return true;

	list = m_inventory->getList("craftresult");
	if (list && list->getUsedSlots() > 0)
		return true;

	return false;
}
bool ForgeNodeMetadata::receiveFields(std::string formname, std::map<std::string, std::string> fields, Player *player)
{
	if (fields["craft"] != "") {
		m_show_craft = true;
	}else if (fields["enchant"] != "") {
		m_show_craft = false;
	}
	return true;
}
std::string ForgeNodeMetadata::getDrawSpecString(Player *player)
{
	std::string spec("size[8,8]");
	if (m_show_craft) {
		spec += "list[current_name;craft;2,0;3,3;]";
		spec += "list[current_name;craftresult;6,1;1,1;]";
		spec += "button[3,3.2;3,1;enchant;";
		spec += gettext("Show Enchanting");;
		spec += "]";
	}else{
		spec += "list[current_name;mithril;1,1;1,1;ingot_bg.png]";
		spec += "list[current_name;gem;3,1;1,1;gem_bg.png]";
		spec += "list[current_name;craftresult;6,1;1,1;]";
		spec += "button[3,3.2;3,1;craft;";
		spec += gettext("Show Crafting");;
		spec += "]";
	}

	spec += "list[current_player;main;0,3.8;8,1;0,8;]";
	spec += "list[current_player;main;0,5;8,3;8,-1;]";

	return spec;
}
