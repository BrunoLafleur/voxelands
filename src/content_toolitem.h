#ifndef CONTENT_TOOLITEM_HEADER
#define CONTENT_TOOLITEM_HEADER

#include "mapnode.h"

#define CONTENT_TOOLITEM_MASK 0x4000

enum ToolType {
	TT_NONE = 0, // used only when checking against an item that isn't a tool
	TT_SPECIAL, // special toolitems cannot dig
	TT_AXE,
	TT_PICK,
	TT_SHOVEL,
	TT_SHEAR,
	TT_BUCKET,
	TT_SWORD,
	TT_SPEAR,
	TT_KNIFE,
	TT_HAMMER,
	TT_CLUB
};

typedef struct diginfo_s {
	uint16_t uses;
	float time;
	uint8_t level;
} diginfo_t;

struct ToolItemFeatures {
	content_t content;
	std::string texture;
	// tooltip used in inventory
	char* description;
	// the result of cooking this item
	std::string cook_result;
	// what type of cooking device this item needs
	uint16_t cook_type;
	// the fuel value of this item
	std::string crush_result;
	// what type of crushing device this item needs
        CrushType crush_type;
        // the crush value of this item
	float fuel_time;
	// 0 if inedible, otherwise the value it improves hp by
	s16 edible;
	// the number dropped on right click, -1 for all
	s16 drop_count;
	// whether this tool can point at liquid nodes
	bool liquids_pointable;
	// whether this tool should die when trying to pick up damaging nodes
	bool damaging_nodes_diggable;
	// whether this tool has a punch effect, such as open doors
	bool has_punch_effect;
	// whether this tool can lock/unlock nodes
	bool has_unlock_effect;
	bool has_super_unlock_effect;
	// whether this tool can rotate nodes
	bool has_rotate_effect;
	// whether this tool can start fires
	bool has_fire_effect;
	// digging info for this tool
	diginfo_t diginfo;
	// the type of this tool
	ToolType type;
	// the data value of this item
	ContentParamType param_type;
	// used for eg. bows throwing an arrow
	content_t thrown_item;
	// when this item is placed, it should be replaced by this in inventory
	content_t onplace_replace_item;
	// when this item is placed, it should place this node instead
	content_t onplace_node;

	ToolItemFeatures():
		content(CONTENT_IGNORE),
		texture("unknown_item.png"),
		cook_result(""),
		cook_type(COOK_ANY),
		crush_result(""),
		crush_type(CRUSH_ANY),
		fuel_time(0.0),
		edible(0),
		drop_count(-1),
		liquids_pointable(false),
		damaging_nodes_diggable(true),
		has_punch_effect(true),
		has_unlock_effect(false),
		has_super_unlock_effect(false),
		has_rotate_effect(false),
		has_fire_effect(false),
		type(TT_NONE),
		param_type(CPT_NONE),
		thrown_item(CONTENT_IGNORE),
		onplace_replace_item(CONTENT_IGNORE),
		onplace_node(CONTENT_IGNORE)
	{
		description = (char*)"";
		diginfo.uses = 256;
		diginfo.time = 4.0;
		diginfo.level = 0;
	}
};

typedef struct tooluse_s {
	bool diggable;
	// Digging time in seconds or hit damage
	float data;
	// Caused wear
	uint16_t wear;
	// delay till tool can be used again
	float delay;
} tooluse_t;

// For getting the default properties, set toolid=CONTENT_IGNORE
int get_tool_use(tooluse_t *info, content_t target, uint16_t data, content_t toolid, uint16_t tooldata);
std::string toolitem_overlay(content_t content, std::string ol);
void content_toolitem_init();
ToolItemFeatures & content_toolitem_features(content_t i);
ToolItemFeatures & content_toolitem_features(std::string subname);

