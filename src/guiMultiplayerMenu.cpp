/************************************************************************
* guiMultiplayerMenu.cpp
* voxelands - 3d voxel world sandbox game
* Copyright (C) Lisa 'darkrose' Milne 2013-2015 <lisa@ltmnet.com>
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

#include "guiMultiplayerMenu.h"
#include "debug.h"
#include "serialization.h"
#include "main.h"
#include <string>
#include <IGUICheckBox.h>
#include <IGUIEditBox.h>
#include <IGUIButton.h>
#include <IGUIStaticText.h>
#include <IGUIListBox.h>
#include <IGUIFont.h>
#include <IGUIScrollBar.h>
#include "intlGUIEditBox.h"
#include "gui_colours.h"
#include "http.h"

GUIMultiplayerMenu::GUIMultiplayerMenu(
	gui::IGUIEnvironment* env,
	gui::IGUIElement*
	parent,
	s32 id,
	MainMenuData *data,
	IMenuManager *menumgr,
	IGameCallback *gamecallback
):
	GUIModalMenu(env, parent, id, menumgr),
	m_accepted(false),
	m_gamecallback(gamecallback)
{
	m_data.mmdata = data;

	loadServers();
}

GUIMultiplayerMenu::~GUIMultiplayerMenu()
{
	removeChildren();
}

void GUIMultiplayerMenu::removeChildren()
{
	const core::list<gui::IGUIElement*> &children = getChildren();
	core::list<gui::IGUIElement*> children_copy;
	for (core::list<gui::IGUIElement*>::ConstIterator i = children.begin(); i != children.end(); i++) {
		children_copy.push_back(*i);
	}
	for (core::list<gui::IGUIElement*>::Iterator i = children_copy.begin(); i != children_copy.end(); i++) {
		(*i)->remove();
	}
}

void GUIMultiplayerMenu::regenerateGui(v2u32 screensize)
{
	std::wstring text_name;
	std::wstring text_address;
	std::wstring text_port;

	m_screensize = screensize;

	// Client options
	{
		gui::IGUIElement *e = getElementFromId(GUI_ID_NAME_INPUT);
		if (e != NULL) {
			text_name = e->getText();
		}else{
			text_name = m_data.mmdata->name;
		}
		if (text_name == L"")
			text_name = narrow_to_wide(porting::getUser());
	}
	{
		gui::IGUIElement *e = getElementFromId(GUI_ID_ADDRESS_INPUT);
		if (e != NULL) {
			text_address = e->getText();
		}else{
			text_address = m_data.mmdata->address;
		}
	}
	{
		gui::IGUIElement *e = getElementFromId(GUI_ID_PORT_INPUT);
		if (e != NULL) {
			text_port = e->getText();
		}else{
			text_port = m_data.mmdata->port;
		}
	}

	/*
	 Remove stuff
	 */
	removeChildren();

	/*
	 Calculate new sizes and positions
	 */

	v2s32 size(800, 550);

	core::rect<s32> rect(
			screensize.X/2 - size.X/2,
			screensize.Y/2 - size.Y/2,
			screensize.X/2 + size.X/2,
			screensize.Y/2 + size.Y/2
	);

	DesiredRect = rect;
	recalculateAbsolutePosition(false);

	// Main Menu button
	{
		core::rect<s32> rect(0, 0, 200, 40);
		rect += v2s32(25, 200);
		Environment->addButton(rect, this, GUI_ID_TAB_MP_MAINMENU, narrow_to_wide(gettext("Main Menu")).c_str());
	}

	// Dynamic List button
	{
		core::rect<s32> rect(0, 0, 180, 40);
		rect += v2s32(35, 260);
		Environment->addButton(rect, this, GUI_ID_TAB_MP_LIST, narrow_to_wide(gettext("All Servers")).c_str());
	}
	// Favourites List button
	{
		core::rect<s32> rect(0, 0, 180, 40);
		rect += v2s32(35, 305);
		Environment->addButton(rect, this, GUI_ID_TAB_MP_FAVOURITES, narrow_to_wide(gettext("Favourite Servers")).c_str());
	}
	// Custom Connect button (the old multiplayer menu)
	{
		core::rect<s32> rect(0, 0, 180, 40);
		rect += v2s32(35, 350);
		Environment->addButton(rect, this, GUI_ID_TAB_MP_CUSTOM, narrow_to_wide(gettext("Custom Connect")).c_str());
	}

	v2s32 topleft_content(250, 0);
	{
		core::rect<s32> rect(0, 0, 550, 20);
		rect += topleft_content + v2s32(0, 10);
		gui::IGUIStaticText *t = Environment->addStaticText(narrow_to_wide(gettext("Multi Player")).c_str(), rect, false, true, this, -1);
		t->setTextAlignment(gui::EGUIA_CENTER, gui::EGUIA_UPPERLEFT);
	}

	if (m_data.selected_tab == TAB_MP_CUSTOM) {
		{
			core::rect<s32> rect(0, 0, 550, 20);
			rect += topleft_content + v2s32(0, 30);
			gui::IGUIStaticText *t = Environment->addStaticText(narrow_to_wide(gettext("Custom Connection")).c_str(), rect, false, true, this, -1);
			t->setTextAlignment(gui::EGUIA_CENTER, gui::EGUIA_UPPERLEFT);
		}
		// Address + port
		{
			core::rect<s32> rect(0, 0, 110, 20);
			rect += topleft_content + v2s32(120, 60);
			Environment->addStaticText(narrow_to_wide(gettext("Address/Port")).c_str(), rect, false, true, this, -1);
		}
		{
			if (text_address == L"")
				text_address = L"servers.voxelands.com";
			core::rect<s32> rect(0, 0, 230, 30);
			rect += topleft_content + v2s32(135, 90);
#if USE_FREETYPE
			new gui::intlGUIEditBox(text_address.c_str(), true, Environment, this, GUI_ID_ADDRESS_INPUT, rect);
#else
			Environment->addEditBox(text_address.c_str(), rect, false, this, GUI_ID_ADDRESS_INPUT);
#endif
		}
		{
			core::rect<s32> rect(0, 0, 120, 30);
			rect += topleft_content + v2s32(245, 125);
#if USE_FREETYPE
			new gui::intlGUIEditBox(text_port.c_str(), true, Environment, this, GUI_ID_PORT_INPUT, rect);
#else
			Environment->addEditBox(text_port.c_str(), rect, false, this, GUI_ID_PORT_INPUT);
#endif
		}
		{
			core::rect<s32> rect(0, 0, 180, 30);
			rect += topleft_content + v2s32(80, 160);
			Environment->addButton(rect, this, GUI_ID_ADDFAV_BUTTON, narrow_to_wide(gettext("Add to Favourites")).c_str());
		}
		{
			core::rect<s32> rect(0, 0, 180, 30);
			rect += topleft_content + v2s32(270, 160);
			Environment->addButton(rect, this, GUI_ID_CONNECT_BUTTON, narrow_to_wide(gettext("Connect")).c_str());
		}
	}else if (m_data.selected_tab == TAB_MP_CONNECT) {
		// Nickname + password
		{
			core::rect<s32> rect(0, 0, 110, 20);
			rect += topleft_content + v2s32(120, 60);
			Environment->addStaticText(narrow_to_wide(gettext("Name/Password")).c_str(), rect, false, true, this, -1);
		}
		{
			core::rect<s32> rect(0, 0, 230, 30);
			rect += topleft_content + v2s32(135, 90);
#if USE_FREETYPE
			new gui::intlGUIEditBox(text_name.c_str(), true, Environment, this, GUI_ID_NAME_INPUT, rect);
#else
			Environment->addEditBox(text_name.c_str(), rect, false, this, GUI_ID_NAME_INPUT);
#endif
		}
		{
			core::rect<s32> rect(0, 0, 230, 30);
			rect += topleft_content + v2s32(135, 125);
			gui::IGUIEditBox *e;
#if USE_FREETYPE
			e = (gui::IGUIEditBox *) new gui::intlGUIEditBox(L"", true, Environment, this, GUI_ID_PW_INPUT, rect);
#else
			e = Environment->addEditBox(L"", rect, false, this, GUI_ID_PW_INPUT);
#endif
			e->setPasswordBox(true);
			Environment->setFocus(e);

		// Start game button
		{
			core::rect<s32> rect(0, 0, 180, 30);
			rect += topleft_content + v2s32(160, 160);
			Environment->addButton(rect, this, GUI_ID_START_BUTTON, narrow_to_wide(gettext("Join Server")).c_str());
		}
		}
	}else{
		gui::IGUIListBox *box = NULL;
		gui::IGUIStaticText *header = NULL;
		std::vector<ServerInfo> *list = NULL;

		{
			core::rect<s32> rect(0, 0, 550, 20);
			rect += topleft_content + v2s32(0, 30);
			header = Environment->addStaticText(L"", rect, false, true, this, -1);
			header->setTextAlignment(gui::EGUIA_CENTER, gui::EGUIA_UPPERLEFT);
		}
		{
			core::rect<s32> rect(0, 0, 350, 200);
			rect += topleft_content + v2s32(100, 60);
			box = Environment->addListBox(rect, this, GUI_ID_SERVER_LIST, true);
			box->setItemHeight(25);
		}

		if (m_data.selected_tab == TAB_MP_FAVOURITES) {
			header->setText(narrow_to_wide(gettext("My Favourites")).c_str());
			list = &m_data.favourites;
		}else{
			header->setText(narrow_to_wide(gettext("Server List")).c_str());
			list = &m_data.servers;
			{
				core::rect<s32> rect(0, 0, 180, 25);
				rect += topleft_content + v2s32(270, 260);
				Environment->addButton(rect, this, GUI_ID_REFRESH_BUTTON, narrow_to_wide(gettext("Get New List")).c_str());
			}
		}

		for (std::vector<ServerInfo>::iterator i = list->begin(); i != list->end(); i++) {
			box->addItem(i->name.c_str());
		}

		if (m_data.selected_row > -1) {
			ServerInfo info = list->at(m_data.selected_row);
			if (info.name == L"") {
				m_data.selected_row = -1;
			}else{
				box->setSelected(m_data.selected_row);
				{
					core::rect<s32> rect(0, 0, 550, 20);
					rect += topleft_content + v2s32(0, 300);
					gui::IGUIStaticText *t = Environment->addStaticText(info.name.c_str(), rect, false, true, this, -1);
					t->setTextAlignment(gui::EGUIA_CENTER, gui::EGUIA_UPPERLEFT);
				}
				{
					core::rect<s32> rect(0, 0, 350, 20);
					rect += topleft_content + v2s32(100, 330);
					gui::IGUIStaticText *t = Environment->addStaticText(info.addr.c_str(), rect, false, true, this, -1);
					t->setTextAlignment(gui::EGUIA_UPPERLEFT, gui::EGUIA_UPPERLEFT);
				}
				{
					core::rect<s32> rect(0, 0, 350, 20);
					rect += topleft_content + v2s32(100, 350);
					gui::IGUIStaticText *t = Environment->addStaticText(info.mode.c_str(), rect, false, true, this, -1);
					t->setTextAlignment(gui::EGUIA_UPPERLEFT, gui::EGUIA_UPPERLEFT);
				}

				if (info.is_favourite) {
					{
						core::rect<s32> rect(0, 0, 180, 30);
						rect += topleft_content + v2s32(80, 390);
						Environment->addButton(rect, this, GUI_ID_REMFAV_BUTTON, narrow_to_wide(gettext("Remove from Favourites")).c_str());
					}
				}else{
					{
						core::rect<s32> rect(0, 0, 180, 30);
						rect += topleft_content + v2s32(80, 390);
						Environment->addButton(rect, this, GUI_ID_ADDFAV_BUTTON, narrow_to_wide(gettext("Add to Favourites")).c_str());
					}
				}

				{
					core::rect<s32> rect(0, 0, 180, 30);
					rect += topleft_content + v2s32(270, 390);
					Environment->addButton(rect, this, GUI_ID_CONNECT_BUTTON, narrow_to_wide(gettext("Connect")).c_str());
				}
			}
		}
	}
}

