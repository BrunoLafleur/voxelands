/************************************************************************
* content_toolitem.cpp
* voxelands - 3d voxel world sandbox game
* Copyright (C) Lisa Milne 2014 <lisa@ltmnet.com>
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

#include "content_toolitem.h"
#include "content_craftitem.h"
#include "content_clothesitem.h"
#include "content_mob.h"
#include "content_craft.h"
#include "content_list.h"
#include "content_mapnode.h"
#include "mineral.h"
#include <map>
#include "intl.h"
#include "enchantment.h"
#include "auth.h"
#include "content_burntimes.h"

std::map<content_t,struct ToolItemFeatures> g_content_toolitem_features;

ToolItemFeatures & content_toolitem_features(content_t i)
{
	if ((i&CONTENT_TOOLITEM_MASK) != CONTENT_TOOLITEM_MASK)
		return g_content_toolitem_features[CONTENT_IGNORE];

	std::map<content_t,struct ToolItemFeatures>::iterator it = g_content_toolitem_features.find(i);
	if (it == g_content_toolitem_features.end())
		return g_content_toolitem_features[CONTENT_IGNORE];
	return it->second;
}

int get_tool_use(tooluse_t *info, content_t target, uint16_t data, content_t toolid, uint16_t tooldata)
{
	ToolItemFeatures *t_features;
	bool type_match = false;
	bool is_material = false;
	bool is_mob = false;

	if (!info)
		return 1;

	info->wear = 1;
	info->diggable = false;
	info->data = 0.0;
	info->delay = 0.25;

	if (toolid == CONTENT_TOOLITEM_CREATIVEPICK) {
		info->wear = 0;
		info->diggable = true;
		info->data = 0.1;
		info->delay = 0.1;
		return 0;
	}

	t_features = &content_toolitem_features(toolid);

	if ((target&CONTENT_TOOLITEM_MASK) == CONTENT_TOOLITEM_MASK) {
		return 1;
	}else if ((target&CONTENT_CLOTHESITEM_MASK) == CONTENT_CLOTHESITEM_MASK) {
		return 1;
	}else if ((target&CONTENT_CRAFTITEM_MASK) == CONTENT_CRAFTITEM_MASK) {
		return 1;
	}else if ((target&CONTENT_MOB_MASK) == CONTENT_MOB_MASK) {
/*		mobs should have some per-mob defense, or armour, or something
		MobFeatures *m_features;

		m_features = &content_mob_features(target);
*/
		is_mob = true;

		switch (t_features->type) {
		case TT_SPECIAL:
		case TT_SHOVEL:
		case TT_PICK:
		case TT_BUCKET:
		case TT_SHEAR:
		case TT_NONE:
			info->diggable = true;
			info->data = (5.0-t_features->diginfo.time)*0.5;
			info->delay = 1.0;
			break;
		case TT_AXE:
			type_match = true;
			info->diggable = true;
			info->data = (6.0-t_features->diginfo.time);
			info->delay = 1.0;
			break;
		case TT_SWORD:
			type_match = true;
			info->diggable = true;
			info->data = (6.0-t_features->diginfo.time)*0.9;
			info->delay = 0.5;
			break;
		case TT_SPEAR:
			type_match = true;
			info->diggable = true;
			info->data = (5.0-t_features->diginfo.time)*0.75;
			info->delay = 0.5;
			break;
		case TT_CLUB:
		case TT_KNIFE:
			type_match = true;
			info->diggable = true;
			info->data = (5.0-t_features->diginfo.time)*0.65;
			info->delay = 0.5;
			break;
		default:
			break;
		}
	}else{ /* material item */
		ContentFeatures *c_features;

		c_features = &content_features(target);

		is_material = true;
		info->diggable = true;
		if (
			(toolid&CONTENT_CLOTHESITEM_MASK) == CONTENT_CLOTHESITEM_MASK
			|| (c_features->type == CMT_STONE && t_features->type != TT_PICK)
			|| (c_features->type == CMT_TREE && t_features->type == TT_NONE)
		) {
			info->diggable = false;
		}

		switch (t_features->type) {
		case TT_SPECIAL:
			info->diggable = false;
			break;
		case TT_AXE:
			if (
				c_features->type == CMT_PLANT
				|| c_features->type == CMT_WOOD
				|| c_features->type == CMT_TREE
				|| c_features->type == CMT_GLASS
			) {
				type_match = true;
			}
			break;
		case TT_PICK:
			if (c_features->type == CMT_STONE && t_features->diginfo.level >= mineral_features(data).min_level)
				type_match = true;
			break;
		case TT_SHOVEL:
			if (c_features->type == CMT_DIRT)
				type_match = true;
			break;
		case TT_SWORD:
			if (c_features->type == CMT_PLANT)
				type_match = true;
			break;
		case TT_KNIFE:
		case TT_SHEAR:
			if (c_features->type == CMT_PLANT)
				type_match = true;
			break;
		case TT_BUCKET:
			if (c_features->type == CMT_LIQUID) {
				info->wear = 0;
				type_match = true;
			}
			break;
		case TT_SPEAR:
			if (c_features->type == CMT_DIRT)
				type_match = true;
			break;
		case TT_HAMMER:
		case TT_NONE:
		default:
			break;
		}

		if (type_match) {
			if (c_features->type == CMT_LIQUID) {
				info->data = 0.1;
			}else{
				info->data = t_features->diginfo.time;
			}
		}else{
			info->data = 4.0;
		}

		info->data *= c_features->dig_time;
	}

	if (type_match && data != 0 && info->diggable) {
		uint16_t r;
		EnchantmentInfo enchant;
		while (enchantment_get(&data,&enchant)) {
			switch (enchant.type) {
			case ENCHANTMENT_FAST:
				if (is_material) {
					info->data /= (float)((enchant.level*enchant.level)+1);
				}else if (is_mob) {
					info->delay /= (float)((enchant.level*enchant.level)+1);
				}
				break;
			case ENCHANTMENT_LONGLASTING:
			{
				r = myrand_range(0,100);
				if (r < (100-(100/(enchant.level+1))))
					info->wear = 0;
				break;
			}
			case ENCHANTMENT_FLAME:
				r = myrand_range(0,100);
				if (r > 20)
					info->wear += 1;
				break;
			default:;
			}
		}
	}

	return 0;
}

