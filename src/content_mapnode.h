/************************************************************************
* Minetest-c55
* Copyright (C) 2010-2011 celeron55, Perttu Ahola <celeron55@gmail.com>
*
* content_mapnode.h
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

#ifndef CONTENT_MAPNODE_HEADER
#define CONTENT_MAPNODE_HEADER

#include "mapnode.h"

void content_mapnode_init(bool repeat);
void content_mapnode_circuit(bool repeat);
void content_mapnode_farm(bool repeat);
void content_mapnode_furniture(bool repeat);
void content_mapnode_door(bool repeat);
void content_mapnode_stair(bool repeat);
void content_mapnode_slab(bool repeat);
void content_mapnode_special(bool repeat);
void content_mapnode_sign(bool repeat);
void content_mapnode_plants(bool repeat);

MapNode mapnode_translate_from_internal(MapNode n_from, u8 version);
MapNode mapnode_translate_to_internal(MapNode n_from, u8 version);

#define WATER_ALPHA 160
#define WATER_VISC 1
#define LAVA_VISC 7

/*
	Node content type IDs
	Range: 0x0 - 0xFFF
*/

#define CONTENT_STONE 0x000
#define CONTENT_LIMESTONE 0x001
#define CONTENT_WATER 0x002
#define CONTENT_ROCK 0x003
#define CONTENT_GLASSLIGHT 0x004
#define CONTENT_CHAIR 0x005
#define CONTENT_FORGE 0x006
#define CONTENT_TORCH 0x007
#define CONTENT_TABLE 0x008
#define CONTENT_WATERSOURCE 0x009
#define CONTENT_FORGE_FIRE 0x00A
#define CONTENT_MARBLE 0x00B
#define CONTENT_SPACEROCK 0x00C
#define CONTENT_SAFE 0x00D
#define CONTENT_SIGN_WALL 0x00E
#define CONTENT_CHEST_DEPRECATED 0x00F
#define CONTENT_FURNACE_DEPRECATED 0x010
#define CONTENT_LOCKABLE_CHEST_DEPRECATED 0x011
#define CONTENT_SIGN 0x012
#define CONTENT_SIGN_UD 0x013
#define CONTENT_LOCKABLE_FURNACE_DEPRECATED 0x014
#define CONTENT_FENCE 0x015
#define CONTENT_LOCKABLE_SIGN_WALL 0x016
#define CONTENT_LOCKABLE_SIGN 0x017
#define CONTENT_LOCKABLE_SIGN_UD 0x018
#define CONTENT_CREATIVE_CHEST 0x019
#define CONTENT_FURNACE 0x01A
#define CONTENT_SMELTERY 0x01B
#define CONTENT_IRON_FENCE 0x01C
#define CONTENT_IRON_BARS 0x01D
#define CONTENT_RAIL 0x01E
#define CONTENT_DESERT_SAND 0x01F
#define CONTENT_LAVA 0x020
#define CONTENT_LAVASOURCE 0x021
#define CONTENT_LADDER_WALL 0x022
#define CONTENT_LADDER_FLOOR 0x023
#define CONTENT_LADDER_ROOF 0x024
// walls
#define CONTENT_ROUGHSTONE_WALL 0x025
#define CONTENT_MOSSYCOBBLE_WALL 0x026
#define CONTENT_SANDSTONE_WALL 0x027
#define CONTENT_STONE_WALL 0x028
#define CONTENT_COBBLE_WALL 0x029
// fences
#define CONTENT_JUNGLE_FENCE 0x02A
#define CONTENT_PINE_FENCE 0x02B
// more walls
#define CONTENT_LIMESTONE_WALL 0x02C
#define CONTENT_MARBLE_WALL 0x02D
#define CONTENT_CAMPFIRE 0x02E
#define CONTENT_CHEST 0x02F
#define CONTENT_CHEST_PINE 0x030
#define CONTENT_CHEST_JUNGLE 0x031
#define CONTENT_CHEST_APPLE 0x032
#define CONTENT_SIGN_JUNGLE 0x033
#define CONTENT_SIGN_APPLE 0x034
#define CONTENT_SIGN_PINE 0x035
#define CONTENT_SIGN_IRON 0x036
#define CONTENT_SIGN_WALL_JUNGLE 0x037
#define CONTENT_SIGN_WALL_APPLE 0x038
#define CONTENT_SIGN_WALL_PINE 0x039
#define CONTENT_SIGN_WALL_IRON 0x03A
#define CONTENT_SIGN_UD_JUNGLE 0x03B
#define CONTENT_SIGN_UD_APPLE 0x03C
#define CONTENT_SIGN_UD_PINE 0x03D
#define CONTENT_SIGN_UD_IRON 0x03E
#define CONTENT_APPLE_FENCE 0x03F
#define CONTENT_BUSH_BLUEBERRY 0x040
#define CONTENT_BUSH_RASPBERRY 0x041
#define CONTENT_WOOD_BARREL 0x042
#define CONTENT_APPLEWOOD_BARREL 0x043
#define CONTENT_JUNGLEWOOD_BARREL 0x044
#define CONTENT_PINE_BARREL 0x045
#define CONTENT_WOOD_BARREL_SEALED 0x046
#define CONTENT_APPLEWOOD_BARREL_SEALED 0x047
#define CONTENT_JUNGLEWOOD_BARREL_SEALED 0x048
#define CONTENT_PINE_BARREL_SEALED 0x049
#define CONTENT_CLAY_VESSEL 0x04A
#define CONTENT_CLAY_VESSEL_RAW 0x04B
#define CONTENT_DESERT_SANDSTONE 0x04C
#define CONTENT_DESERT_SANDSTONE_BRICK 0x04D
#define CONTENT_DESERT_SANDSTONE_BLOCK 0x04E
// FREE 0x04F-0x07C
// 0x7D-0x7F reserved values, air, ignore, etc
#define CONTENT_CHAIR_CENTRE 0x080
#define CONTENT_CHAIR_ENDL 0x081
#define CONTENT_CHAIR_ENDR 0x082
#define CONTENT_CHAIR_INNER 0x083
#define CONTENT_CHAIR_OUTER 0x084
#define CONTENT_JUNGLEFERN 0x085

