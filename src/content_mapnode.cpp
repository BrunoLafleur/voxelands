/************************************************************************
* Minetest-c55
* Copyright (C) 2010-2011 celeron55, Perttu Ahola <celeron55@gmail.com>
*
* content_mapnode.cpp
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

#include "common.h"
#include "main.h"

#include "content_mapnode.h"
#include "mapnode.h"
#include "content_nodemeta.h"
#include "content_craftitem.h"
#include "content_toolitem.h"
#include "content_craft.h"
#include "content_list.h"
#include "content_nodebox.h"
#include "content_mapnode_util.h"
#include "mineral.h"
#ifndef SERVER
#include "tile.h"
#endif
#include "intl.h"
#include "content_burntimes.h"

MapNode mapnode_translate_from_internal(MapNode n_from, u8 version)
{
	return n_from;
}
MapNode mapnode_translate_to_internal(MapNode n_from, u8 version)
{
	MapNode result = n_from;
	switch (n_from.getContent()) {
	case CONTENT_GRASS:
		result.setContent(CONTENT_MUD);
		result.param1 = 0x01;
		result.param2 = 0;
		break;
	case CONTENT_GRASS_FOOTSTEPS:
		result.setContent(CONTENT_MUD);
		result.param1 = 0x11;
		result.param2 = 0;
		break;
	case CONTENT_GRASS_AUTUMN:
		result.setContent(CONTENT_MUD);
		result.param1 = 0x02;
		result.param2 = 0;
		break;
	case CONTENT_GRASS_FOOTSTEPS_AUTUMN:
		result.setContent(CONTENT_MUD);
		result.param1 = 0x12;
		result.param2 = 0;
		break;
	case CONTENT_GROWING_GRASS:
		result.setContent(CONTENT_MUD);
		result.param1 = 0x01;
		result.param2 = n_from.param2;
		break;
	case CONTENT_GROWING_GRASS_AUTUMN:
		result.setContent(CONTENT_MUD);
		result.param1 = 0x02;
		result.param2 = n_from.param2;
		break;
	case CONTENT_MUDSNOW:
		result.setContent(CONTENT_MUD);
		result.param1 = 0x04;
		result.param2 = 0;
		break;
	case CONTENT_FARM_WHEAT_1:
		result.setContent(CONTENT_FARM_WHEAT);
		result.param2 = 4;
		break;
	case CONTENT_FARM_WHEAT_2:
		result.setContent(CONTENT_FARM_WHEAT);
		result.param2 = 8;
		break;
	case CONTENT_FARM_WHEAT_3:
		result.setContent(CONTENT_FARM_WHEAT);
		result.param2 = 12;
		break;
	case CONTENT_FARM_POTATO_1:
		result.setContent(CONTENT_FARM_POTATO);
		result.param2 = 4;
		break;
	case CONTENT_FARM_POTATO_2:
		result.setContent(CONTENT_FARM_POTATO);
		result.param2 = 8;
		break;
	case CONTENT_FARM_POTATO_3:
		result.setContent(CONTENT_FARM_POTATO);
		result.param2 = 12;
		break;
	case CONTENT_FARM_CARROT_1:
		result.setContent(CONTENT_FARM_CARROT);
		result.param2 = 4;
		break;
	case CONTENT_FARM_CARROT_2:
		result.setContent(CONTENT_FARM_CARROT);
		result.param2 = 8;
		break;
	case CONTENT_FARM_CARROT_3:
		result.setContent(CONTENT_FARM_CARROT);
		result.param2 = 12;
		break;
	case CONTENT_FARM_BEETROOT_1:
		result.setContent(CONTENT_FARM_BEETROOT);
		result.param2 = 4;
		break;
	case CONTENT_FARM_BEETROOT_2:
		result.setContent(CONTENT_FARM_BEETROOT);
		result.param2 = 8;
		break;
	case CONTENT_FARM_BEETROOT_3:
		result.setContent(CONTENT_FARM_BEETROOT);
		result.param2 = 12;
		break;
	case CONTENT_FARM_GRAPEVINE_1:
		result.setContent(CONTENT_FARM_GRAPEVINE);
		result.param2 = 4;
		break;
	case CONTENT_FARM_GRAPEVINE_2:
		result.setContent(CONTENT_FARM_GRAPEVINE);
		result.param2 = 8;
		break;
	case CONTENT_FARM_GRAPEVINE_3:
		result.setContent(CONTENT_FARM_GRAPEVINE);
		result.param2 = 12;
		break;
	case CONTENT_FARM_COTTON_1:
		result.setContent(CONTENT_FARM_COTTON);
		result.param2 = 4;
		break;
	case CONTENT_FARM_COTTON_2:
		result.setContent(CONTENT_FARM_COTTON);
		result.param2 = 8;
		break;
	case CONTENT_FARM_COTTON_3:
		result.setContent(CONTENT_FARM_COTTON);
		result.param2 = 12;
		break;
	case CONTENT_FARM_PUMPKIN_1:
		result.setContent(CONTENT_FARM_PUMPKIN);
		result.param2 = 4;
		break;
	case CONTENT_FARM_PUMPKIN_2:
		result.setContent(CONTENT_FARM_PUMPKIN);
		result.param2 = 8;
		break;
	case CONTENT_FARM_PUMPKIN_3:
		result.setContent(CONTENT_FARM_PUMPKIN);
		result.param2 = 12;
		break;
	case CONTENT_FARM_MELON_1:
		result.setContent(CONTENT_FARM_MELON);
		result.param2 = 4;
		break;
	case CONTENT_FARM_MELON_2:
		result.setContent(CONTENT_FARM_MELON);
		result.param2 = 8;
		break;
	case CONTENT_FARM_MELON_3:
		result.setContent(CONTENT_FARM_MELON);
		result.param2 = 12;
		break;
	case CONTENT_FARM_TRELLIS_GRAPE_1:
		result.setContent(CONTENT_FARM_TRELLIS_GRAPE);
		result.param2 = 4;
		break;
	case CONTENT_FARM_TRELLIS_GRAPE_2:
		result.setContent(CONTENT_FARM_TRELLIS_GRAPE);
		result.param2 = 8;
		break;
	case CONTENT_FARM_TRELLIS_GRAPE_3:
		result.setContent(CONTENT_FARM_TRELLIS_GRAPE);
		result.param2 = 12;
		break;
	default:;
	}
	return result;
}

void content_mapnode_init(bool repeat)
{
	// Read some settings
#ifndef SERVER
	bool opaque_water = config_get_bool("client.graphics.water.opaque");
#endif

	content_t i;
	ContentFeatures *f = NULL;

	i = CONTENT_STONE;
	f = &content_features(i);
	f->description = gettext("Stone");
	f->setAllTextures("stone.png");
	f->setInventoryTextureCube("stone.png", "stone.png", "stone.png");
	f->param_type = CPT_MINERAL;
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->often_contains_mineral = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_ROUGHSTONE)+" 1";
	f->crush_result = std::string("MaterialItem2 ")+itos(CONTENT_COBBLE)+" 1";
	f->crush_type = CRUSH_CRUSHER;
	f->type = CMT_STONE;
	f->dig_time = 1.0;
	content_list_add("creative",i,1,0);
	content_list_add("decrafting",i,1,0);

	i = CONTENT_LIMESTONE;
	f = &content_features(i);
	f->description = gettext("Limestone");
	f->setAllTextures("limestone.png");
	f->setInventoryTextureCube("limestone.png", "limestone.png", "limestone.png");
	f->param_type = CPT_MINERAL;
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->often_contains_mineral = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->cook_result = std::string("MaterialItem2 ")+itos(CONTENT_MARBLE)+" 1";
	f->cook_type = COOK_FURNACE|COOK_SMELTERY;
	f->type = CMT_STONE;
	f->dig_time = 0.6;
	f->crush_result = std::string("MaterialItem2 ")+itos(CONTENT_COBBLE)+" 1";
	f->crush_type = CRUSH_CRUSHER;
	content_list_add("creative",i,1,0);
	content_list_add("cooking",i,1,0);
	content_list_add("decrafting",i,1,0);

	i = CONTENT_SPACEROCK;
	f = &content_features(i);
	f->description = gettext("Space Rock");
	f->setAllTextures("spacerock.png");
	f->setInventoryTextureCube("spacerock.png", "spacerock.png", "spacerock.png");
	f->param_type = CPT_MINERAL;
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->often_contains_mineral = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->type = CMT_STONE;
	f->dig_time = 1.1;

	i = CONTENT_MARBLE;
	f = &content_features(i);
	f->description = gettext("Marble");
	f->setAllTextures("marble.png");
	f->setInventoryTextureCube("marble.png", "marble.png", "marble.png");
	f->draw_type = CDT_CUBELIKE;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->type = CMT_STONE;
	f->dig_time = 1.0;
	content_list_add("creative",i,1,0);

	i = CONTENT_ROCK;
	f = &content_features(i);
	f->description = gettext("Rock");
	f->setAllTextures("stone.png");
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->type = CMT_DIRT;
	f->dig_time = 0.01;
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->air_equivalent = true;
	f->buildable_to = true;
	f->suffocation_per_second = 0;
	content_nodebox_rock(f);
	f->setInventoryTextureNodeBox(i,"stone.png", "stone.png", "stone.png");
	crafting::set1To2Recipe(CONTENT_STONE_KNOB,CONTENT_ROCK);
	crafting::set1To2Recipe(CONTENT_ROUGHSTONE_KNOB,CONTENT_ROCK);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_ICE;
	f = &content_features(i);
	f->description = gettext("Ice");
	f->setAllTextures("ice.png");
	f->setInventoryTextureCube("ice.png", "ice.png", "ice.png");
	f->draw_type = CDT_CUBELIKE;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->param_type = CPT_LIGHT;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->type = CMT_STONE;
	f->dig_time = 1.0;
	f->warmth_per_second = 10;
	content_list_add("creative",i,1,0);

	i = CONTENT_COAL;
	f = &content_features(i);
	f->description = gettext("Coal Block");
	f->setAllTextures("coal.png");
	f->setInventoryTextureCube("coal.png", "coal.png", "coal.png");
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->flammable = 2; // can be set on fire
	f->fuel_time = BT_COAL_BLOCK;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->type = CMT_STONE;
	f->dig_time = 1.0;
	crafting::setHardBlockRecipe(CONTENT_CRAFTITEM_COAL,CONTENT_COAL);
	crafting::setUncraftHardBlockRecipe(CONTENT_COAL,CONTENT_CRAFTITEM_COAL);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CHARCOAL;
	f = &content_features(i);
	f->description = gettext("Charcoal Block");
	f->setAllTextures("charcoal.png");
	f->setInventoryTextureCube("charcoal.png", "charcoal.png", "charcoal.png");
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->flammable = 2; // can be set on fire
	f->fuel_time = BT_COAL_BLOCK;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->type = CMT_STONE;
	f->dig_time = 1.0;
	crafting::setHardBlockRecipe(CONTENT_CRAFTITEM_CHARCOAL,CONTENT_CHARCOAL);
	crafting::setUncraftHardBlockRecipe(CONTENT_CHARCOAL,CONTENT_CRAFTITEM_CHARCOAL);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_STONEBRICK;
	f = &content_features(i);
	f->description = gettext("Stone Brick");
	f->setAllTextures("stone.png^overlay_brick.png");
	f->setInventoryTextureCube("stone.png^overlay_brick.png", "stone.png^overlay_brick.png", "stone.png^overlay_brick.png");
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->type = CMT_STONE;
	f->dig_time = 1.0;
	crafting::setBrickRecipe(CONTENT_STONE,CONTENT_STONEBRICK);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_STONEBLOCK;
	f = &content_features(i);
	f->description = gettext("Stone Block");
	f->setAllTextures("stone.png^overlay_block.png");
	f->setInventoryTextureCube("stone.png^overlay_block.png", "stone.png^overlay_block.png", "stone.png^overlay_block.png");
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->type = CMT_STONE;
	f->dig_time = 1.0;
	crafting::setBlockRecipe(CONTENT_STONE,CONTENT_STONEBLOCK);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_ROUGHSTONEBRICK;
	f = &content_features(i);
	f->description = gettext("Rough Stone Brick");
	f->setAllTextures("roughstone.png^overlay_brick.png");
	f->setInventoryTextureCube("roughstone.png^overlay_brick.png", "roughstone.png^overlay_brick.png", "roughstone.png^overlay_brick.png");
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->cook_result = std::string("MaterialItem2 ")+itos(CONTENT_STONEBRICK)+" 1";
	f->cook_type = COOK_FURNACE|COOK_SMELTERY;
	f->type = CMT_STONE;
	f->dig_time = 1.0;
	crafting::setBrickRecipe(CONTENT_ROUGHSTONE,CONTENT_ROUGHSTONEBRICK);
	content_list_add("craftguide",i,1,0);
	content_list_add("player-creative",i,1,0);
	content_list_add("cooking",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_ROUGHSTONEBLOCK;
	f = &content_features(i);
	f->description = gettext("Rough Stone Block");
	f->setAllTextures("roughstone.png^overlay_block.png");
	f->setInventoryTextureCube("roughstone.png^overlay_block.png", "roughstone.png^overlay_block.png", "roughstone.png^overlay_block.png");
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->cook_result = std::string("MaterialItem2 ")+itos(CONTENT_STONEBLOCK)+" 1";
	f->cook_type = COOK_FURNACE|COOK_SMELTERY;
	f->type = CMT_STONE;
	f->dig_time = 1.0;
	crafting::setBlockRecipe(CONTENT_ROUGHSTONE,CONTENT_ROUGHSTONEBLOCK);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);
	content_list_add("cooking",i,1,0);

	i = CONTENT_GRASS;
	f = &content_features(i);
	f->description = gettext("Grass");
	f->setAllTextures("mud.png^grass_side.png");
	f->setTexture(0, "grass.png");
	f->setTexture(1, "mud.png");
	f->setInventoryTextureCube("grass.png","mud.png^grass_side.png","mud.png^grass_side.png");
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_MUD)+" 1";
	f->extra_dug_item = std::string("MaterialItem2 ")+itos(CONTENT_WILDGRASS_SHORT)+" 1";
	f->extra_dug_item_rarity = 10;
	f->sound_step = "grass-step";
	f->type = CMT_DIRT;
	f->dig_time = 1.0;
	f->farm_ploughable = true;
	content_list_add("decrafting",i,1,0);

	i = CONTENT_GROWING_GRASS;
	f = &content_features(i);
	f->description = gettext("Growing Grass");
	f->setAllTextures("mud.png");
	f->setTexture(0, "grass_growing.png");
	f->setInventoryTextureCube("grass.png","mud.png","mud.png");
	f->draw_type = CDT_CUBELIKE;
	f->param2_type = CPT_PLANTGROWTH;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_MUD)+" 1";
	f->special_alternate_node = CONTENT_GRASS;
	f->type = CMT_DIRT;
	f->dig_time = 1.0;
	f->farm_ploughable = true;

	i = CONTENT_GRASS_FOOTSTEPS;
	f = &content_features(i);
	f->description = gettext("Grass");
	f->setAllTextures("mud.png^grass_side.png");
	f->setTexture(0, "grass_footsteps.png");
	f->setTexture(1, "mud.png");
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_MUD)+" 1";
	f->sound_step = "grass-step";
	f->type = CMT_DIRT;
	f->dig_time = 1.0;
	f->farm_ploughable = true;

	i = CONTENT_GRASS_AUTUMN;
	f = &content_features(i);
	f->description = gettext("Grass");
	f->setAllTextures("mud.png^grass_side_autumn.png");
	f->setTexture(0, "grass_autumn.png");
	f->setTexture(1, "mud.png");
	f->setInventoryTextureCube("grass_autumn.png","mud.png^grass_side_autumn.png","mud.png^grass_side_autumn.png");
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_MUD)+" 1";
	f->extra_dug_item = std::string("MaterialItem2 ")+itos(CONTENT_DEADGRASS)+" 1";
	f->extra_dug_item_rarity = 10;
	f->sound_step = "grass-step";
	f->type = CMT_DIRT;
	f->dig_time = 1.0;
	f->farm_ploughable = true;
	content_list_add("decrafting",i,1,0);

	i = CONTENT_GROWING_GRASS_AUTUMN;
	f = &content_features(i);
	f->description = gettext("Growing Grass");
	f->setAllTextures("mud.png");
	f->setTexture(0, "grass_growing_autumn.png");
	f->setInventoryTextureCube("grass_autumn.png","mud.png","mud.png");
	f->draw_type = CDT_CUBELIKE;
	f->param2_type = CPT_PLANTGROWTH;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_MUD)+" 1";
	f->special_alternate_node = CONTENT_GRASS_AUTUMN;
	f->type = CMT_DIRT;
	f->dig_time = 1.0;
	f->farm_ploughable = true;

	i = CONTENT_GRASS_FOOTSTEPS_AUTUMN;
	f = &content_features(i);
	f->description = gettext("Grass");
	f->setAllTextures("mud.png^grass_side_autumn.png");
	f->setTexture(0, "grass_footsteps_autumn.png");
	f->setTexture(1, "mud.png");
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_MUD)+" 1";
	f->sound_step = "grass-step";
	f->type = CMT_DIRT;
	f->dig_time = 1.0;
	f->farm_ploughable = true;

	i = CONTENT_MUDSNOW;
	f = &content_features(i);
	f->description = gettext("Muddy Snow");
	f->setAllTextures("mud.png^snow_side.png");
	f->setTexture(0, "snow.png");
	f->setTexture(1, "mud.png");
	f->setInventoryTextureCube("snow.png","mud.png^snow_side.png","mud.png^snow_side.png");
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_MUD)+" 1";
	f->extra_dug_item = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_SNOW_BALL)+" 1";
	f->extra_dug_item_rarity = 5;
	f->type = CMT_DIRT;
	f->dig_time = 1.0;
	content_list_add("decrafting",i,1,0);

	i = CONTENT_MUD;
	f = &content_features(i);
	f->description = gettext("Mud");
	f->setAllTextures("mud.png");
	f->setTexture(0, "grass.png");
	f->setInventoryTextureCube("mud.png", "mud.png", "mud.png");
	f->draw_type = CDT_DIRTLIKE;
	f->param_type = CPT_BLOCKDATA;
	f->param2_type = CPT_PLANTGROWTH;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->extra_dug_item = std::string("MaterialItem2 ")+itos(CONTENT_ROCK)+" 1";
	f->extra_dug_item_rarity = 3;
	f->extra_dug_item_max_level = 1;
	f->type = CMT_DIRT;
	f->dig_time = 1.0;
	f->farm_ploughable = true;
	content_list_add("creative",i,1,0);

	i = CONTENT_ASH;
	f = &content_features(i);
	f->description = gettext("Ash Block");
	f->setAllTextures("ash.png");
	f->setInventoryTextureCube("ash.png", "ash.png", "ash.png");
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_ASH)+" 9";
	f->type = CMT_DIRT;
	f->dig_time = 0.5;
	crafting::setHardBlockRecipe(CONTENT_CRAFTITEM_ASH,CONTENT_ASH);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);
	content_list_add("decrafting",i,1,0);

	i = CONTENT_SAND;
	f = &content_features(i);
	f->description = gettext("Sand");
	f->setAllTextures("sand.png");
	f->setInventoryTextureCube("sand.png", "sand.png", "sand.png");
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->pressure_type = CST_DROPABLE;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->extra_dug_item = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_SALT)+" 1";
	f->extra_dug_item_rarity = 30;
	f->extra_dug_item_min_level = 3;
	f->cook_result = std::string("MaterialItem2 ")+itos(CONTENT_GLASS)+" 1";
	f->cook_type = COOK_FURNACE|COOK_SMELTERY;
	f->type = CMT_DIRT;
	f->dig_time = 1.0;
	content_list_add("creative",i,1,0);
	content_list_add("cooking",i,1,0);
	content_list_add("decrafting",i,1,0);

	i = CONTENT_DESERT_SAND;
	f = &content_features(i);
	f->description = gettext("Desert Sand");
	f->setAllTextures("sand_desert.png");
	f->setInventoryTextureCube("sand_desert.png", "sand_desert.png", "sand_desert.png");
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->pressure_type = CST_DROPABLE;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->extra_dug_item = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_SALTPETER)+" 1";
	f->extra_dug_item_rarity = 30;
	f->extra_dug_item_min_level = 3;
	f->cook_result = std::string("MaterialItem2 ")+itos(CONTENT_GLASS)+" 1";
	f->cook_type = COOK_FURNACE|COOK_SMELTERY;
	f->type = CMT_DIRT;
	f->dig_time = 1.0;
	content_list_add("creative",i,1,0);
	content_list_add("cooking",i,1,0);
	content_list_add("decrafting",i,1,0);

	i = CONTENT_GRAVEL;
	f = &content_features(i);
	f->description = gettext("Gravel");
	f->setAllTextures("gravel.png");
	f->setInventoryTextureCube("gravel.png", "gravel.png", "gravel.png");
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->pressure_type = CST_DROPABLE;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->extra_dug_item = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_FLINT)+" 1";
	f->extra_dug_item_rarity = 10;
	f->extra_dug_item_min_level = 1;
	f->type = CMT_DIRT;
	f->dig_time = 1.75;
	f->crush_result = std::string("MaterialItem2 ")+itos(CONTENT_SAND)+" 1";
	f->crush_type = CRUSH_CRUSHER;
	content_list_add("creative",i,1,0);
	content_list_add("decrafting",i,1,0);

	i = CONTENT_SANDSTONE;
	f = &content_features(i);
	f->description = gettext("Sandstone");
	f->setAllTextures("sandstone.png");
	f->setInventoryTextureCube("sandstone.png", "sandstone.png", "sandstone.png");
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->type = CMT_STONE;
	f->dig_time = 0.5;
	crafting::setSoftBlockRecipe(CONTENT_SAND,CONTENT_SANDSTONE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_SANDSTONE_BRICK;
	f = &content_features(i);
	f->description = gettext("Sandstone Bricks");
	f->setAllTextures("sandstone.png^overlay_brick.png");
	f->setInventoryTextureCube("sandstone.png^overlay_brick.png", "sandstone.png^overlay_brick.png", "sandstone.png^overlay_brick.png");
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->type = CMT_STONE;
	f->dig_time = 0.5;
	crafting::setBrickRecipe(CONTENT_SANDSTONE,CONTENT_SANDSTONE_BRICK);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_SANDSTONE_BLOCK;
	f = &content_features(i);
	f->description = gettext("Sandstone Blocks");
	f->setAllTextures("sandstone.png^overlay_block.png");
	f->setInventoryTextureCube("sandstone.png^overlay_block.png", "sandstone.png^overlay_block.png", "sandstone.png^overlay_block.png");
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->type = CMT_STONE;
	f->dig_time = 0.5;
	crafting::setBlockRecipe(CONTENT_SANDSTONE,CONTENT_SANDSTONE_BLOCK);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_DESERT_SANDSTONE;
	f = &content_features(i);
	f->description = gettext("Desert Sandstone");
	f->setAllTextures("sandstone_desert.png");
	f->setInventoryTextureCube("sandstone_desert.png", "sandstone_desert.png", "sandstone_desert.png");
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->type = CMT_STONE;
	f->dig_time = 0.5;
	crafting::setSoftBlockRecipe(CONTENT_DESERT_SAND,CONTENT_DESERT_SANDSTONE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_DESERT_SANDSTONE_BRICK;
	f = &content_features(i);
	f->description = gettext("Desert Sandstone Bricks");
	f->setAllTextures("sandstone_desert.png^overlay_brick.png");
	f->setInventoryTextureCube("sandstone_desert.png^overlay_brick.png", "sandstone_desert.png^overlay_brick.png", "sandstone_desert.png^overlay_brick.png");
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->type = CMT_STONE;
	f->dig_time = 0.5;
	crafting::setBrickRecipe(CONTENT_DESERT_SANDSTONE,CONTENT_DESERT_SANDSTONE_BRICK);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_DESERT_SANDSTONE_BLOCK;
	f = &content_features(i);
	f->description = gettext("Desert Sandstone Blocks");
	f->setAllTextures("sandstone_desert.png^overlay_block.png");
	f->setInventoryTextureCube("sandstone_desert.png^overlay_block.png", "sandstone_desert.png^overlay_block.png", "sandstone_desert.png^overlay_block.png");
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->type = CMT_STONE;
	f->dig_time = 0.5;
	crafting::setBlockRecipe(CONTENT_DESERT_SANDSTONE,CONTENT_DESERT_SANDSTONE_BLOCK);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CLAY;
	f = &content_features(i);
	f->description = gettext("Clay");
	f->setAllTextures("clay.png");
	f->setTexture(0,"grass.png");
	f->setInventoryTextureCube("clay.png", "clay.png", "clay.png");
	f->draw_type = CDT_DIRTLIKE;
	f->param_type = CPT_BLOCKDATA;
	f->param2_type = CPT_PLANTGROWTH;
	f->is_ground_content = true;
	f->dug_item = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_CLAY)+" 4";
	f->cook_result = std::string("MaterialItem2 ")+itos(CONTENT_TERRACOTTA)+" 1";
	f->type = CMT_DIRT;
	f->dig_time = 1.0;
	crafting::setSoftBlockRecipe(CONTENT_CRAFTITEM_CLAY,CONTENT_CLAY);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);
	content_list_add("cooking",i,1,0);
	content_list_add("decrafting",i,1,0);

	i = CONTENT_CLAY_BLUE;
	f = &content_features(i);
	f->description = gettext("Blue Clay");
	f->setAllTextures("clay_blue.png");
	f->setInventoryTextureCube("clay_blue.png", "clay_blue.png", "clay_blue.png");
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_CLAY)+" 4";
	f->extra_dug_item = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_DYE_BLUE)+" 1";
	f->extra_dug_item_rarity = 1;
	f->type = CMT_DIRT;
	f->dig_time = 1.0;
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_DYE_BLUE,CONTENT_CLAY,CONTENT_CLAY_BLUE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);
	content_list_add("decrafting",i,1,0);

	i = CONTENT_CLAY_GREEN;
	f = &content_features(i);
	f->description = gettext("Green Clay");
	f->setAllTextures("clay_green.png");
	f->setInventoryTextureCube("clay_green.png", "clay_green.png", "clay_green.png");
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_CLAY)+" 4";
	f->extra_dug_item = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_DYE_GREEN)+" 1";
	f->extra_dug_item_rarity = 1;
	f->type = CMT_DIRT;
	f->dig_time = 1.0;
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_DYE_GREEN,CONTENT_CLAY,CONTENT_CLAY_GREEN);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);
	content_list_add("decrafting",i,1,0);

	i = CONTENT_CLAY_ORANGE;
	f = &content_features(i);
	f->description = gettext("Orange Clay");
	f->setAllTextures("clay_orange.png");
	f->setInventoryTextureCube("clay_orange.png", "clay_orange.png", "clay_orange.png");
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_CLAY)+" 4";
	f->extra_dug_item = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_DYE_ORANGE)+" 1";
	f->extra_dug_item_rarity = 1;
	f->type = CMT_DIRT;
	f->dig_time = 1.0;
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_DYE_ORANGE,CONTENT_CLAY,CONTENT_CLAY_ORANGE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);
	content_list_add("decrafting",i,1,0);

	i = CONTENT_CLAY_PURPLE;
	f = &content_features(i);
	f->description = gettext("Purple Clay");
	f->setAllTextures("clay_purple.png");
	f->setInventoryTextureCube("clay_purple.png", "clay_purple.png", "clay_purple.png");
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_CLAY)+" 4";
	f->extra_dug_item = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_DYE_PURPLE)+" 1";
	f->extra_dug_item_rarity = 1;
	f->type = CMT_DIRT;
	f->dig_time = 1.0;
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_DYE_PURPLE,CONTENT_CLAY,CONTENT_CLAY_PURPLE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);
	content_list_add("decrafting",i,1,0);

	i = CONTENT_CLAY_RED;
	f = &content_features(i);
	f->description = gettext("Red Clay");
	f->setAllTextures("clay_red.png");
	f->setInventoryTextureCube("clay_red.png", "clay_red.png", "clay_red.png");
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->is_ground_content = true;
	f->dug_item = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_CLAY)+" 4";
	f->extra_dug_item = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_DYE_RED)+" 1";
	f->extra_dug_item_rarity = 1;
	f->type = CMT_DIRT;
	f->dig_time = 1.0;
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_DYE_RED,CONTENT_CLAY,CONTENT_CLAY_RED);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);
	content_list_add("decrafting",i,1,0);

	i = CONTENT_CLAY_YELLOW;
	f = &content_features(i);
	f->description = gettext("Yellow Clay");
	f->setAllTextures("clay_yellow.png");
	f->setInventoryTextureCube("clay_yellow.png", "clay_yellow.png", "clay_yellow.png");
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_CLAY)+" 4";
	f->extra_dug_item = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_DYE_YELLOW)+" 1";
	f->extra_dug_item_rarity = 1;
	f->type = CMT_DIRT;
	f->dig_time = 1.0;
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_DYE_YELLOW,CONTENT_CLAY,CONTENT_CLAY_YELLOW);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);
	content_list_add("decrafting",i,1,0);

	i = CONTENT_CLAY_BLACK;
	f = &content_features(i);
	f->description = gettext("Black Clay");
	f->setAllTextures("clay_black.png");
	f->setInventoryTextureCube("clay_black.png", "clay_black.png", "clay_black.png");
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_CLAY)+" 4";
	f->extra_dug_item = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_DYE_BLACK)+" 1";
	f->extra_dug_item_rarity = 1;
	f->type = CMT_DIRT;
	f->dig_time = 1.0;
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_DYE_BLACK,CONTENT_CLAY,CONTENT_CLAY_BLACK);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);
	content_list_add("decrafting",i,1,0);

	i = CONTENT_BRICK;
	f = &content_features(i);
	f->description = gettext("Brick");
	f->setAllTextures("brick.png");
	f->setTexture(0,"brick_top.png");
	f->setTexture(1,"brick_bottom.png");
	f->setTexture(2,"brick_side.png");
	f->setTexture(3,"brick_side.png");
	f->setInventoryTextureCube("brick_top.png", "brick.png", "brick_side.png");
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_CLAY_BRICK)+" 4";
	f->type = CMT_STONE;
	f->dig_time = 1.0;
	crafting::setSoftBlockRecipe(CONTENT_CRAFTITEM_CLAY_BRICK,CONTENT_BRICK);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);
	content_list_add("decrafting",i,1,0);

	i = CONTENT_TERRACOTTA;
	f = &content_features(i);
	f->description = gettext("Terracotta");
	f->setAllTextures("terracotta.png");
	f->setInventoryTextureCube("terracotta.png", "terracotta.png", "terracotta.png");
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->type = CMT_STONE;
	f->dig_time = 1.0;
	content_list_add("creative",i,1,0);

	i = CONTENT_TERRACOTTA_BRICK;
	f = &content_features(i);
	f->description = gettext("Terracotta Brick");
	f->setAllTextures("terracotta.png^overlay_brick.png");
	f->setInventoryTextureCube("terracotta.png^overlay_brick.png", "terracotta.png^overlay_brick.png", "terracotta.png^overlay_brick.png");
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->type = CMT_STONE;
	f->dig_time = 1.0;
	crafting::setBrickRecipe(CONTENT_TERRACOTTA,CONTENT_TERRACOTTA_BRICK);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_TERRACOTTA_BLOCK;
	f = &content_features(i);
	f->description = gettext("Terracotta Block");
	f->setAllTextures("terracotta.png^overlay_block.png");
	f->setInventoryTextureCube("terracotta.png^overlay_block.png", "terracotta.png^overlay_block.png", "terracotta.png^overlay_block.png");
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->type = CMT_STONE;
	f->dig_time = 1.0;
	crafting::setBlockRecipe(CONTENT_TERRACOTTA,CONTENT_TERRACOTTA_BLOCK);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_TERRACOTTA_TILE;
	f = &content_features(i);
	f->description = gettext("Terracotta Tile");
	f->setAllTextures("terracotta.png^overlay_tile.png");
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->type = CMT_STONE;
	f->dig_time = 1.0;
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->suffocation_per_second = 0;
	content_nodebox_carpet(f);
	f->setInventoryTextureNodeBox(i,"terracotta.png^overlay_tile.png", "terracotta.png^overlay_tile.png", "terracotta.png^overlay_tile.png");
	crafting::setTileRecipe(CONTENT_TERRACOTTA,CONTENT_TERRACOTTA_TILE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_GLASS;
	f = &content_features(i);
	f->description = gettext("Glass");
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_GLASSLIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->setAllTextures("glass.png");
#ifndef SERVER
	f->setAllTextureTypes(MATERIAL_ALPHA_BLEND);
#endif
	f->setInventoryTextureCube("glass.png", "glass.png", "glass.png");
	f->type = CMT_GLASS;
	f->dig_time = 0.15;
	content_list_add("creative",i,1,0);

	i = CONTENT_GLASS_BLUE;
	f = &content_features(i);
	f->description = gettext("Blue Glass");
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_GLASSLIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->setAllTextures("glass.png^glass_pane_blue_side.png");
#ifndef SERVER
	f->setAllTextureTypes(MATERIAL_ALPHA_BLEND);
#endif
	f->setInventoryTextureCube("glass.png^glass_pane_blue_side.png", "glass.png^glass_pane_blue_side.png", "glass.png^glass_pane_blue_side.png");
	f->type = CMT_GLASS;
	f->dig_time = 0.15;
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_DYE_BLUE,CONTENT_GLASS,CONTENT_GLASS_BLUE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_GLASS_GREEN;
	f = &content_features(i);
	f->description = gettext("Green Glass");
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_GLASSLIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->setAllTextures("glass.png^glass_pane_green_side.png");
#ifndef SERVER
	f->setAllTextureTypes(MATERIAL_ALPHA_BLEND);
#endif
	f->setInventoryTextureCube("glass.png^glass_pane_green_side.png", "glass.png^glass_pane_green_side.png", "glass.png^glass_pane_green_side.png");
	f->type = CMT_GLASS;
	f->dig_time = 0.15;
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_DYE_GREEN,CONTENT_GLASS,CONTENT_GLASS_GREEN);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_GLASS_ORANGE;
	f = &content_features(i);
	f->description = gettext("Orange Glass");
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_GLASSLIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->setAllTextures("glass.png^glass_pane_orange_side.png");
#ifndef SERVER
	f->setAllTextureTypes(MATERIAL_ALPHA_BLEND);
#endif
	f->setInventoryTextureCube("glass.png^glass_pane_orange_side.png", "glass.png^glass_pane_orange_side.png", "glass.png^glass_pane_orange_side.png");
	f->type = CMT_GLASS;
	f->dig_time = 0.15;
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_DYE_ORANGE,CONTENT_GLASS,CONTENT_GLASS_ORANGE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_GLASS_PURPLE;
	f = &content_features(i);
	f->description = gettext("Purple Glass");
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_GLASSLIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->setAllTextures("glass.png^glass_pane_purple_side.png");
#ifndef SERVER
	f->setAllTextureTypes(MATERIAL_ALPHA_BLEND);
#endif
	f->setInventoryTextureCube("glass.png^glass_pane_purple_side.png", "glass.png^glass_pane_purple_side.png", "glass.png^glass_pane_purple_side.png");
	f->type = CMT_GLASS;
	f->dig_time = 0.15;
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_DYE_PURPLE,CONTENT_GLASS,CONTENT_GLASS_PURPLE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_GLASS_RED;
	f = &content_features(i);
	f->description = gettext("Red Glass");
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_GLASSLIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->setAllTextures("glass.png^glass_pane_red_side.png");
#ifndef SERVER
	f->setAllTextureTypes(MATERIAL_ALPHA_BLEND);
#endif
	f->setInventoryTextureCube("glass.png^glass_pane_red_side.png", "glass.png^glass_pane_red_side.png", "glass.png^glass_pane_red_side.png");
	f->type = CMT_GLASS;
	f->dig_time = 0.15;
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_DYE_RED,CONTENT_GLASS,CONTENT_GLASS_RED);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_GLASS_YELLOW;
	f = &content_features(i);
	f->description = gettext("Yellow Glass");
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_GLASSLIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->setAllTextures("glass.png^glass_pane_yellow_side.png");
#ifndef SERVER
	f->setAllTextureTypes(MATERIAL_ALPHA_BLEND);
#endif
	f->setInventoryTextureCube("glass.png^glass_pane_yellow_side.png", "glass.png^glass_pane_yellow_side.png", "glass.png^glass_pane_yellow_side.png");
	f->type = CMT_GLASS;
	f->dig_time = 0.15;
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_DYE_YELLOW,CONTENT_GLASS,CONTENT_GLASS_YELLOW);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_GLASS_BLACK;
	f = &content_features(i);
	f->description = gettext("Black Glass");
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_GLASSLIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->setAllTextures("glass.png^glass_pane_black_side.png");
#ifndef SERVER
	f->setAllTextureTypes(MATERIAL_ALPHA_BLEND);
#endif
	f->setInventoryTextureCube("glass.png^glass_pane_black_side.png", "glass.png^glass_pane_black_side.png", "glass.png^glass_pane_black_side.png");
	f->type = CMT_GLASS;
	f->dig_time = 0.15;
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_DYE_BLACK,CONTENT_GLASS,CONTENT_GLASS_BLACK);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_GLASS_PANE;
	f = &content_features(i);
	f->description = gettext("Glass Pane");
	f->param2_type = CPT_FACEDIR_SIMPLE;
	f->param_type = CPT_LIGHT;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->setAllTextures("glass_pane_side.png");
	f->setTexture(4,"glass.png");
	f->setTexture(5,"glass.png");
#ifndef SERVER
	f->setAllTextureTypes(MATERIAL_ALPHA_BLEND);
#endif
	content_nodebox_glass_pane(f);
	f->setInventoryTextureNodeBox(i,"glass_pane_side.png", "glass.png", "glass_pane_side.png");
	f->type = CMT_GLASS;
	f->dig_time = 0.15;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	crafting::setCol3Recipe(CONTENT_GLASS,CONTENT_GLASS_PANE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_GLASS_PANE_BLUE;
	f = &content_features(i);
	f->description = gettext("Blue Glass Pane");
	f->param2_type = CPT_FACEDIR_SIMPLE;
	f->param_type = CPT_LIGHT;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->setAllTextures("glass_pane_blue_side.png");
	f->setTexture(4,"glass.png^glass_pane_blue_side.png");
	f->setTexture(5,"glass.png^glass_pane_blue_side.png");
#ifndef SERVER
	f->setAllTextureTypes(MATERIAL_ALPHA_BLEND);
#endif
		content_nodebox_glass_pane(f);
	f->setInventoryTextureNodeBox(i,"glass_pane_blue_side.png", "glass.png^glass_pane_blue_side.png", "glass_pane_blue_side.png");
	f->type = CMT_GLASS;
	f->dig_time = 0.15;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	crafting::setCol3Recipe(CONTENT_GLASS_BLUE,CONTENT_GLASS_PANE_BLUE);
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_DYE_BLUE,CONTENT_GLASS_PANE,CONTENT_GLASS_PANE_BLUE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_GLASS_PANE_GREEN;
	f = &content_features(i);
	f->description = gettext("Green Glass Pane");
	f->param2_type = CPT_FACEDIR_SIMPLE;
	f->param_type = CPT_LIGHT;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->setAllTextures("glass_pane_green_side.png");
	f->setTexture(4,"glass.png^glass_pane_green_side.png");
	f->setTexture(5,"glass.png^glass_pane_green_side.png");
#ifndef SERVER
	f->setAllTextureTypes(MATERIAL_ALPHA_BLEND);
#endif
	content_nodebox_glass_pane(f);
	f->setInventoryTextureNodeBox(i,"glass_pane_green_side.png", "glass.png^glass_pane_green_side.png", "glass_pane_green_side.png");
	f->type = CMT_GLASS;
	f->dig_time = 0.15;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	crafting::setCol3Recipe(CONTENT_GLASS_GREEN,CONTENT_GLASS_PANE_GREEN);
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_DYE_GREEN,CONTENT_GLASS_PANE,CONTENT_GLASS_PANE_GREEN);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_GLASS_PANE_ORANGE;
	f = &content_features(i);
	f->description = gettext("Orange Glass Pane");
	f->param2_type = CPT_FACEDIR_SIMPLE;
	f->param_type = CPT_LIGHT;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->setAllTextures("glass_pane_orange_side.png");
	f->setTexture(4,"glass.png^glass_pane_orange_side.png");
	f->setTexture(5,"glass.png^glass_pane_orange_side.png");
#ifndef SERVER
	f->setAllTextureTypes(MATERIAL_ALPHA_BLEND);
#endif
	content_nodebox_glass_pane(f);
	f->setInventoryTextureNodeBox(i,"glass_pane_orange_side.png", "glass.png^glass_pane_orange_side.png", "glass_pane_orange_side.png");
	f->type = CMT_GLASS;
	f->dig_time = 0.15;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	crafting::setCol3Recipe(CONTENT_GLASS_ORANGE,CONTENT_GLASS_PANE_ORANGE);
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_DYE_ORANGE,CONTENT_GLASS_PANE,CONTENT_GLASS_PANE_ORANGE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_GLASS_PANE_PURPLE;
	f = &content_features(i);
	f->description = gettext("Purple Glass Pane");
	f->param2_type = CPT_FACEDIR_SIMPLE;
	f->param_type = CPT_LIGHT;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->setAllTextures("glass_pane_purple_side.png");
	f->setTexture(4,"glass.png^glass_pane_purple_side.png");
	f->setTexture(5,"glass.png^glass_pane_purple_side.png");
#ifndef SERVER
	f->setAllTextureTypes(MATERIAL_ALPHA_BLEND);
#endif
	content_nodebox_glass_pane(f);
	f->setInventoryTextureNodeBox(i,"glass_pane_purple_side.png", "glass.png^glass_pane_purple_side.png", "glass_pane_purple_side.png");
	f->type = CMT_GLASS;
	f->dig_time = 0.15;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	crafting::setCol3Recipe(CONTENT_GLASS_PURPLE,CONTENT_GLASS_PANE_PURPLE);
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_DYE_PURPLE,CONTENT_GLASS_PANE,CONTENT_GLASS_PANE_PURPLE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_GLASS_PANE_RED;
	f = &content_features(i);
	f->description = gettext("Red Glass Pane");
	f->param2_type = CPT_FACEDIR_SIMPLE;
	f->param_type = CPT_LIGHT;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->setAllTextures("glass_pane_red_side.png");
	f->setTexture(4,"glass.png^glass_pane_red_side.png");
	f->setTexture(5,"glass.png^glass_pane_red_side.png");
#ifndef SERVER
	f->setAllTextureTypes(MATERIAL_ALPHA_BLEND);
#endif
	content_nodebox_glass_pane(f);
	f->setInventoryTextureNodeBox(i,"glass_pane_red_side.png", "glass.png^glass_pane_red_side.png", "glass_pane_red_side.png");
	f->type = CMT_GLASS;
	f->dig_time = 0.15;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	crafting::setCol3Recipe(CONTENT_GLASS_RED,CONTENT_GLASS_PANE_RED);
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_DYE_RED,CONTENT_GLASS_PANE,CONTENT_GLASS_PANE_RED);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_GLASS_PANE_YELLOW;
	f = &content_features(i);
	f->description = gettext("Yellow Glass Pane");
	f->param2_type = CPT_FACEDIR_SIMPLE;
	f->param_type = CPT_LIGHT;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->setAllTextures("glass_pane_yellow_side.png");
	f->setTexture(4,"glass.png^glass_pane_yellow_side.png");
	f->setTexture(5,"glass.png^glass_pane_yellow_side.png");
#ifndef SERVER
	f->setAllTextureTypes(MATERIAL_ALPHA_BLEND);
#endif
	content_nodebox_glass_pane(f);
	f->setInventoryTextureNodeBox(i,"glass_pane_yellow_side.png", "glass.png^glass_pane_yellow_side.png", "glass_pane_yellow_side.png");
	f->type = CMT_GLASS;
	f->dig_time = 0.15;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	crafting::setCol3Recipe(CONTENT_GLASS_YELLOW,CONTENT_GLASS_PANE_YELLOW);
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_DYE_YELLOW,CONTENT_GLASS_PANE,CONTENT_GLASS_PANE_YELLOW);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_GLASS_PANE_BLACK;
	f = &content_features(i);
	f->description = gettext("Black Glass Pane");
	f->param2_type = CPT_FACEDIR_SIMPLE;
	f->param_type = CPT_LIGHT;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->setAllTextures("glass_pane_black_side.png");
	f->setTexture(4,"glass.png^glass_pane_black_side.png");
	f->setTexture(5,"glass.png^glass_pane_black_side.png");
#ifndef SERVER
	f->setAllTextureTypes(MATERIAL_ALPHA_BLEND);
#endif
	content_nodebox_glass_pane(f);
	f->setInventoryTextureNodeBox(i,"glass_pane_black_side.png", "glass.png^glass_pane_black_side.png", "glass_pane_black_side.png");
	f->type = CMT_GLASS;
	f->dig_time = 0.15;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	crafting::setCol3Recipe(CONTENT_GLASS_BLACK,CONTENT_GLASS_PANE_BLACK);
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_DYE_BLACK,CONTENT_GLASS_PANE,CONTENT_GLASS_PANE_BLACK);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_GLASSLIGHT;
	f = &content_features(i);
	f->description = gettext("Glass Light");
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_GLASSLIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->setAllTextures("glasslight.png");
#ifndef SERVER
	f->setAllTextureTypes(MATERIAL_ALPHA_BLEND);
#endif
	f->setInventoryTextureCube("glasslight.png", "glasslight.png", "glasslight.png");
	f->type = CMT_GLASS;
	f->dig_time = 0.15;
	f->light_source = LIGHT_MAX-1;
	crafting::setSurroundRecipe(CONTENT_GLASS,CONTENT_CRAFTITEM_FIREFLY,CONTENT_GLASSLIGHT);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_WOOD;
	f = &content_features(i);
	f->description = gettext("Wood");
	f->setAllTextures("wood.png");
	f->setInventoryTextureCube("wood.png", "wood.png", "wood.png");
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_WOOD;
	f->type = CMT_WOOD;
	f->dig_time = 0.75;
	crafting::set1To4Recipe(CONTENT_TREE,CONTENT_WOOD);
	crafting::set4SpacedTo1Recipe(CONTENT_CRAFTITEM_WOOD_PLANK,CONTENT_WOOD);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_APPLEWOOD;
	f = &content_features(i);
	f->description = gettext("Apple Wood");
	f->setAllTextures("applewood.png");
	f->setInventoryTextureCube("applewood.png", "applewood.png", "applewood.png");
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_WOOD;
	f->type = CMT_WOOD;
	f->dig_time = 0.75;
	crafting::set1To4Recipe(CONTENT_APPLE_TREE,CONTENT_APPLEWOOD);
	crafting::set4SpacedTo1Recipe(CONTENT_CRAFTITEM_APPLE_PLANK,CONTENT_APPLEWOOD);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_JUNGLEWOOD;
	f = &content_features(i);
	f->description = gettext("Jungle Wood");
	f->setAllTextures("junglewood.png");
	f->setInventoryTextureCube("junglewood.png", "junglewood.png", "junglewood.png");
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_WOOD;
	f->type = CMT_WOOD;
	f->dig_time = 0.75;
	crafting::set1To4Recipe(CONTENT_JUNGLETREE,CONTENT_JUNGLEWOOD);
	crafting::set4SpacedTo1Recipe(CONTENT_CRAFTITEM_JUNGLE_PLANK,CONTENT_JUNGLEWOOD);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_WOOD_PINE;
	f = &content_features(i);
	f->description = gettext("Pine");
	f->setAllTextures("pine.png");
	f->setInventoryTextureCube("pine.png", "pine.png", "pine.png");
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_WOOD;
	f->type = CMT_WOOD;
	f->dig_time = 0.5;
	crafting::set1To4Recipe(CONTENT_CONIFER_TREE,CONTENT_WOOD_PINE);
	crafting::set4SpacedTo1Recipe(CONTENT_CRAFTITEM_PINE_PLANK,CONTENT_WOOD_PINE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_SPONGE;
	f = &content_features(i);
	f->description = gettext("Sponge");
	f->setAllTextures("sponge.png");
	f->setInventoryTextureCube("sponge.png", "sponge.png", "sponge.png");
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_SPONGE;
	f->type = CMT_DIRT;
	f->dig_time = 1.0;
	f->pressure_type = CST_CRUSHABLE;
	content_list_add("creative",i,1,0);

	i = CONTENT_SPONGE_FULL;
	f = &content_features(i);
	f->description = gettext("Waterlogged Sponge");
	f->setAllTextures("sponge_full.png");
	f->setInventoryTextureCube("sponge_full.png", "sponge_full.png", "sponge_full.png");
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->ondig_special_drop = CONTENT_SPONGE;
	f->ondig_special_tool = TT_BUCKET;
	f->type = CMT_DIRT;
	f->dig_time = 1.0;
	f->pressure_type = CST_CRUSHABLE;

	i = CONTENT_HAY;
	f = &content_features(i);
	f->description = gettext("Hay Bale");
	f->setAllTextures("hay_bale.png");
	f->setInventoryTextureCube("hay_bale.png", "hay_bale.png", "hay_bale.png");
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->flammable = 1;
	f->fuel_time = BT_HAY;
	f->type = CMT_PLANT;
	f->dig_time = 0.6;
	f->pressure_type = CST_CRUSHABLE;
	crafting::setSoftBlockRecipe(CONTENT_CRAFTITEM_STRAW,CONTENT_HAY);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_APPLE_PIE_RAW;
	f = &content_features(i);
	f->description = gettext("Raw Apple Pie");
	f->setTexture(0, "apple_pie_raw.png");
	f->setTexture(1, "apple_pie_raw.png");
	f->setTexture(2, "apple_pie_raw.png");
	f->setTexture(3, "apple_pie_raw.png");
	f->setTexture(4, "apple_pie_raw.png");
	f->setTexture(5, "apple_pie_raw.png");
	f->draw_type = CDT_NODEBOX;
	f->param_type = CPT_LIGHT;
	f->flammable = true;
	f->walkable = true;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->cook_result = std::string("MaterialItem2 ")+itos(CONTENT_APPLE_PIE)+"  1";
	content_nodebox_pie(f);
	f->setInventoryTextureNodeBox(i, "apple_pie_raw.png", "apple_pie_raw.png", "apple_pie_raw.png");
	f->type = CMT_DIRT;
	f->dig_time = 0.1;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	crafting::set1over1Recipe(CONTENT_CRAFTITEM_APPLE,CONTENT_CRAFTITEM_DOUGH,CONTENT_APPLE_PIE_RAW);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);
	content_list_add("cooking",i,1,0);

	i = CONTENT_APPLE_PIE;
	f = &content_features(i);
	f->description = gettext("Apple Pie");
	f->setTexture(0, "apple_pie.png");
	f->setTexture(1, "apple_pie.png");
	f->setTexture(2, "apple_pie.png");
	f->setTexture(3, "apple_pie.png");
	f->setTexture(4, "apple_pie.png");
	f->setTexture(5, "apple_pie.png");
	f->draw_type = CDT_NODEBOX;
	f->param_type = CPT_LIGHT;
	f->flammable = true;
	f->walkable = true;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_pie(f);
	f->setInventoryTextureNodeBox(i, "apple_pie.png", "apple_pie.png", "apple_pie.png");
	f->dug_item = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_APPLE_PIE_SLICE)+" 1";
	f->ondig_replace_node = CONTENT_APPLE_PIE_3;
	f->type = CMT_DIRT;
	f->dig_time = 0.1;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_list_add("creative",i,1,0);
	content_list_add("decrafting",i,1,0);

	i = CONTENT_APPLE_PIE_3;
	f = &content_features(i);
	f->setTexture(0, "apple_pie.png");
	f->setTexture(1, "apple_pie.png");
	f->setTexture(2, "apple_pie.png^[transformR90");
	f->setTexture(3, "apple_pie.png");
	f->setTexture(4, "apple_pie.png");
	f->setTexture(5, "apple_pie.png^[transformR270");
	f->draw_type = CDT_NODEBOX;
	f->param_type = CPT_LIGHT;
	f->flammable = true;
	f->walkable = true;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_pie_3(f);
	f->dug_item = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_APPLE_PIE_SLICE)+" 1";
	f->ondig_replace_node=CONTENT_APPLE_PIE_2;
	f->type = CMT_DIRT;
	f->dig_time = 0.1;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;

	i = CONTENT_APPLE_PIE_2;
	f = &content_features(i);
	f->setTexture(0, "apple_pie.png");
	f->setTexture(1, "apple_pie.png");
	f->setTexture(2, "apple_pie.png^[transformFY");
	f->setTexture(3, "apple_pie.png");
	f->setTexture(4, "apple_pie.png");
	f->setTexture(5, "apple_pie.png");
	f->draw_type = CDT_NODEBOX;
	f->param_type = CPT_LIGHT;
	f->flammable = true;
	f->walkable = true;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_pie_2(f);
	f->dug_item = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_APPLE_PIE_SLICE)+" 1";
	f->ondig_replace_node=CONTENT_APPLE_PIE_1;
	f->type = CMT_DIRT;
	f->dig_time = 0.1;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;

	i = CONTENT_APPLE_PIE_1;
	f = &content_features(i);
	f->setTexture(0, "apple_pie.png");
	f->setTexture(1, "apple_pie.png");
	f->setTexture(2, "apple_pie.png^[transformFY");
	f->setTexture(3, "apple_pie.png");
	f->setTexture(4, "apple_pie.png^[transformFY");
	f->setTexture(5, "apple_pie.png");
	f->draw_type = CDT_NODEBOX;
	f->param_type = CPT_LIGHT;
	f->flammable = true;
	f->walkable = true;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_pie_1(f);
	f->dug_item = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_APPLE_PIE_SLICE)+" 1";
	f->type = CMT_DIRT;
	f->dig_time = 0.1;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;

	i = CONTENT_PUMPKIN_PIE_RAW;
	f = &content_features(i);
	f->description = gettext("Raw Pumpkin Pie");
	f->setTexture(0, "pumpkin_pie_raw.png");
	f->setTexture(1, "pumpkin_pie_raw.png");
	f->setTexture(2, "pumpkin_pie_raw.png");
	f->setTexture(3, "pumpkin_pie_raw.png");
	f->setTexture(4, "pumpkin_pie_raw.png");
	f->setTexture(5, "pumpkin_pie_raw.png");
	f->draw_type = CDT_NODEBOX;
	f->param_type = CPT_LIGHT;
	f->flammable = true;
	f->walkable = true;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->cook_result = std::string("MaterialItem2 ")+itos(CONTENT_PUMPKIN_PIE)+"  1";
	content_nodebox_pie(f);
	f->setInventoryTextureNodeBox(i, "pumpkin_pie_raw.png", "pumpkin_pie_raw.png", "pumpkin_pie_raw.png");
	f->type = CMT_DIRT;
	f->dig_time = 0.1;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	crafting::set1over1Recipe(CONTENT_CRAFTITEM_PUMPKINSLICE,CONTENT_CRAFTITEM_DOUGH,CONTENT_PUMPKIN_PIE_RAW);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);
	content_list_add("cooking",i,1,0);

	i = CONTENT_PUMPKIN_PIE;
	f = &content_features(i);
	f->description = gettext("Pumpkin Pie");
	f->setTexture(0, "pumpkin_pie.png");
	f->setTexture(1, "pumpkin_pie.png");
	f->setTexture(2, "pumpkin_pie.png");
	f->setTexture(3, "pumpkin_pie.png");
	f->setTexture(4, "pumpkin_pie.png");
	f->setTexture(5, "pumpkin_pie.png");
	f->draw_type = CDT_NODEBOX;
	f->param_type = CPT_LIGHT;
	f->flammable = true;
	f->walkable = true;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_pie(f);
	f->setInventoryTextureNodeBox(i, "pumpkin_pie.png", "pumpkin_pie.png", "pumpkin_pie.png");
	f->dug_item = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_PUMPKIN_PIE_SLICE)+" 1";
	f->ondig_replace_node=CONTENT_PUMPKIN_PIE_3;
	f->type = CMT_DIRT;
	f->dig_time = 0.1;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_list_add("creative",i,1,0);
	content_list_add("decrafting",i,1,0);

	i = CONTENT_PUMPKIN_PIE_3;
	f = &content_features(i);
	f->setTexture(0, "pumpkin_pie.png");
	f->setTexture(1, "pumpkin_pie.png");
	f->setTexture(2, "pumpkin_pie.png^[transformR90");
	f->setTexture(3, "pumpkin_pie.png");
	f->setTexture(4, "pumpkin_pie.png");
	f->setTexture(5, "pumpkin_pie.png^[transformR270");
	f->draw_type = CDT_NODEBOX;
	f->param_type = CPT_LIGHT;
	f->flammable = true;
	f->walkable = true;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_pie_3(f);
	f->dug_item = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_PUMPKIN_PIE_SLICE)+" 1";
	f->ondig_replace_node=CONTENT_PUMPKIN_PIE_2;
	f->type = CMT_DIRT;
	f->dig_time = 0.1;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;

	i = CONTENT_PUMPKIN_PIE_2;
	f = &content_features(i);
	f->setTexture(0, "pumpkin_pie.png");
	f->setTexture(1, "pumpkin_pie.png");
	f->setTexture(2, "pumpkin_pie.png^[transformFY");
	f->setTexture(3, "pumpkin_pie.png");
	f->setTexture(4, "pumpkin_pie.png");
	f->setTexture(5, "pumpkin_pie.png");
	f->draw_type = CDT_NODEBOX;
	f->param_type = CPT_LIGHT;
	f->flammable = true;
	f->walkable = true;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_pie_2(f);
	f->dug_item = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_PUMPKIN_PIE_SLICE)+" 1";
	f->ondig_replace_node=CONTENT_PUMPKIN_PIE_1;
	f->type = CMT_DIRT;
	f->dig_time = 0.1;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;

	i = CONTENT_PUMPKIN_PIE_1;
	f = &content_features(i);
	f->setTexture(0, "pumpkin_pie.png");
	f->setTexture(1, "pumpkin_pie.png");
	f->setTexture(2, "pumpkin_pie.png^[transformFY");
	f->setTexture(3, "pumpkin_pie.png");
	f->setTexture(4, "pumpkin_pie.png^[transformFY");
	f->setTexture(5, "pumpkin_pie.png");
	f->draw_type = CDT_NODEBOX;
	f->param_type = CPT_LIGHT;
	f->flammable = true;
	f->walkable = true;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_pie_1(f);
	f->dug_item = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_PUMPKIN_PIE_SLICE)+" 1";
	f->type = CMT_DIRT;
	f->dig_time = 0.1;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;

	i = CONTENT_SNOW_BLOCK;
	f = &content_features(i);
	f->description = gettext("Snow");
	f->setAllTextures("snow.png");
	f->setInventoryTextureCube("snow.png", "snow.png", "snow.png");
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_SNOW_BALL)+" 9";
	f->type = CMT_DIRT;
	f->dig_time = 0.3;
	f->warmth_per_second = 10;
	crafting::setHardBlockRecipe(CONTENT_CRAFTITEM_SNOW_BALL,CONTENT_SNOW_BLOCK);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);
	content_list_add("decrafting",i,1,0);

	i = CONTENT_SNOWMAN;
	f = &content_features(i);
	f->description = gettext("Snowman");
	f->param2_type = CPT_FACEDIR_SIMPLE;
	f->setAllTextures("snowman.png");
	f->setTexture(0,"snowman_top.png");
	f->setTexture(1,"snowman_top.png");
	f->setTexture(5,"snowman_front.png");
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_SNOW_BALL)+" 18";
	f->extra_dug_item = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_STICK)+" 3";
	f->extra_dug_item_rarity = 1;
	f->rotate_tile_with_nodebox = true;
	f->type = CMT_DIRT;
	f->pressure_type = CST_CRUSHABLE;
	f->dig_time = 0.3;
	content_nodebox_snowman(f);
	f->setInventoryTextureNodeBox(i,"snow.png", "snow.png", "snow.png");
	{
		content_t r[9] = {
			CONTENT_IGNORE,			CONTENT_IGNORE,		CONTENT_IGNORE,
			CONTENT_IGNORE,			CONTENT_SNOW_BLOCK,	CONTENT_CRAFTITEM_CARROT,
			CONTENT_CRAFTITEM_STICK,	CONTENT_SNOW_BLOCK,	CONTENT_CRAFTITEM_STICK
		};
		crafting::setRecipe(r,CONTENT_SNOWMAN,1);
	}
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);
	content_list_add("decrafting",i,1,0);

	i = CONTENT_SNOW;
	f = &content_features(i);
	f->description = gettext("Snow");
	f->setAllTextures("snow.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->light_propagates = true;
	f->buildable_to = true;
	f->walkable = false;
	f->material_pointable = false;
	f->sunlight_propagates = true;
	f->dug_item = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_SNOW_BALL)+" 1";
	content_nodebox_carpet(f);
	f->setInventoryTextureNodeBox(i,"snow.png", "snow.png", "snow.png");
	f->type = CMT_DIRT;
	f->dig_time = 0.3;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	f->warmth_per_second = 10;
	content_list_add("creative",i,1,0);
	content_list_add("decrafting",i,1,0);

	i = CONTENT_COTTON;
	f = &content_features(i);
	f->description = gettext("Cotton");
	f->setAllTextures("cotton.png");
	f->setInventoryTextureCube("cotton.png", "cotton.png", "cotton.png");
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_COTTON;
	f->type = CMT_DIRT;
	f->dig_time = 1.0;
	f->pressure_type = CST_DROPABLE;
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_DYE_WHITE,CONTENT_COTTON_BLUE,CONTENT_COTTON);
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_DYE_WHITE,CONTENT_COTTON_GREEN,CONTENT_COTTON);
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_DYE_WHITE,CONTENT_COTTON_ORANGE,CONTENT_COTTON);
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_DYE_WHITE,CONTENT_COTTON_PURPLE,CONTENT_COTTON);
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_DYE_WHITE,CONTENT_COTTON_RED,CONTENT_COTTON);
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_DYE_WHITE,CONTENT_COTTON_YELLOW,CONTENT_COTTON);
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_DYE_WHITE,CONTENT_COTTON_BLACK,CONTENT_COTTON);
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_STARCH,CONTENT_COTTON_BLUE,CONTENT_COTTON);
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_STARCH,CONTENT_COTTON_GREEN,CONTENT_COTTON);
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_STARCH,CONTENT_COTTON_ORANGE,CONTENT_COTTON);
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_STARCH,CONTENT_COTTON_PURPLE,CONTENT_COTTON);
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_STARCH,CONTENT_COTTON_RED,CONTENT_COTTON);
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_STARCH,CONTENT_COTTON_YELLOW,CONTENT_COTTON);
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_STARCH,CONTENT_COTTON_BLACK,CONTENT_COTTON);
	crafting::setSoftBlockRecipe(CONTENT_CRAFTITEM_STRING,CONTENT_COTTON);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_COTTON_BLUE;
	f = &content_features(i);
	f->description = gettext("Blue Cotton");
	f->setAllTextures("cotton_blue.png");
	f->setInventoryTextureCube("cotton_blue.png", "cotton_blue.png", "cotton_blue.png");
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_COTTON;
	f->type = CMT_DIRT;
	f->dig_time = 1.0;
	f->pressure_type = CST_DROPABLE;
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_DYE_BLUE,CONTENT_COTTON,CONTENT_COTTON_BLUE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_COTTON_GREEN;
	f = &content_features(i);
	f->description = gettext("Green Cotton");
	f->setAllTextures("cotton_green.png");
	f->setInventoryTextureCube("cotton_green.png", "cotton_green.png", "cotton_green.png");
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_COTTON;
	f->type = CMT_DIRT;
	f->dig_time = 1.0;
	f->pressure_type = CST_DROPABLE;
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_DYE_GREEN,CONTENT_COTTON,CONTENT_COTTON_GREEN);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_COTTON_ORANGE;
	f = &content_features(i);
	f->description = gettext("Orange Cotton");
	f->setAllTextures("cotton_orange.png");
	f->setInventoryTextureCube("cotton_orange.png", "cotton_orange.png", "cotton_orange.png");
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_COTTON;
	f->type = CMT_DIRT;
	f->dig_time = 1.0;
	f->pressure_type = CST_DROPABLE;
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_DYE_ORANGE,CONTENT_COTTON,CONTENT_COTTON_ORANGE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_COTTON_PURPLE;
	f = &content_features(i);
	f->description = gettext("Purple Cotton");
	f->setAllTextures("cotton_purple.png");
	f->setInventoryTextureCube("cotton_purple.png", "cotton_purple.png", "cotton_purple.png");
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_COTTON;
	f->type = CMT_DIRT;
	f->dig_time = 1.0;
	f->pressure_type = CST_DROPABLE;
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_DYE_PURPLE,CONTENT_COTTON,CONTENT_COTTON_PURPLE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_COTTON_RED;
	f = &content_features(i);
	f->description = gettext("Red Cotton");
	f->setAllTextures("cotton_red.png");
	f->setInventoryTextureCube("cotton_red.png", "cotton_red.png", "cotton_red.png");
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_COTTON;
	f->type = CMT_DIRT;
	f->dig_time = 1.0;
	f->pressure_type = CST_DROPABLE;
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_DYE_RED,CONTENT_COTTON,CONTENT_COTTON_RED);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_COTTON_YELLOW;
	f = &content_features(i);
	f->description = gettext("Yellow Cotton");
	f->setAllTextures("cotton_yellow.png");
	f->setInventoryTextureCube("cotton_yellow.png", "cotton_yellow.png", "cotton_yellow.png");
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_COTTON;
	f->type = CMT_DIRT;
	f->dig_time = 1.0;
	f->pressure_type = CST_DROPABLE;
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_DYE_YELLOW,CONTENT_COTTON,CONTENT_COTTON_YELLOW);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_COTTON_BLACK;
	f = &content_features(i);
	f->description = gettext("Black Cotton");
	f->setAllTextures("cotton_black.png");
	f->setInventoryTextureCube("cotton_black.png", "cotton_black.png", "cotton_black.png");
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_COTTON;
	f->type = CMT_DIRT;
	f->dig_time = 1.0;
	f->pressure_type = CST_DROPABLE;
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_DYE_BLACK,CONTENT_COTTON,CONTENT_COTTON_BLACK);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CARPET;
	f = &content_features(i);
	f->description = gettext("Carpet");
	f->setAllTextures("cotton.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	content_nodebox_carpet(f);
	f->setInventoryTextureNodeBox(i,"cotton.png", "cotton.png", "cotton.png");
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_CARPET;
	f->type = CMT_DIRT;
	f->dig_time = 1.0;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	crafting::setTileRecipe(CONTENT_COTTON,CONTENT_CARPET);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CARPET_BLUE;
	f = &content_features(i);
	f->description = gettext("Blue Carpet");
	f->setAllTextures("cotton_blue.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	content_nodebox_carpet(f);
	f->setInventoryTextureNodeBox(i,"cotton_blue.png", "cotton_blue.png", "cotton_blue.png");
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_CARPET;
	f->type = CMT_DIRT;
	f->dig_time = 1.0;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	crafting::setTileRecipe(CONTENT_COTTON_BLUE,CONTENT_CARPET_BLUE);
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_DYE_BLUE,CONTENT_CARPET,CONTENT_CARPET_BLUE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CARPET_GREEN;
	f = &content_features(i);
	f->description = gettext("Green Carpet");
	f->setAllTextures("cotton_green.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	content_nodebox_carpet(f);
	f->setInventoryTextureNodeBox(i,"cotton_green.png", "cotton_green.png", "cotton_green.png");
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_CARPET;
	f->type = CMT_DIRT;
	f->dig_time = 1.0;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	crafting::setTileRecipe(CONTENT_COTTON_GREEN,CONTENT_CARPET_GREEN);
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_DYE_GREEN,CONTENT_CARPET,CONTENT_CARPET_GREEN);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CARPET_ORANGE;
	f = &content_features(i);
	f->description = gettext("Orange Carpet");
	f->setAllTextures("cotton_orange.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	content_nodebox_carpet(f);
	f->setInventoryTextureNodeBox(i,"cotton_orange.png", "cotton_orange.png", "cotton_orange.png");
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_CARPET;
	f->type = CMT_DIRT;
	f->dig_time = 1.0;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	crafting::setTileRecipe(CONTENT_COTTON_ORANGE,CONTENT_CARPET_ORANGE);
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_DYE_ORANGE,CONTENT_CARPET,CONTENT_CARPET_ORANGE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CARPET_PURPLE;
	f = &content_features(i);
	f->description = gettext("Purple Carpet");
	f->setAllTextures("cotton_purple.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	content_nodebox_carpet(f);
	f->setInventoryTextureNodeBox(i,"cotton_purple.png", "cotton_purple.png", "cotton_purple.png");
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_CARPET;
	f->type = CMT_DIRT;
	f->dig_time = 1.0;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	crafting::setTileRecipe(CONTENT_COTTON_PURPLE,CONTENT_CARPET_PURPLE);
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_DYE_PURPLE,CONTENT_CARPET,CONTENT_CARPET_PURPLE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CARPET_RED;
	f = &content_features(i);
	f->description = gettext("Red Carpet");
	f->setAllTextures("cotton_red.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	content_nodebox_carpet(f);
	f->setInventoryTextureNodeBox(i,"cotton_red.png", "cotton_red.png", "cotton_red.png");
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_CARPET;
	f->type = CMT_DIRT;
	f->dig_time = 1.0;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	crafting::setTileRecipe(CONTENT_COTTON_RED,CONTENT_CARPET_RED);
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_DYE_RED,CONTENT_CARPET,CONTENT_CARPET_RED);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CARPET_YELLOW;
	f = &content_features(i);
	f->description = gettext("Yellow Carpet");
	f->setAllTextures("cotton_yellow.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	content_nodebox_carpet(f);
	f->setInventoryTextureNodeBox(i,"cotton_yellow.png", "cotton_yellow.png", "cotton_yellow.png");
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_CARPET;
	f->type = CMT_DIRT;
	f->dig_time = 1.0;
	f->suffocation_per_second = 0;
	f->pressure_type = CST_CRUSHABLE;
	crafting::setTileRecipe(CONTENT_COTTON_YELLOW,CONTENT_CARPET_YELLOW);
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_DYE_YELLOW,CONTENT_CARPET,CONTENT_CARPET_YELLOW);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CARPET_BLACK;
	f = &content_features(i);
	f->description = gettext("Black Carpet");
	f->setAllTextures("cotton_black.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	content_nodebox_carpet(f);
	f->setInventoryTextureNodeBox(i,"cotton_black.png", "cotton_black.png", "cotton_black.png");
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_CARPET;
	f->type = CMT_DIRT;
	f->dig_time = 1.0;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	crafting::setTileRecipe(CONTENT_COTTON_BLACK,CONTENT_CARPET_BLACK);
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_DYE_BLACK,CONTENT_CARPET,CONTENT_CARPET_BLACK);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_AIR;
	f = &content_features(i);
	f->description = gettext("Air");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_AIRLIKE;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->walkable = false;
	f->pointable = false;
	f->diggable = false;
	f->buildable_to = true;
	f->air_equivalent = true;
	f->pressure_type = CST_CRUSHED;
	f->suffocation_per_second = 0;

	i = CONTENT_VACUUM;
	f = &content_features(i);
	f->description = gettext("Vacuum");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_AIRLIKE;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->walkable = false;
	f->pointable = false;
	f->diggable = false;
	f->buildable_to = true;
	f->air_equivalent = true;
	f->pressure_type = CST_CRUSHED;
	f->suffocation_per_second = 75;
	f->pressure_per_second = 50;

	i = CONTENT_WATER;
	f = &content_features(i);
	f->description = gettext("Water");
	f->setAllTextures("water.png");
	f->setInventoryTextureCube("water.png", "water.png", "water.png");
	f->param_type = CPT_LIGHT;
	f->param2_type = CPT_LIQUID;
	f->draw_type = CDT_LIQUID;
	f->light_propagates = true;
	f->walkable = false;
	f->pointable = false;
	f->diggable = false;
	f->buildable_to = true;
	f->pressure_type = CST_CRUSHABLE;
	f->liquid_type = LIQUID_FLOWING;
	f->liquid_alternative_flowing = CONTENT_WATER;
	f->liquid_alternative_source = CONTENT_WATERSOURCE;
	f->liquid_viscosity = WATER_VISC;
#ifndef SERVER
	if(!opaque_water)
		f->vertex_alpha = WATER_ALPHA;
	f->setAllTextureTypes(MATERIAL_ALPHA_VERTEX);
	f->post_effect_color = video::SColor(64, 100, 100, 200);
	f->animation_length = 0.8;
	f->setAllTextureFlags(MATERIAL_FLAG_ANIMATION_VERTICAL_FRAMES);
	f->setAllTextures("water_flowing.png");
#endif
	f->sound_ambient = "env-water";
	f->suffocation_per_second = 10;

	i = CONTENT_WATERSOURCE;
	f = &content_features(i);
	f->description = gettext("Water");
	f->setAllTextures("water.png");
	f->setInventoryTextureCube("water.png", "water.png", "water.png");
	f->param_type = CPT_LIGHT;
	f->param2_type = CPT_LIQUID;
	f->draw_type = CDT_LIQUID_SOURCE;
	f->light_propagates = true;
	f->walkable = false;
	f->pointable = false;
	f->buildable_to = true;
	f->liquid_type = LIQUID_SOURCE;
	f->dug_item = std::string("_water"); // won't resolve to an item, but will work when added to a bucket tool name
	f->liquid_alternative_flowing = CONTENT_WATER;
	f->liquid_alternative_source = CONTENT_WATERSOURCE;
	f->liquid_viscosity = WATER_VISC;
#ifndef SERVER
	if(!opaque_water)
		f->vertex_alpha = WATER_ALPHA;
	f->setAllTextureTypes(MATERIAL_ALPHA_VERTEX);
	f->post_effect_color = video::SColor(64, 100, 100, 200);
	f->animation_length = 2.0;
	f->setAllTextureFlags(MATERIAL_FLAG_ANIMATION_VERTICAL_FRAMES);
	f->setAllTextures("water_source.png");
#endif
	f->type = CMT_LIQUID;
	f->dig_time = 0.5;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 10;

	i = CONTENT_LAVA;
	f = &content_features(i);
	f->description = gettext("Lava");
	f->setAllTextures("lava.png");
	f->setInventoryTextureCube("lava.png", "lava.png", "lava.png");
	f->param_type = CPT_LIGHT;
	f->param2_type = CPT_LIQUID;
	f->draw_type = CDT_LIQUID;
	f->light_propagates = false;
	f->light_source = LIGHT_MAX-1;
	f->walkable = false;
	f->pointable = false;
	f->diggable = false;
	f->buildable_to = true;
	f->flammable = 2; // can be set on fire
	f->fuel_time = BT_LAVA;
	f->pressure_type = CST_CRUSHABLE;
	f->liquid_type = LIQUID_FLOWING;
	f->liquid_alternative_flowing = CONTENT_LAVA;
	f->liquid_alternative_source = CONTENT_LAVASOURCE;
	f->liquid_viscosity = LAVA_VISC;
	f->damage_per_second = 40;
#ifndef SERVER
	f->post_effect_color = video::SColor(192, 255, 64, 0);
	f->animation_length = 3.0;
	f->setAllTextureFlags(MATERIAL_FLAG_ANIMATION_VERTICAL_FRAMES);
	f->setAllTextures("lava_flowing.png");
#endif

	i = CONTENT_LAVASOURCE;
	f = &content_features(i);
	f->description = gettext("Lava");
	f->setAllTextures("lava.png");
	f->setInventoryTextureCube("lava.png", "lava.png", "lava.png");
	f->param_type = CPT_LIGHT;
	f->param2_type = CPT_LIQUID;
	f->draw_type = CDT_LIQUID_SOURCE;
	f->light_propagates = false;
	f->light_source = LIGHT_MAX-1;
	f->walkable = false;
	f->pointable = false;
	//f->diggable = false;
	f->buildable_to = true;
	f->flammable = 2; // can be set on fire
	f->fuel_time = BT_LAVA;
	f->liquid_type = LIQUID_SOURCE;
	f->dug_item = std::string("_lava"); // won't resolve to an item, but will work when added to a bucket tool name
	f->liquid_alternative_flowing = CONTENT_LAVA;
	f->liquid_alternative_source = CONTENT_LAVASOURCE;
	f->liquid_viscosity = LAVA_VISC;
	f->damage_per_second = 40;
	f->sound_ambient = "env-lava";
#ifndef SERVER
	f->post_effect_color = video::SColor(192, 255, 64, 0);
	f->animation_length = 3.0;
	f->setAllTextureFlags(MATERIAL_FLAG_ANIMATION_VERTICAL_FRAMES);
	f->setAllTextures("lava_source.png");
#endif
	f->type = CMT_LIQUID;
	f->dig_time = 0.5;
	f->pressure_type = CST_CRUSHABLE;

	i = CONTENT_ROUGHSTONE;
	f = &content_features(i);
	f->description = gettext("Rough Stone");
	f->setAllTextures("roughstone.png");
	f->setInventoryTextureCube("roughstone.png", "roughstone.png", "roughstone.png");
	f->param_type = CPT_NONE;
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->cook_result = std::string("MaterialItem2 ")+itos(CONTENT_STONE)+" 1";
	f->cook_type = COOK_FURNACE|COOK_SMELTERY;
	f->type = CMT_STONE;
	f->dig_time = 0.9;
	f->crush_result = std::string("MaterialItem2 ")+itos(CONTENT_COBBLE)+" 1";
	f->crush_type = CRUSH_CRUSHER;
	crafting::setBlockRecipe(CONTENT_COBBLE,CONTENT_ROUGHSTONE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);
	content_list_add("cooking",i,1,0);

	i = CONTENT_COBBLE;
	f = &content_features(i);
	f->description = gettext("Cobble Stone");
	f->setAllTextures("cobble.png");
	f->setInventoryTextureCube("cobble.png", "cobble.png", "cobble.png");
	f->param_type = CPT_NONE;
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->type = CMT_STONE;
	f->dig_time = 0.9;
	f->crush_result = std::string("MaterialItem2 ")+itos(CONTENT_GRAVEL)+" 1";
	f->crush_type = CRUSH_CRUSHER;
	crafting::set5Recipe(CONTENT_ROUGHSTONE,CONTENT_COBBLE);
	crafting::setHardBlockRecipe(CONTENT_ROCK,CONTENT_COBBLE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_MOSSYCOBBLE;
	f = &content_features(i);
	f->description = gettext("Mossy Cobble Stone");
	f->setAllTextures("mossycobble.png");
	f->setInventoryTextureCube("mossycobble.png", "mossycobble.png", "mossycobble.png");
	f->param_type = CPT_NONE;
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->type = CMT_STONE;
	f->dig_time = 0.8;
	f->crush_result = std::string("MaterialItem2 ")+itos(CONTENT_GRAVEL)+" 1";
	f->crush_type = CRUSH_CRUSHER;
	content_list_add("creative",i,1,0);

	i = CONTENT_IRON;
	f = &content_features(i);
	f->description = gettext("Iron Block");
	f->setAllTextures("iron_block.png");
	f->setInventoryTextureCube("iron_block.png", "iron_block.png", "iron_block.png");
	f->param_type = CPT_NONE;
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->type = CMT_STONE;
	f->dig_time = 5.0;
	f->destructive_mob_safe = true;
	crafting::setHardBlockRecipe(CONTENT_CRAFTITEM_IRON_INGOT,CONTENT_IRON);
	crafting::setUncraftHardBlockRecipe(CONTENT_IRON,CONTENT_CRAFTITEM_IRON_INGOT);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_MITHRIL_BLOCK;
	f = &content_features(i);
	f->description = gettext("Mithril Block");
	f->setAllTextures("mithril_block.png");
	f->setInventoryTextureCube("mithril_block.png", "mithril_block.png", "mithril_block.png");
	f->param_type = CPT_NONE;
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->type = CMT_STONE;
	f->dig_time = 10.0;
	f->destructive_mob_safe = true;
	crafting::setHardBlockRecipe(CONTENT_CRAFTITEM_MITHRIL_UNBOUND,CONTENT_MITHRIL_BLOCK);
	crafting::setHardBlockRecipe(CONTENT_CRAFTITEM_MITHRIL,CONTENT_MITHRIL_BLOCK);
	crafting::setUncraftHardBlockRecipe(CONTENT_MITHRIL_BLOCK,CONTENT_CRAFTITEM_MITHRIL_UNBOUND);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	content_nodedef_knob(CONTENT_STONE_KNOB, CONTENT_STONE, CMT_STONE, "stone.png", gettext("Stone Knob"));
	content_nodedef_knob(CONTENT_ROUGHSTONE_KNOB, CONTENT_ROUGHSTONE, CMT_STONE, "roughstone.png", gettext("Rough Stone Knob"));
	content_nodedef_knob(CONTENT_SANDSTONE_KNOB, CONTENT_SANDSTONE, CMT_STONE, "sandstone.png", gettext("Sandstone Knob"));
	content_nodedef_knob(CONTENT_WOOD_KNOB, CONTENT_CRAFTITEM_WOOD_PLANK, CMT_WOOD, "wood.png", gettext("Wooden Knob"));
	content_nodedef_knob(CONTENT_JUNGLEWOOD_KNOB, CONTENT_CRAFTITEM_JUNGLE_PLANK, CMT_WOOD, "junglewood.png", gettext("Junglewood Knob"));
	content_nodedef_knob(CONTENT_PINE_KNOB, CONTENT_CRAFTITEM_PINE_PLANK, CMT_WOOD, "pine.png", gettext("Pine Knob"));

	i = CONTENT_COPPER;
	f = &content_features(i);
	f->description = gettext("Copper Block");
	f->setAllTextures("copper_block.png");
	f->setInventoryTextureCube("copper_block.png", "copper_block.png", "copper_block.png");
	f->param_type = CPT_NONE;
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->type = CMT_STONE;
	f->dig_time = 5.0;
	f->destructive_mob_safe = true;
	crafting::setHardBlockRecipe(CONTENT_CRAFTITEM_COPPER_INGOT,CONTENT_COPPER);
	crafting::setUncraftHardBlockRecipe(CONTENT_COPPER,CONTENT_CRAFTITEM_COPPER_INGOT);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_GOLD;
	f = &content_features(i);
	f->description = gettext("Gold Block");
	f->setAllTextures("gold_block.png");
	f->setInventoryTextureCube("gold_block.png", "gold_block.png", "gold_block.png");
	f->param_type = CPT_NONE;
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->type = CMT_STONE;
	f->dig_time = 5.0;
	f->destructive_mob_safe = true;
	crafting::setHardBlockRecipe(CONTENT_CRAFTITEM_GOLD_INGOT,CONTENT_GOLD);
	crafting::setUncraftHardBlockRecipe(CONTENT_GOLD,CONTENT_CRAFTITEM_GOLD_INGOT);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_SILVER;
	f = &content_features(i);
	f->description = gettext("Silver Block");
	f->setAllTextures("silver_block.png");
	f->setInventoryTextureCube("silver_block.png", "silver_block.png", "silver_block.png");
	f->param_type = CPT_NONE;
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->type = CMT_STONE;
	f->dig_time = 5.0;
	f->destructive_mob_safe = true;
	crafting::setHardBlockRecipe(CONTENT_CRAFTITEM_SILVER_INGOT,CONTENT_SILVER);
	crafting::setUncraftHardBlockRecipe(CONTENT_SILVER,CONTENT_CRAFTITEM_SILVER_INGOT);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_TIN;
	f = &content_features(i);
	f->description = gettext("Tin Block");
	f->setAllTextures("tin_block.png");
	f->setInventoryTextureCube("tin_block.png", "tin_block.png", "tin_block.png");
	f->param_type = CPT_NONE;
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->type = CMT_STONE;
	f->dig_time = 5.0;
	f->destructive_mob_safe = true;
	crafting::setHardBlockRecipe(CONTENT_CRAFTITEM_TIN_INGOT,CONTENT_TIN);
	crafting::setUncraftHardBlockRecipe(CONTENT_TIN,CONTENT_CRAFTITEM_TIN_INGOT);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_QUARTZ;
	f = &content_features(i);
	f->description = gettext("Quartz Block");
	f->setAllTextures("quartz_block.png");
	f->setInventoryTextureCube("quartz_block.png", "quartz_block.png", "quartz_block.png");
	f->param_type = CPT_NONE;
	f->draw_type = CDT_CUBELIKE;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->type = CMT_STONE;
	f->dig_time = 5.0;
	f->destructive_mob_safe = true;
	crafting::setHardBlockRecipe(CONTENT_CRAFTITEM_QUARTZ,CONTENT_QUARTZ);
	crafting::setUncraftHardBlockRecipe(CONTENT_QUARTZ,CONTENT_CRAFTITEM_QUARTZ);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_STONE_TILE;
	f = &content_features(i);
	f->description = gettext("Stone Tiles");
	f->setAllTextures("stone.png^overlay_oddtile.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	content_nodebox_carpet(f);
	f->setInventoryTextureNodeBox(i,"stone.png^overlay_oddtile.png", "stone.png^overlay_oddtile.png", "stone.png^overlay_oddtile.png");
	f->type = CMT_STONE;
	f->dig_time = 0.5;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	crafting::setTileRecipe(CONTENT_STONE,CONTENT_STONE_TILE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_WOOD_TILE;
	f = &content_features(i);
	f->description = gettext("Wood Tiles");
	f->setAllTextures("wood_tile.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	content_nodebox_carpet(f);
	f->setInventoryTextureNodeBox(i,"wood_tile.png", "wood_tile.png", "wood_tile.png");
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_TILE;
	f->type = CMT_WOOD;
	f->dig_time = 0.5;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	crafting::setTileRecipe(CONTENT_WOOD,CONTENT_WOOD_TILE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_STONE_COLUMN_SQUARE;
	f = &content_features(i);
	f->description = gettext("Stone Column");
	f->setAllTextures("stone.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->air_equivalent = true; // grass grows underneath
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_column_square(f);
    f->setInventoryTextureNodeBox(i,"stone.png",
									"stone.png",
									"stone.png");
	f->type = CMT_STONE;
	f->dig_time = 1.0;
	f->onpunch_replace_node = CONTENT_STONE_COLUMN_SQUARE_BASE;
	f->onpunch_replace_respects_borderstone = true;
	crafting::setCol3Recipe(CONTENT_STONE,
							CONTENT_STONE_COLUMN_SQUARE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_STONE_COLUMN_SQUARE_BASE;
	f = &content_features(i);
	f->setAllTextures("stone.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+
	              itos(CONTENT_STONE_COLUMN_SQUARE)+" 1";
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_column_square_base(f);
	f->type = CMT_STONE;
	f->dig_time = 1.0;
	f->onpunch_replace_node = CONTENT_STONE_COLUMN_SQUARE_TOP;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_STONE_COLUMN_SQUARE_TOP;
	f = &content_features(i);
	f->setAllTextures("stone.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+
	              itos(CONTENT_STONE_COLUMN_SQUARE)+" 1";
	f->air_equivalent = true; // grass grows underneath
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_column_square_top(f);
	f->type = CMT_STONE;
	f->dig_time = 1.0;
	f->onpunch_replace_node = CONTENT_STONE_COLUMN_CROSS;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_LIMESTONE_COLUMN_SQUARE;
	f = &content_features(i);
	f->description = gettext("Limestone Column");
	f->setAllTextures("limestone.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->air_equivalent = true; // grass grows underneath
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_column_square(f);
    f->setInventoryTextureNodeBox(i,"limestone.png",
									"limestone.png",
									"limestone.png");
	f->type = CMT_STONE;
	f->dig_time = 0.6;
	f->onpunch_replace_node = CONTENT_LIMESTONE_COLUMN_SQUARE_BASE;
	f->onpunch_replace_respects_borderstone = true;
	crafting::setCol3Recipe(CONTENT_LIMESTONE,
							CONTENT_LIMESTONE_COLUMN_SQUARE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_LIMESTONE_COLUMN_SQUARE_BASE;
	f = &content_features(i);
	f->setAllTextures("limestone.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+
				  itos(CONTENT_LIMESTONE_COLUMN_SQUARE)+" 1";
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_column_square_base(f);
	f->type = CMT_STONE;
	f->dig_time = 0.6;
	f->onpunch_replace_node = CONTENT_LIMESTONE_COLUMN_SQUARE_TOP;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_LIMESTONE_COLUMN_SQUARE_TOP;
	f = &content_features(i);
	f->setAllTextures("limestone.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+
				  itos(CONTENT_LIMESTONE_COLUMN_SQUARE)+" 1";
	f->air_equivalent = true; // grass grows underneath
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_column_square_top(f);
	f->type = CMT_STONE;
	f->dig_time = 0.6;
	f->onpunch_replace_node = CONTENT_LIMESTONE_COLUMN_CROSS;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_MARBLE_COLUMN_SQUARE;
	f = &content_features(i);
	f->description = gettext("Marble Column");
	f->setAllTextures("marble.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->air_equivalent = true; // grass grows underneath
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_column_square(f);
    f->setInventoryTextureNodeBox(i,"marble.png",
									"marble.png",
									"marble.png");
	f->type = CMT_STONE;
	f->dig_time = 0.6;
	f->onpunch_replace_node = CONTENT_MARBLE_COLUMN_SQUARE_BASE;
	f->onpunch_replace_respects_borderstone = true;
	crafting::setCol3Recipe(CONTENT_MARBLE,
							CONTENT_MARBLE_COLUMN_SQUARE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_MARBLE_COLUMN_SQUARE_BASE;
	f = &content_features(i);
	f->setAllTextures("marble.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")
				  +itos(CONTENT_MARBLE_COLUMN_SQUARE)+" 1";
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_column_square_base(f);
	f->type = CMT_STONE;
	f->dig_time = 0.6;
	f->onpunch_replace_node = CONTENT_MARBLE_COLUMN_SQUARE_TOP;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_MARBLE_COLUMN_SQUARE_TOP;
	f = &content_features(i);
	f->setAllTextures("marble.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")
				  +itos(CONTENT_MARBLE_COLUMN_SQUARE)+" 1";
	f->air_equivalent = true; // grass grows underneath
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_column_square_top(f);
	f->type = CMT_STONE;
	f->dig_time = 0.6;
	f->onpunch_replace_node = CONTENT_MARBLE_COLUMN_CROSS;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_SANDSTONE_COLUMN_SQUARE;
	f = &content_features(i);
	f->description = gettext("Sandstone Column");
	f->setAllTextures("sandstone.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+
				  itos(CONTENT_SAND)+" 4";
	f->air_equivalent = true; // grass grows underneath
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_column_square(f);
    f->setInventoryTextureNodeBox(i,"sandstone.png",
									"sandstone.png",
									"sandstone.png");
	f->type = CMT_STONE;
	f->dig_time = 1.0;
	f->onpunch_replace_node = CONTENT_SANDSTONE_COLUMN_SQUARE_BASE;
	f->onpunch_replace_respects_borderstone = true;
	crafting::setCol3Recipe(CONTENT_SANDSTONE,
							CONTENT_SANDSTONE_COLUMN_SQUARE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_SANDSTONE_COLUMN_SQUARE_BASE;
	f = &content_features(i);
	f->setAllTextures("sandstone.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+
				  itos(CONTENT_SAND)+" 4";
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_column_square_base(f);
	f->type = CMT_STONE;
	f->dig_time = 1.0;
	f->onpunch_replace_node = CONTENT_SANDSTONE_COLUMN_SQUARE_TOP;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_SANDSTONE_COLUMN_SQUARE_TOP;
	f = &content_features(i);
	f->setAllTextures("sandstone.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+
				  itos(CONTENT_SAND)+" 4";
	f->air_equivalent = true; // grass grows underneath
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_column_square_top(f);
	f->type = CMT_STONE;
	f->dig_time = 1.0;
	f->onpunch_replace_node = CONTENT_SANDSTONE_COLUMN_CROSS;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_BRICK_COLUMN_SQUARE;
	f = &content_features(i);
	f->description = gettext("Brick Column");
	f->setAllTextures("brick.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_CLAY_BRICK)+" 4";
	f->air_equivalent = true; // grass grows underneath
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_column_square(f);
    f->setInventoryTextureNodeBox(i,"brick.png",
									"brick.png",
									"brick.png");
	f->type = CMT_STONE;
	f->dig_time = 1.0;
	f->onpunch_replace_node = CONTENT_BRICK_COLUMN_SQUARE_BASE;
	f->onpunch_replace_respects_borderstone = true;
	crafting::setCol3Recipe(CONTENT_BRICK,
							CONTENT_BRICK_COLUMN_SQUARE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_BRICK_COLUMN_SQUARE_BASE;
	f = &content_features(i);
	f->setAllTextures("brick.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_CLAY_BRICK)+" 4";
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_column_square_base(f);
	f->type = CMT_STONE;
	f->dig_time = 1.0;
	f->onpunch_replace_node = CONTENT_BRICK_COLUMN_SQUARE_TOP;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_BRICK_COLUMN_SQUARE_TOP;
	f = &content_features(i);
	f->setAllTextures("brick.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_CLAY_BRICK)+" 4";
	f->air_equivalent = true; // grass grows underneath
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_column_square_top(f);
	f->type = CMT_STONE;
	f->dig_time = 1.0;
	f->onpunch_replace_node = CONTENT_BRICK_COLUMN_CROSS;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_APPLEWOOD_COLUMN_SQUARE;
	f = &content_features(i);
	f->description = gettext("Apple wood Column");
	f->setAllTextures("applewood.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->air_equivalent = true; // grass grows underneath
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_column_square(f);
	f->setInventoryTextureNodeBox(i,"applewood.png","applewood.png","applewood.png");
	f->type = CMT_WOOD;
	f->dig_time = 0.75;
	f->onpunch_replace_node = CONTENT_APPLEWOOD_COLUMN_SQUARE_BASE;
	f->onpunch_replace_respects_borderstone = true;
	f->flammable = 1; // can be replaced by fire if the node under
					  // it is set on fire
	f->fuel_time = BT_COLUMN;
	f->cook_result = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_ASH)+" 1";
	crafting::setCol3Recipe(CONTENT_APPLEWOOD,CONTENT_APPLEWOOD_COLUMN_SQUARE);
	content_list_add("cooking",i,1,0);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_APPLEWOOD_COLUMN_SQUARE_BASE;
	f = &content_features(i);
	f->setAllTextures("applewood.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_APPLEWOOD_COLUMN_SQUARE)+" 1";
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_column_square_base(f);
	f->type = CMT_WOOD;
	f->dig_time = 0.75;
	f->flammable = 1; // can be replaced by fire if the node under
					  // it is set on fire
	f->onpunch_replace_node = CONTENT_APPLEWOOD_COLUMN_SQUARE_TOP;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_APPLEWOOD_COLUMN_SQUARE_TOP;
	f = &content_features(i);
	f->setAllTextures("applewood.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_APPLEWOOD_COLUMN_SQUARE)+" 1";
	f->air_equivalent = true; // grass grows underneath
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_column_square_top(f);
	f->type = CMT_WOOD;
	f->dig_time = 0.75;
	f->flammable = 1; // can be replaced by fire if the node under
					  // it is set on fire
	f->onpunch_replace_node = CONTENT_APPLEWOOD_COLUMN_CROSS;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_WOOD_COLUMN_SQUARE;
	f = &content_features(i);
	f->description = gettext("Wood Column");
	f->setAllTextures("wood.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->air_equivalent = true; // grass grows underneath
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_column_square(f);
	f->setInventoryTextureNodeBox(i,"wood.png","wood.png","wood.png");
	f->type = CMT_WOOD;
	f->dig_time = 0.75;
	f->onpunch_replace_node = CONTENT_WOOD_COLUMN_SQUARE_BASE;
	f->onpunch_replace_respects_borderstone = true;
	f->flammable = 1; // can be replaced by fire if the node under
					  // it is set on fire
	f->fuel_time = BT_COLUMN;
	f->cook_result = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_ASH)+" 1";
	crafting::setCol3Recipe(CONTENT_WOOD,CONTENT_WOOD_COLUMN_SQUARE);
	content_list_add("cooking",i,1,0);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_WOOD_COLUMN_SQUARE_BASE;
	f = &content_features(i);
	f->setAllTextures("wood.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_WOOD_COLUMN_SQUARE)+" 1";
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_column_square_base(f);
	f->type = CMT_WOOD;
	f->dig_time = 0.75;
	f->flammable = 1; // can be replaced by fire if the node under
					  // it is set on fire
	f->onpunch_replace_node = CONTENT_WOOD_COLUMN_SQUARE_TOP;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_WOOD_COLUMN_SQUARE_TOP;
	f = &content_features(i);
	f->setAllTextures("wood.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_WOOD_COLUMN_SQUARE)+" 1";
	f->air_equivalent = true; // grass grows underneath
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_column_square_top(f);
	f->type = CMT_WOOD;
	f->dig_time = 0.75;
	f->flammable = 1; // can be replaced by fire if the node under
					  // it is set on fire
	f->onpunch_replace_node = CONTENT_WOOD_COLUMN_CROSS;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_JUNGLEWOOD_COLUMN_SQUARE;
	f = &content_features(i);
	f->description = gettext("Jungle Wood Column");
	f->setAllTextures("junglewood.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->air_equivalent = true; // grass grows underneath
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_column_square(f);
	f->setInventoryTextureNodeBox(i,"junglewood.png","junglewood.png","junglewood.png");
	f->type = CMT_WOOD;
	f->dig_time = 0.75;
	f->onpunch_replace_node = CONTENT_JUNGLEWOOD_COLUMN_SQUARE_BASE;
	f->onpunch_replace_respects_borderstone = true;
	f->flammable = 1; // can be replaced by fire if the node under
					  // it is set on fire
	f->fuel_time = BT_COLUMN;
	f->cook_result = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_ASH)+" 1";
	crafting::setCol3Recipe(CONTENT_JUNGLEWOOD,CONTENT_JUNGLEWOOD_COLUMN_SQUARE);
	content_list_add("cooking",i,1,0);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_JUNGLEWOOD_COLUMN_SQUARE_BASE;
	f = &content_features(i);
	f->setAllTextures("junglewood.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_JUNGLEWOOD_COLUMN_SQUARE)+" 1";
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_column_square_base(f);
	f->type = CMT_WOOD;
	f->dig_time = 0.75;
	f->flammable = 1; // can be replaced by fire if the node under
					  // it is set on fire
	f->onpunch_replace_node = CONTENT_JUNGLEWOOD_COLUMN_SQUARE_TOP;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_JUNGLEWOOD_COLUMN_SQUARE_TOP;
	f = &content_features(i);
	f->setAllTextures("junglewood.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_JUNGLEWOOD_COLUMN_SQUARE)+" 1";
	f->air_equivalent = true; // grass grows underneath
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_column_square_top(f);
	f->type = CMT_WOOD;
	f->dig_time = 0.75;
	f->flammable = 1; // can be replaced by fire if the node under
					  // it is set on fire
	f->onpunch_replace_node = CONTENT_JUNGLEWOOD_COLUMN_CROSS;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_WOOD_PINE_COLUMN_SQUARE;
	f = &content_features(i);
	f->description = gettext("Pine Wood Column");
	f->setAllTextures("pine.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->air_equivalent = true; // grass grows underneath
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_column_square(f);
	f->setInventoryTextureNodeBox(i,"pine.png","pine.png","pine.png");
	f->type = CMT_WOOD;
	f->dig_time = 0.5;
	f->onpunch_replace_node = CONTENT_WOOD_PINE_COLUMN_SQUARE_BASE;
	f->onpunch_replace_respects_borderstone = true;
	f->flammable = 1; // can be replaced by fire if the node under
					  // it is set on fire
	f->fuel_time = BT_COLUMN;
	f->cook_result = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_ASH)+" 1";
	crafting::setCol3Recipe(CONTENT_WOOD_PINE,CONTENT_WOOD_PINE_COLUMN_SQUARE);
	content_list_add("cooking",i,1,0);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_WOOD_PINE_COLUMN_SQUARE_BASE;
	f = &content_features(i);
	f->setAllTextures("pine.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_WOOD_PINE_COLUMN_SQUARE)+" 1";
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_column_square_base(f);
	f->type = CMT_WOOD;
	f->dig_time = 0.5;
	f->flammable = 1; // can be replaced by fire if the node under
					  // it is set on fire
	f->onpunch_replace_node = CONTENT_WOOD_PINE_COLUMN_SQUARE_TOP;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_WOOD_PINE_COLUMN_SQUARE_TOP;
	f = &content_features(i);
	f->setAllTextures("pine.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_WOOD_PINE_COLUMN_SQUARE)+" 1";
	f->air_equivalent = true; // grass grows underneath
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_column_square_top(f);
	f->type = CMT_WOOD;
	f->dig_time = 0.5;
	f->flammable = 1; // can be replaced by fire if the node under
					  // it is set on fire
	f->onpunch_replace_node = CONTENT_WOOD_PINE_COLUMN_CROSS;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_ROUGHSTONE_COLUMN_SQUARE;
	f = &content_features(i);
	f->description = gettext("Rough Stone Column");
	f->setAllTextures("roughstone.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->air_equivalent = true; // grass grows underneath
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_column_square(f);
	f->setInventoryTextureNodeBox(i,"roughstone.png","roughstone.png","roughstone.png");
	f->type = CMT_STONE;
	f->dig_time = 0.9;
	f->onpunch_replace_node = CONTENT_ROUGHSTONE_COLUMN_SQUARE_BASE;
	f->onpunch_replace_respects_borderstone = true;
	crafting::setCol3Recipe(CONTENT_ROUGHSTONE,CONTENT_ROUGHSTONE_COLUMN_SQUARE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_ROUGHSTONE_COLUMN_SQUARE_BASE;
	f = &content_features(i);
	f->setAllTextures("roughstone.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_ROUGHSTONE_COLUMN_SQUARE)+" 1";
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_column_square_base(f);
	f->type = CMT_STONE;
	f->dig_time = 0.9;
	f->onpunch_replace_node = CONTENT_ROUGHSTONE_COLUMN_SQUARE_TOP;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_ROUGHSTONE_COLUMN_SQUARE_TOP;
	f = &content_features(i);
	f->setAllTextures("roughstone.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_ROUGHSTONE_COLUMN_SQUARE)+" 1";
	f->air_equivalent = true; // grass grows underneath
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_column_square_top(f);
	f->type = CMT_STONE;
	f->dig_time = 0.9;
	f->onpunch_replace_node = CONTENT_ROUGHSTONE_COLUMN_CROSS;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_STONE_COLUMN_CROSS;
	f = &content_features(i);
	f->setAllTextures("stone.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_STONE_COLUMN_SQUARE)+" 1";
	f->air_equivalent = true; // grass grows underneath
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_column_cross(f);
	f->type = CMT_STONE;
	f->dig_time = 1.0;
	f->onpunch_replace_node = CONTENT_STONE_COLUMN_CROSS_BASE;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_STONE_COLUMN_CROSS_BASE;
	f = &content_features(i);
	f->setAllTextures("stone.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_STONE_COLUMN_SQUARE)+" 1";
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_column_cross_base(f);
	f->type = CMT_STONE;
	f->dig_time = 1.0;
	f->onpunch_replace_node = CONTENT_STONE_COLUMN_CROSS_TOP;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_STONE_COLUMN_CROSS_TOP;
	f = &content_features(i);
	f->setAllTextures("stone.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_STONE_COLUMN_SQUARE)+" 1";
	f->air_equivalent = true; // grass grows underneath
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_column_cross_top(f);
	f->type = CMT_STONE;
	f->dig_time = 1.0;
	f->onpunch_replace_node = CONTENT_STONE_COLUMN_SQUARE;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_LIMESTONE_COLUMN_CROSS;
	f = &content_features(i);
	f->setAllTextures("limestone.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_LIMESTONE_COLUMN_SQUARE)+" 1";
	f->air_equivalent = true; // grass grows underneath
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_column_cross(f);
	f->type = CMT_STONE;
	f->dig_time = 0.6;
	f->onpunch_replace_node = CONTENT_LIMESTONE_COLUMN_CROSS_BASE;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_LIMESTONE_COLUMN_CROSS_BASE;
	f = &content_features(i);
	f->setAllTextures("limestone.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_LIMESTONE_COLUMN_SQUARE)+" 1";
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_column_cross_base(f);
	f->type = CMT_STONE;
	f->dig_time = 0.6;
	f->onpunch_replace_node = CONTENT_LIMESTONE_COLUMN_CROSS_TOP;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_LIMESTONE_COLUMN_CROSS_TOP;
	f = &content_features(i);
	f->setAllTextures("limestone.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_LIMESTONE_COLUMN_SQUARE)+" 1";
	f->air_equivalent = true; // grass grows underneath
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_column_cross_top(f);
	f->type = CMT_STONE;
	f->dig_time = 0.6;
	f->onpunch_replace_node = CONTENT_LIMESTONE_COLUMN_SQUARE;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_MARBLE_COLUMN_CROSS;
	f = &content_features(i);
	f->setAllTextures("marble.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_MARBLE_COLUMN_SQUARE)+" 1";
	f->air_equivalent = true; // grass grows underneath
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_column_cross(f);
	f->type = CMT_STONE;
	f->dig_time = 0.6;
	f->onpunch_replace_node = CONTENT_MARBLE_COLUMN_CROSS_BASE;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_MARBLE_COLUMN_CROSS_BASE;
	f = &content_features(i);
	f->setAllTextures("marble.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_MARBLE_COLUMN_SQUARE)+" 1";
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_column_cross_base(f);
	f->type = CMT_STONE;
	f->dig_time = 0.6;
	f->onpunch_replace_node = CONTENT_MARBLE_COLUMN_CROSS_TOP;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_MARBLE_COLUMN_CROSS_TOP;
	f = &content_features(i);
	f->setAllTextures("marble.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_MARBLE_COLUMN_SQUARE)+" 1";
	f->air_equivalent = true; // grass grows underneath
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_column_cross_top(f);
	f->type = CMT_STONE;
	f->dig_time = 0.6;
	f->onpunch_replace_node = CONTENT_MARBLE_COLUMN_SQUARE;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_SANDSTONE_COLUMN_CROSS;
	f = &content_features(i);
	f->setAllTextures("sandstone.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_SAND)+" 4";
	f->air_equivalent = true; // grass grows underneath
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_column_cross(f);
	f->type = CMT_STONE;
	f->dig_time = 1.0;
	f->onpunch_replace_node = CONTENT_SANDSTONE_COLUMN_CROSS_BASE;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_SANDSTONE_COLUMN_CROSS_BASE;
	f = &content_features(i);
	f->setAllTextures("sandstone.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_SAND)+" 4";
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_column_cross_base(f);
	f->type = CMT_STONE;
	f->dig_time = 1.0;
	f->onpunch_replace_node = CONTENT_SANDSTONE_COLUMN_CROSS_TOP;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_SANDSTONE_COLUMN_CROSS_TOP;
	f = &content_features(i);
	f->setAllTextures("sandstone.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_SAND)+" 4";
	f->air_equivalent = true; // grass grows underneath
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_column_cross_top(f);
	f->type = CMT_STONE;
	f->dig_time = 1.0;
	f->onpunch_replace_node = CONTENT_SANDSTONE_COLUMN_SQUARE;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_BRICK_COLUMN_CROSS;
	f = &content_features(i);
	f->setAllTextures("brick.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_CLAY_BRICK)+" 4";
	f->air_equivalent = true; // grass grows underneath
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_column_cross(f);
	f->type = CMT_STONE;
	f->dig_time = 1.0;
	f->onpunch_replace_node = CONTENT_BRICK_COLUMN_CROSS_BASE;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_BRICK_COLUMN_CROSS_BASE;
	f = &content_features(i);
	f->setAllTextures("brick.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_CLAY_BRICK)+" 4";
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_column_cross_base(f);
	f->type = CMT_STONE;
	f->dig_time = 1.0;
	f->onpunch_replace_node = CONTENT_BRICK_COLUMN_CROSS_TOP;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_BRICK_COLUMN_CROSS_TOP;
	f = &content_features(i);
	f->setAllTextures("brick.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_CLAY_BRICK)+" 4";
	f->air_equivalent = true; // grass grows underneath
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_column_cross_top(f);
	f->type = CMT_STONE;
	f->dig_time = 1.0;
	f->onpunch_replace_node = CONTENT_BRICK_COLUMN_SQUARE;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_APPLEWOOD_COLUMN_CROSS;
	f = &content_features(i);
	f->setAllTextures("applewood.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_APPLEWOOD_COLUMN_SQUARE)+" 1";
	f->air_equivalent = true; // grass grows underneath
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_column_cross(f);
	f->type = CMT_WOOD;
	f->dig_time = 0.75;
	f->flammable = 1; // can be replaced by fire if the node under
					  // it is set on fire
	f->onpunch_replace_node = CONTENT_APPLEWOOD_COLUMN_CROSS_BASE;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_APPLEWOOD_COLUMN_CROSS_BASE;
	f = &content_features(i);
	f->setAllTextures("applewood.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_APPLEWOOD_COLUMN_SQUARE)+" 1";
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_column_cross_base(f);
	f->type = CMT_WOOD;
	f->dig_time = 0.75;
	f->flammable = 1; // can be replaced by fire if the node under
					  // it is set on fire
	f->onpunch_replace_node = CONTENT_APPLEWOOD_COLUMN_CROSS_TOP;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_APPLEWOOD_COLUMN_CROSS_TOP;
	f = &content_features(i);
	f->setAllTextures("applewood.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_APPLEWOOD_COLUMN_SQUARE)+" 1";
	f->air_equivalent = true; // grass grows underneath
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_column_cross_top(f);
	f->type = CMT_WOOD;
	f->dig_time = 0.75;
	f->flammable = 1; // can be replaced by fire if the node under
					  // it is set on fire
	f->onpunch_replace_node = CONTENT_APPLEWOOD_COLUMN_SQUARE;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_WOOD_COLUMN_CROSS;
	f = &content_features(i);
	f->setAllTextures("wood.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_WOOD_COLUMN_SQUARE)+" 1";
	f->air_equivalent = true; // grass grows underneath
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_column_cross(f);
	f->type = CMT_WOOD;
	f->dig_time = 0.75;
	f->flammable = 1; // can be replaced by fire if the node under
					  // it is set on fire
	f->onpunch_replace_node = CONTENT_WOOD_COLUMN_CROSS_BASE;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_WOOD_COLUMN_CROSS_BASE;
	f = &content_features(i);
	f->setAllTextures("wood.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_WOOD_COLUMN_SQUARE)+" 1";
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_column_cross_base(f);
	f->type = CMT_WOOD;
	f->dig_time = 0.75;
	f->flammable = 1; // can be replaced by fire if the node under
					  // it is set on fire
	f->onpunch_replace_node = CONTENT_WOOD_COLUMN_CROSS_TOP;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_WOOD_COLUMN_CROSS_TOP;
	f = &content_features(i);
	f->setAllTextures("wood.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_WOOD_COLUMN_SQUARE)+" 1";
	f->air_equivalent = true; // grass grows underneath
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_column_cross_top(f);
	f->type = CMT_WOOD;
	f->dig_time = 0.75;
	f->flammable = 1; // can be replaced by fire if the node under
					  // it is set on fire
	f->onpunch_replace_node = CONTENT_WOOD_COLUMN_SQUARE;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_JUNGLEWOOD_COLUMN_CROSS;
	f = &content_features(i);
	f->setAllTextures("junglewood.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_JUNGLEWOOD_COLUMN_SQUARE)+" 1";
	f->air_equivalent = true; // grass grows underneath
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_column_cross(f);
	f->type = CMT_WOOD;
	f->dig_time = 0.75;
	f->flammable = 1; // can be replaced by fire if the node under
					  // it is set on fire
	f->onpunch_replace_node = CONTENT_JUNGLEWOOD_COLUMN_CROSS_BASE;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_JUNGLEWOOD_COLUMN_CROSS_BASE;
	f = &content_features(i);
	f->setAllTextures("junglewood.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_JUNGLEWOOD_COLUMN_SQUARE)+" 1";
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_column_cross_base(f);
	f->type = CMT_WOOD;
	f->dig_time = 0.75;
	f->flammable = 1; // can be replaced by fire if the node under
					  // it is set on fire
	f->onpunch_replace_node = CONTENT_JUNGLEWOOD_COLUMN_CROSS_TOP;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_JUNGLEWOOD_COLUMN_CROSS_TOP;
	f = &content_features(i);
	f->setAllTextures("junglewood.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_JUNGLEWOOD_COLUMN_SQUARE)+" 1";
	f->air_equivalent = true; // grass grows underneath
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_column_cross_top(f);
	f->type = CMT_WOOD;
	f->dig_time = 0.75;
	f->flammable = 1; // can be replaced by fire if the node under
					  // it is set on fire
	f->onpunch_replace_node = CONTENT_JUNGLEWOOD_COLUMN_SQUARE;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_WOOD_PINE_COLUMN_CROSS;
	f = &content_features(i);
	f->setAllTextures("pine.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_WOOD_PINE_COLUMN_SQUARE)+" 1";
	f->air_equivalent = true; // grass grows underneath
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_column_cross(f);
	f->type = CMT_WOOD;
	f->dig_time = 0.5;
	f->flammable = 1; // can be replaced by fire if the node under
					  // it is set on fire
	f->onpunch_replace_node = CONTENT_WOOD_PINE_COLUMN_CROSS_BASE;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_WOOD_PINE_COLUMN_CROSS_BASE;
	f = &content_features(i);
	f->setAllTextures("pine.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_WOOD_PINE_COLUMN_SQUARE)+" 1";
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_column_cross_base(f);
	f->type = CMT_WOOD;
	f->dig_time = 0.5;
	f->flammable = 1; // can be replaced by fire if the node under
					  // it is set on fire
	f->onpunch_replace_node = CONTENT_WOOD_PINE_COLUMN_CROSS_TOP;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_WOOD_PINE_COLUMN_CROSS_TOP;
	f = &content_features(i);
	f->setAllTextures("pine.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_WOOD_PINE_COLUMN_SQUARE)+" 1";
	f->air_equivalent = true; // grass grows underneath
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_column_cross_top(f);
	f->type = CMT_WOOD;
	f->dig_time = 0.5;
	f->flammable = 1; // can be replaced by fire if the node under
					  // it is set on fire
	f->onpunch_replace_node = CONTENT_WOOD_PINE_COLUMN_SQUARE;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_ROUGHSTONE_COLUMN_CROSS;
	f = &content_features(i);
	f->setAllTextures("roughstone.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_ROUGHSTONE_COLUMN_SQUARE)+" 1";
	f->air_equivalent = true; // grass grows underneath
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_column_cross(f);
	f->type = CMT_STONE;
	f->dig_time = 0.9;
	f->onpunch_replace_node = CONTENT_ROUGHSTONE_COLUMN_CROSS_BASE;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_ROUGHSTONE_COLUMN_CROSS_BASE;
	f = &content_features(i);
	f->setAllTextures("roughstone.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_ROUGHSTONE_COLUMN_SQUARE)+" 1";
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_column_cross_base(f);
	f->type = CMT_STONE;
	f->dig_time = 0.9;
	f->onpunch_replace_node = CONTENT_ROUGHSTONE_COLUMN_CROSS_TOP;
	f->onpunch_replace_respects_borderstone = true;

	i = CONTENT_ROUGHSTONE_COLUMN_CROSS_TOP;
	f = &content_features(i);
	f->setAllTextures("roughstone.png");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_NODEBOX;
	f->is_ground_content = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_ROUGHSTONE_COLUMN_SQUARE)+" 1";
	f->air_equivalent = true; // grass grows underneath
	f->light_propagates = true;
	f->sunlight_propagates = true;
	content_nodebox_column_cross_top(f);
	f->type = CMT_STONE;
	f->dig_time = 0.9;
	f->onpunch_replace_node = CONTENT_ROUGHSTONE_COLUMN_SQUARE;
	f->onpunch_replace_respects_borderstone = true;
}
