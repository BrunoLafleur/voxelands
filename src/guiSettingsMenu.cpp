/************************************************************************
* Minetest-c55
* Copyright (C) 2010-2011 celeron55, Perttu Ahola <celeron55@gmail.com>
* Copyright (C) 2011 Ciaran Gultnieks <ciaran@ciarang.com>
* Copyright (C) 2011 teddydestodes <derkomtur@schattengang.net>
*
* guiSettingsMenu.cpp
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

#include "guiSettingsMenu.h"
#include "debug.h"
#include "serialization.h"
#include "main.h"
#include <string>
#include <IGUICheckBox.h>
#include <IGUIEditBox.h>
#include <IGUIButton.h>
#include <IGUIStaticText.h>
#include <IGUIFont.h>
#include <IGUIScrollBar.h>
#include <IGUIComboBox.h>
#include "gui_colours.h"

GUISettingsMenu::GUISettingsMenu(
	gui::IGUIEnvironment* env,
	gui::IGUIElement*
	parent,
	s32 id,
	IMenuManager *menumgr,
	bool is_ingame
):
	GUIModalMenu(env, parent, id, menumgr),
	m_accepted(false),
	m_is_ingame(is_ingame)
{
	activeKey = -1;
	init_keys();
	m_data.mesh_detail = config_get_int("client.graphics.mesh.lod");
	m_data.texture_detail = config_get_int("client.graphics.texture.lod");
	m_data.light_detail = config_get_int("client.graphics.light.lod");
	m_data.fullscreen = config_get_bool("client.video.fullscreen");
	m_data.particles = config_get_bool("client.graphics.particles");
	m_data.mip_map = config_get_bool("client.video.mipmaps");
	m_data.anisotropic_filter = config_get_bool("client.video.anisotropic");
	m_data.bilinear_filter = config_get_bool("client.video.bilinear");
	m_data.trilinear_filter = config_get_bool("client.video.trilinear");
	m_data.hotbar = config_get_bool("client.ui.hud.old");
	m_data.wield_index = config_get_bool("client.ui.hud.wieldindex");
	m_data.volume_master = config_get_float("client.sound.volume");
	m_data.volume_effects = config_get_float("client.sound.volume.effects");
	m_data.volume_music = config_get_float("client.sound.volume.music");
	m_data.texture_animation = config_get_bool("client.graphics.texture.animations");

	keynames[VLKC_FORWARD] = narrow_to_wide(gettext("Forward"));
	keynames[VLKC_BACKWARD] = narrow_to_wide(gettext("Backward"));
	keynames[VLKC_LEFT] = narrow_to_wide(gettext("Left"));
	keynames[VLKC_RIGHT] = narrow_to_wide(gettext("Right"));
	keynames[VLKC_JUMP] = narrow_to_wide(gettext("Jump"));
	keynames[VLKC_SNEAK] = narrow_to_wide(gettext("Sneak"));
	keynames[VLKC_INVENTORY] = narrow_to_wide(gettext("Inventory"));
	keynames[VLKC_USE] = narrow_to_wide(gettext("Use Item"));
	keynames[VLKC_CHAT] = narrow_to_wide(gettext("Chat"));
	keynames[VLKC_COMMAND] = narrow_to_wide(gettext("Command"));
	keynames[VLKC_RANGE] = narrow_to_wide(gettext("Range Select"));
	keynames[VLKC_FREEMOVE] = narrow_to_wide(gettext("Toggle Fly"));
	keynames[VLKC_UP] = narrow_to_wide(gettext("Up"));
	keynames[VLKC_DOWN] = narrow_to_wide(gettext("Down"));
	keynames[VLKC_RUN] = narrow_to_wide(gettext("Run"));
	keynames[VLKC_EXAMINE] = narrow_to_wide(gettext("Examine/Open"));
	keynames[VLKC_SCREENSHOT] = narrow_to_wide(gettext("Take Screenshot"));
	keynames[VLKC_TOGGLE_HUD] = narrow_to_wide(gettext("Show/Hide HUD"));
	keynames[VLKC_TOGGLE_CHAT] = narrow_to_wide(gettext("Show/Hide Chat"));
	keynames[VLKC_TOGGLE_FOG] = narrow_to_wide(gettext("Toggle Fog"));
	keynames[VLKC_TOGGLE_CAMERA] = L"";
	keynames[VLKC_TOGGLE_DEBUG] = L"";
	keynames[VLKC_TOGGLE_PROFILER] = L"";
	keynames[VLKC_RANGE_PLUS] = narrow_to_wide(gettext("Increase Viewing Range"));
	keynames[VLKC_RANGE_MINUS] = narrow_to_wide(gettext("Decrease Viewing Range"));
	keynames[VLKC_PRINT_DEBUG] = L"";
	keynames[VLKC_SELECT_PREV] = narrow_to_wide(gettext("Previous Item"));
	keynames[VLKC_SELECT_NEXT] = narrow_to_wide(gettext("Next Item"));
}

GUISettingsMenu::~GUISettingsMenu()
{
	removeChildren();
}

void GUISettingsMenu::removeChildren()
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

void GUISettingsMenu::save()
{
	// controls
	int m = VLKC_MAX;
	for (int i=0; i<m; i++) {
		saveKeySetting(keys[i],(KeyCode)i);
	}

	config_set_int("client.graphics.mesh.lod",m_data.mesh_detail);
	config_set_int("client.graphics.texture.lod",m_data.texture_detail);
	config_set_int("client.graphics.light.lod",m_data.light_detail);
	config_set_int("client.video.fullscreen",m_data.fullscreen);
	config_set_int("client.graphics.particles",m_data.particles);
	config_set_int("client.video.mipmaps",m_data.mip_map);
	config_set_int("client.video.anisotropic",m_data.anisotropic_filter);
	config_set_int("client.video.bilinear",m_data.bilinear_filter);
	config_set_int("client.video.trilinear",m_data.trilinear_filter);
	config_set_int("client.ui.hud.old",m_data.hotbar);
	config_set_int("client.ui.hud.wieldindex",m_data.wield_index);
	config_set_int("client.graphics.texture.animations",m_data.texture_animation);
	Environment->getVideoDriver()->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, m_data.mip_map);
}

void GUISettingsMenu::regenerateGui(v2u32 screensize)
{
	u16 mesh_detail;
	u16 texture_detail;
	u16 light_detail;
	bool fullscreen;
	bool particles;
	bool mipmap;
	bool bilinear;
	bool trilinear;
	bool anisotropic;
	bool hotbar;
	bool wield_index;
	bool texture_animation;
	f32 volume_master;
	f32 volume_effects;
	f32 volume_music;

	m_screensize = screensize;

	{
		gui::IGUIElement *e = getElementFromId(GUI_ID_MESH_DETAIL_COMBO);
		if (e != NULL && e->getType() == gui::EGUIET_COMBO_BOX) {
			gui::IGUIComboBox *c = (gui::IGUIComboBox*)e;
			switch (c->getItemData(c->getSelected())) {
			case GUI_ID_MESH_DETAIL_LOW:
				mesh_detail = 1;
				break;
			case GUI_ID_MESH_DETAIL_MEDIUM:
				mesh_detail = 2;
				break;
			case GUI_ID_MESH_DETAIL_HIGH:
				mesh_detail = 3;
				break;
			default:
				mesh_detail = 3;
			}
		}else{
			mesh_detail = m_data.mesh_detail;
		}
	}
	{
		gui::IGUIElement *e = getElementFromId(GUI_ID_TEXTURE_DETAIL_COMBO);
		if (e != NULL && e->getType() == gui::EGUIET_COMBO_BOX) {
			gui::IGUIComboBox *c = (gui::IGUIComboBox*)e;
			switch (c->getItemData(c->getSelected())) {
			case GUI_ID_TEXTURE_DETAIL_LOW:
				texture_detail = 1;
				break;
			case GUI_ID_TEXTURE_DETAIL_MEDIUM:
				texture_detail = 2;
				break;
			case GUI_ID_TEXTURE_DETAIL_HIGH:
				texture_detail = 3;
				break;
			default:
				texture_detail = 3;
			}
		}else{
			texture_detail = m_data.texture_detail;
		}
	}
	{
		gui::IGUIElement *e = getElementFromId(GUI_ID_LIGHT_DETAIL_COMBO);
		if (e != NULL && e->getType() == gui::EGUIET_COMBO_BOX) {
			gui::IGUIComboBox *c = (gui::IGUIComboBox*)e;
			switch (c->getItemData(c->getSelected())) {
			case GUI_ID_LIGHT_DETAIL_LOW:
				light_detail = 1;
				break;
			case GUI_ID_LIGHT_DETAIL_MEDIUM:
				light_detail = 2;
				break;
			case GUI_ID_LIGHT_DETAIL_HIGH:
				light_detail = 3;
				break;
			default:
				light_detail = 3;
			}
		}else{
			light_detail = m_data.light_detail;
		}
	}
	{
		gui::IGUIElement *e = getElementFromId(GUI_ID_FULLSCREEN_CB);
		if(e != NULL && e->getType() == gui::EGUIET_CHECK_BOX)
			fullscreen = ((gui::IGUICheckBox*)e)->isChecked();
		else
			fullscreen = m_data.fullscreen;
	}
	{
		gui::IGUIElement *e = getElementFromId(GUI_ID_PARTICLES_CB);
		if(e != NULL && e->getType() == gui::EGUIET_CHECK_BOX)
			particles = ((gui::IGUICheckBox*)e)->isChecked();
		else
			particles = m_data.particles;
	}
	{
		gui::IGUIElement *e = getElementFromId(GUI_ID_MIPMAP_CB);
		if(e != NULL && e->getType() == gui::EGUIET_CHECK_BOX)
			mipmap = ((gui::IGUICheckBox*)e)->isChecked();
		else
			mipmap = m_data.mip_map;
	}
	{
		gui::IGUIElement *e = getElementFromId(GUI_ID_BILINEAR_CB);
		if(e != NULL && e->getType() == gui::EGUIET_CHECK_BOX)
			bilinear = ((gui::IGUICheckBox*)e)->isChecked();
		else
			bilinear = m_data.bilinear_filter;
	}
	{
		gui::IGUIElement *e = getElementFromId(GUI_ID_TRILINEAR_CB);
		if(e != NULL && e->getType() == gui::EGUIET_CHECK_BOX)
			trilinear = ((gui::IGUICheckBox*)e)->isChecked();
		else
			trilinear = m_data.trilinear_filter;
	}
	{
		gui::IGUIElement *e = getElementFromId(GUI_ID_ANISOTROPIC_CB);
		if(e != NULL && e->getType() == gui::EGUIET_CHECK_BOX)
			anisotropic = ((gui::IGUICheckBox*)e)->isChecked();
		else
			anisotropic = m_data.anisotropic_filter;
	}
	{
		gui::IGUIElement *e = getElementFromId(GUI_ID_TEXTUREANIM_CB);
		if(e != NULL && e->getType() == gui::EGUIET_CHECK_BOX)
			texture_animation = ((gui::IGUICheckBox*)e)->isChecked();
		else
			texture_animation = m_data.texture_animation;
	}
	{
		gui::IGUIElement *e = getElementFromId(GUI_ID_HOTBAR_CB);
		if(e != NULL && e->getType() == gui::EGUIET_CHECK_BOX)
			hotbar = ((gui::IGUICheckBox*)e)->isChecked();
		else
			hotbar = m_data.hotbar;
	}
	{
		gui::IGUIElement *e = getElementFromId(GUI_ID_WIELDINDEX_CB);
		if(e != NULL && e->getType() == gui::EGUIET_CHECK_BOX)
			wield_index = ((gui::IGUICheckBox*)e)->isChecked();
		else
			wield_index = m_data.wield_index;
	}
	{
		gui::IGUIElement *e = getElementFromId(GUI_ID_VOLUME_MASTER_SB);
		if(e != NULL && e->getType() == gui::EGUIET_SCROLL_BAR)
			volume_master = (float)((gui::IGUIScrollBar*)e)->getPos();
		else
			volume_master = m_data.volume_master;
	}
	{
		gui::IGUIElement *e = getElementFromId(GUI_ID_VOLUME_EFFECTS_SB);
		if(e != NULL && e->getType() == gui::EGUIET_SCROLL_BAR)
			volume_effects = (float)((gui::IGUIScrollBar*)e)->getPos();
		else
			volume_effects = m_data.volume_effects;
	}
	{
		gui::IGUIElement *e = getElementFromId(GUI_ID_VOLUME_MUSIC_SB);
		if(e != NULL && e->getType() == gui::EGUIET_SCROLL_BAR)
			volume_music = (float)((gui::IGUIScrollBar*)e)->getPos();
		else
			volume_music = m_data.volume_music;
	}
	/*
	 Remove stuff
	 */
	removeChildren();

	/*
	 Calculate new sizes and positions
	 */

	v2s32 size(800, 500);

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
		Environment->addButton(rect, this, GUI_ID_TAB_MAINMENU, narrow_to_wide(gettext("Main Menu")).c_str());
	}

	// Controls Settings button
	{
		core::rect<s32> rect(0, 0, 180, 40);
		rect += v2s32(35, 260);
		Environment->addButton(rect, this, GUI_ID_TAB_SETTINGS_CONTROLS, narrow_to_wide(gettext("Controls")).c_str());
	}
	// Graphics Settings button
	{
		core::rect<s32> rect(0, 0, 180, 40);
		rect += v2s32(35, 305);
		Environment->addButton(rect, this, GUI_ID_TAB_SETTINGS_GRAPHICS, narrow_to_wide(gettext("Graphics")).c_str());
	}
	// Video Settings button
	{
		core::rect<s32> rect(0, 0, 180, 40);
		rect += v2s32(35, 350);
		Environment->addButton(rect, this, GUI_ID_TAB_SETTINGS_VIDEO, narrow_to_wide(gettext("Video")).c_str());
	}
	// Sound Settings button
	{
		core::rect<s32> rect(0, 0, 180, 40);
		rect += v2s32(35, 395);
		Environment->addButton(rect, this, GUI_ID_TAB_SETTINGS_SOUND, narrow_to_wide(gettext("Sound")).c_str());
	}

	v2s32 topleft_content(250, 0);

	if (m_data.selected_tab == TAB_SETTINGS_CONTROLS) {
		{
			core::rect<s32> rect(0, 0, 550, 20);
			rect += topleft_content + v2s32(0, 0);
			gui::IGUIStaticText *t = Environment->addStaticText(narrow_to_wide(gettext("Controls")).c_str(), rect, false, true, this, -1);
			t->setTextAlignment(gui::EGUIA_CENTER, gui::EGUIA_UPPERLEFT);
		}

		v2s32 offset(0, 40);

		for (int i=0; i<VLKC_MAX; i++) {
			if (keynames[i] == L"")
				continue;
			{
				core::rect < s32 > rect(0, 0, 150, 20);
				rect += topleft_content + offset;
				gui::IGUIStaticText *t = Environment->addStaticText(keynames[i].c_str(), rect, false, true, this, -1);
				t->setTextAlignment(gui::EGUIA_LOWERRIGHT, gui::EGUIA_UPPERLEFT);
			}

			{
				core::rect < s32 > rect(0, 0, 110, 30);
				rect += topleft_content + offset + v2s32(155, -5);
				Environment->addButton(rect, this, GUI_ID_KEYSETTINGS_BASE+i, keys[i].guiName().c_str());
			}
			offset += v2s32(0, 33);
			if (offset.Y > 450) {
				offset.Y = 40;
				offset.X = 275;
			}
		}
	}else if (m_data.selected_tab == TAB_SETTINGS_GRAPHICS) {
		{
			core::rect<s32> rect(0, 0, 550, 20);
			rect += topleft_content + v2s32(0, 0);
			gui::IGUIStaticText *t = Environment->addStaticText(narrow_to_wide(gettext("Graphics")).c_str(), rect, false, true, this, -1);
			t->setTextAlignment(gui::EGUIA_CENTER, gui::EGUIA_UPPERLEFT);
		}
		{
			core::rect<s32> rect(0, 0, 125, 20);
			rect += topleft_content + v2s32(40, 60);
			gui::IGUIStaticText *t = Environment->addStaticText(narrow_to_wide(gettext("Terrain Mesh Detail")).c_str(), rect, false, true, this, -1);
			t->setTextAlignment(gui::EGUIA_LOWERRIGHT, gui::EGUIA_UPPERLEFT);
		}
		{
			core::rect<s32> rect(0, 0, 240, 25);
			rect += topleft_content + v2s32(175, 55);
			gui::IGUIComboBox *c = Environment->addComboBox(rect, this, GUI_ID_MESH_DETAIL_COMBO);
			u32 ld = c->addItem(narrow_to_wide(gettext("Low")).c_str(),GUI_ID_MESH_DETAIL_LOW);
			u32 md = c->addItem(narrow_to_wide(gettext("Medium")).c_str(),GUI_ID_MESH_DETAIL_MEDIUM);
			u32 hd = c->addItem(narrow_to_wide(gettext("High")).c_str(),GUI_ID_MESH_DETAIL_HIGH);
			switch (mesh_detail) {
			case 1:
				c->setSelected(ld);
				break;
			case 2:
				c->setSelected(md);
				break;
			default:
				c->setSelected(hd);
				break;
			}
		}
		{
			core::rect<s32> rect(0, 0, 125, 20);
			rect += topleft_content + v2s32(40, 90);
			gui::IGUIStaticText *t = Environment->addStaticText(narrow_to_wide(gettext("Texture Detail")).c_str(), rect, false, true, this, -1);
			t->setTextAlignment(gui::EGUIA_LOWERRIGHT, gui::EGUIA_UPPERLEFT);
		}
		{
			core::rect<s32> rect(0, 0, 240, 25);
			rect += topleft_content + v2s32(175, 85);
			gui::IGUIComboBox *c = Environment->addComboBox(rect, this, GUI_ID_TEXTURE_DETAIL_COMBO);
			u32 ld = c->addItem(narrow_to_wide(gettext("Low")).c_str(),GUI_ID_TEXTURE_DETAIL_LOW);
			u32 md = c->addItem(narrow_to_wide(gettext("Medium")).c_str(),GUI_ID_TEXTURE_DETAIL_MEDIUM);
			u32 hd = c->addItem(narrow_to_wide(gettext("High")).c_str(),GUI_ID_TEXTURE_DETAIL_HIGH);
			switch (texture_detail) {
			case 1:
				c->setSelected(ld);
				break;
			case 2:
				c->setSelected(md);
				break;
			default:
				c->setSelected(hd);
				break;
			}
		}
		{
			core::rect<s32> rect(0, 0, 125, 20);
			rect += topleft_content + v2s32(40, 120);
			gui::IGUIStaticText *t = Environment->addStaticText(narrow_to_wide(gettext("Light Detail")).c_str(), rect, false, true, this, -1);
			t->setTextAlignment(gui::EGUIA_LOWERRIGHT, gui::EGUIA_UPPERLEFT);
		}
		{
			core::rect<s32> rect(0, 0, 240, 25);
			rect += topleft_content + v2s32(175, 115);
			gui::IGUIComboBox *c = Environment->addComboBox(rect, this, GUI_ID_LIGHT_DETAIL_COMBO);
			u32 ld = c->addItem(narrow_to_wide(gettext("Low")).c_str(),GUI_ID_LIGHT_DETAIL_LOW);
			u32 md = c->addItem(narrow_to_wide(gettext("Medium")).c_str(),GUI_ID_LIGHT_DETAIL_MEDIUM);
			u32 hd = c->addItem(narrow_to_wide(gettext("High")).c_str(),GUI_ID_LIGHT_DETAIL_HIGH);
			switch (light_detail) {
			case 1:
				c->setSelected(ld);
				break;
			case 2:
				c->setSelected(md);
				break;
			default:
				c->setSelected(hd);
				break;
			}
		}
		{
			core::rect<s32> rect(0, 0, 200, 30);
			rect += topleft_content + v2s32(80, 160);
			Environment->addCheckBox(hotbar, rect, this, GUI_ID_HOTBAR_CB, narrow_to_wide(gettext("Classic HUD")).c_str());
		}
		{
			core::rect<s32> rect(0, 0, 200, 30);
			rect += topleft_content + v2s32(290, 160);
			gui::IGUICheckBox *c = Environment->addCheckBox(wield_index, rect, this, GUI_ID_WIELDINDEX_CB, narrow_to_wide(gettext("Wieldring Index")).c_str());
			c->setEnabled(!hotbar);
		}
		if (m_is_ingame) {
			core::rect<s32> rect(0, 0, 550, 20);
			rect += topleft_content + v2s32(0, 220);
			gui::IGUIStaticText *t = Environment->addStaticText(narrow_to_wide(gettext("Some settings cannot be changed in-game.")).c_str(), rect, false, true, this, -1);
			t->setTextAlignment(gui::EGUIA_CENTER, gui::EGUIA_UPPERLEFT);
		}
	}else if (m_data.selected_tab == TAB_SETTINGS_VIDEO) {
		{
			core::rect<s32> rect(0, 0, 550, 20);
			rect += topleft_content + v2s32(0, 0);
			gui::IGUIStaticText *t = Environment->addStaticText(narrow_to_wide(gettext("Video")).c_str(), rect, false, true, this, -1);
			t->setTextAlignment(gui::EGUIA_CENTER, gui::EGUIA_UPPERLEFT);
		}
		{
			core::rect<s32> rect(0, 0, 200, 30);
			rect += topleft_content + v2s32(80, 60);
			Environment->addCheckBox(fullscreen, rect, this, GUI_ID_FULLSCREEN_CB, narrow_to_wide(gettext("Fullscreen")).c_str());
		}
		{
			core::rect<s32> rect(0, 0, 200, 30);
			rect += topleft_content + v2s32(80, 90);
			Environment->addCheckBox(particles, rect, this, GUI_ID_PARTICLES_CB, narrow_to_wide(gettext("Particles")).c_str());
		}
		{
			core::rect<s32> rect(0, 0, 200, 30);
			rect += topleft_content + v2s32(80, 120);
			Environment->addCheckBox(mipmap, rect, this, GUI_ID_MIPMAP_CB, narrow_to_wide(gettext("Mip-Mapping")).c_str());
		}
		{
			core::rect<s32> rect(0, 0, 200, 30);
			rect += topleft_content + v2s32(80, 150);
			Environment->addCheckBox(bilinear, rect, this, GUI_ID_BILINEAR_CB, narrow_to_wide(gettext("Bi-Linear Filtering")).c_str());
		}
		{
			core::rect<s32> rect(0, 0, 200, 30);
			rect += topleft_content + v2s32(80, 180);
			Environment->addCheckBox(trilinear, rect, this, GUI_ID_TRILINEAR_CB, narrow_to_wide(gettext("Tri-Linear Filtering")).c_str());
		}
		{
			core::rect<s32> rect(0, 0, 200, 30);
			rect += topleft_content + v2s32(80, 210);
			Environment->addCheckBox(anisotropic, rect, this, GUI_ID_ANISOTROPIC_CB, narrow_to_wide(gettext("Anisotropic Filtering")).c_str());
		}
		{
			core::rect<s32> rect(0, 0, 200, 30);
			rect += topleft_content + v2s32(80, 240);
			Environment->addCheckBox(texture_animation, rect, this, GUI_ID_TEXTUREANIM_CB, narrow_to_wide(gettext("Enable Texture Animation")).c_str());
		}
		if (m_is_ingame) {
			core::rect<s32> rect(0, 0, 550, 20);
			rect += topleft_content + v2s32(0, 280);
			gui::IGUIStaticText *t = Environment->addStaticText(narrow_to_wide(gettext("Some settings cannot be changed in-game.")).c_str(), rect, false, true, this, -1);
			t->setTextAlignment(gui::EGUIA_CENTER, gui::EGUIA_UPPERLEFT);
		}
	}else if (m_data.selected_tab == TAB_SETTINGS_SOUND) {
		{
			core::rect<s32> rect(0, 0, 550, 20);
			rect += topleft_content + v2s32(0, 0);
			gui::IGUIStaticText *t = Environment->addStaticText(narrow_to_wide(gettext("Sound")).c_str(), rect, false, true, this, -1);
			t->setTextAlignment(gui::EGUIA_CENTER, gui::EGUIA_UPPERLEFT);
		}
		{
			core::rect<s32> rect(0, 0, 200, 15);
			rect += topleft_content + v2s32(80, 60);
			Environment->addStaticText(narrow_to_wide(gettext("Master Volume:")).c_str(), rect, false, false, this, -1);
		}
		{
			core::rect<s32> rect(0, 0, 200, 15);
			rect += topleft_content + v2s32(290, 60);
			gui::IGUIScrollBar *sb = Environment->addScrollBar(true, rect, this, GUI_ID_VOLUME_MASTER_SB);
			sb->setMin(0);
			sb->setMax(100);
			sb->setPos(volume_master);
		}
		{
			core::rect<s32> rect(0, 0, 200, 15);
			rect += topleft_content + v2s32(80, 90);
			Environment->addStaticText(narrow_to_wide(gettext("Effects Volume:")).c_str(), rect, false, false, this, -1);
		}
		{
			core::rect<s32> rect(0, 0, 200, 15);
			rect += topleft_content + v2s32(290, 90);
			gui::IGUIScrollBar *sb = Environment->addScrollBar(true, rect, this, GUI_ID_VOLUME_EFFECTS_SB);
			sb->setMin(0);
			sb->setMax(100);
			sb->setPos(volume_effects);
		}
		{
			core::rect<s32> rect(0, 0, 200, 15);
			rect += topleft_content + v2s32(80, 120);
			Environment->addStaticText(narrow_to_wide(gettext("Music Volume:")).c_str(), rect, false, false, this, -1);
		}
		{
			core::rect<s32> rect(0, 0, 200, 15);
			rect += topleft_content + v2s32(290, 120);
			gui::IGUIScrollBar *sb = Environment->addScrollBar(true, rect, this, GUI_ID_VOLUME_MUSIC_SB);
			sb->setMin(0);
			sb->setMax(100);
			sb->setPos(volume_music);
		}
	}
}

