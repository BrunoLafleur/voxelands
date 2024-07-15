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
	CircuitNodeMetadata
*/

// Prototype
CircuitNodeMetadata proto_CircuitNodeMetadata;

CircuitNodeMetadata::CircuitNodeMetadata():
	m_energy(0),
	m_ptime(0)
{
	m_sources.clear();
	NodeMetadata::registerType(typeId(), create);
}
CircuitNodeMetadata::~CircuitNodeMetadata()
{
}
u16 CircuitNodeMetadata::typeId() const
{
	return CONTENT_CIRCUIT_MITHRILWIRE;
}
NodeMetadata* CircuitNodeMetadata::create(std::istream &is)
{
	CircuitNodeMetadata *d = new CircuitNodeMetadata();
	int temp;
	is>>temp;
	d->m_energy = temp;
	is>>temp;
	d->m_ptime = (float)temp/10;
	int i;
	is>>i;
	v3s16 p;
	for (; i > 0; i--) {
		is>>temp;
		p.X = temp;
		is>>temp;
		p.Y = temp;
		is>>temp;
		p.Z = temp;
		is>>temp;
		d->m_sources[p] = temp;
	}
	return d;
}
NodeMetadata* CircuitNodeMetadata::clone()
{
	CircuitNodeMetadata *d = new CircuitNodeMetadata();
	d->m_energy = m_energy;
	d->m_ptime = m_ptime;
	d->m_sources = m_sources;
	return d;
}
void CircuitNodeMetadata::serializeBody(std::ostream &os)
{
	os<<itos(m_energy) << " ";
	os<<itos(m_ptime*10)<<" ";
	os<<itos(m_sources.size()) << " ";
	for (std::map<v3s16,u8>::iterator i = m_sources.begin(); i != m_sources.end(); i++) {
		os<<itos(i->first.X) << " ";
		os<<itos(i->first.Y) << " ";
		os<<itos(i->first.Z) << " ";
		os<<itos(i->second) << " ";
	}
}
bool CircuitNodeMetadata::stepCircuit(float dtime, v3s16 pos, ServerEnvironment *env)
{
	m_ptime += dtime;
	if (!m_energy || m_ptime < 0.6)
		return false;
	m_energy = 0;
	MapNode n = env->getMap().getNodeNoEx(pos);
	if (content_features(n).unpowered_node != CONTENT_IGNORE) {
		n.setContent(content_features(n).unpowered_node);
		env->getMap().addNodeWithEvent(pos, n);
	}
	return true;
}
bool CircuitNodeMetadata::energise(u8 level, v3s16 powersrc, v3s16 signalsrc, v3s16 pos)
{
	m_ptime = 0;
	if (!m_energy)
		m_sources.clear();
	if (m_sources[powersrc] == level)
		return true;
	if (level && m_sources[powersrc] > level)
		return false;
	m_sources[powersrc] = level;
	if (!level || m_energy < level) {
		m_energy = level;
		if (!level) {
			m_sources.erase(powersrc);
			for (std::map<v3s16,u8>::iterator i = m_sources.begin(); i != m_sources.end(); i++) {
				u8 v = i->second;
				if (v > m_energy)
					m_energy = v;
			}
		}
	}
	return true;
}

/*
	SwitchNodeMetadata
*/

// Prototype
SwitchNodeMetadata proto_SwitchNodeMetadata;

SwitchNodeMetadata::SwitchNodeMetadata()
{
	m_energy = 0;
	m_ptime = 0;
	m_sources.clear();
	NodeMetadata::registerType(typeId(), create);
}
u16 SwitchNodeMetadata::typeId() const
{
	return CONTENT_CIRCUIT_SWITCH;
}
NodeMetadata* SwitchNodeMetadata::create(std::istream &is)
{
	SwitchNodeMetadata *d = new SwitchNodeMetadata();
	int temp;
	is>>temp;
	d->m_energy = temp;
	is>>temp;
	d->m_ptime = (float)temp/10;
	int i;
	is>>i;
	v3s16 p;
	for (; i > 0; i--) {
		is>>temp;
		p.X = temp;
		is>>temp;
		p.Y = temp;
		is>>temp;
		p.Z = temp;
		is>>temp;
		d->m_sources[p] = temp;
	}
	return d;
}
NodeMetadata* SwitchNodeMetadata::clone()
{
	SwitchNodeMetadata *d = new SwitchNodeMetadata();
	d->m_energy = m_energy;
	d->m_ptime = m_ptime;
	d->m_sources = m_sources;
	return d;
}
bool SwitchNodeMetadata::stepCircuit(float dtime, v3s16 pos, ServerEnvironment *env)
{
	if (!m_energy)
		return false;

	core::map<v3s16,MapBlock*> modified_blocks;
	env->propogateEnergy(ENERGY_MAX,pos,pos,pos,modified_blocks);
	MapEditEvent event;
	event.type = MEET_OTHER;
	for (core::map<v3s16, MapBlock*>::Iterator i = modified_blocks.getIterator(); i.atEnd() == false; i++) {
		v3s16 p = i->getKey();
		event.modified_blocks.insert(p, true);
	}
	env->getMap().dispatchEvent(&event);

	return true;
}
bool SwitchNodeMetadata::energise(u8 level, v3s16 powersrc, v3s16 signalsrc, v3s16 pos)
{
	m_ptime = 0;
	if (m_energy == level)
		return true;
	if (powersrc != pos)
		return false;
	m_energy = level;
	return true;
}
std::vector<NodeBox>  SwitchNodeMetadata::getNodeBoxes(MapNode &n)
{
	std::vector<NodeBox> boxes;
	boxes.clear();

	if (m_energy) {
		boxes.push_back(NodeBox(
			-0.125*BS,0.*BS,0.375*BS,-0.0625*BS,0.0625*BS,0.4375*BS
		));
	}else{
		boxes.push_back(NodeBox(
			0.0625*BS,0.*BS,0.375*BS,0.125*BS,0.0625*BS,0.4375*BS
		));
	}

	return boxes;
}