#define CONTENT_TOOLITEM_SMALL_PICK (CONTENT_TOOLITEM_MASK | 0x01)
#define CONTENT_TOOLITEM_COPPER_PICK (CONTENT_TOOLITEM_MASK | 0x02)
#define CONTENT_TOOLITEM_IRON_PICK (CONTENT_TOOLITEM_MASK | 0x03)
#define CONTENT_TOOLITEM_CREATIVEPICK (CONTENT_TOOLITEM_MASK | 0x04)
#define CONTENT_TOOLITEM_TROWEL (CONTENT_TOOLITEM_MASK | 0x05)
#define CONTENT_TOOLITEM_COPPER_SHOVEL (CONTENT_TOOLITEM_MASK | 0x06)
#define CONTENT_TOOLITEM_IRON_SHOVEL (CONTENT_TOOLITEM_MASK | 0x07)
#define CONTENT_TOOLITEM_SMALL_AXE (CONTENT_TOOLITEM_MASK | 0x08)
#define CONTENT_TOOLITEM_COPPER_AXE (CONTENT_TOOLITEM_MASK | 0x09)
#define CONTENT_TOOLITEM_IRON_AXE (CONTENT_TOOLITEM_MASK | 0x0A)
#define CONTENT_TOOLITEM_CLUB (CONTENT_TOOLITEM_MASK | 0x0B)
#define CONTENT_TOOLITEM_COPPER_SWORD (CONTENT_TOOLITEM_MASK | 0x0C)
#define CONTENT_TOOLITEM_IRON_SWORD (CONTENT_TOOLITEM_MASK | 0x0D)
#define CONTENT_TOOLITEM_IRON_SHEARS (CONTENT_TOOLITEM_MASK | 0x0E)
#define CONTENT_TOOLITEM_WBUCKET (CONTENT_TOOLITEM_MASK | 0x0F)
#define CONTENT_TOOLITEM_TINBUCKET (CONTENT_TOOLITEM_MASK | 0x10)
#define CONTENT_TOOLITEM_WBUCKET_WATER (CONTENT_TOOLITEM_MASK | 0x11)
#define CONTENT_TOOLITEM_TINBUCKET_WATER (CONTENT_TOOLITEM_MASK | 0x12)
#define CONTENT_TOOLITEM_IRON_BUCKET (CONTENT_TOOLITEM_MASK | 0x13)
#define CONTENT_TOOLITEM_IRON_BUCKET_WATER (CONTENT_TOOLITEM_MASK | 0x14)
#define CONTENT_TOOLITEM_IRON_BUCKET_LAVA (CONTENT_TOOLITEM_MASK | 0x15)
#define CONTENT_TOOLITEM_FIRESTARTER (CONTENT_TOOLITEM_MASK | 0x16)
#define CONTENT_TOOLITEM_CROWBAR (CONTENT_TOOLITEM_MASK | 0x17)
#define CONTENT_TOOLITEM_FLINTPICK (CONTENT_TOOLITEM_MASK | 0x18)
#define CONTENT_TOOLITEM_FLINTSHOVEL (CONTENT_TOOLITEM_MASK | 0x19)
#define CONTENT_TOOLITEM_FLINTAXE (CONTENT_TOOLITEM_MASK | 0x1A)
#define CONTENT_TOOLITEM_FLINTSHEARS (CONTENT_TOOLITEM_MASK | 0x1B)
#define CONTENT_TOOLITEM_KEY (CONTENT_TOOLITEM_MASK | 0x1C)
#define CONTENT_TOOLITEM_STONE_SPEAR (CONTENT_TOOLITEM_MASK | 0x1D)
#define CONTENT_TOOLITEM_FLINTSPEAR (CONTENT_TOOLITEM_MASK | 0x1E)
#define CONTENT_TOOLITEM_IRON_SPEAR (CONTENT_TOOLITEM_MASK | 0x1F)
#define CONTENT_TOOLITEM_BOW (CONTENT_TOOLITEM_MASK | 0x20)
#define CONTENT_TOOLITEM_BRONZE_PICK (CONTENT_TOOLITEM_MASK | 0x21)
#define CONTENT_TOOLITEM_BRONZE_SHOVEL (CONTENT_TOOLITEM_MASK | 0x22)
#define CONTENT_TOOLITEM_BRONZE_AXE (CONTENT_TOOLITEM_MASK | 0x23)
#define CONTENT_TOOLITEM_BRONZE_SWORD (CONTENT_TOOLITEM_MASK | 0x24)
#define CONTENT_TOOLITEM_BRONZE_SPEAR (CONTENT_TOOLITEM_MASK | 0x25)
#define CONTENT_TOOLITEM_MITHRIL_UNBOUND_PICK (CONTENT_TOOLITEM_MASK | 0x26)
#define CONTENT_TOOLITEM_MITHRIL_UNBOUND_SHOVEL (CONTENT_TOOLITEM_MASK | 0x27)
#define CONTENT_TOOLITEM_MITHRIL_UNBOUND_AXE (CONTENT_TOOLITEM_MASK | 0x28)
#define CONTENT_TOOLITEM_MITHRIL_UNBOUND_SWORD (CONTENT_TOOLITEM_MASK | 0x29)
#define CONTENT_TOOLITEM_MITHRIL_UNBOUND_SPEAR (CONTENT_TOOLITEM_MASK | 0x2A)
#define CONTENT_TOOLITEM_MITHRIL_PICK (CONTENT_TOOLITEM_MASK | 0x2B)
#define CONTENT_TOOLITEM_MITHRIL_SHOVEL (CONTENT_TOOLITEM_MASK | 0x2C)
#define CONTENT_TOOLITEM_MITHRIL_AXE (CONTENT_TOOLITEM_MASK | 0x2D)
#define CONTENT_TOOLITEM_MITHRIL_SWORD (CONTENT_TOOLITEM_MASK | 0x2E)
#define CONTENT_TOOLITEM_MITHRIL_SPEAR (CONTENT_TOOLITEM_MASK | 0x2F)
#define CONTENT_TOOLITEM_MOB_SPAWNER (CONTENT_TOOLITEM_MASK | 0x30)
#define CONTENT_TOOLITEM_MITHRIL_KEY (CONTENT_TOOLITEM_MASK | 0x31)
#define CONTENT_TOOLITEM_STONE_KNIFE (CONTENT_TOOLITEM_MASK | 0x32)
#define CONTENT_TOOLITEM_COPPER_KNIFE (CONTENT_TOOLITEM_MASK | 0x33)
#define CONTENT_TOOLITEM_FLINT_KNIFE (CONTENT_TOOLITEM_MASK | 0x34)
#define CONTENT_TOOLITEM_BRONZE_KNIFE (CONTENT_TOOLITEM_MASK | 0x35)
#define CONTENT_TOOLITEM_IRON_KNIFE (CONTENT_TOOLITEM_MASK | 0x36)
#define CONTENT_TOOLITEM_MITHRIL_UNBOUND_KNIFE (CONTENT_TOOLITEM_MASK | 0x37)
#define CONTENT_TOOLITEM_MITHRIL_KNIFE (CONTENT_TOOLITEM_MASK | 0x38)
#define CONTENT_TOOLITEM_COPPER_SPEAR (CONTENT_TOOLITEM_MASK | 0x39)
#define CONTENT_TOOLITEM_COPPER_SHEARS (CONTENT_TOOLITEM_MASK | 0x3A)

#endif
