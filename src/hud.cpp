/************************************************************************
* hud.cpp
* voxelands - 3d voxel world sandbox game
* Copyright (C) Lisa 'darkrose' Milne 2015 <lisa@ltmnet.com>
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

#include "hud.h"
#include "common_irrlicht.h"
#include <IGUIFont.h>
#include "path.h"
#include "main.h"
#include "log.h"
#include "game.h"
#include "intl.h"
#include "environment.h"

video::ITexture *getTexture(video::IVideoDriver *driver, const char* tex)
{
	char buff[1024];
	if (!path_get((char*)"texture",const_cast<char*>(tex),1,buff,1024))
		return NULL;

	return driver->getTexture(buff);
}

void draw_image(
	video::IVideoDriver *driver,
	video::ITexture *texture,
	const video::SColor &colour,
	core::rect<s32> drect,
	const core::rect<s32> *srect,
	const core::rect<s32> *clip
)
{
	const video::SColor colours[] = {colour,colour,colour,colour};
	core::rect<s32> s_rect;
	if (!srect) {
		s_rect = core::rect<s32>(
			core::position2d<s32>(0,0),
			core::dimension2di(texture->getOriginalSize())
		);
		srect = &s_rect;
	}
	driver->draw2DImage(
		texture,
		drect,
		*srect,
		clip,
		colours,
		true
	);
}

void draw_inv_item(
	video::IVideoDriver *driver,
	gui::IGUIFont *font,
	InventoryItem *item,
	core::rect<s32> rect,
	const core::rect<s32> *clip
)
{
	if (item == NULL)
		return;

	video::ITexture *texture = NULL;
	texture = item->getImage();

	if (texture != NULL) {
		const video::SColor color(255,255,255,255);
		draw_image(driver, texture, color, rect,NULL,clip);
	}else{
		video::SColor bgcolor(255,50,50,128);
		driver->draw2DRectangle(bgcolor, rect, clip);
	}

	if (font != NULL) {
		std::string text = item->getText();
		if (font && text != "") {
			v2u32 dim = font->getDimension(narrow_to_wide(text).c_str());
			v2s32 sdim(dim.X,dim.Y);

			core::rect<s32> rect2(
				rect.LowerRightCorner - sdim,
				sdim
			);

			video::SColor bgcolor(128,0,0,0);
			driver->draw2DRectangle(bgcolor, rect2, clip);

			font->draw(
				text.c_str(),
				rect2,
				video::SColor(255,255,255,255),
				false,
				false,
				clip
			);
		}
	}
}

void draw_progress_ring(
	video::IVideoDriver *driver,
	v2s32 screensize,
	core::position2d<s32> pos,
	s32 radius,
	s16 value,
	video::SColor color
)
{
	if (!value)
		return;
	video::ITexture *texture = getTexture(driver,"progress_ring.png");
	core::rect<s32> rect(pos.X-radius,pos.Y-radius,pos.X+radius,pos.Y+radius);
	if (value >= 25) {
		if (value >= 50) {
			{
				core::rect<s32> drect(rect);
				drect.LowerRightCorner.X -= radius;
				core::rect<s32> srect(
					core::position2d<s32>(0,0),
					core::dimension2di(texture->getOriginalSize())
				);
				srect.LowerRightCorner.X /= 2;
				draw_image(driver,texture,color,drect,&srect,NULL);
			}
			if (value >= 75) {
				if (value > 99) {
					{
						core::rect<s32> drect(rect);
						drect.UpperLeftCorner.X += radius;
						core::rect<s32> srect(
							core::position2d<s32>(0,0),
							core::dimension2di(texture->getOriginalSize())
						);
						srect.UpperLeftCorner.X += srect.LowerRightCorner.X/2;
						draw_image(driver,texture,color,drect,&srect,NULL);
					}
				}else{ // top right corner
					{
						core::rect<s32> drect(rect);
						drect.UpperLeftCorner.X += radius;
						drect.UpperLeftCorner.Y += radius;
						core::rect<s32> srect(
							core::position2d<s32>(0,0),
							core::dimension2di(texture->getOriginalSize())
						);
						srect.UpperLeftCorner.X += srect.LowerRightCorner.X/2;
						srect.UpperLeftCorner.Y += srect.LowerRightCorner.Y/2;
						draw_image(driver,texture,color,drect,&srect,NULL);
					}
					{
						float os = (float)((value%25)*4)/100.0;
						core::rect<s32> drect(rect);;
						drect.UpperLeftCorner.X += radius+(radius-((float)radius*os));
						drect.UpperLeftCorner.Y += (radius-((float)radius*os));
						drect.LowerRightCorner.Y -= radius;
						core::rect<s32> srect(
							core::position2d<s32>(0,0),
							core::dimension2di(texture->getOriginalSize())
						);
						float v = srect.LowerRightCorner.X/2;
						srect.UpperLeftCorner.X += v+(v-((float)v*os));
						v = srect.LowerRightCorner.Y/2;
						srect.UpperLeftCorner.Y += (v-((float)v*os));
						srect.LowerRightCorner.Y -= v;
						draw_image(driver,texture,color,drect,&srect,NULL);
					}
				}
			}else{ // bottom right corner
				{
					float os = (float)((value%25)*4)/100.0;
					core::rect<s32> drect(rect);
					drect.UpperLeftCorner.Y += radius+(radius-((float)radius*os));
					drect.UpperLeftCorner.X += radius;
					drect.LowerRightCorner.X -= (radius-((float)radius*os));
					core::rect<s32> srect(
						core::position2d<s32>(0,0),
						core::dimension2di(texture->getOriginalSize())
					);
					float v = srect.LowerRightCorner.X/2;
					srect.UpperLeftCorner.X += v;
					srect.LowerRightCorner.X -= (v-((float)v*os));
					v = srect.LowerRightCorner.Y/2;
					srect.UpperLeftCorner.Y += v+(v-((float)v*os));
					draw_image(driver,texture,color,drect,&srect,NULL);
				}
			}
		}else{ // bottom left corner
			{
				core::rect<s32> drect(rect);
				drect.LowerRightCorner.X -= radius;
				drect.LowerRightCorner.Y -= radius;
				core::rect<s32> srect(
					core::position2d<s32>(0,0),
					core::dimension2di(texture->getOriginalSize())
				);
				srect.LowerRightCorner.X /= 2;
				srect.LowerRightCorner.Y /= 2;
				draw_image(driver,texture,color,drect,&srect,NULL);
			}
			{
				float os = (float)((value%25)*4)/100.0;
				core::rect<s32> drect(rect);
				drect.UpperLeftCorner.Y += radius;
				drect.LowerRightCorner.X -= radius+((radius-(float)radius*os));
				drect.LowerRightCorner.Y -= (radius-((float)radius*os));
				core::rect<s32> srect(
					core::position2d<s32>(0,0),
					core::dimension2di(texture->getOriginalSize())
				);
				float v = srect.LowerRightCorner.X/2;
				srect.LowerRightCorner.X -= v+(v-((float)v*os));
				v = srect.LowerRightCorner.Y/2;
				srect.UpperLeftCorner.Y += v;
				srect.LowerRightCorner.Y -= (v-((float)v*os));
				draw_image(driver,texture,color,drect,&srect,NULL);
			}
		}
	}else{ // top left corner
		{
			float os = (float)((value%25)*4)/100.0;
			core::rect<s32> drect(rect);
			drect.UpperLeftCorner.X += radius-((float)radius*os);
			drect.LowerRightCorner.X -= radius;
			drect.LowerRightCorner.Y -= radius+(radius-((float)radius*os));
			core::rect<s32> srect(
				core::position2d<s32>(0,0),
				core::dimension2di(texture->getOriginalSize())
			);
			float v = srect.LowerRightCorner.X/2;
			srect.UpperLeftCorner.X += v-((float)v*os);
			srect.LowerRightCorner.X /= 2;
			v = srect.LowerRightCorner.Y/2;
			srect.LowerRightCorner.Y -= v+(v-((float)v*os));
			draw_image(driver,texture,color,drect,&srect,NULL);
		}
	}
}

/*
	Hotbar draw routine
*/
void hud_draw_old(
	video::IVideoDriver *driver,
	gui::IGUIFont *font,
	v2s32 centrelowerpos,
	Inventory *inventory,
	s32 halfheartcount,
	s32 halfbubblecount,
	s32 halfhungercount
)
{
	InventoryList *mainlist = inventory->getList("main");
	if (mainlist == NULL) {
		errorstream<<"draw_hotbar(): mainlist == NULL"<<std::endl;
		return;
	}

	s32 imagesize = 64;

	if (centrelowerpos.Y <= 400) {
		imagesize = 32;
	}else if (centrelowerpos.Y <= 640) {
		imagesize = 48;
	}

	s32 padding = imagesize/12;
	s32 width = 8*(imagesize+padding*2);

	// Position of upper left corner of bar
	v2s32 pos = centrelowerpos - v2s32(width/2, imagesize+padding*2);

	core::rect<s32> imgrect(0,0,imagesize,imagesize);

	std::wstring selected = L"";

	for (s32 i=0; i<8; i++) {
		InventoryItem *item = mainlist->getItem(i);

		core::rect<s32> rect = imgrect + pos + v2s32(padding+i*(imagesize+padding*2), padding);

		if (g_selected_item == i) {
			video::SColor c_outside(255,255,0,0);
			s32 xo = (rect.LowerRightCorner.X-rect.UpperLeftCorner.X) / 4;
			s32 yo = (rect.LowerRightCorner.Y-rect.UpperLeftCorner.Y) / 4;
			rect.LowerRightCorner.X += xo;
			rect.UpperLeftCorner.Y -= yo;
			pos.X += xo;
			video::SColor bgcolor2(128,0,0,0);
			driver->draw2DRectangle(bgcolor2, rect, NULL);
			driver->draw2DRectangleOutline(rect, c_outside);

			if (item != NULL) {
				draw_inv_item(driver, font, item, rect, NULL);
				std::wstring name = item->getGuiName();
				if (name != L"")
					selected = name;
			}
		}else{
			video::SColor bgcolor2(128,0,0,0);
			driver->draw2DRectangle(bgcolor2, rect, NULL);

			if (item != NULL)
				draw_inv_item(driver, font, item, rect, NULL);
		}
	}

	/*
		Draw hearts
	*/
	struct {
		s32 count;
		s32 halfcount;
		const char* texture;
		bool show_full;
	} barData[3] = {
		{halfheartcount/2,halfheartcount,"heart.png",true},
		{halfbubblecount/2,halfbubblecount,"bubble.png",false},
		{halfhungercount/2,halfhungercount,"harvested_carrot.png",true},
	};
	v2s32 bar_base(0,-25);
	for (s32 k=0; k<3; k++) {
		if (barData[k].count == 10 && !barData[k].show_full)
			continue;
		video::ITexture *texture = getTexture(driver,barData[k].texture);
		v2s32 p = pos + bar_base;
		for (s32 i=0; i<barData[k].count; i++) {
			const video::SColor color(255,255,255,255);
			core::rect<s32> rect(0,0,16,16);
			rect += p;
			draw_image(driver,texture,color,rect,NULL,NULL);
			p += v2s32(16,0);
		}
		if (barData[k].halfcount % 2 == 1) {
			const video::SColor color(255,255,255,255);
			core::rect<s32> rect(0,0,16/2,16);
			rect += p;
			core::rect<s32> srect(
				core::position2d<s32>(0,0),
				core::dimension2di(texture->getOriginalSize())
			);
			srect.LowerRightCorner.X /= 2;
			draw_image(driver,texture,color,rect,&srect,NULL);
			p += v2s32(16,0);
		}
		bar_base.Y -= 20;
	}
	if (selected != L"") {
		v2u32 dim = font->getDimension(selected.c_str());
		v2s32 sdim(dim.X,dim.Y);
		v2s32 p = pos + v2s32(170, -(24+(sdim.Y-16)));

		core::rect<s32> rect2(
			p,
			sdim
		);
		font->draw(selected.c_str(), rect2, video::SColor(255,255,255,255), false, false, NULL);
	}
}