/*
	ButtonNodeMetadata
*/

// Prototype
ButtonNodeMetadata proto_ButtonNodeMetadata;

ButtonNodeMetadata::ButtonNodeMetadata()
{
	m_energy = 0;
	m_ptime = 0;
	m_sources.clear();
	NodeMetadata::registerType(typeId(), create);
}
u16 ButtonNodeMetadata::typeId() const
{
	return CONTENT_CIRCUIT_BUTTON;
}
NodeMetadata* ButtonNodeMetadata::create(std::istream &is)
{
	ButtonNodeMetadata *d = new ButtonNodeMetadata();
	int temp;
	is>>temp;
	d->m_energy = temp;
	is>>temp;
	d->m_ptime = (float)temp/10;
	int i;
	is>>i;
	v3s16 p;
	for (; i > 0; i--) {
		is>>temp;
		p.X = temp;
		is>>temp;
		p.Y = temp;
		is>>temp;
		p.Z = temp;
		is>>temp;
		d->m_sources[p] = temp;
	}
	return d;
}
NodeMetadata* ButtonNodeMetadata::clone()
{
	ButtonNodeMetadata *d = new ButtonNodeMetadata();
	d->m_energy = m_energy;
	d->m_ptime = m_ptime;
	d->m_sources = m_sources;
	return d;
}
bool ButtonNodeMetadata::stepCircuit(float dtime, v3s16 pos, ServerEnvironment *env)
{
	if (!m_energy)
		return false;

	core::map<v3s16,MapBlock*> modified_blocks;
	env->propogateEnergy(ENERGY_MAX,pos,pos,pos,modified_blocks);
	MapEditEvent event;
	event.type = MEET_OTHER;
	for (core::map<v3s16, MapBlock*>::Iterator i = modified_blocks.getIterator(); i.atEnd() == false; i++) {
		v3s16 p = i->getKey();
		event.modified_blocks.insert(p, true);
	}
	env->getMap().dispatchEvent(&event);

	m_energy = 0;
	return true;
}
bool ButtonNodeMetadata::energise(u8 level, v3s16 powersrc, v3s16 signalsrc, v3s16 pos)
{
	m_ptime = 0;
	if (m_energy == level)
		return true;
	if (powersrc != pos)
		return false;
	m_energy = level;
	return true;
}

/*
	SolarPanelNodeMetadata
*/

// Prototype
SolarPanelNodeMetadata proto_SolarPanelNodeMetadata;

SolarPanelNodeMetadata::SolarPanelNodeMetadata()
{
	m_energy = 0;
	m_ptime = 0;
	m_sources.clear();
	NodeMetadata::registerType(typeId(), create);
}
u16 SolarPanelNodeMetadata::typeId() const
{
	return CONTENT_CIRCUIT_SOLARPANEL;
}
NodeMetadata* SolarPanelNodeMetadata::create(std::istream &is)
{
	SolarPanelNodeMetadata *d = new SolarPanelNodeMetadata();
	int temp;
	is>>temp;
	d->m_energy = temp;
	is>>temp;
	d->m_ptime = (float)temp/10;
	int i;
	is>>i;
	v3s16 p;
	for (; i > 0; i--) {
		is>>temp;
		p.X = temp;
		is>>temp;
		p.Y = temp;
		is>>temp;
		p.Z = temp;
		is>>temp;
		d->m_sources[p] = temp;
	}
	return d;
}
NodeMetadata* SolarPanelNodeMetadata::clone()
{
	SolarPanelNodeMetadata *d = new SolarPanelNodeMetadata();
	d->m_energy = m_energy;
	d->m_ptime = m_ptime;
	d->m_sources = m_sources;
	return d;
}
bool SolarPanelNodeMetadata::stepCircuit(float dtime, v3s16 pos, ServerEnvironment *env)
{
	MapNode n = env->getMap().getNodeNoEx(pos);
	if (n.getLightBlend(env->getDayNightRatio()) < 10) {
		m_energy = 0;
		return false;
	}

	m_energy = ENERGY_MAX;
	core::map<v3s16,MapBlock*> modified_blocks;
	env->propogateEnergy(ENERGY_MAX,pos,pos,pos,modified_blocks);
	MapEditEvent event;
	event.type = MEET_OTHER;
	for (core::map<v3s16, MapBlock*>::Iterator i = modified_blocks.getIterator(); i.atEnd() == false; i++) {
		v3s16 p = i->getKey();
		event.modified_blocks.insert(p, true);
	}
	env->getMap().dispatchEvent(&event);

	return true;
}
bool SolarPanelNodeMetadata::energise(u8 level, v3s16 powersrc, v3s16 signalsrc, v3s16 pos)
{
	if (!m_energy)
		return false;
	return true;
}

