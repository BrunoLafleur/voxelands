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

// For g_settings
#include "main.h"

#include "content_mapnode.h"
#include "content_nodebox.h"
#include "content_list.h"
#include "content_craft.h"
#include "content_nodemeta.h"
#include "intl.h"
#include "content_burntimes.h"

void content_mapnode_plants(bool repeat)
{
	content_t i;
	ContentFeatures *f = NULL;


	i = CONTENT_TREE;
	f = &content_features(i);
	f->description = gettext("Tree");
	f->setAllTextures("tree.png");
	f->setTexture(0, "tree_top.png");
	f->setTexture(1, "tree_top.png");
	f->param_type = CPT_BLOCKDATA;
	f->setInventoryTextureCube("tree_top.png", "tree.png", "tree.png");
	f->draw_type = CDT_TRUNKLIKE;
	f->is_ground_content = true;
	f->flammable = 2; // can be set on fire
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->cook_result = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_CHARCOAL)+" 1";
	f->fuel_time = BT_TREE;
	f->type = CMT_TREE;
	f->dig_time = 1.0;
	content_list_add("creative",i,1,0);
	content_list_add("cooking",i,1,0);

	i = CONTENT_APPLE_TREE;
	f = &content_features(i);
	f->description = gettext("Apple Tree");
	f->setAllTextures("apple_tree.png");
	f->setTexture(0, "apple_tree_top.png");
	f->setTexture(1, "apple_tree_top.png");
	f->param_type = CPT_BLOCKDATA;
	f->setInventoryTextureCube("apple_tree_top.png", "apple_tree.png", "apple_tree.png");
	f->draw_type = CDT_TRUNKLIKE;
	f->is_ground_content = true;
	f->flammable = 2; // can be set on fire
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->cook_result = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_CHARCOAL)+" 1";
	f->fuel_time = BT_TREE;
	f->type = CMT_TREE;
	f->dig_time = 1.0;
	content_list_add("creative",i,1,0);
	content_list_add("cooking",i,1,0);

	i = CONTENT_JUNGLETREE;
	f = &content_features(i);
	f->description = gettext("Jungle Tree");
	f->setAllTextures("jungletree.png");
	f->setTexture(0, "jungletree_top.png");
	f->setTexture(1, "jungletree_top.png");
	f->param_type = CPT_BLOCKDATA;
	f->setInventoryTextureCube("jungletree_top.png", "jungletree.png", "jungletree.png");
	f->draw_type = CDT_TRUNKLIKE;
	//f->is_ground_content = true;
	f->flammable = 2; // can be set on fire
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->cook_result = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_CHARCOAL)+" 1";
	f->fuel_time = BT_TREE;
	f->type = CMT_TREE;
	f->dig_time = 1.0;
	content_list_add("creative",i,1,0);

	i = CONTENT_CONIFER_TREE;
	f = &content_features(i);
	f->description = gettext("Conifer Tree");
	f->setAllTextures("conifer_tree.png");
	f->setTexture(0, "conifer_tree_top.png");
	f->setTexture(1, "conifer_tree_top.png");
	f->param_type = CPT_BLOCKDATA;
	f->setInventoryTextureCube("conifer_tree_top.png", "conifer_tree.png", "conifer_tree.png");
	f->draw_type = CDT_TRUNKLIKE;
	f->is_ground_content = true;
	f->flammable = 2; // can be set on fire
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->cook_result = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_CHARCOAL)+" 1";
	f->fuel_time = BT_TREE;
	f->type = CMT_TREE;
	f->dig_time = 0.8;
	content_list_add("creative",i,1,0);
	content_list_add("cooking",i,1,0);

	i = CONTENT_YOUNG_TREE;
	f = &content_features(i);
	f->description = gettext("Young Tree");
	f->setAllTextures("tree.png");
	f->setTexture(0, "tree_top.png");
	f->setTexture(1, "tree_top.png");
	f->draw_type = CDT_NODEBOX;
	f->param_type = CPT_LIGHT;
	f->light_propagates = true;
	content_nodebox_youngtree(f);
	f->setInventoryTextureNodeBox(i,"tree_top.png", "tree.png", "tree.png");
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->special_alternate_node = CONTENT_LEAVES;
	f->fuel_time = BT_YOUNG_TREE;
	f->type = CMT_TREE;
	f->dig_time = 1.0;
	f->suffocation_per_second = 0;
	content_list_add("creative",i,1,0);

	i = CONTENT_YOUNG_JUNGLETREE;
	f = &content_features(i);
	f->description = gettext("Young Jungle Tree");
	f->setAllTextures("jungletree.png");
	f->setTexture(0, "jungletree_top.png");
	f->setTexture(1, "jungletree_top.png");
	f->draw_type = CDT_NODEBOX;
	f->param_type = CPT_LIGHT;
	f->light_propagates = true;
	content_nodebox_youngtree(f);
	f->setInventoryTextureNodeBox(i,"jungletree_top.png", "jungletree.png", "jungletree.png");
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->special_alternate_node = CONTENT_JUNGLELEAVES;
	f->fuel_time = BT_YOUNG_TREE;
	f->type = CMT_TREE;
	f->dig_time = 1.0;
	f->suffocation_per_second = 0;
	content_list_add("creative",i,1,0);

	i = CONTENT_YOUNG_APPLE_TREE;
	f = &content_features(i);
	f->description = gettext("Young Apple Tree");
	f->setAllTextures("apple_tree.png");
	f->setTexture(0, "apple_tree_top.png");
	f->setTexture(1, "apple_tree_top.png");
	f->draw_type = CDT_NODEBOX;
	f->param_type = CPT_LIGHT;
	f->light_propagates = true;
	content_nodebox_youngtree(f);
	f->setInventoryTextureNodeBox(i,"apple_tree_top.png", "apple_tree.png", "apple_tree.png");
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->special_alternate_node = CONTENT_APPLE_LEAVES;
	f->fuel_time = BT_YOUNG_TREE;
	f->type = CMT_TREE;
	f->dig_time = 1.0;
	f->suffocation_per_second = 0;
	content_list_add("creative",i,1,0);

	i = CONTENT_YOUNG_CONIFER_TREE;
	f = &content_features(i);
	f->description = gettext("Young Conifer Tree");
	f->setAllTextures("conifer_tree.png");
	f->setTexture(0, "conifer_tree_top.png");
	f->setTexture(1, "conifer_tree_top.png");
	f->draw_type = CDT_NODEBOX;
	f->param_type = CPT_LIGHT;
	f->light_propagates = true;
	content_nodebox_youngtree(f);
	f->setInventoryTextureNodeBox(i,"conifer_tree_top.png", "conifer_tree.png", "conifer_tree.png");
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->special_alternate_node = CONTENT_CONIFER_LEAVES;
	f->fuel_time = BT_YOUNG_TREE;
	f->type = CMT_TREE;
	f->dig_time = 1.0;
	f->suffocation_per_second = 0;
	content_list_add("creative",i,1,0);

	i = CONTENT_JUNGLEGRASS;
	f = &content_features(i);
	f->description = gettext("Jungle Grass");
	f->setInventoryTexture("junglegrass.png");
	f->setAllTextures("junglegrass.png");
	f->setTexture(1,"junglegrass_leaf.png");
	f->setAllTextureFlags(0);
	f->light_propagates = true;
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_PLANTLIKE_FERN;
	f->air_equivalent = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->ondig_special_drop = CONTENT_CRAFTITEM_STRAW;
	f->ondig_special_drop_count = 2;
	f->ondig_special_tool = TT_KNIFE;
	f->walkable = false;
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_BUSH;
	f->type = CMT_PLANT;
	f->dig_time = 0.20;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_list_add("creative",i,1,0);

	i = CONTENT_JUNGLEFERN;
	f = &content_features(i);
	f->description = gettext("Jungle Fern");
	f->setInventoryTexture("junglegrass.png");
	f->setAllTextures("junglegrass.png");
	f->setTexture(1,"leaf_big.png");
	f->setAllTextureFlags(0);
	f->light_propagates = true;
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_PLANTLIKE_FERN;
	f->air_equivalent = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->walkable = false;
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_BUSH;
	f->type = CMT_PLANT;
	f->dig_time = 0.20;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_list_add("creative",i,1,0);

	i = CONTENT_LEAVES;
	f = &content_features(i);
	f->description = gettext("Leaves");
	f->light_propagates = true;
	f->air_equivalent = true;
	f->walkable = false;
	f->climbable = true;
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_LEAFLIKE;
	f->setAllTextures("leaves.png");