// columns
#define CONTENT_STONE_COLUMN_SQUARE 0x086
#define CONTENT_STONE_COLUMN_SQUARE_BASE 0x087
#define CONTENT_STONE_COLUMN_SQUARE_TOP 0x088
#define CONTENT_STONE_COLUMN_CROSS 0x089
#define CONTENT_STONE_COLUMN_CROSS_BASE 0x08A
#define CONTENT_STONE_COLUMN_CROSS_TOP 0x08B
#define CONTENT_LIMESTONE_COLUMN_SQUARE 0x08C
#define CONTENT_LIMESTONE_COLUMN_SQUARE_BASE 0x08D
#define CONTENT_LIMESTONE_COLUMN_SQUARE_TOP 0x08E
#define CONTENT_LIMESTONE_COLUMN_CROSS 0x08F
#define CONTENT_LIMESTONE_COLUMN_CROSS_BASE 0x090
#define CONTENT_LIMESTONE_COLUMN_CROSS_TOP 0x091
#define CONTENT_MARBLE_COLUMN_SQUARE 0x092
#define CONTENT_MARBLE_COLUMN_SQUARE_BASE 0x093
#define CONTENT_MARBLE_COLUMN_SQUARE_TOP 0x094
#define CONTENT_MARBLE_COLUMN_CROSS 0x095
#define CONTENT_MARBLE_COLUMN_CROSS_BASE 0x099
#define CONTENT_MARBLE_COLUMN_CROSS_TOP 0x09A
#define CONTENT_SANDSTONE_COLUMN_SQUARE 0x09B
#define CONTENT_SANDSTONE_COLUMN_SQUARE_BASE 0x09C
#define CONTENT_SANDSTONE_COLUMN_SQUARE_TOP 0x09D
#define CONTENT_SANDSTONE_COLUMN_CROSS 0x09E
#define CONTENT_SANDSTONE_COLUMN_CROSS_BASE 0x09F
#define CONTENT_SANDSTONE_COLUMN_CROSS_TOP 0x0A0
#define CONTENT_BRICK_COLUMN_SQUARE 0x0A1
#define CONTENT_BRICK_COLUMN_SQUARE_BASE 0x0A2
#define CONTENT_BRICK_COLUMN_SQUARE_TOP 0x0A3
#define CONTENT_BRICK_COLUMN_CROSS 0x0A4
#define CONTENT_BRICK_COLUMN_CROSS_BASE 0x0A5
#define CONTENT_BRICK_COLUMN_CROSS_TOP 0x0A6
#define CONTENT_WOOD_COLUMN_SQUARE 0x0A7
#define CONTENT_WOOD_COLUMN_SQUARE_BASE 0x0A8
#define CONTENT_WOOD_COLUMN_SQUARE_TOP 0x0A9
#define CONTENT_WOOD_COLUMN_CROSS 0x0AA
#define CONTENT_WOOD_COLUMN_CROSS_BASE 0x0AB
#define CONTENT_WOOD_COLUMN_CROSS_TOP 0x0AC
#define CONTENT_JUNGLEWOOD_COLUMN_SQUARE 0x0AD
#define CONTENT_JUNGLEWOOD_COLUMN_SQUARE_BASE 0x0AE
#define CONTENT_JUNGLEWOOD_COLUMN_SQUARE_TOP 0x0AF
#define CONTENT_JUNGLEWOOD_COLUMN_CROSS 0x0B0
#define CONTENT_JUNGLEWOOD_COLUMN_CROSS_BASE 0x0B1
#define CONTENT_JUNGLEWOOD_COLUMN_CROSS_TOP 0x0B2
#define CONTENT_WOOD_PINE_COLUMN_SQUARE 0x0B3
#define CONTENT_WOOD_PINE_COLUMN_SQUARE_BASE 0x0B4
#define CONTENT_WOOD_PINE_COLUMN_SQUARE_TOP 0x0B5
#define CONTENT_WOOD_PINE_COLUMN_CROSS 0x0B6
#define CONTENT_WOOD_PINE_COLUMN_CROSS_BASE 0x0B7
#define CONTENT_WOOD_PINE_COLUMN_CROSS_TOP 0x0B8
#define CONTENT_ROUGHSTONE_COLUMN_SQUARE 0x0B9
#define CONTENT_ROUGHSTONE_COLUMN_SQUARE_BASE 0x0BA
#define CONTENT_ROUGHSTONE_COLUMN_SQUARE_TOP 0x0BB
#define CONTENT_ROUGHSTONE_COLUMN_CROSS 0x0BC
#define CONTENT_ROUGHSTONE_COLUMN_CROSS_BASE 0x0BD
#define CONTENT_ROUGHSTONE_COLUMN_CROSS_TOP 0x0BE
#define CONTENT_APPLEWOOD_COLUMN_SQUARE 0X0BF
#define CONTENT_APPLEWOOD_COLUMN_SQUARE_BASE 0X0C0
#define CONTENT_APPLEWOOD_COLUMN_SQUARE_TOP 0X0C1
#define CONTENT_APPLEWOOD_COLUMN_CROSS 0x0C2
#define CONTENT_APPLEWOOD_COLUMN_CROSS_BASE 0x0C3
#define CONTENT_APPLEWOOD_COLUMN_CROSS_TOP 0x0C4
// FREE 0x0C5-0x7F5
#define CONTENT_APPLEWOOD 0x7F6
#define CONTENT_LEAVES_SNOWY 0x7F7
#define CONTENT_TRIMMED_LEAVES_AUTUMN 0x7F8
#define CONTENT_TRIMMED_LEAVES_WINTER 0x7F9
#define CONTENT_LEAVES_AUTUMN 0x7FA
#define CONTENT_LEAVES_WINTER 0x7FB
#define CONTENT_GRASS_AUTUMN 0x7FC
#define CONTENT_GROWING_GRASS_AUTUMN 0x7FD
#define CONTENT_GRASS_FOOTSTEPS_AUTUMN 0x7FE
#define CONTENT_GROWING_GRASS 0x7FF
#define CONTENT_GRASS 0x800
#define CONTENT_TREE 0x801
#define CONTENT_LEAVES 0x802
#define CONTENT_FARM_DIRT 0x803
// FREE 0x804
#define CONTENT_MUD 0x805
#define CONTENT_COTTON 0x806
#define CONTENT_BORDERSTONE 0x807
#define CONTENT_WOOD 0x808
#define CONTENT_SAND 0x809
#define CONTENT_ROUGHSTONE 0x80a
#define CONTENT_IRON 0x80b
#define CONTENT_GLASS 0x80c
#define CONTENT_MOSSYCOBBLE 0x80d
#define CONTENT_GRAVEL 0x80e
#define CONTENT_SANDSTONE 0x80f
#define CONTENT_CACTUS 0x810
#define CONTENT_BRICK 0x811
#define CONTENT_CLAY 0x812
#define CONTENT_PAPYRUS 0x813
#define CONTENT_BOOKSHELF 0x814
#define CONTENT_JUNGLETREE 0x815
#define CONTENT_JUNGLEGRASS 0x816
#define CONTENT_NC 0x817
#define CONTENT_NC_RB 0x818
#define CONTENT_APPLE 0x819
#define CONTENT_JUNGLEWOOD 0x81a
#define CONTENT_STONEBRICK 0x81b
#define CONTENT_STONEBLOCK 0x81c
#define CONTENT_SAPLING 0x820
// paintings
#define CONTENT_PAINTING_RED 0x821
#define CONTENT_PAINTING_BLUE 0x822
#define CONTENT_PAINTING_GREEN 0x823
#define CONTENT_PAINTING_CANVAS 0x824
#define CONTENT_PAINTING_WHITE 0x825

