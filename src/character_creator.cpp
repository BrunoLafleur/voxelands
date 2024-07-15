/************************************************************************
* character_creator.cpp
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

#include "character_creator.h"
#include "debug.h"
#include "serialization.h"
#include <string>
#include <IGUIButton.h>
#include <IGUIStaticText.h>
#include <IGUIFont.h>
#include "path.h"
#include "utility.h"
#include "gui_colours.h"

#include "intl.h"

GUICharDefMenu::GUICharDefMenu(
	IrrlichtDevice* device,
	gui::IGUIEnvironment* env,
	gui::IGUIElement* parent, s32 id,
	IMenuManager *menumgr
):
	GUIModalMenu(env, parent, id, menumgr),
	m_accepted(false)
{
	this->env = env;
	this->parent = parent;
	this->id = id;
	this->menumgr = menumgr;
	m_model_rotation = 90;

	fetchPlayerSkin();

	m_skintone_types["white"] = narrow_to_wide(gettext("White Skin"));
	m_skintone_types["red"] = narrow_to_wide(gettext("Red Skin"));
	m_skintone_types["green"] = narrow_to_wide(gettext("Green Skin"));
	m_skintone_types["fair"] = narrow_to_wide(gettext("Fair Skin"));
	m_skintone_types["tanned"] = narrow_to_wide(gettext("Tanned Skin"));
	m_skintone_types["dark"] = narrow_to_wide(gettext("Dark Skin"));
	m_skintone_types["black"] = narrow_to_wide(gettext("Black Skin"));

	m_face_types["human"] = narrow_to_wide(gettext("Human Face"));
	m_face_types["elven"] = narrow_to_wide(gettext("Elven Face"));
	m_face_types["dwarven"] = narrow_to_wide(gettext("Dwarven Face"));
	m_face_types["alien"] = narrow_to_wide(gettext("Alien Face"));

	m_hairtone_types["white"] = narrow_to_wide(gettext("White Hair"));
	m_hairtone_types["blue"] = narrow_to_wide(gettext("Blue Hair"));
	m_hairtone_types["green"] = narrow_to_wide(gettext("Green Hair"));
	m_hairtone_types["orange"] = narrow_to_wide(gettext("Orange Hair"));
	m_hairtone_types["brown"] = narrow_to_wide(gettext("Brown Hair"));
	m_hairtone_types["purple"] = narrow_to_wide(gettext("Purple Hair"));
	m_hairtone_types["red"] = narrow_to_wide(gettext("Red Hair"));
	m_hairtone_types["blonde"] = narrow_to_wide(gettext("Blonde Hair"));
	m_hairtone_types["black"] = narrow_to_wide(gettext("Black Hair"));

	m_hair_types["short"] = narrow_to_wide(gettext("Short Hair"));
	m_hair_types["medium"] = narrow_to_wide(gettext("Medium Hair"));
	m_hair_types["long"] = narrow_to_wide(gettext("Long Hair"));
	m_hair_types["special"] = narrow_to_wide(gettext("Styled Hair"));

	m_eyes_types["white"] = narrow_to_wide(gettext("White Eyes"));
	m_eyes_types["blue"] = narrow_to_wide(gettext("Blue Eyes"));
	m_eyes_types["green"] = narrow_to_wide(gettext("Green Eyes"));
	m_eyes_types["orange"] = narrow_to_wide(gettext("Orange Eyes"));
	m_eyes_types["brown"] = narrow_to_wide(gettext("Brown Eyes"));
	m_eyes_types["purple"] = narrow_to_wide(gettext("Purple Eyes"));
	m_eyes_types["red"] = narrow_to_wide(gettext("Red Eyes"));
	m_eyes_types["yellow"] = narrow_to_wide(gettext("Yellow Eyes"));
	m_eyes_types["black"] = narrow_to_wide(gettext("Black Eyes"));

	m_shirt_types["white"] = narrow_to_wide(gettext("White T-Shirt"));
	m_shirt_types["blue"] = narrow_to_wide(gettext("Blue T-Shirt"));
	m_shirt_types["green"] = narrow_to_wide(gettext("Green T-Shirt"));
	m_shirt_types["orange"] = narrow_to_wide(gettext("Orange T-Shirt"));
	m_shirt_types["purple"] = narrow_to_wide(gettext("Purple T-Shirt"));
	m_shirt_types["red"] = narrow_to_wide(gettext("Red T-Shirt"));
	m_shirt_types["yellow"] = narrow_to_wide(gettext("Yellow T-Shirt"));
	m_shirt_types["black"] = narrow_to_wide(gettext("Black T-Shirt"));

	m_pants_types["white"] = narrow_to_wide(gettext("White Pants"));
	m_pants_types["blue"] = narrow_to_wide(gettext("Blue Pants"));
	m_pants_types["green"] = narrow_to_wide(gettext("Green Pants"));
	m_pants_types["orange"] = narrow_to_wide(gettext("Orange Pants"));
	m_pants_types["purple"] = narrow_to_wide(gettext("Purple Pants"));
	m_pants_types["red"] = narrow_to_wide(gettext("Red Pants"));
	m_pants_types["yellow"] = narrow_to_wide(gettext("Yellow Pants"));
	m_pants_types["black"] = narrow_to_wide(gettext("Black Pants"));

	m_shoes_types["leather"] = narrow_to_wide(gettext("Leather Shoes"));
	m_shoes_types["fur"] = narrow_to_wide(gettext("Fur Shoes"));
	m_shoes_types["canvas"] = narrow_to_wide(gettext("Canvas Shoes"));

	scene::ISceneManager* smgr = device->getSceneManager();

	m_cameranode = smgr->addCameraSceneNode(smgr->getRootSceneNode());
	m_cameranode->setPosition(v3f(20,0,0));
	// *100.0 helps in large map coordinates
	m_cameranode->setTarget(v3f(0,0,0));

	// get a unique mesh so that the player model has it's own lighting
	scene::IAnimatedMesh* mesh = createModelMesh(smgr,"character.b3d",true);
	if (!mesh)
		return;

	m_model = smgr->addAnimatedMeshSceneNode(mesh,smgr->getRootSceneNode());

	if (m_model) {
		m_model->setFrameLoop(0,79);
		m_model->setScale(v3f(m_xz_scale,m_y_scale,m_xz_scale));
		m_model->setRotation(v3f(0,m_model_rotation,0));
		setMeshColor(m_model->getMesh(), video::SColor(255,255,255,255));

		// Set material flags and texture
		m_model->setMaterialTexture(0, getPlayerTexture());
		video::SMaterial& material = m_model->getMaterial(0);
		material.setFlag(video::EMF_LIGHTING, false);
		material.setFlag(video::EMF_BILINEAR_FILTER, false);
		material.MaterialType = video::EMT_TRANSPARENT_ALPHA_CHANNEL_REF;

		m_model->setPosition(v3f(0,0,0));
	}
#if (IRRLICHT_VERSION_MAJOR >= 1 && IRRLICHT_VERSION_MINOR >= 8) || IRRLICHT_VERSION_MAJOR >= 2
	mesh->drop();
#endif
}

GUICharDefMenu::~GUICharDefMenu()
{
	removeChildren();
}

void GUICharDefMenu::removeChildren()
{
	/* irrlicht internal */
	const core::list<gui::IGUIElement*> &children = getChildren();
	core::list<gui::IGUIElement*> children_copy;
	for (core::list<gui::IGUIElement*>::ConstIterator i = children.begin(); i != children.end(); i++) {
		children_copy.push_back(*i);
	}
	for (core::list<gui::IGUIElement*>::Iterator i = children_copy.begin(); i != children_copy.end(); i++) {
		(*i)->remove();
	}
}