#ifndef SERVER
	f->setAllTextureTypes(MATERIAL_ALPHA_SIMPLE);
	f->setAllTextureFlags(0);
#endif
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_LEAF;
	f->cook_result = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_RESIN)+" 1";
	f->extra_dug_item = std::string("MaterialItem2 ")+itos(CONTENT_SAPLING)+" 1";
	f->extra_dug_item_rarity = 20;
	f->dug_item = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_STICK)+" 2";
	f->ondig_special_drop = CONTENT_TRIMMED_LEAVES;
	f->ondig_special_drop_count = 1;
	f->ondig_special_tool = TT_SHEAR;
	f->place_on_drop = CONTENT_SAPLING;
	f->place_on_drop_alternate = CONTENT_WILDGRASS_SHORT;
	f->type = CMT_PLANT;
	f->dig_time = 0.15;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_list_add("decrafting",i,1,0);
	content_list_add("cooking",i,1,0);

	i = CONTENT_LEAVES_AUTUMN;
	f = &content_features(i);
	f->description = gettext("Leaves");
	f->light_propagates = true;
	f->air_equivalent = true;
	f->walkable = false;
	f->climbable = true;
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_LEAFLIKE;
	f->setAllTextures("leaves_autumn.png");
#ifndef SERVER
	f->setAllTextureTypes(MATERIAL_ALPHA_SIMPLE);
	f->setAllTextureFlags(0);