// FREE 826-82E
#define CONTENT_PARCEL 0x82F
#define CONTENT_CLOCK 0x830
#define CONTENT_BOOKSHELF_PINE 0x831
#define CONTENT_BOOKSHELF_JUNGLE 0x832
#define CONTENT_BOOKSHELF_APPLE 0x833
// FREE 834-835
#define CONTENT_COUCH_CENTRE 0x836
#define CONTENT_COUCH_ENDL 0x837
#define CONTENT_COUCH_ENDR 0x838
#define CONTENT_COUCH_INNER 0x839
#define CONTENT_COUCH_OUTER 0x83A
#define CONTENT_COUCH_CHAIR 0x83B
#define CONTENT_COUCH_CENTRE_BLUE 0x83C
#define CONTENT_COUCH_ENDL_BLUE 0x83D
#define CONTENT_COUCH_ENDR_BLUE 0x83E
#define CONTENT_COUCH_INNER_BLUE 0x83F
#define CONTENT_COUCH_OUTER_BLUE 0x840
#define CONTENT_COUCH_CHAIR_BLUE 0x841
#define CONTENT_COUCH_CENTRE_GREEN 0x842
#define CONTENT_COUCH_ENDL_GREEN 0x843
#define CONTENT_COUCH_ENDR_GREEN 0x844
#define CONTENT_COUCH_INNER_GREEN 0x845
#define CONTENT_COUCH_OUTER_GREEN 0x846
#define CONTENT_COUCH_CHAIR_GREEN 0x847
#define CONTENT_COUCH_CENTRE_ORANGE 0x848
#define CONTENT_COUCH_ENDL_ORANGE 0x849
#define CONTENT_COUCH_ENDR_ORANGE 0x84A
#define CONTENT_COUCH_INNER_ORANGE 0x84B
#define CONTENT_COUCH_OUTER_ORANGE 0x84C
#define CONTENT_COUCH_CHAIR_ORANGE 0x84D
#define CONTENT_COUCH_CENTRE_PURPLE 0x84E
#define CONTENT_COUCH_ENDL_PURPLE 0x84F
#define CONTENT_COUCH_ENDR_PURPLE 0x850
#define CONTENT_COUCH_INNER_PURPLE 0x851
#define CONTENT_COUCH_OUTER_PURPLE 0x852
#define CONTENT_COUCH_CHAIR_PURPLE 0x853
#define CONTENT_COUCH_CENTRE_RED 0x854
#define CONTENT_COUCH_ENDL_RED 0x855
#define CONTENT_COUCH_ENDR_RED 0x856
#define CONTENT_COUCH_INNER_RED 0x857
#define CONTENT_COUCH_OUTER_RED 0x858
#define CONTENT_COUCH_CHAIR_RED 0x859
#define CONTENT_COUCH_CENTRE_YELLOW 0x85A
#define CONTENT_COUCH_ENDL_YELLOW 0x85B
#define CONTENT_COUCH_ENDR_YELLOW 0x85C
#define CONTENT_COUCH_INNER_YELLOW 0x85D
#define CONTENT_COUCH_OUTER_YELLOW 0x85E
#define CONTENT_COUCH_CHAIR_YELLOW 0x85F
#define CONTENT_COUCH_CENTRE_BLACK 0x860
#define CONTENT_COUCH_ENDL_BLACK 0x861
#define CONTENT_COUCH_ENDR_BLACK 0x862
#define CONTENT_COUCH_INNER_BLACK 0x863
#define CONTENT_COUCH_OUTER_BLACK 0x864
#define CONTENT_COUCH_CHAIR_BLACK 0x865
#define CONTENT_CAULDRON 0x866
#define CONTENT_CRUSHER 0x867
// FREE 868-86F
#define CONTENT_FLAG 0x870
#define CONTENT_FLAG_BLUE 0x871
#define CONTENT_FLAG_GREEN 0x872
#define CONTENT_FLAG_ORANGE 0x873
#define CONTENT_FLAG_PURPLE 0x874
#define CONTENT_FLAG_RED 0x875
#define CONTENT_FLAG_YELLOW 0x876
#define CONTENT_FLAG_BLACK 0x877
// FREE 878-87F
// plants
#define CONTENT_WILDGRASS_SHORT 0x880
#define CONTENT_WILDGRASS_LONG 0x881
#define CONTENT_DEADGRASS 0x882
#define CONTENT_FLOWER_STEM 0x883
#define CONTENT_FLOWER_ROSE 0x884
#define CONTENT_FLOWER_DAFFODIL 0x885
#define CONTENT_FLOWER_TULIP 0x886
#define CONTENT_FLOWER_POT_RAW 0x887
#define CONTENT_FLOWER_POT 0x888
#define CONTENT_TRIMMED_LEAVES 0x889
#define CONTENT_PLANTS_MIN 0x880
#define CONTENT_PLANTS_MAX 0x886