//gender:Yscale:XZscale:skintone:eyes:hairtone:hair:face:shirt-colour:pants-colour:shoe-type
//M:10:10:fair:blue:brown:medium:normal:green:blue:leather

void GUICharDefMenu::regenerateGui(v2u32 screensize)
{

	m_screensize = screensize;

	/*
		Remove stuff
	*/
	removeChildren();

	/*
		Calculate new sizes and positions
	*/
	v2s32 size(250, 500);

	core::rect<s32> rect(
			10,
			screensize.Y/2 - size.Y/2,
			screensize.X - 10,
			screensize.Y/2 + size.Y/2
	);

	v2s32 leftside(0, 0);
	v2s32 rightside(screensize.X - (size.X+10), 0);

	DesiredRect = rect;
	recalculateAbsolutePosition(false);

	{
		core::rect<s32> rect(0, 0, 230, 300);
		rect += leftside + v2s32(10, 220);
		gui::IGUIStaticText *t = Environment->addStaticText(
			narrow_to_wide(gettext(
				"Here you can create your default character,"
				" this is how other players will see you in-game."
				" When you join a new server you will start with"
				" the clothing you select here. Additional"
				" clothing, both protective and decorative,"
				" can be crafted in-game.\n"
				"You can return here from the main menu anytime to"
				" change your character."
			)).c_str(),
			rect,
			false,
			true,
			this,
			-1
		);
		t->setTextAlignment(gui::EGUIA_CENTER, gui::EGUIA_UPPERLEFT);
	}

	{
		core::rect<s32> rect(0, 0, 250, 20);
		rect += rightside + v2s32(0, 20);
		gui::IGUIStaticText *t = Environment->addStaticText(narrow_to_wide(gettext("Create Your Character")).c_str(), rect, false, true, this, -1);
		t->setTextAlignment(gui::EGUIA_CENTER, gui::EGUIA_UPPERLEFT);
	}
	// gender
	{
		core::rect<s32> rect(0, 0, 105, 30);
		rect += rightside + v2s32(15, 50);
		Environment->addButton(rect, this, GUI_ID_CD_MALE_BUTTON, narrow_to_wide(gettext("Male")).c_str());
	}
	{
		core::rect<s32> rect(0, 0, 105, 30);
		rect += rightside + v2s32(130, 50);
		Environment->addButton(rect, this, GUI_ID_CD_FEMALE_BUTTON, narrow_to_wide(gettext("Female")).c_str());
	}

	// Yscale
	{
		core::rect<s32> rect(0, 0, 105, 30);
		rect += rightside + v2s32(15, 95);
		Environment->addButton(rect, this, GUI_ID_CD_TALLER_BUTTON, narrow_to_wide(gettext("Taller")).c_str());
	}
	{
		core::rect<s32> rect(0, 0, 105, 30);
		rect += rightside + v2s32(15, 130);
		Environment->addButton(rect, this, GUI_ID_CD_SHORTER_BUTTON, narrow_to_wide(gettext("Shorter")).c_str());
	}

	// XZscale
	{
		core::rect<s32> rect(0, 0, 105, 30);
		rect += rightside + v2s32(130, 95);
		Environment->addButton(rect, this, GUI_ID_CD_WIDER_BUTTON, narrow_to_wide(gettext("Wider")).c_str());
	}
	{
		core::rect<s32> rect(0, 0, 105, 30);
		rect += rightside + v2s32(130, 130);
		Environment->addButton(rect, this, GUI_ID_CD_THINNER_BUTTON, narrow_to_wide(gettext("Thinner")).c_str());
	}

	// skintone
	//185
	{
		core::rect<s32> rect(0, 0, 160, 20);
		rect += rightside + v2s32(45, 170);
		gui::IGUIStaticText *t = Environment->addStaticText(m_skintone_types[m_parts["skintone"]].c_str(), rect, false, true, this, -1);
		t->setTextAlignment(gui::EGUIA_CENTER, gui::EGUIA_UPPERLEFT);
	}
	{
		core::rect<s32> rect(0, 0, 30, 30);
		rect += rightside + v2s32(15, 165);
		Environment->addButton(rect, this, GUI_ID_CD_SKINTONE_PREV_BUTTON, L"<<");
	}
	{
		core::rect<s32> rect(0, 0, 30, 30);
		rect += rightside + v2s32(205, 165);
		Environment->addButton(rect, this, GUI_ID_CD_SKINTONE_NEXT_BUTTON, L">>");
	}

	// face
	//220
	{
		core::rect<s32> rect(0, 0, 160, 20);
		rect += rightside + v2s32(45, 205);
		gui::IGUIStaticText *t = Environment->addStaticText(m_face_types[m_parts["face"]].c_str(), rect, false, true, this, -1);
		t->setTextAlignment(gui::EGUIA_CENTER, gui::EGUIA_UPPERLEFT);
	}
	{
		core::rect<s32> rect(0, 0, 30, 30);
		rect += rightside + v2s32(15, 200);
		Environment->addButton(rect, this, GUI_ID_CD_FACE_PREV_BUTTON, L"<<");
	}
	{
		core::rect<s32> rect(0, 0, 30, 30);
		rect += rightside + v2s32(205, 200);
		Environment->addButton(rect, this, GUI_ID_CD_FACE_NEXT_BUTTON, L">>");
	}

	// hairtone
	//255
	{
		core::rect<s32> rect(0, 0, 160, 20);
		rect += rightside + v2s32(45, 240);
		gui::IGUIStaticText *t = Environment->addStaticText(m_hairtone_types[m_parts["hairtone"]].c_str(), rect, false, true, this, -1);
		t->setTextAlignment(gui::EGUIA_CENTER, gui::EGUIA_UPPERLEFT);
	}
	{
		core::rect<s32> rect(0, 0, 30, 30);
		rect += rightside + v2s32(15, 235);
		Environment->addButton(rect, this, GUI_ID_CD_HAIRTONE_PREV_BUTTON, L"<<");
	}
	{
		core::rect<s32> rect(0, 0, 30, 30);
		rect += rightside + v2s32(205, 235);
		Environment->addButton(rect, this, GUI_ID_CD_HAIRTONE_NEXT_BUTTON, L">>");
	}

	// hair
	//290
	{
		core::rect<s32> rect(0, 0, 160, 20);
		rect += rightside + v2s32(45, 275);
		gui::IGUIStaticText *t = Environment->addStaticText(m_hair_types[m_parts["hair"]].c_str(), rect, false, true, this, -1);
		t->setTextAlignment(gui::EGUIA_CENTER, gui::EGUIA_UPPERLEFT);
	}
	{
		core::rect<s32> rect(0, 0, 30, 30);
		rect += rightside + v2s32(15, 270);
		Environment->addButton(rect, this, GUI_ID_CD_HAIR_PREV_BUTTON, L"<<");
	}
	{
		core::rect<s32> rect(0, 0, 30, 30);
		rect += rightside + v2s32(205, 270);
		Environment->addButton(rect, this, GUI_ID_CD_HAIR_NEXT_BUTTON, L">>");
	}

	// eyes
	//325
	{
		core::rect<s32> rect(0, 0, 160, 20);
		rect += rightside + v2s32(45, 315);
		gui::IGUIStaticText *t = Environment->addStaticText(m_eyes_types[m_parts["eyes"]].c_str(), rect, false, true, this, -1);
		t->setTextAlignment(gui::EGUIA_CENTER, gui::EGUIA_UPPERLEFT);
	}
	{
		core::rect<s32> rect(0, 0, 30, 30);
		rect += rightside + v2s32(15, 305);
		Environment->addButton(rect, this, GUI_ID_CD_EYES_PREV_BUTTON, L"<<");
	}
	{
		core::rect<s32> rect(0, 0, 30, 30);
		rect += rightside + v2s32(205, 305);
		Environment->addButton(rect, this, GUI_ID_CD_EYES_NEXT_BUTTON, L">>");
	}

	// t-shirt
	//370
	{
		core::rect<s32> rect(0, 0, 160, 20);
		rect += rightside + v2s32(45, 355);
		gui::IGUIStaticText *t = Environment->addStaticText(m_shirt_types[m_parts["shirt"]].c_str(), rect, false, true, this, -1);
		t->setTextAlignment(gui::EGUIA_CENTER, gui::EGUIA_UPPERLEFT);
	}
	{
		core::rect<s32> rect(0, 0, 30, 30);
		rect += rightside + v2s32(15, 350);
		Environment->addButton(rect, this, GUI_ID_CD_SHIRT_PREV_BUTTON, L"<<");
	}
	{
		core::rect<s32> rect(0, 0, 30, 30);
		rect += rightside + v2s32(205, 350);
		Environment->addButton(rect, this, GUI_ID_CD_SHIRT_NEXT_BUTTON, L">>");
	}

	// pants
	//405
	{
		core::rect<s32> rect(0, 0, 160, 20);
		rect += rightside + v2s32(45, 390);
		gui::IGUIStaticText *t = Environment->addStaticText(m_pants_types[m_parts["pants"]].c_str(), rect, false, true, this, -1);
		t->setTextAlignment(gui::EGUIA_CENTER, gui::EGUIA_UPPERLEFT);
	}
	{
		core::rect<s32> rect(0, 0, 30, 30);
		rect += rightside + v2s32(15, 385);
		Environment->addButton(rect, this, GUI_ID_CD_PANTS_PREV_BUTTON, L"<<");
	}
	{
		core::rect<s32> rect(0, 0, 30, 30);
		rect += rightside + v2s32(205, 385);
		Environment->addButton(rect, this, GUI_ID_CD_PANTS_NEXT_BUTTON, L">>");
	}

	// shoes
	//440
	{
		core::rect<s32> rect(0, 0, 160, 20);
		rect += rightside + v2s32(45, 425);
		gui::IGUIStaticText *t = Environment->addStaticText(m_shoes_types[m_parts["shoes"]].c_str(), rect, false, true, this, -1);
		t->setTextAlignment(gui::EGUIA_CENTER, gui::EGUIA_UPPERLEFT);
	}
	{
		core::rect<s32> rect(0, 0, 30, 30);
		rect += rightside + v2s32(15, 420);
		Environment->addButton(rect, this, GUI_ID_CD_SHOES_PREV_BUTTON, L"<<");
	}
	{
		core::rect<s32> rect(0, 0, 30, 30);
		rect += rightside + v2s32(205, 420);
		Environment->addButton(rect, this, GUI_ID_CD_SHOES_NEXT_BUTTON, L">>");
	}

	// save/cancel
	{
		core::rect<s32> rect(0, 0, 105, 30);
		rect += rightside + v2s32(15, 460);
		Environment->addButton(rect, this, GUI_ID_CD_SAVE_BUTTON, narrow_to_wide(gettext("Done")).c_str());
	}
	{
		core::rect<s32> rect(0, 0, 105, 30);
		rect += rightside + v2s32(130, 460);
		Environment->addButton(rect, this, GUI_ID_CD_QUIT_BUTTON, narrow_to_wide(gettext("Cancel")).c_str());
	}
}

