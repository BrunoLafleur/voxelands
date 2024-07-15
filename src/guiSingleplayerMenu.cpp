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

#include "guiSingleplayerMenu.h"
#include "guiSettingsMenu.h"
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

GUISingleplayerMenu::GUISingleplayerMenu(
	gui::IGUIEnvironment* env,
	gui::IGUIElement*
	parent,
	s32 id,
	IMenuManager *menumgr,
	IGameCallback *gamecallback
):
	GUIModalMenu(env, parent, id, menumgr),
	m_accepted(false),
	config(0),
	selected_row(-1),
	m_gamecallback(gamecallback)
{
	this->env = env;
	this->parent = parent;
	this->menumgr = menumgr;

	worlds = NULL;
	selected[0] = 0;
}

GUISingleplayerMenu::~GUISingleplayerMenu()
{
	if (worlds)
		world_list_free(worlds);
	removeChildren();
}

void GUISingleplayerMenu::removeChildren()
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

void GUISingleplayerMenu::regenerateGui(v2u32 screensize)
{
	m_screensize = screensize;
	{
		gui::IGUIElement *e = getElementFromId(GUI_ID_SP_WORLD_NAME);
		if (e != NULL) {
			std::wstring w_name = e->getText();
			if (snprintf(selected,256,"%s",wide_to_narrow(w_name).c_str()) >= 256)
				snprintf(selected,256,"New World");
		}
	}
	{
		gui::IGUIElement *e = getElementFromId(GUI_ID_SP_MAP_SEED);
		if (e != NULL) {
			std::wstring w_name = e->getText();
			if (snprintf(fixed_seed,256,"%s",wide_to_narrow(w_name).c_str()) >= 256)
				snprintf(fixed_seed,256,"0");
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
	v2s32 topleft_content(250, 0);
	{
		core::rect<s32> rect(0, 0, 550, 20);
		rect += topleft_content + v2s32(0, 10);
		gui::IGUIStaticText *t = Environment->addStaticText(narrow_to_wide(gettext("Single Player")).c_str(), rect, false, true, this, -1);
		t->setTextAlignment(gui::EGUIA_CENTER, gui::EGUIA_UPPERLEFT);
	}

	if (!config) {
		gui::IGUIListBox *box = NULL;
		// Main Menu button
		{
			core::rect<s32> rect(0, 0, 200, 40);
			rect += v2s32(25, 200);
			Environment->addButton(rect, this, GUI_ID_SP_MAINMENU_BUTTON, narrow_to_wide(gettext("Main Menu")).c_str());
		}
		{
		// Create button
			core::rect<s32> rect(0, 0, 200, 40);
			rect += v2s32(25, 260);
			Environment->addButton(rect, this, GUI_ID_SP_CREATE_BUTTON, narrow_to_wide(gettext("Create New World")).c_str());
		}
		// Edit button
		if (selected_row > -1) {
			{
				core::rect<s32> rect(0, 0, 200, 40);
				rect += v2s32(25, 305);
				Environment->addButton(rect, this, GUI_ID_SP_EDIT_BUTTON, narrow_to_wide(gettext("Edit World")).c_str());
			}
			{
				core::rect<s32> rect(0, 0, 200, 40);
				rect += v2s32(25, 350);
				Environment->addButton(rect, this, GUI_ID_SP_START_BUTTON, narrow_to_wide(gettext("Start Game")).c_str());
			}
		}

		if (!worlds)
			worlds = world_list_get();

		if (!worlds) {
			core::rect<s32> rect(0, 0, 550, 20);
			rect += topleft_content + v2s32(0, 60);
			gui::IGUIStaticText *t = Environment->addStaticText(narrow_to_wide(gettext("No Worlds Found")).c_str(), rect, false, true, this, -1);
			t->setTextAlignment(gui::EGUIA_CENTER, gui::EGUIA_UPPERLEFT);
		}else{
			worldlist_t *w;
			worldlist_t *s = NULL;

			core::rect<s32> rect(0, 0, 450, 400);
			rect += topleft_content + v2s32(50, 60);
			box = Environment->addListBox(rect, this, GUI_ID_WORLD_LIST, true);
			box->setItemHeight(25);

			w = worlds;
			for (int i=0; w; i++) {
				box->addItem(narrow_to_wide(w->name).c_str());
				if (i == selected_row)
					s = w;
				w = w->next;
			}

			if (s) {
				if (snprintf(selected,256,"%s",s->path) >= 256)
					selected[0] = 0;
				vlprintf(CN_INFO,"Selected: '%s'",selected);
				box->setSelected(selected_row);
				{
					core::rect<s32> rect(0, 0, 550, 20);
					rect += topleft_content + v2s32(0, 480);
					gui::IGUIStaticText *t = Environment->addStaticText(narrow_to_wide(s->name).c_str(), rect, false, true, this, -1);
					t->setTextAlignment(gui::EGUIA_CENTER, gui::EGUIA_UPPERLEFT);
				}
				{
					core::rect<s32> rect(0, 0, 350, 20);
					rect += topleft_content + v2s32(100, 510);
					gui::IGUIStaticText *t = Environment->addStaticText(narrow_to_wide(s->mode).c_str(), rect, false, true, this, -1);
					t->setTextAlignment(gui::EGUIA_UPPERLEFT, gui::EGUIA_UPPERLEFT);
				}
				if (s->version) {
					core::rect<s32> rect(0, 0, 350, 20);
					rect += topleft_content + v2s32(100, 510);
					gui::IGUIStaticText *t = Environment->addStaticText(narrow_to_wide(s->version).c_str(), rect, false, true, this, -1);
					t->setTextAlignment(gui::EGUIA_LOWERRIGHT, gui::EGUIA_UPPERLEFT);
				}
			}
		}
	/* import */
	}else if (config == 3) {
	}else{
		// Save button
		{
			core::rect<s32> rect(0, 0, 200, 40);
			rect += v2s32(25, 260);
			Environment->addButton(rect, this, GUI_ID_SP_SAVE_BUTTON, narrow_to_wide(gettext("Save")).c_str());
		}
		// Cancel button
		{
			core::rect<s32> rect(0, 0, 200, 40);
			rect += v2s32(25, 305);
			Environment->addButton(rect, this, GUI_ID_SP_CANCEL_BUTTON, narrow_to_wide(gettext("Cancel")).c_str());
		}
		/* create */
		if (config == 1) {
			{
				core::rect<s32> rect(0, 0, 550, 20);
				rect += topleft_content + v2s32(0, 40);
				gui::IGUIStaticText *t = Environment->addStaticText(
					narrow_to_wide(gettext("Create New World")).c_str(),
					rect,
					false,
					true,
					this,
					-1
				);
				t->setTextAlignment(gui::EGUIA_CENTER, gui::EGUIA_UPPERLEFT);
			}
			// world name
			{
				core::rect<s32> rect(0, 0, 110, 20);
				rect += topleft_content + v2s32(120, 70);
				Environment->addStaticText(narrow_to_wide(gettext("World Name:")).c_str(), rect, false, true, this, -1);
			}
			{
				core::rect<s32> rect(0, 0, 230, 30);
				rect += topleft_content + v2s32(135, 90);
#if USE_FREETYPE
				new gui::intlGUIEditBox(narrow_to_wide(selected).c_str(), true, Environment, this, GUI_ID_SP_WORLD_NAME, rect);
#else
				Environment->addEditBox(narrow_to_wide(selected).c_str(), rect, false, this, GUI_ID_SP_WORLD_NAME);
#endif
			}
		/* edit */
		}else if (config == 2) {
			{
				core::rect<s32> rect(0, 0, 550, 20);
				rect += topleft_content + v2s32(0, 40);
				gui::IGUIStaticText *t = Environment->addStaticText(
					narrow_to_wide(gettext("Edit World")).c_str(),
					rect,
					false,
					true,
					this,
					-1
				);
				t->setTextAlignment(gui::EGUIA_CENTER, gui::EGUIA_UPPERLEFT);
			}
			{
				core::rect<s32> rect(0, 0, 550, 20);
				rect += topleft_content + v2s32(0, 70);
				gui::IGUIStaticText *t = Environment->addStaticText(
					narrow_to_wide(selected).c_str(),
					rect,
					false,
					true,
					this,
					-1
				);
				t->setTextAlignment(gui::EGUIA_CENTER, gui::EGUIA_UPPERLEFT);
			}
		}
		{
			core::rect<s32> rect(0, 0, 200, 30);
			rect += topleft_content + v2s32(80, 160);
			Environment->addCheckBox(
				survival_mode,
				rect,
				this,
				GUI_ID_SP_MODE_SURVIVAL,
				narrow_to_wide(gettext("Survival Mode")).c_str()
			);
		}
		{
			core::rect<s32> rect(0, 0, 200, 30);
			rect += topleft_content + v2s32(80, 200);
			Environment->addCheckBox(
				!survival_mode,
				rect,
				this,
				GUI_ID_SP_MODE_CREATIVE,
				narrow_to_wide(gettext("Creative Mode")).c_str()
			);
		}
		{
			core::rect<s32> rect(0, 0, 200, 30);
			rect += topleft_content + v2s32(80, 260);
			Environment->addCheckBox(
				flat_map,
				rect,
				this,
				GUI_ID_SP_MAP_FLAT,
				narrow_to_wide(gettext("Flat Mapgen")).c_str()
			);
		}
		{
			core::rect<s32> rect(0, 0, 200, 30);
			rect += topleft_content + v2s32(80, 300);
			Environment->addCheckBox(
				random_seed,
				rect,
				this,
				GUI_ID_SP_MAP_RANDOM_SEED,
				narrow_to_wide(gettext("Use Random Seed")).c_str()
			);
		}
		if (!random_seed) {
			// seed
			{
				core::rect<s32> rect(0, 0, 110, 20);
				rect += topleft_content + v2s32(120, 330);
				Environment->addStaticText(narrow_to_wide(gettext("World Seed:")).c_str(), rect, false, true, this, -1);
			}
			{
				core::rect<s32> rect(0, 0, 230, 30);
				rect += topleft_content + v2s32(135, 350);
#if USE_FREETYPE
				new gui::intlGUIEditBox(narrow_to_wide(fixed_seed).c_str(), true, Environment, this, GUI_ID_SP_MAP_SEED, rect);
#else
				Environment->addEditBox(narrow_to_wide(fixed_seed).c_str(), rect, false, this, GUI_ID_SP_MAP_SEED);
#endif
			}
		}
	}
}

void GUISingleplayerMenu::drawMenu()
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
bool GUISingleplayerMenu::OnEvent(const SEvent& event)
{
	if (event.EventType==EET_KEY_INPUT_EVENT) {
		if (
			event.KeyInput.PressedDown
			&& (
				event.KeyInput.Key == KEY_ESCAPE
				|| event.KeyInput.Key == KEY_RETURN
			)
		) {
			quitMenu();
			return true;
		}
	}
	if (event.EventType == EET_GUI_EVENT) {
		if (event.GUIEvent.EventType == gui::EGET_ELEMENT_FOCUS_LOST && isVisible()) {
			{
				gui::IGUIElement *e = getElementFromId(GUI_ID_SP_WORLD_NAME);
				if (e != NULL) {
					std::wstring w_name = e->getText();
					if (snprintf(selected,256,"%s",wide_to_narrow(w_name).c_str()) >= 256)
						snprintf(selected,256,"New World");
				}
			}
			{
				gui::IGUIElement *e = getElementFromId(GUI_ID_SP_MAP_SEED);
				if (e != NULL) {
					std::wstring w_name = e->getText();
					if (snprintf(fixed_seed,256,"%s",wide_to_narrow(w_name).c_str()) >= 256)
						snprintf(fixed_seed,256,"0");
				}
			}
			if (!canTakeFocus(event.GUIEvent.Element)) {
				// Returning true disables focus change
				return true;
			}
		}
		if (event.GUIEvent.EventType==gui::EGET_CHECKBOX_CHANGED) {
			gui::IGUICheckBox *e = (gui::IGUICheckBox*)event.GUIEvent.Caller;
			if (e) {
				switch (event.GUIEvent.Caller->getID()) {
				case GUI_ID_SP_MODE_SURVIVAL:
					survival_mode = e->isChecked();
					break;
				case GUI_ID_SP_MODE_CREATIVE:
					survival_mode = !e->isChecked();
					break;
				case GUI_ID_SP_MAP_FLAT:
					flat_map = e->isChecked();
					break;
				case GUI_ID_SP_MAP_RANDOM_SEED:
					random_seed = e->isChecked();
					break;
				}
			}
			m_accepted = false;
			regenerateGui(m_screensize);
		}
		if (event.GUIEvent.EventType == gui::EGET_BUTTON_CLICKED) {
			switch (event.GUIEvent.Caller->getID()) {
			case GUI_ID_SP_START_BUTTON: // Start game
				if (selected[0] != 0) {
					config_set("client.world",selected);
					m_gamecallback->startGame();
					quitMenu();
				}
				return true;
			case GUI_ID_SP_CREATE_BUTTON:
				selected_row = -1;
				loadWorld(true);
				config = 1;
				regenerateGui(m_screensize);
				return true;
			case GUI_ID_SP_EDIT_BUTTON:
				if (selected[0] != 0) {
					loadWorld(false);
					config = 2;
				}
				regenerateGui(m_screensize);
				return true;
			case GUI_ID_SP_CANCEL_BUTTON:
				config = 0;
				regenerateGui(m_screensize);
				return true;
			case GUI_ID_SP_SAVE_BUTTON:
				if (config == 1 || config == 2) {
					saveWorld((config == 1));
				}
				config = 0;
				if (worlds) {
					world_list_free(worlds);
					worlds = NULL;
				}
				regenerateGui(m_screensize);
				return true;
			case GUI_ID_SP_IMPORT_BUTTON:
				config = 3;
				regenerateGui(m_screensize);
				return true;
			case GUI_ID_SP_MAINMENU_BUTTON: //back
				quitMenu();
				return true;
			}
		}
		if (
			event.GUIEvent.EventType == gui::EGET_LISTBOX_CHANGED
			|| event.GUIEvent.EventType == gui::EGET_LISTBOX_SELECTED_AGAIN
		) {
			if (event.GUIEvent.Caller->getID() == GUI_ID_WORLD_LIST) {
				gui::IGUIListBox *e = (gui::IGUIListBox*)event.GUIEvent.Caller;
				if (e != NULL) {
					selected_row = e->getSelected();
				}else if (selected_row > -1) {
					selected_row = -1;
				}
				selected[0] = 0;
				regenerateGui(m_screensize);
				return true;
			}
		}
	}
	return Parent ? Parent->OnEvent(event) : false;
}
void GUISingleplayerMenu::saveWorld(bool create)
{
	if (create) {
		world_create(selected);
	}else{
		world_load(selected);
	}
	if (survival_mode) {
		config_set("world.game.mode","survival");
	}else{
		config_set("world.game.mode","creative");
	}
	if (random_seed || fixed_seed[0] == 0) {
		config_set("world.seed",NULL);
	}else{
		config_set("world.seed",fixed_seed);
	}

	if (flat_map) {
		config_set("world.map.type","flat");
	}else{
		config_set("world.map.type","default");
	}

	if (random_seed || fixed_seed[0] == 0) {
		config_set("world.seed",NULL);
	}else{
		config_set("world.seed",fixed_seed);
	}

	world_unload();
}
void GUISingleplayerMenu::loadWorld(bool create)
{
	char* v;
	config_clear("world");

	survival_mode = true;
	random_seed = true;
	flat_map = false;
	snprintf(fixed_seed,256,"0");

	if (create) {
		snprintf(selected,256,"New World");
	}else{
		world_load(selected);

		v = config_get("world.game.mode");
		if (v && !strcmp(v,"creative"))
			survival_mode = false;

		v = config_get("world.seed");
		if (v) {
			random_seed = false;
			snprintf(fixed_seed,256,"%s",v);
		}

		v = config_get("world.name");
		if (!v || snprintf(selected,256,"%s",v) >= 256)
			snprintf(selected,256,"New World");

		v = config_get("world.map.type");
		if (v && !strcmp(v,"flat"))
			flat_map = false;

		world_unload();
	}
}