// coloured cotton
#define CONTENT_COTTON_BLUE 0x890
#define CONTENT_COTTON_GREEN 0x891
#define CONTENT_COTTON_ORANGE 0x892
#define CONTENT_COTTON_PURPLE 0x893
#define CONTENT_COTTON_RED 0x894
#define CONTENT_COTTON_YELLOW 0x895
#define CONTENT_COTTON_BLACK 0x896
// FREE 0x897-0x89F
// sponge
#define CONTENT_SPONGE 0x8A0
#define CONTENT_SPONGE_FULL 0x8A1
// more
#define CONTENT_HAY 0x8A2
#define CONTENT_SANDSTONE_BRICK 0x8A3
#define CONTENT_SANDSTONE_BLOCK 0x8A4
#define CONTENT_TERRACOTTA 0x8A5
#define CONTENT_TERRACOTTA_BRICK 0x8A6
#define CONTENT_TERRACOTTA_BLOCK 0x8A7
#define CONTENT_TERRACOTTA_TILE 0x8A8
#define CONTENT_CLAY_BLUE 0x8A9
#define CONTENT_CLAY_GREEN 0x8AA
#define CONTENT_CLAY_ORANGE 0x8AB
#define CONTENT_CLAY_PURPLE 0x8AC
#define CONTENT_CLAY_RED 0x8AD
#define CONTENT_CLAY_YELLOW 0x8AE
#define CONTENT_CLAY_BLACK 0x8AF
#define CONTENT_GLASS_BLUE 0x8B0
#define CONTENT_GLASS_GREEN 0x8B1
#define CONTENT_GLASS_ORANGE 0x8B2
#define CONTENT_GLASS_PURPLE 0x8B3
#define CONTENT_GLASS_RED 0x8B4
#define CONTENT_GLASS_YELLOW 0x8B5
#define CONTENT_GLASS_BLACK 0x8B6
#define CONTENT_CARPET 0x8B7
#define CONTENT_CARPET_BLUE 0x8B8
#define CONTENT_CARPET_GREEN 0x8B9
#define CONTENT_CARPET_ORANGE 0x8BA
#define CONTENT_CARPET_PURPLE 0x8BB
#define CONTENT_CARPET_RED 0x8BC
#define CONTENT_CARPET_YELLOW 0x8BD
#define CONTENT_CARPET_BLACK 0x8BE
#define CONTENT_COAL 0x8BF
#define CONTENT_CHARCOAL 0x8C0
#define CONTENT_TIN 0x8C1
#define CONTENT_COPPER 0x8C2
#define CONTENT_SILVER 0x8C3
#define CONTENT_GOLD 0x8C4
#define CONTENT_QUARTZ 0x8C5
// fire and tnt
#define CONTENT_FIRE 0x8C6
#define CONTENT_FIRE_SHORTTERM 0x8C7
#define CONTENT_TNT 0x8C8
#define CONTENT_FLASH 0x8C9
#define CONTENT_MITHRIL_BLOCK 0x8CA
#define CONTENT_COBBLE 0x8CB
#define CONTENT_ROUGHSTONEBRICK 0x8CC
#define CONTENT_ROUGHSTONEBLOCK 0x8CD
#define CONTENT_STEAM 0x8CE
#define CONTENT_INCINERATOR 0x8CF
#define CONTENT_ASH 0x8D0
#define CONTENT_GRASS_FOOTSTEPS 0x8D1
#define CONTENT_TRELLIS 0x8D2
// books
#define CONTENT_BOOK 0x8D3
#define CONTENT_COOK_BOOK 0x8D4
#define CONTENT_DECRAFT_BOOK 0x8D5
#define CONTENT_DIARY_BOOK 0x8D6
#define CONTENT_RCRAFT_BOOK 0x8D7
// FREE 8D8-8D9
#define CONTENT_BOOK_OPEN 0x8DA
#define CONTENT_COOK_BOOK_OPEN 0x8DB
#define CONTENT_DECRAFT_BOOK_OPEN 0x8DC
#define CONTENT_DIARY_BOOK_OPEN 0x8DD
#define CONTENT_RCRAFT_BOOK_OPEN 0x8DE
// FREE 8DF-8EF
#define CONTENT_YOUNG_TREE 0x8F0
#define CONTENT_YOUNG_JUNGLETREE 0x8F1
#define CONTENT_YOUNG_APPLE_TREE 0x8F2
#define CONTENT_YOUNG_CONIFER_TREE 0x8F3
// FREE 8F4-8FF
// glass pane
#define CONTENT_GLASS_PANE 0x900
#define CONTENT_GLASS_PANE_BLUE 0x901
#define CONTENT_GLASS_PANE_GREEN 0x902
#define CONTENT_GLASS_PANE_ORANGE 0x903
#define CONTENT_GLASS_PANE_PURPLE 0x904
#define CONTENT_GLASS_PANE_RED 0x905
#define CONTENT_GLASS_PANE_YELLOW 0x906
#define CONTENT_GLASS_PANE_BLACK 0x907
// more more
#define CONTENT_JUNGLELEAVES 0x910
#define CONTENT_JUNGLESAPLING 0x911
#define CONTENT_APPLE_LEAVES 0x912
#define CONTENT_APPLE_SAPLING 0x913
#define CONTENT_APPLE_TREE 0x914
#define CONTENT_TRIMMED_APPLE_LEAVES 0x915
#define CONTENT_TRIMMED_JUNGLE_LEAVES 0x916
#define CONTENT_TRIMMED_CONIFER_LEAVES 0x917
#define CONTENT_APPLE_BLOSSOM 0x918
#define CONTENT_TRIMMED_APPLE_BLOSSOM 0x919
#define CONTENT_CACTUS_BLOSSOM 0x91A
#define CONTENT_CACTUS_FLOWER 0x91B
#define CONTENT_CACTUS_FRUIT 0x91C
#define CONTENT_FERTILIZER 0x91D
#define CONTENT_SEEDS_WHEAT 0x91E
#define CONTENT_SEEDS_MELON 0x91F
#define CONTENT_SEEDS_PUMPKIN 0x920
#define CONTENT_SEEDS_POTATO 0x921
#define CONTENT_SEEDS_CARROT 0x922
#define CONTENT_SEEDS_BEETROOT 0x923
#define CONTENT_SEEDS_GRAPE 0x924
#define CONTENT_SEEDS_COTTON 0x925
// FREE 925-930
#define CONTENT_FARM_WHEAT_1 0x930
#define CONTENT_FARM_WHEAT_2 0x931
#define CONTENT_FARM_WHEAT_3 0x932
#define CONTENT_FARM_WHEAT 0x933
#define CONTENT_FARM_MELON_1 0x934
#define CONTENT_FARM_MELON_2 0x935
#define CONTENT_FARM_MELON_3 0x936
#define CONTENT_FARM_MELON 0x937
#define CONTENT_FARM_PUMPKIN_1 0x938
#define CONTENT_FARM_PUMPKIN_2 0x939
#define CONTENT_FARM_PUMPKIN_3 0x93A
#define CONTENT_FARM_PUMPKIN 0x93B
#define CONTENT_FARM_POTATO_1 0x93C
#define CONTENT_FARM_POTATO_2 0x93D
#define CONTENT_FARM_POTATO_3 0x93E
#define CONTENT_FARM_POTATO 0x93F
#define CONTENT_FARM_CARROT_1 0x940
#define CONTENT_FARM_CARROT_2 0x941
#define CONTENT_FARM_CARROT_3 0x942
#define CONTENT_FARM_CARROT 0x943
#define CONTENT_FARM_BEETROOT_1 0x944
#define CONTENT_FARM_BEETROOT_2 0x945
#define CONTENT_FARM_BEETROOT_3 0x946
#define CONTENT_FARM_BEETROOT 0x947
#define CONTENT_FARM_GRAPEVINE_1 0x948
#define CONTENT_FARM_GRAPEVINE_2 0x949
#define CONTENT_FARM_GRAPEVINE_3 0x94A
#define CONTENT_FARM_GRAPEVINE 0x94B
#define CONTENT_FARM_COTTON_1 0x94C
#define CONTENT_FARM_COTTON_2 0x94D
#define CONTENT_FARM_COTTON_3 0x94E
#define CONTENT_FARM_COTTON 0x94F
#define CONTENT_FARM_TRELLIS_GRAPE_1 0x950
#define CONTENT_FARM_TRELLIS_GRAPE_2 0x951
#define CONTENT_FARM_TRELLIS_GRAPE_3 0x952
#define CONTENT_FARM_TRELLIS_GRAPE 0x953
#define CONTENT_APPLE_PIE_RAW 0x954
#define CONTENT_APPLE_PIE 0x955
#define CONTENT_APPLE_PIE_3 0x956
#define CONTENT_APPLE_PIE_2 0x957
#define CONTENT_APPLE_PIE_1 0x958
#define CONTENT_PUMPKIN_PIE_RAW 0x959
#define CONTENT_PUMPKIN_PIE 0x95A
#define CONTENT_PUMPKIN_PIE_3 0x95B
#define CONTENT_PUMPKIN_PIE_2 0x95C
#define CONTENT_PUMPKIN_PIE_1 0x95D
#define CONTENT_SEEDS_TEA 0x95E
#define CONTENT_TEA 0x95F
#define CONTENT_BEANS_COFFEE 0x960
#define CONTENT_COFFEE 0x961
// FREE 962-9FA
#define CONTENT_DEAD_VINE 0x9FB
#define CONTENT_TRELLIS_DEAD_VINE 0x9FC
#define CONTENT_FARM_PUMPKIN_JACK 0x9FD
// books
#define CONTENT_CRAFT_BOOK 0x9FE
#define CONTENT_CRAFT_BOOK_OPEN 0x9FF

