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

#ifndef GUISINGLEPLAYERMENU_HEADER
#define GUISINGLEPLAYERMENU_HEADER

#include "common.h"

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
	GUI_ID_SP_START_BUTTON = 101,
	GUI_ID_SP_CREATE_BUTTON,
	GUI_ID_SP_EDIT_BUTTON,
	GUI_ID_SP_IMPORT_BUTTON,
	GUI_ID_SP_MAINMENU_BUTTON,
	GUI_ID_SP_SAVE_BUTTON,
	GUI_ID_SP_CANCEL_BUTTON,
	GUI_ID_WORLD_LIST,
	/* create/edit stuff */
	GUI_ID_SP_WORLD_NAME,
	GUI_ID_SP_MODE_SURVIVAL,
	GUI_ID_SP_MODE_CREATIVE,
	GUI_ID_SP_MAP_FLAT,
	GUI_ID_SP_MAP_RANDOM_SEED,
	GUI_ID_SP_MAP_SEED
};

class GUISingleplayerMenu: public GUIModalMenu
{
public:
	GUISingleplayerMenu(
		gui::IGUIEnvironment* env,
		gui::IGUIElement* parent,
		s32 id,
		IMenuManager *menumgr,
		IGameCallback *gamecallback
	);
	~GUISingleplayerMenu();

	void removeChildren();
	/*
	 Remove and re-add (or reposition) stuff
	 */
	void regenerateGui(v2u32 screensize);

	void drawMenu();

	void loadWorld(bool create);
	void saveWorld(bool create);

	bool OnEvent(const SEvent& event);

private:
	char selected[256];
	worldlist_t *worlds;
	bool m_accepted;
	int config;
	int selected_row;
	IGameCallback *m_gamecallback;
	gui::IGUIEnvironment* env;
	gui::IGUIElement* parent;
	IMenuManager *menumgr;
	v2u32 m_screensize;

	bool survival_mode;
	bool random_seed;
	bool flat_map;
	char fixed_seed[256];
};

#endif