#endif
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_LEAF;
	f->cook_result = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_RESIN)+" 1";
	f->extra_dug_item = std::string("MaterialItem2 ")+itos(CONTENT_SAPLING)+" 1";
	f->extra_dug_item_rarity = 20;
	f->dug_item = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_STICK)+" 2";
	f->ondig_special_drop = CONTENT_TRIMMED_LEAVES_AUTUMN;
	f->ondig_special_drop_count = 1;
	f->ondig_special_tool = TT_SHEAR;
	f->place_on_drop = CONTENT_SAPLING;
	f->place_on_drop_alternate = CONTENT_WILDGRASS_SHORT;
	f->type = CMT_PLANT;
	f->dig_time = 0.15;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_list_add("decrafting",i,1,0);
	content_list_add("cooking",i,1,0);

	i = CONTENT_LEAVES_WINTER;
	f = &content_features(i);
	f->description = gettext("Leaves");
	f->light_propagates = true;
	f->air_equivalent = true;
	f->walkable = false;
	f->climbable = true;
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_LEAFLIKE;
	f->setAllTextures("leaves_winter.png");
#ifndef SERVER
	f->setAllTextureTypes(MATERIAL_ALPHA_SIMPLE);
	f->setAllTextureFlags(0);
#endif
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_LEAF;
	f->cook_result = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_ASH)+" 1";
	f->extra_dug_item = std::string("MaterialItem2 ")+itos(CONTENT_SAPLING)+" 1";
	f->extra_dug_item_rarity = 20;
	f->dug_item = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_STICK)+" 2";
	f->ondig_special_drop = CONTENT_TRIMMED_LEAVES_WINTER;
	f->ondig_special_drop_count = 1;
	f->ondig_special_tool = TT_SHEAR;
	f->place_on_drop = CONTENT_SAPLING;
	f->place_on_drop_alternate = CONTENT_WILDGRASS_SHORT;
	f->type = CMT_PLANT;
	f->dig_time = 0.15;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_list_add("decrafting",i,1,0);
	content_list_add("cooking",i,1,0);

	i = CONTENT_LEAVES_SNOWY;
	f = &content_features(i);
	f->description = gettext("Leaves");
	f->light_propagates = true;
	f->air_equivalent = true;
	f->walkable = false;
	f->climbable = true;
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_LEAFLIKE;
	f->setAllTextures("leaves_snowy.png");
#ifndef SERVER
	f->setAllTextureTypes(MATERIAL_ALPHA_SIMPLE);
	f->setAllTextureFlags(0);
#endif
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_LEAF;
	f->cook_result = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_ASH)+" 1";
	f->extra_dug_item = std::string("MaterialItem2 ")+itos(CONTENT_SAPLING)+" 1";
	f->extra_dug_item_rarity = 20;
	f->dug_item = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_STICK)+" 2";
	f->ondig_special_drop = CONTENT_TRIMMED_LEAVES_WINTER;
	f->ondig_special_drop_count = 1;
	f->ondig_special_tool = TT_SHEAR;
	f->place_on_drop = CONTENT_SAPLING;
	f->place_on_drop_alternate = CONTENT_WILDGRASS_SHORT;
	f->type = CMT_PLANT;
	f->dig_time = 0.15;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_list_add("decrafting",i,1,0);
	content_list_add("cooking",i,1,0);

	i = CONTENT_APPLE_LEAVES;
	f = &content_features(i);
	f->description = gettext("Apple Tree Leaves");
	f->light_propagates = true;
	f->air_equivalent = true;
	f->walkable = false;
	f->climbable = true;
	f->param_type = CPT_LIGHT;
	f->special_alternate_node = CONTENT_APPLE_BLOSSOM;
	f->draw_type = CDT_LEAFLIKE;
	f->setAllTextures("apple_leaves.png");
#ifndef SERVER
	f->setAllTextureTypes(MATERIAL_ALPHA_SIMPLE);
	f->setAllTextureFlags(0);
#endif
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_LEAF;
	f->cook_result = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_RESIN)+" 1";
	f->extra_dug_item = std::string("MaterialItem2 ")+itos(CONTENT_APPLE_SAPLING)+" 1";
	f->extra_dug_item_rarity = 20;
	f->dug_item = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_STICK)+" 2";
	f->ondig_special_drop = CONTENT_TRIMMED_APPLE_LEAVES;
	f->ondig_special_drop_count = 1;
	f->ondig_special_tool = TT_SHEAR;
	f->place_on_drop = CONTENT_APPLE_SAPLING;
	f->place_on_drop_alternate = CONTENT_WILDGRASS_SHORT;
	f->type = CMT_PLANT;
	f->dig_time = 0.15;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_list_add("decrafting",i,1,0);
	content_list_add("cooking",i,1,0);

	i = CONTENT_JUNGLELEAVES;
	f = &content_features(i);
	f->description = gettext("Jungle Leaves");
	f->light_propagates = true;
	f->air_equivalent = true;
	f->walkable = false;
	f->climbable = true;
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_LEAFLIKE;
	f->setAllTextures("jungleleaves.png");
