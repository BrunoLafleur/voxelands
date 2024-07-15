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
	LockedDoorNodeMetadata
*/

// Prototype
TNTNodeMetadata proto_TNTNodeMetadata;

TNTNodeMetadata::TNTNodeMetadata():
	m_armed(false),
	m_time(0)
{
	NodeMetadata::registerType(typeId(), create);
}
TNTNodeMetadata::~TNTNodeMetadata()
{
}
u16 TNTNodeMetadata::typeId() const
{
	return CONTENT_TNT;
}
NodeMetadata* TNTNodeMetadata::create(std::istream &is)
{
	TNTNodeMetadata *d = new TNTNodeMetadata();
	int temp;
	is>>temp;
	d->m_time = (float)temp/10;
	is>>temp;
	d->m_armed = (bool)temp;
	return d;
}
NodeMetadata* TNTNodeMetadata::clone()
{
	TNTNodeMetadata *d = new TNTNodeMetadata();
	return d;
}
bool TNTNodeMetadata::step(float dtime, v3s16 pos, ServerEnvironment *env)
{
	if (!m_armed)
		return false;
	m_time -= dtime;
	if (m_time < 0.0)
		m_time = 0.0;
	return true;
}
bool TNTNodeMetadata::energise(u8 level, v3s16 powersrc, v3s16 signalsrc, v3s16 pos)
{
	m_armed = true;
	m_time = 5.0;
	return true;
}
u8 TNTNodeMetadata::getEnergy()
{
	if (!m_armed)
		return 0;
	return ENERGY_MAX-((u8)m_time);
}
void TNTNodeMetadata::serializeBody(std::ostream &os)
{
	os<<itos(m_time*10) << " ";
	os<<itos(m_armed) << " ";
}
std::wstring TNTNodeMetadata::infoText()
{
	char buff[256];
	if (!m_armed)
		return L"";

	int s = (int)ceil(m_time);
	if (s < 1)
		return narrow_to_wide(gettext("Armed Explosive: about to detonate"));

	snprintf(buff, 256, ngettext("Armed Explosive: %d second till detonation","Armed Explosive: %d seconds till detonation",s),s);
	return narrow_to_wide(buff);
}
