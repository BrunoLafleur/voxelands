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

/*
	BorderStoneNodeMetadata
*/

// Prototype
BorderStoneNodeMetadata proto_BorderStoneNodeMetadata;

BorderStoneNodeMetadata::BorderStoneNodeMetadata()
{
	NodeMetadata::registerType(typeId(), create);
}
BorderStoneNodeMetadata::~BorderStoneNodeMetadata()
{
}
u16 BorderStoneNodeMetadata::typeId() const
{
	return CONTENT_BORDERSTONE;
}
NodeMetadata* BorderStoneNodeMetadata::create(std::istream &is)
{
	BorderStoneNodeMetadata *d = new BorderStoneNodeMetadata();
	d->setOwner(deSerializeString(is));
	return d;
}
NodeMetadata* BorderStoneNodeMetadata::clone()
{
	BorderStoneNodeMetadata *d = new BorderStoneNodeMetadata();
	return d;
}
void BorderStoneNodeMetadata::serializeBody(std::ostream &os)
{
	os<<serializeString(m_text);
}
std::wstring BorderStoneNodeMetadata::infoText()
{
	char buff[256];
	snprintf(buff, 256, gettext("Border Stone owned by '%s'"), m_text.c_str());
	return narrow_to_wide(buff);
}
