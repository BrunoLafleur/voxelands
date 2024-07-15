 /************************************************************************
* guiMultiplayerMenu.h
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

#ifndef GUIMULTIPLAYERMENU_HEADER
#define GUIMULTIPLAYERMENU_HEADER

#include "common_irrlicht.h"
#include "utility.h"
#include "modalMenu.h"
#include "client.h"
#include "intl.h"
#include "keycode.h"
#include <string>
// For IGameCallback
#include "guiPauseMenu.h"
#include "guiMainMenu.h"

enum
{
	GUI_ID_START_BUTTON = 101,
	GUI_ID_CONNECT_BUTTON,
	// full list/favourites
	GUI_ID_SERVER_LIST,
	GUI_ID_ADDFAV_BUTTON,
	GUI_ID_REMFAV_BUTTON,
	GUI_ID_REFRESH_BUTTON,
	// custom connect
	GUI_ID_NAME_INPUT,
	GUI_ID_PW_INPUT,
	GUI_ID_ADDRESS_INPUT,
	GUI_ID_PORT_INPUT,
	// tabs
	GUI_ID_TAB_MP_MAINMENU,
	GUI_ID_TAB_MP_LIST,
	GUI_ID_TAB_MP_FAVOURITES,
	GUI_ID_TAB_MP_CUSTOM
};

enum {
	TAB_MP_LIST=0,
	TAB_MP_FAVOURITES,
	TAB_MP_CUSTOM,
	TAB_MP_CONNECT
};

struct ServerInfo {
	std::wstring name;
	std::wstring addr;
	std::wstring mode;
	bool is_favourite;
};

struct MultiplayerMenuData
{
	MultiplayerMenuData():
		// Generic
		selected_tab(TAB_MP_LIST),
		selected_row(-1),
		mmdata(NULL)
	{}

	// Generic
	int selected_tab;
	int selected_row;
	// Options
	std::vector<ServerInfo> servers;
	std::vector<ServerInfo> favourites;
	// server address/port/etc is stored in here
	MainMenuData *mmdata;
};

class GUIMultiplayerMenu: public GUIModalMenu
{
public:
	GUIMultiplayerMenu(
		gui::IGUIEnvironment* env,
		gui::IGUIElement* parent,
		s32 id,
		MainMenuData *data,
		IMenuManager *menumgr,
		IGameCallback *gamecallback
	);
	~GUIMultiplayerMenu();

	void removeChildren();
	/*
	 Remove and re-add (or reposition) stuff
	 */
	void regenerateGui(v2u32 screensize);

	void drawMenu();

	bool acceptInput();

	bool OnEvent(const SEvent& event);

private:
	MultiplayerMenuData m_data;
	bool m_accepted;
	IGameCallback *m_gamecallback;
	v2u32 m_screensize;

	bool fetchServers();
	bool loadServers();
	bool saveFavourites();
	bool parseFile(std::string data, std::vector<ServerInfo> &list);
};

#endif

