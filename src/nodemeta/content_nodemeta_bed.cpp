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
#include "environment.h"


/*
	BedNodeMetadata
*/

// Prototype
BedNodeMetadata proto_BedNodeMetadata();

BedNodeMetadata::BedNodeMetadata()
{
	m_owner = "";
	m_nope = false;

	NodeMetadata::registerType(typeId(), create);
}
u16 BedNodeMetadata::typeId() const
{
	return CONTENT_BED_HEAD;
}
NodeMetadata* BedNodeMetadata::create(std::istream &is)
{
	BedNodeMetadata *d = new BedNodeMetadata();
	d->m_owner = deSerializeString(is);
	int temp;
	is>>temp;
	d->m_nope = !!temp;
	return d;
}
NodeMetadata* BedNodeMetadata::clone()
{
	BedNodeMetadata *d = new BedNodeMetadata();
	d->m_owner = m_owner;
	d->m_nope = m_nope;
	return d;
}
void BedNodeMetadata::serializeBody(std::ostream &os)
{
	os<<serializeString(m_owner);
	os<<itos(m_nope) << " ";
}
bool BedNodeMetadata::nodeRemovalDisabled()
{
	if (m_owner != "")
		return true;
	return false;
}
bool BedNodeMetadata::receiveFields(std::string formname, std::map<std::string, std::string> fields, Player *player)
{
	if (fields["wake"] != "") {
		m_nope = false;
		player->in_bed = false;
		m_owner = "";
		return true;
	}else if (fields["sleep"] != "") {
		if (m_owner != "")
			return false;
		if (player->wake_timeout > 0.0) {
			m_nope = true;
			return true;
		}
		m_nope = false;
		player->in_bed = true;
		m_owner = player->getName();
		return true;
	}else if (player->getName() == m_owner) { // this will happen if the player escape closes the form
		m_nope = false;
		player->in_bed = false;
		m_owner = "";
		return true;
	}else if (m_nope) {
		m_nope = false;
		return true;
	}
	return false;
}
std::string BedNodeMetadata::getDrawSpecString(Player *player)
{
	std::string spec("size[5,2.5]");
	if (m_owner == "") {
		if (m_nope) {
			spec += "label[1.25,1;";
			spec += gettext("You can't sleep yet.");;
			spec += "]";
		}else{
			spec += "button[1.25,1;3,1;sleep;";
			spec += gettext("Go to sleep");;
			spec += "]";
		}
	}else if (m_owner != player->getName()) {
		spec += "label[1.25,1;";
		spec += gettext("Someone else is sleeping here.");;
		spec += "]";
	}else if (m_nope) {
		spec += "label[1.25,1;";
		spec += gettext("You can't sleep yet.");;
		spec += "]";
	}else{
		spec += "button_exit[1.25,1;3,1;wake;";
		spec += gettext("Get out of bed");;
		spec += "]";
	}
	return spec;
}

/*
	CampBedNodeMetadata
*/

// Prototype
CampBedNodeMetadata proto_CampBedNodeMetadata();

CampBedNodeMetadata::CampBedNodeMetadata()
{
	m_owner = "";
	m_nope = false;
	m_used = false;

	NodeMetadata::registerType(typeId(), create);
}
u16 CampBedNodeMetadata::typeId() const
{
	return CONTENT_BED_CAMP_HEAD;
}
NodeMetadata* CampBedNodeMetadata::create(std::istream &is)
{
	CampBedNodeMetadata *d = new CampBedNodeMetadata();
	d->m_owner = deSerializeString(is);
	int temp;
	is>>temp;
	d->m_nope = !!temp;
	is>>temp;
	d->m_used = !!temp;
	return d;
}
NodeMetadata* CampBedNodeMetadata::clone()
{
	CampBedNodeMetadata *d = new CampBedNodeMetadata();
	d->m_owner = m_owner;
	d->m_nope = m_nope;
	d->m_used = m_used;
	return d;
}
void CampBedNodeMetadata::serializeBody(std::ostream &os)
{
	os<<serializeString(m_owner);
	os<<itos(m_nope) << " ";
	os<<itos(m_used) << " ";
}
bool CampBedNodeMetadata::step(float dtime, v3s16 pos, ServerEnvironment *env)
{
	if (!m_used)
		return false;
	m_ticks++;
	if (m_ticks < 2)
		return false;

	MapNode n = env->getMap().getNodeNoEx(pos);
	v3s16 fp = pos+n.getEffectedRotation();
	if (fp == pos)
		return false;

	// add random dead grass/fur to parcel
	{
		u16 c = myrand_range(1,4);
		InventoryItem *item = InventoryItem::create(CONTENT_DEADGRASS,c);
		env->dropToParcel(pos,item);
	}
	if (myrand_range(0,1)) {
		InventoryItem *item = InventoryItem::create(CONTENT_CRAFTITEM_FUR,1);
		env->dropToParcel(pos,item);
	}

	n.setContent(CONTENT_DEADGRASS);
	env->setPostStepNodeSwap(pos,n);

	return true;
}
bool CampBedNodeMetadata::nodeRemovalDisabled()
{
	if (m_owner != "")
		return true;
	if (m_used)
		return true;
	return false;
}
bool CampBedNodeMetadata::receiveFields(std::string formname, std::map<std::string, std::string> fields, Player *player)
{
	if (fields["wake"] != "") {
		if (player->in_bed)
			m_used = false;
		m_nope = false;
		player->in_bed = false;
		m_owner = "";
		return true;
	}else if (fields["sleep"] != "") {
		if (m_used)
			return false;
		if (m_owner != "")
			return false;
		if (player->wake_timeout > 0.0) {
			m_nope = true;
			return true;
		}
		m_nope = false;
		player->in_bed = true;
		m_used = true;
		m_ticks = 0;
		m_owner = player->getName();
		return true;
	}else if (player->getName() == m_owner) { // this will happen if the player escape closes the form
		if (player->in_bed)
			m_used = false;
		m_nope = false;
		player->in_bed = false;
		m_owner = "";
		return true;
	}else if (m_nope) {
		m_nope = false;
		return true;
	}
	return false;
}
std::string CampBedNodeMetadata::getDrawSpecString(Player *player)
{
	std::string spec("size[5,2.5]");
	if (m_owner == "") {
		if (m_used) {
			spec += "label[1.25,1;";
			spec += gettext("This bed is too uncomfortable to sleep in.");;
			spec += "]";
		}else if (m_nope) {
			spec += "label[1.25,1;";
			spec += gettext("You can't sleep yet.");;
			spec += "]";
		}else{
			spec += "button[1.25,1;3,1;sleep;";
			spec += gettext("Go to sleep");;
			spec += "]";
		}
	}else if (m_owner != player->getName()) {
		spec += "label[1.25,1;";
		spec += gettext("Someone else is sleeping here.");;
		spec += "]";
	}else if (m_nope) {
		spec += "label[1.25,1;";
		spec += gettext("You can't sleep yet.");;
		spec += "]";
	}else{
		spec += "button_exit[1.25,1;3,1;wake;";
		spec += gettext("Get out of bed");;
		spec += "]";
	}
	return spec;
}