void GUICharDefMenu::drawMenu()
{
	char buff[1024];
	gui::IGUISkin* skin = Environment->getSkin();
	if (!skin)
		return;

	video::IVideoDriver* driver = Environment->getVideoDriver();

	core::rect<s32> left(
		0,
		0,
		250,
		m_screensize.Y
	);
	core::rect<s32> right(
		m_screensize.X-230,
		0,
		m_screensize.X,
		m_screensize.Y
	);

	driver->draw2DRectangle(left, GUI_BG_TOP, GUI_BG_TOP, GUI_BG_BTM, GUI_BG_BTM, &AbsoluteClippingRect);

	driver->draw2DRectangle(right, GUI_BG_TOP, GUI_BG_TOP, GUI_BG_BTM, GUI_BG_BTM, &AbsoluteClippingRect);

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

	gui::IGUIElement::draw();

	if (!m_model)
		return;

	m_model_rotation++;
	m_model->setRotation(v3f(0,m_model_rotation,0));
}

void GUICharDefMenu::acceptInput()
{
	//{
		//gui::IGUIElement *e = getElementFromId(GUI_ID_NAME_INPUT);
		//if (e != NULL) {
			//m_data->name = e->getText();
		//}else if (m_data->name == L"") {
			//m_data->name = std::wstring(L"singleplayer");
		//}
	//}


	m_accepted = true;
}

