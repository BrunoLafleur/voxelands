/************************************************************************
* Minetest-c55
* Copyright (C) 2010-2011 celeron55, Perttu Ahola <celeron55@gmail.com>
*
* content_nodemeta.cpp
* voxelands - 3d voxel world sandbox game
* Copyright (C) Lisa 'darkrose' Milne 2013-2017 <lisa@ltmnet.com>
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
#include "list.h"
#include "content_nodemeta.h"
#include "inventory.h"
#include "content_mapnode.h"
#include "content_craft.h"
#include "content_list.h"
#include <algorithm>

/*
	CraftGuideNodeMetadata
*/

// Prototype
CraftGuideNodeMetadata proto_CraftGuideNodeMetadata;

CraftGuideNodeMetadata::CraftGuideNodeMetadata()
{
	NodeMetadata::registerType(typeId(), create);

	m_page = 0;
	m_recipe = 0;

	m_inventory = new Inventory();
	m_inventory->addList("list", 300);
	m_inventory->addList("recipe", 9);
	m_inventory->addList("result", 1);
}
CraftGuideNodeMetadata::~CraftGuideNodeMetadata()
{
	delete m_inventory;
}
u16 CraftGuideNodeMetadata::typeId() const
{
	return CONTENT_CRAFT_BOOK_OPEN;
}
NodeMetadata* CraftGuideNodeMetadata::clone()
{
	contentlist_t *cl;
	listdata_t *ld;
	InventoryList *l;
	InventoryItem *t;
	content_t *r;
	uint32_t list_size = 0;
	uint32_t start;
	uint32_t end;
	uint32_t i;
	CraftGuideNodeMetadata *d = new CraftGuideNodeMetadata();

	*d->m_inventory = *m_inventory;
	d->m_page = m_page;

	l = d->m_inventory->getList("list");
	l->clearItems();

	cl = content_list_get("craftguide");

	if (!cl)
		return d;

	list_size = list_count(&cl->data);

	start = m_page*40;
	end = start+40;

	if (end > list_size)
		end = list_size;

	ld = cl->data;
	for (i=0; ld && i<end; i++) {
		if (i >= start) {
			t = InventoryItem::create(ld->content,ld->count,0,ld->data);
			r = crafting::getRecipe(t);
			if (!r) {
				delete t;
				ld = ld->next;
				continue;
			}
			l->addItem(t);
		}
		ld = ld->next;
	}

	return d;
}
NodeMetadata* CraftGuideNodeMetadata::create(std::istream &is)
{
	CraftGuideNodeMetadata *d = new CraftGuideNodeMetadata();

	d->m_inventory->deSerialize(is);
	is>>d->m_page;
	is>>d->m_recipe;

	return d;
}
void CraftGuideNodeMetadata::serializeBody(std::ostream &os)
{
	m_inventory->serialize(os);
	os<<itos(m_page) << " ";
	os<<itos(m_recipe) << " ";
}
bool CraftGuideNodeMetadata::nodeRemovalDisabled()
{
	return false;
}
void CraftGuideNodeMetadata::inventoryModified()
{
	vlprintf(CN_INFO,"CraftGuide inventory modification callback");
}
bool CraftGuideNodeMetadata::step(float dtime, v3s16 pos, ServerEnvironment *env)
{
	InventoryList *l = m_inventory->getList("result");
	InventoryItem *t = l->getItem(0);
	if (!t || t->getContent() == CONTENT_IGNORE)
		return false;
	content_t *r = crafting::getRecipe(t,m_recipe);
	if (!r) {
		if (m_recipe == 0)
			return false;

		m_recipe = 0;
		r = crafting::getRecipe(t,m_recipe);
		if (!r)
			return false;
	}
	l = m_inventory->getList("recipe");
	l->clearItems();
	for (int i=0; i<9; i++) {
		if (r[i] == CONTENT_IGNORE)
			continue;
		t = InventoryItem::create(r[i],1);
		l->addItem(i,t);
	}

	delete[] r;

	return true;
}
bool CraftGuideNodeMetadata::import(NodeMetadata *meta)
{
	contentlist_t *cl;
	listdata_t *ld;
	InventoryList *l;
	InventoryItem *t;
	content_t *r;
	uint32_t list_size = 0;
	uint32_t start;
	uint32_t end;
	uint32_t i;

	if (meta->typeId() == CONTENT_BOOK)
		m_page = ((ClosedBookNodeMetadata*)meta)->getPage();

	if (m_page < 0)
		m_page = 0;

	cl = content_list_get("craftguide");

	if (!cl)
		return false;

	list_size = list_count(&cl->data);

	if (m_page > (list_size/40))
		m_page = list_size/40;

	start = m_page*40;
	end = start+40;

	l = m_inventory->getList("list");
	l->clearItems();

	if (end > list_size)
		end = list_size;

	ld = cl->data;
	for (i=0; ld && i<end; i++) {
		if (i >= start) {
			t = InventoryItem::create(ld->content,ld->count,0,ld->data);
			r = crafting::getRecipe(t);
			if (!r) {
				delete t;
				ld = ld->next;
				continue;
			}
			l->addItem(t);
		}
		ld = ld->next;
	}

	return true;
}
bool CraftGuideNodeMetadata::receiveFields(std::string formname, std::map<std::string, std::string> fields, Player *player)
{
	InventoryList *l;
	InventoryItem *t;
	contentlist_t *cl;
	listdata_t *ld;
	content_t *r;
	uint32_t list_size;
	uint32_t start;
	uint32_t end;
	uint32_t i;

	l = m_inventory->getList("list");
	if (fields["rprev"] != "" || fields["rnext"] != "") {
		l = m_inventory->getList("result");
		t = l->getItem(0);
		if (fields["rprev"] != "") {
			if (m_recipe > 0)
				m_recipe--;
		}else{
			m_recipe++;
		}
		int m = 1;
		if (t && t->getContent() != CONTENT_IGNORE)
			m = crafting::getRecipeCount(t);
		if (m_recipe >= m)
			m_recipe = m-1;
		step(0,v3s16(0,0,0),NULL);
		return true;
	}
	if (fields["prev"] == "" && fields["next"] == "")
		return false;

	cl = content_list_get("craftguide");

	if (!cl)
		return false;

	list_size = list_count(&cl->data);

	if (fields["prev"] != "") {
		if (m_page > 0) {
			m_page--;
		}else{
			m_page = list_size/40;
		}
	}
	if (fields["next"] != "")
		m_page++;
	if (m_page > (list_size/40))
		m_page = 0;

	l->clearItems();

	start = m_page*40;
	end = start+40;
	if (end > list_size)
		end = list_size;

	ld = cl->data;
	for (i=0; ld && i<end; i++) {
		if (i >= start) {
			t = InventoryItem::create(ld->content,ld->count,0,ld->data);
			r = crafting::getRecipe(t);
			if (!r) {
				delete t;
				ld = ld->next;
				continue;
			}
			l->addItem(t);
		}
		ld = ld->next;
	}

	return true;
}
std::string CraftGuideNodeMetadata::getDrawSpecString(Player *player)
{
	InventoryList *l = m_inventory->getList("result");
	InventoryItem *q = l->getItem(0);
	int tr = 0;
	int rc = 0;
	char buff[256];
	uint32_t list_size = 0;
	contentlist_t *cl;

	cl = content_list_get("craftguide");
	if (cl)
		list_size = list_count(&cl->data);

	if (q && q->getContent() != CONTENT_IGNORE) {
		tr = crafting::getResultCount(q);
		rc = crafting::getRecipeCount(q);
	}

	snprintf(buff,256,gettext("Page %d of %d"),(int)(m_page+1),(int)((list_size/40)+1));

	std::string spec("size[8,10]");
	spec +=	"label[0.5,0.75;";
	spec += gettext("Add item here to see recipe");;
	spec += "]";
	spec +=	"list[current_name;result;2,1;1,1;]";
	if (rc > 1) {
		char rbuff[256];
		snprintf(rbuff, 256, gettext("Recipe %d of %d"), (int)(m_recipe+1),rc);
		spec += "button[2.5,3.5;1,0.75;rprev;<<]";
		spec += "label[3.5,3.5;";
		spec += rbuff;
		spec += "]";
		spec += "button[5.5,3.5;1,0.75;rnext;>>]";
	}
	if (q && tr) {
		spec += "label[1,1.5;";
		spec += gettext("Gives");;
		spec += " ";
		spec += itos(tr);
		// this overflows into the craft grid... but could be cool
		//spec += " ";
		//spec += wide_to_narrow(q->getGuiName());
		spec += "]";
	}
	spec +=	"list[current_name;recipe;4,0;3,3;]";
	spec +=	"button[0.25,4.5;2.5,0.75;prev;";
	spec += gettext("<< Previous Page");;
	spec += "]";
	spec +=	"label[3.5,4.5;";
	spec += buff;
	spec += "]";
	spec +=	"button[6,4.5;2.5,0.75;next;";
	spec += gettext("Next Page >>");;
	spec += "]";
	spec +=	"list[current_name;list;0,5;8,5;]";
	return spec;
}

