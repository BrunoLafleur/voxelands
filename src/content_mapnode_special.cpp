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

void content_mapnode_special(bool repeat)
{
	content_t i;
	ContentFeatures *f = NULL;

	i = CONTENT_FENCE;
	f = &content_features(i);
	f->description = gettext("Fence");
	f->setAllTextures("wood.png");
	f->light_propagates = true;
	f->param_type = CPT_LIGHT;
	f->param2_type = CPT_SPECIAL;
	f->draw_type = CDT_FENCELIKE;
	f->is_ground_content = true;
	f->jumpable = false;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->air_equivalent = true; // grass grows underneath
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_FENCE;
	f->special_alternate_node = CONTENT_WOOD;
	f->type = CMT_WOOD;
	f->dig_time = 0.75;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_nodebox_fence_inv(f);
	content_nodebox_fence(f);
	f->setInventoryTextureNodeBox(i,"wood.png","wood.png","wood.png");
	crafting::setWallRecipe(CONTENT_CRAFTITEM_WOOD_PLANK,CONTENT_FENCE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_IRON_FENCE;
	f = &content_features(i);
	f->description = gettext("Iron Fence");
	f->setAllTextures("fence_iron.png");
	f->setTexture(0,"fence_iron_top.png");
	f->setTexture(1,"fence_iron_top.png");
	f->light_propagates = true;
	f->param_type = CPT_LIGHT;
	f->param2_type = CPT_SPECIAL;
	f->draw_type = CDT_FENCELIKE;
	f->is_ground_content = true;
	f->jumpable = false;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->air_equivalent = true; // grass grows underneath
	f->special_alternate_node = CONTENT_IRON;
	f->type = CMT_STONE;
	f->dig_time = 0.75;
	f->pressure_type = CST_SOLID;
	f->suffocation_per_second = 0;
	f->destructive_mob_safe = true;
	content_nodebox_fence_inv(f);
	content_nodebox_fence(f);
	f->setInventoryTextureNodeBox(i,"fence_iron.png","fence_iron_top.png","fence_iron.png");
	crafting::setWallRecipe(CONTENT_CRAFTITEM_IRON_INGOT,CONTENT_IRON_FENCE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_JUNGLE_FENCE;
	f = &content_features(i);
	f->description = gettext("Jungle Wood Fence");
	f->setAllTextures("junglewood.png");
	f->light_propagates = true;
	f->param_type = CPT_LIGHT;
	f->param2_type = CPT_SPECIAL;
	f->draw_type = CDT_FENCELIKE;
	f->is_ground_content = true;
	f->jumpable = false;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->air_equivalent = true; // grass grows underneath
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_FENCE;
	f->special_alternate_node = CONTENT_JUNGLEWOOD;
	f->type = CMT_WOOD;
	f->dig_time = 0.75;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_nodebox_fence_inv(f);
	content_nodebox_fence(f);
	f->setInventoryTextureNodeBox(i,"junglewood.png","junglewood.png","junglewood.png");
	crafting::setWallRecipe(CONTENT_CRAFTITEM_JUNGLE_PLANK,CONTENT_JUNGLE_FENCE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_PINE_FENCE;
	f = &content_features(i);
	f->description = gettext("Pine Fence");
	f->setAllTextures("pine.png");
	f->light_propagates = true;
	f->param_type = CPT_LIGHT;
	f->param2_type = CPT_SPECIAL;
	f->draw_type = CDT_FENCELIKE;
	f->is_ground_content = true;
	f->jumpable = false;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->air_equivalent = true; // grass grows underneath
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_FENCE;
	f->special_alternate_node = CONTENT_WOOD_PINE;
	f->type = CMT_WOOD;
	f->dig_time = 0.75;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_nodebox_fence_inv(f);
	content_nodebox_fence(f);
	f->setInventoryTextureNodeBox(i,"pine.png","pine.png","pine.png");
	crafting::setWallRecipe(CONTENT_CRAFTITEM_PINE_PLANK,CONTENT_PINE_FENCE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_APPLE_FENCE;
	f = &content_features(i);
	f->description = gettext("Apple Wood Fence");
	f->setAllTextures("applewood.png");
	f->light_propagates = true;
	f->param_type = CPT_LIGHT;
	f->param2_type = CPT_SPECIAL;
	f->draw_type = CDT_FENCELIKE;
	f->is_ground_content = true;
	f->jumpable = false;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->air_equivalent = true; // grass grows underneath
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_FENCE;
	f->special_alternate_node = CONTENT_APPLEWOOD;
	f->type = CMT_WOOD;
	f->dig_time = 0.75;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_nodebox_fence_inv(f);
	content_nodebox_fence(f);
	f->setInventoryTextureNodeBox(i,"applewood.png","applewood.png","applewood.png");
	crafting::setWallRecipe(CONTENT_CRAFTITEM_APPLE_PLANK,CONTENT_APPLE_FENCE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_IRON_BARS;
	f = &content_features(i);
	f->description = gettext("Iron Bars");
	f->setAllTextures("iron_block.png");
	f->light_propagates = true;
	f->param_type = CPT_LIGHT;
	f->param2_type = CPT_FACEDIR_SIMPLE;
	f->draw_type = CDT_NODEBOX;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->air_equivalent = true; // grass grows underneath
	f->special_alternate_node = CONTENT_IRON;
	f->type = CMT_STONE;
	f->dig_time = 0.75;
	f->pressure_type = CST_SOLID;
	f->suffocation_per_second = 0;
	content_nodebox_bars(f);
	f->setInventoryTextureNodeBox(i,"iron_block.png","iron_block.png","iron_block.png");
	{
		u16 r[9] = {
			CONTENT_CRAFTITEM_IRON_INGOT,	CONTENT_IGNORE,	CONTENT_CRAFTITEM_IRON_INGOT,
			CONTENT_CRAFTITEM_IRON_INGOT,	CONTENT_IGNORE,	CONTENT_CRAFTITEM_IRON_INGOT,
			CONTENT_CRAFTITEM_IRON_INGOT,	CONTENT_IGNORE,	CONTENT_CRAFTITEM_IRON_INGOT
		};
		crafting::setRecipe(r,CONTENT_IRON_BARS,6);
	}
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_RAIL;
	f = &content_features(i);
	f->description = gettext("Rail");
	f->setAllTextures("rail.png");
	f->setTexture(0,"track_tie.png");
	f->setTexture(1,"track_rail.png");
	f->light_propagates = true;
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_RAILLIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->air_equivalent = true; // grass grows underneath
	f->type = CMT_DIRT;
	f->dig_time = 0.75;
	f->suffocation_per_second = 0;
	{
		u16 r[9] = {
			CONTENT_CRAFTITEM_IRON_INGOT,	CONTENT_CRAFTITEM_WOOD_PLANK,	CONTENT_CRAFTITEM_IRON_INGOT,
			CONTENT_CRAFTITEM_IRON_INGOT,	CONTENT_IGNORE,			CONTENT_CRAFTITEM_IRON_INGOT,
			CONTENT_CRAFTITEM_IRON_INGOT,	CONTENT_CRAFTITEM_WOOD_PLANK,	CONTENT_CRAFTITEM_IRON_INGOT
		};
		crafting::setRecipe(r,CONTENT_RAIL,15);
		r[1] = CONTENT_CRAFTITEM_JUNGLE_PLANK;
		r[7] = CONTENT_CRAFTITEM_JUNGLE_PLANK;
		crafting::setRecipe(r,CONTENT_RAIL,15);
	}
	content_nodebox_rail(f);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_ROOFTILE_TERRACOTTA;
	f = &content_features(i);
	f->description = gettext("Terracotta Roof Tile");
	f->setAllTextures("terracotta.png^overlay_rooftile.png");
	f->setAllTextureFlags(0);
	f->draw_type = CDT_ROOFLIKE;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->type = CMT_DIRT;
	f->dig_time = 0.75;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_nodebox_roofcollide(f);
	crafting::set1over4Recipe(CONTENT_TERRACOTTA,CONTENT_TERRACOTTA,CONTENT_ROOFTILE_TERRACOTTA);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_ROOFTILE_WOOD;
	f = &content_features(i);
	f->description = gettext("Wood Roof Tile");
	f->setAllTextures("rooftile_wood.png");
	f->setAllTextureFlags(0);
	f->draw_type = CDT_ROOFLIKE;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->type = CMT_WOOD;
	f->dig_time = 0.75;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_nodebox_roofcollide(f);
	crafting::set1over4Recipe(CONTENT_WOOD,CONTENT_WOOD,CONTENT_ROOFTILE_WOOD);
	crafting::set1over4Recipe(CONTENT_JUNGLEWOOD,CONTENT_JUNGLEWOOD,CONTENT_ROOFTILE_WOOD);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_ROOFTILE_ASPHALT;
	f = &content_features(i);
	f->description = gettext("Asphalt Roof Tile");
	f->setAllTextures("rooftile_asphalt.png");
	f->setAllTextureFlags(0);
	f->draw_type = CDT_ROOFLIKE;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->type = CMT_DIRT;
	f->dig_time = 0.75;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_nodebox_roofcollide(f);
	{
		u16 r[9] = {
			CONTENT_SAND,			CONTENT_CRAFTITEM_PAPER,	CONTENT_IGNORE,
			CONTENT_CRAFTITEM_PAPER,	CONTENT_CRAFTITEM_DYE_BLACK,	CONTENT_IGNORE,
			CONTENT_IGNORE,			CONTENT_IGNORE,			CONTENT_IGNORE,
		};
		crafting::setRecipe(r,CONTENT_ROOFTILE_ASPHALT,4);
	}
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_ROOFTILE_STONE;
	f = &content_features(i);
	f->description = gettext("Stone Roof Tile");
	f->setAllTextures("stone.png^overlay_rooftile.png");
	f->setAllTextureFlags(0);
	f->draw_type = CDT_ROOFLIKE;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->type = CMT_DIRT;
	f->dig_time = 0.75;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_nodebox_roofcollide(f);
	crafting::set1over4Recipe(CONTENT_ROUGHSTONE,CONTENT_ROUGHSTONE,CONTENT_ROOFTILE_STONE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_ROOFTILE_GLASS;
	f = &content_features(i);
	f->description = gettext("Glass Roof Tile");
	f->setAllTextures("glass.png");
	f->setTexture(1,"glass_slab.png"); // special texture for top sections
	f->setAllTextureFlags(0);
#ifndef SERVER
	f->setAllTextureTypes(MATERIAL_ALPHA_BLEND);
#endif
	f->param_type = CPT_LIGHT;
	f->light_propagates = true;
	f->draw_type = CDT_ROOFLIKE;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->type = CMT_GLASS;
	f->dig_time = 0.75;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_nodebox_roofcollide(f);
	crafting::set1over4Recipe(CONTENT_GLASS,CONTENT_GLASS,CONTENT_ROOFTILE_GLASS);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_ROOFTILE_GLASS_BLUE;
	f = &content_features(i);
	f->description = gettext("Blue Glass Roof Tile");
	f->setAllTextures("glass.png^glass_pane_blue_side.png");
	f->setTexture(1,"glass_slab.png^glass_pane_blue_side.png"); // special texture for top sections
	f->setAllTextureFlags(0);
#ifndef SERVER
	f->setAllTextureTypes(MATERIAL_ALPHA_BLEND);
#endif
	f->param_type = CPT_LIGHT;
	f->light_propagates = true;
	f->draw_type = CDT_ROOFLIKE;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->type = CMT_GLASS;
	f->dig_time = 0.75;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_nodebox_roofcollide(f);
	crafting::set1over4Recipe(CONTENT_GLASS_BLUE,CONTENT_GLASS_BLUE,CONTENT_ROOFTILE_GLASS_BLUE);
	crafting::set1Any2Recipe(CONTENT_ROOFTILE_GLASS,CONTENT_CRAFTITEM_DYE_BLUE,CONTENT_ROOFTILE_GLASS_BLUE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_ROOFTILE_GLASS_GREEN;
	f = &content_features(i);
	f->description = gettext("Green Glass Roof Tile");
	f->setAllTextures("glass.png^glass_pane_green_side.png");
	f->setTexture(1,"glass_slab.png^glass_pane_green_side.png"); // special texture for top sections
	f->setAllTextureFlags(0);
#ifndef SERVER
	f->setAllTextureTypes(MATERIAL_ALPHA_BLEND);
#endif
	f->param_type = CPT_LIGHT;
	f->light_propagates = true;
	f->draw_type = CDT_ROOFLIKE;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->type = CMT_GLASS;
	f->dig_time = 0.75;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_nodebox_roofcollide(f);
	crafting::set1over4Recipe(CONTENT_GLASS_GREEN,CONTENT_GLASS_GREEN,CONTENT_ROOFTILE_GLASS_GREEN);
	crafting::set1Any2Recipe(CONTENT_ROOFTILE_GLASS,CONTENT_CRAFTITEM_DYE_GREEN,CONTENT_ROOFTILE_GLASS_GREEN);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_ROOFTILE_GLASS_ORANGE;
	f = &content_features(i);
	f->description = gettext("Orange Glass Roof Tile");
	f->setAllTextures("glass.png^glass_pane_orange_side.png");
	f->setTexture(1,"glass_slab.png^glass_pane_orange_side.png"); // special texture for top sections
	f->setAllTextureFlags(0);
#ifndef SERVER
	f->setAllTextureTypes(MATERIAL_ALPHA_BLEND);
#endif
	f->param_type = CPT_LIGHT;
	f->light_propagates = true;
	f->draw_type = CDT_ROOFLIKE;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->type = CMT_GLASS;
	f->dig_time = 0.75;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_nodebox_roofcollide(f);
	crafting::set1over4Recipe(CONTENT_GLASS_ORANGE,CONTENT_GLASS_ORANGE,CONTENT_ROOFTILE_GLASS_ORANGE);
	crafting::set1Any2Recipe(CONTENT_ROOFTILE_GLASS,CONTENT_CRAFTITEM_DYE_ORANGE,CONTENT_ROOFTILE_GLASS_ORANGE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_ROOFTILE_GLASS_PURPLE;
	f = &content_features(i);
	f->description = gettext("Purple Glass Roof Tile");
	f->setAllTextures("glass.png^glass_pane_purple_side.png");
	f->setTexture(1,"glass_slab.png^glass_pane_purple_side.png"); // special texture for top sections
	f->setAllTextureFlags(0);
#ifndef SERVER
	f->setAllTextureTypes(MATERIAL_ALPHA_BLEND);
#endif
	f->param_type = CPT_LIGHT;
	f->light_propagates = true;
	f->draw_type = CDT_ROOFLIKE;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->type = CMT_GLASS;
	f->dig_time = 0.75;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_nodebox_roofcollide(f);
	crafting::set1over4Recipe(CONTENT_GLASS_PURPLE,CONTENT_GLASS_PURPLE,CONTENT_ROOFTILE_GLASS_PURPLE);
	crafting::set1Any2Recipe(CONTENT_ROOFTILE_GLASS,CONTENT_CRAFTITEM_DYE_PURPLE,CONTENT_ROOFTILE_GLASS_PURPLE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_ROOFTILE_GLASS_RED;
	f = &content_features(i);
	f->description = gettext("Red Glass Roof Tile");
	f->setAllTextures("glass.png^glass_pane_red_side.png");
	f->setTexture(1,"glass_slab.png^glass_pane_red_side.png"); // special texture for top sections
	f->setAllTextureFlags(0);
#ifndef SERVER
	f->setAllTextureTypes(MATERIAL_ALPHA_BLEND);
#endif
	f->param_type = CPT_LIGHT;
	f->light_propagates = true;
	f->draw_type = CDT_ROOFLIKE;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->type = CMT_GLASS;
	f->dig_time = 0.75;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_nodebox_roofcollide(f);
	crafting::set1over4Recipe(CONTENT_GLASS_RED,CONTENT_GLASS_RED,CONTENT_ROOFTILE_GLASS_RED);
	crafting::set1Any2Recipe(CONTENT_ROOFTILE_GLASS,CONTENT_CRAFTITEM_DYE_RED,CONTENT_ROOFTILE_GLASS_RED);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_ROOFTILE_GLASS_YELLOW;
	f = &content_features(i);
	f->description = gettext("Yellow Glass Roof Tile");
	f->setAllTextures("glass.png^glass_pane_yellow_side.png");
	f->setTexture(1,"glass_slab.png^glass_pane_yellow_side.png"); // special texture for top sections
	f->setAllTextureFlags(0);
#ifndef SERVER
	f->setAllTextureTypes(MATERIAL_ALPHA_BLEND);
#endif
	f->param_type = CPT_LIGHT;
	f->light_propagates = true;
	f->draw_type = CDT_ROOFLIKE;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->type = CMT_GLASS;
	f->dig_time = 0.75;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_nodebox_roofcollide(f);
	crafting::set1over4Recipe(CONTENT_GLASS_YELLOW,CONTENT_GLASS_YELLOW,CONTENT_ROOFTILE_GLASS_YELLOW);
	crafting::set1Any2Recipe(CONTENT_ROOFTILE_GLASS,CONTENT_CRAFTITEM_DYE_YELLOW,CONTENT_ROOFTILE_GLASS_YELLOW);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_ROOFTILE_GLASS_BLACK;
	f = &content_features(i);
	f->description = gettext("Black Glass Roof Tile");
	f->setAllTextures("glass.png^glass_pane_black_side.png");
	f->setTexture(1,"glass_slab.png^glass_pane_black_side.png"); // special texture for top sections
	f->setAllTextureFlags(0);
#ifndef SERVER
	f->setAllTextureTypes(MATERIAL_ALPHA_BLEND);
#endif
	f->param_type = CPT_LIGHT;
	f->light_propagates = true;
	f->draw_type = CDT_ROOFLIKE;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->type = CMT_GLASS;
	f->dig_time = 0.75;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_nodebox_roofcollide(f);
	crafting::set1over4Recipe(CONTENT_GLASS_BLACK,CONTENT_GLASS_BLACK,CONTENT_ROOFTILE_GLASS_BLACK);
	crafting::set1Any2Recipe(CONTENT_ROOFTILE_GLASS,CONTENT_CRAFTITEM_DYE_BLACK,CONTENT_ROOFTILE_GLASS_BLACK);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_ROOFTILE_THATCH;
	f = &content_features(i);
	f->description = gettext("Thatch Roof Tile");
	f->setAllTextures("rooftile_thatch.png");
	f->setAllTextureFlags(0);
	f->draw_type = CDT_ROOFLIKE;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->type = CMT_DIRT;
	f->dig_time = 0.75;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_nodebox_roofcollide(f);
	crafting::set1over4Recipe(CONTENT_CRAFTITEM_STRAW,CONTENT_DEADGRASS,CONTENT_ROOFTILE_THATCH);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_LADDER_WALL;
	f = &content_features(i);
	f->description = gettext("Ladder");
	f->setAllTextures("ladder.png");
	f->light_propagates = true;
	f->param_type = CPT_LIGHT;
	f->param2_type = CPT_FACEDIR_WALLMOUNT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem ")+itos(i)+" 1";
	f->floormount_alternate_node = CONTENT_LADDER_FLOOR;
	f->roofmount_alternate_node = CONTENT_LADDER_ROOF;
	f->rotate_tile_with_nodebox = true;
	f->climbable = true;
	f->air_equivalent = true;
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_LADDER;
	f->type = CMT_WOOD;
	f->dig_time = 0.5;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_nodebox_ladder_wall(f);
	f->setInventoryTextureNodeBox(i,"ladder.png","ladder.png","ladder.png");
	{
		u16 r[9] = {
			CONTENT_CRAFTITEM_PINE_PLANK,	CONTENT_CRAFTITEM_STICK,	CONTENT_CRAFTITEM_PINE_PLANK,
			CONTENT_CRAFTITEM_PINE_PLANK,	CONTENT_CRAFTITEM_STICK,	CONTENT_CRAFTITEM_PINE_PLANK,
			CONTENT_CRAFTITEM_PINE_PLANK,	CONTENT_CRAFTITEM_STICK,	CONTENT_CRAFTITEM_PINE_PLANK
		};
		crafting::setRecipe(r,CONTENT_LADDER_WALL,4);
		r[0] = CONTENT_CRAFTITEM_WOOD_PLANK;
		r[2] = CONTENT_CRAFTITEM_WOOD_PLANK;
		r[3] = CONTENT_CRAFTITEM_WOOD_PLANK;
		r[5] = CONTENT_CRAFTITEM_WOOD_PLANK;
		r[6] = CONTENT_CRAFTITEM_WOOD_PLANK;
		r[8] = CONTENT_CRAFTITEM_WOOD_PLANK;
		crafting::setRecipe(r,CONTENT_LADDER_WALL,4);
		r[0] = CONTENT_CRAFTITEM_JUNGLE_PLANK;
		r[2] = CONTENT_CRAFTITEM_JUNGLE_PLANK;
		r[3] = CONTENT_CRAFTITEM_JUNGLE_PLANK;
		r[5] = CONTENT_CRAFTITEM_JUNGLE_PLANK;
		r[6] = CONTENT_CRAFTITEM_JUNGLE_PLANK;
		r[8] = CONTENT_CRAFTITEM_JUNGLE_PLANK;
		crafting::setRecipe(r,CONTENT_LADDER_WALL,4);
	}
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_LADDER_FLOOR;
	f = &content_features(i);
	f->description = gettext("Ladder");
	f->setAllTextures("ladder.png");
	f->light_propagates = true;
	f->param_type = CPT_LIGHT;
	f->param2_type = CPT_FACEDIR_SIMPLE;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem ")+itos(CONTENT_LADDER_WALL)+" 1";
	f->wallmount_alternate_node = CONTENT_LADDER_WALL;
	f->roofmount_alternate_node = CONTENT_LADDER_ROOF;
	f->rotate_tile_with_nodebox = true;
	f->climbable = true;
	f->air_equivalent = true;
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_LADDER;
	f->type = CMT_WOOD;
	f->dig_time = 0.5;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_nodebox_ladder_floor(f);

	i = CONTENT_LADDER_ROOF;
	f = &content_features(i);
	f->description = gettext("Ladder");
	f->setAllTextures("ladder.png");
	f->light_propagates = true;
	f->param_type = CPT_LIGHT;
	f->param2_type = CPT_FACEDIR_SIMPLE;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem ")+itos(CONTENT_LADDER_WALL)+" 1";
	f->floormount_alternate_node = CONTENT_LADDER_FLOOR;
	f->wallmount_alternate_node = CONTENT_LADDER_WALL;
	f->rotate_tile_with_nodebox = true;
	f->climbable = true;
	f->air_equivalent = true;
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_LADDER;
	f->type = CMT_WOOD;
	f->dig_time = 0.5;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_nodebox_ladder_roof(f);

	i = CONTENT_BORDERSTONE;
	f = &content_features(i);
	f->description = gettext("Border Stone");
	f->setAllTextures("borderstone.png");
	f->setInventoryTextureCube("borderstone.png", "borderstone.png", "borderstone.png");
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	if(f->initial_metadata == NULL)
		f->initial_metadata = new BorderStoneNodeMetadata();
	f->type = CMT_STONE;
	f->dig_time = 2.0;
	f->pressure_type = CST_SOLID;
	crafting::setFilledRoundRecipe(CONTENT_STONE,CONTENT_CRAFTITEM_MITHRIL_RAW,CONTENT_BORDERSTONE);
	crafting::setFilledRoundRecipe(CONTENT_STONE,CONTENT_CRAFTITEM_MITHRIL_UNBOUND,CONTENT_BORDERSTONE);
	{
		u16 r[9] = {
			CONTENT_CRAFTITEM_IRON_INGOT,	CONTENT_STONE,			CONTENT_CRAFTITEM_IRON_INGOT,
			CONTENT_STONE,			CONTENT_CRAFTITEM_IRON_INGOT,	CONTENT_STONE,
			CONTENT_CRAFTITEM_IRON_INGOT,	CONTENT_STONE,			CONTENT_CRAFTITEM_IRON_INGOT
		};
		crafting::setRecipe(r,CONTENT_BORDERSTONE,1);
	}
	content_list_add("craftguide",i,1,0);
	content_list_add("player-creative",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_BOOK;
	f = &content_features(i);
	f->description = gettext("Book");
	f->setTexture(0, "book_cover.png");
	f->setTexture(1, "book_cover.png^[transformFX");
	f->setTexture(2, "book_side.png^[transformFY");
	f->setTexture(3, "book_side.png");
	f->setTexture(4, "book_end.png");
	f->setTexture(5, "book_end.png^[transformFX");
	f->param_type = CPT_LIGHT;
	f->param2_type = CPT_FACEDIR_SIMPLE;
	f->draw_type = CDT_NODEBOX;
	f->rotate_tile_with_nodebox = true;
	f->light_propagates = true;
	f->air_equivalent = true;
	f->onpunch_replace_node = CONTENT_BOOK_OPEN;
	f->flammable = 1;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	content_nodebox_book(f);
	f->setFaceText(0,FaceText(0.45,0.3,0.9875,0.7375));
	f->setInventoryTextureNodeBox(i, "book_cover.png", "book_end.png^[transformFX", "book_side.png^[transformFY");
	f->type = CMT_DIRT;
	f->dig_time = 1.0;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	crafting::setCol1Recipe(CONTENT_CRAFTITEM_PAPER,i);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);
	if (f->initial_metadata == NULL)
		f->initial_metadata = new ClosedBookNodeMetadata();

	i = CONTENT_COOK_BOOK;
	f = &content_features(i);
	f->description = gettext("Cook Book");
	f->setTexture(0, "book_cook_cover.png");
	f->setTexture(1, "book_cook_cover.png^[transformFX");
	f->setTexture(2, "book_cook_side.png^[transformFY");
	f->setTexture(3, "book_cook_side.png");
	f->setTexture(4, "book_cook_end.png");
	f->setTexture(5, "book_cook_end.png^[transformFX");
	f->param_type = CPT_LIGHT;
	f->param2_type = CPT_FACEDIR_SIMPLE;
	f->draw_type = CDT_NODEBOX;
	f->rotate_tile_with_nodebox = true;
	f->light_propagates = true;
	f->air_equivalent = true;
	f->onpunch_replace_node = CONTENT_COOK_BOOK_OPEN;
	f->flammable = 1;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	content_nodebox_book(f);
	f->setInventoryTextureNodeBox(i, "book_cook_cover.png", "book_cook_end.png^[transformFX", "book_cook_side.png^[transformFY");
	f->type = CMT_DIRT;
	f->dig_time = 1.0;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	crafting::set1Any2Recipe(CONTENT_BOOK,CONTENT_CRAFTITEM_COAL,i);
	crafting::set1Any2Recipe(CONTENT_BOOK,CONTENT_CRAFTITEM_CHARCOAL,i);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);
	if (f->initial_metadata == NULL)
		f->initial_metadata = new ClosedBookNodeMetadata();

	i = CONTENT_DECRAFT_BOOK;
	f = &content_features(i);
	f->description = gettext("Decraft Book");
	f->setTexture(0, "book_decraft_cover.png");
	f->setTexture(1, "book_decraft_cover.png^[transformFX");
	f->setTexture(2, "book_decraft_side.png^[transformFY");
	f->setTexture(3, "book_decraft_side.png");
	f->setTexture(4, "book_decraft_end.png");
	f->setTexture(5, "book_decraft_end.png^[transformFX");
	f->param_type = CPT_LIGHT;
	f->param2_type = CPT_FACEDIR_SIMPLE;
	f->draw_type = CDT_NODEBOX;
	f->rotate_tile_with_nodebox = true;
	f->light_propagates = true;
	f->air_equivalent = true;
	f->onpunch_replace_node = CONTENT_DECRAFT_BOOK_OPEN;
	f->flammable = 1;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	content_nodebox_book(f);
	f->setInventoryTextureNodeBox(i, "book_decraft_cover.png", "book_decraft_end.png^[transformFX", "book_decraft_side.png^[transformFY");
	f->type = CMT_DIRT;
	f->dig_time = 1.0;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	crafting::set1Any2Recipe(CONTENT_BOOK,CONTENT_CRAFTITEM_GUNPOWDER,i);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);
	if (f->initial_metadata == NULL)
		f->initial_metadata = new ClosedBookNodeMetadata();

	i = CONTENT_DIARY_BOOK;
	f = &content_features(i);
	f->description = gettext("Diary");
	f->setTexture(0, "book_diary_cover.png");
	f->setTexture(1, "book_diary_cover.png^[transformFX");
	f->setTexture(2, "book_diary_side.png^[transformFY");
	f->setTexture(3, "book_diary_side.png");
	f->setTexture(4, "book_diary_end.png");
	f->setTexture(5, "book_diary_end.png^[transformFX");
	f->param_type = CPT_LIGHT;
	f->param2_type = CPT_FACEDIR_SIMPLE;
	f->draw_type = CDT_NODEBOX;
	f->rotate_tile_with_nodebox = true;
	f->light_propagates = true;
	f->air_equivalent = true;
	f->onpunch_replace_node = CONTENT_DIARY_BOOK_OPEN;
	f->flammable = 1;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	content_nodebox_book(f);
	f->setFaceText(0,FaceText(0.45,0.3,0.9875,0.7375));
	f->setInventoryTextureNodeBox(i, "book_diary_cover.png", "book_diary_end.png^[transformFX", "book_diary_side.png^[transformFY");
	f->type = CMT_DIRT;
	f->dig_time = 1.0;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	crafting::set1Any2Recipe(CONTENT_BOOK,CONTENT_CRAFTITEM_IRON_INGOT,i);
	if (f->initial_metadata == NULL)
		f->initial_metadata = new ClosedBookNodeMetadata();
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFT_BOOK;
	f = &content_features(i);
	f->description = gettext("Craft Book");
	f->setTexture(0, "book_craft_cover.png");
	f->setTexture(1, "book_craft_cover.png^[transformFX");
	f->setTexture(2, "book_craft_side.png^[transformFY");
	f->setTexture(3, "book_craft_side.png");
	f->setTexture(4, "book_craft_end.png");
	f->setTexture(5, "book_craft_end.png^[transformFX");
	f->param_type = CPT_LIGHT;
	f->param2_type = CPT_FACEDIR_SIMPLE;
	f->draw_type = CDT_NODEBOX;
	f->rotate_tile_with_nodebox = true;
	f->light_propagates = true;
	f->air_equivalent = true;
	f->onpunch_replace_node = CONTENT_CRAFT_BOOK_OPEN;
	f->flammable = 1;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	content_nodebox_book(f);
	f->setInventoryTextureNodeBox(i, "book_craft_cover.png", "book_craft_end.png^[transformFX", "book_craft_side.png^[transformFY");
	f->type = CMT_DIRT;
	f->dig_time = 1.0;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	crafting::set1Any2Recipe(CONTENT_BOOK,CONTENT_CRAFTITEM_MITHRIL_RAW,i);
	crafting::set1Any2Recipe(CONTENT_BOOK,CONTENT_CRAFTITEM_MITHRIL_UNBOUND,i);
	{
		content_t r[9] = {
			CONTENT_CRAFTITEM_STICK, CONTENT_CRAFTITEM_STICK, CONTENT_IGNORE,
			CONTENT_IGNORE,		 CONTENT_CRAFTITEM_STICK, CONTENT_IGNORE,
			CONTENT_CRAFTITEM_STICK, CONTENT_IGNORE,	  CONTENT_IGNORE
		};
		crafting::setRecipe(r,CONTENT_CRAFT_BOOK,1);
	}
	if (f->initial_metadata == NULL)
		f->initial_metadata = new ClosedBookNodeMetadata();
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_RCRAFT_BOOK;
	f = &content_features(i);
	f->description = gettext("Reverse Craft Book");
	f->setTexture(0, "book_rcraft_cover.png");
	f->setTexture(1, "book_rcraft_cover.png^[transformFX");
	f->setTexture(2, "book_rcraft_side.png^[transformFY");
	f->setTexture(3, "book_rcraft_side.png");
	f->setTexture(4, "book_rcraft_end.png");
	f->setTexture(5, "book_rcraft_end.png^[transformFX");
	f->param_type = CPT_LIGHT;
	f->param2_type = CPT_FACEDIR_SIMPLE;
	f->draw_type = CDT_NODEBOX;
	f->rotate_tile_with_nodebox = true;
	f->light_propagates = true;
	f->air_equivalent = true;
	f->onpunch_replace_node = CONTENT_RCRAFT_BOOK_OPEN;
	f->flammable = 1;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	content_nodebox_book(f);
	f->setInventoryTextureNodeBox(i, "book_rcraft_cover.png", "book_rcraft_end.png^[transformFX", "book_rcraft_side.png^[transformFY");
	f->type = CMT_DIRT;
	f->dig_time = 1.0;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	{
		content_t r[9] = {
			CONTENT_CRAFTITEM_STICK, CONTENT_IGNORE,          CONTENT_IGNORE,
			CONTENT_IGNORE,          CONTENT_CRAFTITEM_STICK, CONTENT_IGNORE,
			CONTENT_CRAFTITEM_STICK, CONTENT_CRAFTITEM_STICK,	CONTENT_IGNORE
		};
		crafting::setRecipe(r,CONTENT_RCRAFT_BOOK,1);
	}
	if (f->initial_metadata == NULL)
		f->initial_metadata = new ClosedBookNodeMetadata();
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_BOOK_OPEN;
	f = &content_features(i);
	f->description = gettext("Guide");
	f->setAllTextures("guide_side.png");
	f->setTexture(0, "guide_top.png");
	f->setTexture(1, "guide_bottom.png");
	f->setTexture(4, "guide_end.png");
	f->setTexture(5, "guide_end.png");
	f->param_type = CPT_LIGHT;
	f->param2_type = CPT_FACEDIR_SIMPLE;
	f->draw_type = CDT_NODEBOX;
	f->rotate_tile_with_nodebox = true;
	f->light_propagates = true;
	f->air_equivalent = true;
	f->onpunch_replace_node = CONTENT_BOOK;
	f->flammable = 1;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_BOOK)+" 1";
	content_nodebox_guide(f);
	f->setFaceText(0,FaceText(0.15,0.35,0.85,0.85,FTT_BOOKCONTENT));
	f->setInventoryTextureNodeBox(i, "guide_top.png", "guide_end.png", "guide_side.png");
	f->type = CMT_DIRT;
	f->dig_time = 1.0;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	if (f->initial_metadata == NULL)
		f->initial_metadata = new BookNodeMetadata();
	f->sound_access = "open-book";

	i = CONTENT_COOK_BOOK_OPEN;
	f = &content_features(i);
	f->description = gettext("Cooking Guide");
	f->setAllTextures("guide_cook_side.png");
	f->setTexture(0, "guide_cook_top.png");
	f->setTexture(1, "guide_cook_bottom.png");
	f->setTexture(4, "guide_cook_end.png");
	f->setTexture(5, "guide_cook_end.png");
	f->param_type = CPT_LIGHT;
	f->param2_type = CPT_FACEDIR_SIMPLE;
	f->draw_type = CDT_NODEBOX;
	f->rotate_tile_with_nodebox = true;
	f->light_propagates = true;
	f->air_equivalent = true;
	f->onpunch_replace_node = CONTENT_COOK_BOOK;
	f->flammable = 1;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_COOK_BOOK)+" 1";
	content_nodebox_guide(f);
	f->setInventoryTextureNodeBox(i, "guide_cook_top.png", "guide_cook_end.png", "guide_cook_side.png");
	f->type = CMT_DIRT;
	f->dig_time = 1.0;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	if (f->initial_metadata == NULL)
		f->initial_metadata = new CookBookNodeMetadata();
	f->sound_access = "open-book";

	i = CONTENT_DECRAFT_BOOK_OPEN;
	f = &content_features(i);
	f->description = gettext("Decrafting Guide");
	f->setAllTextures("guide_decraft_side.png");
	f->setTexture(0, "guide_decraft_top.png");
	f->setTexture(1, "guide_decraft_bottom.png");
	f->setTexture(4, "guide_decraft_end.png");
	f->setTexture(5, "guide_decraft_end.png");
	f->param_type = CPT_LIGHT;
	f->param2_type = CPT_FACEDIR_SIMPLE;
	f->draw_type = CDT_NODEBOX;
	f->rotate_tile_with_nodebox = true;
	f->light_propagates = true;
	f->air_equivalent = true;
	f->onpunch_replace_node = CONTENT_DECRAFT_BOOK;
	f->flammable = 1;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_DECRAFT_BOOK)+" 1";
	content_nodebox_guide(f);
	f->setInventoryTextureNodeBox(i, "guide_decraft_top.png", "guide_decraft_end.png", "guide_decraft_side.png");
	f->type = CMT_DIRT;
	f->dig_time = 1.0;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	if (f->initial_metadata == NULL)
		f->initial_metadata = new DeCraftNodeMetadata();
	f->sound_access = "open-book";

	i = CONTENT_DIARY_BOOK_OPEN;
	f = &content_features(i);
	f->description = gettext("Diary");
	f->setAllTextures("guide_diary_side.png");
	f->setTexture(0, "guide_diary_top.png");
	f->setTexture(1, "guide_diary_bottom.png");
	f->setTexture(4, "guide_diary_end.png");
	f->setTexture(5, "guide_diary_end.png");
	f->param_type = CPT_LIGHT;
	f->param2_type = CPT_FACEDIR_SIMPLE;
	f->draw_type = CDT_NODEBOX;
	f->rotate_tile_with_nodebox = true;
	f->light_propagates = true;
	f->air_equivalent = true;
	f->onpunch_replace_node = CONTENT_DIARY_BOOK;
	f->flammable = 1;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_DIARY_BOOK)+" 1";
	content_nodebox_guide(f);
	f->setFaceText(0,FaceText(0.15,0.35,0.85,0.85,FTT_BOOKCONTENT));
	f->setInventoryTextureNodeBox(i, "guide_diary_top.png", "guide_diary_end.png", "guide_diary_side.png");
	f->type = CMT_DIRT;
	f->dig_time = 1.0;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	if (f->initial_metadata == NULL)
		f->initial_metadata = new DiaryNodeMetadata();
	f->sound_access = "open-book";

	i = CONTENT_CRAFT_BOOK_OPEN;
	f = &content_features(i);
	f->description = gettext("Craft Guide");
	f->setAllTextures("guide_craft_side.png");
	f->setTexture(0, "guide_craft_top.png");
	f->setTexture(1, "guide_craft_bottom.png");
	f->setTexture(4, "guide_craft_end.png");
	f->setTexture(5, "guide_craft_end.png");
	f->param_type = CPT_LIGHT;
	f->param2_type = CPT_FACEDIR_SIMPLE;
	f->draw_type = CDT_NODEBOX;
	f->rotate_tile_with_nodebox = true;
	f->light_propagates = true;
	f->air_equivalent = true;
	f->onpunch_replace_node = CONTENT_CRAFT_BOOK;
	f->flammable = 1;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_CRAFT_BOOK)+" 1";
	content_nodebox_guide(f);
	f->setInventoryTextureNodeBox(i, "guide_craft_top.png", "guide_craft_end.png", "guide_craft_side.png");
	f->type = CMT_DIRT;
	f->dig_time = 1.0;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	if (f->initial_metadata == NULL)
		f->initial_metadata = new CraftGuideNodeMetadata();
	f->sound_access = "open-book";

	i = CONTENT_RCRAFT_BOOK_OPEN;
	f = &content_features(i);
	f->description = gettext("Reverse Craft Guide");
	f->setAllTextures("guide_rcraft_side.png");
	f->setTexture(0, "guide_rcraft_top.png");
	f->setTexture(1, "guide_rcraft_bottom.png");
	f->setTexture(4, "guide_rcraft_end.png");
	f->setTexture(5, "guide_rcraft_end.png");
	f->param_type = CPT_LIGHT;
	f->param2_type = CPT_FACEDIR_SIMPLE;
	f->draw_type = CDT_NODEBOX;
	f->rotate_tile_with_nodebox = true;
	f->light_propagates = true;
	f->air_equivalent = true;
	f->onpunch_replace_node = CONTENT_RCRAFT_BOOK;
	f->flammable = 1;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_RCRAFT_BOOK)+" 1";
	content_nodebox_guide(f);
	f->setInventoryTextureNodeBox(i, "guide_rcraft_top.png", "guide_rcraft_end.png", "guide_rcraft_side.png");
	f->type = CMT_DIRT;
	f->dig_time = 1.0;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	if (f->initial_metadata == NULL)
		f->initial_metadata = new ReverseCraftGuideNodeMetadata();
	f->sound_access = "open-book";

/* barrels */

	i = CONTENT_WOOD_BARREL;
	f = &content_features(i);
	f->description = gettext("Wood Barrel");
	f->setAllTextures("wood.png^[transformR90");
	f->setAllMetaTextures("water.png",WATER_ALPHA);
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX_META;
	f->rotate_tile_with_nodebox = true;
	f->light_propagates = true;
	f->air_equivalent = true;
	f->flammable = 1;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->special_alternate_node = CONTENT_WOOD_BARREL_SEALED;
	content_nodebox_barrel(f);
	f->setInventoryTextureNodeBox(i,"wood.png","wood.png^[transformR90","wood.png^[transformR90");
	f->type = CMT_WOOD;
	f->dig_time = 1.0;
	f->pressure_type = CST_SOLID;
	f->suffocation_per_second = 0;
	if (f->initial_metadata == NULL)
		f->initial_metadata = new BarrelNodeMetadata();
	crafting::setURecipe(CONTENT_WOOD,i);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_APPLEWOOD_BARREL;
	f = &content_features(i);
	f->description = gettext("Applewood Barrel");
	f->setAllTextures("applewood.png^[transformR90");
	f->setAllMetaTextures("water.png",WATER_ALPHA);
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX_META;
	f->rotate_tile_with_nodebox = true;
	f->light_propagates = true;
	f->air_equivalent = true;
	f->flammable = 1;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->special_alternate_node = CONTENT_APPLEWOOD_BARREL_SEALED;
	content_nodebox_barrel(f);
	f->setInventoryTextureNodeBox(i,"applewood.png","applewood.png^[transformR90","applewood.png^[transformR90");
	f->type = CMT_WOOD;
	f->dig_time = 1.0;
	f->pressure_type = CST_SOLID;
	f->suffocation_per_second = 0;
	if (f->initial_metadata == NULL)
		f->initial_metadata = new BarrelNodeMetadata();
	crafting::setURecipe(CONTENT_APPLEWOOD,i);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_JUNGLEWOOD_BARREL;
	f = &content_features(i);
	f->description = gettext("Junglewood Barrel");
	f->setAllTextures("junglewood.png^[transformR90");
	f->setAllMetaTextures("water.png",WATER_ALPHA);
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX_META;
	f->rotate_tile_with_nodebox = true;
	f->light_propagates = true;
	f->air_equivalent = true;
	f->flammable = 1;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->special_alternate_node = CONTENT_JUNGLEWOOD_BARREL_SEALED;
	content_nodebox_barrel(f);
	f->setInventoryTextureNodeBox(i,"junglewood.png","junglewood.png^[transformR90","junglewood.png^[transformR90");
	f->type = CMT_WOOD;
	f->dig_time = 1.0;
	f->pressure_type = CST_SOLID;
	f->suffocation_per_second = 0;
	if (f->initial_metadata == NULL)
		f->initial_metadata = new BarrelNodeMetadata();
	crafting::setURecipe(CONTENT_JUNGLEWOOD,i);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_PINE_BARREL;
	f = &content_features(i);
	f->description = gettext("Pine Barrel");
	f->setAllTextures("pine.png^[transformR90");
	f->setAllMetaTextures("water.png",WATER_ALPHA);
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX_META;
	f->rotate_tile_with_nodebox = true;
	f->light_propagates = true;
	f->air_equivalent = true;
	f->flammable = 1;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->special_alternate_node = CONTENT_PINE_BARREL_SEALED;
	content_nodebox_barrel(f);
	f->setInventoryTextureNodeBox(i,"pine.png","pine.png^[transformR90","pine.png^[transformR90");
	f->type = CMT_WOOD;
	f->dig_time = 1.0;
	f->pressure_type = CST_SOLID;
	f->suffocation_per_second = 0;
	if (f->initial_metadata == NULL)
		f->initial_metadata = new BarrelNodeMetadata();
	crafting::setURecipe(CONTENT_WOOD_PINE,i);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

/* sealed barrels */

	i = CONTENT_WOOD_BARREL_SEALED;
	f = &content_features(i);
	f->description = gettext("Wood Barrel");
	f->setAllTextures("wood.png^[transformR90");
	f->setAllMetaTextures("water.png",WATER_ALPHA);
	f->param_type = CPT_LIGHT;
	f->item_param_type = CPT_METADATA;
	f->draw_type = CDT_NODEBOX_META;
	f->rotate_tile_with_nodebox = true;
	f->light_propagates = true;
	f->air_equivalent = true;
	f->flammable = 1;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->special_alternate_node = CONTENT_WOOD_BARREL;
	content_nodebox_barrel_sealed(f);
	f->setInventoryTextureNodeBox(i,"wood.png","wood.png^[transformR90","wood.png^[transformR90");
	f->type = CMT_WOOD;
	f->dig_time = 1.0;
	f->pressure_type = CST_SOLID;
	f->suffocation_per_second = 0;
	if (f->initial_metadata == NULL)
		f->initial_metadata = new SealedBarrelNodeMetadata();

	i = CONTENT_APPLEWOOD_BARREL_SEALED;
	f = &content_features(i);
	f->description = gettext("Applewood Barrel");
	f->setAllTextures("applewood.png^[transformR90");
	f->setAllMetaTextures("water.png",WATER_ALPHA);
	f->param_type = CPT_LIGHT;
	f->item_param_type = CPT_METADATA;
	f->draw_type = CDT_NODEBOX_META;
	f->rotate_tile_with_nodebox = true;
	f->light_propagates = true;
	f->air_equivalent = true;
	f->flammable = 1;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->special_alternate_node = CONTENT_APPLEWOOD_BARREL;
	content_nodebox_barrel_sealed(f);
	f->setInventoryTextureNodeBox(i,"applewood.png","applewood.png^[transformR90","applewood.png^[transformR90");
	f->type = CMT_WOOD;
	f->dig_time = 1.0;
	f->pressure_type = CST_SOLID;
	f->suffocation_per_second = 0;
	if (f->initial_metadata == NULL)
		f->initial_metadata = new SealedBarrelNodeMetadata();

	i = CONTENT_JUNGLEWOOD_BARREL_SEALED;
	f = &content_features(i);
	f->description = gettext("Junglewood Barrel");
	f->setAllTextures("junglewood.png^[transformR90");
	f->setAllMetaTextures("water.png",WATER_ALPHA);
	f->param_type = CPT_LIGHT;
	f->item_param_type = CPT_METADATA;
	f->draw_type = CDT_NODEBOX_META;
	f->rotate_tile_with_nodebox = true;
	f->light_propagates = true;
	f->air_equivalent = true;
	f->flammable = 1;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->special_alternate_node = CONTENT_JUNGLEWOOD_BARREL;
	content_nodebox_barrel_sealed(f);
	f->setInventoryTextureNodeBox(i,"junglewood.png","junglewood.png^[transformR90","junglewood.png^[transformR90");
	f->type = CMT_WOOD;
	f->dig_time = 1.0;
	f->pressure_type = CST_SOLID;
	f->suffocation_per_second = 0;
	if (f->initial_metadata == NULL)
		f->initial_metadata = new SealedBarrelNodeMetadata();

	i = CONTENT_PINE_BARREL_SEALED;
	f = &content_features(i);
	f->description = gettext("Pine Barrel");
	f->setAllTextures("pine.png^[transformR90");
	f->setAllMetaTextures("water.png",WATER_ALPHA);
	f->param_type = CPT_LIGHT;
	f->item_param_type = CPT_METADATA;
	f->draw_type = CDT_NODEBOX_META;
	f->rotate_tile_with_nodebox = true;
	f->light_propagates = true;
	f->air_equivalent = true;
	f->flammable = 1;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->special_alternate_node = CONTENT_PINE_BARREL;
	content_nodebox_barrel_sealed(f);
	f->setInventoryTextureNodeBox(i,"pine.png","pine.png^[transformR90","pine.png^[transformR90");
	f->type = CMT_WOOD;
	f->dig_time = 1.0;
	f->pressure_type = CST_SOLID;
	f->suffocation_per_second = 0;
	if (f->initial_metadata == NULL)
		f->initial_metadata = new SealedBarrelNodeMetadata();

	i = CONTENT_FIRE;
	f = &content_features(i);
	f->description = gettext("Fire");
	f->setAllTextures("fire.png");
	f->setAllTextureFlags(0);
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_FIRELIKE;
	f->light_propagates = true;
	f->light_source = LIGHT_MAX-1;
	f->walkable = false;
	f->pointable = false;
	f->diggable = false;
	f->buildable_to = true;
	f->damage_per_second = 40;
	f->sound_ambient = "env-fire";
#ifndef SERVER
	f->post_effect_color = video::SColor(192, 255, 64, 0);
#endif
	f->pressure_type = CST_CRUSHED;

	i = CONTENT_FIRE_SHORTTERM;
	f = &content_features(i);
	f->description = gettext("Fire");
	f->setAllTextures("fire.png");
	f->setAllTextureFlags(0);
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_FIRELIKE;
	f->light_propagates = true;
	f->light_source = LIGHT_MAX-1;
	f->walkable = false;
	f->pointable = false;
	f->diggable = false;
	f->buildable_to = true;
	f->damage_per_second = 40;
#ifndef SERVER
	f->post_effect_color = video::SColor(192, 255, 64, 0);
#endif
	f->pressure_type = CST_CRUSHED;

	i = CONTENT_TORCH;
	f = &content_features(i);
	f->description = gettext("Torch");
	f->setAllTextures("torch.png");
	f->setInventoryTexture("torch_inventory.png");
	f->setAllTextureFlags(0);
	f->param_type = CPT_LIGHT;
	f->param2_type = CPT_FACEDIR_WALLMOUNT;
	f->draw_type = CDT_TORCHLIKE;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->walkable = false;
	f->air_equivalent = true;
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_TORCH;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->light_source = LIGHT_MAX-1;
	f->type = CMT_WOOD;
	f->dig_time = 0.0;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	crafting::set1over4Recipe(CONTENT_CRAFTITEM_COAL,CONTENT_CRAFTITEM_STICK,CONTENT_TORCH);
	crafting::set1over4Recipe(CONTENT_CRAFTITEM_CHARCOAL,CONTENT_CRAFTITEM_STICK,CONTENT_TORCH);
	content_list_add("craftguide",i,1,0);
	content_list_add("player-creative",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CHEST;
	f = &content_features(i);
	f->description = gettext("Chest");
	f->draw_type = CDT_NODEBOX_META;
	f->setAllTextures("wood.png^chests_side.png");
	f->setTexture(0, "wood.png^chests_top.png");
	f->setTexture(1, "wood.png^chests_top.png");
	f->setTexture(4, "wood.png^chests_back.png");
	f->setTexture(5, "wood.png^chests_front.png"); // Z-
	f->setMetaTexture(0,"chests_meta_top.png");
	f->setMetaTexture(1,"chests_meta_bottom.png");
	f->setMetaTexture(2,"chests_meta_side.png");
	f->setMetaTexture(3,"chests_meta_side.png^[transformFX");
	f->setMetaTexture(4,"chests_meta_exo.png");
	f->setMetaTexture(5,"chests_meta_lock.png");
	f->param_type = CPT_LIGHT;
	f->param2_type = CPT_FACEDIR_SIMPLE;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->air_equivalent = true;
	f->rotate_tile_with_nodebox = true;
	content_nodebox_chest(f);
	f->setInventoryTextureNodeBox(i,"wood.png^chests_top.png", "wood.png^chests_front.png", "wood.png^chests_side.png");
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	if (f->initial_metadata == NULL)
		f->initial_metadata = new ChestNodeMetadata();
	f->sound_access = "open-chest";
	f->type = CMT_WOOD;
	f->dig_time = 1.0;
	f->pressure_type = CST_SOLID;
	crafting::setRoundRecipe(CONTENT_WOOD,CONTENT_CHEST);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CHEST_PINE;
	f = &content_features(i);
	f->description = gettext("Chest");
	f->draw_type = CDT_NODEBOX_META;
	f->setAllTextures("pine.png^chests_side.png");
	f->setTexture(0, "pine.png^chests_top.png");
	f->setTexture(1, "pine.png^chests_top.png");
	f->setTexture(4, "pine.png^chests_back.png");
	f->setTexture(5, "pine.png^chests_front.png"); // Z-
	f->setMetaTexture(0,"chests_meta_top.png");
	f->setMetaTexture(1,"chests_meta_bottom.png");
	f->setMetaTexture(2,"chests_meta_side.png^[transformFX");
	f->setMetaTexture(3,"chests_meta_side.png");
	f->setMetaTexture(4,"chests_meta_lock.png");
	f->setMetaTexture(5,"chests_meta_exo.png");
	f->param_type = CPT_LIGHT;
	f->param2_type = CPT_FACEDIR_SIMPLE;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->air_equivalent = true;
	f->rotate_tile_with_nodebox = true;
	content_nodebox_chest(f);
	f->setInventoryTextureNodeBox(i,"pine.png^chests_top.png", "pine.png^chests_front.png", "pine.png^chests_side.png");
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	if (f->initial_metadata == NULL)
		f->initial_metadata = new ChestNodeMetadata();
	f->sound_access = "open-chest";
	f->type = CMT_WOOD;
	f->dig_time = 1.0;
	f->pressure_type = CST_SOLID;
	crafting::setRoundRecipe(CONTENT_WOOD_PINE,CONTENT_CHEST_PINE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CHEST_JUNGLE;
	f = &content_features(i);
	f->description = gettext("Chest");
	f->draw_type = CDT_NODEBOX_META;
	f->setAllTextures("junglewood.png^chests_side.png");
	f->setTexture(0, "junglewood.png^chests_top.png");
	f->setTexture(1, "junglewood.png^chests_top.png");
	f->setTexture(4, "junglewood.png^chests_back.png");
	f->setTexture(5, "junglewood.png^chests_front.png"); // Z-
	f->setMetaTexture(0,"chests_meta_top.png");
	f->setMetaTexture(1,"chests_meta_bottom.png");
	f->setMetaTexture(2,"chests_meta_side.png^[transformFX");
	f->setMetaTexture(3,"chests_meta_side.png");
	f->setMetaTexture(4,"chests_meta_lock.png");
	f->setMetaTexture(5,"chests_meta_exo.png");
	f->param_type = CPT_LIGHT;
	f->param2_type = CPT_FACEDIR_SIMPLE;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->air_equivalent = true;
	f->rotate_tile_with_nodebox = true;
	content_nodebox_chest(f);
	f->setInventoryTextureNodeBox(i,"junglewood.png^chests_top.png", "junglewood.png^chests_front.png", "junglewood.png^chests_side.png");
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	if (f->initial_metadata == NULL)
		f->initial_metadata = new ChestNodeMetadata();
	f->sound_access = "open-chest";
	f->type = CMT_WOOD;
	f->dig_time = 1.0;
	f->pressure_type = CST_SOLID;
	crafting::setRoundRecipe(CONTENT_JUNGLEWOOD,CONTENT_CHEST_JUNGLE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CHEST_APPLE;
	f = &content_features(i);
	f->description = gettext("Chest");
	f->draw_type = CDT_NODEBOX_META;
	f->setAllTextures("applewood.png^chests_side.png");
	f->setTexture(0, "applewood.png^chests_top.png");
	f->setTexture(1, "applewood.png^chests_top.png");
	f->setTexture(4, "applewood.png^chests_back.png");
	f->setTexture(5, "applewood.png^chests_front.png"); // Z-
	f->setMetaTexture(0,"chests_meta_top.png");
	f->setMetaTexture(1,"chests_meta_bottom.png");
	f->setMetaTexture(2,"chests_meta_side.png^[transformFX");
	f->setMetaTexture(3,"chests_meta_side.png");
	f->setMetaTexture(4,"chests_meta_lock.png");
	f->setMetaTexture(5,"chests_meta_exo.png");
	f->param_type = CPT_LIGHT;
	f->param2_type = CPT_FACEDIR_SIMPLE;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->air_equivalent = true;
	f->rotate_tile_with_nodebox = true;
	content_nodebox_chest(f);
	f->setInventoryTextureNodeBox(i,"applewood.png^chests_top.png", "applewood.png^chests_front.png", "applewood.png^chests_side.png");
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	if (f->initial_metadata == NULL)
		f->initial_metadata = new ChestNodeMetadata();
	f->sound_access = "open-chest";
	f->type = CMT_WOOD;
	f->dig_time = 1.0;
	f->pressure_type = CST_SOLID;
	crafting::setRoundRecipe(CONTENT_APPLEWOOD,CONTENT_CHEST_APPLE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CHEST_DEPRECATED;
	f = &content_features(i);
	f->description = gettext("Chest");
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->draw_type = CDT_CUBELIKE;
	f->setAllTextures("chest_side.png");
	f->setTexture(0, "chest_top.png");
	f->setTexture(1, "chest_top.png");
	f->setTexture(5, "chest_front.png"); // Z-
	f->setInventoryTexture("chest_top.png");
	f->setInventoryTextureCube("chest_top.png", "chest_front.png", "chest_side.png");
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_CHEST)+" 1";
	if(f->initial_metadata == NULL)
		f->initial_metadata = new DeprecatedChestNodeMetadata();
	f->sound_access = "open-chest";
	f->type = CMT_WOOD;
	f->dig_time = 1.0;
	f->pressure_type = CST_SOLID;

	i = CONTENT_CREATIVE_CHEST;
	f = &content_features(i);
	f->description = gettext("Creative Chest");
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->draw_type = CDT_CUBELIKE;
	f->setAllTextures("chest_side.png");
	f->setTexture(0, "chest_top.png");
	f->setTexture(1, "chest_top.png");
	f->setTexture(5, "chest_creative.png"); // Z-
	f->setInventoryTexture("chest_top.png");
	f->setInventoryTextureCube("chest_top.png", "chest_creative.png", "chest_side.png");
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	if(f->initial_metadata == NULL)
		f->initial_metadata = new CreativeChestNodeMetadata();
	f->sound_access = "open-chest";
	f->type = CMT_WOOD;
	f->dig_time = 1.0;
	f->pressure_type = CST_SOLID;
	content_list_add("player-creative",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_LOCKABLE_CHEST_DEPRECATED;
	f = &content_features(i);
	f->description = gettext("Locking Chest");
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->draw_type = CDT_CUBELIKE;
	f->setAllTextures("chest_side.png");
	f->setTexture(0, "chest_top.png");
	f->setTexture(1, "chest_top.png");
	f->setTexture(5, "chest_lock.png"); // Z-
	f->setInventoryTexture("chest_lock.png");
	f->setInventoryTextureCube("chest_top.png", "chest_lock.png", "chest_side.png");
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_CHEST)+" 1";
	/* TODO: give padlock */
	if(f->initial_metadata == NULL)
		f->initial_metadata = new LockingDeprecatedChestNodeMetadata();
	f->sound_access = "open-chest";
	f->type = CMT_WOOD;
	f->dig_time = 1.0;
	f->pressure_type = CST_SOLID;

	i = CONTENT_SAFE;
	f = &content_features(i);
	f->description = gettext("Safe");
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->draw_type = CDT_CUBELIKE;
	f->setAllTextures("safe_side.png");
	f->setTexture(0, "safe_top.png");
	f->setTexture(1, "safe_top.png");
	f->setTexture(5, "safe_lock.png"); // Z-
	f->setInventoryTexture("safe_lock.png");
	f->setInventoryTextureCube("safe_top.png", "safe_lock.png", "safe_side.png");
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	if(f->initial_metadata == NULL)
		f->initial_metadata = new SafeNodeMetadata();
	f->type = CMT_WOOD;
	f->dig_time = 1.0;
	f->pressure_type = CST_SOLID;
	f->destructive_mob_safe = true;
	crafting::setFilledRoundRecipe(CONTENT_CRAFTITEM_IRON_INGOT,CONTENT_CRAFTITEM_COPPER_INGOT,CONTENT_SAFE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_FURNACE;
	f = &content_features(i);
	f->description = gettext("Furnace");
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->draw_type = CDT_NODEBOX_META;
	f->setAllTextures("terracotta.png");
	f->setAllMetaTextures("fire.png");
	content_nodebox_furnace(f);
	f->setInventoryTextureNodeBox(i,"terracotta.png", "terracotta.png", "terracotta.png");
	f->rotate_tile_with_nodebox = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_FURNACE)+" 1";
	if(f->initial_metadata == NULL)
		f->initial_metadata = new FurnaceNodeMetadata();
	f->type = CMT_STONE;
	f->dig_time = 3.0;
	f->pressure_type = CST_SOLID;
	{
		content_t recipe[9] = {
			CONTENT_CRAFTITEM_CLAY,		CONTENT_CRAFTITEM_CLAY,		CONTENT_CRAFTITEM_CLAY,
			CONTENT_CRAFTITEM_CLAY,		CONTENT_CRAFTITEM_BRONZE_INGOT,	CONTENT_CRAFTITEM_CLAY,
			CONTENT_CRAFTITEM_CLAY,		CONTENT_CRAFTITEM_CLAY,		CONTENT_CRAFTITEM_CLAY
		};
		crafting::setRecipe(recipe,i,1,0);
	}
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_SMELTERY;
	f = &content_features(i);
	f->description = gettext("Smeltery");
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->draw_type = CDT_NODEBOX_META;
	f->setAllTextures("iron_sheet_worn.png");
	f->setMetaTexture(0,"iron_sheet.png"); // Y+
	f->setMetaTexture(1,"iron_sheet.png"); // Y-
	f->setMetaTexture(2,"iron_sheet.png"); // X+
	f->setMetaTexture(3,"iron_sheet.png"); // X-
	f->setMetaTexture(4,"smeltery_meta_exo.png"); // Z+
	f->setMetaTexture(5,"smeltery_meta_fire.png"); // Z-
	content_nodebox_smeltery(f);
	f->setInventoryTextureNodeBox(i,"iron_sheet_worn.png", "iron_sheet_worn.png", "iron_sheet_worn.png");
	f->rotate_tile_with_nodebox = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	if(f->initial_metadata == NULL)
		f->initial_metadata = new SmelteryNodeMetadata();
	f->type = CMT_STONE;
	f->dig_time = 3.0;
	f->pressure_type = CST_SOLID;
	{
		content_t recipe[9] = {
			CONTENT_CRAFTITEM_IRON_INGOT,	CONTENT_CRAFTITEM_IRON_INGOT,	CONTENT_CRAFTITEM_IRON_INGOT,
			CONTENT_CRAFTITEM_IRON_INGOT,	CONTENT_STONE,			CONTENT_CRAFTITEM_IRON_INGOT,
			CONTENT_CRAFTITEM_IRON_INGOT,	CONTENT_CRAFTITEM_IRON_INGOT,	CONTENT_CRAFTITEM_IRON_INGOT
		};
		crafting::setRecipe(recipe,i,1,0);
	}
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_FURNACE_DEPRECATED;
	f = &content_features(i);
	f->description = gettext("Furnace");
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->draw_type = CDT_CUBELIKE;
	f->setAllTextures("furnace_side.png");
	f->setTexture(0, "furnace_top.png");
	f->setTexture(1, "furnace_top.png^[transformFY");
	f->setTexture(2, "furnace_side.png^[transformFX");
	f->setTexture(4, "furnace_back.png");
	f->setTexture(5, "furnace_front.png"); // Z-
	f->setAllMetaTextures("fire.png");
	f->setInventoryTextureCube("furnace_top.png^[transformR90", "furnace_front.png", "furnace_side.png^[transformFX");
	f->rotate_tile_with_nodebox = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_FURNACE)+" 1";
	if(f->initial_metadata == NULL)
		f->initial_metadata = new DeprecatedFurnaceNodeMetadata();
	f->type = CMT_STONE;
	f->dig_time = 3.0;
	f->pressure_type = CST_SOLID;

	i = CONTENT_LOCKABLE_FURNACE_DEPRECATED;
	f = &content_features(i);
	f->description = gettext("Locking Furnace");
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->draw_type = CDT_CUBELIKE;
	f->setAllTextures("furnace_side.png");
	f->setTexture(0, "furnace_top.png");
	f->setTexture(1, "furnace_top.png^[transformFY");
	f->setTexture(2, "furnace_side.png^[transformFX");
	f->setTexture(4, "furnace_back.png");
	f->setTexture(5, "furnace_lock.png"); // Z-
	f->setAllMetaTextures("fire.png");
	f->setInventoryTextureCube("furnace_top.png^[transformR90", "furnace_lock.png", "furnace_side.png^[transformFX");
	f->rotate_tile_with_nodebox = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_FURNACE)+" 1";
	if(f->initial_metadata == NULL)
		f->initial_metadata = new LockingDeprecatedFurnaceNodeMetadata();
	f->type = CMT_STONE;
	f->dig_time = 3.0;
	f->pressure_type = CST_SOLID;

	i = CONTENT_INCINERATOR;
	f = &content_features(i);
	f->description = gettext("Incinerator");
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->draw_type = CDT_NODEBOX_META;
	f->setAllTextures("incinerator_side.png");
	f->setTexture(0, "incinerator_top.png");
	f->setTexture(1, "incinerator_top.png^[transformFY");
	f->setTexture(2, "incinerator_side.png^[transformFX");
	f->setTexture(4, "incinerator_back.png");
	f->setTexture(5, "incinerator_front.png"); // Z-
	f->setAllMetaTextures("fire.png");
	content_nodebox_incinerator(f);
	f->setInventoryTextureNodeBox(i,"incinerator_top.png^[transformR90", "incinerator_front.png", "incinerator_side.png^[transformFX");
	f->rotate_tile_with_nodebox = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	if(f->initial_metadata == NULL)
		f->initial_metadata = new IncineratorNodeMetadata();
	f->type = CMT_STONE;
	f->dig_time = 0.4;
	f->pressure_type = CST_SOLID;
	crafting::setFilledRoundRecipe(CONTENT_ROUGHSTONE,CONTENT_CRAFTITEM_MITHRIL_UNBOUND,CONTENT_INCINERATOR);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_INCINERATOR_ACTIVE;
	f = &content_features(i);
	f->description = gettext("Incinerator");
	f->param_type = CPT_LIGHT;
	f->param2_type = CPT_FACEDIR_SIMPLE;
	f->draw_type = CDT_CUBELIKE;
	f->light_source = 9;
	f->setAllTextures("incinerator_side.png");
	f->setTexture(0, "incinerator_top.png"); // Z-
	f->setTexture(1, "incinerator_top.png"); // Z-
	f->setTexture(5, "incinerator_front_active.png"); // Z-
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_INCINERATOR)+" 1";
	if (f->initial_metadata == NULL)
		f->initial_metadata = new IncineratorNodeMetadata();
	f->type = CMT_STONE;
	f->dig_time = 0.4;
	f->pressure_type = CST_SOLID;



	i = CONTENT_CAMPFIRE;
	f = &content_features(i);
	f->description = gettext("Camp Fire");
	f->draw_type = CDT_CAMPFIRELIKE;
	f->param_type = CPT_LIGHT;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->walkable = false;
	f->fuel_time = BT_CAMP_FIRE;
	f->light_source = LIGHT_MAX-3;
	f->damage_per_second = 10;
	f->setAllTextures("campfire_stones.png");
	f->setTexture(1, "campfire_logs.png");
#ifndef SERVER
	f->animation_length = 2.0;
	f->setTextureFlags(2, MATERIAL_FLAG_ANIMATION_VERTICAL_FRAMES);
	f->setTextureFlags(3, MATERIAL_FLAG_ANIMATION_VERTICAL_FRAMES);
#endif
	f->setTexture(2, "campfire_embers.png");
	f->setTexture(3, "campfire_flame.png");
	content_nodebox_roofcollide(f);
	f->setInventoryTexture("campfire_inv.png");
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	if(f->initial_metadata == NULL)
		f->initial_metadata = new CampFireNodeMetadata();
	f->type = CMT_WOOD;
	f->dig_time = 2.0;
	f->pressure_type = CST_CRUSHED;
	{
		u16 r[9] = {
			CONTENT_ROCK,			CONTENT_CRAFTITEM_STICK,	CONTENT_ROCK,
			CONTENT_CRAFTITEM_STICK,	CONTENT_IGNORE,			CONTENT_CRAFTITEM_STICK,
			CONTENT_ROCK,			CONTENT_CRAFTITEM_STICK,	CONTENT_ROCK
		};
		crafting::setRecipe(r,CONTENT_CAMPFIRE,1);
	}
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRUSHER;
	f = &content_features(i);
	f->description = gettext("Crusher");
	f->param_type = CPT_FACEDIR_SIMPLE;
	f->draw_type = CDT_NODEBOX_META;
	f->setAllTextures("iron_sheet_worn.png");
	f->setAllMetaTextures("iron_sheet_worn.png");
	content_nodebox_crusher(f);
	f->setInventoryTextureNodeBox(i,"iron_sheet_worn.png", "iron_sheet_worn.png", "iron_sheet_worn.png");
	f->rotate_tile_with_nodebox = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	if(f->initial_metadata == NULL)
		f->initial_metadata = new CrusherNodeMetadata();
	f->type = CMT_STONE;
	f->dig_time = 3.0;
	f->pressure_type = CST_SOLID;
	crafting::setFilledRoundRecipe(CONTENT_CRAFTITEM_IRON_INGOT,CONTENT_CRAFTITEM_TIN_INGOT,CONTENT_CRUSHER);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_NC;
	f = &content_features(i);
	f->description = gettext("Nyan Cat");
	f->draw_type = CDT_CUBELIKE;
	f->setAllTextures("mithril_block.png");
	f->setInventoryTextureCube("mithril_block.png", "mithril_block.png", "mithril_block.png");
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_MITHRIL_BLOCK)+" 1";
	f->type = CMT_STONE;
	f->dig_time = 3.0;

	i = CONTENT_NC_RB;
	f = &content_features(i);
	f->description = gettext("Rainbow");
	f->draw_type = CDT_CUBELIKE;
	f->setAllTextures("mithril_block.png");
	f->setInventoryTextureCube("mithril_block.png", "mithril_block.png", "mithril_block.png");
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_MITHRIL_BLOCK)+" 1";
	f->type = CMT_STONE;
	f->dig_time = 3.0;

	i = CONTENT_FLOWER_POT_RAW;
	f = &content_features(i);
	f->param_type = CPT_LIGHT;
	f->description = gettext("Unfired Flower Pot");
	f->setAllTextures("clay.png");
	f->setTexture(0,"clay.png^flower_pot_top.png");
	f->draw_type = CDT_NODEBOX;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->air_equivalent = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->cook_result = std::string("MaterialItem2 ")+itos(CONTENT_FLOWER_POT)+" 1";
	f->type = CMT_STONE;
	f->dig_time = 0.75;
	f->pressure_type = CST_CRUSHABLE;
	content_nodebox_flower_pot(f);
	f->setInventoryTextureNodeBox(i,"clay.png^flower_pot_top.png","clay.png","clay.png");
	crafting::setVRecipe(CONTENT_CRAFTITEM_CLAY,CONTENT_FLOWER_POT_RAW);
	content_list_add("craftguide",i,1,0);
	content_list_add("cooking",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_FLOWER_POT;
	f = &content_features(i);
	f->param_type = CPT_LIGHT;
	f->description = gettext("Flower Pot");
	f->setAllTextures("terracotta.png");
	f->setTexture(0,"terracotta.png^flower_pot_top.png");
	f->draw_type = CDT_NODEBOX;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->air_equivalent = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->type = CMT_STONE;
	f->dig_time = 0.75;
	content_nodebox_flower_pot(f);
	f->setInventoryTextureNodeBox(i,"terracotta.png^flower_pot_top.png","terracotta.png","terracotta.png");
	content_list_add("creative",i,1,0);

	// walls
	i = CONTENT_COBBLE_WALL;
	f = &content_features(i);
	f->description = gettext("Cobblestone Wall");
	f->setAllTextures("cobble.png");
	f->light_propagates = true;
	f->jumpable = false;
	f->param_type = CPT_LIGHT;
	f->param2_type = CPT_SPECIAL;
	f->draw_type = CDT_WALLLIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->air_equivalent = true; // grass grows underneath
	content_nodebox_wall_inv(f);
	content_nodebox_wall(f);
	f->setInventoryTextureNodeBox(i,"cobble.png","cobble.png","cobble.png");
	f->special_alternate_node = CONTENT_COBBLE;
	f->type = CMT_STONE;
	f->dig_time = 0.9;
	f->suffocation_per_second = 0;
	crafting::setWallRecipe(CONTENT_COBBLE,CONTENT_COBBLE_WALL);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_ROUGHSTONE_WALL;
	f = &content_features(i);
	f->description = gettext("Rough Stone Wall");
	f->setAllTextures("roughstone.png");
	f->light_propagates = true;
	f->jumpable = false;
	f->param_type = CPT_LIGHT;
	f->param2_type = CPT_SPECIAL;
	f->draw_type = CDT_WALLLIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->air_equivalent = true; // grass grows underneath
	content_nodebox_wall_inv(f);
	content_nodebox_wall(f);
	f->setInventoryTextureNodeBox(i,"roughstone.png","roughstone.png","roughstone.png");
	f->special_alternate_node = CONTENT_ROUGHSTONE;
	f->type = CMT_STONE;
	f->dig_time = 0.9;
	f->suffocation_per_second = 0;
	crafting::setWallRecipe(CONTENT_ROUGHSTONE,CONTENT_ROUGHSTONE_WALL);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_MOSSYCOBBLE_WALL;
	f = &content_features(i);
	f->description = gettext("Mossy Cobblestone Wall");
	f->setAllTextures("mossycobble.png");
	f->light_propagates = true;
	f->jumpable = false;
	f->param_type = CPT_LIGHT;
	f->param2_type = CPT_SPECIAL;
	f->draw_type = CDT_WALLLIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->air_equivalent = true; // grass grows underneath
	content_nodebox_wall_inv(f);
	content_nodebox_wall(f);
	f->setInventoryTextureNodeBox(i,"mossycobble.png","mossycobble.png","mossycobble.png");
	f->special_alternate_node = CONTENT_MOSSYCOBBLE;
	f->type = CMT_STONE;
	f->dig_time = 0.8;
	f->suffocation_per_second = 0;
	crafting::setWallRecipe(CONTENT_MOSSYCOBBLE,CONTENT_MOSSYCOBBLE_WALL);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_STONE_WALL;
	f = &content_features(i);
	f->description = gettext("Stone Wall");
	f->setAllTextures("stone.png");
	f->light_propagates = true;
	f->jumpable = false;
	f->param_type = CPT_LIGHT;
	f->param2_type = CPT_SPECIAL;
	f->draw_type = CDT_WALLLIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->air_equivalent = true; // grass grows underneath
	content_nodebox_wall_inv(f);
	content_nodebox_wall(f);
	f->setInventoryTextureNodeBox(i,"stone.png","stone.png","stone.png");
	f->special_alternate_node = CONTENT_STONE;
	f->type = CMT_STONE;
	f->dig_time = 1.0;
	f->suffocation_per_second = 0;
	crafting::setWallRecipe(CONTENT_STONE,CONTENT_STONE_WALL);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_SANDSTONE_WALL;
	f = &content_features(i);
	f->description = gettext("Sand Stone Wall");
	f->setAllTextures("sandstone.png");
	f->light_propagates = true;
	f->jumpable = false;
	f->param_type = CPT_LIGHT;
	f->param2_type = CPT_SPECIAL;
	f->draw_type = CDT_WALLLIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->air_equivalent = true; // grass grows underneath
	content_nodebox_wall_inv(f);
	content_nodebox_wall(f);
	f->setInventoryTextureNodeBox(i,"sandstone.png","sandstone.png","sandstone.png");
	f->special_alternate_node = CONTENT_SANDSTONE;
	f->type = CMT_DIRT;
	f->dig_time = 1.0;
	f->suffocation_per_second = 0;
	crafting::setWallRecipe(CONTENT_SANDSTONE,CONTENT_SANDSTONE_WALL);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_LIMESTONE_WALL;
	f = &content_features(i);
	f->description = gettext("Limestone Wall");
	f->setAllTextures("limestone.png");
	f->light_propagates = true;
	f->jumpable = false;
	f->param_type = CPT_LIGHT;
	f->param2_type = CPT_SPECIAL;
	f->draw_type = CDT_WALLLIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->air_equivalent = true; // grass grows underneath
	content_nodebox_wall_inv(f);
	content_nodebox_wall(f);
	f->setInventoryTextureNodeBox(i,"limestone.png","limestone.png","limestone.png");
	f->special_alternate_node = CONTENT_LIMESTONE;
	f->type = CMT_STONE;
	f->dig_time = 0.9;
	f->suffocation_per_second = 0;
	crafting::setWallRecipe(CONTENT_LIMESTONE,CONTENT_LIMESTONE_WALL);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_MARBLE_WALL;
	f = &content_features(i);
	f->description = gettext("Marble Wall");
	f->setAllTextures("marble.png");
	f->light_propagates = true;
	f->jumpable = false;
	f->param_type = CPT_LIGHT;
	f->param2_type = CPT_SPECIAL;
	f->draw_type = CDT_WALLLIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->air_equivalent = true; // grass grows underneath
	content_nodebox_wall_inv(f);
	content_nodebox_wall(f);
	f->setInventoryTextureNodeBox(i,"marble.png","marble.png","marble.png");
	f->special_alternate_node = CONTENT_MARBLE;
	f->type = CMT_STONE;
	f->dig_time = 1.0;
	f->suffocation_per_second = 0;
	crafting::setWallRecipe(CONTENT_MARBLE,CONTENT_MARBLE_WALL);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_TNT;
	f = &content_features(i);
	f->description = gettext("TNT");
	f->setAllTextures("tnt.png");
	f->setTexture(0, "tnt_top.png");
	f->setTexture(1, "tnt_bottom.png");
	f->setInventoryTextureCube("tnt_top.png", "tnt.png", "tnt.png");
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->energy_type = CET_DEVICE;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	if(f->initial_metadata == NULL)
		f->initial_metadata = new TNTNodeMetadata();
	f->type = CMT_DIRT;
	f->dig_time = 1.0;
	f->suffocation_per_second = 0;
	crafting::setSoftBlockRecipe(CONTENT_CRAFTITEM_TNT,CONTENT_TNT);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_FLASH;
	f = &content_features(i);
	f->description = gettext("In-Progress explosion - how did you get this???");
	f->setAllTextures("flash.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_GLASSLIKE;
	f->light_propagates = true;
	f->light_source = LIGHT_MAX-1;
	f->walkable = false;
	f->pointable = false;
	f->diggable = false;
	f->buildable_to = true;
	f->damage_per_second = 80;
	f->pressure_type = CST_CRUSHED;
#ifndef SERVER
	f->setAllTextureTypes(MATERIAL_ALPHA_BLEND);
	f->post_effect_color = video::SColor(192, 255, 255, 64);
#endif

	i = CONTENT_STEAM;
	f = &content_features(i);
	f->description = gettext("Steam");
	f->setAllTextures("steam.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_GLASSLIKE;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->walkable = false;
	f->pointable = false;
	f->diggable = false;
	f->buildable_to = true;
	f->damage_per_second = 20;
	f->pressure_type = CST_CRUSHED;
#ifndef SERVER
	f->setAllTextureTypes(MATERIAL_ALPHA_BLEND);
	f->post_effect_color = video::SColor(120, 200, 200, 200);
#endif

	// flags
	i = CONTENT_FLAG;
	f = &content_features(i);
	f->description = gettext("Home Flag");
	f->setAllTextures("flag.png");
	f->setAllTextureFlags(0);
	f->light_propagates = true;
	f->walkable = false;
	f->param_type = CPT_LIGHT;
	f->param2_type = CPT_FACEDIR_SIMPLE;
	f->draw_type = CDT_FLAGLIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->air_equivalent = true; // grass grows underneath
	content_nodebox_flag(f);
	f->home_node = PLAYERFLAG_WHITE;
	f->type = CMT_WOOD;
	f->dig_time = 0.4;
	f->suffocation_per_second = 0;
	{
		content_t r[9] = {
			CONTENT_CRAFTITEM_STICK,	CONTENT_CRAFTITEM_PAPER,	CONTENT_IGNORE,
			CONTENT_CRAFTITEM_STICK,	CONTENT_IGNORE,			CONTENT_IGNORE,
			CONTENT_IGNORE,			CONTENT_IGNORE,			CONTENT_IGNORE
		};
		crafting::setRecipe(r,CONTENT_FLAG,1);
	}
	crafting::set1Any2Recipe(CONTENT_FLAG_BLUE,CONTENT_CRAFTITEM_DYE_WHITE,CONTENT_FLAG);
	crafting::set1Any2Recipe(CONTENT_FLAG_BLUE,CONTENT_CRAFTITEM_STARCH,CONTENT_FLAG);
	crafting::set1Any2Recipe(CONTENT_FLAG_GREEN,CONTENT_CRAFTITEM_DYE_WHITE,CONTENT_FLAG);
	crafting::set1Any2Recipe(CONTENT_FLAG_GREEN,CONTENT_CRAFTITEM_STARCH,CONTENT_FLAG);
	crafting::set1Any2Recipe(CONTENT_FLAG_ORANGE,CONTENT_CRAFTITEM_DYE_WHITE,CONTENT_FLAG);
	crafting::set1Any2Recipe(CONTENT_FLAG_ORANGE,CONTENT_CRAFTITEM_STARCH,CONTENT_FLAG);
	crafting::set1Any2Recipe(CONTENT_FLAG_PURPLE,CONTENT_CRAFTITEM_DYE_WHITE,CONTENT_FLAG);
	crafting::set1Any2Recipe(CONTENT_FLAG_PURPLE,CONTENT_CRAFTITEM_STARCH,CONTENT_FLAG);
	crafting::set1Any2Recipe(CONTENT_FLAG_RED,CONTENT_CRAFTITEM_DYE_WHITE,CONTENT_FLAG);
	crafting::set1Any2Recipe(CONTENT_FLAG_RED,CONTENT_CRAFTITEM_STARCH,CONTENT_FLAG);
	crafting::set1Any2Recipe(CONTENT_FLAG_YELLOW,CONTENT_CRAFTITEM_DYE_WHITE,CONTENT_FLAG);
	crafting::set1Any2Recipe(CONTENT_FLAG_YELLOW,CONTENT_CRAFTITEM_STARCH,CONTENT_FLAG);
	crafting::set1Any2Recipe(CONTENT_FLAG_BLACK,CONTENT_CRAFTITEM_DYE_WHITE,CONTENT_FLAG);
	crafting::set1Any2Recipe(CONTENT_FLAG_BLACK,CONTENT_CRAFTITEM_STARCH,CONTENT_FLAG);
	if(f->initial_metadata == NULL)
		f->initial_metadata = new FlagNodeMetadata();
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_FLAG_BLUE;
	f = &content_features(i);
	f->description = gettext("Blue Home Flag");
	f->setAllTextures("flag_blue.png");
	f->setAllTextureFlags(0);
	f->light_propagates = true;
	f->walkable = false;
	f->param_type = CPT_LIGHT;
	f->param2_type = CPT_FACEDIR_SIMPLE;
	f->draw_type = CDT_FLAGLIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->air_equivalent = true; // grass grows underneath
	content_nodebox_flag(f);
	f->home_node = PLAYERFLAG_BLUE;
	f->type = CMT_WOOD;
	f->dig_time = 0.4;
	f->suffocation_per_second = 0;
	crafting::set1Any2Recipe(CONTENT_FLAG,CONTENT_CRAFTITEM_DYE_BLUE,CONTENT_FLAG_BLUE);
	if(f->initial_metadata == NULL)
		f->initial_metadata = new FlagNodeMetadata();
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_FLAG_GREEN;
	f = &content_features(i);
	f->description = gettext("Green Home Flag");
	f->setAllTextures("flag_green.png");
	f->setAllTextureFlags(0);
	f->light_propagates = true;
	f->walkable = false;
	f->param_type = CPT_LIGHT;
	f->param2_type = CPT_FACEDIR_SIMPLE;
	f->draw_type = CDT_FLAGLIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->air_equivalent = true; // grass grows underneath
	content_nodebox_flag(f);
	f->home_node = PLAYERFLAG_GREEN;
	f->type = CMT_WOOD;
	f->dig_time = 0.4;
	f->suffocation_per_second = 0;
	crafting::set1Any2Recipe(CONTENT_FLAG,CONTENT_CRAFTITEM_DYE_GREEN,CONTENT_FLAG_GREEN);
	if(f->initial_metadata == NULL)
		f->initial_metadata = new FlagNodeMetadata();
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_FLAG_ORANGE;
	f = &content_features(i);
	f->description = gettext("Orange Home Flag");
	f->setAllTextures("flag_orange.png");
	f->setAllTextureFlags(0);
	f->light_propagates = true;
	f->walkable = false;
	f->param_type = CPT_LIGHT;
	f->param2_type = CPT_FACEDIR_SIMPLE;
	f->draw_type = CDT_FLAGLIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->air_equivalent = true; // grass grows underneath
	content_nodebox_flag(f);
	f->home_node = PLAYERFLAG_ORANGE;
	f->type = CMT_WOOD;
	f->dig_time = 0.4;
	f->suffocation_per_second = 0;
	crafting::set1Any2Recipe(CONTENT_FLAG,CONTENT_CRAFTITEM_DYE_ORANGE,CONTENT_FLAG_ORANGE);
	if(f->initial_metadata == NULL)
		f->initial_metadata = new FlagNodeMetadata();
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_FLAG_PURPLE;
	f = &content_features(i);
	f->description = gettext("Purple Home Flag");
	f->setAllTextures("flag_purple.png");
	f->setAllTextureFlags(0);
	f->light_propagates = true;
	f->walkable = false;
	f->param_type = CPT_LIGHT;
	f->param2_type = CPT_FACEDIR_SIMPLE;
	f->draw_type = CDT_FLAGLIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->air_equivalent = true; // grass grows underneath
	content_nodebox_flag(f);
	f->home_node = PLAYERFLAG_PURPLE;
	f->type = CMT_WOOD;
	f->dig_time = 0.4;
	f->suffocation_per_second = 0;
	crafting::set1Any2Recipe(CONTENT_FLAG,CONTENT_CRAFTITEM_DYE_PURPLE,CONTENT_FLAG_PURPLE);
	if(f->initial_metadata == NULL)
		f->initial_metadata = new FlagNodeMetadata();
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_FLAG_RED;
	f = &content_features(i);
	f->description = gettext("Red Home Flag");
	f->setAllTextures("flag_red.png");
	f->setAllTextureFlags(0);
	f->light_propagates = true;
	f->walkable = false;
	f->param_type = CPT_LIGHT;
	f->param2_type = CPT_FACEDIR_SIMPLE;
	f->draw_type = CDT_FLAGLIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->air_equivalent = true; // grass grows underneath
	content_nodebox_flag(f);
	f->home_node = PLAYERFLAG_RED;
	f->type = CMT_WOOD;
	f->dig_time = 0.4;
	f->suffocation_per_second = 0;
	crafting::set1Any2Recipe(CONTENT_FLAG,CONTENT_CRAFTITEM_DYE_RED,CONTENT_FLAG_RED);
	if(f->initial_metadata == NULL)
		f->initial_metadata = new FlagNodeMetadata();
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_FLAG_YELLOW;
	f = &content_features(i);
	f->description = gettext("Yellow Home Flag");
	f->setAllTextures("flag_yellow.png");
	f->setAllTextureFlags(0);
	f->light_propagates = true;
	f->walkable = false;
	f->param_type = CPT_LIGHT;
	f->param2_type = CPT_FACEDIR_SIMPLE;
	f->draw_type = CDT_FLAGLIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->air_equivalent = true; // grass grows underneath
	content_nodebox_flag(f);
	f->home_node = PLAYERFLAG_YELLOW;
	f->type = CMT_WOOD;
	f->dig_time = 0.4;
	f->suffocation_per_second = 0;
	crafting::set1Any2Recipe(CONTENT_FLAG,CONTENT_CRAFTITEM_DYE_YELLOW,CONTENT_FLAG_YELLOW);
	if(f->initial_metadata == NULL)
		f->initial_metadata = new FlagNodeMetadata();
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_FLAG_BLACK;
	f = &content_features(i);
	f->description = gettext("Black Home Flag");
	f->setAllTextures("flag_black.png");
	f->setAllTextureFlags(0);
	f->light_propagates = true;
	f->walkable = false;
	f->param_type = CPT_LIGHT;
	f->param2_type = CPT_FACEDIR_SIMPLE;
	f->draw_type = CDT_FLAGLIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->air_equivalent = true; // grass grows underneath
	content_nodebox_flag(f);
	f->home_node = PLAYERFLAG_BLACK;
	f->type = CMT_WOOD;
	f->dig_time = 0.4;
	f->suffocation_per_second = 0;
	crafting::set1Any2Recipe(CONTENT_FLAG,CONTENT_CRAFTITEM_DYE_BLACK,CONTENT_FLAG_BLACK);
	if(f->initial_metadata == NULL)
		f->initial_metadata = new FlagNodeMetadata();
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_LIFE_SUPPORT;
	f = &content_features(i);
	f->description = gettext("Life Support System");
	f->setAllTextures("life_support.png");
	f->setTexture(0, "life_support_top.png");
	f->setTexture(1, "life_support_bottom.png");
	f->setInventoryTextureCube("life_support_top.png", "life_support.png", "life_support.png");
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->type = CMT_DIRT;
	f->dig_time = 1.0;
	f->suffocation_per_second = 0;
	{
		content_t r[9] = {
			CONTENT_IGNORE,				CONTENT_SPONGE,				CONTENT_IGNORE,
			CONTENT_CRAFTITEM_MITHRIL_UNBOUND,	CONTENT_IGNORE,				CONTENT_CRAFTITEM_MITHRIL_UNBOUND,
			CONTENT_CRAFTITEM_MITHRIL_UNBOUND,	CONTENT_CRAFTITEM_MITHRIL_UNBOUND,	CONTENT_CRAFTITEM_MITHRIL_UNBOUND
		};
		crafting::setRecipe(r,CONTENT_LIFE_SUPPORT,1);
	}
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_PARCEL;
	f = &content_features(i);
	f->description = gettext("Parcel");
	f->setAllTextures("parcel.png");
	f->setTexture(0, "parcel_top.png");
	f->setTexture(1, "parcel_bottom.png");
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->walkable = false;
	f->param_type = CPT_LIGHT;
	f->rotate_tile_with_nodebox = true;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->buildable_to = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->air_equivalent = true; // grass grows underneath
	f->borderstone_diggable = true;
	content_nodebox_parcel(f);
	f->setInventoryTextureNodeBox(i,"parcel_top.png", "parcel.png", "parcel.png");
	f->type = CMT_DIRT;
	f->dig_time = 0.01;
	f->suffocation_per_second = 0;
	if (f->initial_metadata == NULL)
		f->initial_metadata = new ParcelNodeMetadata();

	i = CONTENT_CAULDRON;
	f = &content_features(i);
	f->description = gettext("Cauldron");
	f->setAllTextures("cauldron_outer.png");
	f->setAllMetaTextures("cauldron_inner.png");
	f->setMetaTexture(0,"water.png");
	f->draw_type = CDT_NODEBOX_META;
	f->type = CMT_STONE;
	f->dig_time = 3.0;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	content_nodebox_cauldron(f);
	f->setInventoryTextureNodeBox(i,"cauldron_outer.png", "cauldron_outer.png", "cauldron_outer.png");
	if (f->initial_metadata == NULL)
		f->initial_metadata = new CauldronNodeMetadata();
	crafting::setDeepURecipe(CONTENT_CRAFTITEM_IRON_INGOT,CONTENT_CAULDRON);
	f->pressure_type = CST_SOLID;
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);
	f->suffocation_per_second = 0;

	i = CONTENT_FORGE;
	f = &content_features(i);
	f->description = gettext("Forge");
	f->setAllTextures("forge_side.png");
	f->setTexture(0,"forge_top.png");
	f->setTexture(1,"forge_bottom.png");
	f->draw_type = CDT_NODEBOX;
	f->type = CMT_STONE;
	f->dig_time = 3.0;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	content_nodebox_forge(f);
	f->setInventoryTextureNodeBox(i,"forge_top.png", "forge_side.png", "forge_side.png");
	if (f->initial_metadata == NULL)
		f->initial_metadata = new ForgeNodeMetadata();
	{
		content_t r[9] = {
			CONTENT_ROUGHSTONE,	CONTENT_CHARCOAL,	CONTENT_ROUGHSTONE,
			CONTENT_ROUGHSTONE,	CONTENT_SAND,		CONTENT_ROUGHSTONE,
			CONTENT_ROUGHSTONE,	CONTENT_IRON,		CONTENT_ROUGHSTONE
		};
		crafting::setRecipe(r,CONTENT_FORGE,1);
	}
	f->pressure_type = CST_SOLID;
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);
	f->suffocation_per_second = 0;

	i = CONTENT_FORGE_FIRE;
	f = &content_features(i);
	f->description = gettext("Forge Fire");
	f->setAllTextures("forge_fire.png");
	f->setAllTextureFlags(0);
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_PLANTLIKE;
	f->light_propagates = true;
	f->light_source = LIGHT_MAX-4;
	f->walkable = false;
	f->pointable = false;
	f->diggable = false;
	f->buildable_to = true;
	f->sound_ambient = "env-fire";
#ifndef SERVER
	f->post_effect_color = video::SColor(192, 255, 64, 0);
#endif
	f->pressure_type = CST_CRUSHED;

	i = CONTENT_SCAFFOLDING;
	f = &content_features(i);
	f->description = gettext("Scaffolding");
	f->setAllTextures("wood.png");
	f->air_equivalent = true;
	f->climbable = true;
	f->cook_result = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_CHARCOAL)+" 1";
	f->draw_type = CDT_NODEBOX;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->flammable = 1;
        f->fuel_time = BT_SCAFFOLDING;
	f->dig_time = 0.75;
	f->is_ground_content = true;
	f->light_propagates = true;
	f->param_type = CPT_LIGHT;
	f->param2_type = CPT_FACEDIR_SIMPLE;
	f->pressure_type = CST_CRUSHABLE;
	f->type = CMT_WOOD;
	f->walkable = false;
	f->suffocation_per_second = 0;
	content_nodebox_scaffolding(f);
    f->setInventoryTextureNodeBox(i, "wood.png", "wood.png", "wood.png");
	{
		u16 r[9] = {
			CONTENT_IGNORE,                 CONTENT_WOOD_SLAB,              CONTENT_IGNORE,
			CONTENT_CRAFTITEM_WOOD_PLANK,   CONTENT_IGNORE,                 CONTENT_CRAFTITEM_WOOD_PLANK,
			CONTENT_CRAFTITEM_WOOD_PLANK,   CONTENT_CRAFTITEM_WOOD_PLANK,   CONTENT_CRAFTITEM_WOOD_PLANK,
		};
		crafting::setRecipe(r,CONTENT_SCAFFOLDING,1);
	}
	content_list_add("cooking",i,1,0);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CLAY_VESSEL_RAW;
	f = &content_features(i);
	f->description = gettext("Unfired Clay Vessel");
	f->setAllTextures("clay.png");
	f->draw_type = CDT_NODEBOX;
	f->param_type = CPT_LIGHT;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->air_equivalent = true;
	f->type = CMT_DIRT;
	f->dig_time = 0.1;
	f->suffocation_per_second = 0;
	f->pressure_type = CST_SOLID;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->cook_result = std::string("MaterialItem2 ")+itos(CONTENT_CLAY_VESSEL)+" 1";
	content_nodebox_clay_vessel(f);
	f->setInventoryTextureNodeBox(i,"clay.png", "clay.png", "clay.png");
	crafting::setURecipe(CONTENT_CRAFTITEM_CLAY,CONTENT_CLAY_VESSEL_RAW);
	content_list_add("craftguide",i,1,0);
	content_list_add("cooking",i,1,0);

	i = CONTENT_CLAY_VESSEL;
	f = &content_features(i);
	f->description = gettext("Clay Vessel");
	f->setAllTextures("terracotta.png");
	f->setAllMetaTextures("terracotta.png");
	f->draw_type = CDT_NODEBOX_META;
	f->param_type = CPT_LIGHT;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->air_equivalent = true;
	f->type = CMT_DIRT;
	f->dig_time = 0.1;
	f->suffocation_per_second = 0;
	f->pressure_type = CST_SOLID;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	content_nodebox_clay_vessel(f);
	f->setInventoryTextureNodeBox(i,"terracotta.png", "terracotta.png", "terracotta.png");
	if (f->initial_metadata == NULL)
		f->initial_metadata = new ClayVesselNodeMetadata();
	content_list_add("creative",i,1,0);
}