// slabs
#define CONTENT_ROUGHSTONE_SLAB 0xA00
#define CONTENT_MOSSYCOBBLE_SLAB 0xA01
#define CONTENT_STONE_SLAB 0xA02
#define CONTENT_WOOD_SLAB 0xA03
#define CONTENT_JUNGLE_SLAB 0xA04
#define CONTENT_BRICK_SLAB 0xA05
#define CONTENT_SANDSTONE_SLAB 0xA06
#define CONTENT_COBBLE_SLAB 0xA07
#define CONTENT_GLASS_SLAB 0xA08
#define CONTENT_GLASS_BLUE_SLAB 0xA09
#define CONTENT_GLASS_GREEN_SLAB 0xA0A
#define CONTENT_GLASS_ORANGE_SLAB 0xA0B
#define CONTENT_GLASS_PURPLE_SLAB 0xA0C
#define CONTENT_GLASS_RED_SLAB 0xA0D
#define CONTENT_GLASS_YELLOW_SLAB 0xA0E
#define CONTENT_GLASS_BLACK_SLAB 0xA0F
#define CONTENT_LIMESTONE_SLAB 0xA10
#define CONTENT_APPLEWOOD_SLAB 0xA11
// stairs
#define CONTENT_ROUGHSTONE_STAIR 0xA20
#define CONTENT_MOSSYCOBBLE_STAIR 0xA21
#define CONTENT_STONE_STAIR 0xA22
#define CONTENT_WOOD_STAIR 0xA23
#define CONTENT_JUNGLE_STAIR 0xA24
#define CONTENT_BRICK_STAIR 0xA25
#define CONTENT_SANDSTONE_STAIR 0xA26
#define CONTENT_COBBLE_STAIR 0xA27
#define CONTENT_LIMESTONE_STAIR 0xA28
#define CONTENT_APPLEWOOD_STAIR 0xA29
// upside down slabs
#define CONTENT_ROUGHSTONE_SLAB_UD 0xA40
#define CONTENT_MOSSYCOBBLE_SLAB_UD 0xA41
#define CONTENT_STONE_SLAB_UD 0xA42
#define CONTENT_WOOD_SLAB_UD 0xA43
#define CONTENT_JUNGLE_SLAB_UD 0xA44
#define CONTENT_BRICK_SLAB_UD 0xA45
#define CONTENT_SANDSTONE_SLAB_UD 0xA46
#define CONTENT_COBBLE_SLAB_UD 0xA47
#define CONTENT_GLASS_SLAB_UD 0xA48
#define CONTENT_GLASS_BLUE_SLAB_UD 0xA49
#define CONTENT_GLASS_GREEN_SLAB_UD 0xA4A
#define CONTENT_GLASS_ORANGE_SLAB_UD 0xA4B
#define CONTENT_GLASS_PURPLE_SLAB_UD 0xA4C
#define CONTENT_GLASS_RED_SLAB_UD 0xA4D
#define CONTENT_GLASS_YELLOW_SLAB_UD 0xA4E
#define CONTENT_GLASS_BLACK_SLAB_UD 0xA4F
#define CONTENT_LIMESTONE_SLAB_UD 0xA50
#define CONTENT_APPLEWOOD_SLAB_UD 0xA51
// upside down stairs
#define CONTENT_ROUGHSTONE_STAIR_UD 0xA60
#define CONTENT_MOSSYCOBBLE_STAIR_UD 0xA61
#define CONTENT_STONE_STAIR_UD 0xA62
#define CONTENT_WOOD_STAIR_UD 0xA63
#define CONTENT_JUNGLE_STAIR_UD 0xA64
#define CONTENT_BRICK_STAIR_UD 0xA65
#define CONTENT_SANDSTONE_STAIR_UD 0xA66
#define CONTENT_COBBLE_STAIR_UD 0xA67
#define CONTENT_LIMESTONE_STAIR_UD 0xA68
#define CONTENT_APPLEWOOD_STAIR_UD 0xA69
// slab/stair masks
#define CONTENT_SLAB_STAIR_MIN 0xA00
#define CONTENT_SLAB_STAIR_MAX 0xA6F
#define CONTENT_SLAB_STAIR_FLIP 0x040
#define CONTENT_SLAB_STAIR_UD_MIN 0xA40
#define CONTENT_SLAB_STAIR_UD_MAX 0xA6F
// roof tiles
#define CONTENT_ROOFTILE_TERRACOTTA 0xA70
#define CONTENT_ROOFTILE_WOOD 0xA71
#define CONTENT_ROOFTILE_ASPHALT 0xA72
#define CONTENT_ROOFTILE_STONE 0xA73
#define CONTENT_ROOFTILE_GLASS 0xA74
#define CONTENT_ROOFTILE_GLASS_BLUE 0xA75
#define CONTENT_ROOFTILE_GLASS_GREEN 0xA76
#define CONTENT_ROOFTILE_GLASS_ORANGE 0xA77
#define CONTENT_ROOFTILE_GLASS_PURPLE 0xA78
#define CONTENT_ROOFTILE_GLASS_RED 0xA79
#define CONTENT_ROOFTILE_GLASS_YELLOW 0xA7A
#define CONTENT_ROOFTILE_GLASS_BLACK 0xA7B
#define CONTENT_ROOFTILE_THATCH 0xA7C
// FREE A7D-AFF
// doors
#define CONTENT_WOOD_DOOR_LB 0xB00
#define CONTENT_WOOD_DOOR_LT 0xB01
#define CONTENT_IRON_DOOR_LB 0xB02
#define CONTENT_IRON_DOOR_LT 0xB03
#define CONTENT_GLASS_DOOR_LB 0xB04
#define CONTENT_GLASS_DOOR_LT 0xB05
// windowed doors
#define CONTENT_WOOD_W_DOOR_LB 0xB10
#define CONTENT_WOOD_W_DOOR_LT 0xB11
#define CONTENT_IRON_W_DOOR_LB 0xB12
#define CONTENT_IRON_W_DOOR_LT 0xB13
// right doors
#define CONTENT_WOOD_DOOR_RB 0xB20
#define CONTENT_WOOD_DOOR_RT 0xB21
#define CONTENT_IRON_DOOR_RB 0xB22
#define CONTENT_IRON_DOOR_RT 0xB23
#define CONTENT_GLASS_DOOR_RB 0xB24
#define CONTENT_GLASS_DOOR_RT 0xB25
// right windowed doors
#define CONTENT_WOOD_W_DOOR_RB 0xB30
#define CONTENT_WOOD_W_DOOR_RT 0xB31
#define CONTENT_IRON_W_DOOR_RB 0xB32
#define CONTENT_IRON_W_DOOR_RT 0xB33
// hatches
#define CONTENT_WOOD_HATCH 0xB40
#define CONTENT_IRON_HATCH 0xB42
#define CONTENT_WOOD_W_HATCH 0xB44
#define CONTENT_IRON_W_HATCH 0xB46
// gates
#define CONTENT_WOOD_GATE 0xB60
#define CONTENT_IRON_GATE 0xB62

