/************************************************************************
* Minetest-c55
* Copyright (C) 2010 celeron55, Perttu Ahola <celeron55@gmail.com>
*
* content_mapnode_special.cpp
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
************************************************************************/

#include "content_mapnode.h"
#include "content_nodebox.h"
#include "content_list.h"
#include "content_craft.h"
#include "content_nodemeta.h"
#include "player.h"
#include "intl.h"
#include "content_burntimes.h"

void content_mapnode_sign(bool repeat)
{
	content_t i;
	ContentFeatures *f = NULL;

	i = CONTENT_SIGN;
	f = &content_features(i);
	f->description = gettext("Sign");
	f->setAllTextures("wood.png");
	f->param_type = CPT_LIGHT;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->air_equivalent = true;
	f->wallmount_alternate_node = CONTENT_SIGN_WALL;
	f->roofmount_alternate_node = CONTENT_SIGN_UD;
	f->param2_type = CPT_FACEDIR_SIMPLE;
	f->draw_type = CDT_NODEBOX;
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_SIGN;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	if(f->initial_metadata == NULL)
		f->initial_metadata = new SignNodeMetadata("Some sign");
	f->type = CMT_WOOD;
	f->dig_time = 0.1;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	f->alternate_lockstate_node = CONTENT_LOCKABLE_SIGN;
	content_nodebox_sign(f);
	f->setFaceText(5,FaceText(0.05,0.0675,0.95,0.55));
	f->setInventoryTextureNodeBox(i,"wood.png", "wood.png", "wood.png");
	crafting::setSignRecipe(CONTENT_CRAFTITEM_WOOD_PLANK,CONTENT_SIGN);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_SIGN_WALL;
	f = &content_features(i);
	f->description = gettext("Sign");
	f->setAllTextures("wood.png");
	f->param_type = CPT_LIGHT;
	f->param2_type = CPT_FACEDIR_WALLMOUNT;
	f->draw_type = CDT_NODEBOX;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->floormount_alternate_node = CONTENT_SIGN;
	f->roofmount_alternate_node = CONTENT_SIGN_UD;
	f->walkable = false;
	f->air_equivalent = true;
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_SIGN;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_SIGN)+" 1";
	if(f->initial_metadata == NULL)
		f->initial_metadata = new SignNodeMetadata("Some sign");
	f->type = CMT_WOOD;
	f->dig_time = 0.1;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	f->alternate_lockstate_node = CONTENT_LOCKABLE_SIGN_WALL;
	content_nodebox_sign_wall(f);
	f->setFaceText(5,FaceText(0.05,0.3,0.95,0.7));

	i = CONTENT_SIGN_UD;
	f = &content_features(i);
	f->description = gettext("Sign");
	f->setAllTextures("wood.png");
	f->param_type = CPT_LIGHT;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->air_equivalent = true;
	f->wallmount_alternate_node = CONTENT_SIGN_WALL;
	f->floormount_alternate_node = CONTENT_SIGN;
	f->param2_type = CPT_FACEDIR_SIMPLE;
	f->draw_type = CDT_NODEBOX;
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_SIGN;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_SIGN)+" 1";
	if(f->initial_metadata == NULL)
		f->initial_metadata = new SignNodeMetadata("Some sign");
	f->type = CMT_WOOD;
	f->dig_time = 0.1;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	f->alternate_lockstate_node = CONTENT_LOCKABLE_SIGN_UD;
	content_nodebox_sign_ud(f);
	f->setFaceText(5,FaceText(0.05,0.45,0.95,0.8875));

	i = CONTENT_SIGN_APPLE;
	f = &content_features(i);
	f->description = gettext("Sign");
	f->setAllTextures("applewood.png");
	f->param_type = CPT_LIGHT;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->air_equivalent = true;
	f->wallmount_alternate_node = CONTENT_SIGN_WALL_APPLE;
	f->roofmount_alternate_node = CONTENT_SIGN_UD_APPLE;
	f->param2_type = CPT_FACEDIR_SIMPLE;
	f->draw_type = CDT_NODEBOX;
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_SIGN;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	if(f->initial_metadata == NULL)
		f->initial_metadata = new SignNodeMetadata("Some sign");
	f->type = CMT_WOOD;
	f->dig_time = 0.1;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_nodebox_sign(f);
	f->setFaceText(5,FaceText(0.05,0.0675,0.95,0.55));
	f->setInventoryTextureNodeBox(i,"applewood.png", "applewood.png", "applewood.png");
	crafting::setSignRecipe(CONTENT_CRAFTITEM_APPLE_PLANK,CONTENT_SIGN_APPLE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_SIGN_WALL_APPLE;
	f = &content_features(i);
	f->description = gettext("Sign");
	f->setAllTextures("applewood.png");
	f->param_type = CPT_LIGHT;
	f->param2_type = CPT_FACEDIR_WALLMOUNT;
	f->draw_type = CDT_NODEBOX;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->floormount_alternate_node = CONTENT_SIGN_APPLE;
	f->roofmount_alternate_node = CONTENT_SIGN_UD_APPLE;
	f->walkable = false;
	f->air_equivalent = true;
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_SIGN;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_SIGN_APPLE)+" 1";
	if(f->initial_metadata == NULL)
		f->initial_metadata = new SignNodeMetadata("Some sign");
	f->type = CMT_WOOD;
	f->dig_time = 0.1;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_nodebox_sign_wall(f);
	f->setFaceText(5,FaceText(0.05,0.3,0.95,0.7));

	i = CONTENT_SIGN_UD_APPLE;
	f = &content_features(i);
	f->description = gettext("Sign");
	f->setAllTextures("applewood.png");
	f->param_type = CPT_LIGHT;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->air_equivalent = true;
	f->wallmount_alternate_node = CONTENT_SIGN_WALL_APPLE;
	f->floormount_alternate_node = CONTENT_SIGN_APPLE;
	f->param2_type = CPT_FACEDIR_SIMPLE;
	f->draw_type = CDT_NODEBOX;
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_SIGN;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_SIGN_APPLE)+" 1";
	if(f->initial_metadata == NULL)
		f->initial_metadata = new SignNodeMetadata("Some sign");
	f->type = CMT_WOOD;
	f->dig_time = 0.1;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_nodebox_sign_ud(f);
	f->setFaceText(5,FaceText(0.05,0.45,0.95,0.8875));

	i = CONTENT_SIGN_JUNGLE;
	f = &content_features(i);
	f->description = gettext("Sign");
	f->setAllTextures("junglewood.png");
	f->param_type = CPT_LIGHT;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->air_equivalent = true;
	f->wallmount_alternate_node = CONTENT_SIGN_WALL_JUNGLE;
	f->roofmount_alternate_node = CONTENT_SIGN_UD_JUNGLE;
	f->param2_type = CPT_FACEDIR_SIMPLE;
	f->draw_type = CDT_NODEBOX;
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_SIGN;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	if(f->initial_metadata == NULL)
		f->initial_metadata = new SignNodeMetadata("Some sign");
	f->type = CMT_WOOD;
	f->dig_time = 0.1;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_nodebox_sign(f);
	f->setFaceText(5,FaceText(0.05,0.0675,0.95,0.55));
	f->setInventoryTextureNodeBox(i,"junglewood.png", "junglewood.png", "junglewood.png");
	crafting::setSignRecipe(CONTENT_CRAFTITEM_JUNGLE_PLANK,CONTENT_SIGN_JUNGLE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_SIGN_WALL_JUNGLE;
	f = &content_features(i);
	f->description = gettext("Sign");
	f->setAllTextures("junglewood.png");
	f->param_type = CPT_LIGHT;
	f->param2_type = CPT_FACEDIR_WALLMOUNT;
	f->draw_type = CDT_NODEBOX;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->floormount_alternate_node = CONTENT_SIGN_JUNGLE;
	f->roofmount_alternate_node = CONTENT_SIGN_UD_JUNGLE;
	f->walkable = false;
	f->air_equivalent = true;
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_SIGN;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_SIGN_JUNGLE)+" 1";
	if(f->initial_metadata == NULL)
		f->initial_metadata = new SignNodeMetadata("Some sign");
	f->type = CMT_WOOD;
	f->dig_time = 0.1;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_nodebox_sign_wall(f);
	f->setFaceText(5,FaceText(0.05,0.3,0.95,0.7));

	i = CONTENT_SIGN_UD_JUNGLE;
	f = &content_features(i);
	f->description = gettext("Sign");
	f->setAllTextures("junglewood.png");
	f->param_type = CPT_LIGHT;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->air_equivalent = true;
	f->wallmount_alternate_node = CONTENT_SIGN_WALL_JUNGLE;
	f->floormount_alternate_node = CONTENT_SIGN_JUNGLE;
	f->param2_type = CPT_FACEDIR_SIMPLE;
	f->draw_type = CDT_NODEBOX;
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_SIGN;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_SIGN_JUNGLE)+" 1";
	if(f->initial_metadata == NULL)
		f->initial_metadata = new SignNodeMetadata("Some sign");
	f->type = CMT_WOOD;
	f->dig_time = 0.1;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_nodebox_sign_ud(f);
	f->setFaceText(5,FaceText(0.05,0.45,0.95,0.8875));

	i = CONTENT_SIGN_PINE;
	f = &content_features(i);
	f->description = gettext("Sign");
	f->setAllTextures("pine.png");
	f->param_type = CPT_LIGHT;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->air_equivalent = true;
	f->wallmount_alternate_node = CONTENT_SIGN_WALL_PINE;
	f->roofmount_alternate_node = CONTENT_SIGN_UD_PINE;
	f->param2_type = CPT_FACEDIR_SIMPLE;
	f->draw_type = CDT_NODEBOX;
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_SIGN;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	if(f->initial_metadata == NULL)
		f->initial_metadata = new SignNodeMetadata("Some sign");
	f->type = CMT_WOOD;
	f->dig_time = 0.1;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_nodebox_sign(f);
	f->setFaceText(5,FaceText(0.05,0.0675,0.95,0.55));
	f->setInventoryTextureNodeBox(i,"pine.png", "pine.png", "pine.png");
	crafting::setSignRecipe(CONTENT_CRAFTITEM_PINE_PLANK,CONTENT_SIGN_PINE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_SIGN_WALL_PINE;
	f = &content_features(i);
	f->description = gettext("Sign");
	f->setAllTextures("pine.png");
	f->param_type = CPT_LIGHT;
	f->param2_type = CPT_FACEDIR_WALLMOUNT;
	f->draw_type = CDT_NODEBOX;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->floormount_alternate_node = CONTENT_SIGN_PINE;
	f->roofmount_alternate_node = CONTENT_SIGN_UD_PINE;
	f->walkable = false;
	f->air_equivalent = true;
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_SIGN;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_SIGN_PINE)+" 1";
	if(f->initial_metadata == NULL)
		f->initial_metadata = new SignNodeMetadata("Some sign");
	f->type = CMT_WOOD;
	f->dig_time = 0.1;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_nodebox_sign_wall(f);
	f->setFaceText(5,FaceText(0.05,0.3,0.95,0.7));

	i = CONTENT_SIGN_UD_PINE;
	f = &content_features(i);
	f->description = gettext("Sign");
	f->setAllTextures("pine.png");
	f->param_type = CPT_LIGHT;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->air_equivalent = true;
	f->wallmount_alternate_node = CONTENT_SIGN_WALL_PINE;
	f->floormount_alternate_node = CONTENT_SIGN_PINE;
	f->param2_type = CPT_FACEDIR_SIMPLE;
	f->draw_type = CDT_NODEBOX;
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_SIGN;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_SIGN_PINE)+" 1";
	if(f->initial_metadata == NULL)
		f->initial_metadata = new SignNodeMetadata("Some sign");
	f->type = CMT_WOOD;
	f->dig_time = 0.1;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_nodebox_sign_ud(f);
	f->setFaceText(5,FaceText(0.05,0.45,0.95,0.8875));

	i = CONTENT_SIGN_IRON;
	f = &content_features(i);
	f->description = gettext("Sign");
	f->setAllTextures("iron_sheet.png");
	f->param_type = CPT_LIGHT;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->air_equivalent = true;
	f->wallmount_alternate_node = CONTENT_SIGN_WALL_IRON;
	f->roofmount_alternate_node = CONTENT_SIGN_UD_IRON;
	f->param2_type = CPT_FACEDIR_SIMPLE;
	f->draw_type = CDT_NODEBOX;
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_SIGN;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	if(f->initial_metadata == NULL)
		f->initial_metadata = new SignNodeMetadata("Some sign");
	f->type = CMT_STONE;
	f->dig_time = 0.1;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_nodebox_sign(f);
	f->setFaceText(5,FaceText(0.05,0.0675,0.95,0.55));
	f->setInventoryTextureNodeBox(i,"iron_sheet.png", "iron_sheet.png", "iron_sheet.png");
	crafting::setSignRecipe(CONTENT_CRAFTITEM_IRON_INGOT,CONTENT_SIGN_IRON);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_SIGN_WALL_IRON;
	f = &content_features(i);
	f->description = gettext("Sign");
	f->setAllTextures("iron_sheet.png");
	f->param_type = CPT_LIGHT;
	f->param2_type = CPT_FACEDIR_WALLMOUNT;
	f->draw_type = CDT_NODEBOX;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->floormount_alternate_node = CONTENT_SIGN_IRON;
	f->roofmount_alternate_node = CONTENT_SIGN_UD_IRON;
	f->walkable = false;
	f->air_equivalent = true;
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_SIGN;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_SIGN_IRON)+" 1";
	if(f->initial_metadata == NULL)
		f->initial_metadata = new SignNodeMetadata("Some sign");
	f->type = CMT_STONE;
	f->dig_time = 0.1;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_nodebox_sign_wall(f);
	f->setFaceText(5,FaceText(0.05,0.3,0.95,0.7));

	i = CONTENT_SIGN_UD_IRON;
	f = &content_features(i);
	f->description = gettext("Sign");
	f->setAllTextures("iron_sheet.png");
	f->param_type = CPT_LIGHT;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->air_equivalent = true;
	f->wallmount_alternate_node = CONTENT_SIGN_WALL_IRON;
	f->floormount_alternate_node = CONTENT_SIGN_IRON;
	f->param2_type = CPT_FACEDIR_SIMPLE;
	f->draw_type = CDT_NODEBOX;
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_SIGN;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_SIGN_IRON)+" 1";
	if(f->initial_metadata == NULL)
		f->initial_metadata = new SignNodeMetadata("Some sign");
	f->type = CMT_STONE;
	f->dig_time = 0.1;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_nodebox_sign_ud(f);
	f->setFaceText(5,FaceText(0.05,0.45,0.95,0.8875));

