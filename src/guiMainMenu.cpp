/************************************************************************
* Minetest-c55
* Copyright (C) 2010-2011 celeron55, Perttu Ahola <celeron55@gmail.com>
*
* guiMainMenu.cpp
* voxelands - 3d voxel world sandbox game
* Copyright (C) Lisa 'darkrose' Milne 2013-2014 <lisa@ltmnet.com>
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

#include "guiMainMenu.h"
#include "main.h"
#include "guiSettingsMenu.h"
#include "guiMultiplayerMenu.h"
#include "guiSingleplayerMenu.h"
#include "debug.h"
#include "serialization.h"
#include <string>
#include <IGUICheckBox.h>
#include <IGUIEditBox.h>
#include <IGUIComboBox.h>
#include <IGUIButton.h>
#include <IGUIStaticText.h>
#include <IGUIFont.h>
#include "path.h"
#include "porting.h"
#include "gui_colours.h"
#if USE_FREETYPE
#include "intlGUIEditBox.h"
#endif
#include "sound.h"

#include "intl.h"

GUIMainMenu::GUIMainMenu(gui::IGUIEnvironment* env,
		gui::IGUIElement* parent, s32 id,
		IMenuManager *menumgr,
		MainMenuData *data,
		IGameCallback *gamecallback
):
	GUIModalMenu(env, parent, id, menumgr),
	m_data(data),
	m_accepted(false),
	m_gamecallback(gamecallback)
{
	assert(m_data);
	this->env = env;
	this->parent = parent;
	this->id = id;
	this->menumgr = menumgr;
}

GUIMainMenu::~GUIMainMenu()
{
	removeChildren();
}

void GUIMainMenu::removeChildren()
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

void GUIMainMenu::regenerateGui(v2u32 screensize)
{
	std::wstring game_mode;

	m_screensize = screensize;

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

	// Character Creator button
	{
		core::rect<s32> rect(0, 0, 200, 40);
		rect += v2s32(25, 200);
		Environment->addButton(rect, this, GUI_ID_CHARACTER_CREATOR, narrow_to_wide(gettext("Character Creator")).c_str());
	}
	// Single Player button
	{
		core::rect<s32> rect(0, 0, 200, 40);
		rect += v2s32(25, 260);
		Environment->addButton(rect, this, GUI_ID_TAB_SINGLEPLAYER, narrow_to_wide(gettext("Single Player")).c_str());
	}
	// Multi Player button
	{
		core::rect<s32> rect(0, 0, 200, 40);
		rect += v2s32(25, 305);
		Environment->addButton(rect, this, GUI_ID_TAB_MULTIPLAYER, narrow_to_wide(gettext("Multi Player")).c_str());
	}
	// Settings button
	{
		core::rect<s32> rect(0, 0, 180, 40);
		rect += v2s32(35, 350);
		Environment->addButton(rect, this, GUI_ID_TAB_SETTINGS, narrow_to_wide(gettext("Settings")).c_str());
	}
	// Credits button
	{
		core::rect<s32> rect(0, 0, 180, 40);
		rect += v2s32(35, 395);
		Environment->addButton(rect, this, GUI_ID_TAB_CREDITS, narrow_to_wide(gettext("Credits")).c_str());
	}
	// Quit button
	{
		core::rect<s32> rect(0, 0, 180, 40);
		rect += v2s32(35, 440);
		Environment->addButton(rect, this, GUI_ID_TAB_QUIT, narrow_to_wide(gettext("Quit")).c_str());
	}

	v2s32 topleft_content(250, 0);

	//v2s32 size = rect.getSize();

	/*
		Add stuff
	*/

	/*
		Client section
	*/
	char* selected_tab = config_get("client.ui.mainmenu.tab");
	if (!selected_tab) {
		m_data->selected_tab = TAB_SINGLEPLAYER;
	}else if (!strcmp(selected_tab,"multiplayer")) {
		m_data->selected_tab = TAB_MULTIPLAYER;
	}else if (!strcmp(selected_tab,"settings")) {
		m_data->selected_tab = TAB_SETTINGS;
	}else if (!strcmp(selected_tab,"credits")) {
		m_data->selected_tab = TAB_CREDITS;
	}else{
		m_data->selected_tab = TAB_SINGLEPLAYER;
	}

	if (m_data->selected_tab == TAB_MULTIPLAYER) {
		GUIMultiplayerMenu *mmenu = new GUIMultiplayerMenu(env, parent, -1,m_data,menumgr,m_gamecallback);
		mmenu->drop();
	}else if (m_data->selected_tab == TAB_SINGLEPLAYER) {
		GUISingleplayerMenu *mmenu = new GUISingleplayerMenu(env, parent, -1,menumgr,m_gamecallback);
		mmenu->drop();
	}else if (m_data->selected_tab == TAB_CREDITS) {
		// CREDITS
		{
			core::rect<s32> rect(0, 0, 550, 550);
			rect += topleft_content + v2s32(0, 0);
			std::string txt("");

			txt += gettext(
				"Voxelands\n"
				"http://www.voxelands.com/\n"
				"By Lisa 'darkrose' Milne <lisa@ltmnet.com> and contributors."
			);

			txt += "\n\n";

			txt += gettext("Programmers");
			txt += "\ndarkrose, sdzen, Pentium44, Menche, MavJS, tiemay, Tapoky, JHeaton, Wayward_One, Stephane, hdastwb, VargaD, GagaPete, Nascher.\n\n";

			txt += gettext("Artists and Modellers");
			txt += "\ndarkrose, sdzen, Menche, Pentium44, Wuzzy, Carl 'melkior' Kidwell, Jordach, Gwinna, elky, p0ss, GagaPete, nic96.\n\n";

			txt += gettext("Translators");
			txt += "\ndarkrose, Szkodnix, soognarf, Wuzzy, Akien, redhound, Rui, loh, wario, puma_rc, Uniaika, meldrian, vahe.\n\n";

			txt += gettext("Documentation Writers");
			txt += "\ndarkrose, Menche, Pentium44, elky, cambam, meldrian, comrad.\n\n";

			txt += gettext("Testers, Packagers, and Builders");
			txt += "\ndarkrose, sdzen, Menche, tiemay, JHeaton, MavJS, mcnalu.\n\n";

			txt += gettext("Music and Sound Effects Composers");
			txt += "\ndarkrose, Jordach, AudioRichter, Tom Peter, OwlStorm, DjDust, Taira Komori.\n\n";

			txt += gettext("Other Contributers, and Special Thanks");
			txt += "\nnadnadnad, Honeypaw, tiemay, stormchaser3000, MichaelEh?, NCC74656.\n\n";

			txt += gettext("Patreon Supporters");
			txt += "\nLerura.\n\n";

			txt += gettext(
				"Based on Minetest-C55 by Perttu Ahola <celeron55@gmail.com>\n"
				"and contributors: PilzAdam, Taoki, tango_, kahrl (kaaaaaahrl?), darkrose, matttpt, erlehmann,"
				" SpeedProg, JacobF, teddydestodes, marktraceur, Jonathan Neuschafer, thexyz, VanessaE, sfan5..."
				" and tens of more random people."
			);
			gui::IGUIStaticText *t = Environment->addStaticText(narrow_to_wide(txt).c_str(),rect,false,true,this,-1);
			t->setTextAlignment(gui::EGUIA_CENTER, gui::EGUIA_UPPERLEFT);
		}
	}
}