// open doors
#define CONTENT_WOOD_DOOR_LB_OPEN 0xB80
#define CONTENT_WOOD_DOOR_LT_OPEN 0xB81
#define CONTENT_IRON_DOOR_LB_OPEN 0xB82
#define CONTENT_IRON_DOOR_LT_OPEN 0xB83
#define CONTENT_GLASS_DOOR_LB_OPEN 0xB84
#define CONTENT_GLASS_DOOR_LT_OPEN 0xB85
// open windowed doors
#define CONTENT_WOOD_W_DOOR_LB_OPEN 0xB90
#define CONTENT_WOOD_W_DOOR_LT_OPEN 0xB91
#define CONTENT_IRON_W_DOOR_LB_OPEN 0xB92
#define CONTENT_IRON_W_DOOR_LT_OPEN 0xB93
// open right doors
#define CONTENT_WOOD_DOOR_RB_OPEN 0xBA0
#define CONTENT_WOOD_DOOR_RT_OPEN 0xBA1
#define CONTENT_IRON_DOOR_RB_OPEN 0xBA2
#define CONTENT_IRON_DOOR_RT_OPEN 0xBA3
#define CONTENT_GLASS_DOOR_RB_OPEN 0xBA4
#define CONTENT_GLASS_DOOR_RT_OPEN 0xBA5
// open right windowed doors
#define CONTENT_WOOD_W_DOOR_RB_OPEN 0xBB0
#define CONTENT_WOOD_W_DOOR_RT_OPEN 0xBB1
#define CONTENT_IRON_W_DOOR_RB_OPEN 0xBB2
#define CONTENT_IRON_W_DOOR_RT_OPEN 0xBB3
// open hatches
#define CONTENT_WOOD_HATCH_OPEN 0xBC0
#define CONTENT_IRON_HATCH_OPEN 0xBC2
#define CONTENT_WOOD_W_HATCH_OPEN 0xBC4
#define CONTENT_IRON_W_HATCH_OPEN 0xBC6
// open gates
#define CONTENT_WOOD_GATE_OPEN 0xBE0
#define CONTENT_IRON_GATE_OPEN 0xBE2
// door/hatch/gate masks
#define CONTENT_DOOR_MIN 0xB00
#define CONTENT_DOOR_MAX 0xBFF
#define CONTENT_DOOR_IRON_MASK 0x002
#define CONTENT_DOOR_SECT_MASK 0x001
#define CONTENT_DOOR_OPEN_MASK 0x080
#define CONTENT_HATCH_MASK 0x040

