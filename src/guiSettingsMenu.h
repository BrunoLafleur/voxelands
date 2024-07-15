 /************************************************************************
* Minetest-c55
* Copyright (C) 2010-2011 celeron55, Perttu Ahola <celeron55@gmail.com>
* Copyright (C) 2011 Ciaran Gultnieks <ciaran@ciarang.com>
* Copyright (C) 2011 teddydestodes <derkomtur@schattengang.net>
*
* guiSettingsMenu.h
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

#ifndef GUISETTINGSMENU_HEADER
#define GUISETTINGSMENU_HEADER

#include "common_irrlicht.h"
#include "utility.h"
#include "modalMenu.h"
#include "client.h"
#include "intl.h"
#include "keycode.h"
#include <string>

enum
{
	GUI_ID_BACK_BUTTON = 101,
	GUI_ID_ABORT_BUTTON,
	// graphics
	GUI_ID_MESH_DETAIL_COMBO,
	GUI_ID_MESH_DETAIL_LOW,
	GUI_ID_MESH_DETAIL_MEDIUM,
	GUI_ID_MESH_DETAIL_HIGH,
	GUI_ID_TEXTURE_DETAIL_COMBO,
	GUI_ID_TEXTURE_DETAIL_LOW,
	GUI_ID_TEXTURE_DETAIL_MEDIUM,
	GUI_ID_TEXTURE_DETAIL_HIGH,
	GUI_ID_LIGHT_DETAIL_COMBO,
	GUI_ID_LIGHT_DETAIL_LOW,
	GUI_ID_LIGHT_DETAIL_MEDIUM,
	GUI_ID_LIGHT_DETAIL_HIGH,
	GUI_ID_HOTBAR_CB,
	GUI_ID_WIELDINDEX_CB,
	GUI_ID_TEXTUREANIM_CB,
	// video
	GUI_ID_MIPMAP_CB,
	GUI_ID_BILINEAR_CB,
	GUI_ID_TRILINEAR_CB,
	GUI_ID_ANISOTROPIC_CB,
	GUI_ID_PARTICLES_CB,
	GUI_ID_FULLSCREEN_CB,
	// sound
	GUI_ID_VOLUME_MASTER_SB,
	GUI_ID_VOLUME_EFFECTS_SB,
	GUI_ID_VOLUME_MUSIC_SB,
	// tabs
	GUI_ID_TAB_MAINMENU,
	GUI_ID_TAB_SETTINGS_CONTROLS,
	GUI_ID_TAB_SETTINGS_GRAPHICS,
	GUI_ID_TAB_SETTINGS_VIDEO,
	GUI_ID_TAB_SETTINGS_SOUND,
	// put new elements above this
	GUI_ID_KEYSETTINGS_BASE = 200
};

enum {
	TAB_SETTINGS_CONTROLS=0,
	TAB_SETTINGS_GRAPHICS,
	TAB_SETTINGS_VIDEO,
	TAB_SETTINGS_SOUND
};

struct SettingsMenuData
{
	SettingsMenuData():
		// Generic
		selected_tab(TAB_SETTINGS_CONTROLS),
		// Client opts
		mesh_detail(3),
		texture_detail(3),
		light_detail(3),
		hotbar(false),
		wield_index(false),
		volume_master(0.0f),
		volume_effects(0.0f),
		volume_music(0.0f),
		particles(true),
		fullscreen(false),
		texture_animation(true)
	{}

	// These are in the native format of the gui elements

	// Generic
	int selected_tab;
	// Client options
	int mesh_detail;
	int texture_detail;
	int light_detail;
	bool mip_map;
	bool anisotropic_filter;
	bool bilinear_filter;
	bool trilinear_filter;
	bool hotbar;
	bool wield_index;
	f32 volume_master;
	f32 volume_effects;
	f32 volume_music;
	//int enable_shaders;
	bool particles;
	bool fullscreen;
	bool texture_animation;
};

class GUISettingsMenu: public GUIModalMenu
{
public:
	GUISettingsMenu(gui::IGUIEnvironment* env, gui::IGUIElement* parent,
			s32 id, IMenuManager *menumgr, bool is_ingame);
	~GUISettingsMenu();

	void removeChildren();
	/*
	 Remove and re-add (or reposition) stuff
	 */
	void regenerateGui(v2u32 screensize);

	void drawMenu();

	bool acceptInput();

	bool OnEvent(const SEvent& event);

private:
	SettingsMenuData m_data;
	bool m_accepted;
	v2u32 m_screensize;
	bool m_is_ingame;

	void init_keys();
	void save();
	bool resetMenu();

	std::wstring keynames[VLKC_MAX];
	s32 activeKey;
	KeyPress keys[VLKC_MAX];
};

#endif