#ifndef SERVER
	f->setAllTextureTypes(MATERIAL_ALPHA_SIMPLE);
	f->setAllTextureFlags(0);
#endif
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_LEAF;
	f->cook_result = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_RESIN)+" 1";
	f->extra_dug_item = std::string("MaterialItem2 ")+itos(CONTENT_JUNGLESAPLING)+" 1";
	f->extra_dug_item_rarity = 20;
	f->dug_item = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_STICK)+" 2";
	f->ondig_special_drop = CONTENT_TRIMMED_JUNGLE_LEAVES;
	f->ondig_special_drop_count = 1;
	f->ondig_special_tool = TT_SHEAR;
	f->place_on_drop = CONTENT_JUNGLESAPLING;
	f->place_on_drop_alternate = CONTENT_WILDGRASS_SHORT;
	f->type = CMT_PLANT;
	f->dig_time = 0.15;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_list_add("decrafting",i,1,0);
	content_list_add("cooking",i,1,0);

	i = CONTENT_CONIFER_LEAVES;
	f = &content_features(i);
	f->description = gettext("Conifer Leaves");
	f->light_propagates = true;
	f->air_equivalent = true;
	f->walkable = false;
	f->climbable = true;
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_LEAFLIKE;
	f->setAllTextures("conifer_leaves.png");
#ifndef SERVER
	f->setAllTextureTypes(MATERIAL_ALPHA_SIMPLE);
	f->setAllTextureFlags(0);
#endif
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_LEAF;
	f->cook_result = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_RESIN)+" 1";
	f->extra_dug_item = std::string("MaterialItem2 ")+itos(CONTENT_CONIFER_SAPLING)+" 1";
	f->extra_dug_item_rarity = 20;
	f->dug_item = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_STICK)+" 2";
	f->ondig_special_drop = CONTENT_TRIMMED_CONIFER_LEAVES;
	f->ondig_special_drop_count = 1;
	f->ondig_special_tool = TT_SHEAR;
	f->place_on_drop = CONTENT_CONIFER_SAPLING;
	f->place_on_drop_alternate = CONTENT_WILDGRASS_SHORT;
	f->type = CMT_PLANT;
	f->dig_time = 0.15;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_list_add("decrafting",i,1,0);
	content_list_add("cooking",i,1,0);

	i = CONTENT_TRIMMED_LEAVES;
	f = &content_features(i);
	f->description = gettext("Trimmed Leaves");
	f->light_propagates = true;
	f->air_equivalent = true;
	f->walkable = false;
	f->climbable = true;
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_GLASSLIKE;
	f->setAllTextures("leaves.png");
#ifndef SERVER
	f->setAllTextureTypes(MATERIAL_ALPHA_SIMPLE);
#endif
	f->setInventoryTextureCube("leaves.png", "leaves.png", "leaves.png");
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_LEAF;
	f->cook_result = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_RESIN)+" 1";
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->type = CMT_PLANT;
	f->dig_time = 0.15;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_list_add("creative",i,1,0);
	content_list_add("cooking",i,1,0);

	i = CONTENT_TRIMMED_LEAVES_AUTUMN;
	f = &content_features(i);
	f->description = gettext("Trimmed Leaves");
	f->light_propagates = true;
	f->air_equivalent = true;
	f->walkable = false;
	f->climbable = true;
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_GLASSLIKE;
	f->setAllTextures("leaves_autumn.png");
#ifndef SERVER
	f->setAllTextureTypes(MATERIAL_ALPHA_SIMPLE);
#endif
	f->setInventoryTextureCube("leaves_autumn.png", "leaves_autumn.png", "leaves_autumn.png");
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_LEAF;
	f->cook_result = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_RESIN)+" 1";
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->type = CMT_PLANT;
	f->dig_time = 0.15;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_list_add("creative",i,1,0);
	content_list_add("cooking",i,1,0);

	i = CONTENT_TRIMMED_LEAVES_WINTER;
	f = &content_features(i);
	f->description = gettext("Trimmed Leaves");
	f->light_propagates = true;
	f->air_equivalent = true;
	f->walkable = false;
	f->climbable = true;
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_GLASSLIKE;
	f->setAllTextures("leaves_winter.png");
#ifndef SERVER
	f->setAllTextureTypes(MATERIAL_ALPHA_SIMPLE);
#endif
	f->setInventoryTextureCube("leaves_winter.png", "leaves_winter.png", "leaves_winter.png");
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_LEAF;
	f->cook_result = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_ASH)+" 1";
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->type = CMT_PLANT;
	f->dig_time = 0.15;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_list_add("creative",i,1,0);
	content_list_add("cooking",i,1,0);

	i = CONTENT_TRIMMED_APPLE_LEAVES;
	f = &content_features(i);
	f->description = gettext("Trimmed Apple Tree Leaves");
	f->light_propagates = true;
	f->air_equivalent = true;
	f->walkable = false;
	f->climbable = true;
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_GLASSLIKE;
	f->setAllTextures("apple_leaves.png");