void GUIMainMenu::drawMenu()
{
	gui::IGUISkin* skin = Environment->getSkin();
	if (!skin)
		return;
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

void GUIMainMenu::acceptInput()
{
	if (m_data->name == L"")
		m_data->name = std::wstring(L"singleplayer");
	m_data->password = L"";
	m_data->address = L"";

	m_accepted = true;
}

bool GUIMainMenu::OnEvent(const SEvent& event)
{
	if(event.EventType==EET_KEY_INPUT_EVENT)
	{
		if(event.KeyInput.Key==KEY_ESCAPE && event.KeyInput.PressedDown)
		{
			m_gamecallback->exitToOS();
			quitMenu();
			return true;
		}
		if(event.KeyInput.Key==KEY_RETURN && event.KeyInput.PressedDown)
		{
			acceptInput();
			quitMenu();
			return true;
		}
	}
	if (event.EventType==EET_GUI_EVENT) {
		if (event.GUIEvent.EventType==gui::EGET_ELEMENT_FOCUS_LOST && isVisible()) {
			if (!canTakeFocus(event.GUIEvent.Element)) {
				dstream<<"GUIMainMenu: Not allowing focus change."
						<<std::endl;
				// Returning true disables focus change
				return true;
			}
		}
		if (event.GUIEvent.EventType==gui::EGET_BUTTON_CLICKED) {
			switch (event.GUIEvent.Caller->getID()) {
			case GUI_ID_CHARACTER_CREATOR:
			{
				acceptInput();
				m_data->character_creator = true;
				quitMenu();
				return true;
			}
			case GUI_ID_TAB_SINGLEPLAYER:
			{
				if (m_data->selected_tab == TAB_SETTINGS)
					acceptInput();
				GUISingleplayerMenu *mmenu = new GUISingleplayerMenu(env, parent, -1,menumgr,m_gamecallback);
				mmenu->drop();
				m_data->selected_tab = TAB_SINGLEPLAYER;
				config_set("client.ui.mainmenu.tab","singleplayer");
				return true;
			}
			case GUI_ID_TAB_MULTIPLAYER:
			{
				if (m_data->selected_tab == TAB_SETTINGS)
					acceptInput();
				GUIMultiplayerMenu *mmenu = new GUIMultiplayerMenu(env, parent, -1,m_data,menumgr,m_gamecallback);
				mmenu->drop();
				m_data->selected_tab = TAB_MULTIPLAYER;
				config_set("client.ui.mainmenu.tab","multiplayer");
				return true;
			}
			case GUI_ID_TAB_SETTINGS:
			{
				GUISettingsMenu *smenu = new GUISettingsMenu(env, parent, -1,menumgr, false);
				smenu->drop();
				return true;
			}
			case GUI_ID_TAB_CREDITS:
				if (m_data->selected_tab == TAB_SETTINGS)
					acceptInput();
				m_accepted = false;
				m_data->selected_tab = TAB_CREDITS;
				config_set("client.ui.mainmenu.tab","credits");
				regenerateGui(m_screensize);
				return true;
			case GUI_ID_TAB_QUIT:
				m_gamecallback->exitToOS();
				quitMenu();
				return true;
			}
		}
		if (event.GUIEvent.EventType==gui::EGET_EDITBOX_ENTER) {
			switch (event.GUIEvent.Caller->getID()) {
			case GUI_ID_ADDRESS_INPUT:
			case GUI_ID_PORT_INPUT:
			case GUI_ID_NAME_INPUT:
			case GUI_ID_PW_INPUT:
				acceptInput();
				quitMenu();
				return true;
			}
		}
	}

	return Parent ? Parent->OnEvent(event) : false;
}
