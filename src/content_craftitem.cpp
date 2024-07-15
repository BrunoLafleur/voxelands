/************************************************************************
* Minetest-c55
* Copyright (C) 2010-2011 celeron55, Perttu Ahola <celeron55@gmail.com>
*
* content_craftitem.cpp
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

#include "content_craftitem.h"
#include "content_mapnode.h"
#include "content_craft.h"
#include "content_mob.h"
#include "content_list.h"
#include "intl.h"
#include "player.h"
#include "content_burntimes.h"

struct CraftItemFeatures g_content_craftitem_features[4096];

CraftItemFeatures *content_craftitem_features(content_t i)
{
	uint32_t j;
	if ((i&CONTENT_CRAFTITEM_MASK) != CONTENT_CRAFTITEM_MASK)
		return &g_content_craftitem_features[0];

	j = (i&~CONTENT_CRAFTITEM_MASK);
	if (j > 4095)
		return &g_content_craftitem_features[0];

	return &g_content_craftitem_features[j];
}

void content_craftitem_init()
{
	content_t i;
	CraftItemFeatures *f = NULL;
	int k;

	for (k=0; k<4096; k++) {
		g_content_craftitem_features[k].content = CONTENT_IGNORE;
		g_content_craftitem_features[k].texture = "unknown_item.png";
		g_content_craftitem_features[k].overlay_base = "";
		g_content_craftitem_features[k].description = (char*)"";
		g_content_craftitem_features[k].cook_result = CONTENT_IGNORE;
		g_content_craftitem_features[k].cook_type = COOK_ANY;
		g_content_craftitem_features[k].crush_result = CONTENT_IGNORE;
		g_content_craftitem_features[k].crush_type = CRUSH_ANY;
		g_content_craftitem_features[k].fuel_time = 0.0;
		g_content_craftitem_features[k].stackable = true;
		g_content_craftitem_features[k].consumable = false;
		g_content_craftitem_features[k].hunger_effect = 0;
		g_content_craftitem_features[k].health_effect = 0;
		g_content_craftitem_features[k].cold_effect = 0;
		g_content_craftitem_features[k].energy_effect = 0;
		g_content_craftitem_features[k].drop_count = -1;
		g_content_craftitem_features[k].teleports = -2;
		g_content_craftitem_features[k].param_type = CPT_NONE;
		g_content_craftitem_features[k].drop_item = CONTENT_IGNORE;
		g_content_craftitem_features[k].thrown_item = CONTENT_IGNORE;
		g_content_craftitem_features[k].shot_item = CONTENT_IGNORE;
		g_content_craftitem_features[k].onuse_replace_item = CONTENT_IGNORE;
		g_content_craftitem_features[k].enchanted_item = CONTENT_IGNORE;
		g_content_craftitem_features[k].sound_use = "";
	}

	i = CONTENT_CRAFTITEM_PAPER;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_PAPER;
	f->texture = "paper.png";
	f->description = gettext("Paper");
	crafting::setRow3Recipe(CONTENT_PAPYRUS,CONTENT_CRAFTITEM_PAPER);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_PADLOCK;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_PADLOCK;
	f->texture = "padlock.png";
	f->description = gettext("Padlock");
	crafting::set1over1Recipe(CONTENT_CRAFTITEM_IRON_INGOT,CONTENT_CRAFTITEM_COPPER_INGOT,CONTENT_CRAFTITEM_PADLOCK);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_CHARCOAL;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_CHARCOAL;
	f->texture = "lump_of_charcoal.png";
	f->description = gettext("Charcoal Lump");
	f->fuel_time = BT_COAL;
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_COAL;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_COAL;
	f->texture = "lump_of_coal.png";
	f->description = gettext("Coal Lump");
	f->fuel_time = BT_COAL;
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_IRON;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_IRON;
	f->texture = "lump_of_iron.png";
	f->description = gettext("Iron Ore");
	f->cook_result = CONTENT_CRAFTITEM_IRON_INGOT;
	f->cook_type = COOK_FURNACE|COOK_SMELTERY;
	content_list_add("creative",i,1,0);
	content_list_add("cooking",i,1,0);

	i = CONTENT_CRAFTITEM_CLAY;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_CLAY;
	f->texture = "lump_of_clay.png";
	f->description = gettext("Clay Lump");
	f->cook_result = CONTENT_CRAFTITEM_CLAY_BRICK;
	content_list_add("creative",i,1,0);
	content_list_add("cooking",i,1,0);

	i = CONTENT_CRAFTITEM_TIN;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_TIN;
	f->texture = "lump_of_tin.png";
	f->description = gettext("Tin Ore");
	f->cook_result = CONTENT_CRAFTITEM_TIN_INGOT;
	content_list_add("creative",i,1,0);
	content_list_add("cooking",i,1,0);

	i = CONTENT_CRAFTITEM_COPPER;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_COPPER;
	f->texture = "lump_of_copper.png";
	f->description = gettext("Copper Ore");
	f->cook_result = CONTENT_CRAFTITEM_COPPER_INGOT;
	content_list_add("creative",i,1,0);
	content_list_add("cooking",i,1,0);

	i = CONTENT_CRAFTITEM_SILVER;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_SILVER;
	f->texture = "lump_of_silver.png";
	f->description = gettext("Silver Ore");
	f->cook_result = CONTENT_CRAFTITEM_SILVER_INGOT;
	f->cook_type = COOK_FURNACE|COOK_SMELTERY;
	content_list_add("creative",i,1,0);
	content_list_add("cooking",i,1,0);

	i = CONTENT_CRAFTITEM_GOLD;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_GOLD;
	f->texture = "lump_of_gold.png";
	f->description = gettext("Gold Ore");
	f->cook_result = CONTENT_CRAFTITEM_GOLD_INGOT;
	f->cook_type = COOK_FURNACE|COOK_SMELTERY;
	content_list_add("creative",i,1,0);
	content_list_add("cooking",i,1,0);

	i = CONTENT_CRAFTITEM_QUARTZ;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_QUARTZ;
	f->texture = "lump_of_quartz.png";
	f->description = gettext("Quartz Crystal");
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_TIN_INGOT;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_TIN_INGOT;
	f->texture = "tin_ingot.png";
	f->description = gettext("Tin Ingot");
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_COPPER_INGOT;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_COPPER_INGOT;
	f->texture = "copper_ingot.png";
	f->description = gettext("Copper Ingot");
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_SILVER_INGOT;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_SILVER_INGOT;
	f->texture = "silver_ingot.png";
	f->description = gettext("Silver Ingot");
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_GOLD_INGOT;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_GOLD_INGOT;
	f->texture = "gold_ingot.png";
	f->description = gettext("Gold Ingot");
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_FLINT;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_FLINT;
	f->texture = "lump_of_flint.png";
	f->description = gettext("Flint");
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_IRON_INGOT;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_IRON_INGOT;
	f->texture = "iron_ingot.png";
	f->description = gettext("Iron Ingot");
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_BRONZE_INGOT;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_BRONZE_INGOT;
	f->texture = "bronze_ingot.png";
	f->description = gettext("Bronze Ingot");
	content_list_add("creative",i,1,0);
	{
		content_t r[2] = {CONTENT_CRAFTITEM_TIN,CONTENT_CRAFTITEM_COPPER};
		crafting::setAlloy(r,i,1);
		r[0] = CONTENT_CRAFTITEM_COPPER;
		r[1] = CONTENT_CRAFTITEM_TIN;
		crafting::setAlloy(r,i,1);
		r[0] = CONTENT_CRAFTITEM_TIN_INGOT;
		r[1] = CONTENT_CRAFTITEM_COPPER_INGOT;
		crafting::setAlloy(r,i,1);
		r[0] = CONTENT_CRAFTITEM_COPPER_INGOT;
		r[1] = CONTENT_CRAFTITEM_TIN_INGOT;
		crafting::setAlloy(r,i,1);
	}

	i = CONTENT_CRAFTITEM_CLAY_BRICK;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_CLAY_BRICK;
	f->texture = "clay_brick.png";
	f->description = gettext("Brick");
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_RAT;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_RAT;
	f->texture = "rat.png";
	f->description = gettext("Rat");
	f->cook_result = CONTENT_CRAFTITEM_COOKED_RAT;
	f->drop_count = 1;
	f->drop_item = CONTENT_MOB_RAT;
	content_list_add("creative",i,1,0);
	content_list_add("cooking",i,1,0);

	i = CONTENT_CRAFTITEM_COOKED_RAT;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_COOKED_RAT;
	f->texture = "cooked_rat.png";
	f->description = gettext("Cooked Rat");
	f->cook_result = CONTENT_CRAFTITEM_ASH;
	f->consumable = true;
	f->sound_use = "use-eat";
	f->hunger_effect = 40;
	f->health_effect = 20;
	content_list_add("creative",i,1,0);
	content_list_add("cooking",i,1,0);

	i = CONTENT_CRAFTITEM_FIREFLY;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_FIREFLY;
	f->texture = "firefly.png";
	f->description = gettext("Firefly");
	f->drop_count = 1;
	f->drop_item = CONTENT_MOB_FIREFLY;
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_APPLE;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_APPLE;
	f->texture = "apple.png^[forcesingle";
	f->description = gettext("Apple");
	f->consumable = true;
	f->sound_use = "use-eat";
	f->hunger_effect = 30;
	f->health_effect = 15;
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_APPLE_IRON;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_APPLE_IRON;
	f->texture = "apple_iron.png";
	f->description = gettext("Iron Apple");
	f->consumable = true;
	f->sound_use = "use-eat";
	f->hunger_effect = 80;
	f->health_effect = 40;
	{
		u16 recipe[9] = {
			CONTENT_CRAFTITEM_IRON_INGOT,		CONTENT_IGNORE,	CONTENT_CRAFTITEM_IRON_INGOT,
			CONTENT_IGNORE,	CONTENT_CRAFTITEM_APPLE,		CONTENT_IGNORE,
			CONTENT_CRAFTITEM_IRON_INGOT,		CONTENT_IGNORE,	CONTENT_CRAFTITEM_IRON_INGOT
		};
		crafting::setRecipe(recipe,CONTENT_CRAFTITEM_APPLE_IRON,1);
	}
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_DYE_BLUE;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_DYE_BLUE;
	f->texture = "dye_blue.png";
	f->description = gettext("Blue Dye");
	crafting::set1To2Recipe(CONTENT_FLOWER_TULIP,CONTENT_CRAFTITEM_DYE_BLUE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_DYE_GREEN;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_DYE_GREEN;
	f->texture = "dye_green.png";
	f->description = gettext("Green Dye");
	crafting::set2Any2Recipe(CONTENT_CRAFTITEM_DYE_BLUE,CONTENT_CRAFTITEM_DYE_YELLOW,CONTENT_CRAFTITEM_DYE_GREEN);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_DYE_ORANGE;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_DYE_ORANGE;
	f->texture = "dye_orange.png";
	f->description = gettext("Orange Dye");
	crafting::set2Any2Recipe(CONTENT_CRAFTITEM_DYE_RED,CONTENT_CRAFTITEM_DYE_YELLOW,CONTENT_CRAFTITEM_DYE_ORANGE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_DYE_PURPLE;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_DYE_PURPLE;
	f->texture = "dye_purple.png";
	f->description = gettext("Purple Dye");
	crafting::set2Any2Recipe(CONTENT_CRAFTITEM_DYE_BLUE,CONTENT_CRAFTITEM_DYE_RED,CONTENT_CRAFTITEM_DYE_PURPLE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_DYE_RED;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_DYE_RED;
	f->texture = "dye_red.png";
	f->description = gettext("Red Dye");
	crafting::set1To2Recipe(CONTENT_FLOWER_ROSE,CONTENT_CRAFTITEM_DYE_RED);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_DYE_YELLOW;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_DYE_YELLOW;
	f->texture = "dye_yellow.png";
	f->description = gettext("Yellow Dye");
	crafting::set1To2Recipe(CONTENT_CACTUS_BLOSSOM,CONTENT_CRAFTITEM_DYE_YELLOW);
	crafting::set1To2Recipe(CONTENT_FLOWER_DAFFODIL,CONTENT_CRAFTITEM_DYE_YELLOW);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_DYE_WHITE;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_DYE_WHITE;
	f->texture = "dye_white.png";
	f->description = gettext("White Dye");
	crafting::set1To1Recipe(CONTENT_CRAFTITEM_APPLE_BLOSSOM,CONTENT_CRAFTITEM_DYE_WHITE);
	crafting::set2Any3Recipe(CONTENT_CRAFTITEM_DYE_BLUE,CONTENT_CRAFTITEM_DYE_RED,CONTENT_CRAFTITEM_DYE_YELLOW,CONTENT_CRAFTITEM_DYE_WHITE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_DYE_BLACK;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_DYE_BLACK;
	f->texture = "dye_black.png";
	f->description = gettext("Black Dye");
	crafting::set2Any2Recipe(CONTENT_CRAFTITEM_DYE_WHITE,CONTENT_CRAFTITEM_COAL,CONTENT_CRAFTITEM_DYE_BLACK);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_QUARTZ_DUST;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_QUARTZ_DUST;
	f->texture = "quartz_dust.png";
	f->description = gettext("Quartz Dust");
	crafting::set1To2Recipe(CONTENT_CRAFTITEM_QUARTZ,CONTENT_CRAFTITEM_QUARTZ_DUST);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_SALTPETER;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_SALTPETER;
	f->texture = "saltpeter.png";
	f->description = gettext("Saltpeter");
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_GUNPOWDER;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_GUNPOWDER;
	f->texture = "gunpowder.png";
	f->description = gettext("Gun Powder");
	crafting::set1Any3Recipe(CONTENT_CRAFTITEM_CHARCOAL,CONTENT_CRAFTITEM_FLINT,CONTENT_CRAFTITEM_SALTPETER,CONTENT_CRAFTITEM_GUNPOWDER);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_SNOW_BALL;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_SNOW_BALL;
	f->texture = "snow_ball.png";
	f->description = gettext("Snow Ball");
	f->thrown_item = CONTENT_MOB_SNOWBALL;

	i = CONTENT_CRAFTITEM_STICK;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_STICK;
	f->texture = "stick.png";
	f->description = gettext("Stick");
	f->fuel_time = BT_STICK;
	crafting::set1To2Recipe(CONTENT_CRAFTITEM_PINE_PLANK,CONTENT_CRAFTITEM_STICK);
	crafting::set1To2Recipe(CONTENT_CRAFTITEM_WOOD_PLANK,CONTENT_CRAFTITEM_STICK);
	crafting::set1To2Recipe(CONTENT_CRAFTITEM_JUNGLE_PLANK,CONTENT_CRAFTITEM_STICK);
	crafting::set1To2Recipe(CONTENT_CRAFTITEM_APPLE_PLANK,CONTENT_CRAFTITEM_STICK);
	crafting::set1To2Recipe(CONTENT_LEAVES,CONTENT_CRAFTITEM_STICK);
	crafting::set1To2Recipe(CONTENT_LEAVES_AUTUMN,CONTENT_CRAFTITEM_STICK);
	crafting::set1To2Recipe(CONTENT_LEAVES_WINTER,CONTENT_CRAFTITEM_STICK);
	crafting::set1To2Recipe(CONTENT_LEAVES_SNOWY,CONTENT_CRAFTITEM_STICK);
	crafting::set1To2Recipe(CONTENT_APPLE_LEAVES,CONTENT_CRAFTITEM_STICK);
	crafting::set1To2Recipe(CONTENT_JUNGLELEAVES,CONTENT_CRAFTITEM_STICK);
	crafting::set1To2Recipe(CONTENT_CONIFER_LEAVES,CONTENT_CRAFTITEM_STICK);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_PINE_PLANK;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_PINE_PLANK;
	f->texture = "pine_plank.png";
	f->description = gettext("Pine Plank");
	f->fuel_time = BT_PLANK;
	crafting::set1To4Recipe(CONTENT_WOOD_PINE,CONTENT_CRAFTITEM_PINE_PLANK);
	crafting::set1To2Recipe(CONTENT_YOUNG_CONIFER_TREE,CONTENT_CRAFTITEM_PINE_PLANK);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_APPLE_PLANK;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_APPLE_PLANK;
	f->texture = "applewood_plank.png";
	f->description = gettext("Apple Wood Plank");
	f->fuel_time = BT_PLANK;
	crafting::set1To4Recipe(CONTENT_APPLEWOOD,CONTENT_CRAFTITEM_APPLE_PLANK);
	crafting::set1To2Recipe(CONTENT_YOUNG_APPLE_TREE,CONTENT_CRAFTITEM_APPLE_PLANK);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_WOOD_PLANK;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_WOOD_PLANK;
	f->texture = "wood_plank.png";
	f->description = gettext("Wood Plank");
	f->fuel_time = BT_PLANK;
	crafting::set1To4Recipe(CONTENT_WOOD,CONTENT_CRAFTITEM_WOOD_PLANK);
	crafting::set1To2Recipe(CONTENT_YOUNG_TREE,CONTENT_CRAFTITEM_WOOD_PLANK);
	crafting::set1To2Recipe(CONTENT_YOUNG_APPLE_TREE,CONTENT_CRAFTITEM_WOOD_PLANK);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_JUNGLE_PLANK;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_JUNGLE_PLANK;
	f->texture = "jungle_plank.png";
	f->description = gettext("Jungle Wood Plank");
	f->fuel_time = BT_PLANK;
	crafting::set1To4Recipe(CONTENT_JUNGLEWOOD,CONTENT_CRAFTITEM_JUNGLE_PLANK);
	crafting::set1To2Recipe(CONTENT_YOUNG_JUNGLETREE,CONTENT_CRAFTITEM_JUNGLE_PLANK);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_TNT;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_TNT;
	f->texture = "tnt_stick.png";
	f->description = gettext("TNT Stick");
	{
		u16 recipe[9] = {
			CONTENT_CRAFTITEM_PAPER,	CONTENT_CRAFTITEM_GUNPOWDER,	CONTENT_CRAFTITEM_PAPER,
			CONTENT_IGNORE,			CONTENT_IGNORE,			CONTENT_IGNORE,
			CONTENT_IGNORE,			CONTENT_IGNORE,			CONTENT_IGNORE
		};
		crafting::setRecipe(recipe,CONTENT_CRAFTITEM_TNT,1);
	}
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_ASH;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_ASH;
	f->texture = "lump_of_ash.png";
	f->description = gettext("Ash");
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_APPLE_BLOSSOM;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_APPLE_BLOSSOM;
	f->texture = "apple_blossom.png";
	f->description = gettext("Apple Blossoms");
	f->consumable = true;
	f->sound_use = "use-eat";
	f->hunger_effect = 10;
	f->health_effect = 5;
	f->fuel_time = BT_FLOWER;
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_CACTUS_FRUIT;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_CACTUS_FRUIT;
	f->texture = "cactus_fruit.png^[forcesingle";
	f->description = gettext("Cactus Berry");
	f->consumable = true;
	f->sound_use = "use-eat";
	f->hunger_effect = 30;
	f->health_effect = 15;
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_MUSH;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_MUSH;
	f->texture = "mush.png";
	f->cook_result = CONTENT_CRAFTITEM_ASH;
	f->description = gettext("Mush");
	f->consumable = true;
	f->sound_use = "use-eat";
	f->health_effect = -5;
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_PUMPKINSLICE;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_PUMPKINSLICE;
	f->texture = "pumpkin_slice.png";
	f->description = gettext("Sliced Pumpkin");
	f->consumable = true;
	f->sound_use = "use-eat";
	f->hunger_effect = 30;
	f->health_effect = 15;
	crafting::set1To2Recipe(CONTENT_FARM_PUMPKIN,CONTENT_CRAFTITEM_PUMPKINSLICE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_PUMPKIN_PIE_SLICE;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_PUMPKIN_PIE_SLICE;
	f->texture = "pumpkin_pie_slice.png";
	f->description = gettext("Pumpkin Pie Slice");
	f->consumable = true;
	f->sound_use = "use-eat";
	f->hunger_effect = 60;
	f->health_effect = 30;
	content_list_add("craftguide",i,1,0);

	i = CONTENT_CRAFTITEM_APPLE_PIE_SLICE;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_APPLE_PIE_SLICE;
	f->texture = "apple_pie_slice.png";
	f->description = gettext("Apple Pie Slice");
	f->consumable = true;
	f->sound_use = "use-eat";
	f->hunger_effect = 60;
	f->health_effect = 30;
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_MELONSLICE;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_MELONSLICE;
	f->texture = "melon_slice.png";
	f->description = gettext("Sliced Melon");
	f->consumable = true;
	f->sound_use = "use-eat";
	f->hunger_effect = 40;
	f->health_effect = 20;
	crafting::set1To2Recipe(CONTENT_FARM_MELON,CONTENT_CRAFTITEM_MELONSLICE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_WHEAT;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_WHEAT;
	f->texture = "harvested_wheat.png";
	f->description = gettext("Wheat");
	f->consumable = true;
	f->sound_use = "use-eat";
	f->hunger_effect = 10;
	f->health_effect = 5;
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_FLOUR;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_FLOUR;
	f->texture = "flour.png";
	f->description = gettext("Flour");
	f->consumable = true;
	f->sound_use = "use-eat";
	f->hunger_effect = 10;
	f->health_effect = 5;
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_WHEAT,CONTENT_CRAFTITEM_WHEAT,CONTENT_CRAFTITEM_FLOUR);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_DOUGH;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_DOUGH;
	f->texture = "dough.png";
	f->description = gettext("Dough");
	f->cook_result = CONTENT_CRAFTITEM_BREAD;
	f->consumable = true;
	f->sound_use = "use-eat";
	f->hunger_effect = 20;
	f->health_effect = 10;
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_FLOUR,CONTENT_CRAFTITEM_FLOUR,CONTENT_CRAFTITEM_DOUGH);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);
	content_list_add("cooking",i,1,0);

	i = CONTENT_CRAFTITEM_BREAD;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_BREAD;
	f->texture = "bread.png";
	f->description = gettext("Bread");
	f->consumable = true;
	f->sound_use = "use-eat";
	f->hunger_effect = 50;
	f->health_effect = 20;
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_POTATO;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_POTATO;
	f->texture = "harvested_potato.png";
	f->description = gettext("Potato");
	f->cook_result = CONTENT_CRAFTITEM_ROASTPOTATO;
	f->consumable = true;
	f->sound_use = "use-eat";
	f->hunger_effect = 30;
	f->health_effect = 15;
	content_list_add("creative",i,1,0);
	content_list_add("cooking",i,1,0);

	i = CONTENT_CRAFTITEM_STARCH;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_STARCH;
	f->texture = "potato_starch.png";
	f->description = gettext("Potato Starch");
	crafting::set2Any2Recipe(CONTENT_CRAFTITEM_POTATO,CONTENT_CRAFTITEM_POTATO,CONTENT_CRAFTITEM_STARCH);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_ROASTPOTATO;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_ROASTPOTATO;
	f->texture = "roast_potato.png";
	f->description = gettext("Roast Potato");
	f->consumable = true;
	f->sound_use = "use-eat";
	f->hunger_effect = 60;
	f->health_effect = 30;
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_CARROT;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_CARROT;
	f->texture = "harvested_carrot.png";
	f->description = gettext("Carrot");
	f->consumable = true;
	f->sound_use = "use-eat";
	f->hunger_effect = 30;
	f->health_effect = 20;
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_CARROT_CAKE_RAW;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_CARROT_CAKE_RAW;
	f->texture = "carrot_cake_raw.png";
	f->description = gettext("Raw Carrot Cake");
	f->cook_result = CONTENT_CRAFTITEM_CARROT_CAKE;
	f->consumable = true;
	f->sound_use = "use-eat";
	f->hunger_effect = 30;
	f->health_effect = 20;
	crafting::set1over1Recipe(CONTENT_CRAFTITEM_CARROT,CONTENT_CRAFTITEM_DOUGH,CONTENT_CRAFTITEM_CARROT_CAKE_RAW);
	content_list_add("craftguide",i,1,0);
	content_list_add("cooking",i,1,0);

	i = CONTENT_CRAFTITEM_CARROT_CAKE;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_CARROT_CAKE;
	f->texture = "carrot_cake.png";
	f->description = gettext("Carrot Cake");
	f->consumable = true;
	f->sound_use = "use-eat";
	f->hunger_effect = 60;
	f->health_effect = 30;
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_BEETROOT;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_BEETROOT;
	f->texture = "harvested_beetroot.png";
	f->description = gettext("Beetroot");
	f->consumable = true;
	f->sound_use = "use-eat";
	f->hunger_effect = 30;
	f->health_effect = 15;
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_GRAPE;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_GRAPE;
	f->texture = "harvested_grape.png";
	f->description = gettext("Bunch of Grapes");
	f->consumable = true;
	f->sound_use = "use-eat";
	f->hunger_effect = 30;
	f->health_effect = 20;
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_STRING;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_STRING;
	f->texture = "string.png";
	f->description = gettext("String");
	crafting::set1To4Recipe(CONTENT_COTTON,CONTENT_CRAFTITEM_STRING);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_MITHRILDUST;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_MITHRILDUST;
	f->texture = "mithril_dust.png";
	f->description = gettext("Mithril Dust");
	f->drop_count = 1;
	f->drop_item = CONTENT_CIRCUIT_MITHRILWIRE;
	crafting::set1To2Recipe(CONTENT_CRAFTITEM_MITHRIL_RAW,CONTENT_CRAFTITEM_MITHRILDUST);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_RESIN;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_RESIN;
	f->texture = "resin.png";
	f->description = gettext("Resin");
	f->drop_count = 1;
	content_list_add("cooking",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_OERKKI_DUST;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_OERKKI_DUST;
	f->texture = "oerkki_dust.png";
	f->description = gettext("Oerkki Dust");
	f->drop_count = 1;
	f->teleports = PLAYERFLAG_HOME; // teleports player to default home location
	{
		u16 recipe[9] = {
			CONTENT_CRAFTITEM_RUBY,		CONTENT_CRAFTITEM_TURQUOISE,	CONTENT_CRAFTITEM_AMETHYST,
			CONTENT_CRAFTITEM_SAPPHIRE,	CONTENT_CRAFTITEM_SUNSTONE,	CONTENT_CRAFTITEM_QUARTZ,
			CONTENT_IGNORE,			CONTENT_IGNORE,			CONTENT_IGNORE
		};
		crafting::setShapelessRecipe(recipe, i, 2);
	}
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_FISH;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_FISH;
	f->texture = "fish.png";
	f->description = gettext("Fish");
	f->cook_result = CONTENT_CRAFTITEM_COOKED_FISH;
	f->drop_count = 1;
	f->drop_item = CONTENT_MOB_FISH;
	f->consumable = true;
	f->sound_use = "use-eat";
	f->hunger_effect = 5;
	f->health_effect = -5;
	content_list_add("creative",i,1,0);
	content_list_add("cooking",i,1,0);

	i = CONTENT_CRAFTITEM_COOKED_FISH;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_COOKED_FISH;
	f->texture = "cooked_fish.png";
	f->description = gettext("Cooked Fish");
	f->cook_result = CONTENT_CRAFTITEM_ASH;
	f->consumable = true;
	f->sound_use = "use-eat";
	f->hunger_effect = 40;
	f->health_effect = 30;
	content_list_add("creative",i,1,0);
	content_list_add("cooking",i,1,0);

	i = CONTENT_CRAFTITEM_MEAT;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_MEAT;
	f->texture = "meat.png";
	f->description = gettext("Meat");
	f->cook_result = CONTENT_CRAFTITEM_COOKED_MEAT;
	f->consumable = true;
	f->sound_use = "use-eat";
	f->hunger_effect = 5;
	f->health_effect = -5;
	content_list_add("creative",i,1,0);
	content_list_add("cooking",i,1,0);

	i = CONTENT_CRAFTITEM_COOKED_MEAT;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_COOKED_MEAT;
	f->texture = "cooked_meat.png";
	f->description = gettext("Cooked Meat");
	f->cook_result = CONTENT_CRAFTITEM_ASH;
	f->consumable = true;
	f->sound_use = "use-eat";
	f->hunger_effect = 50;
	f->health_effect = 30;
	content_list_add("creative",i,1,0);
	content_list_add("cooking",i,1,0);

	i = CONTENT_CRAFTITEM_COTTON_SHEET;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_COTTON_SHEET;
	f->texture = "cotton_sheet.png";
	f->description = gettext("Cotton Sheet");
	crafting::setRow2Recipe(CONTENT_CRAFTITEM_STRING,CONTENT_CRAFTITEM_COTTON_SHEET);
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_STARCH,CONTENT_CRAFTITEM_COTTON_SHEET_BLUE,CONTENT_CRAFTITEM_COTTON_SHEET);
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_STARCH,CONTENT_CRAFTITEM_COTTON_SHEET_GREEN,CONTENT_CRAFTITEM_COTTON_SHEET);
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_STARCH,CONTENT_CRAFTITEM_COTTON_SHEET_ORANGE,CONTENT_CRAFTITEM_COTTON_SHEET);
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_STARCH,CONTENT_CRAFTITEM_COTTON_SHEET_PURPLE,CONTENT_CRAFTITEM_COTTON_SHEET);
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_STARCH,CONTENT_CRAFTITEM_COTTON_SHEET_RED,CONTENT_CRAFTITEM_COTTON_SHEET);
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_STARCH,CONTENT_CRAFTITEM_COTTON_SHEET_YELLOW,CONTENT_CRAFTITEM_COTTON_SHEET);
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_STARCH,CONTENT_CRAFTITEM_COTTON_SHEET_BLACK,CONTENT_CRAFTITEM_COTTON_SHEET);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_COTTON_SHEET_BLUE;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_COTTON_SHEET_BLUE;
	f->texture = "cotton_sheet_blue.png";
	f->description = gettext("Blue Cotton Sheet");
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_COTTON_SHEET,CONTENT_CRAFTITEM_DYE_BLUE,CONTENT_CRAFTITEM_COTTON_SHEET_BLUE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_COTTON_SHEET_GREEN;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_COTTON_SHEET_GREEN;
	f->texture = "cotton_sheet_green.png";
	f->description = gettext("Green Cotton Sheet");
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_COTTON_SHEET,CONTENT_CRAFTITEM_DYE_GREEN,CONTENT_CRAFTITEM_COTTON_SHEET_GREEN);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_COTTON_SHEET_ORANGE;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_COTTON_SHEET_ORANGE;
	f->texture = "cotton_sheet_orange.png";
	f->description = gettext("Orange Cotton Sheet");
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_COTTON_SHEET,CONTENT_CRAFTITEM_DYE_ORANGE,CONTENT_CRAFTITEM_COTTON_SHEET_ORANGE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_COTTON_SHEET_PURPLE;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_COTTON_SHEET_PURPLE;
	f->texture = "cotton_sheet_purple.png";
	f->description = gettext("Purple Cotton Sheet");
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_COTTON_SHEET,CONTENT_CRAFTITEM_DYE_PURPLE,CONTENT_CRAFTITEM_COTTON_SHEET_PURPLE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_COTTON_SHEET_RED;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_COTTON_SHEET_RED;
	f->texture = "cotton_sheet_red.png";
	f->description = gettext("Red Cotton Sheet");
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_COTTON_SHEET,CONTENT_CRAFTITEM_DYE_RED,CONTENT_CRAFTITEM_COTTON_SHEET_RED);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_COTTON_SHEET_YELLOW;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_COTTON_SHEET_YELLOW;
	f->texture = "cotton_sheet_yellow.png";
	f->description = gettext("Yellow Cotton Sheet");
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_COTTON_SHEET,CONTENT_CRAFTITEM_DYE_YELLOW,CONTENT_CRAFTITEM_COTTON_SHEET_YELLOW);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_COTTON_SHEET_BLACK;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_COTTON_SHEET_BLACK;
	f->texture = "cotton_sheet_black.png";
	f->description = gettext("Black Cotton Sheet");
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_COTTON_SHEET,CONTENT_CRAFTITEM_DYE_BLACK,CONTENT_CRAFTITEM_COTTON_SHEET_BLACK);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_CANVAS_SHEET;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_CANVAS_SHEET;
	f->texture = "canvas_sheet.png";
	f->description = gettext("Canvas Sheet");
	crafting::setCol2Recipe(CONTENT_CRAFTITEM_COTTON_SHEET,CONTENT_CRAFTITEM_CANVAS_SHEET);
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_STARCH,CONTENT_CRAFTITEM_CANVAS_SHEET_BLUE,CONTENT_CRAFTITEM_CANVAS_SHEET);
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_STARCH,CONTENT_CRAFTITEM_CANVAS_SHEET_GREEN,CONTENT_CRAFTITEM_CANVAS_SHEET);
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_STARCH,CONTENT_CRAFTITEM_CANVAS_SHEET_ORANGE,CONTENT_CRAFTITEM_CANVAS_SHEET);
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_STARCH,CONTENT_CRAFTITEM_CANVAS_SHEET_PURPLE,CONTENT_CRAFTITEM_CANVAS_SHEET);
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_STARCH,CONTENT_CRAFTITEM_CANVAS_SHEET_RED,CONTENT_CRAFTITEM_CANVAS_SHEET);
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_STARCH,CONTENT_CRAFTITEM_CANVAS_SHEET_YELLOW,CONTENT_CRAFTITEM_CANVAS_SHEET);
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_STARCH,CONTENT_CRAFTITEM_CANVAS_SHEET_BLACK,CONTENT_CRAFTITEM_CANVAS_SHEET);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_CANVAS_SHEET_BLUE;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_CANVAS_SHEET_BLUE;
	f->texture = "canvas_sheet_blue.png";
	f->description = gettext("Blue Canvas Sheet");
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_CANVAS_SHEET,CONTENT_CRAFTITEM_DYE_BLUE,CONTENT_CRAFTITEM_CANVAS_SHEET_BLUE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_CANVAS_SHEET_GREEN;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_CANVAS_SHEET_GREEN;
	f->texture = "canvas_sheet_green.png";
	f->description = gettext("Green Canvas Sheet");
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_CANVAS_SHEET,CONTENT_CRAFTITEM_DYE_GREEN,CONTENT_CRAFTITEM_CANVAS_SHEET_GREEN);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_CANVAS_SHEET_ORANGE;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_CANVAS_SHEET_ORANGE;
	f->texture = "canvas_sheet_orange.png";
	f->description = gettext("Orange Canvas Sheet");
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_CANVAS_SHEET,CONTENT_CRAFTITEM_DYE_ORANGE,CONTENT_CRAFTITEM_CANVAS_SHEET_ORANGE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_CANVAS_SHEET_PURPLE;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_CANVAS_SHEET_PURPLE;
	f->texture = "canvas_sheet_purple.png";
	f->description = gettext("Purple Canvas Sheet");
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_CANVAS_SHEET,CONTENT_CRAFTITEM_DYE_PURPLE,CONTENT_CRAFTITEM_CANVAS_SHEET_PURPLE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_CANVAS_SHEET_RED;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_CANVAS_SHEET_RED;
	f->texture = "canvas_sheet_red.png";
	f->description = gettext("Red Canvas Sheet");
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_CANVAS_SHEET,CONTENT_CRAFTITEM_DYE_RED,CONTENT_CRAFTITEM_CANVAS_SHEET_RED);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_CANVAS_SHEET_YELLOW;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_CANVAS_SHEET_YELLOW;
	f->texture = "canvas_sheet_yellow.png";
	f->description = gettext("Yellow Canvas Sheet");
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_CANVAS_SHEET,CONTENT_CRAFTITEM_DYE_YELLOW,CONTENT_CRAFTITEM_CANVAS_SHEET_YELLOW);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_CANVAS_SHEET_BLACK;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_CANVAS_SHEET_BLACK;
	f->texture = "canvas_sheet_black.png";
	f->description = gettext("Black Canvas Sheet");
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_CANVAS_SHEET,CONTENT_CRAFTITEM_DYE_BLACK,CONTENT_CRAFTITEM_CANVAS_SHEET_BLACK);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_FUR;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_FUR;
	f->texture = "fur.png";
	f->description = gettext("Fur");
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_FUR_WHITE;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_FUR_WHITE;
	f->texture = "fur_white.png";
	f->description = gettext("White Fur");
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_FUR,CONTENT_CRAFTITEM_DYE_WHITE,CONTENT_CRAFTITEM_FUR_WHITE);
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_FUR,CONTENT_CRAFTITEM_STARCH,CONTENT_CRAFTITEM_FUR_WHITE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_FUR_BLUE;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_FUR_BLUE;
	f->texture = "fur_blue.png";
	f->description = gettext("Blue Fur");
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_FUR_WHITE,CONTENT_CRAFTITEM_DYE_BLUE,CONTENT_CRAFTITEM_FUR_BLUE);
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_FUR,CONTENT_CRAFTITEM_DYE_BLUE,CONTENT_CRAFTITEM_FUR_BLUE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_FUR_GREEN;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_FUR_GREEN;
	f->texture = "fur_green.png";
	f->description = gettext("Green Fur");
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_FUR_WHITE,CONTENT_CRAFTITEM_DYE_GREEN,CONTENT_CRAFTITEM_FUR_GREEN);
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_FUR,CONTENT_CRAFTITEM_DYE_GREEN,CONTENT_CRAFTITEM_FUR_GREEN);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_FUR_ORANGE;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_FUR_ORANGE;
	f->texture = "fur_orange.png";
	f->description = gettext("Orange Fur");
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_FUR_WHITE,CONTENT_CRAFTITEM_DYE_ORANGE,CONTENT_CRAFTITEM_FUR_ORANGE);
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_FUR,CONTENT_CRAFTITEM_DYE_ORANGE,CONTENT_CRAFTITEM_FUR_ORANGE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_FUR_PURPLE;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_FUR_PURPLE;
	f->texture = "fur_purple.png";
	f->description = gettext("Purple Fur");
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_FUR_WHITE,CONTENT_CRAFTITEM_DYE_PURPLE,CONTENT_CRAFTITEM_FUR_PURPLE);
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_FUR,CONTENT_CRAFTITEM_DYE_PURPLE,CONTENT_CRAFTITEM_FUR_PURPLE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_FUR_RED;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_FUR_RED;
	f->texture = "fur_red.png";
	f->description = gettext("Red Fur");
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_FUR_WHITE,CONTENT_CRAFTITEM_DYE_RED,CONTENT_CRAFTITEM_FUR_RED);
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_FUR,CONTENT_CRAFTITEM_DYE_RED,CONTENT_CRAFTITEM_FUR_RED);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_FUR_YELLOW;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_FUR_YELLOW;
	f->texture = "fur_yellow.png";
	f->description = gettext("Yellow Fur");
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_FUR_WHITE,CONTENT_CRAFTITEM_DYE_YELLOW,CONTENT_CRAFTITEM_FUR_YELLOW);
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_FUR,CONTENT_CRAFTITEM_DYE_YELLOW,CONTENT_CRAFTITEM_FUR_YELLOW);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_FUR_BLACK;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_FUR_BLACK;
	f->texture = "fur_black.png";
	f->description = gettext("Black Fur");
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_FUR_WHITE,CONTENT_CRAFTITEM_DYE_BLACK,CONTENT_CRAFTITEM_FUR_BLACK);
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_FUR,CONTENT_CRAFTITEM_DYE_BLACK,CONTENT_CRAFTITEM_FUR_BLACK);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_LEATHER;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_LEATHER;
	f->texture = "leather.png";
	f->description = gettext("Leather");
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_FUR,CONTENT_CRAFTITEM_ASH,CONTENT_CRAFTITEM_LEATHER);
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_FUR,CONTENT_CRAFTITEM_MUSH,CONTENT_CRAFTITEM_LEATHER);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_LEATHER_WHITE;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_LEATHER_WHITE;
	f->texture = "leather_white.png";
	f->description = gettext("White Leather");
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_LEATHER,CONTENT_CRAFTITEM_DYE_WHITE,CONTENT_CRAFTITEM_LEATHER_WHITE);
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_LEATHER,CONTENT_CRAFTITEM_STARCH,CONTENT_CRAFTITEM_LEATHER_WHITE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_LEATHER_BLUE;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_LEATHER_BLUE;
	f->texture = "leather_blue.png";
	f->description = gettext("Blue Leather");
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_LEATHER_WHITE,CONTENT_CRAFTITEM_DYE_BLUE,CONTENT_CRAFTITEM_LEATHER_BLUE);
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_LEATHER,CONTENT_CRAFTITEM_DYE_BLUE,CONTENT_CRAFTITEM_LEATHER_BLUE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_LEATHER_GREEN;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_LEATHER_GREEN;
	f->texture = "leather_green.png";
	f->description = gettext("Green Leather");
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_LEATHER_WHITE,CONTENT_CRAFTITEM_DYE_GREEN,CONTENT_CRAFTITEM_LEATHER_GREEN);
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_LEATHER,CONTENT_CRAFTITEM_DYE_GREEN,CONTENT_CRAFTITEM_LEATHER_GREEN);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_LEATHER_ORANGE;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_LEATHER_ORANGE;
	f->texture = "leather_orange.png";
	f->description = gettext("Orange Leather");
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_LEATHER_WHITE,CONTENT_CRAFTITEM_DYE_ORANGE,CONTENT_CRAFTITEM_LEATHER_ORANGE);
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_LEATHER,CONTENT_CRAFTITEM_DYE_ORANGE,CONTENT_CRAFTITEM_LEATHER_ORANGE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_LEATHER_PURPLE;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_LEATHER_PURPLE;
	f->texture = "leather_purple.png";
	f->description = gettext("Purple Leather");
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_LEATHER_WHITE,CONTENT_CRAFTITEM_DYE_PURPLE,CONTENT_CRAFTITEM_LEATHER_PURPLE);
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_LEATHER,CONTENT_CRAFTITEM_DYE_PURPLE,CONTENT_CRAFTITEM_LEATHER_PURPLE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_LEATHER_RED;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_LEATHER_RED;
	f->texture = "leather_red.png";
	f->description = gettext("Red Leather");
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_LEATHER_WHITE,CONTENT_CRAFTITEM_DYE_RED,CONTENT_CRAFTITEM_LEATHER_RED);
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_LEATHER,CONTENT_CRAFTITEM_DYE_RED,CONTENT_CRAFTITEM_LEATHER_RED);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_LEATHER_YELLOW;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_LEATHER_YELLOW;
	f->texture = "leather_yellow.png";
	f->description = gettext("Yellow Leather");
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_LEATHER_WHITE,CONTENT_CRAFTITEM_DYE_YELLOW,CONTENT_CRAFTITEM_LEATHER_YELLOW);
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_LEATHER,CONTENT_CRAFTITEM_DYE_YELLOW,CONTENT_CRAFTITEM_LEATHER_YELLOW);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_LEATHER_BLACK;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_LEATHER_BLACK;
	f->texture = "leather_black.png";
	f->description = gettext("Black Leather");
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_LEATHER_WHITE,CONTENT_CRAFTITEM_DYE_BLACK,CONTENT_CRAFTITEM_LEATHER_BLACK);
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_LEATHER,CONTENT_CRAFTITEM_DYE_BLACK,CONTENT_CRAFTITEM_LEATHER_BLACK);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_ARROW;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_ARROW;
	f->texture = "arrow.png";
	f->description = gettext("Arrow");
	f->shot_item = CONTENT_MOB_ARROW;
	crafting::set1over4Recipe(CONTENT_CRAFTITEM_IRON_INGOT,CONTENT_CRAFTITEM_STICK,CONTENT_CRAFTITEM_ARROW);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_FERTILIZER;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_FERTILIZER;
	f->texture = "fertilizer_item.png";
	f->description = gettext("Fertilizer");
	f->drop_count = 1;
	f->drop_item = CONTENT_FERTILIZER;
	crafting::set2Any2Recipe(CONTENT_CRAFTITEM_MUSH,CONTENT_CRAFTITEM_ASH,CONTENT_CRAFTITEM_FERTILIZER);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_OERKKI_DUST_WHITE;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = i;
	f->texture = "oerkki_dust_white.png";
	f->description = gettext("White Oerkki Dust");
	f->drop_count = 1;
	f->teleports = PLAYERFLAG_WHITE;
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_OERKKI_DUST,CONTENT_CRAFTITEM_DYE_WHITE,CONTENT_CRAFTITEM_OERKKI_DUST_WHITE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_OERKKI_DUST_BLUE;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = i;
	f->texture = "oerkki_dust_blue.png";
	f->description = gettext("Blue Oerkki Dust");
	f->drop_count = 1;
	f->teleports = PLAYERFLAG_BLUE;
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_OERKKI_DUST,CONTENT_CRAFTITEM_DYE_BLUE,CONTENT_CRAFTITEM_OERKKI_DUST_BLUE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_OERKKI_DUST_GREEN;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = i;
	f->texture = "oerkki_dust_green.png";
	f->description = gettext("Green Oerkki Dust");
	f->drop_count = 1;
	f->teleports = PLAYERFLAG_GREEN;
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_OERKKI_DUST,CONTENT_CRAFTITEM_DYE_GREEN,CONTENT_CRAFTITEM_OERKKI_DUST_GREEN);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_OERKKI_DUST_ORANGE;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = i;
	f->texture = "oerkki_dust_orange.png";
	f->description = gettext("Orange Oerkki Dust");
	f->drop_count = 1;
	f->teleports = PLAYERFLAG_ORANGE;
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_OERKKI_DUST,CONTENT_CRAFTITEM_DYE_ORANGE,CONTENT_CRAFTITEM_OERKKI_DUST_ORANGE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_OERKKI_DUST_PURPLE;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = i;
	f->texture = "oerkki_dust_purple.png";
	f->description = gettext("Purple Oerkki Dust");
	f->drop_count = 1;
	f->teleports = PLAYERFLAG_PURPLE;
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_OERKKI_DUST,CONTENT_CRAFTITEM_DYE_PURPLE,CONTENT_CRAFTITEM_OERKKI_DUST_PURPLE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_OERKKI_DUST_RED;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = i;
	f->texture = "oerkki_dust_red.png";
	f->description = gettext("Red Oerkki Dust");
	f->drop_count = 1;
	f->teleports = PLAYERFLAG_RED;
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_OERKKI_DUST,CONTENT_CRAFTITEM_DYE_RED,CONTENT_CRAFTITEM_OERKKI_DUST_RED);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_OERKKI_DUST_YELLOW;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = i;
	f->texture = "oerkki_dust_yellow.png";
	f->description = gettext("Yellow Oerkki Dust");
	f->drop_count = 1;
	f->teleports = PLAYERFLAG_YELLOW;
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_OERKKI_DUST,CONTENT_CRAFTITEM_DYE_YELLOW,CONTENT_CRAFTITEM_OERKKI_DUST_YELLOW);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_OERKKI_DUST_BLACK;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = i;
	f->texture = "oerkki_dust_black.png";
	f->description = gettext("Black Oerkki Dust");
	f->drop_count = 1;
	f->teleports = PLAYERFLAG_BLACK;
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_OERKKI_DUST,CONTENT_CRAFTITEM_DYE_BLACK,CONTENT_CRAFTITEM_OERKKI_DUST_BLACK);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_GLASS_BOTTLE;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = i;
	f->texture = "glass_bottle.png";
	f->description = gettext("Glass Bottle");
	f->drop_count = 1;
	{
		u16 r[9] = {
			CONTENT_GLASS,	CONTENT_IGNORE,	CONTENT_GLASS,
			CONTENT_GLASS,	CONTENT_IGNORE,	CONTENT_GLASS,
			CONTENT_IGNORE,	CONTENT_GLASS,	CONTENT_IGNORE
		};
		crafting::setRecipe(r,CONTENT_CRAFTITEM_GLASS_BOTTLE,5);
	}
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_GRAPE_JUICE;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = i;
	f->texture = "drink_grape.png^glass_bottle.png";
	f->description = gettext("Grape Juice");
	f->stackable = false;
	f->drop_count = 1;
	f->consumable = true;
	f->sound_use = "use-drink";
	f->hunger_effect = 15;
	f->health_effect = 15;
	f->energy_effect = 30;
	f->onuse_replace_item = CONTENT_CRAFTITEM_GLASS_BOTTLE;
	crafting::set1over1Recipe(CONTENT_CRAFTITEM_GRAPE,CONTENT_CRAFTITEM_GLASS_BOTTLE,CONTENT_CRAFTITEM_GRAPE_JUICE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_APPLE_JUICE;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = i;
	f->texture = "drink_apple.png^glass_bottle.png";
	f->description = gettext("Apple Juice");
	f->stackable = false;
	f->drop_count = 1;
	f->consumable = true;
	f->sound_use = "use-drink";
	f->hunger_effect = 10;
	f->health_effect = 15;
	f->energy_effect = 10;
	f->onuse_replace_item = CONTENT_CRAFTITEM_GLASS_BOTTLE;
	crafting::set1over1Recipe(CONTENT_CRAFTITEM_APPLE,CONTENT_CRAFTITEM_GLASS_BOTTLE,CONTENT_CRAFTITEM_APPLE_JUICE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_TEA_LEAVES;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = i;
	f->texture = "tea_leaves.png";
	f->description = gettext("Tea Leaves");
	f->drop_count = 1;
	f->consumable = true;
	f->sound_use = "use-eat";
	f->hunger_effect = 5;
	f->health_effect = 10;
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_TEA;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = i;
	f->texture = "iron_bottle.png^tea_bag.png";
	f->description = gettext("Tea");
	f->stackable = false;
	f->drop_count = 1;
	f->consumable = true;
	f->sound_use = "use-drink";
	f->hunger_effect = 15;
	f->health_effect = 20;
	f->cold_effect = 300;
	f->energy_effect = 10;
	f->onuse_replace_item = CONTENT_CRAFTITEM_IRON_BOTTLE;
	crafting::set1over1Recipe(CONTENT_CRAFTITEM_TEA_LEAVES,CONTENT_CRAFTITEM_IRON_BOTTLE_WATER,CONTENT_CRAFTITEM_TEA);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_COFFEE_BEANS;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = i;
	f->texture = "coffee_beans.png";
	f->description = gettext("Roasted Coffee Beans");
	f->drop_count = 1;
	f->consumable = true;
	f->sound_use = "use-eat";
	f->hunger_effect = 10;
	f->health_effect = 15;
	f->energy_effect = 30;
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_COFFEE;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = i;
	f->texture = "iron_bottle.png^coffee_bean.png";
	f->description = gettext("Coffee");
	f->stackable = false;
	f->drop_count = 1;
	f->consumable = true;
	f->sound_use = "use-drink";
	f->hunger_effect = 10;
	f->health_effect = 15;
	f->cold_effect = 10;
	f->energy_effect = 300;
	f->onuse_replace_item = CONTENT_CRAFTITEM_IRON_BOTTLE;
	crafting::set1over1Recipe(CONTENT_CRAFTITEM_COFFEE_BEANS,CONTENT_CRAFTITEM_IRON_BOTTLE_WATER,CONTENT_CRAFTITEM_COFFEE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_IRON_BOTTLE;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = i;
	f->texture = "iron_bottle.png";
	f->description = gettext("Iron Bottle");
	f->drop_count = 1;
	{
		u16 r[9] = {
			CONTENT_CRAFTITEM_IRON_INGOT,	CONTENT_IGNORE,			CONTENT_CRAFTITEM_IRON_INGOT,
			CONTENT_CRAFTITEM_IRON_INGOT,	CONTENT_IGNORE,			CONTENT_CRAFTITEM_IRON_INGOT,
			CONTENT_IGNORE,			CONTENT_CRAFTITEM_IRON_INGOT,	CONTENT_IGNORE
		};
		crafting::setRecipe(r,CONTENT_CRAFTITEM_IRON_BOTTLE,5);
	}
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_GLASS_BOTTLE_WATER;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = i;
	f->texture = "drink_water.png^glass_bottle.png";
	f->description = gettext("Bottle of Water");
	f->stackable = false;
	f->drop_count = 1;
	f->consumable = true;
	f->sound_use = "use-drink";
	f->hunger_effect = 5;
	f->health_effect = 5;
	f->onuse_replace_item = CONTENT_CRAFTITEM_GLASS_BOTTLE;
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_IRON_BOTTLE_WATER;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = i;
	f->texture = "iron_bottle.png^water_droplet.png";
	f->description = gettext("Bottle of Hot Water");
	f->stackable = false;
	f->drop_count = 1;
	f->consumable = true;
	f->sound_use = "use-drink";
	f->hunger_effect = 5;
	f->health_effect = 8;
	f->cold_effect = 5;
	f->onuse_replace_item = CONTENT_CRAFTITEM_IRON_BOTTLE;
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_MITHRIL_RAW;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_MITHRIL_RAW;
	f->texture = "mithril_raw.png";
	f->description = gettext("Raw Mithril");
	f->cook_result = CONTENT_CRAFTITEM_MITHRIL_UNBOUND;
	f->cook_type = COOK_SMELTERY;
	content_list_add("creative",i,1,0);
	content_list_add("cooking",i,1,0);

	i = CONTENT_CRAFTITEM_MITHRIL_UNBOUND;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_MITHRIL_UNBOUND;
	f->texture = "mithril_unbound.png";
	f->enchanted_item = CONTENT_CRAFTITEM_MITHRIL;
	f->description = gettext("Unbound Mithril");
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_MITHRIL;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_MITHRIL;
	f->texture = "mithril_unbound.png";
	f->param_type = CPT_ENCHANTMENT;
	f->description = gettext("Mithril");
	f->overlay_base = "ingot_overlay";
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_RUBY;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = i;
	f->texture = "lump_of_ruby.png";
	f->description = gettext("Ruby");
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_TURQUOISE;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = i;
	f->texture = "lump_of_turquoise.png";
	f->description = gettext("Turquiose");
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_AMETHYST;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = i;
	f->texture = "lump_of_amethyst.png";
	f->description = gettext("Amethyst");
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_SAPPHIRE;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = i;
	f->texture = "lump_of_sapphire.png";
	f->description = gettext("Sapphire");
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_SUNSTONE;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = i;
	f->texture = "lump_of_sunstone.png";
	f->description = gettext("Sunstone");
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_SALT;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = i;
	f->texture = "salt.png";
	f->description = gettext("Salt");
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_OERKKI_DUST_SPACE;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_OERKKI_DUST_SPACE;
	f->texture = "oerkki_dust_space.png";
	f->description = gettext("Space Dust");
	f->drop_count = 1;
	f->teleports = PLAYERFLAG_JUMP; // teleports player up 1500
	{
		u16 r[9] = {
			CONTENT_IGNORE,			CONTENT_CRAFTITEM_OERKKI_DUST,	CONTENT_IGNORE,
			CONTENT_CRAFTITEM_OERKKI_DUST,	CONTENT_CRAFTITEM_MITHRILDUST,	CONTENT_CRAFTITEM_OERKKI_DUST,
			CONTENT_IGNORE,			CONTENT_CRAFTITEM_OERKKI_DUST,	CONTENT_IGNORE
		};
		crafting::setRecipe(r,CONTENT_CRAFTITEM_OERKKI_DUST_SPACE,1);
	}
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_BLUEBERRY;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_BLUEBERRY;
	f->texture = "harvested_blueberry.png";
	f->description = gettext("Blueberries");
	f->consumable = true;
	f->sound_use = "use-eat";
	f->hunger_effect = 10;
	f->health_effect = 10;
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_RASPBERRY;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_RASPBERRY;
	f->texture = "harvested_raspberry.png";
	f->description = gettext("Raspberries");
	f->consumable = true;
	f->sound_use = "use-eat";
	f->hunger_effect = 10;
	f->health_effect = 10;
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_UPGRADE_STORAGE;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_UPGRADE_STORAGE;
	f->texture = "upgrade_storage.png";
	f->description = gettext("Storage Upgrade");
	crafting::set1To1Recipe(CONTENT_CHEST,CONTENT_CRAFTITEM_UPGRADE_STORAGE);
	crafting::set1To1Recipe(CONTENT_CHEST_APPLE,CONTENT_CRAFTITEM_UPGRADE_STORAGE);
	crafting::set1To1Recipe(CONTENT_CHEST_JUNGLE,CONTENT_CRAFTITEM_UPGRADE_STORAGE);
	crafting::set1To1Recipe(CONTENT_CHEST_PINE,CONTENT_CRAFTITEM_UPGRADE_STORAGE);
	{
		u16 r[9] = {
			CONTENT_IGNORE,			CONTENT_CRAFTITEM_PAPER,	CONTENT_IGNORE,
			CONTENT_CRAFTITEM_PAPER,	CONTENT_CHEST,			CONTENT_CRAFTITEM_PAPER,
			CONTENT_IGNORE,			CONTENT_CRAFTITEM_PAPER,	CONTENT_IGNORE
		};
		/* why 3? because I couldn't decide between 2 and 4 */
		crafting::setRecipe(r,CONTENT_CRAFTITEM_UPGRADE_STORAGE,3);
	}
	{
		u16 r[9] = {
			CONTENT_IGNORE,			CONTENT_CRAFTITEM_PAPER,	CONTENT_IGNORE,
			CONTENT_CRAFTITEM_PAPER,	CONTENT_CHEST_APPLE,			CONTENT_CRAFTITEM_PAPER,
			CONTENT_IGNORE,			CONTENT_CRAFTITEM_PAPER,	CONTENT_IGNORE
		};
		crafting::setRecipe(r,CONTENT_CRAFTITEM_UPGRADE_STORAGE,3);
	}
	{
		u16 r[9] = {
			CONTENT_IGNORE,			CONTENT_CRAFTITEM_PAPER,	CONTENT_IGNORE,
			CONTENT_CRAFTITEM_PAPER,	CONTENT_CHEST_JUNGLE,			CONTENT_CRAFTITEM_PAPER,
			CONTENT_IGNORE,			CONTENT_CRAFTITEM_PAPER,	CONTENT_IGNORE
		};
		crafting::setRecipe(r,CONTENT_CRAFTITEM_UPGRADE_STORAGE,3);
	}
	{
		u16 r[9] = {
			CONTENT_IGNORE,			CONTENT_CRAFTITEM_PAPER,	CONTENT_IGNORE,
			CONTENT_CRAFTITEM_PAPER,	CONTENT_CHEST_PINE,			CONTENT_CRAFTITEM_PAPER,
			CONTENT_IGNORE,			CONTENT_CRAFTITEM_PAPER,	CONTENT_IGNORE
		};
		crafting::setRecipe(r,CONTENT_CRAFTITEM_UPGRADE_STORAGE,3);
	}
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_UPGRADE_EXO;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_UPGRADE_EXO;
	f->texture = "upgrade_exo.png";
	f->description = gettext("Exo Upgrade");
	crafting::set1To1Recipe(CONTENT_CRAFTITEM_OERKKI_DUST,CONTENT_CRAFTITEM_UPGRADE_EXO);
	{
		u16 r[9] = {
			CONTENT_IGNORE,			CONTENT_CRAFTITEM_PAPER,	CONTENT_IGNORE,
			CONTENT_CRAFTITEM_PAPER,	CONTENT_CRAFTITEM_OERKKI_DUST,	CONTENT_CRAFTITEM_PAPER,
			CONTENT_IGNORE,			CONTENT_CRAFTITEM_PAPER,	CONTENT_IGNORE
		};
		crafting::setRecipe(r,CONTENT_CRAFTITEM_UPGRADE_EXO,3);
	}
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_UPGRADE_COOKING;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_UPGRADE_COOKING;
	f->texture = "upgrade_cook.png";
	f->description = gettext("Cooking Upgrade");
	{
		u16 r[9] = {
			CONTENT_CRAFTITEM_PAPER,	CONTENT_COAL,	CONTENT_CRAFTITEM_PAPER,
			CONTENT_COAL,			CONTENT_IGNORE,	CONTENT_COAL,
			CONTENT_CRAFTITEM_PAPER,	CONTENT_COAL,	CONTENT_CRAFTITEM_PAPER
		};
		crafting::setRecipe(r,CONTENT_CRAFTITEM_UPGRADE_COOKING,1);
	}
	{
		u16 r[9] = {
			CONTENT_CRAFTITEM_PAPER,	CONTENT_CHARCOAL,	CONTENT_CRAFTITEM_PAPER,
			CONTENT_CHARCOAL,		CONTENT_IGNORE,		CONTENT_CHARCOAL,
			CONTENT_CRAFTITEM_PAPER,	CONTENT_CHARCOAL,	CONTENT_CRAFTITEM_PAPER
		};
		crafting::setRecipe(r,CONTENT_CRAFTITEM_UPGRADE_COOKING,1);
	}
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_UPGRADE_BURNING;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = CONTENT_CRAFTITEM_UPGRADE_BURNING;
	f->texture = "upgrade_burn.png";
	f->description = gettext("Fuel Upgrade");
	{
		u16 r[9] = {
			CONTENT_COAL,			CONTENT_CRAFTITEM_PAPER,	CONTENT_COAL,
			CONTENT_CRAFTITEM_PAPER,	CONTENT_IGNORE,			CONTENT_CRAFTITEM_PAPER,
			CONTENT_COAL,			CONTENT_CRAFTITEM_PAPER,	CONTENT_COAL
		};
		crafting::setRecipe(r,CONTENT_CRAFTITEM_UPGRADE_BURNING,1);
	}
	{
		u16 r[9] = {
			CONTENT_CHARCOAL,		CONTENT_CRAFTITEM_PAPER,	CONTENT_CHARCOAL,
			CONTENT_CRAFTITEM_PAPER,	CONTENT_IGNORE,			CONTENT_CRAFTITEM_PAPER,
			CONTENT_CHARCOAL,		CONTENT_CRAFTITEM_PAPER,	CONTENT_CHARCOAL
		};
		crafting::setRecipe(r,CONTENT_CRAFTITEM_UPGRADE_BURNING,1);
	}
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_CRAFTITEM_STRAW;
	f = &g_content_craftitem_features[(i&~CONTENT_CRAFTITEM_MASK)];
	f->content = i;
	f->texture = "straw.png";
	f->description = gettext("Straw");
	content_list_add("creative",i,1,0);

}