void GUISettingsMenu::drawMenu()
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

bool GUISettingsMenu::acceptInput()
{
	{
		gui::IGUIElement *e = getElementFromId(GUI_ID_MESH_DETAIL_COMBO);
		if (e != NULL && e->getType() == gui::EGUIET_COMBO_BOX) {
			gui::IGUIComboBox *c = (gui::IGUIComboBox*)e;
			switch (c->getItemData(c->getSelected())) {
			case GUI_ID_MESH_DETAIL_LOW:
				m_data.mesh_detail = 1;
				break;
			case GUI_ID_MESH_DETAIL_MEDIUM:
				m_data.mesh_detail = 2;
				break;
			case GUI_ID_MESH_DETAIL_HIGH:
				m_data.mesh_detail = 3;
				break;
			default:
				m_data.mesh_detail = 3;
			}
		}
	}
	{
		gui::IGUIElement *e = getElementFromId(GUI_ID_TEXTURE_DETAIL_COMBO);
		if (e != NULL && e->getType() == gui::EGUIET_COMBO_BOX) {
			gui::IGUIComboBox *c = (gui::IGUIComboBox*)e;
			switch (c->getItemData(c->getSelected())) {
			case GUI_ID_TEXTURE_DETAIL_LOW:
				m_data.texture_detail = 1;
				break;
			case GUI_ID_TEXTURE_DETAIL_MEDIUM:
				m_data.texture_detail = 2;
				break;
			case GUI_ID_TEXTURE_DETAIL_HIGH:
				m_data.texture_detail = 3;
				break;
			default:
				m_data.texture_detail = 3;
			}
		}
	}
	{
		gui::IGUIElement *e = getElementFromId(GUI_ID_LIGHT_DETAIL_COMBO);
		if (e != NULL && e->getType() == gui::EGUIET_COMBO_BOX) {
			gui::IGUIComboBox *c = (gui::IGUIComboBox*)e;
			switch (c->getItemData(c->getSelected())) {
			case GUI_ID_LIGHT_DETAIL_LOW:
				m_data.light_detail = 1;
				break;
			case GUI_ID_LIGHT_DETAIL_MEDIUM:
				m_data.light_detail = 2;
				break;
			case GUI_ID_LIGHT_DETAIL_HIGH:
				m_data.light_detail = 3;
				break;
			default:
				m_data.light_detail = 3;
			}
		}
	}
	{
		gui::IGUIElement *e = getElementFromId(GUI_ID_FULLSCREEN_CB);
		if(e != NULL && e->getType() == gui::EGUIET_CHECK_BOX)
			m_data.fullscreen = ((gui::IGUICheckBox*)e)->isChecked();
	}
	{
		gui::IGUIElement *e = getElementFromId(GUI_ID_PARTICLES_CB);
		if(e != NULL && e->getType() == gui::EGUIET_CHECK_BOX)
			m_data.particles = ((gui::IGUICheckBox*)e)->isChecked();
	}
	{
		gui::IGUIElement *e = getElementFromId(GUI_ID_MIPMAP_CB);
		if(e != NULL && e->getType() == gui::EGUIET_CHECK_BOX)
			m_data.mip_map = ((gui::IGUICheckBox*)e)->isChecked();
	}
	{
		gui::IGUIElement *e = getElementFromId(GUI_ID_BILINEAR_CB);
		if(e != NULL && e->getType() == gui::EGUIET_CHECK_BOX)
			m_data.bilinear_filter = ((gui::IGUICheckBox*)e)->isChecked();
	}
	{
		gui::IGUIElement *e = getElementFromId(GUI_ID_TRILINEAR_CB);
		if(e != NULL && e->getType() == gui::EGUIET_CHECK_BOX)
			m_data.trilinear_filter = ((gui::IGUICheckBox*)e)->isChecked();
	}
	{
		gui::IGUIElement *e = getElementFromId(GUI_ID_ANISOTROPIC_CB);
		if(e != NULL && e->getType() == gui::EGUIET_CHECK_BOX)
			m_data.anisotropic_filter = ((gui::IGUICheckBox*)e)->isChecked();
	}
	{
		gui::IGUIElement *e = getElementFromId(GUI_ID_TEXTUREANIM_CB);
		if (e != NULL && e->getType() == gui::EGUIET_CHECK_BOX)
			m_data.texture_animation = ((gui::IGUICheckBox*)e)->isChecked();
	}
	{
		gui::IGUIElement *e = getElementFromId(GUI_ID_HOTBAR_CB);
		if(e != NULL && e->getType() == gui::EGUIET_CHECK_BOX)
			m_data.hotbar = ((gui::IGUICheckBox*)e)->isChecked();
	}
	{
		gui::IGUIElement *e = getElementFromId(GUI_ID_WIELDINDEX_CB);
		if(e != NULL && e->getType() == gui::EGUIET_CHECK_BOX)
			m_data.wield_index = ((gui::IGUICheckBox*)e)->isChecked();
	}
	{
		gui::IGUIElement *e = getElementFromId(GUI_ID_VOLUME_MASTER_SB);
		if(e != NULL && e->getType() == gui::EGUIET_SCROLL_BAR)
			m_data.volume_master = (float)((gui::IGUIScrollBar*)e)->getPos();
	}
	{
		gui::IGUIElement *e = getElementFromId(GUI_ID_VOLUME_EFFECTS_SB);
		if(e != NULL && e->getType() == gui::EGUIET_SCROLL_BAR)
			m_data.volume_effects = (float)((gui::IGUIScrollBar*)e)->getPos();
	}
	{
		gui::IGUIElement *e = getElementFromId(GUI_ID_VOLUME_MUSIC_SB);
		if(e != NULL && e->getType() == gui::EGUIET_SCROLL_BAR)
			m_data.volume_music = (float)((gui::IGUIScrollBar*)e)->getPos();
	}
	return true;
}
void GUISettingsMenu::init_keys()
{
	int m = VLKC_MAX;
	for (int i=0; i<m; i++) {
		keys[i] = getKeySetting((KeyCode)i);
	}
}