#ifndef SERVER
	f->setAllTextureTypes(MATERIAL_ALPHA_SIMPLE);
#endif
	f->setInventoryTextureCube("apple_leaves.png", "apple_leaves.png", "apple_leaves.png");
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_LEAF;
	f->cook_result = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_RESIN)+" 1";
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->type = CMT_PLANT;
	f->dig_time = 0.15;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_list_add("creative",i,1,0);
	content_list_add("cooking",i,1,0);

	i = CONTENT_TRIMMED_JUNGLE_LEAVES;
	f = &content_features(i);
	f->description = gettext("Trimmed Jungle Leaves");
	f->light_propagates = true;
	f->air_equivalent = true;
	f->walkable = false;
	f->climbable = true;
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_GLASSLIKE;
	f->setAllTextures("jungleleaves.png");
#ifndef SERVER
	f->setAllTextureTypes(MATERIAL_ALPHA_SIMPLE);
#endif
	f->setInventoryTextureCube("jungleleaves.png", "jungleleaves.png", "jungleleaves.png");
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_LEAF;
	f->cook_result = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_RESIN)+" 1";
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->type = CMT_PLANT;
	f->dig_time = 0.15;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_list_add("creative",i,1,0);
	content_list_add("cooking",i,1,0);

	i = CONTENT_TRIMMED_CONIFER_LEAVES;
	f = &content_features(i);
	f->description = gettext("Trimmed Conifer Leaves");
	f->light_propagates = true;
	f->air_equivalent = true;
	f->walkable = false;
	f->climbable = true;
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_GLASSLIKE;
	f->setAllTextures("conifer_leaves.png");
#ifndef SERVER
	f->setAllTextureTypes(MATERIAL_ALPHA_SIMPLE);
#endif
	f->setInventoryTextureCube("conifer_leaves.png", "conifer_leaves.png", "conifer_leaves.png");
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_LEAF;
	f->cook_result = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_RESIN)+" 1";
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->type = CMT_PLANT;
	f->dig_time = 0.15;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_list_add("creative",i,1,0);
	content_list_add("cooking",i,1,0);

	i = CONTENT_APPLE_BLOSSOM;
	f = &content_features(i);
	f->description = gettext("Apple Tree Blossom");
	f->light_propagates = true;
	f->air_equivalent = true;
	f->walkable = false;
	f->climbable = true;
	f->param_type = CPT_LIGHT;
	f->special_alternate_node = CONTENT_APPLE_LEAVES;
	f->draw_type = CDT_LEAFLIKE;
	f->setAllTextures("apple_leaves.png^apple_blossom.png");
#ifndef SERVER
	f->setAllTextureTypes(MATERIAL_ALPHA_SIMPLE);
	f->setAllTextureFlags(0);
#endif
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->extra_dug_item = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_APPLE_BLOSSOM)+" 1";
	f->extra_dug_item_rarity = 1;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_APPLE_LEAVES)+" 1";
	f->type = CMT_PLANT;
	f->dig_time = 0.15;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_list_add("decrafting",i,1,0);

	i = CONTENT_TRIMMED_APPLE_BLOSSOM;
	f = &content_features(i);
	f->description = gettext("Trimmed Apple Tree Blossom");
	f->light_propagates = true;
	f->air_equivalent = true;
	f->walkable = false;
	f->climbable = true;
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_GLASSLIKE;
	f->setAllTextures("apple_leaves.png^apple_blossom.png");
#ifndef SERVER
	f->setAllTextureTypes(MATERIAL_ALPHA_SIMPLE);