/*
	WaterWheelNodeMetadata
*/

// Prototype
WaterWheelNodeMetadata proto_WaterWheelNodeMetadata;

WaterWheelNodeMetadata::WaterWheelNodeMetadata()
{
	m_energy = 0;
	m_ptime = 0;
	m_sources.clear();
	NodeMetadata::registerType(typeId(), create);
}
u16 WaterWheelNodeMetadata::typeId() const
{
	return CONTENT_CIRCUIT_WATERWHEEL;
}
NodeMetadata* WaterWheelNodeMetadata::create(std::istream &is)
{
	WaterWheelNodeMetadata *d = new WaterWheelNodeMetadata();
	int temp;
	is>>temp;
	d->m_energy = temp;
	is>>temp;
	d->m_ptime = (float)temp/10;
	int i;
	is>>i;
	v3s16 p;
	for (; i > 0; i--) {
		is>>temp;
		p.X = temp;
		is>>temp;
		p.Y = temp;
		is>>temp;
		p.Z = temp;
		is>>temp;
		d->m_sources[p] = temp;
	}
	return d;
}
NodeMetadata* WaterWheelNodeMetadata::clone()
{
	WaterWheelNodeMetadata *d = new WaterWheelNodeMetadata();
	d->m_energy = m_energy;
	d->m_ptime = m_ptime;
	d->m_sources = m_sources;
	return d;
}
bool WaterWheelNodeMetadata::stepCircuit(float dtime, v3s16 pos, ServerEnvironment *env)
{
	MapNode n = env->getMap().getNodeNoEx(pos);
	v3s16 dir = n.getRotation();
	if (dir == v3s16(1,1,1)) {
		dir = v3s16(0,0,-1);
	}else if (dir == v3s16(-1,1,1)) {
		dir = v3s16(-1,0,0);
	}else if (dir == v3s16(-1,1,-1)) {
		dir = v3s16(0,0,1);
	}else if (dir == v3s16(1,1,-1)) {
		dir = v3s16(1,0,0);
	}
	MapNode inlet = env->getMap().getNodeNoEx(pos-dir);
	MapNode outlet = env->getMap().getNodeNoEx(pos+dir);
	if (inlet.getContent() != CONTENT_WATERSOURCE) {
		if (outlet.getContent() == CONTENT_WATERSOURCE)
			env->getMap().removeNodeWithEvent(pos+dir);
		if (m_energy) {
			m_energy = 0;
			return true;
		}
		return false;
	}
	if (outlet.getContent() != CONTENT_WATERSOURCE) {
		if (outlet.getContent() != CONTENT_AIR) {
			if (m_energy) {
				m_energy = 0;
				return true;
			}
			return false;
		}
		outlet.setContent(CONTENT_WATERSOURCE);
		env->getMap().addNodeWithEvent(pos+dir,outlet);
	}

	m_energy = ENERGY_MAX;
	core::map<v3s16,MapBlock*> modified_blocks;
	env->propogateEnergy(ENERGY_MAX,pos,pos,pos,modified_blocks);
	MapEditEvent event;
	event.type = MEET_OTHER;
	for (core::map<v3s16, MapBlock*>::Iterator i = modified_blocks.getIterator(); i.atEnd() == false; i++) {
		v3s16 p = i->getKey();
		event.modified_blocks.insert(p, true);
	}
	env->getMap().dispatchEvent(&event);

	return true;
}
bool WaterWheelNodeMetadata::energise(u8 level, v3s16 powersrc, v3s16 signalsrc, v3s16 pos)
{
	if (!m_energy)
		return false;
	return true;
}

/*
	SourceNodeMetadata
*/

// Prototype
SourceNodeMetadata proto_SourceNodeMetadata;

SourceNodeMetadata::SourceNodeMetadata()
{
	m_energy = ENERGY_MAX;
	m_ptime = 0;
	m_sources.clear();
	NodeMetadata::registerType(typeId(), create);
}
u16 SourceNodeMetadata::typeId() const
{
	return CONTENT_CIRCUIT_REACTOR;
}
NodeMetadata* SourceNodeMetadata::create(std::istream &is)
{
	SourceNodeMetadata *d = new SourceNodeMetadata();
	int temp;
	is>>temp;
	d->m_energy = temp;
	is>>temp;
	d->m_ptime = (float)temp/10;
	int i;
	is>>i;
	v3s16 p;
	for (; i > 0; i--) {
		is>>temp;
		p.X = temp;
		is>>temp;
		p.Y = temp;
		is>>temp;
		p.Z = temp;
		is>>temp;
		d->m_sources[p] = temp;
	}
	return d;
}
NodeMetadata* SourceNodeMetadata::clone()
{
	SourceNodeMetadata *d = new SourceNodeMetadata();
	d->m_energy = m_energy;
	d->m_ptime = m_ptime;
	d->m_sources = m_sources;
	return d;
}

/*
	NotGateNodeMetadata
*/

// Prototype
NotGateNodeMetadata proto_NotGateNodeMetadata;

