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
#include "list.h"
#include "content_nodemeta.h"
#include "inventory.h"
#include "content_mapnode.h"
#include "content_list.h"
#include "environment.h"

/*
	ParcelNodeMetadata
*/

// Prototype
ParcelNodeMetadata proto_ParcelNodeMetadata;

ParcelNodeMetadata::ParcelNodeMetadata()
{
	NodeMetadata::registerType(typeId(), create);

	m_inventory = new Inventory();
	m_inventory->addList("0", 8*4);
}
ParcelNodeMetadata::~ParcelNodeMetadata()
{
	delete m_inventory;
}
u16 ParcelNodeMetadata::typeId() const
{
	return CONTENT_PARCEL;
}
NodeMetadata* ParcelNodeMetadata::create(std::istream &is)
{
	ParcelNodeMetadata *d = new ParcelNodeMetadata();
	d->m_inventory->deSerialize(is);
	return d;
}
NodeMetadata* ParcelNodeMetadata::clone()
{
	ParcelNodeMetadata *d = new ParcelNodeMetadata();
	*d->m_inventory = *m_inventory;
	return d;
}
void ParcelNodeMetadata::serializeBody(std::ostream &os)
{
	m_inventory->serialize(os);
}
std::string ParcelNodeMetadata::getDrawSpecString(Player *player)
{
	return
		"size[8,4]"
		"list[current_name;0;0,0;8,4;]";
}

/*
	CreativeChestNodeMetadata
*/

// Prototype
CreativeChestNodeMetadata proto_CreativeChestNodeMetadata;

CreativeChestNodeMetadata::CreativeChestNodeMetadata()
{
	NodeMetadata::registerType(typeId(), create);

	m_page = 0;

	m_inventory = new Inventory();
	m_inventory->addList("0", 8*4);
}
CreativeChestNodeMetadata::~CreativeChestNodeMetadata()
{
	delete m_inventory;
}
u16 CreativeChestNodeMetadata::typeId() const
{
	return CONTENT_CREATIVE_CHEST;
}
NodeMetadata* CreativeChestNodeMetadata::create(std::istream &is)
{
	CreativeChestNodeMetadata *d = new CreativeChestNodeMetadata();
	d->m_inventory->deSerialize(is);
	is>>d->m_page;
	return d;
}
NodeMetadata* CreativeChestNodeMetadata::clone()
{
	contentlist_t *cl;
	listdata_t *ld;
	CreativeChestNodeMetadata *d = new CreativeChestNodeMetadata();

	*d->m_inventory = *m_inventory;
	InventoryList *l = d->m_inventory->getList("0");
	InventoryItem *t;
	l->clearItems();

	cl = content_list_get("creative");
	if (!cl)
		return d;

	ld = cl->data;
	while (ld) {
		t = InventoryItem::create(ld->content,ld->count,0,ld->data);
		l->addItem(t);
		ld = ld->next;
	}

	return d;
}
void CreativeChestNodeMetadata::serializeBody(std::ostream &os)
{
	m_inventory->serialize(os);
	os<<itos(m_page) << " ";
}
std::wstring CreativeChestNodeMetadata::infoText()
{
	return narrow_to_wide(gettext("Creative Chest"));
}
bool CreativeChestNodeMetadata::nodeRemovalDisabled()
{
	return false;
}
bool CreativeChestNodeMetadata::receiveFields(std::string formname, std::map<std::string, std::string> fields, Player *player)
{
	contentlist_t *cl;
	listdata_t *ld;
	uint32_t list_size = 0;
	uint32_t start;
	uint32_t end;
	uint32_t i;

	if (fields["prev"] == "" && fields["next"] == "")
		return false;

	cl = content_list_get("creative");
	if (!cl)
		return false;

	list_size = list_count(&cl->data);

	if (fields["prev"] != "") {
		if (m_page > 0) {
			m_page--;
		}else{
			m_page = list_size/32;
		}
	}
	if (fields["next"] != "")
		m_page++;
	if (m_page < 0)
		m_page = 0;
	if (m_page > (list_size/32))
		m_page = 0;
	InventoryList *l = m_inventory->getList("0");
	InventoryItem *t;
	l->clearItems();

	start = m_page*32;
	end = start+32;

	if (end > list_size)
		end = list_size;

	ld = cl->data;
	for (i=0; ld && i<end; i++) {
		if (i >= start) {
			t = InventoryItem::create(ld->content,ld->count,0,ld->data);
			l->addItem(t);
		}
		ld = ld->next;
	}

	return true;
}
std::string CreativeChestNodeMetadata::getDrawSpecString(Player *player)
{
	char buff[256];
	contentlist_t *cl;
	uint32_t list_size = 0;

	cl = content_list_get("creative");
	if (cl)
		list_size = list_count(&cl->data);

	snprintf(buff,256,gettext("Page %d of %d"),(int)(m_page+1),(int)((list_size/32)+1));
	std::string spec("size[8,10]");
		spec += "list[current_name;0;0,0.5;8,4;]";
		spec += "button[0.25,5;2.5,0.75;prev;";
		spec += gettext("<< Previous Page");;
		spec += "]";
		spec += "label[3.5,5;";
		spec += buff;
		spec += "]";
		spec += "button[6,5;2.5,0.75;next;";
		spec += gettext("Next Page >>");;
		spec += "]";
		spec += "list[current_player;main;0,6;8,4;]";

	return spec;
}