#endif
	f->setInventoryTextureCube("apple_leaves.png^apple_blossom.png", "apple_leaves.png^apple_blossom.png", "apple_leaves.png^apple_blossom.png");
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->type = CMT_PLANT;
	f->dig_time = 0.15;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_list_add("creative",i,1,0);

	i = CONTENT_CACTUS_BLOSSOM;
	f = &content_features(i);
	f->description = gettext("Cactus Blossom");
	f->setInventoryTexture("cactus_blossom.png");
	f->setAllTextures("cactus_blossom.png");
	f->setAllTextureFlags(0);
	f->light_propagates = true;
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_PLANTLIKE;
	f->air_equivalent = false; // grass grows underneath
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->walkable = false;
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_FLOWER;
	f->type = CMT_PLANT;
	f->dig_time = 0.20;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_list_add("creative",i,1,0);

	i = CONTENT_CACTUS_FLOWER;
	f = &content_features(i);
	f->description = gettext("Cactus Flower");
	f->setInventoryTexture("cactus_flower.png");
	f->setAllTextures("cactus_flower.png");
	f->setAllTextureFlags(0);
	f->light_propagates = true;
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_PLANTLIKE;
	f->air_equivalent = true;
	f->dug_item = std::string("MaterialItem2 ")+itos(CONTENT_CACTUS_BLOSSOM)+" 1";
	f->walkable = false;
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_FLOWER;
	f->type = CMT_PLANT;
	f->dig_time = 0.20;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_list_add("creative",i,1,0);
	content_list_add("decrafting",i,1,0);

	i = CONTENT_CACTUS_FRUIT;
	f = &content_features(i);
	f->description = gettext("Cactus Berry");
	f->setInventoryTexture("cactus_fruit.png");
	f->setAllTextures("cactus_fruit.png");
	f->setAllTextureFlags(0);
	f->light_propagates = true;
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_PLANTLIKE;
	f->air_equivalent = true;
	f->dug_item = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_CACTUS_FRUIT)+" 1";
	f->walkable = false;
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_FRUIT;
	f->type = CMT_PLANT;
	f->dig_time = 0.20;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_list_add("creative",i,1,0);

	i = CONTENT_CACTUS;
	f = &content_features(i);
	f->description = gettext("Cactus");
	f->setAllTextures("cactus_side.png");
	f->setTexture(0, "cactus_top.png");
	f->setTexture(1, "cactus_top.png");
	f->draw_type = CDT_NODEBOX;
	f->param_type = CPT_LIGHT;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->damage_per_second = 10;
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_CACTUS;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->extra_dug_item = std::string("CraftItem2 ") + itos (CONTENT_CRAFTITEM_MUSH) + " 1";
	f->extra_dug_item_rarity = 10;
	content_nodebox_cactus(f);
	f->setInventoryTextureCube("cactus_top.png", "cactus_side.png", "cactus_side.png");
	f->type = CMT_WOOD;
	f->dig_time = 0.75;
	f->pressure_type = CST_CRUSHABLE;
	content_list_add("creative",i,1,0);

	i = CONTENT_PAPYRUS;
	f = &content_features(i);
	f->description = gettext("Papyrus");
	f->setInventoryTexture("papyrus.png");
	f->setAllTextures("papyrus.png");
	f->setAllTextureFlags(0);
	f->light_propagates = true;
	f->param_type = CPT_LIGHT;
	f->param2_type = CPT_PLANTGROWTH;
	f->draw_type = CDT_PLANTLIKE;
	f->plantlike_tiled = true;
	f->plantgrowth_large_dug_node = CONTENT_PAPYRUS;
	f->plantgrowth_large_count = 1;
	f->plantgrowth_max_height = 5;
	f->walkable = false;
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_BUSH;
	f->type = CMT_PLANT;
	f->dig_time = 0.25;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_list_add("creative",i,1,0);

	i = CONTENT_SAPLING;
	f = &content_features(i);
	f->description = gettext("Sapling");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_PLANTLIKE;
	f->setAllTextures("sapling.png");
	f->setInventoryTexture("sapling.png");
	f->setAllTextureFlags(0);
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->light_propagates = true;
	f->air_equivalent = false;
	f->walkable = false;
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_SAPLING;
	f->type = CMT_WOOD;
	f->dig_time = 0.1;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	f->fertilizer_affects = true;
	content_list_add("creative",i,1,0);

	i = CONTENT_APPLE_SAPLING;
	f = &content_features(i);
	f->description = gettext("Apple Tree Sapling");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_PLANTLIKE;
	f->setAllTextures("apple_sapling.png");
	f->setInventoryTexture("apple_sapling.png");
	f->setAllTextureFlags(0);
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->light_propagates = true;
	f->air_equivalent = false;
	f->walkable = false;
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_SAPLING;
	f->type = CMT_WOOD;
	f->dig_time = 0.1;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	f->fertilizer_affects = true;
	content_list_add("creative",i,1,0);

	i = CONTENT_JUNGLESAPLING;
	f = &content_features(i);
	f->description = gettext("Jungle Sapling");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_PLANTLIKE;
	f->setAllTextures("junglesapling.png");
	f->setInventoryTexture("junglesapling.png");
	f->setAllTextureFlags(0);
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->light_propagates = true;
	f->air_equivalent = false;
	f->walkable = false;
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_SAPLING;
	f->type = CMT_WOOD;
	f->dig_time = 0.1;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	f->fertilizer_affects = true;
	content_list_add("creative",i,1,0);

	i = CONTENT_CONIFER_SAPLING;
	f = &content_features(i);
	f->description = gettext("Conifer Sapling");
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_PLANTLIKE;
	f->setAllTextures("conifer_sapling.png");
	f->setInventoryTexture("conifer_sapling.png");
	f->setAllTextureFlags(0);
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->light_propagates = true;
	f->air_equivalent = false;
	f->walkable = false;
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_SAPLING;
	f->type = CMT_WOOD;
	f->dig_time = 0.1;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	f->fertilizer_affects = true;
	content_list_add("creative",i,1,0);

	i = CONTENT_APPLE;
	f = &content_features(i);
	f->description = gettext("Apple");
	f->setInventoryTexture("apple.png");
	f->setAllTextures("apple.png");
	f->setAllTextureFlags(0);
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_PLANTLIKE;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->walkable = false;
	f->air_equivalent = true;
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_FRUIT;
	f->dug_item = std::string("CraftItem2 ") +itos(CONTENT_CRAFTITEM_APPLE)+" 1";
	f->ondig_replace_node = CONTENT_APPLE_LEAVES;
	f->ondig_replace_node_requires = CONTENT_APPLE_TREE;
	f->type = CMT_PLANT;
	f->dig_time = 0.0;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_list_add("creative",i,1,0);

	// plants
	i = CONTENT_WILDGRASS_SHORT;
	f = &content_features(i);
	f->description = gettext("Wild Grass");
	f->setInventoryTexture("wildgrass_short.png");
	f->setAllTextures("wildgrass_short.png");
	f->setAllTextureFlags(0);
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_PLANTLIKE;
	f->air_equivalent = true;
	f->ondig_special_drop = CONTENT_CRAFTITEM_STRAW;
	f->ondig_special_tool = TT_KNIFE;
	f->walkable = false;
	f->material_pointable = false;
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_GRASS;
	//f->pointable = false;
	f->buildable_to = true;
	f->type = CMT_PLANT;
	f->dig_time = 0.10;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;

	i = CONTENT_WILDGRASS_LONG;
	f = &content_features(i);
	f->description = gettext("Wild Grass");
	f->setInventoryTexture("wildgrass_long.png");
	f->setAllTextures("wildgrass_long.png");
	f->setAllTextureFlags(0);
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_PLANTLIKE;
	f->air_equivalent = true;
	f->ondig_special_drop = CONTENT_CRAFTITEM_STRAW;
	f->ondig_special_tool = TT_KNIFE;
	f->walkable = false;
	f->material_pointable = false;
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_GRASS;
	f->buildable_to = true;
	f->type = CMT_PLANT;
	f->dig_time = 0.15;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;

	i = CONTENT_DEADGRASS;
	f = &content_features(i);
	f->description = gettext("Dead Grass");
	f->setInventoryTexture("deadgrass.png");
	f->setAllTextures("deadgrass.png");
	f->setAllTextureFlags(0);
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->air_equivalent = true;
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_PLANTLIKE;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->walkable = false;
	f->material_pointable = false;
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_GRASS;
	f->buildable_to = true;
	f->type = CMT_PLANT;
	f->dig_time = 0.10;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;

	i = CONTENT_FLOWER_STEM;
	f = &content_features(i);
	f->description = gettext("Flower Stem");
	f->setInventoryTexture("flower_stem.png");
	f->setAllTextures("flower_stem.png");
	f->setAllTextureFlags(0);
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->air_equivalent = true;
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_PLANTLIKE;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->walkable = false;
	f->material_pointable = false;
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_GRASS;
	f->buildable_to = true;
	f->type = CMT_PLANT;
	f->dig_time = 0.10;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;

	i = CONTENT_FLOWER_ROSE;
	f = &content_features(i);
	f->description = gettext("Rose");
	f->setInventoryTexture("flower_rose.png");
	f->setAllTextures("flower_rose.png");
	f->setAllTextureFlags(0);
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->air_equivalent = true;
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_PLANTLIKE;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->walkable = false;
	f->material_pointable = false;
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_GRASS;
	f->buildable_to = true;
	f->type = CMT_PLANT;
	f->dig_time = 0.10;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_list_add("creative",i,1,0);

	i = CONTENT_FLOWER_DAFFODIL;
	f = &content_features(i);
	f->description = gettext("Daffodil");
	f->setInventoryTexture("flower_daffodil.png");
	f->setAllTextures("flower_daffodil.png");
	f->setAllTextureFlags(0);
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->air_equivalent = true;
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_PLANTLIKE;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->walkable = false;
	f->material_pointable = false;
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_GRASS;
	f->buildable_to = true;
	f->type = CMT_PLANT;
	f->dig_time = 0.10;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_list_add("creative",i,1,0);

	i = CONTENT_FLOWER_TULIP;
	f = &content_features(i);
	f->description = gettext("Tulip");
	f->setInventoryTexture("flower_tulip.png");
	f->setAllTextures("flower_tulip.png");
	f->setAllTextureFlags(0);
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->air_equivalent = true;
	f->param_type = CPT_LIGHT;
	f->draw_type = CDT_PLANTLIKE;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->walkable = false;
	f->material_pointable = false;
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_GRASS;
	f->buildable_to = true;
	f->type = CMT_PLANT;
	f->dig_time = 0.10;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_list_add("creative",i,1,0);

	i = CONTENT_SEEDS_TEA;
	f = &content_features(i);
	f->description = gettext("Tea Seeds");
	f->setAllTextures("farm_seeds_tea.png");
	f->draw_type = CDT_PLANTLIKE;
	f->param_type = CPT_LIGHT;
	f->setAllTextureFlags(0);
	f->walkable = false;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->air_equivalent = true;
	f->special_alternate_node = CONTENT_TEA;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->type = CMT_PLANT;
	f->dig_time = 0.4;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_list_add("creative",i,1,0);

	i = CONTENT_TEA;
	f = &content_features(i);
	f->description = gettext("Tea Plant");
	f->setInventoryTexture("plant_tea.png");
	f->setAllTextures("plant_tea.png");
	f->setAllTextureFlags(0);
	f->light_propagates = true;
	f->param_type = CPT_LIGHT;
	f->param2_type = CPT_PLANTGROWTH;
	f->draw_type = CDT_PLANTLIKE;
	f->plantgrowth_small_dug_node = CONTENT_SEEDS_TEA;
	f->plantgrowth_large_dug_node = CONTENT_CRAFTITEM_TEA_LEAVES;
	f->plantgrowth_large_gives_small = true;
	f->walkable = false;
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_BUSH;
	f->buildable_to = true;
	f->type = CMT_PLANT;
	f->dig_time = 0.15;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_list_add("creative",i,1,0);

	i = CONTENT_BEANS_COFFEE;
	f = &content_features(i);
	f->description = gettext("Coffee Beans");
	f->setAllTextures("farm_seeds_coffee.png");
	f->draw_type = CDT_PLANTLIKE;
	f->param_type = CPT_LIGHT;
	f->setAllTextureFlags(0);
	f->walkable = false;
	f->light_propagates = true;
	f->sunlight_propagates = true;
	f->air_equivalent = true;
	f->special_alternate_node = CONTENT_COFFEE;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->cook_result = std::string("CraftItem2 ")+itos(CONTENT_CRAFTITEM_COFFEE_BEANS)+" 1";
	f->type = CMT_PLANT;
	f->dig_time = 0.4;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_list_add("creative",i,1,0);

	i = CONTENT_COFFEE;
	f = &content_features(i);
	f->description = gettext("Coffee Plant");
	f->setInventoryTexture("plant_coffe.png");
	f->setAllTextures("plant_coffee.png");
	f->setAllTextureFlags(0);
	f->light_propagates = true;
	f->param_type = CPT_LIGHT;
	f->param2_type = CPT_PLANTGROWTH;
	f->draw_type = CDT_PLANTLIKE;
	f->plantgrowth_small_dug_node = CONTENT_BEANS_COFFEE;
	f->plantgrowth_large_dug_node = CONTENT_BEANS_COFFEE;
	f->walkable = false;
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_BUSH;
	f->buildable_to = true;
	f->type = CMT_PLANT;
	f->dig_time = 0.15;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	content_list_add("creative",i,1,0);

	i = CONTENT_BUSH_BLUEBERRY;
	f = &content_features(i);
	f->description = gettext("Blueberry Bush");
	f->light_propagates = true;
	f->air_equivalent = true;
	f->param_type = CPT_LIGHT;
	f->param2_type = CPT_PLANTGROWTH;
	f->draw_type = CDT_BUSHLIKE;
	f->ondig_gives_inventory = true;
	f->setAllTextures("bush_blueberry.png");
	f->setAllMetaTextures("bush_blueberry_berry.png");