bool GUICharDefMenu::OnEvent(const SEvent& event)
{
	if (event.EventType==EET_KEY_INPUT_EVENT) {
		if (event.KeyInput.Key==KEY_ESCAPE && event.KeyInput.PressedDown) {
			acceptInput();
			quitMenu();
			return true;
		}
	}
	if (event.EventType==EET_GUI_EVENT) {
		if (event.GUIEvent.EventType==gui::EGET_ELEMENT_FOCUS_LOST && isVisible()) {
			if (!canTakeFocus(event.GUIEvent.Element)) {
				dstream<<"GUIMainMenu: Not allowing focus change."<<std::endl;
				// Returning true disables focus change
				return true;
			}
		}
		if (event.GUIEvent.EventType==gui::EGET_BUTTON_CLICKED) {
			switch (event.GUIEvent.Caller->getID()) {
			case GUI_ID_CD_MALE_BUTTON:
			{
				m_parts["gender"] = "M";
				m_parts["Yscale"] = "10";
				m_parts["XZscale"] = "10";
				m_y_scale = 1.0;
				m_xz_scale = 1.0;
				m_model->setMaterialTexture(0, getPlayerTexture());
				m_model->setScale(v3f(m_xz_scale,m_y_scale,m_xz_scale));
				printCharDef();
				return true;
			}
			case GUI_ID_CD_FEMALE_BUTTON:
			{
				m_parts["gender"] = "F";
				m_parts["Yscale"] = "10";
				m_parts["XZscale"] = "9";
				m_y_scale = 1.0;
				m_xz_scale = 0.9;
				m_model->setMaterialTexture(0, getPlayerTexture());
				m_model->setScale(v3f(m_xz_scale,m_y_scale,m_xz_scale));
				printCharDef();
				return true;
			}
			case GUI_ID_CD_TALLER_BUTTON:
			{
				if (m_y_scale >= 1.1)
					return true;
				m_y_scale += 0.1;
				m_parts["Yscale"] = ftos(m_y_scale*10.);
				m_model->setScale(v3f(m_xz_scale,m_y_scale,m_xz_scale));
				printCharDef();
				return true;
			}
			case GUI_ID_CD_SHORTER_BUTTON:
			{
				if (m_y_scale <= 0.8)
					return true;
				m_y_scale -= 0.1;
				m_parts["Yscale"] = ftos(m_y_scale*10.);
				m_model->setScale(v3f(m_xz_scale,m_y_scale,m_xz_scale));
				printCharDef();
				return true;
			}
			case GUI_ID_CD_WIDER_BUTTON:
			{
				if (m_xz_scale >= 1.1)
					return true;
				m_xz_scale += 0.1;
				m_parts["XZscale"] = ftos(m_xz_scale*10.);
				m_model->setScale(v3f(m_xz_scale,m_y_scale,m_xz_scale));
				printCharDef();
				return true;
			}
			case GUI_ID_CD_THINNER_BUTTON:
			{
				if (m_xz_scale <= 0.8)
					return true;
				m_xz_scale -= 0.1;
				m_parts["XZscale"] = ftos(m_xz_scale*10.);
				m_model->setScale(v3f(m_xz_scale,m_y_scale,m_xz_scale));
				printCharDef();
				return true;
			}
			case GUI_ID_CD_SKINTONE_PREV_BUTTON:
			{
				std::map<std::string,std::wstring>::iterator i = m_skintone_types.find(m_parts["skintone"]);
				if (i == m_skintone_types.begin())
					i = m_skintone_types.end();
				--i;
				m_parts["skintone"] = i->first;
				m_model->setMaterialTexture(0, getPlayerTexture());
				regenerateGui(m_screensize);
				printCharDef();
				return true;
			}
			case GUI_ID_CD_SKINTONE_NEXT_BUTTON:
			{
				std::map<std::string,std::wstring>::iterator i = m_skintone_types.find(m_parts["skintone"]);
				++i;
				if (i == m_skintone_types.end())
					i = m_skintone_types.begin();
				m_parts["skintone"] = i->first;
				m_model->setMaterialTexture(0, getPlayerTexture());
				regenerateGui(m_screensize);
				printCharDef();
				return true;
			}
			case GUI_ID_CD_FACE_PREV_BUTTON:
			{
				std::map<std::string,std::wstring>::iterator i = m_face_types.find(m_parts["face"]);
				if (i == m_face_types.begin())
					i = m_face_types.end();
				--i;
				m_parts["face"] = i->first;
				m_model->setMaterialTexture(0, getPlayerTexture());
				regenerateGui(m_screensize);
				printCharDef();
				return true;
			}
			case GUI_ID_CD_FACE_NEXT_BUTTON:
			{
				std::map<std::string,std::wstring>::iterator i = m_face_types.find(m_parts["face"]);
				++i;
				if (i == m_face_types.end())
					i = m_face_types.begin();
				m_parts["face"] = i->first;
				m_model->setMaterialTexture(0, getPlayerTexture());
				regenerateGui(m_screensize);
				printCharDef();
				return true;
			}
			case GUI_ID_CD_HAIRTONE_PREV_BUTTON:
			{
				std::map<std::string,std::wstring>::iterator i = m_hairtone_types.find(m_parts["hairtone"]);
				if (i == m_hairtone_types.begin())
					i = m_hairtone_types.end();
				--i;
				m_parts["hairtone"] = i->first;
				m_model->setMaterialTexture(0, getPlayerTexture());
				regenerateGui(m_screensize);
				printCharDef();
				return true;
			}
			case GUI_ID_CD_HAIRTONE_NEXT_BUTTON:
			{
				std::map<std::string,std::wstring>::iterator i = m_hairtone_types.find(m_parts["hairtone"]);
				++i;
				if (i == m_hairtone_types.end())
					i = m_hairtone_types.begin();
				m_parts["hairtone"] = i->first;
				m_model->setMaterialTexture(0, getPlayerTexture());
				regenerateGui(m_screensize);
				printCharDef();
				return true;
			}
			case GUI_ID_CD_HAIR_PREV_BUTTON:
			{
				std::map<std::string,std::wstring>::iterator i = m_hair_types.find(m_parts["hair"]);
				if (i == m_hair_types.begin())
					i = m_hair_types.end();
				--i;
				m_parts["hair"] = i->first;
				m_model->setMaterialTexture(0, getPlayerTexture());
				regenerateGui(m_screensize);
				printCharDef();
				return true;
			}
			case GUI_ID_CD_HAIR_NEXT_BUTTON:
			{
				std::map<std::string,std::wstring>::iterator i = m_hair_types.find(m_parts["hair"]);
				++i;
				if (i == m_hair_types.end())
					i = m_hair_types.begin();
				m_parts["hair"] = i->first;
				m_model->setMaterialTexture(0, getPlayerTexture());
				regenerateGui(m_screensize);
				printCharDef();
				return true;
			}
			case GUI_ID_CD_EYES_PREV_BUTTON:
			{
				std::map<std::string,std::wstring>::iterator i = m_eyes_types.find(m_parts["eyes"]);
				if (i == m_eyes_types.begin())
					i = m_eyes_types.end();
				--i;
				m_parts["eyes"] = i->first;
				m_model->setMaterialTexture(0, getPlayerTexture());
				regenerateGui(m_screensize);
				printCharDef();
				return true;
			}
			case GUI_ID_CD_EYES_NEXT_BUTTON:
			{
				std::map<std::string,std::wstring>::iterator i = m_eyes_types.find(m_parts["eyes"]);
				++i;
				if (i == m_eyes_types.end())
					i = m_eyes_types.begin();
				m_parts["eyes"] = i->first;
				m_model->setMaterialTexture(0, getPlayerTexture());
				regenerateGui(m_screensize);
				printCharDef();
				return true;
			}
			case GUI_ID_CD_SHIRT_PREV_BUTTON:
			{
				std::map<std::string,std::wstring>::iterator i = m_shirt_types.find(m_parts["shirt"]);
				if (i == m_shirt_types.begin())
					i = m_shirt_types.end();
				--i;
				m_parts["shirt"] = i->first;
				m_model->setMaterialTexture(0, getPlayerTexture());
				regenerateGui(m_screensize);
				printCharDef();
				return true;
			}
			case GUI_ID_CD_SHIRT_NEXT_BUTTON:
			{
				std::map<std::string,std::wstring>::iterator i = m_shirt_types.find(m_parts["shirt"]);
				++i;
				if (i == m_shirt_types.end())
					i = m_shirt_types.begin();
				m_parts["shirt"] = i->first;
				m_model->setMaterialTexture(0, getPlayerTexture());
				regenerateGui(m_screensize);
				printCharDef();
				return true;
			}
			case GUI_ID_CD_PANTS_PREV_BUTTON:
			{
				std::map<std::string,std::wstring>::iterator i = m_pants_types.find(m_parts["pants"]);
				if (i == m_pants_types.begin())
					i = m_pants_types.end();
				--i;
				m_parts["pants"] = i->first;
				m_model->setMaterialTexture(0, getPlayerTexture());
				regenerateGui(m_screensize);
				printCharDef();
				return true;
			}
			case GUI_ID_CD_PANTS_NEXT_BUTTON:
			{
				std::map<std::string,std::wstring>::iterator i = m_pants_types.find(m_parts["pants"]);
				++i;
				if (i == m_pants_types.end())
					i = m_pants_types.begin();
				m_parts["pants"] = i->first;
				m_model->setMaterialTexture(0, getPlayerTexture());
				regenerateGui(m_screensize);
				printCharDef();
				return true;
			}
			case GUI_ID_CD_SHOES_PREV_BUTTON:
			{
				std::map<std::string,std::wstring>::iterator i = m_shoes_types.find(m_parts["shoes"]);
				if (i == m_shoes_types.begin())
					i = m_shoes_types.end();
				--i;
				m_parts["shoes"] = i->first;
				m_model->setMaterialTexture(0, getPlayerTexture());
				regenerateGui(m_screensize);
				printCharDef();
				return true;
			}
			case GUI_ID_CD_SHOES_NEXT_BUTTON:
			{
				std::map<std::string,std::wstring>::iterator i = m_shoes_types.find(m_parts["shoes"]);
				++i;
				if (i == m_shoes_types.end())
					i = m_shoes_types.begin();
				m_parts["shoes"] = i->first;
				m_model->setMaterialTexture(0, getPlayerTexture());
				regenerateGui(m_screensize);
				printCharDef();
				return true;
			}
			case GUI_ID_CD_SAVE_BUTTON:
			{
				savePlayerSkin();
				acceptInput();
				quitMenu();
				return true;
			}
			case GUI_ID_CD_QUIT_BUTTON:
			{
				acceptInput();
				quitMenu();
				return true;
			}
			}
		}
	}

	return Parent ? Parent->OnEvent(event) : false;
}

