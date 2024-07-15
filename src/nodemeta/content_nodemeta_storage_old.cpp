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
#include "player.h"


/*
	DeprecatedChestNodeMetadata
*/

// Prototype
DeprecatedChestNodeMetadata proto_DeprecatedChestNodeMetadata;

DeprecatedChestNodeMetadata::DeprecatedChestNodeMetadata()
{
	NodeMetadata::registerType(typeId(), create);

	m_inventory = new Inventory();
	m_inventory->addList("0", 8*4);
}
DeprecatedChestNodeMetadata::~DeprecatedChestNodeMetadata()
{
	delete m_inventory;
}
u16 DeprecatedChestNodeMetadata::typeId() const
{
	return CONTENT_CHEST_DEPRECATED;
}
NodeMetadata* DeprecatedChestNodeMetadata::create(std::istream &is)
{
	DeprecatedChestNodeMetadata *d = new DeprecatedChestNodeMetadata();
	d->m_inventory->deSerialize(is);
	return d;
}
NodeMetadata* DeprecatedChestNodeMetadata::clone()
{
	DeprecatedChestNodeMetadata *d = new DeprecatedChestNodeMetadata();
	*d->m_inventory = *m_inventory;
	return d;
}
void DeprecatedChestNodeMetadata::serializeBody(std::ostream &os)
{
	m_inventory->serialize(os);
}
std::wstring DeprecatedChestNodeMetadata::infoText()
{
	return narrow_to_wide(gettext("Chest"));
}
bool DeprecatedChestNodeMetadata::nodeRemovalDisabled()
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
std::string DeprecatedChestNodeMetadata::getDrawSpecString(Player *player)
{
	return
		"size[8,9]"
		"list[current_name;0;0,0;8,4;]"
		"list[current_player;main;0,5;8,4;]";
}
bool DeprecatedChestNodeMetadata::import(NodeMetadata *meta)
{
	if (meta->typeId() != CONTENT_LOCKABLE_CHEST_DEPRECATED)
		return false;
	LockingDeprecatedChestNodeMetadata *l = (LockingDeprecatedChestNodeMetadata*)meta;
	*m_inventory = *l->getInventory();
	return true;
}

/*
	LockingDeprecatedChestNodeMetadata
*/

// Prototype
LockingDeprecatedChestNodeMetadata proto_LockingDeprecatedChestNodeMetadata;

LockingDeprecatedChestNodeMetadata::LockingDeprecatedChestNodeMetadata()
{
	NodeMetadata::registerType(typeId(), create);

	m_inventory = new Inventory();
	m_inventory->addList("0", 8*4);
}
LockingDeprecatedChestNodeMetadata::~LockingDeprecatedChestNodeMetadata()
{
	delete m_inventory;
}
u16 LockingDeprecatedChestNodeMetadata::typeId() const
{
	return CONTENT_LOCKABLE_CHEST_DEPRECATED;
}
NodeMetadata* LockingDeprecatedChestNodeMetadata::create(std::istream &is)
{
	LockingDeprecatedChestNodeMetadata *d = new LockingDeprecatedChestNodeMetadata();
	d->setOwner(deSerializeString(is));
	d->m_inventory->deSerialize(is);
	return d;
}
NodeMetadata* LockingDeprecatedChestNodeMetadata::clone()
{
	LockingDeprecatedChestNodeMetadata *d = new LockingDeprecatedChestNodeMetadata();
	*d->m_inventory = *m_inventory;
	return d;
}
void LockingDeprecatedChestNodeMetadata::serializeBody(std::ostream &os)
{
	os<<serializeString(m_owner);
	m_inventory->serialize(os);
}
std::wstring LockingDeprecatedChestNodeMetadata::infoText()
{
	char buff[256];
	snprintf(buff, 256, gettext("Locking Chest owned by '%s'"), m_owner.c_str());
	return narrow_to_wide(buff);
}
bool LockingDeprecatedChestNodeMetadata::nodeRemovalDisabled()
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
std::string LockingDeprecatedChestNodeMetadata::getDrawSpecString(Player *player)
{
	return
		"size[8,9]"
		"list[current_name;0;0,0;8,4;]"
		"list[current_player;main;0,5;8,4;]";
}
bool LockingDeprecatedChestNodeMetadata::import(NodeMetadata *meta)
{
	if (meta->typeId() != CONTENT_CHEST_DEPRECATED)
		return false;
	DeprecatedChestNodeMetadata *l = (DeprecatedChestNodeMetadata*)meta;
	*m_inventory = *l->getInventory();
	return true;
}

/*
	SafeNodeMetadata
*/

// Prototype
SafeNodeMetadata proto_SafeNodeMetadata;

SafeNodeMetadata::SafeNodeMetadata()
{
	NodeMetadata::registerType(typeId(), create);

	m_inventory = new Inventory();
	m_inventory->addList("0", 8*4);
}
SafeNodeMetadata::~SafeNodeMetadata()
{
	delete m_inventory;
}
u16 SafeNodeMetadata::typeId() const
{
	return CONTENT_SAFE;
}
NodeMetadata* SafeNodeMetadata::create(std::istream &is)
{
	SafeNodeMetadata *d = new SafeNodeMetadata();
	d->setOwner(deSerializeString(is));
	d->m_inventory->deSerialize(is);
	return d;
}
NodeMetadata* SafeNodeMetadata::clone()
{
	SafeNodeMetadata *d = new SafeNodeMetadata();
	*d->m_inventory = *m_inventory;
	return d;
}
void SafeNodeMetadata::serializeBody(std::ostream &os)
{
	os<<serializeString(m_owner);
	m_inventory->serialize(os);
}
std::wstring SafeNodeMetadata::infoText()
{
	char buff[256];
	snprintf(buff, 256, gettext("Safe owned by '%s'"), m_owner.c_str());
	return narrow_to_wide(buff);
}
bool SafeNodeMetadata::nodeRemovalDisabled()
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
std::string SafeNodeMetadata::getDrawSpecString(Player *player)
{
	return
		"size[8,9]"
		"list[current_name;0;0,0;8,4;]"
		"list[current_player;main;0,5;8,4;]";
}
bool SafeNodeMetadata::import(NodeMetadata *meta)
{
	if (meta->typeId() != CONTENT_CHEST_DEPRECATED)
		return false;
	DeprecatedChestNodeMetadata *l = (DeprecatedChestNodeMetadata*)meta;
	*m_inventory = *l->getInventory();
	return true;
}