void hud_draw(
	video::IVideoDriver *driver,
	gui::IGUIFont *font,
	v2s32 screensize,
	bool show_index,
	Inventory *inventory,
	bool have_health,
	s32 health,
	u8 damage_type,
	u8 damage_pos,
	float cold_boost,
	bool have_suffocation,
	s32 air,
	bool have_hunger,
	s32 hunger,
	float energy,
	float energy_boost,
	int crosshair,
	bool nodeinfo,
	bool selected,
	v3s16 pos,
	MapNode node,
	uint8_t biome,
	u32 time
)
{
	InventoryList *mainlist = inventory->getList("main");
	if (mainlist == NULL) {
		errorstream<<"draw_hotbar(): mainlist == NULL"<<std::endl;
		return;
	}

	// background
	{
		const video::SColor color(255,255,255,255);
		video::ITexture *texture = getTexture(driver,"ringbg.png");
		core::rect<s32> rect(screensize.X-165,screensize.Y-165,screensize.X-19,screensize.Y-19);
		draw_image(driver,texture,color,rect,NULL,NULL);
	}

	// wielded item
	{
		InventoryItem *item = mainlist->getItem(g_selected_item);
		const video::SColor color(255,255,255,255);
		core::rect<s32> rect(screensize.X-124,screensize.Y-124,screensize.X-60,screensize.Y-60);
		video::ITexture *texture = NULL;

		std::wstring selected = L"";
		std::wstring txt = L"";

		if (item != NULL) {
			texture = item->getImageRaw();
			if (texture == NULL)
				texture = item->getImage();
			std::wstring name = item->getGuiName();
			if (name != L"")
				selected = name;
			content_t type = item->getContent();
			if ((type&CONTENT_TOOLITEM_MASK) == CONTENT_TOOLITEM_MASK || (type&CONTENT_CLOTHESITEM_MASK) == CONTENT_CLOTHESITEM_MASK) {
				float w = item->getWear();

				if ((type&CONTENT_CLOTHESITEM_MASK) == CONTENT_CLOTHESITEM_MASK) {
					w = 100.0-((100.0/65535.0)*w);
				}else{
					float wt = (float)content_toolitem_features(type).diginfo.uses;
					w = ((100.0/wt)*w);
				}
				txt = itows(w);
				txt += L"%";
				{

					video::SColor bcolor[10] = {
						video::SColor(255,255,0,0),
						video::SColor(255,255,40,0),
						video::SColor(255,255,80,0),
						video::SColor(255,255,110,0),
						video::SColor(255,255,120,0),
						video::SColor(255,255,140,0),
						video::SColor(255,255,160,0),
						video::SColor(255,170,180,0),
						video::SColor(255,50,200,0),
						video::SColor(255,0,255,0)
					};
					int i = ((int)w)/10;
					if (i < 0)
						i = 0;
					if (i > 9)
						i = 9;
					draw_progress_ring(driver,screensize,core::position2d<s32>(screensize.X-92,screensize.Y-92),40,w,bcolor[i]);
				}
			}else{
				txt = itows(item->getCount());
			}
		}else{
			texture = getTexture(driver,"wieldhand.png");
		}

		draw_image(driver,texture,color,rect,NULL,NULL);

		// item count/wear
		if (txt != L"") {
			v2u32 dim = font->getDimension(txt.c_str());
			v2s32 sdim(dim.X,dim.Y);
			v2s32 p(screensize.X-92,screensize.Y-44);
			p -= v2s32(sdim.X/2, sdim.Y/2);
			core::rect<s32> rect2(
				p,
				sdim
			);
			font->draw(txt.c_str(), rect2, video::SColor(255,255,255,255), false, false, NULL);
		}
		// tool name
		if (selected != L"") {
			v2u32 dim = font->getDimension(selected.c_str());
			v2s32 sdim(dim.X,dim.Y);
			v2s32 p = v2s32(screensize.X-92,screensize.Y-30);
			p -= v2s32(sdim.X/2, sdim.Y/2);
			if (sdim.X > 175)
				p.X -= (sdim.X-175)/2;

			core::rect<s32> rect2(
				p,
				sdim
			);
			font->draw(selected.c_str(), rect2, video::SColor(255,255,255,255), false, false, NULL);
		}
	}

	// rest of the wieldring
	{
		const video::SColor color(255,255,255,255);

		v2s32 pos[8] = {
			v2s32(0,-56),
			v2s32(39,-39),
			v2s32(56,0),
			v2s32(39,39),
			v2s32(0,56),
			v2s32(-39,39),
			v2s32(-56,0),
			v2s32(-39,-39)
		};

		v2s32 ipos[8] = {
			v2s32(10,-68),
			v2s32(69,-44),
			v2s32(90,12),
			v2s32(69,68),
			v2s32(0,56),
			v2s32(-44,68),
			v2s32(-66,12),
			v2s32(-44,-44)
		};

		core::rect<s32> base_rect(screensize.X-104,screensize.Y-104,screensize.X-80,screensize.Y-80);
		for (s32 i=g_selected_item+1, p=3; ; i++,p--) {
			if (i >= 8)
				i = 0;
			if (i == g_selected_item)
				break;
			if (p < 0)
				p = 7;
			InventoryItem *item = mainlist->getItem(i);

			core::rect<s32> rect = base_rect + pos[p];

			if (item != NULL) {
				video::ITexture *texture = item->getImageRaw();
				if (texture == NULL)
					texture = item->getImage();
				draw_image(driver,texture,color,rect,NULL,NULL);
			}

			if (show_index) {
				std::wstring txt = itows(i+1);
				v2u32 dim = font->getDimension(txt.c_str());
				v2s32 sdim(dim.X,dim.Y);
				v2s32 pos = ipos[p] + base_rect.UpperLeftCorner;
				pos -= v2s32(sdim.X/2, sdim.Y/2);
				core::rect<s32> rect2(
					pos,
					sdim
				);
				font->draw(txt.c_str(), rect2, video::SColor(255,255,255,255), false, false, NULL);
			}
		}
	}

	// inventory status
	{
		const char* states[3] = {
			"inventory.png",
			"inventory_full.png",
			"inventory_max.png"
		};
		int state = 2;
		{
			u32 count = mainlist->getSize();
			for (u32 i=0; i<count; i++) {
				InventoryItem *item = mainlist->getItem(i);
				if (!item) {
					state = 0;
					break;
				}
				if (state == 2 && item->freeSpace())
					state = 1;
			}
		}
		const video::SColor color(255,255,255,255);
		video::ITexture *texture = getTexture(driver,states[state]);
		core::rect<s32> rect(screensize.X-51,screensize.Y-186,screensize.X-19,screensize.Y-154);

		draw_image(driver,texture,color,rect,NULL,NULL);
	}

	// health
	if (have_health) {
		int c = 55+(health*2);
		float e = 0.0;
		if (health > 0.0)
			e = energy/((float)health/100.0);
		if (e > 100.0)
			e = 100.0;
		if (e < 0.0)
			e = 0.0;
		{
			u8 r = c;
			u8 b = 0;
			if (energy_boost > 0.0) {
				r = 0;
				b = c;
			}
			const video::SColor color(255,r,0,b);
			draw_progress_ring(driver,screensize,core::position2d<s32>(84,screensize.Y-84),30,e,color);
		}
		{
			u8 r = c;
			u8 b = 0;
			if (cold_boost > 0.0) {
				r = 0;
				b = c;
			}
			const video::SColor color(220,r,0,b);
			video::ITexture *texture = getTexture(driver,"heart.png");
			core::rect<s32> rect(60,screensize.Y-108,108,screensize.Y-60);
			draw_image(driver,texture,color,rect,NULL,NULL);
		}

		std::wstring txt = itows(health);
		txt += L"%";

		v2u32 dim = font->getDimension(txt.c_str());
		v2s32 sdim(dim.X,dim.Y);
		v2s32 p(84,screensize.Y-84);
		p -= v2s32(sdim.X/2, sdim.Y/2);
		core::rect<s32> rect2(
			p,
			sdim
		);
		font->draw(txt.c_str(), rect2, video::SColor(255,255,255,255), false, false, NULL);

		{
			char* image[8] = {
				(char*)"body_feet.png",
				(char*)"body_lleg.png",
				(char*)"body_rleg.png",
				(char*)"body_torso.png",
				(char*)"body_hands.png",
				(char*)"body_larm.png",
				(char*)"body_rarm.png",
				(char*)"body_head.png"
			};

			for (int i=0; i<8; i++) {
				u8 a = (1<<i);
				u8 c = 255;
				if ((damage_pos&a) == a)
					c = 0;
				const video::SColor color(255,255,c,c);
				video::ITexture *texture = getTexture(driver,image[i]);
				core::rect<s32> rect(10,screensize.Y-172,42,screensize.Y-108);
				draw_image(driver,texture,color,rect,NULL,NULL);
			}
		}
		{
			char* damage_types[] = {
				(char*)"",
				gettext("Unknown"),
				gettext("Fall"),
				gettext("Exposure"),
				gettext("Cold"),
				gettext("Attack"),
				gettext("Vacuum of Space"),
				gettext("Hunger"),
				gettext("Air"),
				gettext("Lava"),
				gettext("Cactus"),
				gettext("Fire"),
				gettext("TNT"),
				gettext("Steam"),
				gettext("Poison"),
				gettext("Shadows")
			};

			std::wstring t = narrow_to_wide(damage_types[damage_type]);

			v2u32 dim = font->getDimension(t.c_str());
			v2s32 sdim(dim.X,dim.Y);
			v2s32 p(42,screensize.Y-140);
			p.Y -= sdim.Y/2;
			core::rect<s32> rect2(
				p,
				sdim
			);
			font->draw(t.c_str(), rect2, video::SColor(255,255,255,255), false, false, NULL);
		}
	}
	// air
	if (have_suffocation && air < 100) {
		int c = 55+(air*2);
		const video::SColor color(255,255,c,c);
		video::ITexture *texture = getTexture(driver,"bubble.png");
		core::rect<s32> rect(100,screensize.Y-68,132,screensize.Y-36);
		draw_image(driver,texture,color,rect,NULL,NULL);

		std::wstring txt = itows(air);
		txt += L"%";

		v2u32 dim = font->getDimension(txt.c_str());
		v2s32 sdim(dim.X,dim.Y);
		v2s32 p(116,screensize.Y-52);
		p -= v2s32(sdim.X/2, sdim.Y/2);
		core::rect<s32> rect2(
			p,
			sdim
		);
		font->draw(txt.c_str(), rect2, video::SColor(255,255,255,255), false, false, NULL);
	}
	// hunger
	if (have_hunger) {
		int c = 55+(hunger*2);
		const video::SColor color(255,255,c,c);
		video::ITexture *texture = getTexture(driver,"harvested_carrot.png");
		core::rect<s32> rect(36,screensize.Y-68,68,screensize.Y-36);
		draw_image(driver,texture,color,rect,NULL,NULL);

		std::wstring txt = itows(hunger);
		txt += L"%";

		v2u32 dim = font->getDimension(txt.c_str());
		v2s32 sdim(dim.X,dim.Y);
		v2s32 p(52,screensize.Y-52);
		p -= v2s32(sdim.X/2, sdim.Y/2);
		core::rect<s32> rect2(
			p,
			sdim
		);
		font->draw(txt.c_str(), rect2, video::SColor(255,255,255,255), false, false, NULL);
	}

	// crosshair
	if (crosshair) {
		u8 gb = 255;
		if (crosshair > 2)
			gb = 0;
		const video::SColor color(220,255,gb,gb);
		video::ITexture *texture;
		if (crosshair == 1) {
			texture = getTexture(driver,"crosshair_unfocused.png");
		}else{
			texture = getTexture(driver,"crosshair_focused.png");
		}
		core::rect<s32> rect((screensize.X/2)-16,(screensize.Y/2)-16,(screensize.X/2)+16,(screensize.Y/2)+16);
		draw_image(driver,texture,color,rect,NULL,NULL);
	}

	if (nodeinfo) {
		ContentFeatures *f = &content_features(node);
		std::wstring txt(L"Node: (");
		txt += itows(pos.X);
		txt += L",";
		txt += itows(pos.Y);
		txt += L",";
		txt += itows(pos.Z);
		txt += L") '";
		txt += narrow_to_wide(f->description);
		txt += L"' light=";
		if (f->light_propagates || f->sunlight_propagates) {
			txt += L"true ";
		}else{
			txt += L"false ";
		}
		txt += L"param1=";
		switch (f->param_type) {
		case CPT_NONE:
			txt += L"(CPT_NONE)";
			break;
		case CPT_LIGHT:
			txt += L"(CPT_LIGHT)";
			break;
		case CPT_MINERAL:
			txt += L"(CPT_MINERAL)";
			break;
		case CPT_FACEDIR_SIMPLE:
			txt += L"(CPT_FACEDIR_SIMPLE)";
			break;
		case CPT_FACEDIR_WALLMOUNT:
			txt += L"(CPT_FACEDIR_WALLMOUNT)";
			break;
		case CPT_LIQUID:
			txt += L"(CPT_LIQUID)";
			break;
		case CPT_PLANTGROWTH:
			txt += L"(CPT_PLANTGROWTH)";
			break;
		case CPT_SPECIAL:
			txt += L"(CPT_SPECIAL)";
			break;
		case CPT_BLOCKDATA:
			txt += L"(CPT_BLOCKDATA)";
			break;
		default:
			txt += L"(UNKNOWN)";
			break;
		}
		{
			char buff[100];
			sprintf(buff,"(%u)(0x%.2X)",node.param1,node.param1);
			txt += narrow_to_wide(buff);
		}
		txt += L" param2=";
		switch (f->param2_type) {
		case CPT_NONE:
			txt += L"(CPT_NONE)";
			break;
		case CPT_LIGHT:
			txt += L"(CPT_LIGHT)";
			break;
		case CPT_MINERAL:
			txt += L"(CPT_MINERAL)";
			break;
		case CPT_FACEDIR_SIMPLE:
			txt += L"(CPT_FACEDIR_SIMPLE)";
			break;
		case CPT_FACEDIR_WALLMOUNT:
			txt += L"(CPT_FACEDIR_WALLMOUNT)";
			break;
		case CPT_LIQUID:
			txt += L"(CPT_LIQUID)";
			break;
		case CPT_PLANTGROWTH:
			txt += L"(CPT_PLANTGROWTH)";
			break;
		case CPT_SPECIAL:
			txt += L"(CPT_SPECIAL)";
			break;
		case CPT_BLOCKDATA:
			txt += L"(CPT_BLOCKDATA)";
			break;
		default:
			txt += L"(UNKNOWN)";
			break;
		}
		{
			char buff[100];
			sprintf(buff,"(%u)(0x%.2X)",node.param2,node.param2);
			txt += narrow_to_wide(buff);
		}
		if (selected)
			txt += L" (selected)";

		txt += L"\nBiome: ";
		switch (biome) {
		case BIOME_WOODLANDS:
			txt += L"Woodlands";
			break;
		case BIOME_JUNGLE:
			txt += L"Jungle";
			break;
		case BIOME_OCEAN:
			txt += L"Ocean";
			break;
		case BIOME_DESERT:
			txt += L"Desert";
			break;
		case BIOME_PLAINS:
			txt += L"Plains";
			break;
		case BIOME_FOREST:
			txt += L"Forest";
			break;
		case BIOME_SNOWCAP:
			txt += L"Snowcaps";
			break;
		case BIOME_LAKE:
			txt += L"Lake";
			break;
		case BIOME_BEACH:
			txt += L"Beach";
			break;
		case BIOME_SPACE:
			txt += L"Space";
			break;
		case BIOME_THEDEEP:
			txt += L"The Deep";
			break;
		case BIOME_SKY:
			txt += L"Sky";
			break;
		case BIOME_WASTELANDS:
			txt += L"Wastelands";
			break;
		case BIOME_UNKNOWN:
		default:
			txt += L"Unknown";
			break;
		}

		v2u32 dim = font->getDimension(txt.c_str());
		v2s32 sdim(dim.X,dim.Y);
		v2s32 p(100,screensize.Y-200);
		core::rect<s32> rect2(
			p,
			sdim
		);
		font->draw(txt.c_str(), rect2, video::SColor(255,255,255,255), false, false, NULL);
	}

	if (time != 0) {
		u32 day = time%240;
		u32 year = time/240;
		u32 season = day/60;

		std::string txt("");

		switch (season) {
		case ENV_SEASON_SUMMER:
			txt += gettext("Summer");
			break;
		case ENV_SEASON_AUTUMN:
			txt += gettext("Autumn");
			break;
		case ENV_SEASON_WINTER:
			txt += gettext("Winter");
			break;
		case ENV_SEASON_SPRING:
			txt += gettext("Spring");
			break;
		default:;
			txt += "???";
		}

		txt += ", ";

		{
			char buff[256];
			snprintf(buff,256,gettext("Day %u of Year %u"),day+1,year+1);

			txt += buff;
		}

		std::wstring wtxt = narrow_to_wide(txt);

		v2u32 dim = font->getDimension(wtxt.c_str());
		v2s32 sdim(dim.X,dim.Y);
		v2s32 p(screensize.X-(sdim.X+10),5);
		core::rect<s32> rect2(
			p,
			sdim
		);
		font->draw(wtxt.c_str(), rect2, video::SColor(255,255,255,255), false, false, NULL);
	}

	{
		int x_off = 0;
		char buff[512];
		if (energy_boost > 0.0) {
			int m = energy_boost;
			int s = m%60;
			m /= 60;
			sprintf(buff,gettext("Energy Boost: %d:%02d"),m,s);

			const video::SColor color(255,255,255,255);
			video::ITexture *texture = getTexture(driver,"energy.png");

			core::rect<s32> rect(x_off+132,screensize.Y-36,x_off+164,screensize.Y-4);
			draw_image(driver,texture,color,rect,NULL,NULL);

			std::wstring txt = itows(hunger);
			txt += L"%";

			v2u32 dim = font->getDimension(txt.c_str());
			v2s32 sdim(dim.X,dim.Y);
			v2s32 p(x_off+164,screensize.Y-(20+(sdim.Y/2)));
			core::rect<s32> rect2(
				p,
				sdim
			);
			font->draw(narrow_to_wide(buff).c_str(), rect2, video::SColor(255,255,255,255), false, false, NULL);
			x_off += dim.X + 128;
		}
		if (cold_boost > 0.0) {
			int m = cold_boost;
			int s = m%60;
			m /= 60;
			sprintf(buff,gettext("Cold Protection: %d:%02d"),m,s);

			const video::SColor color(255,255,255,255);
			video::ITexture *texture = getTexture(driver,"flame.png");

			core::rect<s32> rect(x_off+132,screensize.Y-36,x_off+164,screensize.Y-4);
			draw_image(driver,texture,color,rect,NULL,NULL);

			std::wstring txt = itows(hunger);
			txt += L"%";

			v2u32 dim = font->getDimension(txt.c_str());
			v2s32 sdim(dim.X,dim.Y);
			v2s32 p(x_off+164,screensize.Y-(20+(sdim.Y/2)));
			core::rect<s32> rect2(
				p,
				sdim
			);
			font->draw(narrow_to_wide(buff).c_str(), rect2, video::SColor(255,255,255,255), false, false, NULL);
			x_off += dim.X + 128;
		}
	}
}