NotGateNodeMetadata::NotGateNodeMetadata()
{
	m_energy = 0;
	m_ptime = 0;
	m_sources.clear();
	NodeMetadata::registerType(typeId(), create);
}
u16 NotGateNodeMetadata::typeId() const
{
	return CONTENT_CIRCUIT_NOTGATE;
}
NodeMetadata* NotGateNodeMetadata::create(std::istream &is)
{
	NotGateNodeMetadata *d = new NotGateNodeMetadata();
	int temp;
	is>>temp;
	d->m_energy = temp;
	is>>temp;
	d->m_ptime = (float)temp/10;
	int i;
	is>>i;
	v3s16 p;
	for (; i > 0; i--) {
		is>>temp;
		p.X = temp;
		is>>temp;
		p.Y = temp;
		is>>temp;
		p.Z = temp;
		is>>temp;
		d->m_sources[p] = temp;
	}
	return d;
}
NodeMetadata* NotGateNodeMetadata::clone()
{
	NotGateNodeMetadata *d = new NotGateNodeMetadata();
	d->m_energy = m_energy;
	d->m_ptime = m_ptime;
	d->m_sources = m_sources;
	return d;
}
bool NotGateNodeMetadata::stepCircuit(float dtime, v3s16 pos, ServerEnvironment *env)
{
	m_ptime += dtime;
	if (m_ptime < 0.6)
		return false;

	m_energy = 0;

	core::map<v3s16,MapBlock*> modified_blocks;
	env->propogateEnergy(ENERGY_MAX,pos,pos,pos,modified_blocks);
	MapEditEvent event;
	event.type = MEET_OTHER;
	for (core::map<v3s16, MapBlock*>::Iterator i = modified_blocks.getIterator(); i.atEnd() == false; i++) {
		v3s16 p = i->getKey();
		event.modified_blocks.insert(p, true);
	}
	env->getMap().dispatchEvent(&event);

	return true;
}
bool NotGateNodeMetadata::energise(u8 level, v3s16 powersrc, v3s16 signalsrc, v3s16 pos)
{
	if (powersrc == pos)
		return true;
	m_ptime = 0;
	if (level && m_sources[powersrc] > level)
		return false;
	if (!level || m_energy < level) {
		m_energy = level;
		if (!level) {
			m_sources.erase(powersrc);
			for (std::map<v3s16,u8>::iterator i = m_sources.begin(); i != m_sources.end(); i++) {
				u8 v = i->second;
				if (v > m_energy)
					m_energy = v;
			}
		}
	}
	return true;
}

/*
	RepeaterNodeMetadata
*/

// Prototype
RepeaterNodeMetadata proto_RepeaterNodeMetadata;

RepeaterNodeMetadata::RepeaterNodeMetadata():
	m_ticks(0)
{
	m_energy = 0;
	m_ptime = 0;
	m_sources.clear();
	NodeMetadata::registerType(typeId(), create);
}
u16 RepeaterNodeMetadata::typeId() const
{
	return CONTENT_CIRCUIT_REPEATER;
}
NodeMetadata* RepeaterNodeMetadata::create(std::istream &is)
{
	RepeaterNodeMetadata *d = new RepeaterNodeMetadata();
	int temp;
	is>>temp;
	d->m_energy = temp;
	is>>temp;
	d->m_ticks = temp;
	is>>temp;
	d->m_ptime = (float)temp/10;
	int i;
	is>>i;
	v3s16 p;
	for (; i > 0; i--) {
		is>>temp;
		p.X = temp;
		is>>temp;
		p.Y = temp;
		is>>temp;
		p.Z = temp;
		is>>temp;
		d->m_sources[p] = temp;
	}
	return d;
}
NodeMetadata* RepeaterNodeMetadata::clone()
{
	RepeaterNodeMetadata *d = new RepeaterNodeMetadata();
	d->m_energy = m_energy;
	d->m_ptime = m_ptime;
	d->m_sources = m_sources;
	return d;
}
void RepeaterNodeMetadata::serializeBody(std::ostream &os)
{
	os<<itos(m_energy) << " ";
	os<<itos(m_ticks) << " ";
	os<<itos(m_ptime*10)<<" ";
	os<<itos(m_sources.size()) << " ";
	for (std::map<v3s16,u8>::iterator i = m_sources.begin(); i != m_sources.end(); i++) {
		os<<itos(i->first.X) << " ";
		os<<itos(i->first.Y) << " ";
		os<<itos(i->first.Z) << " ";
		os<<itos(i->second) << " ";
	}
}
bool RepeaterNodeMetadata::stepCircuit(float dtime, v3s16 pos, ServerEnvironment *env)
{
	m_ptime += dtime;
	if (m_ptime > 1.0 && m_ticks > 0) {
		m_energy = 0;
		m_ticks--;
	}
	if (!m_energy && !m_ticks) {
		return false;
	}else if (m_energy && m_ticks < 6) {
		m_ticks++;
		return true;
	}

	core::map<v3s16,MapBlock*> modified_blocks;
	env->propogateEnergy(ENERGY_MAX,pos,pos,pos,modified_blocks);
	MapEditEvent event;
	event.type = MEET_OTHER;
	for (core::map<v3s16, MapBlock*>::Iterator i = modified_blocks.getIterator(); i.atEnd() == false; i++) {
		v3s16 p = i->getKey();
		event.modified_blocks.insert(p, true);
	}
	env->getMap().dispatchEvent(&event);

	return true;
}
bool RepeaterNodeMetadata::energise(u8 level, v3s16 powersrc, v3s16 signalsrc, v3s16 pos)
{
	if (powersrc == pos)
		return true;
	m_ptime = 0;
	if (level && m_sources[powersrc] > level)
		return false;
	if (!level || m_energy < level) {
		m_energy = level;
		if (!level) {
			m_sources.erase(powersrc);
			for (std::map<v3s16,u8>::iterator i = m_sources.begin(); i != m_sources.end(); i++) {
				u8 v = i->second;
				if (v > m_energy)
					m_energy = v;
			}
		}
	}
	return true;
}


