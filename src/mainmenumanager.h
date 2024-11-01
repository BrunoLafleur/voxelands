/************************************************************************
* Minetest-c55
* Copyright (C) 2010 celeron55, Perttu Ahola <celeron55@gmail.com>
*
* mainmenumanager.h
* voxelands - 3d voxel world sandbox game
* Copyright (C) Lisa 'darkrose' Milne 2014 <lisa@ltmnet.com>
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

#ifndef MAINMENUMANAGER_HEADER
#define MAINMENUMANAGER_HEADER

/*
	All kinds of stuff that needs to be exposed from main.cpp
*/
#include "debug.h" // assert
#include "modalMenu.h"
#include "guiPauseMenu.h" //For IGameCallback

extern gui::IGUIEnvironment* guienv;
extern gui::IGUIStaticText *guiroot;

// Handler for the modal menus

class MainMenuManager : public IMenuManager
{
public:
	virtual void createdMenu(GUIModalMenu *menu)
	{
		for (core::list<GUIModalMenu*>::Iterator i = m_stack.begin(); i != m_stack.end(); i++) {
			assert(*i != menu);
		}

		if (m_stack.size() != 0)
			(*m_stack.getLast())->setVisible(false);
		m_stack.push_back(menu);
	}

	virtual void deletingMenu(GUIModalMenu *menu)
	{
		// Remove all entries if there are duplicates
		bool removed_entry;
		do{
			removed_entry = false;
			for (core::list<GUIModalMenu*>::Iterator i = m_stack.begin(); i != m_stack.end(); i++) {
				if (*i == menu) {
					m_stack.erase(i);
					removed_entry = true;
					break;
				}
			}
		} while(removed_entry);

		if (m_stack.size() != 0)
			(*m_stack.getLast())->setVisible(true);
	}

	u32 menuCount()
	{
		return m_stack.size();
	}

	core::list<GUIModalMenu*> m_stack;
};

extern MainMenuManager g_menumgr;

extern bool noMenuActive();

class MainGameCallback : public IGameCallback
{
public:
	MainGameCallback(IrrlichtDevice *a_device):
		disconnect_requested(false),
		changepassword_requested(false),
		startgame_requested(false),
		device(a_device)
	{
	}

	virtual void exitToOS()
	{
		device->closeDevice();
	}

	virtual void disconnect()
	{
		disconnect_requested = true;
	}

	virtual void changePassword()
	{
		changepassword_requested = true;
	}

	virtual void startGame()
	{
		startgame_requested = true;
	}

	bool disconnect_requested;
	bool changepassword_requested;
	bool startgame_requested;
	IrrlichtDevice *device;
};

extern MainGameCallback *g_gamecallback;

#endif