#ifndef SERVER
	f->setAllTextureTypes(MATERIAL_ALPHA_SIMPLE);
	f->setAllTextureFlags(0);
	f->setAllMetaTextureTypes(MATERIAL_ALPHA_SIMPLE);
	f->setAllMetaTextureFlags(0);
#endif
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_BUSH;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->type = CMT_PLANT;
	f->dig_time = 0.15;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	f->onpunch_gives_inventory = true;
	f->special_alternate_node = CONTENT_CRAFTITEM_BLUEBERRY;
	if(f->initial_metadata == NULL)
		f->initial_metadata = new BushNodeMetadata();
	content_list_add("creative",i,1,0);

	i = CONTENT_BUSH_RASPBERRY;
	f = &content_features(i);
	f->description = gettext("Raspberry Bush");
	f->light_propagates = true;
	f->air_equivalent = true;
	f->param_type = CPT_LIGHT;
	f->param2_type = CPT_PLANTGROWTH;
	f->draw_type = CDT_BUSHLIKE;
	f->ondig_gives_inventory = true;
	f->setAllTextures("bush_raspberry.png");
	f->setAllMetaTextures("bush_raspberry_berry.png");
#ifndef SERVER
	f->setAllTextureTypes(MATERIAL_ALPHA_SIMPLE);
	f->setAllTextureFlags(0);
	f->setAllMetaTextureTypes(MATERIAL_ALPHA_SIMPLE);
	f->setAllMetaTextureFlags(0);
#endif
	f->flammable = 1; // can be replaced by fire if the node under it is set on fire
	f->fuel_time = BT_BUSH;
	f->dug_item = std::string("MaterialItem2 ")+itos(i)+" 1";
	f->type = CMT_PLANT;
	f->dig_time = 0.15;
	f->pressure_type = CST_CRUSHABLE;
	f->suffocation_per_second = 0;
	f->onpunch_gives_inventory = true;
	f->special_alternate_node = CONTENT_CRAFTITEM_RASPBERRY;
	if(f->initial_metadata == NULL)
		f->initial_metadata = new BushNodeMetadata();
	content_list_add("creative",i,1,0);
}