/*
	ReverseCraftGuideNodeMetadata
*/

// Prototype
ReverseCraftGuideNodeMetadata proto_ReverseCraftGuideNodeMetadata;

ReverseCraftGuideNodeMetadata::ReverseCraftGuideNodeMetadata()
{
	//make sure that the type gets registered for this metadata
	NodeMetadata::registerType(typeId(), create);

	//start on the first page, with the first recipe
	m_page = 0;
	m_recipe = 0;

	//build the inventory
	m_inventory = new Inventory;
	m_inventory->addList("list", 300);
	m_inventory->addList("item", 1);
	m_inventory->addList("recipe", 9);
	m_inventory->addList("result", 1);
}
ReverseCraftGuideNodeMetadata::~ReverseCraftGuideNodeMetadata()
{
	delete m_inventory;
}
u16 ReverseCraftGuideNodeMetadata::typeId() const
{
	return CONTENT_RCRAFT_BOOK_OPEN;
}
void ReverseCraftGuideNodeMetadata::reloadPage()
{
	using namespace std;

	//get the inventory list and clear it
	InventoryList *inv_list = m_inventory->getList("list");
	inv_list->clearItems();

	//retrieve the list of things in the ingredient list
	vector<content_t> &ingredient_list = crafting::getCraftGuideIngredientList();

	//get the number of pages
	if (ingredient_list.size() == 0) return;
	u16 page_count = ingredient_list.size()/40;
	if (ingredient_list.size()%40) ++page_count;

	//make sure the page is actually in range (via modulus)
	if (s16(m_page) >= page_count) m_page %= page_count;
	else if (s16(m_page) < 0) m_page = s16(m_page)%page_count + page_count;

	//go through each item on the current page
	vector<content_t>::iterator page_begin = ingredient_list.begin() + m_page*40;
	vector<content_t>::iterator page_end = ingredient_list.begin() + min((m_page+1)*40, signed(ingredient_list.size()));
	for (vector<content_t>::iterator it = page_begin; it != page_end; ++it) {

		//create an inventory item for it
		InventoryItem *cur_item = InventoryItem::create(*it, 1);

		//make extra sure that it actually has recipes in order to not look stupid
		if (!crafting::getReverseRecipe(cur_item)) delete cur_item;

		//if it does, add it
		else inv_list->addItem(cur_item);
	}
}
NodeMetadata* ReverseCraftGuideNodeMetadata::clone()
{
	//create a new metadata object
	ReverseCraftGuideNodeMetadata *d = new ReverseCraftGuideNodeMetadata;

	//copy over the inventory
	*d->m_inventory = *m_inventory;

	//keep the same page
	d->m_page = m_page;

	//rebuild the page on the copy
	d->reloadPage();

	//return the completed copy
	return d;
}
NodeMetadata* ReverseCraftGuideNodeMetadata::create(std::istream &is)
{
	//create a new metadata object
	ReverseCraftGuideNodeMetadata *d = new ReverseCraftGuideNodeMetadata;

	//deserialize the inventory
	d->m_inventory->deSerialize(is);

	//read in the page and recipe
	is>>d->m_page;
	is>>d->m_recipe;

	//return the completed object
	return d;
}
void ReverseCraftGuideNodeMetadata::serializeBody(std::ostream &os)
{
	//serialize the inventory
	m_inventory->serialize(os);

	//also serialize the page and recipe numbers
	os << itos(m_page) << " ";
	os << itos(m_recipe) << " ";
}
bool ReverseCraftGuideNodeMetadata::nodeRemovalDisabled()
{
	//the player can always remove this node
	return false;
}
void ReverseCraftGuideNodeMetadata::inventoryModified()
{
	vlprintf(CN_INFO,"ReverseCraftGuide inventory modification callback");
}
bool ReverseCraftGuideNodeMetadata::step(float dtime, v3s16 pos, ServerEnvironment *env)
{
	//get the item in the item box
	InventoryItem *item = m_inventory->getList("item")->getItem(0);

	//if there's no item in the item box, do nothing
	if (!item || item->getContent() == CONTENT_IGNORE)
		return false;

	//attempt to look up the recipe
	crafting::FoundReverseRecipe recipe = crafting::getReverseRecipe(item, m_recipe);

	//if it doesn't exist, attempt to start over on the first recipe
	if (!recipe) {

		//if it's already on the first recipe, give up
		if (m_recipe == 0)
			return false;

		//otherwise, switch to the first recipe
		m_recipe = 0;
		recipe = crafting::getReverseRecipe(item, m_recipe);

		//give up if that doesn't work
		if (!recipe)
			return false;
	}

	//clear the recipe box
	InventoryList *rec_list = m_inventory->getList("recipe");
	rec_list->clearItems();

	//load the recipe into the recipe box
	for (int i=0; i<9; i++) {
		if (recipe.recipe[i] == CONTENT_IGNORE)
			continue;
		InventoryItem *item = InventoryItem::create(recipe.recipe[i], 1);
		rec_list->addItem(i, item);
	}

	//load the result box too
	{
		InventoryList *res_list = m_inventory->getList("result");
		res_list->clearItems();
		InventoryItem *result = InventoryItem::create(recipe.result, recipe.result_count);
		res_list->addItem(0, result);
	}

	//the node has now been updated
	return true;
}
bool ReverseCraftGuideNodeMetadata::import(NodeMetadata *meta)
{
	using namespace std;

	//if the metadata is from a book being opened, stay on the same page
	if (ClosedBookNodeMetadata *book_meta = dynamic_cast<ClosedBookNodeMetadata*>(meta))
		m_page = book_meta->getPage();

	//reload the page
	reloadPage();

	//node updated
	return true;
}
bool ReverseCraftGuideNodeMetadata::receiveFields(std::string formname, std::map<std::string, std::string> fields, Player *player)
{
	//if the player wants to change the recipe
	if (fields["rprev"] != "" || fields["rnext"] != "") {

		//find the ingredient item
		InventoryItem *item = m_inventory->getList("item")->getItem(0);

		//advance the recipe counter appropriately
		if (fields["rprev"] != "") {
			if (m_recipe > 0)
				m_recipe--;
		} else {
			m_recipe++;
		}

		//get the recipe count
		int rec_count = 1;
		if (item && item->getContent() != CONTENT_IGNORE)
			rec_count = crafting::getReverseRecipeCount(item);

		//fix the counter if needed
		if (m_recipe >= rec_count)
			m_recipe = rec_count - 1;

		//this node now needs updating
		step(0, v3s16(0,0,0), NULL);
		return true;
	}

	//if the player wants to change the list page
	if (fields["prev"] != "" || fields["next"] != "") {

		//advance m_page correctly
		if (fields["prev"] != "") --m_page;
		if (fields["next"] != "") ++m_page;

		//reload the page (if the number is out of bounds it will fix it correctly automatically)
		reloadPage();

		//the node has been updated
		return true;
	}

	//nothing happened
	return false;
}
std::string ReverseCraftGuideNodeMetadata::getDrawSpecString(Player *player)
{
	using namespace std;

	//get the ingredient item
	InventoryItem *item = m_inventory->getList("item")->getItem(0);
	int recipe_count = 0;
	if (item && item->getContent() != CONTENT_IGNORE) {
		recipe_count = crafting::getReverseRecipeCount(item);
	}

	//get the number of pages
	vector<content_t> &ingredient_list = crafting::getCraftGuideIngredientList();
	if (ingredient_list.size() == 0) return "";
	u16 page_count = ingredient_list.size()/40;
	if (ingredient_list.size()%40) ++page_count;

	//write the page count string
	char buff[256];
	snprintf(buff, 256, gettext("Page %d of %d"), (int)(m_page+1), page_count);

	//build the formspec
	string spec("size[8,10]");
	spec +=	"label[0.5,0.75;";
	spec += gettext("Add item here to see recipe");;
	spec += "]";
	spec +=	"list[current_name;item;2,1;1,1;]";
	if (recipe_count > 1) {
		char rbuff[256];
		snprintf(rbuff, 256, gettext("Recipe %d of %d"), (int)(m_recipe+1), recipe_count);
		spec += "button[2.5,3.5;1,0.75;rprev;<<]";
		spec += "label[3.5,3.5;";
		spec += rbuff;
		spec += "]";
		spec += "button[5.5,3.5;1,0.75;rnext;>>]";
	}
	spec +=	"list[current_name;recipe;4,0;3,3;]";
	spec +=	"button[0.25,4.5;2.5,0.75;prev;";
	spec += gettext("<< Previous Page");;
	spec += "]";
	spec +=	"label[3.5,4.5;";
	spec += buff;
	spec += "]";
	spec +=	"button[6,4.5;2.5,0.75;next;";
	spec += gettext("Next Page >>");;
	spec += "]";
	spec += "list[current_name;result;7,0;1,1;]";
	spec +=	"list[current_name;list;0,5;8,5;]";
	return spec;
}