/* TODO: deprecate */

	i = CONTENT_LOCKABLE_SIGN_WALL;
	f = &content_features(i);
	f->description = gettext("Locking Sign");
	f->setAllTextures("wood.png");
	f->param_type = CPT_LIGHT;
	f->param2_type = CPT_FACEDIR_WALLMOUNT;
	f->draw_type = CDT_NODEBOX;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->air_equivalent = true;
	f->floormount_alternate_node = CONTENT_LOCKABLE_SIGN;
	f->roofmount_alternate_node = CONTENT_LOCKABLE_SIGN_UD;
	f->walkable = false;
	f->air_equivalent = true;
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_SIGN;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_LOCKABLE_SIGN)+" 1";
	if(f->initial_metadata == NULL)
		f->initial_metadata = new LockingSignNodeMetadata("Some sign");
	f->type = CMT_WOOD;
	f->dig_time = 0.1;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	f->alternate_lockstate_node = CONTENT_SIGN_WALL;
	content_nodebox_sign_wall(f);
	f->setFaceText(4,FaceText(0.05,0.3,0.95,0.7,FTT_OWNER));
	f->setFaceText(5,FaceText(0.05,0.3,0.95,0.7));

	i = CONTENT_LOCKABLE_SIGN;
	f = &content_features(i);
	f->description = gettext("Locking Sign");
	f->setAllTextures("wood.png");
	f->param_type = CPT_LIGHT;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->air_equivalent = true;
	f->wallmount_alternate_node = CONTENT_LOCKABLE_SIGN_WALL;
	f->roofmount_alternate_node = CONTENT_LOCKABLE_SIGN_UD;
	f->param2_type = CPT_FACEDIR_SIMPLE;
	f->draw_type = CDT_NODEBOX;
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_SIGN;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	if(f->initial_metadata == NULL)
		f->initial_metadata = new LockingSignNodeMetadata("Some sign");
	f->type = CMT_WOOD;
	f->dig_time = 0.1;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	f->alternate_lockstate_node = CONTENT_SIGN;
	content_nodebox_sign(f);
	f->setFaceText(4,FaceText(0.05,0.0675,0.95,0.55,FTT_OWNER));
	f->setFaceText(5,FaceText(0.05,0.0675,0.95,0.55));
	f->setInventoryTextureNodeBox(i,"wood.png", "wood.png", "wood.png");
	crafting::set1Any2Recipe(CONTENT_SIGN,CONTENT_CRAFTITEM_IRON_INGOT,CONTENT_LOCKABLE_SIGN);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_LOCKABLE_SIGN_UD;
	f = &content_features(i);
	f->description = gettext("Locking Sign");
	f->setAllTextures("wood.png");
	f->param_type = CPT_LIGHT;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->air_equivalent = true;
	f->wallmount_alternate_node = CONTENT_LOCKABLE_SIGN_WALL;
	f->floormount_alternate_node = CONTENT_LOCKABLE_SIGN;
	f->param2_type = CPT_FACEDIR_SIMPLE;
	f->draw_type = CDT_NODEBOX;
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_SIGN;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_LOCKABLE_SIGN)+" 1";
	if(f->initial_metadata == NULL)
		f->initial_metadata = new LockingSignNodeMetadata("Some sign");
	f->type = CMT_WOOD;
	f->dig_time = 0.1;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	f->alternate_lockstate_node = CONTENT_SIGN_UD;
	content_nodebox_sign_ud(f);
	f->setFaceText(4,FaceText(0.05,0.45,0.95,0.8875,FTT_OWNER));
	f->setFaceText(5,FaceText(0.05,0.45,0.95,0.8875));
}