/*
	DoorNodeMetadata
*/

// Prototype
DoorNodeMetadata proto_DoorNodeMetadata;

DoorNodeMetadata::DoorNodeMetadata():
	m_otime(0)
{
	m_energy = 0;
	m_ptime = 0;
	m_sources.clear();
	NodeMetadata::registerType(typeId(), create);
}
u16 DoorNodeMetadata::typeId() const
{
	return CONTENT_IRON_DOOR_LB;
}
NodeMetadata* DoorNodeMetadata::create(std::istream &is)
{
	DoorNodeMetadata *d = new DoorNodeMetadata();
	int temp;
	is>>temp;
	d->m_energy = temp;
	is>>temp;
	d->m_ptime = (float)temp/10;
	is>>temp;
	d->m_otime = (float)temp/10;
	int i;
	is>>i;
	v3s16 p;
	for (; i > 0; i--) {
		is>>temp;
		p.X = temp;
		is>>temp;
		p.Y = temp;
		is>>temp;
		p.Z = temp;
		is>>temp;
		d->m_sources[p] = temp;
	}
	return d;
}
NodeMetadata* DoorNodeMetadata::clone()
{
	DoorNodeMetadata *d = new DoorNodeMetadata();
	d->m_energy = m_energy;
	d->m_ptime = m_ptime;
	d->m_otime = m_otime;
	d->m_sources = m_sources;
	return d;
}
void DoorNodeMetadata::serializeBody(std::ostream &os)
{
	os<<itos(m_energy) << " ";
	os<<itos(m_ptime*10)<<" ";
	os<<itos(m_otime*10)<<" ";
	os<<itos(m_sources.size()) << " ";
	for (std::map<v3s16,u8>::iterator i = m_sources.begin(); i != m_sources.end(); i++) {
		os<<itos(i->first.X) << " ";
		os<<itos(i->first.Y) << " ";
		os<<itos(i->first.Z) << " ";
		os<<itos(i->second) << " ";
	}
}
bool DoorNodeMetadata::stepCircuit(float dtime, v3s16 pos, ServerEnvironment *env)
{
	if (m_ptime == 0.0 && m_otime == 0.0 && m_energy == 0) {
		MapNode n = env->getMap().getNodeNoEx(pos);
		if ((n.getContent()&CONTENT_DOOR_OPEN_MASK) == CONTENT_DOOR_OPEN_MASK)
			m_otime = 5.0;
	}
	m_ptime += dtime;
	if (!m_energy) {
		if (m_otime <= 0.0) {
			MapNode n = env->getMap().getNodeNoEx(pos);
			content_t c = n.getContent();
			if ((c&CONTENT_DOOR_OPEN_MASK) == CONTENT_DOOR_OPEN_MASK) {
				n.setContent(n.getContent()&~CONTENT_DOOR_OPEN_MASK);
				env->setPostStepNodeSwap(pos,n);
				return true;
			}
			return false;
		}
		m_otime -= dtime;
		return false;
	}else{
		MapNode n = env->getMap().getNodeNoEx(pos);
		content_t c = n.getContent();
		if ((c&CONTENT_DOOR_OPEN_MASK) != CONTENT_DOOR_OPEN_MASK) {
			n.setContent(n.getContent()|CONTENT_DOOR_OPEN_MASK);
			env->setPostStepNodeSwap(pos,n);
		}
	}
	if (m_ptime < 0.3)
		return false;
	m_energy = 0;
	m_otime = 5.0;
	return true;
}
bool DoorNodeMetadata::energise(u8 level, v3s16 powersrc, v3s16 signalsrc, v3s16 pos)
{
	m_ptime = 0;
	if (!m_energy)
		m_sources.clear();
	if (powersrc == pos) {
		m_otime = 5.0;
		return true;
	}
	if (m_sources[powersrc] == level)
		return true;
	if (level && m_sources[powersrc] > level)
		return false;
	m_sources[powersrc] = level;
	if (!level || m_energy < level) {
		m_energy = level;
		if (!level) {
			m_sources.erase(powersrc);
			for (std::map<v3s16,u8>::iterator i = m_sources.begin(); i != m_sources.end(); i++) {
				u8 v = i->second;
				if (v > m_energy)
					m_energy = v;
			}
		}
	}
	return true;
}


/*
	PistonNodeMetadata
*/

// Prototype
PistonNodeMetadata proto_PistonNodeMetadata;