#define CONTENT_SNOWMAN 0xC00
#define CONTENT_SNOW 0xC01
#define CONTENT_MUDSNOW 0xC02
#define CONTENT_SNOW_BLOCK 0xC03
#define CONTENT_ICE 0xC04
#define CONTENT_CONIFER_LEAVES 0xC06
#define CONTENT_CONIFER_SAPLING 0xC07
#define CONTENT_WOOD_PINE 0xC08
#define CONTENT_CONIFER_TREE 0xC09
#define CONTENT_LIFE_SUPPORT 0xC0A
#define CONTENT_INCINERATOR_ACTIVE 0xC0B
#define CONTENT_STONE_TILE 0xC0C
#define CONTENT_WOOD_TILE 0xC0D
// FREE C0E-C0F
// beds
#define CONTENT_BED_HEAD 0xC10
#define CONTENT_BED_FOOT 0xC11
#define CONTENT_BED_BLUE_HEAD 0xC12
#define CONTENT_BED_BLUE_FOOT 0xC13
#define CONTENT_BED_GREEN_HEAD 0xC14
#define CONTENT_BED_GREEN_FOOT 0xC15
#define CONTENT_BED_ORANGE_HEAD 0xC16
#define CONTENT_BED_ORANGE_FOOT 0xC17
#define CONTENT_BED_PURPLE_HEAD 0xC18
#define CONTENT_BED_PURPLE_FOOT 0xC19
#define CONTENT_BED_RED_HEAD 0xC1A
#define CONTENT_BED_RED_FOOT 0xC1B
#define CONTENT_BED_YELLOW_HEAD 0xC1C
#define CONTENT_BED_YELLOW_FOOT 0xC1D
#define CONTENT_BED_BLACK_HEAD 0xC1E
#define CONTENT_BED_BLACK_FOOT 0xC1F
#define CONTENT_BED_CAMP_HEAD 0xC20
#define CONTENT_BED_CAMP_FOOT 0xC21
#define CONTENT_BED_MIN 0xC10
#define CONTENT_BED_MAX 0xC21
#define CONTENT_BED_FOOT_MASK 0x001

// FREE C22-CFF

// D00-DFF - play space for other devs and experiments
#define CONTENT_STONE_KNOB 0xD00
#define CONTENT_ROUGHSTONE_KNOB 0xD01
#define CONTENT_COBBLE_KNOB 0xD02
#define CONTENT_SANDSTONE_KNOB 0xD03
#define CONTENT_WOOD_KNOB 0xD04
#define CONTENT_PINE_KNOB 0xD05
#define CONTENT_JUNGLEWOOD_KNOB 0xD06
#define CONTENT_SCAFFOLDING 0xD07

// stairs' corners
#define CONTENT_ROUGHSTONE_STAIR_CORNER 0xE00
#define CONTENT_MOSSYCOBBLE_STAIR_CORNER 0xE01
#define CONTENT_STONE_STAIR_CORNER 0xE02
#define CONTENT_WOOD_STAIR_CORNER 0xE03
#define CONTENT_JUNGLE_STAIR_CORNER 0xE04
#define CONTENT_BRICK_STAIR_CORNER 0xE05
#define CONTENT_SANDSTONE_STAIR_CORNER 0xE06
#define CONTENT_COBBLE_STAIR_CORNER 0xE07
#define CONTENT_LIMESTONE_STAIR_CORNER 0xE08

