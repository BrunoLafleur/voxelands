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
	BookNodeMetadata
*/

// Prototype
BookNodeMetadata proto_BookNodeMetadata;

BookNodeMetadata::BookNodeMetadata()
{
	NodeMetadata::registerType(typeId(), create);

	m_title = "Book";
	m_content = "";
}

u16 BookNodeMetadata::typeId() const
{
	return CONTENT_BOOK_OPEN;
}
NodeMetadata* BookNodeMetadata::clone()
{
	BookNodeMetadata *d = new BookNodeMetadata();
	d->m_title = m_title;
	d->m_content = m_content;
	return d;
}
NodeMetadata* BookNodeMetadata::create(std::istream &is)
{
	BookNodeMetadata *d = new BookNodeMetadata();
	d->m_title = deSerializeString(is);
	d->m_content = deSerializeString(is);
	return d;
}
void BookNodeMetadata::serializeBody(std::ostream &os)
{
	os<<serializeString(m_title);
	os<<serializeString(m_content);
}
bool BookNodeMetadata::nodeRemovalDisabled()
{
	if (m_content != "")
		return true;
	return false;
}
bool BookNodeMetadata::import(NodeMetadata *meta)
{
	if (meta->typeId() != CONTENT_BOOK)
		return false;

	ClosedBookNodeMetadata *m = (ClosedBookNodeMetadata*)meta;
	m_title = wide_to_narrow(m->infoText());
	m_content = m->getContent();
	return true;
}
bool BookNodeMetadata::receiveFields(std::string formname, std::map<std::string, std::string> fields, Player *player)
{
	m_title = fields["title"];
	m_content = fields["content"];

	std::string::size_type pos = 0;
	while ((pos = m_content.find("]",pos)) != std::string::npos) {
		m_content.replace(pos,1,")");
		pos++;
	}
	pos = 0;
	while ((pos = m_content.find("[",pos)) != std::string::npos) {
		m_content.replace(pos,1,"(");
		pos++;
	}
	return true;
}

std::string BookNodeMetadata::getDrawSpecString(Player *player)
{
	std::string spec("size[6,6]");
	spec += "field[1,1;5,1;title;";
	spec += gettext("Title");;
	spec += ";";
	spec += m_title;
	spec += "]";
	spec += "field[1,2;5,2;content;";
	spec += gettext("Content");;
	spec += ";";
	spec += m_content;
	spec += "]";
	spec += "button_exit[2,5;3,1;submit;";
	spec += gettext("Save");;
	spec += "]";
	return spec;
}

/*
	DiaryNodeMetadata
*/

// Prototype
DiaryNodeMetadata proto_DiaryNodeMetadata;

DiaryNodeMetadata::DiaryNodeMetadata()
{
	NodeMetadata::registerType(typeId(), create);

	m_title = gettext("Diary");
	m_content = "";
}

u16 DiaryNodeMetadata::typeId() const
{
	return CONTENT_DIARY_BOOK_OPEN;
}
NodeMetadata* DiaryNodeMetadata::clone()
{
	DiaryNodeMetadata *d = new DiaryNodeMetadata();
	d->m_owner = m_owner;
	d->m_title = m_title;
	d->m_content = m_content;
	return d;
}
NodeMetadata* DiaryNodeMetadata::create(std::istream &is)
{
	DiaryNodeMetadata *d = new DiaryNodeMetadata();
	d->m_owner = deSerializeString(is);
	d->m_title = deSerializeString(is);
	d->m_content = deSerializeString(is);
	return d;
}
void DiaryNodeMetadata::serializeBody(std::ostream &os)
{
	os<<serializeString(m_owner);
	os<<serializeString(m_title);
	os<<serializeString(m_content);
}
bool DiaryNodeMetadata::nodeRemovalDisabled()
{
	if (m_content != "")
		return true;
	return false;
}
bool DiaryNodeMetadata::import(NodeMetadata *meta)
{
	if (meta->typeId() != CONTENT_BOOK)
		return false;

	ClosedBookNodeMetadata *m = (ClosedBookNodeMetadata*)meta;
	m_owner = m->getOwner();
	m_title = wide_to_narrow(m->infoText());
	m_content = m->getContent();
	return true;
}
bool DiaryNodeMetadata::receiveFields(std::string formname, std::map<std::string, std::string> fields, Player *player)
{
	if (m_owner == "")
		m_owner = player->getName();
	if (player->getName() != m_owner)
		return false;
	m_title = fields["title"];
	m_content = fields["content"];

	std::string::size_type pos = 0;
	while ((pos = m_content.find("]",pos)) != std::string::npos) {
		m_content.replace(pos,1,")");
		pos++;
	}
	pos = 0;
	while ((pos = m_content.find("[",pos)) != std::string::npos) {
		m_content.replace(pos,1,"(");
		pos++;
	}
	return true;
}