void GUIMultiplayerMenu::drawMenu()
{
	video::IVideoDriver* driver = Environment->getVideoDriver();

	{
		char buff[1024];
		core::rect<s32> rect(
			0,
			0,
			m_screensize.X,
			m_screensize.Y
		);
		driver->draw2DRectangle(rect, GUI_BG_TOP, GUI_BG_BTM, GUI_BG_TOP, GUI_BG_BTM, NULL);

		if (path_get((char*)"texture",(char*)"menulogo.png",1,buff,1024)) {
			video::ITexture *texture = driver->getTexture(buff);
			if (texture != 0) {
				const core::dimension2d<u32>& img_origsize = texture->getOriginalSize();
				core::rect<s32> logo(
					AbsoluteRect.UpperLeftCorner.X+25,
					AbsoluteRect.UpperLeftCorner.Y,
					AbsoluteRect.UpperLeftCorner.X+225,
					AbsoluteRect.UpperLeftCorner.Y+200
				);
				const video::SColor color(255,255,255,255);
				const video::SColor colors[] = {color,color,color,color};
				driver->draw2DImage(texture, logo, core::rect<s32>(core::position2d<s32>(0,0),img_origsize), NULL, colors, true);
			}
		}
	}

	gui::IGUIElement::draw();
}

bool GUIMultiplayerMenu::acceptInput()
{
	{
		gui::IGUIElement *e = getElementFromId(GUI_ID_NAME_INPUT);
		if (e != NULL)
			m_data.mmdata->name = e->getText();
		if (m_data.mmdata->name == L"")
			m_data.mmdata->name = narrow_to_wide(porting::getUser());
	}
	{
		gui::IGUIElement *e = getElementFromId(GUI_ID_PW_INPUT);
		if (e != NULL)
			m_data.mmdata->password = e->getText();
	}
	{
		gui::IGUIElement *e = getElementFromId(GUI_ID_ADDRESS_INPUT);
		if (e != NULL)
			m_data.mmdata->address = e->getText();
	}
	{
		gui::IGUIElement *e = getElementFromId(GUI_ID_PORT_INPUT);
		if (e != NULL)
			m_data.mmdata->port = e->getText();
	}
	{
		gui::IGUIListBox *e = (gui::IGUIListBox*)getElementFromId(GUI_ID_SERVER_LIST);
		if (e != NULL)
			m_data.selected_row = e->getSelected();
		if (m_data.selected_row > -1) {
			if (m_data.selected_tab == TAB_MP_LIST) {
				ServerInfo info = m_data.servers.at(m_data.selected_row);
				if (info.name == L"")
					m_data.selected_row = -1;
				WStrfnd sf(info.addr);
				m_data.mmdata->address = sf.next(L":");
				m_data.mmdata->port = sf.end();
			}else if (m_data.selected_tab == TAB_MP_FAVOURITES) {
				ServerInfo info = m_data.favourites.at(m_data.selected_row);
				if (info.name == L"")
					m_data.selected_row = -1;
				WStrfnd sf(info.addr);
				m_data.mmdata->address = sf.next(L":");
				m_data.mmdata->port = sf.end();
			}else if (m_data.selected_tab != TAB_MP_CONNECT) {
				m_data.selected_row = -1;
			}
		}
	}
	return true;
}
bool GUIMultiplayerMenu::OnEvent(const SEvent& event)
{
	if (event.EventType==EET_KEY_INPUT_EVENT) {
		if (
			event.KeyInput.PressedDown
			&& (
				event.KeyInput.Key == KEY_ESCAPE
				|| event.KeyInput.Key == KEY_RETURN
			)
		) {
			acceptInput();
			quitMenu();
			return true;
		}
	}
	if (event.EventType == EET_GUI_EVENT) {
		if (event.GUIEvent.EventType == gui::EGET_ELEMENT_FOCUS_LOST && isVisible()) {
			if (!canTakeFocus(event.GUIEvent.Element)) {
				dstream << "GUIMainMenu: Not allowing focus change."
						<< std::endl;
				// Returning true disables focus change
				return true;
			}
		}
		if (event.GUIEvent.EventType==gui::EGET_CHECKBOX_CHANGED) {
			acceptInput();
			m_accepted = false;
			regenerateGui(m_screensize);
		}
		if (event.GUIEvent.EventType == gui::EGET_BUTTON_CLICKED) {
			switch (event.GUIEvent.Caller->getID()) {
			case GUI_ID_START_BUTTON: // Start game
				acceptInput();
				m_accepted = false;
				m_gamecallback->startGame();
				quitMenu();
				return true;
			case GUI_ID_CONNECT_BUTTON:
				acceptInput();
				m_data.selected_tab = TAB_MP_CONNECT;
				regenerateGui(m_screensize);
				return true;
			case GUI_ID_REMFAV_BUTTON:
				acceptInput();
				if (m_data.selected_row > -1) {
					if (m_data.selected_tab == TAB_MP_LIST) {
						ServerInfo &info = m_data.servers.at(m_data.selected_row);
						if (info.is_favourite) {
							info.is_favourite = false;
							for (
								std::vector<ServerInfo>::iterator i = m_data.favourites.begin();
								i != m_data.favourites.end();
								i++
							) {
								if (
									i->name == info.name
									&& i->addr == info.addr
									&& i->mode == info.mode
								) {
									m_data.favourites.erase(i);
									break;
								}
							}
							saveFavourites();
						}
					}else if (m_data.selected_tab == TAB_MP_FAVOURITES) {
						ServerInfo &info = m_data.favourites.at(m_data.selected_row);
						if (info.is_favourite) {
							m_data.favourites.erase(m_data.favourites.begin()+m_data.selected_row);
							for (
								std::vector<ServerInfo>::iterator i = m_data.servers.begin();
								i != m_data.servers.end();
								i++
							) {
								if (
									i->name == info.name
									&& i->addr == info.addr
									&& i->mode == info.mode
								) {
									i->is_favourite = false;
									break;
								}
							}
							saveFavourites();
							m_data.selected_row = -1;
						}
					}
				}
				regenerateGui(m_screensize);
				return true;
			case GUI_ID_ADDFAV_BUTTON:
				acceptInput();
				if (m_data.selected_tab == TAB_MP_LIST && m_data.selected_row > -1) {
					ServerInfo &info = m_data.servers.at(m_data.selected_row);
					if (info.name != L"" && !info.is_favourite) {
						info.is_favourite = true;
						m_data.favourites.push_back(info);
						saveFavourites();
					}
				}else if (m_data.selected_tab == TAB_MP_CUSTOM) {
					ServerInfo info;
					info.name = m_data.mmdata->address+L":"+m_data.mmdata->port;
					info.addr = m_data.mmdata->address+L":"+m_data.mmdata->port;
					info.mode = L"custom";
					info.is_favourite = true;
					m_data.servers.push_back(info);
					m_data.favourites.push_back(info);
					m_data.selected_tab = TAB_MP_FAVOURITES;
					m_data.selected_row = m_data.favourites.size()-1;
				}
				regenerateGui(m_screensize);
				return true;
			case GUI_ID_REFRESH_BUTTON:
				acceptInput();
				fetchServers();
				if (
					(
						m_data.selected_tab == TAB_MP_LIST
						&& m_data.selected_row >= (int)m_data.servers.size()
					) || (
						m_data.selected_tab == TAB_MP_FAVOURITES
						&& m_data.selected_row >= (int)m_data.favourites.size()
					)
				) {
					m_data.selected_row = -1;
				}
				regenerateGui(m_screensize);
				return true;
			case GUI_ID_TAB_MP_LIST:
				acceptInput();
				m_accepted = false;
				if (m_data.selected_tab != TAB_MP_LIST) {
					m_data.selected_tab = TAB_MP_LIST;
					m_data.selected_row = -1;
				}
				regenerateGui(m_screensize);
				return true;
			case GUI_ID_TAB_MP_FAVOURITES:
				acceptInput();
				m_accepted = false;
				if (m_data.selected_tab != TAB_MP_FAVOURITES) {
					m_data.selected_tab = TAB_MP_FAVOURITES;
					m_data.selected_row = -1;
				}
				regenerateGui(m_screensize);
				return true;
			case GUI_ID_TAB_MP_CUSTOM:
				acceptInput();
				m_accepted = false;
				m_data.selected_tab = TAB_MP_CUSTOM;
				regenerateGui(m_screensize);
				return true;
			case GUI_ID_TAB_MP_MAINMENU: //back
				acceptInput();
				quitMenu();
				return true;
			}
		}
		if (event.GUIEvent.EventType == gui::EGET_EDITBOX_ENTER) {
			switch (event.GUIEvent.Caller->getID()) {
			case GUI_ID_ADDRESS_INPUT:
			case GUI_ID_PORT_INPUT:
			case GUI_ID_NAME_INPUT:
			case GUI_ID_PW_INPUT:
				m_gamecallback->startGame();
				acceptInput();
				quitMenu();
				return true;
			}
		}
		if (
			event.GUIEvent.EventType == gui::EGET_LISTBOX_CHANGED
			|| event.GUIEvent.EventType == gui::EGET_LISTBOX_SELECTED_AGAIN
		) {
			if (event.GUIEvent.Caller->getID() == GUI_ID_SERVER_LIST) {
				acceptInput();
				regenerateGui(m_screensize);
				return true;
			}
		}
	}
	return Parent ? Parent->OnEvent(event) : false;
}