/*
	CookBookNodeMetadata
*/

// Prototype
CookBookNodeMetadata proto_CookBookNodeMetadata;

CookBookNodeMetadata::CookBookNodeMetadata()
{
	NodeMetadata::registerType(typeId(), create);

	m_page = 0;

	m_inventory = new Inventory();
	m_inventory->addList("list", 300);
	m_inventory->addList("recipe", 1);
	m_inventory->addList("result", 1);
}
CookBookNodeMetadata::~CookBookNodeMetadata()
{
	delete m_inventory;
}
u16 CookBookNodeMetadata::typeId() const
{
	return CONTENT_COOK_BOOK_OPEN;
}
NodeMetadata* CookBookNodeMetadata::clone()
{
	InventoryList *l;
	InventoryItem *t;
	InventoryItem *r;
	contentlist_t *cl;
	listdata_t *ld;
	uint32_t list_size;
	uint32_t start;
	uint32_t end;
	uint32_t i;

	CookBookNodeMetadata *d = new CookBookNodeMetadata();
	*d->m_inventory = *m_inventory;
	d->m_page = m_page;

	l = d->m_inventory->getList("list");
	l->clearItems();

	cl = content_list_get("cooking");
	if (!cl)
		return d;

	list_size = list_count(&cl->data);

	start = m_page*40;
	end = start+40;

	if (end > list_size)
		end = list_size;

	ld = cl->data;
	for (i=0; ld && i<end; i++) {
		if (i >= start) {
			t = InventoryItem::create(ld->content,ld->count,0,ld->data);
			r = t->createCookResult();
			if (!r || r->getContent() == CONTENT_IGNORE) {
				delete t;
				delete r;
				ld = ld->next;
				continue;
			}
			delete r;
			l->addItem(t);
		}
		ld = ld->next;
	}

	return d;
}
NodeMetadata* CookBookNodeMetadata::create(std::istream &is)
{
	CookBookNodeMetadata *d = new CookBookNodeMetadata();

	d->m_inventory->deSerialize(is);
	is>>d->m_page;

	return d;
}
void CookBookNodeMetadata::serializeBody(std::ostream &os)
{
	m_inventory->serialize(os);
	os<<itos(m_page) << " ";
}
bool CookBookNodeMetadata::nodeRemovalDisabled()
{
	return false;
}
void CookBookNodeMetadata::inventoryModified()
{
	vlprintf(CN_INFO,"CookBook inventory modification callback");
}
bool CookBookNodeMetadata::step(float dtime, v3s16 pos, ServerEnvironment *env)
{
	InventoryList *l = m_inventory->getList("result");
	InventoryItem *t = l->getItem(0);
	if (!t || t->getContent() == CONTENT_IGNORE)
		return false;
	InventoryItem *cookresult = t->createCookResult();
	if (!cookresult || cookresult->getContent() == CONTENT_IGNORE)
		return false;
	l = m_inventory->getList("recipe");
	l->clearItems();
	l->addItem(0,cookresult);

	return true;
}
bool CookBookNodeMetadata::import(NodeMetadata *meta)
{
	InventoryList *l;
	InventoryItem *t;
	InventoryItem *r;
	contentlist_t *cl;
	listdata_t *ld;
	uint32_t list_size;
	uint32_t start;
	uint32_t end;
	uint32_t i;

	if (meta->typeId() == CONTENT_BOOK)
		m_page = ((ClosedBookNodeMetadata*)meta)->getPage();

	if (m_page < 0)
		m_page = 0;

	cl = content_list_get("cooking");
	if (!cl)
			return true;

	list_size = list_count(&cl->data);

	if (m_page > (list_size/40))
		m_page = list_size/40;

	l = m_inventory->getList("list");
	l->clearItems();

	start = m_page*40;
	end = start+40;

	if (end > list_size)
		end = list_size;

	ld = cl->data;
	for (i=0; ld && i<end; i++) {
		if (i >= start) {
			t = InventoryItem::create(ld->content,ld->count,0,ld->data);
			r = t->createCookResult();
			if (!r || r->getContent() == CONTENT_IGNORE) {
				delete t;
				delete r;
				ld = ld->next;
				continue;
			}
			delete r;
			l->addItem(t);
		}
		ld = ld->next;
	}

	return true;
}
bool CookBookNodeMetadata::receiveFields(std::string formname, std::map<std::string, std::string> fields, Player *player)
{
	InventoryList *l;
	InventoryItem *t;
	InventoryItem *r;
	contentlist_t *cl;
	listdata_t *ld;
	uint32_t list_size;
	uint32_t start;
	uint32_t end;
	uint32_t i;

	if (fields["prev"] == "" && fields["next"] == "")
		return false;

	cl = content_list_get("cooking");
	if (!cl)
		return false;

	list_size = list_count(&cl->data);

	if (fields["prev"] != "") {
		if (m_page > 0) {
			m_page--;
		}else{
			m_page = list_size/40;
		}
	}
	if (fields["next"] != "")
		m_page++;
	if (m_page > (list_size/40))
		m_page = 0;

	l = m_inventory->getList("list");
	l->clearItems();

	start = m_page*40;
	end = start+40;
	if (end > list_size)
		end = list_size;

	ld = cl->data;
	for (i=0; ld && i<end; i++) {
		if (i >= start) {
			t = InventoryItem::create(ld->content,ld->count,0,ld->data);
			r = t->createCookResult();
			if (!r || r->getContent() == CONTENT_IGNORE) {
				delete t;
				delete r;
				ld = ld->next;
				continue;
			}
			delete r;
			l->addItem(t);
		}
		ld = ld->next;
	}

	return true;
}
std::string CookBookNodeMetadata::getDrawSpecString(Player *player)
{
	char buff[256];
	contentlist_t *cl;
	uint32_t list_size = 0;

	cl = content_list_get("cooking");
	if (cl)
		list_size = list_count(&cl->data);

	snprintf(buff,256,gettext("Page %d of %d"),(int)(m_page+1),(int)((list_size/40)+1));

	std::string spec("size[8,9]");
	spec +=	"label[0.5,0.75;";
	spec += gettext("Add item here to see cook result");;
	spec += "]";
	spec +=	"list[current_name;result;2,1;1,1;]";
	spec +=	"list[current_name;recipe;4,1;1,1;]";
	spec +=	"button[0.25,3.5;2.5,0.75;prev;";
	spec += gettext("<< Previous Page");;
	spec += "]";
	spec +=	"label[3.5,3.5;";
	spec += buff;
	spec += "]";
	spec +=	"button[6,3.5;2.5,0.75;next;";
	spec += gettext("Next Page >>");;
	spec += "]";
	spec +=	"list[current_name;list;0,4;8,5;]";
	return spec;
}