std::string DiaryNodeMetadata::getDrawSpecString(Player *player)
{
	std::string spec("size[6,6]");
	spec += "field[1,1;5,1;title;";
	spec += gettext("Title");;
	spec += ";";
	spec += m_title;
	spec += "]";
	spec += "field[1,2;5,2;content;";
	spec += gettext("Content");;
	spec += ";";
	spec += m_content;
	spec += "]";
	spec += "button_exit[2,5;3,1;submit;";
	spec += gettext("Save");;
	spec += "]";
	return spec;
}

/*
	ClosedBookNodeMetadata
*/

// Prototype
ClosedBookNodeMetadata proto_ClosedBookNodeMetadata;

ClosedBookNodeMetadata::ClosedBookNodeMetadata()
{
	NodeMetadata::registerType(typeId(), create);

	m_page = 0;
	m_owner = "";
	m_title = "";
	m_content = "";
}
u16 ClosedBookNodeMetadata::typeId() const
{
	return CONTENT_BOOK;
}
NodeMetadata* ClosedBookNodeMetadata::clone()
{
	ClosedBookNodeMetadata *d = new ClosedBookNodeMetadata();
	d->m_page = m_page;
	d->m_owner = m_owner;
	d->m_title = m_title;
	d->m_content = m_content;
	return d;
}
NodeMetadata* ClosedBookNodeMetadata::create(std::istream &is)
{
	ClosedBookNodeMetadata *d = new ClosedBookNodeMetadata();

	d->m_owner = deSerializeString(is);
	d->m_title = deSerializeString(is);
	d->m_content = deSerializeString(is);
	is>>d->m_page;

	return d;
}
void ClosedBookNodeMetadata::serializeBody(std::ostream &os)
{
	os<<serializeString(m_owner);
	os<<serializeString(m_title);
	os<<serializeString(m_content);
	os<<itos(m_page) << " ";
}
std::string ClosedBookNodeMetadata::getText()
{
	return m_title;
}
std::wstring ClosedBookNodeMetadata::infoText()
{
	return narrow_to_wide(m_title.c_str());
}
bool ClosedBookNodeMetadata::nodeRemovalDisabled()
{
	return false;
}
bool ClosedBookNodeMetadata::import(NodeMetadata *meta)
{
	switch (meta->typeId()) {
	case CONTENT_CRAFT_BOOK_OPEN:
	{
		CraftGuideNodeMetadata *cm = (CraftGuideNodeMetadata*)meta;
		m_page = cm->getPage();
		m_title = wide_to_narrow(cm->infoText());
		break;
	}
	case CONTENT_RCRAFT_BOOK_OPEN:
	{
		ReverseCraftGuideNodeMetadata *cm = (ReverseCraftGuideNodeMetadata*)meta;
		m_page = cm->getPage();
		m_title = wide_to_narrow(cm->infoText());
		break;
	}
	case CONTENT_DECRAFT_BOOK_OPEN:
	{
		DeCraftNodeMetadata *cm = (DeCraftNodeMetadata*)meta;
		m_page = cm->getPage();
		m_title = wide_to_narrow(cm->infoText());
		break;
	}
	case CONTENT_BOOK_OPEN:
	{
		BookNodeMetadata *bm = (BookNodeMetadata*)meta;
		m_title = wide_to_narrow(bm->infoText());
		m_content = bm->getContent();
		break;
	}
	case CONTENT_DIARY_BOOK_OPEN:
	{
		DiaryNodeMetadata *dm = (DiaryNodeMetadata*)meta;
		m_owner = dm->getOwner();
		m_title = wide_to_narrow(dm->infoText());
		m_content = dm->getContent();
		break;
	}
	default:;
	}
	return false;
}