bool GUIMultiplayerMenu::fetchServers()
{
	char buff[1024];
	char* u = const_cast<char*>("/list");
	std::string data = http_request(NULL,u);

	if (!path_get((char*)"config",(char*)"servers.txt",0,buff,1024))
		return false;

	std::ofstream f;
	f.open(buff);
	if (!f.is_open())
		return false;

	f << data;
	f.close();

	return loadServers();
}
bool GUIMultiplayerMenu::loadServers()
{
	char buff[1024];

	if (path_get((char*)"config",(char*)"servers.txt",0,buff,1024))
		parseFile(buff,m_data.servers);

	if (path_get((char*)"config",(char*)"favourites.txt",0,buff,1024))
		parseFile(buff,m_data.favourites);

	if (m_data.favourites.size()) {
		for (
			std::vector<ServerInfo>::iterator k = m_data.favourites.begin();
			k != m_data.favourites.end();
			k++
		) {
			bool add = true;
			k->is_favourite = true;
			for (
				std::vector<ServerInfo>::iterator i = m_data.servers.begin();
				i != m_data.servers.end();
				i++
			) {
				if (
					i->name == k->name
					&& i->addr == k->addr
					&& i->mode == k->mode
				) {
					i->is_favourite = true;
					add = false;
					break;
				}
			}
			if (add)
				m_data.servers.push_back(*k);
		}
	}

	return true;
}
bool GUIMultiplayerMenu::saveFavourites()
{
	char buff[1024];
	if (!path_get((char*)"config",(char*)"favourites.txt",0,buff,1024))
		return false;

	std::ofstream f;
	f.open(buff);
	if (!f.is_open())
		return false;

	f << "servers: ";
	f << m_data.favourites.size();
	f << "\n\n";

	for (
		std::vector<ServerInfo>::iterator k = m_data.favourites.begin();
		k != m_data.favourites.end();
		k++
	) {
		f << wide_to_narrow(k->name) << '\n';
		f << wide_to_narrow(k->mode) << '\n';
		f << wide_to_narrow(k->addr) << '\n';
		f << '\n';
	}

	f.close();
	return false;
}
bool GUIMultiplayerMenu::parseFile(std::string path, std::vector<ServerInfo> &list)
{
	list.clear();
	std::ifstream f(path.c_str());
	if (!f.is_open())
		return false;

	std::string line;
	std::getline(f,line);
	if (line.substr(0,9) != "servers: ") {
		f.close();
		return false;
	}
	std::getline(f,line);

	while (true) {
		ServerInfo i;
		i.is_favourite = false;
		std::getline(f,line);
		if (line == "")
			break;
		i.name = narrow_to_wide(line);
		std::getline(f,line);
		if (line == "")
			break;
		i.mode = narrow_to_wide(line);
		std::getline(f,line);
		if (line == "")
			break;
		i.addr = narrow_to_wide(line);
		std::getline(f,line);
		list.push_back(i);
	}

	f.close();
	return true;
}
