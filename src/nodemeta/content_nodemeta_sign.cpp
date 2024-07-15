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
#include "content_mapnode.h"
#include "player.h"


/*
	SignNodeMetadata
*/

// Prototype
SignNodeMetadata proto_SignNodeMetadata("");

SignNodeMetadata::SignNodeMetadata(std::string text):
	m_text(text)
{
	NodeMetadata::registerType(typeId(), create);
}
u16 SignNodeMetadata::typeId() const
{
	return CONTENT_SIGN_WALL;
}
NodeMetadata* SignNodeMetadata::create(std::istream &is)
{
	std::string text = deSerializeString(is);
	return new SignNodeMetadata(text);
}
NodeMetadata* SignNodeMetadata::clone()
{
	return new SignNodeMetadata(m_text);
}
void SignNodeMetadata::serializeBody(std::ostream &os)
{
	os<<serializeString(m_text);
}
std::string SignNodeMetadata::getDrawSpecString(Player *player)
{
	std::string spec("size[5,2.5]");
	spec += "field[0.75,0;4,1.5;text;;";
	spec += m_text;
	spec += "]";
	spec += "button_exit[1.25,2;3,1;save;";
	spec += gettext("Save");
	spec += "]";
	return spec;
}
bool SignNodeMetadata::import(NodeMetadata *meta)
{
	if (meta->typeId() != CONTENT_LOCKABLE_SIGN_WALL)
		return false;
	LockingSignNodeMetadata *l = (LockingSignNodeMetadata*)meta;
	m_text = l->getText();
	return true;
}

/*
	LockingSignNodeMetadata
*/

// Prototype
LockingSignNodeMetadata proto_LockingSignNodeMetadata("");

LockingSignNodeMetadata::LockingSignNodeMetadata(std::string text):
	m_text(text)
{
	NodeMetadata::registerType(typeId(), create);
}
u16 LockingSignNodeMetadata::typeId() const
{
	return CONTENT_LOCKABLE_SIGN_WALL;
}
NodeMetadata* LockingSignNodeMetadata::create(std::istream &is)
{
	std::string text = deSerializeString(is);
	LockingSignNodeMetadata *d = new LockingSignNodeMetadata(text);
	d->setOwner(deSerializeString(is));
	return d;
}
NodeMetadata* LockingSignNodeMetadata::clone()
{
	return new LockingSignNodeMetadata(m_text);
}
void LockingSignNodeMetadata::serializeBody(std::ostream &os)
{
	os<<serializeString(m_text);
	os<<serializeString(m_owner);
}
std::wstring LockingSignNodeMetadata::infoText()
{
	return narrow_to_wide(std::string("(")+m_owner+")");
}
bool LockingSignNodeMetadata::receiveFields(std::string formname, std::map<std::string, std::string> fields, Player *player)
{
	if (getOwner() != player->getName())
		return false;
	m_text = fields["text"];
	return true;
}
std::string LockingSignNodeMetadata::getDrawSpecString(Player *player)
{
	std::string spec("size[5,2.5]");
	spec += "field[0.75,0;4,1.5;text;;";
	spec += m_text;
	spec += "]";
	spec += "button_exit[1.25,2;3,1;save;";
	spec += gettext("Save");
	spec += "]";
	return spec;
}
bool LockingSignNodeMetadata::import(NodeMetadata *meta)
{
	if (meta->typeId() != CONTENT_SIGN_WALL)
		return false;
	SignNodeMetadata *l = (SignNodeMetadata*)meta;
	m_text = l->getText();
	return true;
}