// inner stairs' corners
#define CONTENT_ROUGHSTONE_INNER_STAIR_CORNER 0xE09
#define CONTENT_MOSSYCOBBLE_INNER_STAIR_CORNER 0xE0A
#define CONTENT_STONE_INNER_STAIR_CORNER 0xE0B
#define CONTENT_WOOD_INNER_STAIR_CORNER 0xE0C
#define CONTENT_JUNGLE_INNER_STAIR_CORNER 0xE0D
#define CONTENT_BRICK_INNER_STAIR_CORNER 0xE0E
#define CONTENT_SANDSTONE_INNER_STAIR_CORNER 0xE0F
#define CONTENT_COBBLE_INNER_STAIR_CORNER 0xE10
#define CONTENT_LIMESTONE_INNER_STAIR_CORNER 0xE11

// upside down stairs' corners
#define CONTENT_ROUGHSTONE_STAIR_CORNER_UD 0xE12
#define CONTENT_MOSSYCOBBLE_STAIR_CORNER_UD 0xE13
#define CONTENT_STONE_STAIR_CORNER_UD 0xE14
#define CONTENT_WOOD_STAIR_CORNER_UD 0xE15
#define CONTENT_JUNGLE_STAIR_CORNER_UD 0xE16
#define CONTENT_BRICK_STAIR_CORNER_UD 0xE17
#define CONTENT_SANDSTONE_STAIR_CORNER_UD 0xE18
#define CONTENT_COBBLE_STAIR_CORNER_UD 0xE19
#define CONTENT_LIMESTONE_STAIR_CORNER_UD 0xE1A

// upside down inner stairs' corners
#define CONTENT_ROUGHSTONE_INNER_STAIR_CORNER_UD 0xE1B
#define CONTENT_MOSSYCOBBLE_INNER_STAIR_CORNER_UD 0xE1C
#define CONTENT_STONE_INNER_STAIR_CORNER_UD 0xE1D
#define CONTENT_WOOD_INNER_STAIR_CORNER_UD 0xE1E
#define CONTENT_JUNGLE_INNER_STAIR_CORNER_UD 0xE1F
#define CONTENT_BRICK_INNER_STAIR_CORNER_UD 0xE20
#define CONTENT_SANDSTONE_INNER_STAIR_CORNER_UD 0xE21
#define CONTENT_COBBLE_INNER_STAIR_CORNER_UD 0xE22
#define CONTENT_LIMESTONE_INNER_STAIR_CORNER_UD 0xE23

// applewood stairs' corners, inner and upside down
#define CONTENT_APPLEWOOD_STAIR_CORNER 0xE24
#define CONTENT_APPLEWOOD_INNER_STAIR_CORNER 0xE25
#define CONTENT_APPLEWOOD_STAIR_CORNER_UD 0xE26
#define CONTENT_APPLEWOOD_INNER_STAIR_CORNER_UD 0xE27
// FREE E28-EFF

// circuits
// circuits - wire
#define CONTENT_CIRCUIT_MITHRILWIRE 0xF00
#define CONTENT_CIRCUIT_COPPERWIRE 0xF01
// circuits - powersource
#define CONTENT_CIRCUIT_REACTOR 0xF20
#define CONTENT_CIRCUIT_SOLARPANEL 0xF21
#define CONTENT_CIRCUIT_WATERWHEEL 0xF22
#define CONTENT_CIRCUIT_SWITCH 0xF23
#define CONTENT_CIRCUIT_NOTGATE 0xF24
#define CONTENT_CIRCUIT_BUTTON 0xF25
#define CONTENT_CIRCUIT_PRESSUREPLATE_WOOD 0xF26
#define CONTENT_CIRCUIT_PRESSUREPLATE_STONE 0xF27
#define CONTENT_CIRCUIT_REPEATER 0xF28
// circuits - gadgets
#define CONTENT_CIRCUIT_LAMP 0xF40
#define CONTENT_CIRCUIT_LAMP_OFF 0xF41
#define CONTENT_CIRCUIT_PISTON 0xF42
#define CONTENT_CIRCUIT_PISTON_OFF 0xF43
#define CONTENT_CIRCUIT_PISTON_ARM 0xF44
#define CONTENT_CIRCUIT_PISTON_UP 0xF45
#define CONTENT_CIRCUIT_PISTON_UP_OFF 0xF46
#define CONTENT_CIRCUIT_PISTON_UP_ARM 0xF47
#define CONTENT_CIRCUIT_PISTON_DOWN 0xF48
#define CONTENT_CIRCUIT_PISTON_DOWN_OFF 0xF49
#define CONTENT_CIRCUIT_PISTON_DOWN_ARM 0xF4A
#define CONTENT_CIRCUIT_STICKYPISTON 0xF4B
#define CONTENT_CIRCUIT_STICKYPISTON_OFF 0xF4C
#define CONTENT_CIRCUIT_STICKYPISTON_ARM 0xF4D
#define CONTENT_CIRCUIT_STICKYPISTON_UP 0xF4E
#define CONTENT_CIRCUIT_STICKYPISTON_UP_OFF 0xF4F
#define CONTENT_CIRCUIT_STICKYPISTON_UP_ARM 0xF50
#define CONTENT_CIRCUIT_STICKYPISTON_DOWN 0xF52
#define CONTENT_CIRCUIT_STICKYPISTON_DOWN_OFF 0xF53
#define CONTENT_CIRCUIT_STICKYPISTON_DOWN_ARM 0xF54
// circuits - MASKs
#define CONTENT_CIRCUIT_MIN 0xF00
#define CONTENT_CIRCUIT_MAX 0xFFF

#endif