std::string toolitem_overlay(content_t content, std::string ol)
{
	ToolItemFeatures t_features = content_toolitem_features(content);
	if (ol == "")
		return "";
	std::string base = "tool_overlay_";
	switch (t_features.type) {
	case TT_AXE:
		base += "axe_";
		break;
	case TT_PICK:
		base += "pick_";
		break;
	case TT_SHOVEL:
		base += "shovel_";
		break;
	case TT_SWORD:
		base += "sword_";
		break;
	case TT_SHEAR:
		base += "shear_";
		break;
	case TT_BUCKET:
		base += "bucket_";
		break;
	case TT_SPEAR:
		base += "spear_";
		break;
	case TT_KNIFE:
	case TT_HAMMER:
	case TT_SPECIAL:
	case TT_CLUB:
	case TT_NONE:
	default:
		return "";
		break;
	}

	base += ol;
	base += ".png";
	return base;
}

void content_toolitem_init()
{
	g_content_toolitem_features.clear();

	content_t i;
	ToolItemFeatures *f = NULL;

/* PICKS */

	i = CONTENT_TOOLITEM_SMALL_PICK;
	f = &g_content_toolitem_features[i];
	f->content = i;
	f->texture = "tool_handle.png^tool_head_pick_stone.png^tool_binding_pick.png";
	f->description = gettext("Stone Pick");
	f->type = TT_PICK;
	f->diginfo.uses = 32;
	f->diginfo.time = 3.0;
	f->diginfo.level = 1;
	{
		content_t r[9] = {
			CONTENT_ROCK,	CONTENT_ROCK,			CONTENT_ROCK,
			CONTENT_IGNORE,	CONTENT_CRAFTITEM_WOOD_PLANK,	CONTENT_IGNORE,
			CONTENT_IGNORE,	CONTENT_IGNORE,			CONTENT_IGNORE
		};
		crafting::setRecipe(r,i,1);
		r[4] = CONTENT_CRAFTITEM_JUNGLE_PLANK;
		crafting::setRecipe(r,i,1);
	}
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_TOOLITEM_COPPER_PICK;
	f = &g_content_toolitem_features[i];
	f->content = i;
	f->texture = "tool_handle.png^tool_head_pick_copper.png^tool_binding_pick.png";
	f->description = gettext("Copper Pick");
	f->type = TT_PICK;
	f->diginfo.uses = 64;
	f->diginfo.time = 1.5;
	f->diginfo.level = 2;
	crafting::setPickRecipe(CONTENT_CRAFTITEM_COPPER_INGOT,CONTENT_TOOLITEM_COPPER_PICK);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_TOOLITEM_FLINTPICK;
	f = &g_content_toolitem_features[i];
	f->content = i;
	f->texture = "tool_handle.png^tool_head_pick_flint.png^tool_binding_pick.png";
	f->description = gettext("Flint Pick");
	f->type = TT_PICK;
	f->diginfo.uses = 64;
	f->diginfo.time = 1.5;
	f->diginfo.level = 2;
	crafting::setPickRecipe(CONTENT_CRAFTITEM_FLINT,CONTENT_TOOLITEM_FLINTPICK);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_TOOLITEM_BRONZE_PICK;
	f = &g_content_toolitem_features[i];
	f->content = i;
	f->texture = "tool_handle.png^tool_head_pick_bronze.png^tool_binding_pick.png";
	f->description = gettext("Bronze Pick");
	f->type = TT_PICK;
	f->diginfo.uses = 128;
	f->diginfo.time = 1.25;
	f->diginfo.level = 3;
	crafting::setPickRecipe(CONTENT_CRAFTITEM_BRONZE_INGOT,CONTENT_TOOLITEM_BRONZE_PICK);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_TOOLITEM_IRON_PICK;
	f = &g_content_toolitem_features[i];
	f->content = i;
	f->texture = "tool_handle.png^tool_head_pick_iron.png^tool_binding_pick.png";
	f->description = gettext("Iron Pick");
	f->type = TT_PICK;
	f->diginfo.uses = 256;
	f->diginfo.time = 1.0;
	f->diginfo.level = 4;
	crafting::setPickRecipe(CONTENT_CRAFTITEM_IRON_INGOT,CONTENT_TOOLITEM_IRON_PICK);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_TOOLITEM_MITHRIL_UNBOUND_PICK;
	f = &g_content_toolitem_features[i];
	f->content = i;
	f->texture = "tool_handle.png^tool_head_pick_mithril.png^tool_binding_pick.png";
	f->description = gettext("Unbound Mithril Pick");
	f->type = TT_PICK;
	f->diginfo.uses = 1024;
	f->diginfo.time = 0.6;
	f->diginfo.level = 5;
	crafting::setPickRecipe(CONTENT_CRAFTITEM_MITHRIL_UNBOUND,CONTENT_TOOLITEM_MITHRIL_UNBOUND_PICK);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_TOOLITEM_MITHRIL_PICK;
	f = &g_content_toolitem_features[i];
	f->content = i;
	f->texture = "tool_handle.png^tool_head_pick_mithril.png^tool_binding_pick.png";
	f->description = gettext("Mithril Pick");
	f->type = TT_PICK;
	f->param_type = CPT_ENCHANTMENT;
	f->diginfo.uses = 2048;
	f->diginfo.time = 0.4;
	f->diginfo.level = 6;
	f->has_punch_effect = false;
	crafting::setPickRecipe(CONTENT_CRAFTITEM_MITHRIL,CONTENT_TOOLITEM_MITHRIL_PICK);
	content_list_add("craftguide",i,1,0);

	i = CONTENT_TOOLITEM_CREATIVEPICK;
	f = &g_content_toolitem_features[i];
	f->content = i;
	f->texture = "tool_handle.png^tool_head_pick_creative.png^tool_binding_pick.png";
	f->description = gettext("Creative Pick");
	f->type = TT_PICK;
	f->diginfo.uses = 1000;
	f->diginfo.time = 0.1;
	f->diginfo.level = 6;
	f->has_punch_effect = false;
	content_list_add("player-creative",i,1,0);
	content_list_add("creative",i,1,0);

/* SHOVELS */

	i = CONTENT_TOOLITEM_TROWEL;
	f = &g_content_toolitem_features[i];
	f->content = i;
	f->texture = "tool_handle.png^tool_head_shovel_stone.png^tool_binding_shovel.png";
	f->description = gettext("Stone Shovel");
	f->type = TT_SHOVEL;
	f->diginfo.uses = 32;
	f->diginfo.time = 3.0;
	f->diginfo.level = 1;
	crafting::set1over1Recipe(CONTENT_ROCK,CONTENT_CRAFTITEM_STICK,i);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_TOOLITEM_COPPER_SHOVEL;
	f = &g_content_toolitem_features[i];
	f->content = i;
	f->texture = "tool_handle.png^tool_head_shovel_copper.png^tool_binding_shovel.png";
	f->description = gettext("Copper Shovel");
	f->type = TT_SHOVEL;
	f->diginfo.uses = 64;
	f->diginfo.time = 1.5;
	f->diginfo.level = 1;
	crafting::setShovelRecipe(CONTENT_CRAFTITEM_COPPER_INGOT,CONTENT_TOOLITEM_COPPER_SHOVEL);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_TOOLITEM_FLINTSHOVEL;
	f = &g_content_toolitem_features[i];
	f->content = i;
	f->texture = "tool_handle.png^tool_head_shovel_flint.png^tool_binding_shovel.png";
	f->description = gettext("Flint Shovel");
	f->type = TT_SHOVEL;
	f->diginfo.uses = 128;
	f->diginfo.time = 1.75;
	f->diginfo.level = 2;
	crafting::setShovelRecipe(CONTENT_CRAFTITEM_FLINT,CONTENT_TOOLITEM_FLINTSHOVEL);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_TOOLITEM_IRON_SHOVEL;
	f = &g_content_toolitem_features[i];
	f->content = i;
	f->texture = "tool_handle.png^tool_head_shovel_iron.png^tool_binding_shovel.png";
	f->description = gettext("Iron Shovel");
	f->type = TT_SHOVEL;
	f->diginfo.uses = 256;
	f->diginfo.time = 1.0;
	f->diginfo.level = 3;
	crafting::setShovelRecipe(CONTENT_CRAFTITEM_IRON_INGOT,CONTENT_TOOLITEM_IRON_SHOVEL);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_TOOLITEM_BRONZE_SHOVEL;
	f = &g_content_toolitem_features[i];
	f->content = i;
	f->texture = "tool_handle.png^tool_head_shovel_bronze.png^tool_binding_shovel.png";
	f->description = gettext("Bronze Shovel");
	f->type = TT_SHOVEL;
	f->diginfo.uses = 512;
	f->diginfo.time = 0.75;
	f->diginfo.level = 4;
	crafting::setShovelRecipe(CONTENT_CRAFTITEM_BRONZE_INGOT,CONTENT_TOOLITEM_BRONZE_SHOVEL);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_TOOLITEM_MITHRIL_UNBOUND_SHOVEL;
	f = &g_content_toolitem_features[i];
	f->content = i;
	f->texture = "tool_handle.png^tool_head_shovel_mithril.png^tool_binding_shovel.png";
	f->description = gettext("Unbound Mithril Shovel");
	f->type = TT_SHOVEL;
	f->diginfo.uses = 1024;
	f->diginfo.time = 0.6;
	f->diginfo.level = 5;
	crafting::setShovelRecipe(CONTENT_CRAFTITEM_MITHRIL_UNBOUND,CONTENT_TOOLITEM_MITHRIL_UNBOUND_SHOVEL);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_TOOLITEM_MITHRIL_SHOVEL;
	f = &g_content_toolitem_features[i];
	f->content = i;
	f->texture = "tool_handle.png^tool_head_shovel_mithril.png^tool_binding_shovel.png";
	f->description = gettext("Mithril Shovel");
	f->type = TT_SHOVEL;
	f->param_type = CPT_ENCHANTMENT;
	f->diginfo.uses = 2048;
	f->diginfo.time = 0.4;
	f->diginfo.level = 5;
	crafting::setShovelRecipe(CONTENT_CRAFTITEM_MITHRIL,CONTENT_TOOLITEM_MITHRIL_SHOVEL);
	content_list_add("craftguide",i,1,0);

/* AXES */

	i = CONTENT_TOOLITEM_SMALL_AXE;
	f = &g_content_toolitem_features[i];
	f->content = i;
	f->texture = "tool_handle.png^tool_head_axe_stone.png^tool_binding_axe.png";
	f->description = gettext("Stone Axe");
	f->type = TT_AXE;
	f->diginfo.uses = 32;
	f->diginfo.time = 3.0;
	f->diginfo.level = 1;
	{
		content_t r[9] = {
			CONTENT_ROCK,	CONTENT_ROCK,			CONTENT_IGNORE,
			CONTENT_IGNORE,	CONTENT_CRAFTITEM_STICK,	CONTENT_IGNORE,
			CONTENT_IGNORE,	CONTENT_IGNORE,			CONTENT_IGNORE
		};
		crafting::setRecipe(r,i,1);

		r[0] = CONTENT_IGNORE; r[2] = CONTENT_ROCK;
		crafting::setRecipe(r,i,1);
	}
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_TOOLITEM_COPPER_AXE;
	f = &g_content_toolitem_features[i];
	f->content = i;
	f->texture = "tool_handle.png^tool_head_axe_copper.png^tool_binding_axe.png";
	f->description = gettext("Copper Axe");
	f->type = TT_AXE;
	f->diginfo.uses = 64;
	f->diginfo.time = 1.5;
	f->diginfo.level = 1;
	crafting::setAxeRecipe(CONTENT_CRAFTITEM_COPPER_INGOT,CONTENT_TOOLITEM_COPPER_AXE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_TOOLITEM_FLINTAXE;
	f = &g_content_toolitem_features[i];
	f->content = i;
	f->texture = "tool_handle.png^tool_head_axe_flint.png^tool_binding_axe.png";
	f->description = gettext("Flint Axe");
	f->type = TT_AXE;
	f->diginfo.uses = 128;
	f->diginfo.time = 1.75;
	f->diginfo.level = 2;
	crafting::setAxeRecipe(CONTENT_CRAFTITEM_FLINT,CONTENT_TOOLITEM_FLINTAXE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_TOOLITEM_IRON_AXE;
	f = &g_content_toolitem_features[i];
	f->content = i;
	f->texture = "tool_handle.png^tool_head_axe_iron.png^tool_binding_axe.png";
	f->description = gettext("Iron Axe");
	f->type = TT_AXE;
	f->diginfo.uses = 256;
	f->diginfo.time = 1.0;
	f->diginfo.level = 3;
	crafting::setAxeRecipe(CONTENT_CRAFTITEM_IRON_INGOT,CONTENT_TOOLITEM_IRON_AXE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_TOOLITEM_BRONZE_AXE;
	f = &g_content_toolitem_features[i];
	f->content = i;
	f->texture = "tool_handle.png^tool_head_axe_bronze.png^tool_binding_axe.png";
	f->description = gettext("Bronze Axe");
	f->type = TT_AXE;
	f->diginfo.uses = 512;
	f->diginfo.time = 0.75;
	f->diginfo.level = 4;
	crafting::setAxeRecipe(CONTENT_CRAFTITEM_BRONZE_INGOT,CONTENT_TOOLITEM_BRONZE_AXE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_TOOLITEM_MITHRIL_UNBOUND_AXE;
	f = &g_content_toolitem_features[i];
	f->content = i;
	f->texture = "tool_handle.png^tool_head_axe_mithril.png^tool_binding_axe.png";
	f->description = gettext("Unbound Mithril Axe");
	f->type = TT_AXE;
	f->diginfo.uses = 1024;
	f->diginfo.time = 0.6;
	f->diginfo.level = 5;
	crafting::setAxeRecipe(CONTENT_CRAFTITEM_MITHRIL_UNBOUND,CONTENT_TOOLITEM_MITHRIL_UNBOUND_AXE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_TOOLITEM_MITHRIL_AXE;
	f = &g_content_toolitem_features[i];
	f->content = i;
	f->texture = "tool_handle.png^tool_head_axe_mithril.png^tool_binding_axe.png";
	f->description = gettext("Mithril Axe");
	f->type = TT_AXE;
	f->param_type = CPT_ENCHANTMENT;
	f->diginfo.uses = 2048;
	f->diginfo.time = 0.4;
	f->diginfo.level = 5;
	crafting::setAxeRecipe(CONTENT_CRAFTITEM_MITHRIL,CONTENT_TOOLITEM_MITHRIL_AXE);
	content_list_add("craftguide",i,1,0);

/* WEAPONS - CLUBS, BOWS */

	i = CONTENT_TOOLITEM_CLUB;
	f = &g_content_toolitem_features[i];
	f->content = i;
	f->texture = "tool_woodclub.png";
	f->description = gettext("Wooden Club");
	f->type = TT_CLUB;
	f->diginfo.uses = 32;
	f->diginfo.time = 3.0;
	f->diginfo.level = 1;
	crafting::setCol1Recipe(CONTENT_CRAFTITEM_WOOD_PLANK,i);
	crafting::setCol1Recipe(CONTENT_CRAFTITEM_JUNGLE_PLANK,i);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_TOOLITEM_BOW;
	f = &g_content_toolitem_features[i];
	f->content = i;
	f->texture = "tool_bow.png";
	f->description = gettext("Bow");
	f->type = TT_SPECIAL;
	f->diginfo.uses = 256;
	f->diginfo.time = 1.0;
	f->diginfo.level = 2;
	f->thrown_item = CONTENT_CRAFTITEM_ARROW;
	{
		content_t r[9] = {
			CONTENT_CRAFTITEM_STRING,	CONTENT_CRAFTITEM_STICK,	CONTENT_IGNORE,
			CONTENT_CRAFTITEM_STRING,	CONTENT_IGNORE,			CONTENT_CRAFTITEM_STICK,
			CONTENT_CRAFTITEM_STRING,	CONTENT_CRAFTITEM_STICK,	CONTENT_IGNORE
		};
		crafting::setRecipe(r,i,1);
	}
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

/* SPEARS */

	i = CONTENT_TOOLITEM_STONE_SPEAR;
	f = &g_content_toolitem_features[i];
	f->content = i;
	f->texture = "tool_handle_long.png^tool_head_spear_stone.png^tool_binding_spear.png";
	f->description = gettext("Stone Spear");
	f->type = TT_SPEAR;
	f->diginfo.uses = 64;
	f->diginfo.time = 1.5;
	f->diginfo.level = 1;
	crafting::setSpearRecipe(CONTENT_ROCK,CONTENT_TOOLITEM_STONE_SPEAR);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_TOOLITEM_COPPER_SPEAR;
	f = &g_content_toolitem_features[i];
	f->content = i;
	f->texture = "tool_handle_long.png^tool_head_spear_copper.png^tool_binding_spear.png";
	f->description = gettext("Copper Spear");
	f->type = TT_SPEAR;
	f->diginfo.uses = 64;
	f->diginfo.time = 1.5;
	f->diginfo.level = 1;
	crafting::setSpearRecipe(CONTENT_CRAFTITEM_COPPER_INGOT,CONTENT_TOOLITEM_COPPER_SPEAR);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_TOOLITEM_FLINTSPEAR;
	f = &g_content_toolitem_features[i];
	f->content = i;
	f->texture = "tool_handle_long.png^tool_head_spear_flint.png^tool_binding_spear.png";
	f->description = gettext("Flint Spear");
	f->type = TT_SPEAR;
	f->diginfo.uses = 128;
	f->diginfo.time = 1.75;
	f->diginfo.level = 2;
	crafting::setSpearRecipe(CONTENT_CRAFTITEM_FLINT,CONTENT_TOOLITEM_FLINTSPEAR);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_TOOLITEM_IRON_SPEAR;
	f = &g_content_toolitem_features[i];
	f->content = i;
	f->texture = "tool_handle_long.png^tool_head_spear_iron.png^tool_binding_spear.png";
	f->description = gettext("Iron Spear");
	f->type = TT_SPEAR;
	f->diginfo.uses = 256;
	f->diginfo.time = 1.0;
	f->diginfo.level = 3;
	crafting::setSpearRecipe(CONTENT_CRAFTITEM_IRON_INGOT,CONTENT_TOOLITEM_IRON_SPEAR);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_TOOLITEM_BRONZE_SPEAR;
	f = &g_content_toolitem_features[i];
	f->content = i;
	f->texture = "tool_handle_long.png^tool_head_spear_bronze.png^tool_binding_spear.png";
	f->description = gettext("Bronze Spear");
	f->type = TT_SPEAR;
	f->diginfo.uses = 512;
	f->diginfo.time = 0.75;
	f->diginfo.level = 4;
	crafting::setSpearRecipe(CONTENT_CRAFTITEM_BRONZE_INGOT,CONTENT_TOOLITEM_BRONZE_SPEAR);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_TOOLITEM_MITHRIL_UNBOUND_SPEAR;
	f = &g_content_toolitem_features[i];
	f->content = i;
	f->texture = "tool_handle_long.png^tool_head_spear_mithril.png^tool_binding_spear.png";
	f->description = gettext("Unbound Mithril Spear");
	f->type = TT_SPEAR;
	f->diginfo.uses = 1024;
	f->diginfo.time = 0.6;
	f->diginfo.level = 5;
	crafting::setSpearRecipe(CONTENT_CRAFTITEM_MITHRIL_UNBOUND,CONTENT_TOOLITEM_MITHRIL_UNBOUND_SPEAR);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_TOOLITEM_MITHRIL_SPEAR;
	f = &g_content_toolitem_features[i];
	f->content = i;
	f->texture = "tool_handle_long.png^tool_head_spear_mithril.png^tool_binding_spear.png";
	f->description = gettext("Mithril Spear");
	f->type = TT_SPEAR;
	f->param_type = CPT_ENCHANTMENT;
	f->diginfo.uses = 2048;
	f->diginfo.time = 0.4;
	f->diginfo.level = 5;
	crafting::setSpearRecipe(CONTENT_CRAFTITEM_MITHRIL,CONTENT_TOOLITEM_MITHRIL_SPEAR);
	content_list_add("craftguide",i,1,0);

/* SWORDS */

	i = CONTENT_TOOLITEM_COPPER_SWORD;
	f = &g_content_toolitem_features[i];
	f->content = i;
	f->texture = "tool_handle.png^tool_head_sword_copper.png^tool_binding_sword.png";
	f->description = gettext("Copper Sword");
	f->type = TT_SWORD;
	f->diginfo.uses = 64;
	f->diginfo.time = 1.5;
	f->diginfo.level = 1;
	crafting::setSwordRecipe(CONTENT_CRAFTITEM_COPPER_INGOT,CONTENT_TOOLITEM_COPPER_SWORD);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_TOOLITEM_BRONZE_SWORD;
	f = &g_content_toolitem_features[i];
	f->content = i;
	f->texture = "tool_handle.png^tool_head_sword_bronze.png^tool_binding_sword.png";
	f->description = gettext("Bronze Sword");
	f->type = TT_SWORD;
	f->diginfo.uses = 128;
	f->diginfo.time = 1.5;
	f->diginfo.level = 4;
	crafting::setSwordRecipe(CONTENT_CRAFTITEM_BRONZE_INGOT,CONTENT_TOOLITEM_BRONZE_SWORD);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_TOOLITEM_IRON_SWORD;
	f = &g_content_toolitem_features[i];
	f->content = i;
	f->texture = "tool_handle.png^tool_head_sword_iron.png^tool_binding_sword.png";
	f->description = gettext("Iron Sword");
	f->type = TT_SWORD;
	f->diginfo.uses = 256;
	f->diginfo.time = 1.0;
	f->diginfo.level = 3;
	crafting::setSwordRecipe(CONTENT_CRAFTITEM_IRON_INGOT,CONTENT_TOOLITEM_IRON_SWORD);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_TOOLITEM_MITHRIL_UNBOUND_SWORD;
	f = &g_content_toolitem_features[i];
	f->content = i;
	f->texture = "tool_handle.png^tool_head_sword_mithril.png^tool_binding_sword.png";
	f->description = gettext("Unbound Mithril Sword");
	f->type = TT_SWORD;
	f->diginfo.uses = 1024;
	f->diginfo.time = 0.6;
	f->diginfo.level = 5;
	crafting::setSwordRecipe(CONTENT_CRAFTITEM_MITHRIL_UNBOUND,CONTENT_TOOLITEM_MITHRIL_UNBOUND_SWORD);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_TOOLITEM_MITHRIL_SWORD;
	f = &g_content_toolitem_features[i];
	f->content = i;
	f->texture = "tool_handle.png^tool_head_sword_mithril.png^tool_binding_sword.png";
	f->description = gettext("Mithril Sword");
	f->type = TT_SWORD;
	f->param_type = CPT_ENCHANTMENT;
	f->diginfo.uses = 2048;
	f->diginfo.time = 0.4;
	f->diginfo.level = 5;
	crafting::setSwordRecipe(CONTENT_CRAFTITEM_MITHRIL,CONTENT_TOOLITEM_MITHRIL_SWORD);
	content_list_add("craftguide",i,1,0);

/* SHEARS */

	i = CONTENT_TOOLITEM_FLINTSHEARS;
	f = &g_content_toolitem_features[i];
	f->content = i;
	f->texture = "tool_flintshears.png";
	f->description = gettext("Flint Shears");
	f->type = TT_SHEAR;
	f->diginfo.uses = 128;
	f->diginfo.time = 1.5;
	f->diginfo.level = 2;
	crafting::setShearsRecipe(CONTENT_CRAFTITEM_FLINT,CONTENT_TOOLITEM_FLINTSHEARS);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_TOOLITEM_COPPER_SHEARS;
	f = &g_content_toolitem_features[i];
	f->content = i;
	f->texture = "tool_copper_shears.png";
	f->description = gettext("Copper Shears");
	f->type = TT_SHEAR;
	f->diginfo.uses = 128;
	f->diginfo.time = 1.5;
	f->diginfo.level = 2;
	crafting::setShearsRecipe(CONTENT_CRAFTITEM_COPPER_INGOT,CONTENT_TOOLITEM_COPPER_SHEARS);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_TOOLITEM_IRON_SHEARS;
	f = &g_content_toolitem_features[i];
	f->content = i;
	f->texture = "tool_ironshears.png";
	f->description = gettext("Iron Shears");
	f->type = TT_SHEAR;
	f->diginfo.uses = 256;
	f->diginfo.time = 1.0;
	f->diginfo.level = 2;
	crafting::setShearsRecipe(CONTENT_CRAFTITEM_IRON_INGOT,CONTENT_TOOLITEM_IRON_SHEARS);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

/* BUCKETS */

	i = CONTENT_TOOLITEM_WBUCKET;
	f = &g_content_toolitem_features[i];
	f->content = i;
	f->texture = "tool_woodbucket.png";
	f->description = gettext("Wooden Bucket");
	f->liquids_pointable = true;
	f->type = TT_BUCKET;
	f->param_type = CPT_CONTENT;
	f->diginfo.uses = 64;
	f->diginfo.time = 1.5;
	f->diginfo.level = 1;
	f->damaging_nodes_diggable = false;
	crafting::setURecipe(CONTENT_CRAFTITEM_JUNGLE_PLANK,CONTENT_TOOLITEM_WBUCKET);
	crafting::setURecipe(CONTENT_CRAFTITEM_WOOD_PLANK,CONTENT_TOOLITEM_WBUCKET);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);
	content_list_add("creative",i,1,CONTENT_WATERSOURCE);

	i = CONTENT_TOOLITEM_TINBUCKET;
	f = &g_content_toolitem_features[i];
	f->content = i;
	f->texture = "tool_tinbucket.png";
	f->description = gettext("Tin Bucket");
	f->liquids_pointable = true;
	f->type = TT_BUCKET;
	f->param_type = CPT_CONTENT;
	f->diginfo.uses = 128;
	f->diginfo.time = 1.75;
	f->diginfo.level = 2;
	f->damaging_nodes_diggable = false;
	crafting::setURecipe(CONTENT_CRAFTITEM_TIN_INGOT,CONTENT_TOOLITEM_TINBUCKET);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);
	content_list_add("creative",i,1,CONTENT_WATERSOURCE);

	i = CONTENT_TOOLITEM_IRON_BUCKET;
	f = &g_content_toolitem_features[i];
	f->content = i;
	f->texture = "tool_ironbucket.png";
	f->description = gettext("Iron Bucket");
	f->liquids_pointable = true;
	f->type = TT_BUCKET;
	f->param_type = CPT_CONTENT;
	f->diginfo.uses = 256;
	f->diginfo.time = 1.0;
	f->diginfo.level = 3;
	crafting::setURecipe(CONTENT_CRAFTITEM_IRON_INGOT,CONTENT_TOOLITEM_IRON_BUCKET);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);
	content_list_add("creative",i,1,CONTENT_WATERSOURCE);
	content_list_add("creative",i,1,CONTENT_LAVASOURCE);

/* KNIVES */

	i = CONTENT_TOOLITEM_STONE_KNIFE;
	f = &g_content_toolitem_features[i];
	f->content = i;
	f->texture = "tool_handle_short.png^tool_head_knife_stone.png";
	f->description = gettext("Stone Knife");
	f->type = TT_KNIFE;
	f->diginfo.uses = 32;
	f->diginfo.time = 3.0;
	f->diginfo.level = 1;
	crafting::setKnifeRecipe(CONTENT_ROCK,CONTENT_TOOLITEM_STONE_KNIFE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_TOOLITEM_COPPER_KNIFE;
	f = &g_content_toolitem_features[i];
	f->content = i;
	f->texture = "tool_handle_short.png^tool_head_knife_copper.png";
	f->description = gettext("Copper Knife");
	f->type = TT_KNIFE;
	f->diginfo.uses = 64;
	f->diginfo.time = 2.0;
	f->diginfo.level = 1;
	crafting::setKnifeRecipe(CONTENT_CRAFTITEM_COPPER_INGOT,CONTENT_TOOLITEM_COPPER_KNIFE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_TOOLITEM_FLINT_KNIFE;
	f = &g_content_toolitem_features[i];
	f->content = i;
	f->texture = "tool_handle_short.png^tool_head_knife_flint.png";
	f->description = gettext("Flint Knife");
	f->type = TT_KNIFE;
	f->diginfo.uses = 64;
	f->diginfo.time = 2.0;
	f->diginfo.level = 1;
	crafting::setKnifeRecipe(CONTENT_CRAFTITEM_FLINT,CONTENT_TOOLITEM_FLINT_KNIFE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_TOOLITEM_BRONZE_KNIFE;
	f = &g_content_toolitem_features[i];
	f->content = i;
	f->texture = "tool_handle_short.png^tool_head_knife_bronze.png";
	f->description = gettext("Bronze Knife");
	f->type = TT_KNIFE;
	f->diginfo.uses = 128;
	f->diginfo.time = 1.5;
	f->diginfo.level = 1;
	crafting::setKnifeRecipe(CONTENT_CRAFTITEM_BRONZE_INGOT,CONTENT_TOOLITEM_BRONZE_KNIFE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_TOOLITEM_IRON_KNIFE;
	f = &g_content_toolitem_features[i];
	f->content = i;
	f->texture = "tool_handle_short.png^tool_head_knife_iron.png";
	f->description = gettext("Iron Knife");
	f->type = TT_KNIFE;
	f->diginfo.uses = 256;
	f->diginfo.time = 1.0;
	f->diginfo.level = 1;
	crafting::setKnifeRecipe(CONTENT_CRAFTITEM_IRON_INGOT,CONTENT_TOOLITEM_IRON_KNIFE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_TOOLITEM_MITHRIL_UNBOUND_KNIFE;
	f = &g_content_toolitem_features[i];
	f->content = i;
	f->texture = "tool_handle_short.png^tool_head_knife_mithril.png";
	f->description = gettext("Unbound Mithril Knife");
	f->type = TT_KNIFE;
	f->diginfo.uses = 1024;
	f->diginfo.time = 0.5;
	f->diginfo.level = 1;
	crafting::setKnifeRecipe(CONTENT_CRAFTITEM_MITHRIL_UNBOUND,CONTENT_TOOLITEM_MITHRIL_UNBOUND_KNIFE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_TOOLITEM_MITHRIL_KNIFE;
	f = &g_content_toolitem_features[i];
	f->content = i;
	f->texture = "tool_handle_short.png^tool_head_knife_mithril.png";
	f->description = gettext("Mithril Knife");
	f->type = TT_KNIFE;
	f->param_type = CPT_ENCHANTMENT;
	f->diginfo.uses = 2048;
	f->diginfo.time = 0.5;
	f->diginfo.level = 1;
	crafting::setKnifeRecipe(CONTENT_CRAFTITEM_MITHRIL,CONTENT_TOOLITEM_MITHRIL_KNIFE);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

/* SPECIAL TOOLS */

	i = CONTENT_TOOLITEM_FIRESTARTER;
	f = &g_content_toolitem_features[i];
	f->content = i;
	f->texture = "tool_fire_starter.png";
	f->description = gettext("Fire Starter");
	f->liquids_pointable = true;
	f->type = TT_SPECIAL;
	f->diginfo.level = 3;
	f->has_fire_effect = true;
	crafting::set1Any2Recipe(CONTENT_CRAFTITEM_FLINT,CONTENT_CRAFTITEM_IRON_INGOT,CONTENT_TOOLITEM_FIRESTARTER);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_TOOLITEM_CROWBAR;
	f = &g_content_toolitem_features[i];
	f->content = i;
	f->texture = "tool_crowbar.png";
	f->description = gettext("Crowbar");
	f->type = TT_SPECIAL;
	f->diginfo.level = 3;
	f->has_rotate_effect = true;
	crafting::set1over1Recipe(CONTENT_CRAFTITEM_IRON_INGOT,CONTENT_CRAFTITEM_IRON_INGOT,CONTENT_TOOLITEM_CROWBAR);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_TOOLITEM_KEY;
	f = &g_content_toolitem_features[i];
	f->content = i;
	f->texture = "tool_key.png";
	f->description = gettext("Key");
	f->type = TT_SPECIAL;
	f->diginfo.level = 4;
	f->has_unlock_effect = true;
	crafting::set1To1Recipe(CONTENT_CRAFTITEM_GOLD_INGOT,CONTENT_TOOLITEM_KEY);
	content_list_add("craftguide",i,1,0);
	content_list_add("creative",i,1,0);

	i = CONTENT_TOOLITEM_MITHRIL_KEY;
	f = &g_content_toolitem_features[i];
	f->content = i;
	f->texture = "tool_mithril_key.png";
	f->description = gettext("Mithril Key");
	f->type = TT_SPECIAL;
	f->diginfo.level = 5;
	f->has_unlock_effect = true;
	f->has_super_unlock_effect = true;
	/* this can only be crafted by server admin */
	{
		content_t r[9] = {
			CONTENT_CRAFTITEM_MITHRIL_UNBOUND,	CONTENT_IGNORE,				CONTENT_IGNORE,
			CONTENT_IGNORE,				CONTENT_IGNORE,				CONTENT_IGNORE,
			CONTENT_IGNORE,				CONTENT_IGNORE,				CONTENT_IGNORE
		};
		crafting::setRecipe(r,i,1,PRIV_SERVER);
	}

	i = CONTENT_TOOLITEM_MOB_SPAWNER;
	f = &g_content_toolitem_features[i];
	f->content = i;
	f->texture = "tool_mob_spawner.png";
	f->description = gettext("Mob Spawner");
	f->type = TT_SPECIAL;
	f->diginfo.level = 4;
	f->param_type = CPT_DROP;
	{
		content_t r[9] = {
			CONTENT_IGNORE,				CONTENT_CRAFTITEM_MITHRIL_UNBOUND,	CONTENT_IGNORE,
			CONTENT_CRAFTITEM_MITHRIL_UNBOUND,	CONTENT_IGNORE,				CONTENT_CRAFTITEM_MITHRIL_UNBOUND,
			CONTENT_IGNORE,				CONTENT_CRAFTITEM_MITHRIL_UNBOUND,	CONTENT_IGNORE
		};
		crafting::setRecipe(r,i,1);
	}
	content_list_add("craftguide",i,1,0);
}