/*
	DeCraftNodeMetadata
*/

// Prototype
DeCraftNodeMetadata proto_DeCraftNodeMetadata;

DeCraftNodeMetadata::DeCraftNodeMetadata()
{
	NodeMetadata::registerType(typeId(), create);

	m_page = 0;

	m_inventory = new Inventory();
	m_inventory->addList("list", 300);
	m_inventory->addList("recipe", 1);
	m_inventory->addList("random", 1);
	m_inventory->addList("result", 1);
}
DeCraftNodeMetadata::~DeCraftNodeMetadata()
{
	delete m_inventory;
}
u16 DeCraftNodeMetadata::typeId() const
{
	return CONTENT_DECRAFT_BOOK_OPEN;
}
NodeMetadata* DeCraftNodeMetadata::clone()
{
	InventoryList *l;
	InventoryItem *t;
	contentlist_t *cl;
	listdata_t *ld;
	uint32_t list_size;
	uint32_t start;
	uint32_t end;
	uint32_t i;
	ContentFeatures *f;

	DeCraftNodeMetadata *d = new DeCraftNodeMetadata();
	*d->m_inventory = *m_inventory;
	d->m_page = m_page;

	l = d->m_inventory->getList("list");
	l->clearItems();

	cl = content_list_get("decrafting");
	if (!cl)
		return d;

	list_size = list_count(&cl->data);

	start = m_page*40;
	end = start+40;
	if (end > list_size)
		end = list_size;

	ld = cl->data;
	for (i=0; ld && i<end; i++) {
		if (i >= start) {
			if ((ld->content&CONTENT_CRAFTITEM_MASK) == CONTENT_CRAFTITEM_MASK)
				continue;
			if ((ld->content&CONTENT_TOOLITEM_MASK) == CONTENT_TOOLITEM_MASK)
				continue;
			if ((ld->content&CONTENT_CLOTHESITEM_MASK) == CONTENT_CLOTHESITEM_MASK)
				continue;
			f = &content_features(ld->content);
			if (!f || (f->dug_item == "" && f->extra_dug_item == ""))
				continue;

			t = InventoryItem::create(ld->content,ld->count,0,ld->data);
			l->addItem(t);
		}
		ld = ld->next;
	}

	return d;
}
NodeMetadata* DeCraftNodeMetadata::create(std::istream &is)
{
	DeCraftNodeMetadata *d = new DeCraftNodeMetadata();

	d->m_inventory->deSerialize(is);
	is>>d->m_page;

	return d;
}
void DeCraftNodeMetadata::serializeBody(std::ostream &os)
{
	m_inventory->serialize(os);
	os<<itos(m_page) << " ";
}
bool DeCraftNodeMetadata::nodeRemovalDisabled()
{
	return false;
}
void DeCraftNodeMetadata::inventoryModified()
{
	vlprintf(CN_INFO,"DeCraft inventory modification callback");
}
bool DeCraftNodeMetadata::step(float dtime, v3s16 pos, ServerEnvironment *env)
{
	InventoryList *l = m_inventory->getList("result");
	InventoryItem *t = l->getItem(0);
	if (!t || t->getContent() == CONTENT_IGNORE)
		return false;
	if ((t->getContent()&CONTENT_CRAFTITEM_MASK) == CONTENT_CRAFTITEM_MASK)
		return false;
	if ((t->getContent()&CONTENT_TOOLITEM_MASK) == CONTENT_TOOLITEM_MASK)
		return false;
	if ((t->getContent()&CONTENT_CLOTHESITEM_MASK) == CONTENT_CLOTHESITEM_MASK)
		return false;
	if (content_features(t->getContent()).dug_item == "" && content_features(t->getContent()).extra_dug_item == "")
		return false;
	l = m_inventory->getList("recipe");
	l->clearItems();
	if (content_features(t->getContent()).dug_item != "") {
		std::istringstream is(content_features(t->getContent()).dug_item, std::ios::binary);
		InventoryItem *r = InventoryItem::deSerialize(is);
		if (!r)
			return false;
		if (r->getContent() == CONTENT_IGNORE) {
			delete r;
			return false;
		}
		l->addItem(0,r);
	}
	l = m_inventory->getList("random");
	l->clearItems();
	if (content_features(t->getContent()).extra_dug_item != "") {
		std::istringstream is(content_features(t->getContent()).extra_dug_item, std::ios::binary);
		InventoryItem *r = InventoryItem::deSerialize(is);
		if (!r)
			return true;
		if (r->getContent() == CONTENT_IGNORE) {
			delete r;
			return true;
		}
		l->addItem(0,r);
	}
	return true;
}
bool DeCraftNodeMetadata::import(NodeMetadata *meta)
{
	InventoryList *l;
	InventoryItem *t;
	contentlist_t *cl;
	listdata_t *ld;
	uint32_t list_size;
	uint32_t start;
	uint32_t end;
	uint32_t i;
	ContentFeatures *f;

	if (meta->typeId() == CONTENT_BOOK)
		m_page = ((ClosedBookNodeMetadata*)meta)->getPage();

	if (m_page < 0)
		m_page = 0;

	l = m_inventory->getList("list");
	l->clearItems();

	cl = content_list_get("decrafting");
	if (!cl)
		return true;

	list_size = list_count(&cl->data);

	if (m_page > (list_size/40))
		m_page = list_size/40;

	start = m_page*40;
	end = start+40;
	if (end > list_size)
		end = list_size;

	ld = cl->data;
	for (i=0; ld && i<end; i++) {
		if (i >= start) {
			if ((ld->content&CONTENT_CRAFTITEM_MASK) == CONTENT_CRAFTITEM_MASK)
				continue;
			if ((ld->content&CONTENT_TOOLITEM_MASK) == CONTENT_TOOLITEM_MASK)
				continue;
			if ((ld->content&CONTENT_CLOTHESITEM_MASK) == CONTENT_CLOTHESITEM_MASK)
				continue;
			f = &content_features(ld->content);
			if (!f || (f->dug_item == "" && f->extra_dug_item == ""))
				continue;

			t = InventoryItem::create(ld->content,ld->count,0,ld->data);
			l->addItem(t);
		}
		ld = ld->next;
	}

	return true;
}
bool DeCraftNodeMetadata::receiveFields(std::string formname, std::map<std::string, std::string> fields, Player *player)
{
	InventoryList *l;
	InventoryItem *t;
	contentlist_t *cl;
	listdata_t *ld;
	uint32_t list_size;
	uint32_t start;
	uint32_t end;
	uint32_t i;
	ContentFeatures *f;

	if (fields["prev"] == "" && fields["next"] == "")
		return false;

	cl = content_list_get("decrafting");
	if (!cl)
		return false;

	list_size = list_count(&cl->data);

	if (fields["prev"] != "") {
		if (m_page > 0) {
			m_page--;
		}else{
			m_page = list_size/40;
		}
	}
	if (fields["next"] != "")
		m_page++;
	if (m_page > (list_size/40))
		m_page = 0;

	l = m_inventory->getList("list");
	l->clearItems();

	start = m_page*40;
	end = start+40;
	if (end > list_size)
		end = list_size;

	ld = cl->data;
	for (i=0; ld && i<end; i++) {
		if (i >= start) {
			if ((ld->content&CONTENT_CRAFTITEM_MASK) == CONTENT_CRAFTITEM_MASK)
				continue;
			if ((ld->content&CONTENT_TOOLITEM_MASK) == CONTENT_TOOLITEM_MASK)
				continue;
			if ((ld->content&CONTENT_CLOTHESITEM_MASK) == CONTENT_CLOTHESITEM_MASK)
				continue;
			f = &content_features(ld->content);
			if (!f || (f->dug_item == "" && f->extra_dug_item == ""))
				continue;

			t = InventoryItem::create(ld->content,ld->count,0,ld->data);
			l->addItem(t);
		}
		ld = ld->next;
	}

	return true;
}
std::string DeCraftNodeMetadata::getDrawSpecString(Player *player)
{
	char buff[256];
	contentlist_t *cl;
	uint32_t list_size = 0;

	cl = content_list_get("decrafting");
	if (cl)
		list_size = list_count(&cl->data);

	snprintf(buff,256,gettext("Page %d of %d"),(int)(m_page+1),(int)((list_size/40)+1));

	std::string spec("size[8,9]");
	spec +=	"label[0.5,0.75;";
	spec += gettext("Add item here to see dig result");;
	spec += "]";
	spec +=	"list[current_name;result;2,1;1,1;]";
	spec +=	"label[5,1;";
	spec += gettext("Dig Result");;
	spec += "]";
	spec +=	"list[current_name;recipe;6.5,0.5;1,1;]";
	spec +=	"label[5,2;";
	spec += gettext("Random Drop");;
	spec += "]";
	spec +=	"list[current_name;random;6.5,1.5;1,1;]";
	spec +=	"button[0.25,3.5;2.5,0.75;prev;";
	spec += gettext("<< Previous Page");;
	spec += "]";
	spec +=	"label[3.5,3.5;";
	spec += buff;
	spec += "]";
	spec +=	"button[6,3.5;2.5,0.75;next;";
	spec += gettext("Next Page >>");;
	spec += "]";
	spec +=	"list[current_name;list;0,4;8,5;]";
	return spec;
}