bool GUISettingsMenu::resetMenu()
{
	if (activeKey >= 0) {
		gui::IGUIElement *e = getElementFromId(activeKey);
		if (e != NULL && e->getType() == gui::EGUIET_BUTTON) {
			e->setEnabled(true);
			e->setText(keynames[activeKey-GUI_ID_KEYSETTINGS_BASE].c_str());
		}
		activeKey = -1;
		return false;
	}
	return true;
}
bool GUISettingsMenu::OnEvent(const SEvent& event)
{
	if (event.EventType == EET_KEY_INPUT_EVENT && activeKey >= 0 && event.KeyInput.PressedDown) {
		KeyPress kp(event.KeyInput);
		gui::IGUIElement *e = getElementFromId(activeKey);
		if (e != NULL && e->getType() == gui::EGUIET_BUTTON) {
			e->setEnabled(true);
			e->setText(kp.guiName().c_str());
			keys[activeKey-GUI_ID_KEYSETTINGS_BASE] = kp;
		}
		activeKey = -1;
		return true;
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
			s32 id = event.GUIEvent.Caller->getID();
			if (id >= GUI_ID_KEYSETTINGS_BASE) {
				resetMenu();
				activeKey = id;
				gui::IGUIElement *e = getElementFromId(id);
				if (e != NULL && e->getType() == gui::EGUIET_BUTTON) {
					e->setText(narrow_to_wide(gettext("press Key")).c_str());
					e->setEnabled(false);
					return true;
				}
			}else{
				switch (event.GUIEvent.Caller->getID()) {
				case GUI_ID_TAB_SETTINGS_CONTROLS:
					acceptInput();
					m_accepted = false;
					m_data.selected_tab = TAB_SETTINGS_CONTROLS;
					regenerateGui(m_screensize);
					return true;
				case GUI_ID_TAB_SETTINGS_GRAPHICS:
					acceptInput();
					m_accepted = false;
					m_data.selected_tab = TAB_SETTINGS_GRAPHICS;
					regenerateGui(m_screensize);
					return true;
				case GUI_ID_TAB_SETTINGS_VIDEO:
					acceptInput();
					m_accepted = false;
					m_data.selected_tab = TAB_SETTINGS_VIDEO;
					regenerateGui(m_screensize);
					return true;
				case GUI_ID_TAB_SETTINGS_SOUND:
					acceptInput();
					m_accepted = false;
					m_data.selected_tab = TAB_SETTINGS_SOUND;
					regenerateGui(m_screensize);
					return true;
				case GUI_ID_TAB_MAINMENU: //back
					acceptInput();
					save();
					quitMenu();
					return true;
				}
			}
		}
		if (event.GUIEvent.EventType == gui::EGET_SCROLL_BAR_CHANGED) {
			switch (event.GUIEvent.Caller->getID()) {
			case GUI_ID_VOLUME_MASTER_SB:
			{
				gui::IGUIElement *vsb = getElementFromId(GUI_ID_VOLUME_MASTER_SB);
				if(vsb != NULL && vsb->getType() == gui::EGUIET_SCROLL_BAR) {
					m_data.volume_master = (float)((gui::IGUIScrollBar*)vsb)->getPos();
					config_set_int("client.sound.volume",m_data.volume_master);
				}
				return true;
			}
			case GUI_ID_VOLUME_EFFECTS_SB:
			{
				gui::IGUIElement *vsb = getElementFromId(GUI_ID_VOLUME_EFFECTS_SB);
				if(vsb != NULL && vsb->getType() == gui::EGUIET_SCROLL_BAR) {
					m_data.volume_effects = (float)((gui::IGUIScrollBar*)vsb)->getPos();
					config_set_int("client.sound.volume.effects",m_data.volume_effects);
				}
				return true;
			}
			case GUI_ID_VOLUME_MUSIC_SB:
			{
				gui::IGUIElement *vsb = getElementFromId(GUI_ID_VOLUME_MUSIC_SB);
				if(vsb != NULL && vsb->getType() == gui::EGUIET_SCROLL_BAR) {
					m_data.volume_music = (float)((gui::IGUIScrollBar*)vsb)->getPos();
					config_set_int("client.sound.volume.music",m_data.volume_music);
				}
				return true;
			}
			}
		}
		if (event.GUIEvent.EventType == gui::EGET_COMBO_BOX_CHANGED) {
			switch (event.GUIEvent.Caller->getID()) {
			case GUI_ID_MESH_DETAIL_COMBO:
			case GUI_ID_TEXTURE_DETAIL_COMBO:
			case GUI_ID_LIGHT_DETAIL_COMBO:
				acceptInput();
				m_accepted = false;
				regenerateGui(m_screensize);
				return true;
			}
		}
	}
	return Parent ? Parent->OnEvent(event) : false;
}

