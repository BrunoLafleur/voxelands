/************************************************************************
* Minetest-c55
* Copyright (C) 2010 celeron55, Perttu Ahola <celeron55@gmail.com>
*
* content_mapnode_stair.cpp
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
#include "intl.h"
#include "content_burntimes.h"

void content_mapnode_stair(bool repeat)
{
	content_t i;
	ContentFeatures *f = NULL;

	// stairs
	i = CONTENT_ROUGHSTONE_STAIR;
	f = &content_features(i);
	f->description = gettext("Rough Stone Stair");
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->setAllTextures("roughstone.png");
	f->draw_type = CDT_STAIRLIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	content_nodebox_stair(f);
	f->setInventoryTextureNodeBox(i,"roughstone.png", "roughstone.png", "roughstone.png");
	f->type = CMT_STONE;
	f->dig_time = 0.9;
	f->onpunch_replace_node = CONTENT_ROUGHSTONE_STAIR_CORNER;
	f->onpunch_replace_respects_borderstone = true;
	crafting::setStairRecipe(CONTENT_ROUGHSTONE,CONTENT_ROUGHSTONE_STAIR);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_COBBLE_STAIR;
	f = &content_features(i);
	f->description = gettext("Cobble Stone Stair");
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->setAllTextures("cobble.png");
	f->draw_type = CDT_STAIRLIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	content_nodebox_stair(f);
	f->setInventoryTextureNodeBox(i,"cobble.png", "cobble.png", "cobble.png");
	f->type = CMT_STONE;
	f->dig_time = 0.9;
	f->onpunch_replace_node = CONTENT_COBBLE_STAIR_CORNER;
	f->onpunch_replace_respects_borderstone = true;
	crafting::setStairRecipe(CONTENT_COBBLE,CONTENT_COBBLE_STAIR);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_MOSSYCOBBLE_STAIR;
	f = &content_features(i);
	f->description = gettext("Mossy Cobble Stone Stair");
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->setAllTextures("mossycobble.png");
	f->draw_type = CDT_STAIRLIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	content_nodebox_stair(f);
	f->setInventoryTextureNodeBox(i,"mossycobble.png", "mossycobble.png", "mossycobble.png");
	f->type = CMT_STONE;
	f->dig_time = 0.8;
	f->onpunch_replace_node = CONTENT_MOSSYCOBBLE_STAIR_CORNER;
	f->onpunch_replace_respects_borderstone = true;
	crafting::setStairRecipe(CONTENT_MOSSYCOBBLE,CONTENT_MOSSYCOBBLE_STAIR);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_STONE_STAIR;
	f = &content_features(i);
	f->description = gettext("Stone Stair");
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->setAllTextures("stone.png");
	f->draw_type = CDT_STAIRLIKE;
	f->is_ground_content = true;
	f->often_contains_mineral = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	content_nodebox_stair(f);
	f->setInventoryTextureNodeBox(i,"stone.png", "stone.png", "stone.png");
	f->type = CMT_STONE;
	f->dig_time = 1.0;
	f->onpunch_replace_node = CONTENT_STONE_STAIR_CORNER;
	f->onpunch_replace_respects_borderstone = true;
	crafting::setStairRecipe(CONTENT_STONE,CONTENT_STONE_STAIR);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_APPLEWOOD_STAIR;
	f = &content_features(i);
	f->description = gettext("Apple wood Stair");
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->draw_type = CDT_STAIRLIKE;
	f->setAllTextures("applewood.png");
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	content_nodebox_stair(f);
	f->setInventoryTextureNodeBox(i,"applewood.png", "applewood.png", "applewood.png");
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_STAIR;
	f->type = CMT_WOOD;
	f->dig_time = 0.75;
	f->onpunch_replace_node = CONTENT_APPLEWOOD_STAIR_CORNER;
	f->onpunch_replace_respects_borderstone = true;
	crafting::setStairRecipe(CONTENT_APPLEWOOD,CONTENT_APPLEWOOD_STAIR);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_WOOD_STAIR;
	f = &content_features(i);
	f->description = gettext("Wood Stair");
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->draw_type = CDT_STAIRLIKE;
	f->setAllTextures("wood.png");
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	content_nodebox_stair(f);
	f->setInventoryTextureNodeBox(i,"wood.png", "wood.png", "wood.png");
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_STAIR;
	f->type = CMT_WOOD;
	f->dig_time = 0.75;
	f->onpunch_replace_node = CONTENT_WOOD_STAIR_CORNER;
	f->onpunch_replace_respects_borderstone = true;
	crafting::setStairRecipe(CONTENT_WOOD,CONTENT_WOOD_STAIR);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_JUNGLE_STAIR;
	f = &content_features(i);
	f->description = gettext("Jungle Wood Stair");
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->setAllTextures("junglewood.png");
	f->draw_type = CDT_STAIRLIKE;
	//f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	content_nodebox_stair(f);
	f->setInventoryTextureNodeBox(i,"junglewood.png", "junglewood.png", "junglewood.png");
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_STAIR;
	f->type = CMT_WOOD;
	f->dig_time = 1.0;
	f->onpunch_replace_node = CONTENT_JUNGLE_STAIR_CORNER;
	f->onpunch_replace_respects_borderstone = true;
	crafting::setStairRecipe(CONTENT_JUNGLEWOOD,CONTENT_JUNGLE_STAIR);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_BRICK_STAIR;
	f = &content_features(i);
	f->description = gettext("Brick Stair");
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->setAllTextures("brick.png");
	f->setTexture(0,"brick_stair_top.png");
	f->setTexture(1,"brick_bottom.png");
	f->setTexture(2,"brick_side.png");
	f->setTexture(3,"brick_side.png");
	f->setTexture(5,"brick_stair_front.png");
	f->draw_type = CDT_STAIRLIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_CLAY_BRICK)+" 4";
	content_nodebox_stair(f);
	f->setInventoryTextureNodeBox(i,"brick_stair_top.png", "brick_stair_front.png", "brick_side.png");
	f->type = CMT_STONE;
	f->dig_time = 1.0;
	f->onpunch_replace_node = CONTENT_BRICK_STAIR_CORNER;
	f->onpunch_replace_respects_borderstone = true;
	crafting::setStairRecipe(CONTENT_BRICK,CONTENT_BRICK_STAIR);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_SANDSTONE_STAIR;
	f = &content_features(i);
	f->description = gettext("Sand Stone Stair");
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->setAllTextures("sandstone.png");
	f->draw_type = CDT_STAIRLIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_SAND)+" 4";
	content_nodebox_stair(f);
	f->setInventoryTextureNodeBox(i,"sandstone.png", "sandstone.png", "sandstone.png");
	f->type = CMT_DIRT;
	f->dig_time = 1.0;
	f->onpunch_replace_node = CONTENT_SANDSTONE_STAIR_CORNER;
	f->onpunch_replace_respects_borderstone = true;
	crafting::setStairRecipe(CONTENT_SANDSTONE,CONTENT_SANDSTONE_STAIR);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_LIMESTONE_STAIR;
	f = &content_features(i);
	f->description = gettext("Limestone Stair");
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->setAllTextures("limestone.png");
	f->draw_type = CDT_STAIRLIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	content_nodebox_stair(f);
	f->setInventoryTextureNodeBox(i,"limestone.png", "limestone.png", "limestone.png");
	f->type = CMT_STONE;
	f->dig_time = 0.9;
	f->onpunch_replace_node = CONTENT_LIMESTONE_STAIR_CORNER;
	f->onpunch_replace_respects_borderstone = true;
	crafting::setStairRecipe(CONTENT_LIMESTONE,CONTENT_LIMESTONE_STAIR);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	// upside down stairs
	i = CONTENT_ROUGHSTONE_STAIR_UD;
	f = &content_features(i);
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->setAllTextures("roughstone.png");
	f->draw_type = CDT_STAIRLIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_ROUGHSTONE_STAIR)+" 1";
	content_nodebox_stairud(f);
	f->setInventoryTextureNodeBox(i,"roughstone.png", "roughstone.png", "roughstone.png");
	f->type = CMT_STONE;
	f->dig_time = 0.9;
	f->onpunch_replace_node = CONTENT_ROUGHSTONE_STAIR_CORNER_UD;
	f->onpunch_replace_respects_borderstone = true;
	f->suffocation_per_second = 0;

	i = CONTENT_COBBLE_STAIR_UD;
	f = &content_features(i);
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->setAllTextures("cobble.png");
	f->draw_type = CDT_STAIRLIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_COBBLE_STAIR)+" 1";
	content_nodebox_stairud(f);
	f->setInventoryTextureNodeBox(i,"cobble.png", "cobble.png", "cobble.png");
	f->type = CMT_STONE;
	f->dig_time = 0.9;
	f->onpunch_replace_node = CONTENT_COBBLE_STAIR_CORNER_UD;
	f->onpunch_replace_respects_borderstone = true;
	f->suffocation_per_second = 0;

	i = CONTENT_MOSSYCOBBLE_STAIR_UD;
	f = &content_features(i);
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->setAllTextures("mossycobble.png");
	f->draw_type = CDT_STAIRLIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_MOSSYCOBBLE_STAIR)+" 1";
	content_nodebox_stairud(f);
	f->setInventoryTextureNodeBox(i,"mossycobble.png", "mossycobble.png", "mossycobble.png");
	f->type = CMT_STONE;
	f->dig_time = 0.8;
	f->onpunch_replace_node = CONTENT_MOSSYCOBBLE_STAIR_CORNER_UD;
	f->onpunch_replace_respects_borderstone = true;
	f->suffocation_per_second = 0;

	i = CONTENT_STONE_STAIR_UD;
	f = &content_features(i);
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->setAllTextures("stone.png");
	f->draw_type = CDT_STAIRLIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_STONE_STAIR)+" 1";
	content_nodebox_stairud(f);
	f->setInventoryTextureNodeBox(i,"stone.png", "stone.png", "stone.png");
	f->type = CMT_STONE;
	f->dig_time = 1.0;
	f->onpunch_replace_node = CONTENT_STONE_STAIR_CORNER_UD;
	f->onpunch_replace_respects_borderstone = true;
	f->suffocation_per_second = 0;

	i = CONTENT_APPLEWOOD_STAIR_UD;
	f = &content_features(i);
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->setAllTextures("applewood.png");
	f->draw_type = CDT_STAIRLIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_APPLEWOOD_STAIR)+" 1";
	content_nodebox_stairud(f);
	f->setInventoryTextureNodeBox(i,"applewood.png", "applewood.png", "applewood.png");
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_STAIR;
	f->type = CMT_WOOD;
	f->dig_time = 0.75;
	f->onpunch_replace_node = CONTENT_APPLEWOOD_STAIR_CORNER_UD;
	f->onpunch_replace_respects_borderstone = true;
	f->suffocation_per_second = 0;

	i = CONTENT_WOOD_STAIR_UD;
	f = &content_features(i);
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->setAllTextures("wood.png");
	f->draw_type = CDT_STAIRLIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_WOOD_STAIR)+" 1";
	content_nodebox_stairud(f);
	f->setInventoryTextureNodeBox(i,"wood.png", "wood.png", "wood.png");
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_STAIR;
	f->type = CMT_WOOD;
	f->dig_time = 0.75;
	f->onpunch_replace_node = CONTENT_WOOD_STAIR_CORNER_UD;
	f->onpunch_replace_respects_borderstone = true;
	f->suffocation_per_second = 0;

	i = CONTENT_JUNGLE_STAIR_UD;
	f = &content_features(i);
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->setAllTextures("junglewood.png");
	f->draw_type = CDT_STAIRLIKE;
	//f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_JUNGLE_STAIR)+" 1";
	content_nodebox_stairud(f);
	f->setInventoryTextureNodeBox(i,"junglewood.png", "junglewood.png", "junglewood.png");
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_STAIR;
	f->type = CMT_WOOD;
	f->dig_time = 1.0;
	f->onpunch_replace_node = CONTENT_JUNGLE_STAIR_CORNER_UD;
	f->onpunch_replace_respects_borderstone = true;
	f->suffocation_per_second = 0;

	i = CONTENT_BRICK_STAIR_UD;
	f = &content_features(i);
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->setAllTextures("brick.png^[transformfy");
	f->setTexture(0,"brick_stair_top.png");
	f->setTexture(1,"brick_top.png");
	f->setTexture(2,"brick_side.png^[transformfy");
	f->setTexture(3,"brick_side.png^[transformfy");
	f->setTexture(5,"brick_stair_front.png^[transformfy");
	f->draw_type = CDT_STAIRLIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_CLAY_BRICK)+" 4";
	content_nodebox_stairud(f);
	f->setInventoryTextureNodeBox(i,"brick.png", "brick_front.png", "brick.png");
	f->type = CMT_STONE;
	f->dig_time = 1.0;
	f->onpunch_replace_node = CONTENT_BRICK_STAIR_CORNER_UD;
	f->onpunch_replace_respects_borderstone = true;
	f->suffocation_per_second = 0;

	i = CONTENT_SANDSTONE_STAIR_UD;
	f = &content_features(i);
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->setAllTextures("sandstone.png");
	f->draw_type = CDT_STAIRLIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_SAND)+" 4";
	content_nodebox_stairud(f);
	f->setInventoryTextureNodeBox(i,"sandstone.png", "sandstone.png", "sandstone.png");
	f->type = CMT_DIRT;
	f->dig_time = 1.0;
	f->onpunch_replace_node = CONTENT_SANDSTONE_STAIR_CORNER_UD;
	f->onpunch_replace_respects_borderstone = true;
	f->suffocation_per_second = 0;

	i = CONTENT_LIMESTONE_STAIR_UD;
	f = &content_features(i);
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->setAllTextures("limestone.png");
	f->draw_type = CDT_STAIRLIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_LIMESTONE_STAIR)+" 1";
	content_nodebox_stairud(f);
	f->setInventoryTextureNodeBox(i,"limestone.png", "limestone.png", "limestone.png");
	f->type = CMT_STONE;
	f->dig_time = 0.9;
	f->onpunch_replace_node = CONTENT_LIMESTONE_STAIR_CORNER_UD;
	f->onpunch_replace_respects_borderstone = true;
	f->suffocation_per_second = 0;

	// Stairs' corners

	i = CONTENT_ROUGHSTONE_STAIR_CORNER;
	f = &content_features(i);
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->setAllTextures("roughstone.png");
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_ROUGHSTONE_STAIR)+" 1";
	content_nodebox_stair_corner(f);
	f->setInventoryTextureNodeBox(i,"roughstone.png","roughstone.png","roughstone.png");
	f->type = CMT_STONE;
	f->dig_time = 0.9;
	f->onpunch_replace_node = CONTENT_ROUGHSTONE_INNER_STAIR_CORNER;
	f->onpunch_replace_respects_borderstone = true;


	i = CONTENT_COBBLE_STAIR_CORNER;
	f = &content_features(i);
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->setAllTextures("cobble.png");
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_COBBLE_STAIR)+" 1";
	content_nodebox_stair_corner(f);
	f->setInventoryTextureNodeBox(i,"cobble.png","cobble.png","cobble.png");
	f->type = CMT_STONE;
	f->dig_time = 0.9;
	f->onpunch_replace_node = CONTENT_COBBLE_INNER_STAIR_CORNER;
	f->onpunch_replace_respects_borderstone = true;


	i = CONTENT_MOSSYCOBBLE_STAIR_CORNER;
	f = &content_features(i);
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->setAllTextures("mossycobble.png");
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_MOSSYCOBBLE_STAIR)+" 1";
	content_nodebox_stair_corner(f);
	f->setInventoryTextureNodeBox(i,"mossycobble.png","mossycobble.png","mossycobble.png");
	f->type = CMT_STONE;
	f->dig_time = 0.8;
	f->onpunch_replace_node = CONTENT_MOSSYCOBBLE_INNER_STAIR_CORNER;
	f->onpunch_replace_respects_borderstone = true;


	i = CONTENT_STONE_STAIR_CORNER;
	f = &content_features(i);
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->setAllTextures("stone.png");
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_STONE_STAIR)+" 1";
	content_nodebox_stair_corner(f);
	f->setInventoryTextureNodeBox(i,"stone.png","stone.png","stone.png");
	f->type = CMT_STONE;
	f->dig_time = 1.0;
	f->onpunch_replace_node = CONTENT_STONE_INNER_STAIR_CORNER;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_APPLEWOOD_STAIR_CORNER;
	f = &content_features(i);
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->draw_type = CDT_NODEBOX;
	f->setAllTextures("applewood.png");
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_APPLEWOOD_STAIR)+" 1";
	content_nodebox_stair_corner(f);
	f->setInventoryTextureNodeBox(i,"applewood.png","applewood.png","applewood.png");
	f->flammable = 1; // can be replaced by fire if the node under
					  // it is set on fire
	f->fuel_time = BT_STAIR;
	f->type = CMT_WOOD;
	f->dig_time = 0.75;
	f->onpunch_replace_node = CONTENT_APPLEWOOD_INNER_STAIR_CORNER;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_WOOD_STAIR_CORNER;
	f = &content_features(i);
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->draw_type = CDT_NODEBOX;
	f->setAllTextures("wood.png");
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_WOOD_STAIR)+" 1";
	content_nodebox_stair_corner(f);
	f->setInventoryTextureNodeBox(i,"wood.png","wood.png","wood.png");
	f->flammable = 1; // can be replaced by fire if the node under
					  // it is set on fire
	f->fuel_time = BT_STAIR;
	f->type = CMT_WOOD;
	f->dig_time = 0.75;
	f->onpunch_replace_node = CONTENT_WOOD_INNER_STAIR_CORNER;
	f->onpunch_replace_respects_borderstone = true;


	i = CONTENT_JUNGLE_STAIR_CORNER;
	f = &content_features(i);
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->setAllTextures("junglewood.png");
	f->draw_type = CDT_NODEBOX;
	// f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_JUNGLE_STAIR)+" 1";
	content_nodebox_stair_corner(f);
	f->setInventoryTextureNodeBox(i,"junglewood.png","junglewood.png","junglewood.png");
	f->flammable = 1; // can be replaced by fire if the node under
					  // it is set on fire
	f->fuel_time = BT_STAIR;
	f->type = CMT_WOOD;
	f->dig_time = 1.0;
	f->onpunch_replace_node = CONTENT_JUNGLE_INNER_STAIR_CORNER;
	f->onpunch_replace_respects_borderstone = true;


	i = CONTENT_BRICK_STAIR_CORNER;
	f = &content_features(i);
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->rotate_tile_with_nodebox = true;
	f->setAllTextures("brick.png");
	f->setTexture(1,"brick_bottom.png");
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_CLAY_BRICK)+" 4";
	content_nodebox_stair_corner(f);
	f->setInventoryTextureNodeBox(i,"brick.png","brick.png","brick.png");
	f->type = CMT_STONE;
	f->dig_time = 1.0;
	f->onpunch_replace_node = CONTENT_BRICK_INNER_STAIR_CORNER;
	f->onpunch_replace_respects_borderstone = true;


	i = CONTENT_SANDSTONE_STAIR_CORNER;
	f = &content_features(i);
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->setAllTextures("sandstone.png");
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_SAND)+" 4";
	content_nodebox_stair_corner(f);
	f->setInventoryTextureNodeBox(i,"sandstone.png","sandstone.png","sandstone.png");
	f->type = CMT_DIRT;
	f->dig_time = 1.0;
	f->onpunch_replace_node = CONTENT_SANDSTONE_INNER_STAIR_CORNER;
	f->onpunch_replace_respects_borderstone = true;


	i = CONTENT_LIMESTONE_STAIR_CORNER;
	f = &content_features(i);
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->setAllTextures("limestone.png");
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_LIMESTONE_STAIR)+" 1";
	content_nodebox_stair_corner(f);
	f->setInventoryTextureNodeBox(i,"limestone.png","limestone.png","limestone.png");
	f->type = CMT_STONE;
	f->dig_time = 0.9;
	f->onpunch_replace_node = CONTENT_LIMESTONE_INNER_STAIR_CORNER;
	f->onpunch_replace_respects_borderstone = true;

	// Upside down stairs' corners

	i = CONTENT_ROUGHSTONE_STAIR_CORNER_UD;
	f = &content_features(i);
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->setAllTextures("roughstone.png");
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_ROUGHSTONE_STAIR)+" 1";
	content_nodebox_stair_cornerud(f);
	f->setInventoryTextureNodeBox(i,"roughstone.png","roughstone.png","roughstone.png");
	f->type = CMT_STONE;
	f->dig_time = 0.9;
	f->onpunch_replace_node = CONTENT_ROUGHSTONE_INNER_STAIR_CORNER_UD;
	f->onpunch_replace_respects_borderstone = true;


	i = CONTENT_COBBLE_STAIR_CORNER_UD;
	f = &content_features(i);
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->setAllTextures("cobble.png");
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_COBBLE_STAIR)+" 1";
	content_nodebox_stair_cornerud(f);
	f->setInventoryTextureNodeBox(i,"cobble.png","cobble.png","cobble.png");
	f->type = CMT_STONE;
	f->dig_time = 0.9;
	f->onpunch_replace_node = CONTENT_COBBLE_INNER_STAIR_CORNER_UD;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_MOSSYCOBBLE_STAIR_CORNER_UD;
	f = &content_features(i);
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->setAllTextures("mossycobble.png");
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_MOSSYCOBBLE_STAIR)+" 1";
	content_nodebox_stair_cornerud(f);
	f->setInventoryTextureNodeBox(i,"mossycobble.png","mossycobble.png","mossycobble.png");
	f->type = CMT_STONE;
	f->dig_time = 0.8;
	f->onpunch_replace_node = CONTENT_MOSSYCOBBLE_INNER_STAIR_CORNER_UD;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_STONE_STAIR_CORNER_UD;
	f = &content_features(i);
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->setAllTextures("stone.png");
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_STONE_STAIR)+" 1";
	content_nodebox_stair_cornerud(f);
	f->setInventoryTextureNodeBox(i,"stone.png","stone.png","stone.png");
	f->type = CMT_STONE;
	f->dig_time = 1.0;
	f->onpunch_replace_node = CONTENT_STONE_INNER_STAIR_CORNER_UD;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_APPLEWOOD_STAIR_CORNER_UD;
	f = &content_features(i);
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->draw_type = CDT_NODEBOX;
	f->setAllTextures("applewood.png");
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_APPLEWOOD_STAIR)+" 1";
	content_nodebox_stair_cornerud(f);
	f->setInventoryTextureNodeBox(i,"applewood.png","applewood.png","applewood.png");
	f->flammable = 1; // can be replaced by fire if the node under
					  // it is set on fire
	f->fuel_time = BT_STAIR;
	f->type = CMT_WOOD;
	f->dig_time = 0.75;
	f->onpunch_replace_node = CONTENT_APPLEWOOD_INNER_STAIR_CORNER_UD;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_WOOD_STAIR_CORNER_UD;
	f = &content_features(i);
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->draw_type = CDT_NODEBOX;
	f->setAllTextures("wood.png");
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_WOOD_STAIR)+" 1";
	content_nodebox_stair_cornerud(f);
	f->setInventoryTextureNodeBox(i,"wood.png","wood.png","wood.png");
	f->flammable = 1; // can be replaced by fire if the node under
					  // it is set on fire
	f->fuel_time = BT_STAIR;
	f->type = CMT_WOOD;
	f->dig_time = 0.75;
	f->onpunch_replace_node = CONTENT_WOOD_INNER_STAIR_CORNER_UD;
	f->onpunch_replace_respects_borderstone = true;


	i = CONTENT_JUNGLE_STAIR_CORNER_UD;
	f = &content_features(i);
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->setAllTextures("junglewood.png");
	f->draw_type = CDT_NODEBOX;
	// f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_JUNGLE_STAIR)+" 1";
	content_nodebox_stair_cornerud(f);
	f->setInventoryTextureNodeBox(i,"junglewood.png","junglewood.png","junglewood.png");
	f->flammable = 1; // can be replaced by fire if the node under
					  // it is set on fire
	f->fuel_time = BT_STAIR;
	f->type = CMT_WOOD;
	f->dig_time = 1.0;
	f->onpunch_replace_node = CONTENT_JUNGLE_INNER_STAIR_CORNER_UD;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_BRICK_STAIR_CORNER_UD;
	f = &content_features(i);
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->rotate_tile_with_nodebox = true;
	f->setAllTextures("brick.png");
	f->setTexture(1,"brick_bottom.png");
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_CLAY_BRICK)+" 4";
	content_nodebox_stair_cornerud(f);
	f->setInventoryTextureNodeBox(i,"brick.png","brick.png","brick.png");
	f->type = CMT_STONE;
	f->dig_time = 1.0;
	f->onpunch_replace_node = CONTENT_BRICK_INNER_STAIR_CORNER_UD;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_SANDSTONE_STAIR_CORNER_UD;
	f = &content_features(i);
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->setAllTextures("sandstone.png");
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_SAND)+" 4";
	content_nodebox_stair_cornerud(f);
	f->setInventoryTextureNodeBox(i,"sandstone.png","sandstone.png","sandstone.png");
	f->type = CMT_DIRT;
	f->dig_time = 1.0;
	f->onpunch_replace_node = CONTENT_SANDSTONE_INNER_STAIR_CORNER_UD;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_LIMESTONE_STAIR_CORNER_UD;
	f = &content_features(i);
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->setAllTextures("limestone.png");
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_LIMESTONE_STAIR)+" 1";
	content_nodebox_stair_cornerud(f);
	f->setInventoryTextureNodeBox(i,"limestone.png","limestone.png","limestone.png");
	f->type = CMT_STONE;
	f->dig_time = 0.9;
	f->onpunch_replace_node = CONTENT_LIMESTONE_INNER_STAIR_CORNER_UD;
	f->onpunch_replace_respects_borderstone = true;

	// Inner stairs' corners

	i = CONTENT_ROUGHSTONE_INNER_STAIR_CORNER;
	f = &content_features(i);
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->setAllTextures("roughstone.png");
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_ROUGHSTONE_STAIR)+" 1";
	content_nodebox_inner_stair_corner(f);
	f->setInventoryTextureNodeBox(i,"roughstone.png","roughstone.png","roughstone.png");
	f->type = CMT_STONE;
	f->dig_time = 0.9;
	f->onpunch_replace_node = CONTENT_ROUGHSTONE_STAIR;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_COBBLE_INNER_STAIR_CORNER;
	f = &content_features(i);
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->setAllTextures("cobble.png");
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_COBBLE_STAIR)+" 1";
	content_nodebox_inner_stair_corner(f);
	f->setInventoryTextureNodeBox(i,"cobble.png","cobble.png","cobble.png");
	f->type = CMT_STONE;
	f->dig_time = 0.9;
	f->onpunch_replace_node = CONTENT_COBBLE_STAIR;
	f->onpunch_replace_respects_borderstone = true;


	i = CONTENT_MOSSYCOBBLE_INNER_STAIR_CORNER;
	f = &content_features(i);
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->setAllTextures("mossycobble.png");
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_MOSSYCOBBLE_STAIR)+" 1";
	content_nodebox_inner_stair_corner(f);
	f->setInventoryTextureNodeBox(i,"mossycobble.png","mossycobble.png","mossycobble.png");
	f->type = CMT_STONE;
	f->dig_time = 0.8;
	f->onpunch_replace_node = CONTENT_MOSSYCOBBLE_STAIR;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_STONE_INNER_STAIR_CORNER;
	f = &content_features(i);
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->setAllTextures("stone.png");
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_STONE_STAIR)+" 1";
	content_nodebox_inner_stair_corner(f);
	f->setInventoryTextureNodeBox(i,"stone.png","stone.png","stone.png");
	f->type = CMT_STONE;
	f->dig_time = 1.0;
	f->onpunch_replace_node = CONTENT_STONE_STAIR;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_APPLEWOOD_INNER_STAIR_CORNER;
	f = &content_features(i);
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->draw_type = CDT_NODEBOX;
	f->setAllTextures("applewood.png");
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_APPLEWOOD_STAIR)+" 1";
	content_nodebox_inner_stair_corner(f);
	f->setInventoryTextureNodeBox(i,"applewood.png","applewood.png","applewood.png");
	f->flammable = 1; // can be replaced by fire if the node under
					  // it is set on fire
	f->fuel_time = BT_STAIR;
	f->type = CMT_WOOD;
	f->dig_time = 0.75;
	f->onpunch_replace_node = CONTENT_APPLEWOOD_STAIR;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_WOOD_INNER_STAIR_CORNER;
	f = &content_features(i);
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->draw_type = CDT_NODEBOX;
	f->setAllTextures("wood.png");
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_WOOD_STAIR)+" 1";
	content_nodebox_inner_stair_corner(f);
	f->setInventoryTextureNodeBox(i,"wood.png","wood.png","wood.png");
	f->flammable = 1; // can be replaced by fire if the node under
					  // it is set on fire
	f->fuel_time = BT_STAIR;
	f->type = CMT_WOOD;
	f->dig_time = 0.75;
	f->onpunch_replace_node = CONTENT_WOOD_STAIR;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_JUNGLE_INNER_STAIR_CORNER;
	f = &content_features(i);
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->setAllTextures("junglewood.png");
	f->draw_type = CDT_NODEBOX;
	// f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_JUNGLE_STAIR)+" 1";
	content_nodebox_inner_stair_corner(f);
	f->setInventoryTextureNodeBox(i,"junglewood.png","junglewood.png","junglewood.png");
	f->flammable = 1; // can be replaced by fire if the node under
					  // it is set on fire
	f->fuel_time = BT_STAIR;
	f->type = CMT_WOOD;
	f->dig_time = 1.0;
	f->onpunch_replace_node = CONTENT_JUNGLE_STAIR;
	f->onpunch_replace_respects_borderstone = true;


	i = CONTENT_BRICK_INNER_STAIR_CORNER;
	f = &content_features(i);
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->rotate_tile_with_nodebox = true;
	f->setAllTextures("brick.png");
	f->setTexture(0,"brick_stair_inner_corner_top.png");
	f->setTexture(1,"brick_bottom.png");
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_CLAY_BRICK)+" 4";
	content_nodebox_inner_stair_corner(f);
	f->setInventoryTextureNodeBox(i,"brick.png","brick.png","brick.png");
	f->type = CMT_STONE;
	f->dig_time = 1.0;
	f->onpunch_replace_node = CONTENT_BRICK_STAIR;
	f->onpunch_replace_respects_borderstone = true;


	i = CONTENT_SANDSTONE_INNER_STAIR_CORNER;
	f = &content_features(i);
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->setAllTextures("sandstone.png");
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_SAND)+" 4";
	content_nodebox_inner_stair_corner(f);
	f->setInventoryTextureNodeBox(i,"sandstone.png","sandstone.png","sandstone.png");
	f->type = CMT_DIRT;
	f->dig_time = 1.0;
	f->onpunch_replace_node = CONTENT_SANDSTONE_STAIR;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_LIMESTONE_INNER_STAIR_CORNER;
	f = &content_features(i);
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->setAllTextures("limestone.png");
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_LIMESTONE_STAIR)+" 1";
	content_nodebox_inner_stair_corner(f);
	f->setInventoryTextureNodeBox(i,"limestone.png","limestone.png","limestone.png");
	f->type = CMT_STONE;
	f->dig_time = 0.9;
	f->onpunch_replace_node = CONTENT_LIMESTONE_STAIR;
	f->onpunch_replace_respects_borderstone = true;

	// Upside down inner stairs' corners

	i = CONTENT_ROUGHSTONE_INNER_STAIR_CORNER_UD;
	f = &content_features(i);
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->setAllTextures("roughstone.png");
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_ROUGHSTONE_STAIR)+" 1";
	content_nodebox_inner_stair_cornerud(f);
	f->setInventoryTextureNodeBox(i,"roughstone.png","roughstone.png","roughstone.png");
	f->type = CMT_STONE;
	f->dig_time = 0.9;
	f->onpunch_replace_node = CONTENT_ROUGHSTONE_STAIR_UD;
	f->onpunch_replace_respects_borderstone = true;


	i = CONTENT_COBBLE_INNER_STAIR_CORNER_UD;
	f = &content_features(i);
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->setAllTextures("cobble.png");
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_COBBLE_STAIR)+" 1";
	content_nodebox_inner_stair_cornerud(f);
	f->setInventoryTextureNodeBox(i,"cobble.png","cobble.png","cobble.png");
	f->type = CMT_STONE;
	f->dig_time = 0.9;
	f->onpunch_replace_node = CONTENT_COBBLE_STAIR_UD;
	f->onpunch_replace_respects_borderstone = true;


	i = CONTENT_MOSSYCOBBLE_INNER_STAIR_CORNER_UD;
	f = &content_features(i);
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->setAllTextures("mossycobble.png");
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_MOSSYCOBBLE_STAIR)+" 1";
	content_nodebox_inner_stair_cornerud(f);
	f->setInventoryTextureNodeBox(i,"mossycobble.png","mossycobble.png","mossycobble.png");
	f->type = CMT_STONE;
	f->dig_time = 0.8;
	f->onpunch_replace_node = CONTENT_MOSSYCOBBLE_STAIR_UD;
	f->onpunch_replace_respects_borderstone = true;


	i = CONTENT_STONE_INNER_STAIR_CORNER_UD;
	f = &content_features(i);
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->setAllTextures("stone.png");
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_STONE_STAIR)+" 1";
	content_nodebox_inner_stair_cornerud(f);
	f->setInventoryTextureNodeBox(i,"stone.png","stone.png","stone.png");
	f->type = CMT_STONE;
	f->dig_time = 1.0;
	f->onpunch_replace_node = CONTENT_STONE_STAIR_UD;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_APPLEWOOD_INNER_STAIR_CORNER_UD;
	f = &content_features(i);
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->draw_type = CDT_NODEBOX;
	f->setAllTextures("applewood.png");
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_APPLEWOOD_STAIR)+" 1";
	content_nodebox_inner_stair_cornerud(f);
	f->setInventoryTextureNodeBox(i,"applewood.png","applewood.png","applewood.png");
	f->flammable = 1; // can be replaced by fire if the node under
					  // it is set on fire
	f->fuel_time = BT_STAIR;
	f->type = CMT_WOOD;
	f->dig_time = 0.75;
	f->onpunch_replace_node = CONTENT_APPLEWOOD_STAIR_UD;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_WOOD_INNER_STAIR_CORNER_UD;
	f = &content_features(i);
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->draw_type = CDT_NODEBOX;
	f->setAllTextures("wood.png");
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_WOOD_STAIR)+" 1";
	content_nodebox_inner_stair_cornerud(f);
	f->setInventoryTextureNodeBox(i,"wood.png","wood.png","wood.png");
	f->flammable = 1; // can be replaced by fire if the node under
					  // it is set on fire
	f->fuel_time = BT_STAIR;
	f->type = CMT_WOOD;
	f->dig_time = 0.75;
	f->onpunch_replace_node = CONTENT_WOOD_STAIR_UD;
	f->onpunch_replace_respects_borderstone = true;


	i = CONTENT_JUNGLE_INNER_STAIR_CORNER_UD;
	f = &content_features(i);
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->setAllTextures("junglewood.png");
	f->draw_type = CDT_NODEBOX;
	// f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_JUNGLE_STAIR)+" 1";
	content_nodebox_inner_stair_cornerud(f);
	f->setInventoryTextureNodeBox(i,"junglewood.png","junglewood.png","junglewood.png");
	f->flammable = 1; // can be replaced by fire if the node under
					  // it is set on fire
	f->fuel_time = BT_STAIR;
	f->type = CMT_WOOD;
	f->dig_time = 1.0;
	f->onpunch_replace_node = CONTENT_JUNGLE_STAIR_UD;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_BRICK_INNER_STAIR_CORNER_UD;
	f = &content_features(i);
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->rotate_tile_with_nodebox = true;
	f->setAllTextures("brick.png");
	f->setTexture(1,"brick_stair_ud_inner_corner_top.png");
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_CLAY_BRICK)+" 4";
	content_nodebox_inner_stair_cornerud(f);
	f->setInventoryTextureNodeBox(i,"brick.png","brick.png","brick.png");
	f->type = CMT_STONE;
	f->dig_time = 1.0;
	f->onpunch_replace_node = CONTENT_BRICK_STAIR_UD;
	f->onpunch_replace_respects_borderstone = true;


	i = CONTENT_SANDSTONE_INNER_STAIR_CORNER_UD;
	f = &content_features(i);
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->setAllTextures("sandstone.png");
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_SAND)+ " 4";
	content_nodebox_inner_stair_cornerud(f);
	f->setInventoryTextureNodeBox(i,"sandstone.png","sandstone.png","sandstone.png");
	f->type = CMT_DIRT;
	f->dig_time = 1.0;
	f->onpunch_replace_node = CONTENT_SANDSTONE_STAIR_UD;
	f->onpunch_replace_respects_borderstone = true;


	i = CONTENT_LIMESTONE_INNER_STAIR_CORNER_UD;
	f = &content_features(i);
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->setAllTextures("limestone.png");
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_LIMESTONE_STAIR)+" 1";
	content_nodebox_inner_stair_cornerud(f);
	f->setInventoryTextureNodeBox(i,"limestone.png","limestone.png","limestone.png");
	f->type = CMT_STONE;
	f->dig_time = 0.9;
	f->onpunch_replace_node = CONTENT_LIMESTONE_STAIR_UD;
	f->onpunch_replace_respects_borderstone = true;
}