PistonNodeMetadata::PistonNodeMetadata()
{
	m_energy = 0;
	m_ptime = 0;
	m_otime = 0;
	m_sources.clear();
	NodeMetadata::registerType(typeId(), create);
}
u16 PistonNodeMetadata::typeId() const
{
	return CONTENT_CIRCUIT_PISTON;
}
NodeMetadata* PistonNodeMetadata::create(std::istream &is)
{
	PistonNodeMetadata *d = new PistonNodeMetadata();
	int temp;
	is>>temp;
	d->m_energy = temp;
	is>>temp;
	d->m_ptime = (float)temp/10;
	int i;
	is>>i;
	v3s16 p;
	for (; i > 0; i--) {
		is>>temp;
		p.X = temp;
		is>>temp;
		p.Y = temp;
		is>>temp;
		p.Z = temp;
		is>>temp;
		d->m_sources[p] = temp;
	}
	return d;
}
NodeMetadata* PistonNodeMetadata::clone()
{
	PistonNodeMetadata *d = new PistonNodeMetadata();
	d->m_energy = m_energy;
	d->m_ptime = m_ptime;
	d->m_sources = m_sources;
	return d;
}
void PistonNodeMetadata::serializeBody(std::ostream &os)
{
	os<<itos(m_energy) << " ";
	os<<itos(m_ptime*10)<<" ";
	os<<itos(m_sources.size()) << " ";
	for (std::map<v3s16,u8>::iterator i = m_sources.begin(); i != m_sources.end(); i++) {
		os<<itos(i->first.X) << " ";
		os<<itos(i->first.Y) << " ";
		os<<itos(i->first.Z) << " ";
		os<<itos(i->second) << " ";
	}
}
bool PistonNodeMetadata::stepCircuit(float dtime, v3s16 pos, ServerEnvironment *env)
{
	m_ptime += dtime;
	if (!m_energy) {
		MapNode n = env->getMap().getNodeNoEx(pos);
		v3s16 dir = n.getRotation();
		switch (n.getContent()) {
		case CONTENT_CIRCUIT_PISTON:
		{
			if (dir == v3s16(1,1,1)) {
				dir = v3s16(0,0,-1);
			}else if (dir == v3s16(-1,1,1)) {
				dir = v3s16(-1,0,0);
			}else if (dir == v3s16(-1,1,-1)) {
				dir = v3s16(0,0,1);
			}else if (dir == v3s16(1,1,-1)) {
				dir = v3s16(1,0,0);
			}
			n.setContent(CONTENT_CIRCUIT_PISTON_OFF);
			contract(pos,dir,false,n,env);
			return true;
		}
		case CONTENT_CIRCUIT_PISTON_UP:
		{
			dir = v3s16(0,1,0);
			n.setContent(CONTENT_CIRCUIT_PISTON_UP_OFF);
			contract(pos,dir,false,n,env);
			return true;
		}
		case CONTENT_CIRCUIT_PISTON_DOWN:
		{
			dir = v3s16(0,-1,0);
			n.setContent(CONTENT_CIRCUIT_PISTON_DOWN_OFF);
			contract(pos,dir,false,n,env);
			return true;
		}
		case CONTENT_CIRCUIT_STICKYPISTON:
		{
			if (dir == v3s16(1,1,1)) {
				dir = v3s16(0,0,-1);
			}else if (dir == v3s16(-1,1,1)) {
				dir = v3s16(-1,0,0);
			}else if (dir == v3s16(-1,1,-1)) {
				dir = v3s16(0,0,1);
			}else if (dir == v3s16(1,1,-1)) {
				dir = v3s16(1,0,0);
			}
			n.setContent(CONTENT_CIRCUIT_STICKYPISTON_OFF);
			contract(pos,dir,true,n,env);
			return true;
		}
		case CONTENT_CIRCUIT_STICKYPISTON_UP:
		{
			dir = v3s16(0,1,0);
			n.setContent(CONTENT_CIRCUIT_STICKYPISTON_UP_OFF);
			contract(pos,dir,true,n,env);
			return true;
		}
		case CONTENT_CIRCUIT_STICKYPISTON_DOWN:
		{
			dir = v3s16(0,-1,0);
			n.setContent(CONTENT_CIRCUIT_STICKYPISTON_DOWN_OFF);
			contract(pos,dir,true,n,env);
			return true;
		}
		case CONTENT_CIRCUIT_PISTON_OFF:
		{
			if (dir == v3s16(1,1,1)) {
				dir = v3s16(0,0,-1);
			}else if (dir == v3s16(-1,1,1)) {
				dir = v3s16(-1,0,0);
			}else if (dir == v3s16(-1,1,-1)) {
				dir = v3s16(0,0,1);
			}else if (dir == v3s16(1,1,-1)) {
				dir = v3s16(1,0,0);
			}
			if (env->getMap().getNodeNoEx(pos+dir).getContent() == CONTENT_CIRCUIT_PISTON_ARM)
				contract(pos,dir,false,n,env);
			return true;
		}
		case CONTENT_CIRCUIT_PISTON_UP_OFF:
		{
			dir = v3s16(0,1,0);
			if (env->getMap().getNodeNoEx(pos+dir).getContent() == CONTENT_CIRCUIT_PISTON_UP_ARM)
				contract(pos,dir,false,n,env);
			return true;
		}
		case CONTENT_CIRCUIT_PISTON_DOWN_OFF:
		{
			dir = v3s16(0,-1,0);
			if (env->getMap().getNodeNoEx(pos+dir).getContent() == CONTENT_CIRCUIT_PISTON_DOWN_ARM)
				contract(pos,dir,false,n,env);
			return true;
		}
		case CONTENT_CIRCUIT_STICKYPISTON_OFF:
		{
			if (dir == v3s16(1,1,1)) {
				dir = v3s16(0,0,-1);
			}else if (dir == v3s16(-1,1,1)) {
				dir = v3s16(-1,0,0);
			}else if (dir == v3s16(-1,1,-1)) {
				dir = v3s16(0,0,1);
			}else if (dir == v3s16(1,1,-1)) {
				dir = v3s16(1,0,0);
			}
			if (env->getMap().getNodeNoEx(pos+dir).getContent() == CONTENT_CIRCUIT_STICKYPISTON_ARM)
				contract(pos,dir,true,n,env);
			return true;
		}
		case CONTENT_CIRCUIT_STICKYPISTON_UP_OFF:
		{
			dir = v3s16(0,1,0);
			if (env->getMap().getNodeNoEx(pos+dir).getContent() == CONTENT_CIRCUIT_STICKYPISTON_UP_ARM)
				contract(pos,dir,true,n,env);
			return true;
		}
		case CONTENT_CIRCUIT_STICKYPISTON_DOWN_OFF:
		{
			dir = v3s16(0,-1,0);
			if (env->getMap().getNodeNoEx(pos+dir).getContent() == CONTENT_CIRCUIT_STICKYPISTON_DOWN_ARM)
				contract(pos,dir,true,n,env);
			return true;
		}
		default:;
		}
		return false;
	}else{
		m_otime += dtime;
		if (m_otime < 3)
			return false;
		MapNode n = env->getMap().getNodeNoEx(pos);
		v3s16 dir = n.getRotation();
		switch (n.getContent()) {
		case CONTENT_CIRCUIT_PISTON_OFF:
		{
			if (dir == v3s16(1,1,1)) {
				dir = v3s16(0,0,-1);
			}else if (dir == v3s16(-1,1,1)) {
				dir = v3s16(-1,0,0);
			}else if (dir == v3s16(-1,1,-1)) {
				dir = v3s16(0,0,1);
			}else if (dir == v3s16(1,1,-1)) {
				dir = v3s16(1,0,0);
			}
			n.setContent(CONTENT_CIRCUIT_PISTON);
			extend(pos,dir,CONTENT_CIRCUIT_PISTON_ARM,n,env);
			break;
		}
		case CONTENT_CIRCUIT_PISTON_UP_OFF:
		{
			dir = v3s16(0,1,0);
			n.setContent(CONTENT_CIRCUIT_PISTON_UP);
			extend(pos,dir,CONTENT_CIRCUIT_PISTON_UP_ARM,n,env);
			break;
		}
		case CONTENT_CIRCUIT_PISTON_DOWN_OFF:
		{
			dir = v3s16(0,-1,0);
			n.setContent(CONTENT_CIRCUIT_PISTON_DOWN);
			extend(pos,dir,CONTENT_CIRCUIT_PISTON_DOWN_ARM,n,env);
			break;
		}
		case CONTENT_CIRCUIT_STICKYPISTON_OFF:
		{
			if (dir == v3s16(1,1,1)) {
				dir = v3s16(0,0,-1);
			}else if (dir == v3s16(-1,1,1)) {
				dir = v3s16(-1,0,0);
			}else if (dir == v3s16(-1,1,-1)) {
				dir = v3s16(0,0,1);
			}else if (dir == v3s16(1,1,-1)) {
				dir = v3s16(1,0,0);
			}
			n.setContent(CONTENT_CIRCUIT_STICKYPISTON);
			extend(pos,dir,CONTENT_CIRCUIT_STICKYPISTON_ARM,n,env);
			break;
		}
		case CONTENT_CIRCUIT_STICKYPISTON_UP_OFF:
		{
			dir = v3s16(0,1,0);
			n.setContent(CONTENT_CIRCUIT_STICKYPISTON_UP);
			extend(pos,dir,CONTENT_CIRCUIT_STICKYPISTON_UP_ARM,n,env);
			break;
		}
		case CONTENT_CIRCUIT_STICKYPISTON_DOWN_OFF:
		{
			dir = v3s16(0,-1,0);
			n.setContent(CONTENT_CIRCUIT_STICKYPISTON_DOWN);
			extend(pos,dir,CONTENT_CIRCUIT_STICKYPISTON_DOWN_ARM,n,env);
			break;
		}
		default:;
		}
	}
	if (m_ptime < 3)
		return false;
	//m_otime += dtime;
	//if (m_otime < 1.0)
		//return false;
	m_energy = 0;
	return true;
}
bool PistonNodeMetadata::energise(u8 level, v3s16 powersrc, v3s16 signalsrc, v3s16 pos)
{
	m_ptime = 0;
	if (!m_energy)
		m_sources.clear();
	if (m_sources[powersrc] == level)
		return true;
	if (level && m_sources[powersrc] > level)
		return false;
	m_sources[powersrc] = level;
	if (!level || m_energy < level) {
		m_energy = level;
		if (!level) {
			m_sources.erase(powersrc);
			for (std::map<v3s16,u8>::iterator i = m_sources.begin(); i != m_sources.end(); i++) {
				u8 v = i->second;
				if (v > m_energy)
					m_energy = v;
			}
		}
	}
	if (level)
		m_otime = 0;
	return true;
}
bool PistonNodeMetadata::extend(v3s16 pos, v3s16 dir, content_t arm, MapNode piston, ServerEnvironment *env)
{
	bool can_extend = false;
	v3s16 epos = pos;
	s16 max_d = config_get_int("world.game.borderstone.radius");
	for (int i=0; i<17; i++) {
		epos += dir;
		v3s16 test_p;
		MapNode testnode;
		for(s16 z=-max_d; z<=max_d; z++) {
		for(s16 y=-max_d; y<=max_d; y++) {
		for(s16 x=-max_d; x<=max_d; x++) {
			test_p = epos + v3s16(x,y,z);
			testnode = env->getMap().getNodeNoEx(test_p);
			if (testnode.getContent() == CONTENT_IGNORE || testnode.getContent() == CONTENT_BORDERSTONE)
				return false;
		}
		}
		}
		MapNode n = env->getMap().getNodeNoEx(epos);
		if (n.getContent() == CONTENT_IGNORE)
			return false;
		ContentFeatures &f = content_features(n);
		if (f.pressure_type == CST_SOLID)
			return false;
		if (f.pressure_type == CST_CRUSHED || f.pressure_type == CST_CRUSHABLE) {
			can_extend = true;
			break;
		}
	}
	if (!can_extend)
		return false;
	env->addEnvEvent(ENV_EVENT_SOUND,intToFloat(pos,BS),"env-piston");
	MapNode n_prev;
	MapNode n_cur;
	MapNode n_next;
	v3s16 p_prev = pos;
	v3s16 p_cur = p_prev+dir;
	v3s16 p_next = p_cur+dir;
	env->getMap().addNodeWithEvent(pos,piston);

	if (arm == CONTENT_CIRCUIT_PISTON_ARM || arm == CONTENT_CIRCUIT_STICKYPISTON_ARM)
		n_prev = env->getMap().getNodeNoEx(p_prev);
	n_prev.setContent(arm);
	n_cur = env->getMap().getNodeNoEx(p_cur);
	for (int i=0; i<17; i++) {
		ContentFeatures &f = content_features(n_cur);
		n_next = env->getMap().getNodeNoEx(p_next);
		env->getMap().addNodeWithEvent(p_cur,n_prev);
		if (f.pressure_type == CST_CRUSHED)
			break;
		if (f.pressure_type == CST_CRUSHABLE && n_next.getContent() != CONTENT_AIR)
			break;
		n_prev = n_cur;
		n_cur = n_next;
		p_prev = p_cur;
		p_cur = p_next;
		p_next += dir;
	}

	return true;
}
bool PistonNodeMetadata::contract(v3s16 pos, v3s16 dir, bool sticky, MapNode piston, ServerEnvironment *env)
{
	bool dropping = false;
	bool contract = true;
	bool remove_arm = true;
	if (dir.Y == 1)
		dropping = true;
	if (sticky || dropping) {
		s16 max_d = config_get_int("world.game.borderstone.radius");
		v3s16 p_cur = pos+dir;
		v3s16 p_next = p_cur+dir;
		bool walk = true;
		for (int i=0; walk && i<16; i++) {
			MapNode n = env->getMap().getNodeNoEx(p_next);
			if (n.getContent() == CONTENT_IGNORE) {
				contract = false;
				walk = false;
				break;
			}
			ContentFeatures &f = content_features(n);
			if (f.pressure_type == CST_SOLID)
				break;
			if (f.liquid_type != LIQUID_NONE)
				break;
			if ((!sticky || i) && f.pressure_type != CST_DROPABLE)
				break;
			v3s16 test_p;
			MapNode testnode;
			for(s16 z=-max_d; walk && z<=max_d; z++) {
			for(s16 y=-max_d; walk && y<=max_d; y++) {
			for(s16 x=-max_d; walk && x<=max_d; x++) {
				test_p = p_cur + v3s16(x,y,z);
				testnode = env->getMap().getNodeNoEx(test_p);
				if (testnode.getContent() == CONTENT_IGNORE || testnode.getContent() == CONTENT_BORDERSTONE) {
					walk = false;
					contract = false;
					break;
				}
			}
			}
			}
			if (!dropping)
				break;
			p_cur = p_next;
			p_next += dir;
		}

		if (walk) {
			p_cur = pos+dir;
			p_next = p_cur+dir;

			for (int i=0; i<16; i++) {
				MapNode n = env->getMap().getNodeNoEx(p_next);
				if (n.getContent() == CONTENT_IGNORE)
					break;
				ContentFeatures &f = content_features(n);
				if (f.pressure_type == CST_SOLID)
					break;
				if (f.liquid_type != LIQUID_NONE)
					break;
				if ((!sticky || i) && f.pressure_type != CST_DROPABLE)
					break;
				env->getMap().removeNodeWithEvent(p_next);
				env->getMap().addNodeWithEvent(p_cur,n);
				remove_arm = false;
				if (!dropping)
					break;
				p_cur = p_next;
				p_next += dir;
			}
		}
	}
	if (contract) {
		env->addEnvEvent(ENV_EVENT_SOUND,intToFloat(pos,BS),"env-piston");
		env->getMap().addNodeWithEvent(pos,piston);
		if (remove_arm)
			env->getMap().removeNodeWithEvent(pos+dir);
	}

	return true;
}