std::string GUICharDefMenu::getPlayerSkin()
{
	std::string tex = "";

	tex += std::string("skins/")+"skintone_"+m_parts["skintone"]+".png";
	tex += std::string("^skins/")+"gender_"+m_parts["gender"]+".png";
	tex += std::string("^skins/")+"face_"+m_parts["face"]+"_"+m_parts["skintone"]+"_"+m_parts["gender"]+".png";
	tex += std::string("^skins/")+"eyes_"+m_parts["eyes"]+".png";
	tex += std::string("^clothes_player_pants_canvas_")+m_parts["pants"]+".png";
	tex += std::string("^clothes_player_tshirt_cotton_")+m_parts["shirt"]+".png";
	tex += std::string("^skins/")+"hair_"+m_parts["hair"]+"_"+m_parts["hairtone"]+"_"+m_parts["gender"]+".png";
	std::string c = "brown";
	if (m_parts["shoes"] == "canvas")
		c = "black";
	tex += std::string("^clothes_player_shoes_")+m_parts["shoes"]+"_"+c+".png";

	return tex;
}

//gender:Yscale:XZscale:skintone:eyes:hairtone:hair:face:shirt-colour:pants-colour:shoe-type
//M:10:10:fair:blue:brown:medium:normal:green:blue:leather
void GUICharDefMenu::fetchPlayerSkin()
{
	char buff[1024];
	char buf[256];
	char* v;
	std::string chardef = std::string(PLAYER_DEFAULT_CHARDEF);
	v = config_get("client.character");
	if (v)
		chardef = v;
	Strfnd f(chardef);

	m_parts["gender"] = f.next(":");
	m_parts["Yscale"] = f.next(":"); // y scale
	m_parts["XZscale"] = f.next(":"); // xz scale
	m_parts["skintone"] = f.next(":");
	m_parts["eyes"] = f.next(":");
	m_parts["hairtone"] = f.next(":");
	m_parts["hair"] = f.next(":");
	m_parts["face"] = f.next(":");
	m_parts["shirt"] = f.next(":");
	m_parts["pants"] = f.next(":");
	m_parts["shoes"] = f.next(":");

	if (m_parts["gender"] != "M" && m_parts["gender"] != "F")
		m_parts["gender"] = "M";

	m_y_scale = mystof(m_parts["Yscale"])/10.;
	if (m_y_scale < 0.8)
		m_y_scale = 0.8;
	if (m_y_scale > 1.1)
		m_y_scale = 1.1;
	m_parts["Yscale"] = ftos(m_y_scale*10.);

	m_xz_scale = mystof(m_parts["XZscale"])/10.;
	if (m_xz_scale < 0.8)
		m_xz_scale = 0.8;
	if (m_xz_scale > 1.1)
		m_xz_scale = 1.1;
	m_parts["XZscale"] = ftos(m_xz_scale*10.);

	snprintf(buf,256,"skintone_%s_%s.png",m_parts["skintone"].c_str(),m_parts["gender"].c_str());
	if (!path_get((char*)"skin",buf,1,buff,1024))
		m_parts["skintone"] = "fair";


	snprintf(buf,256,"eyes_%s.png",m_parts["eyes"].c_str());
	if (!path_get((char*)"skin",buf,1,buff,1024))
		m_parts["eyes"] = "blue";

	snprintf(buf,256,"hair_%s_%s_%s.png",m_parts["hair"].c_str(),m_parts["hairtone"].c_str(),m_parts["gender"].c_str());
	if (
		m_parts["hairtone"] == ""
		|| m_parts["hair"] == ""
		|| !path_get((char*)"skin",buf,1,buff,1024)
	) {
		m_parts["hairtone"] = "brown";
		m_parts["hair"] = "medium";
	}

	snprintf(buf,256,"face_%s_%s.png",m_parts["face"].c_str(),m_parts["gender"].c_str());
	if (!path_get((char*)"skin",buf,1,buff,1024))
		m_parts["face"] = "human";

	snprintf(buf,256,"clothes_player_tshirt_cotton_%s.png",m_parts["shirt"].c_str());
	if (!path_get((char*)"texture",buf,1,buff,1024))
		m_parts["shirt"] = "green";

	snprintf(buf,256,"clothes_player_pants_canvas_%s.png",m_parts["pants"].c_str());
	if (!path_get((char*)"texture",buf,1,buff,1024))
		m_parts["pants"] = "blue";

	std::string c = "brown";
	if (m_parts["shoes"] == "canvas")
		c = "black";

	snprintf(buf,256,"clothes_player_shoes_%s_%s.png",m_parts["shoes"].c_str(),c.c_str());
	if (!path_get((char*)"texture",buf,1,buff,1024))
		m_parts["shoes"] = "leather";
}

//gender:Yscale:XZscale:skintone:eyes:hairtone:hair:face
//M:10:10:fair:blue:brown:medium:normal
void GUICharDefMenu::savePlayerSkin()
{
	std::string chardef = m_parts["gender"];
	chardef += std::string(":")+m_parts["Yscale"];
	chardef += std::string(":")+m_parts["XZscale"];
	chardef += std::string(":")+m_parts["skintone"];
	chardef += std::string(":")+m_parts["eyes"];
	chardef += std::string(":")+m_parts["hairtone"];
	chardef += std::string(":")+m_parts["hair"];
	chardef += std::string(":")+m_parts["face"];
	chardef += std::string(":")+m_parts["shirt"];
	chardef += std::string(":")+m_parts["pants"];
	chardef += std::string(":")+m_parts["shoes"];

	config_set("client.character",(char*)chardef.c_str());
}
