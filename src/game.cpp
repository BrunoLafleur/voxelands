/************************************************************************
* Minetest-c55
* Copyright (C) 2010-2011 celeron55, Perttu Ahola <celeron55@gmail.com>
*
* game.cpp
* voxelands - 3d voxel world sandbox game
* Copyright (C) Lisa 'darkrose' Milne 2013-2014 <lisa@ltmnet.com>
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

#include "game.h"
#include "common_irrlicht.h"
#include <IGUICheckBox.h>
#include <IGUIEditBox.h>
#include <IGUIButton.h>
#include <IGUIStaticText.h>
#include <IGUIFont.h>
#include "client.h"
#include "server.h"
#include "guiPauseMenu.h"
#include "guiPasswordChange.h"
#include "guiFormSpecMenu.h"
#include "guiTextInputMenu.h"
#include "guiDeathScreen.h"
#include "config.h"
#include "clouds.h"
#include "camera.h"
#include "mapblock.h"
#include "profiler.h"
#include "mainmenumanager.h"
#include "intl.h"
#include "log.h"
#include "path.h"
#include "sound.h"
#ifndef SERVER
#include "main.h"
#endif
#include "hud.h"
#include "sky.h"
#include "selection_mesh.h"

/*
	TODO: Move content-aware stuff to separate file by adding properties
	      and virtual interfaces
*/
#include "content_mapnode.h"
#include "content_nodemeta.h"

/* because windows sucks */
#if _WIN32
#define swprintf _snwprintf
#endif

/*
	Setting this to 1 enables a special camera mode that forces
	the renderers to think that the camera statically points from
	the starting place to a static direction.

	This allows one to move around with the player and see what
	is actually drawn behind solid things and behind the player.
*/
#define FIELD_OF_VIEW_TEST 0


// Chat data
struct ChatLine
{
	ChatLine():
		age(0.0)
	{
	}
	ChatLine(const std::wstring &a_text):
		age(0.0),
		text(a_text)
	{
	}
	ChatLine(const std::wstring &a_text, float a_age):
		age(a_age),
		text(a_text)
	{
	}
	float age;
	std::wstring text;
};

/*
	Inventory stuff
*/

// Inventory actions from the menu are buffered here before sending
Queue<InventoryAction*> inventory_action_queue;
// This is a copy of the inventory that the client's environment has
Inventory local_inventory;

u16 g_selected_item = 0;

/*
	Text input system
*/

class ChatFormIO : public FormIO
{
public:
	ChatFormIO(Client *client)
	{
		m_client = client;
	}

	void gotText(std::map<std::string, std::wstring> fields)
	{
		// Discard empty line
		if (fields["text"] == L"")
			return;

		if (fields["text"][0] == L'/') {
			std::string m = wide_to_narrow(fields["text"]);
			command_exec(NULL,(char*)m.c_str());
		}else{
			// Send to others
			m_client->sendChatMessage(fields["text"]);
			// Show locally
			m_client->addChatMessage(fields["text"]);
		}
	}

	std::string getForm()
	{
		return "";
	}

	Client *m_client;
};

/* Respawn menu callback */

class MainRespawnInitiator: public IRespawnInitiator
{
public:
	MainRespawnInitiator(bool *active, Client *client):
		m_active(active), m_client(client)
	{
		*m_active = true;
	}
	void respawn()
	{
		*m_active = false;
		m_client->sendRespawn();
	}
private:
	bool *m_active;
	Client *m_client;
};

/* Form update callback */

class NodeMetadataFormIO: public FormIO
{
public:
	NodeMetadataFormIO(v3s16 p, Client *client):
		m_client(client),
		m_p(p)
	{
	}

	std::string getForm()
	{
		NodeMetadata *meta = m_client->getEnv().getMap().getNodeMetadata(m_p);
		if (!meta)
			return "";
		return meta->getDrawSpecString(m_client->getLocalPlayer());
	}

	NodeMetadata *getMeta()
	{
		return m_client->getEnv().getMap().getNodeMetadata(m_p);
	}

	void gotText(std::map<std::string, std::wstring> fields)
	{
		m_client->sendNodemetaFields(m_p, "", fields);
	}

	Client *m_client;
	v3s16 m_p;
};

class PlayerInventoryFormIO: public FormIO
{
public:
	PlayerInventoryFormIO(Client *client):
		m_show_appearance(false),
		m_client(client)
	{
	}
	std::string getForm()
	{
		if (m_show_appearance) {
			return
				std::string("size[8,9]"
				"list[current_player;main;0,4.9;8,1;0,8;]"
				"list[current_player;main;0,6;8,3;8,-1;]"
				"button[0.5,3;3,1;show_craft;")+gettext("Show Crafting")+"]"
				"label[1,2;"+gettext("Clothes")+"]"
				"label[4.9,-0.1;"+gettext("Hat/Helmet")+"]"
				"list[current_player;hat;5,0;1,1;]"
				"label[6.6,1.2;"+gettext("Jacket")+"]"
				"list[current_player;jacket;6.3,1.3;1,1;]"
				"label[3.7,0.9;"+gettext("Decorative")+"]"
				"list[current_player;decorative;3.7,1;1,1;]"
				"label[5.4,1.2;"+gettext("Shirt")+"]"
				"list[current_player;shirt;5,1.3;1,1;]"
				"label[4.1,2.2;"+gettext("Belt")+"]"
				"list[current_player;belt;3.7,2.3;1,1;]"
				"label[5.3,2.5;"+gettext("Pants")+"]"
				"list[current_player;pants;5,2.6;1,1;]"
				"label[5.3,3.8;"+gettext("Boots")+"]"
				"list[current_player;boots;5,3.9;1,1;]";
		}
		return
			std::string("size[8,9]"
			"list[current_player;main;0,4.8;8,1;0,8;]"
			"list[current_player;main;0,6;8,3;8,-1;]"
			"label[1,1.7;")+gettext("Drop to Ground")+"]"
			"list[current_player;discard;1.2,2;1,1;]"
			"button[0.5,3.5;3,1;show_appearance;"+gettext("Change Clothing")+"]"
			"list[current_player;craft;3.5,1;3,3;]"
			"list[current_player;craftresult;7,2;1,1;]";
	}

	void gotText(std::map<std::string, std::wstring> fields)
	{
		if (fields["show_appearance"] != L"") {
			m_show_appearance = true;
		}else{
			m_show_appearance = false;
		}
	}

	bool m_show_appearance;
	Client *m_client;
};

/*
	Find what the player is pointing at
*/
void getPointedNode(Client *client, v3f player_position,
		v3f camera_direction, v3f camera_position,
		bool &nodefound, core::line3d<f32> shootline,
		v3s16 &nodepos, v3s16 &neighbourpos, v3s16 camera_offset,
		core::aabbox3d<f32> &nodehilightbox,
		f32 d)
{
	f32 mindistance = BS * 1001;

	v3s16 pos_i = floatToInt(player_position, BS);

	/*infostream<<"pos_i=("<<pos_i.X<<","<<pos_i.Y<<","<<pos_i.Z<<")"
			<<std::endl;*/
//printf("%f,%f,%f - %f,%f,%f\n",shootline.start.X,shootline.start.Y,shootline.start.Z,shootline.end.X,shootline.end.Y,shootline.end.Z);

	s16 a = d;
	s16 ystart = pos_i.Y + 0 - (camera_direction.Y<0 ? a : 1);
	s16 zstart = pos_i.Z - (camera_direction.Z<0 ? a : 1);
	s16 xstart = pos_i.X - (camera_direction.X<0 ? a : 1);
	s16 yend = pos_i.Y + 1 + (camera_direction.Y>0 ? a : 1);
	s16 zend = pos_i.Z + (camera_direction.Z>0 ? a : 1);
	s16 xend = pos_i.X + (camera_direction.X>0 ? a : 1);
	InventoryItem *wield = (InventoryItem*)client->getLocalPlayer()->getWieldItem();
	bool wield_is_hand = (wield == NULL);
	bool wield_is_tool = (wield && wield->getContent()&CONTENT_TOOLITEM_MASK);
	bool wield_is_craft = (wield && wield->getContent()&CONTENT_CRAFTITEM_MASK);
	bool wield_is_material = (!wield_is_hand && !wield_is_tool && !wield_is_craft);

	content_t content = CONTENT_IGNORE;

	for(s16 y = ystart; y <= yend; y++)
	for(s16 z = zstart; z <= zend; z++)
	for(s16 x = xstart; x <= xend; x++)
	{
//printf("%d,%d,%d\n",x,y,z);
		MapNode n;
		try
		{
			n = client->getNode(v3s16(x,y,z));
			if (content_features(n.getContent()).pointable == false) {
				if (content_features(n.getContent()).liquid_type != LIQUID_SOURCE)
					continue;
				if (
					!wield
					|| content_toolitem_features(wield->getContent()).liquids_pointable == false
					|| (
						content_toolitem_features(wield->getContent()).liquids_pointable
						&& content_toolitem_features(wield->getContent()).param_type == CPT_CONTENT
						&& wield->getData() != 0
					)
				)
					continue;
			}else if (content_features(n.getContent()).material_pointable == false && wield_is_material) {
					continue;
			}
		}
		catch(InvalidPositionException &e)
		{
			continue;
		}

		v3s16 np(x,y,z);
		v3f npf = intToFloat(np, BS);

		f32 d = 0.01;

		v3s16 dirs[6] = {
			v3s16(0,0,1), // back
			v3s16(0,1,0), // top
			v3s16(1,0,0), // right
			v3s16(0,0,-1), // front
			v3s16(0,-1,0), // bottom
			v3s16(-1,0,0), // left
		};

		/*
			Meta-objects
		*/
		if(n.getContent() == CONTENT_TORCH) {
			v3s16 dir = unpackDir(n.param2);
			v3f dir_f = v3f(dir.X, dir.Y, dir.Z);
			dir_f *= BS/2 - BS/6 - BS/20;
			v3f cpf = npf + dir_f;
			f32 distance = (cpf - camera_position).getLength();

			core::aabbox3d<f32> box;

			// bottom
			if(dir == v3s16(0,-1,0))
			{
				box = core::aabbox3d<f32>(
					npf - v3f(BS/6, BS/2, BS/6),
					npf + v3f(BS/6, -BS/2+BS/3*2, BS/6)
				);
			}
			// top
			else if(dir == v3s16(0,1,0))
			{
				box = core::aabbox3d<f32>(
					npf - v3f(BS/6, -BS/2+BS/3*2, BS/6),
					npf + v3f(BS/6, BS/2, BS/6)
				);
			}
			// side
			else
			{
				box = core::aabbox3d<f32>(
					cpf - v3f(BS/6, BS/3, BS/6),
					cpf + v3f(BS/6, BS/3, BS/6)
				);
			}

			if(distance < mindistance)
			{
				if(box.intersectsWithLine(shootline))
				{
					nodefound = true;
					nodepos = np;
					content = n.getContent();
					neighbourpos = np;
					mindistance = distance;
					box.MinEdge -= intToFloat(camera_offset,BS);
					box.MaxEdge -= intToFloat(camera_offset,BS);
					nodehilightbox = box;
				}
			}
		}else if(n.getContent() == CONTENT_RAIL) {
			f32 distance = (npf - camera_position).getLength();

			float d = (float)BS/8;
			v3f vertices[4] =
			{
				v3f(BS/2, -BS/2+d, -BS/2),
				v3f(-BS/2, -BS/2, BS/2),
			};

			for(s32 i=0; i<2; i++)
			{
				vertices[i] += npf;
			}

			core::aabbox3d<f32> box;

			box = core::aabbox3d<f32>(vertices[0]);
			box.addInternalPoint(vertices[1]);

			if(distance < mindistance)
			{
				if(box.intersectsWithLine(shootline))
				{
					nodefound = true;
					nodepos = np;
					content = n.getContent();
					neighbourpos = np;
					mindistance = distance;
					box.MinEdge -= intToFloat(camera_offset,BS);
					box.MaxEdge -= intToFloat(camera_offset,BS);
					nodehilightbox = box;
				}
			}
		/*
			Roofs and Node boxes
		*/
		}else if (
			content_features(n).draw_type == CDT_NODEBOX
			|| content_features(n).draw_type == CDT_NODEBOX_META
			|| content_features(n).draw_type == CDT_WIRELIKE
			|| content_features(n).draw_type == CDT_3DWIRELIKE
			|| content_features(n).draw_type == CDT_FENCELIKE
			|| content_features(n).draw_type == CDT_WALLLIKE
			|| content_features(n).draw_type == CDT_STAIRLIKE
			|| content_features(n).draw_type == CDT_SLABLIKE
			|| content_features(n).draw_type == CDT_FLAGLIKE
		) {
			f32 distance = (npf - camera_position).getLength();

			if (distance < mindistance) {
				aabb3f box;
				aabb3f nhbox(0.5*BS,0.5*BS,0.5*BS,-0.5*BS,-0.5*BS,-0.5*BS);
				bool hit = false;
				std::vector<NodeBox> boxes = content_features(n).getNodeBoxes(n);
				for (std::vector<NodeBox>::iterator b = boxes.begin(); b != boxes.end(); b++) {
					box = b->m_box;

					if (nhbox.MinEdge.X > box.MinEdge.X)
						nhbox.MinEdge.X = box.MinEdge.X;
					if (nhbox.MinEdge.Y > box.MinEdge.Y)
						nhbox.MinEdge.Y = box.MinEdge.Y;
					if (nhbox.MinEdge.Z > box.MinEdge.Z)
						nhbox.MinEdge.Z = box.MinEdge.Z;
					if (nhbox.MaxEdge.X < box.MaxEdge.X)
						nhbox.MaxEdge.X = box.MaxEdge.X;
					if (nhbox.MaxEdge.Y < box.MaxEdge.Y)
						nhbox.MaxEdge.Y = box.MaxEdge.Y;
					if (nhbox.MaxEdge.Z < box.MaxEdge.Z)
						nhbox.MaxEdge.Z = box.MaxEdge.Z;

					box.MinEdge += npf;
					box.MaxEdge += npf;

					if (box.intersectsWithLine(shootline)) {
						for(u16 i=0; i<6; i++) {
							v3f dir_f = v3f(dirs[i].X,
									dirs[i].Y, dirs[i].Z);
							v3f centerpoint = npf + dir_f * BS/2;
							f32 distance =
									(centerpoint - camera_position).getLength();

							if(distance < mindistance)
							{
								core::CMatrix4<f32> m;
								m.buildRotateFromTo(v3f(0,0,1), dir_f);

								// This is the back face
								v3f corners[2] = {
									v3f(BS/2, BS/2, BS/2),
									v3f(-BS/2, -BS/2, BS/2+d)
								};

								for(u16 j=0; j<2; j++)
								{
									m.rotateVect(corners[j]);
									corners[j] += npf;
								}

								core::aabbox3d<f32> facebox(corners[0]);
								facebox.addInternalPoint(corners[1]);

								if(facebox.intersectsWithLine(shootline))
								{
									nodefound = true;
									nodepos = np;
									content = n.getContent();
									neighbourpos = np + dirs[i];
									mindistance = distance;

									hit = true;
								}
							} // if distance < mindistance
						} // for dirs
					}
				}
				if (hit) {
					nhbox.MinEdge -= 0.002;
					nhbox.MaxEdge += 0.002;
					v3f nodepos_f = intToFloat(nodepos-camera_offset, BS);
					nhbox.MinEdge += nodepos_f;
					nhbox.MaxEdge += nodepos_f;
					nodehilightbox = nhbox;
				}
				boxes.clear();
			}
		/*
			Regular blocks
		*/
		}else{
			for(u16 i=0; i<6; i++)
			{
				v3f dir_f = v3f(dirs[i].X,
						dirs[i].Y, dirs[i].Z);
				v3f centerpoint = npf + dir_f * BS/2;
				f32 distance =
						(centerpoint - camera_position).getLength();
//printf("%f %f - %d,%d,%d\n",distance, mindistance,x,y,z);

				if(distance < mindistance)
				{
					core::CMatrix4<f32> m;
					m.buildRotateFromTo(v3f(0,0,1), dir_f);

					// This is the back face
					v3f corners[2] = {
						v3f(BS/2, BS/2, BS/2),
						v3f(-BS/2, -BS/2, BS/2+d)
					};

					for(u16 j=0; j<2; j++)
					{
						m.rotateVect(corners[j]);
						corners[j] += npf;
					}

					core::aabbox3d<f32> facebox(corners[0]);
					facebox.addInternalPoint(corners[1]);

					if(facebox.intersectsWithLine(shootline))
					{
						nodefound = true;
						nodepos = np;
						content = n.getContent();
						neighbourpos = np + dirs[i];
						mindistance = distance;

						//nodehilightbox = facebox;

						const float d = 0.502;
						core::aabbox3d<f32> nodebox
								(-BS*d, -BS*d, -BS*d, BS*d, BS*d, BS*d);
						v3f nodepos_f = intToFloat(nodepos-camera_offset, BS);
						nodebox.MinEdge += nodepos_f;
						nodebox.MaxEdge += nodepos_f;
						nodehilightbox = nodebox;
					}
				} // if distance < mindistance
			} // for dirs
		} // regular block
	} // for coords
	if (nodefound) {
		client->setPointedNode(nodepos);
		client->setPointedContent(content);
	}else{
		client->setPointedContent(CONTENT_IGNORE);
	}
}

/*
	Draws a screen with logo and text on it.
	Text will be removed when the screen is drawn the next time.
*/
void drawLoadingScreen(irr::IrrlichtDevice* device, const std::wstring msg)
{
	char buff[1024];
	if (device == NULL)
		return;
	device->run();
	video::IVideoDriver* driver = device->getVideoDriver();
	if (driver == NULL)
		return;
	core::dimension2d<u32> screensize = driver->getScreenSize();
	s32 x = (screensize.Width/2);
	s32 y = (screensize.Height/2);

	driver->beginScene(true, true, video::SColor(255,0,0,0));


	if (path_get((char*)"texture",(char*)"loadingbg.png",1,buff,1024)) {
		video::ITexture *logotexture = driver->getTexture(buff);
		if (logotexture) {
			core::rect<s32> rect(x-512,y-200,x+512,y+200);
			driver->draw2DImage(logotexture, rect,
				core::rect<s32>(core::position2d<s32>(0,0),
				core::dimension2di(logotexture->getSize())),
				NULL, NULL, true);
		}
	}
	if (guienv) {
		gui::IGUIStaticText *info = NULL;
		gui::IGUIStaticText *version = NULL;
		std::wstring m;
		if (msg != L"") {
			m = msg;
		}else{
			m = narrow_to_wide(gettext("Loading"));
		}
		{
			core::dimension2d<u32> textsize = guienv->getSkin()->getFont()->getDimension(m.c_str());
			core::rect<s32> rect(x-(textsize.Width/2), y+220, x+textsize.Width, y+220+textsize.Height);
			info = guienv->addStaticText(m.c_str(),rect);
		}
		{
			m = narrow_to_wide(VERSION_STRING);
			core::dimension2d<u32> textsize = guienv->getSkin()->getFont()->getDimension(m.c_str());
			core::rect<s32> rect((x-412)-(textsize.Width/2), y+10, (x-412)+(textsize.Width/2), y+10+textsize.Height);
			version = guienv->addStaticText(m.c_str(),rect);
		}

		guienv->drawAll();
		info->remove();
		version->remove();
	}

	driver->endScene();
}

/* Profiler display */

void update_profiler_gui(gui::IGUIStaticText *guitext_profiler,
		gui::IGUIFont *font, u32 text_height,
		u32 show_profiler, u32 show_profiler_max)
{
	if (show_profiler == 0) {
		guitext_profiler->setVisible(false);
	}else{

		std::ostringstream os(std::ios_base::binary);
		g_profiler->printPage(os, show_profiler, show_profiler_max);
		std::wstring text = narrow_to_wide(os.str());
		guitext_profiler->setText(text.c_str());
		guitext_profiler->setVisible(true);

		s32 w = font->getDimension(text.c_str()).Width;
		if(w < 400)
			w = 400;
		core::rect<s32> rect(6, 4+(text_height+5)*2, 12+w,
				8+(text_height+5)*2 +
				font->getDimension(text.c_str()).Height);
		guitext_profiler->setRelativePosition(rect);
		guitext_profiler->setVisible(true);
	}
}

void chatline_add(ref_t **chat_lines, std::wstring txt, float time)
{
	ref_t *ref = (ref_t*)malloc(sizeof(ref_t));
	if (!ref)
		return;

	ref->ref = new ChatLine(txt,time);
	*chat_lines = (ref_t*)list_push(chat_lines,ref);
}

void the_game(
	bool &kill,
	InputHandler *input,
	IrrlichtDevice *device,
	gui::IGUIFont* font,
	std::string password,
	std::wstring &error_message
)
{
	u32 text_height;
	v2u32 screensize(0,0);
	uint16_t port;
	bool could_connect = false;
	f32 camera_yaw = 0; // "right/left"
	f32 camera_pitch = 0; // "up/down"
	Clouds *clouds = NULL;
	Sky *sky = NULL;
	gui::IGUIStaticText *guitext;
	gui::IGUIStaticText *guitext2;
	gui::IGUIStaticText *guitext_info;
	gui::IGUIStaticText *guitext_chat;
	gui::IGUIStaticText *guitext_profiler;
	ref_t *chat_lines = NULL;
	u32 drawtime = 0;
	core::list<float> frametime_log;
	float action_delay_counter = 0.0;
	float dig_time = 0.0;
	v3s16 nodepos_old(-32768,-32768,-32768);
	float damage_flash_timer = 0;
	bool invert_mouse = false;
	bool respawn_menu_active = false;
	bool show_hud = true;
	bool show_chat = true;
	bool force_fog_off = false;
	bool disable_camera_update = false;
	bool show_debug = false;
	bool show_debug_frametime = false;
	u32 show_profiler = 0;
	u32 show_profiler_max = 3;  // Number of pages
	float fps_max = 60;
	float profiler_print_interval = 0;
	bool free_move = false;
	f32 mouse_sensitivity = 1.0;
	bool highlight_selected_node = true;
	bool enable_particles = true;
	bool enable_fog = true;
	bool old_hotbar = false;
	bool show_index = false;
	bool has_selected_node = false;
	v3s16 selected_node_pos = v3s16(0,0,0);
	u32 selected_node_crack = 0;
	bool first_loop_after_window_activation = true;
	u32 lasttime = 0;
	v3s16 lastpointed(0,0,0);
	float recent_turn_speed = 0.0;
	float time_of_day = 0;
	float time_of_day_smooth = 0;
	float busytime;
	u32 busytime_u32;
	f32 dtime;

	video::IVideoDriver* driver = device->getVideoDriver();
	scene::ISceneManager* smgr = device->getSceneManager();

	// Calculate text height using the font
	text_height = font->getDimension(L"Random test string").Height;

	screensize = driver->getScreenSize();

	/*
		Draw "Loading" screen
	*/
	drawLoadingScreen(device,narrow_to_wide(gettext("Loading...")));

	/*
		Create server.
		SharedPtr will delete it when it goes out of scope.
	*/
	SharedPtr<Server> server;
	{
		const char* v = config_get("world.server.address");
		if (!v || !v[0]) {
			//draw_load_screen(L"Creating server...", driver, font);
			drawLoadingScreen(device,narrow_to_wide(gettext("Creating server...")));
			infostream<<"Creating server"<<std::endl;
			server = new Server();
			server->start();
		}
	}

	/*
		Create client
	*/

	drawLoadingScreen(device,narrow_to_wide(gettext("Creating client...")));
	infostream<<"Creating client"<<std::endl;
	MapDrawControl draw_control;
	Client client(device, password, draw_control);

	bridge_register_client(&client);

	drawLoadingScreen(device,narrow_to_wide(gettext("Resolving address...")));
	port = config_get_int("world.server.port");
	if (!port)
		port = 30000;
	Address connect_address(0,0,0,0, port);
	try{
		const char* v = config_get("world.server.address");
		if (!v || !v[0]) {
			connect_address.setAddress(127,0,0,1);
		}else{
			connect_address.Resolve(v);
		}
	}
	catch(ResolveError &e)
	{
		errorstream<<"Couldn't resolve address"<<std::endl;
		error_message = narrow_to_wide(gettext("Couldn't resolve address"));
		bridge_register_client(NULL);
		return;
	}

	/*
		Attempt to connect to the server
	*/
	client.connect(connect_address);;

	try{
		float time_counter = 0.0;
		for (;;) {
			if (client.connectedAndInitialized()) {
				could_connect = true;
				break;
			}
			if (client.accessDenied()) {
				break;
			}
			// Wait for 10 seconds
			if (time_counter >= 10.0) {
				break;
			}

			char buff[512];
			int tot = (10.0 - time_counter + 1.0);
			snprintf(
				buff,
				512,
				ngettext(
					"Connecting to server... (timeout in %d second)",
					"Connecting to server... (timeout in %d seconds)",
					tot
				),
				tot
			);
			//draw_load_screen(ss.str(), driver, font);
			drawLoadingScreen(device, narrow_to_wide(buff).c_str());
			// Update client and server
			client.step(0.1);

			if (server != NULL) {
				if (!server->step(0.1)) {
					could_connect = false;
					break;
				}
			}

			// Delay a bit
			sleep_ms(100);
			time_counter += 0.1;
		}
	}
	catch(con::PeerNotFoundException &e)
	{}

	if (could_connect == false) {
		if (client.accessDenied()) {
			char buff[512];
			snprintf(buff,512,gettext("Access denied. Reason: %s"),wide_to_narrow(client.accessDeniedReason()).c_str());
			error_message = narrow_to_wide(buff);
			errorstream<<buff<<std::endl;
		}else if (server != NULL) {
			error_message = narrow_to_wide(gettext("Unable to Connect (port already in use?)."));
			errorstream<<"Timed out."<<std::endl;
		}else{
			error_message = narrow_to_wide(gettext("Connection timed out."));
			errorstream<<"Timed out."<<std::endl;
		}
		bridge_register_client(NULL);
		return;
	}

	/*
		Create the camera node
	*/
	Camera camera(smgr, draw_control, &client);
	if (!camera.successfullyCreated(error_message)) {
		bridge_register_client(NULL);
		return;
	}

	/* Clouds */
	if (config_get_bool("client.graphics.clouds"))
		clouds = new Clouds(smgr->getRootSceneNode(), smgr, -1, time(0));

	/* Skybox */
	sky = new Sky(smgr->getRootSceneNode(), smgr, -1);

	/*
		Move into game
	*/

	/*
		Add some gui stuff
	*/

	// First line of debug text
	guitext = guienv->addStaticText(
			L"Voxelands",
			core::rect<s32>(5, 5, 795, 5+text_height),
			false, false);
	// Second line of debug text
	guitext2 = guienv->addStaticText(
			L"",
			core::rect<s32>(5, 3+(text_height)*1, 795, (5+text_height)*2),
			false, false);
	// At the middle of the screen
	// Object infos are shown in this
	guitext_info = guienv->addStaticText(
			L"",
			core::rect<s32>(0,0,500,text_height+5) + v2s32(100,200),
			false, false);

	// Chat text
	guitext_chat = guienv->addStaticText(
			L"",
			core::rect<s32>(0,0,0,0),
			//false, false); // Disable word wrap as of now
			false, true);

	// Profiler text (size is updated when text is updated)
	guitext_profiler = guienv->addStaticText(
			L"<Profiler>",
			core::rect<s32>(0,0,0,0),
			false, false);
	guitext_profiler->setBackgroundColor(video::SColor(80,0,0,0));
	guitext_profiler->setVisible(false);

	/*
		Some statistics are collected in these
	*/
	invert_mouse = config_get_bool("client.ui.mouse.invert");
	show_debug = config_get_bool("debug.show");
	fps_max = config_get_float("client.graphics.fps.max");
	profiler_print_interval = config_get_float("debug.profiler.interval");
	mouse_sensitivity = config_get_float("client.ui.mouse.sensitivity");
	{
		const char* v = config_get("client.graphics.selection");
		if (v && !strcmp(v,"outline"))
			highlight_selected_node = false;
	}
	enable_particles = config_get_bool("client.graphics.particles");
	enable_fog = config_get_bool("client.graphics.light.fog");
	old_hotbar = config_get_bool("client.ui.hud.old");
	show_index = config_get_bool("client.ui.hud.wieldindex");

	/*
		Main loop
	*/

	// TODO: Convert the static interval timers to these
	// Interval limiter for profiler
	IntervalLimiter m_profiler_interval;

	// Time is in milliseconds
	// NOTE: getRealTime() causes strange problems in wine (imprecision?)
	// NOTE: So we have to use getTime() and call run()s between them
	lasttime = device->getTimer()->getTime();

	while (device->run() && kill == false) {
		//std::cerr<<"frame"<<std::endl;

		if (client.accessDenied()) {
			error_message = narrow_to_wide(gettext("Access denied. Reason: "))
					+client.accessDeniedReason();
			errorstream<<wide_to_narrow(error_message)<<std::endl;
			break;
		}

		if (g_gamecallback->disconnect_requested) {
			g_gamecallback->disconnect_requested = false;
			break;
		}

		if (g_gamecallback->changepassword_requested) {
			(new GUIPasswordChange(guienv, guiroot, -1,
				&g_menumgr, &client))->drop();
			g_gamecallback->changepassword_requested = false;
			client.setFormState(true);
		}

		/*
			Process TextureSource's queue
		*/
		((TextureSource*)g_texturesource)->processQueue();

		/*
			Random calculations
		*/
		screensize = driver->getScreenSize();
		v2s32 displaycenter(screensize.X/2,screensize.Y/2);

		// Hilight boxes collected during the loop and displayed
		core::list< core::aabbox3d<f32> > hilightboxes;

		// Info text
		std::wstring infotext;

		// Time of frame without fps limit
		{
			// not using getRealTime is necessary for wine
			u32 time = device->getTimer()->getTime();
			if (time > lasttime) {
				busytime_u32 = time - lasttime;
			}else{
				busytime_u32 = 0;
			}
			busytime = busytime_u32 / 1000.0;
		}

		// Necessary for device->getTimer()->getTime()
		device->run();

		/*
			FPS limiter
		*/

		{
			u32 frametime_min = 1000./fps_max;

			if (busytime_u32 < frametime_min) {
				u32 sleeptime = frametime_min - busytime_u32;
				device->sleep(sleeptime);
			}
		}

		// Necessary for device->getTimer()->getTime()
		device->run();

		/*
			Time difference calculation
		*/
		u32 time = device->getTimer()->getTime();
		if (time > lasttime) {
			dtime = (time - lasttime) / 1000.0;
		}else{
			dtime = 0;
		}
		lasttime = time;

		/* Run timers */

		g_profiler->add("Elapsed time", dtime);
		g_profiler->avg("FPS", 1./dtime);

		/*
			Log frametime for visualization
		*/
		frametime_log.push_back(dtime);
		if (frametime_log.size() > 100) {
			core::list<float>::Iterator i = frametime_log.begin();
			frametime_log.erase(i);
		}

		/*
			Time average and jitter calculation
		*/

		static f32 dtime_avg1 = 0.0;
		dtime_avg1 = dtime_avg1 * 0.96 + dtime * 0.04;
		f32 dtime_jitter1 = dtime - dtime_avg1;

		static f32 dtime_jitter1_max_sample = 0.0;
		static f32 dtime_jitter1_max_fraction = 0.0;
		{
			static f32 jitter1_max = 0.0;
			static f32 counter = 0.0;
			if (dtime_jitter1 > jitter1_max)
				jitter1_max = dtime_jitter1;
			counter += dtime;
			if (counter > 0.0) {
				counter -= 3.0;
				dtime_jitter1_max_sample = jitter1_max;
				dtime_jitter1_max_fraction = dtime_jitter1_max_sample / (dtime_avg1+0.001);
				jitter1_max = 0.0;
			}
		}

		/*
			Busytime average and jitter calculation
		*/

		static f32 busytime_avg1 = 0.0;
		busytime_avg1 = busytime_avg1 * 0.98 + busytime * 0.02;
		f32 busytime_jitter1 = busytime - busytime_avg1;

		{
			static f32 jitter1_max = 0.0;
			static f32 jitter1_min = 0.0;
			static f32 counter = 0.0;
			if (busytime_jitter1 > jitter1_max)
				jitter1_max = busytime_jitter1;
			if (busytime_jitter1 < jitter1_min)
				jitter1_min = busytime_jitter1;
			counter += dtime;
			if (counter > 0.0) {
				counter -= 3.0;
				jitter1_max = 0.0;
				jitter1_min = 0.0;
			}
		}

		/*
			Debug info for client
		*/
		{
			static float counter = 0.0;
			counter -= dtime;
			if (counter < 0) {
				counter = 30.0;
				client.printDebugInfo(infostream);
			}
		}

		/*
			Profiler
		*/
		bool print_to_log = true;
		if (profiler_print_interval == 0) {
			print_to_log = false;
			profiler_print_interval = 5;
		}
		if (m_profiler_interval.step(dtime, profiler_print_interval)) {
			if (print_to_log) {
				infostream<<"Profiler:"<<std::endl;
				g_profiler->print(infostream);
			}

			update_profiler_gui(guitext_profiler, font, text_height,
					show_profiler, show_profiler_max);

			g_profiler->clear();
		}

		/*
			Direct handling of user input
		*/

		// Reset input if window not active or some menu is active
		if (device->isWindowActive() == false || noMenuActive() == false)
			input->clear();

		// Input handler step() (used by the random input generator)
		input->step(dtime);

		/*
			Launch menus according to keys
		*/
		if (input->wasKeyDown(getKeySetting(VLKC_INVENTORY))) {
			infostream<<"the_game: Launching inventory"<<std::endl;

			GUIFormSpecMenu* const menu = new GUIFormSpecMenu(guienv, guiroot, -1, &g_menumgr, &client);

			InventoryLocation inventoryloc;
			inventoryloc.setCurrentPlayer();

			PlayerInventoryFormIO *fio = new PlayerInventoryFormIO(&client);
			assert(fio);
			menu->setFormSpec(fio->getForm(), inventoryloc);
			menu->setFormIO(fio);
			menu->drop();
			client.setFormState(true);
		}else if (input->wasKeyDown(EscapeKey)) {
			infostream<<"the_game: Launching pause menu"<<std::endl;
			// It will delete itself by itself
			(new GUIPauseMenu(guienv, guiroot, -1, g_gamecallback, &g_menumgr))->drop();

			// Move mouse cursor on top of the disconnect button
			input->setMousePos(displaycenter.X, displaycenter.Y+25);
			client.setFormState(true);
		}else if (input->wasKeyDown(getKeySetting(VLKC_CHAT))) {
			FormIO *fio = new ChatFormIO(&client);

			(new GUITextInputMenu(guienv, guiroot, -1, &g_menumgr, fio, L""))->drop();
		}else if (input->wasKeyDown(getKeySetting(VLKC_COMMAND))) {
			FormIO *fio = new ChatFormIO(&client);

			(new GUITextInputMenu(guienv, guiroot, -1, &g_menumgr, fio, L"/"))->drop();
		}else if(input->wasKeyDown(getKeySetting(VLKC_FREEMOVE))) {
			if (free_move) {
				free_move = false;
				chatline_add(&chat_lines,narrow_to_wide(gettext("free_move disabled")),-103.00);
			}else{
				free_move = true;
				chatline_add(&chat_lines,narrow_to_wide(gettext("free_move enabled")),-103.00);
			}
		}else if(input->wasKeyDown(getKeySetting(VLKC_SCREENSHOT))) {
			irr::video::IImage* const image = driver->createScreenShot();
			if (image) {
				char fn[256];
				char path[1024];
				if (snprintf(fn,256,"screenshot_%u.png", device->getTimer()->getRealTime()) >= 256) {
							infostream << "Failed to save screenshot"<<std::endl;
				}else{
					if (path_get((char*)"screenshot",fn,0,path,1024)) {
						if (driver->writeImageToFile(image, io::path(path))) {
							char buff[512];
							snprintf(buff, 512, gettext("Saved screenshot to '%s'"), path);
							infostream << "Saved screenshot to '" << fn << "'" << std::endl;
							chatline_add(&chat_lines,narrow_to_wide(buff),-103.00);
						}else{
							infostream << "Failed to save screenshot '" << fn << "'"<<std::endl;
						}
						image->drop();
					}
				}
			}
		}else if (input->wasKeyDown(getKeySetting(VLKC_TOGGLE_HUD))) {
			show_hud = !show_hud;
			if (show_hud) {
				chatline_add(&chat_lines,narrow_to_wide(gettext("HUD shown")),-103.00);
			}else{
				chatline_add(&chat_lines,narrow_to_wide(gettext("HUD hidden")),-103.00);
			}
		}else if (input->wasKeyDown(getKeySetting(VLKC_TOGGLE_CHAT))) {
			show_chat = !show_chat;
			if (show_chat) {
				chatline_add(&chat_lines,narrow_to_wide(gettext("Chat shown")),-103.00);
			}else{
				chatline_add(&chat_lines,narrow_to_wide(gettext("Chat hidden")),-103.00);
			}
		}else if (input->wasKeyDown(getKeySetting(VLKC_TOGGLE_FOG))) {
			force_fog_off = !force_fog_off;
			if (force_fog_off) {
				chatline_add(&chat_lines,narrow_to_wide(gettext("Fog disabled")),-103.00);
			}else{
				chatline_add(&chat_lines,narrow_to_wide(gettext("Fog enabled")),-103.00);
			}
		}else if (input->wasKeyDown(getKeySetting(VLKC_TOGGLE_CAMERA))) {
			disable_camera_update = !disable_camera_update;
			if (disable_camera_update) {
				chatline_add(&chat_lines,narrow_to_wide(gettext("Camera update disabled")),-103.00);
			}else{
				chatline_add(&chat_lines,narrow_to_wide(gettext("Camera update enabled")),-103.00);
			}
		}else if (input->wasKeyDown(getKeySetting(VLKC_TOGGLE_DEBUG))) {
			// Initial / 3x toggle: Chat only
			// 1x toggle: Debug text with chat
			// 2x toggle: Debug text with frametime
			if (!show_debug) {
				show_debug = true;
				show_debug_frametime = false;
				chatline_add(&chat_lines,narrow_to_wide(gettext("Debug info shown")),-103.00);
			}else if (show_debug_frametime) {
				show_debug = false;
				show_debug_frametime = false;
				chatline_add(&chat_lines,narrow_to_wide(gettext("Debug info and frametime graph hidden")),-103.00);
			}else{
				show_debug_frametime = true;
				chatline_add(&chat_lines,narrow_to_wide(gettext("Frametime graph shown")),-103.00);
			}
		}else if (input->wasKeyDown(getKeySetting(VLKC_TOGGLE_PROFILER))) {
			show_profiler = (show_profiler + 1) % (show_profiler_max + 1);

			// FIXME: This updates the profiler with incomplete values
			update_profiler_gui(guitext_profiler, font, text_height,
					show_profiler, show_profiler_max);

			if (show_profiler != 0) {
				char buff[512];
				snprintf(buff,512,gettext("Profiler shown (page %d of %d)"),show_profiler,show_profiler_max);
				chatline_add(&chat_lines,narrow_to_wide(buff),-103.00);
			}else{
				chatline_add(&chat_lines,narrow_to_wide(gettext("Profiler hidden")),-103.00);
			}
		}else if (input->wasKeyDown(getKeySetting(VLKC_RANGE_PLUS))) {
			char buff[512];
			int range = config_get_int("client.graphics.range.min");
			range += 10;
			config_set_int("client.graphics.range.min",range);
			snprintf(buff,512,gettext("Minimum viewing range changed to %d"),range);
			chatline_add(&chat_lines,narrow_to_wide(buff),-103.00);
		}else if (input->wasKeyDown(getKeySetting(VLKC_RANGE_MINUS))) {
			char buff[512];
			int range = config_get_int("client.graphics.range.min");
			range -= 10;
			if (range < 10)
				range = 10;
			config_set_int("client.graphics.range.min",range);
			snprintf(buff,512,gettext("Minimum viewing range changed to %d"),range);
			chatline_add(&chat_lines,narrow_to_wide(buff),-103.00);
		}

		// Item selection with mouse wheel
		{
			s32 wheel = input->getMouseWheel();
			u16 max_item = MYMIN(PLAYER_INVENTORY_SIZE-1,7);

			std::string wield_sound = "wield";

			if (wheel < 0) {
				if (g_selected_item < max_item) {
					g_selected_item++;
				}else{
					g_selected_item = 0;
				}
				client.playSound(wield_sound,false);
			}else if (wheel > 0) {
				if (g_selected_item > 0) {
					g_selected_item--;
				}else{
					g_selected_item = max_item;
				}
				client.playSound(wield_sound,false);
			}
		}

		// Item selection
		for (u16 i=0; i<10; i++) {
			const KeyPress *kp = NumberKey + (i + 1) % 10;
			if (input->wasKeyDown(*kp)) {
				if (i < PLAYER_INVENTORY_SIZE && i < 8) {
					g_selected_item = i;

					infostream<<"Selected item: "<<g_selected_item<<std::endl;
					std::string wield_sound = "wield";
					client.playSound(wield_sound,false);
				}
			}
		}
		if (input->wasKeyDown(getKeySetting(VLKC_SELECT_PREV))) {
			u16 max_item = MYMIN(PLAYER_INVENTORY_SIZE-1, 7);
			if (g_selected_item > 0) {
				g_selected_item--;
			}else{
				g_selected_item = max_item;
			}
			std::string wield_sound = "wield";
			client.playSound(wield_sound,false);
		}
		if (input->wasKeyDown(getKeySetting(VLKC_SELECT_NEXT))) {
			u16 max_item = MYMIN(PLAYER_INVENTORY_SIZE-1, 7);
			if (g_selected_item < max_item) {
				g_selected_item++;
			}else{
				g_selected_item = 0;
			}
			std::string wield_sound = "wield";
			client.playSound(wield_sound,false);
		}

		// Viewing range selection
		if (input->wasKeyDown(getKeySetting(VLKC_RANGE))) {
			draw_control.range_all = !draw_control.range_all;
			if (draw_control.range_all) {
				infostream<<"Enabled full viewing range"<<std::endl;
				chatline_add(&chat_lines,narrow_to_wide(gettext("Enabled full viewing range")),-103.00);
			}else{
				infostream<<"Disabled full viewing range"<<std::endl;
				chatline_add(&chat_lines,narrow_to_wide(gettext("Disabled full viewing range")),-103.00);
			}
		}

		// Print debug stacks
		if (input->wasKeyDown(getKeySetting(VLKC_PRINT_DEBUG))) {
			dstream<<"-----------------------------------------"
					<<std::endl;
			dstream<<DTIME<<"Printing debug stacks:"<<std::endl;
			dstream<<"-----------------------------------------"
					<<std::endl;
			debug_stacks_print();
		}

		/*
			Mouse and camera control
			NOTE: Do this before client.setPlayerControl() to not cause a camera lag of one frame
		*/

		float turn_amount = 0.0;
		if ((device->isWindowActive() && noMenuActive())) {
			if (device->getCursorControl()->isVisible())
				device->getCursorControl()->setVisible(false);

			if (first_loop_after_window_activation) {
				//infostream<<"window active, first loop"<<std::endl;
				first_loop_after_window_activation = false;
			}else{
				s32 dx = input->getMousePos().X - displaycenter.X;
				s32 dy = input->getMousePos().Y - displaycenter.Y;
				if (invert_mouse)
					dy = -dy;

				f32 d = rangelim(mouse_sensitivity, 0.01, 100.0);

				camera_yaw -= dx*d;
				camera_pitch += dy*d;
				if (camera_pitch < -89.5)
					camera_pitch = -89.5;
				if (camera_pitch > 89.5)
					camera_pitch = 89.5;

				turn_amount = v2f(dx, dy).getLength() * d;
			}
			input->setMousePos(displaycenter.X, displaycenter.Y);
		}else{
			// Mac OSX gets upset if this is set every frame
			if (device->getCursorControl()->isVisible() == false)
				device->getCursorControl()->setVisible(true);

			//infostream<<"window inactive"<<std::endl;
			first_loop_after_window_activation = true;
		}

		recent_turn_speed = recent_turn_speed * 0.9 + turn_amount * 0.1;

		/*
			Player speed control
		*/

		if (!noMenuActive() || !device->isWindowActive()) {
			PlayerControl control(
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				free_move,
				false,
				camera_pitch,
				camera_yaw
			);
			client.setPlayerControl(control);
		}else{
			/*
			bool a_fwd,
			bool a_back,
			bool a_left,
			bool a_right,
			bool a_jump,
			bool a_sneak,
			bool a_up,
			bool a_down,
			bool a_fast,
			bool a_free,
			float a_pitch,
			float a_yaw
			*/
			PlayerControl control(
				input->isKeyDown(getKeySetting(VLKC_FORWARD)),
				input->isKeyDown(getKeySetting(VLKC_BACKWARD)),
				input->isKeyDown(getKeySetting(VLKC_LEFT)),
				input->isKeyDown(getKeySetting(VLKC_RIGHT)),
				input->isKeyDown(getKeySetting(VLKC_JUMP)),
				input->isKeyDown(getKeySetting(VLKC_SNEAK)),
				input->isKeyDown(getKeySetting(VLKC_UP)),
				input->isKeyDown(getKeySetting(VLKC_DOWN)),
				input->isKeyDown(getKeySetting(VLKC_RUN)),
				free_move,
				input->getLeftState(),
				camera_pitch,
				camera_yaw
			);
			client.setPlayerControl(control);
		}

		/*
			Run server
		*/

		if (server != NULL)
			server->step(dtime);

		/*
			Process environment
		*/

		{
			client.step(dtime);
		}

		{
			// Read client events
			while (1) {
				ClientEvent event = client.getClientEvent();
				if (event.type == CE_NONE) {
					break;
				}else if (event.type == CE_PLAYER_DAMAGE) {
					damage_flash_timer = 0.05;
					if (event.player_damage.amount >= 2) {
						damage_flash_timer += 0.05 * event.player_damage.amount;
					}
#if USE_AUDIO == 1
					{
						const char* v;
						std::string ch = std::string(PLAYER_DEFAULT_CHARDEF);
						v = config_get("client.character");
						if (v)
							ch = v;
						Strfnd f(ch);
						std::string gender = f.next(":");
						std::string snd("player-hurt-");
						snd += gender;
						sound_play_effect((char*)snd.c_str(),1.0,0,NULL);
					}
#endif
				}else if (event.type == CE_PLAYER_FORCE_MOVE) {
					camera_yaw = event.player_force_move.yaw;
					camera_pitch = event.player_force_move.pitch;
				}else if (event.type == CE_DEATHSCREEN) {
					if (respawn_menu_active)
						continue;

					MainRespawnInitiator* const respawner =
							new MainRespawnInitiator(
									&respawn_menu_active, &client);
					GUIDeathScreen* const menu =
							new GUIDeathScreen(guienv, guiroot, -1,
								&g_menumgr, respawner);
					menu->drop();

					/* Handle visualization */
					damage_flash_timer = 0;
				}
			}
		}

		//v3s16 old_camera_offset = camera.getOffset();

		LocalPlayer* player = client.getLocalPlayer();
		camera.update(player, busytime, screensize);
		camera.step(dtime);

		v3f player_position = player->getPosition();
		v3f camera_position = camera.getPosition();
		v3f camera_direction = camera.getDirection();
		v3f camera_up = camera.getCameraNode()->getUpVector();
		f32 camera_fov = camera.getFovMax();
		v3s16 camera_offset = camera.getOffset();

#if USE_AUDIO == 1
		{
			v3_t pos = {camera_position.X,camera_position.Y,camera_position.Z};
			v3_t at = {camera_direction.X,camera_direction.Y,camera_direction.Z};
			v3_t up = {camera_up.X,camera_up.Y,camera_up.Z};
			sound_step(dtime,&pos,&at,&up);
		}
#endif

		if (!disable_camera_update) {
			client.updateCamera(camera_position, camera_direction, camera_fov, camera_offset);
			client.updateCameraOffset(camera_offset);
			client.getEnv().updateObjectsCameraOffset(camera_offset);
			update_particles_camera_offset(camera_offset);
			if (clouds)
				clouds->updateCameraOffset(camera_offset);
		}

		bool left_punch = false;
		bool left_punch_muted = false;

		InventoryItem *wield = (InventoryItem*)client.getLocalPlayer()->getWieldItem();
		InventoryList *ilist;

		if (action_delay_counter > 0.0) {
			action_delay_counter -= dtime;
		}else if (
			wield
			&& (
				content_craftitem_features(wield->getContent())->thrown_item != CONTENT_IGNORE
				|| (
					content_toolitem_features(wield->getContent()).thrown_item != CONTENT_IGNORE
					&& (ilist = client.getLocalPlayer()->inventory.getList("main")) != NULL
					&& ilist->findItem(content_toolitem_features(wield->getContent()).thrown_item) != NULL
				)
			) && input->getLeftClicked()
		) {
			client.throwItem(camera_direction,g_selected_item);
		}else{
			/*
				Calculate what block is the crosshair pointing to
			*/

			f32 d = 4; // max. distance
			core::line3d<f32> shootline(camera_position, camera_position + camera_direction * BS * (d+1));

			ClientActiveObject *selected_active_object = client.getSelectedActiveObject(d*BS, camera_position, shootline);

			if (selected_active_object != NULL) {
				has_selected_node = false;
				client.setPointedContent(selected_active_object->getContent());
				/* Clear possible cracking animation */
				if (nodepos_old != v3s16(-32768,-32768,-32768)) {
					dig_time = 0.0;
					nodepos_old = v3s16(-32768,-32768,-32768);
				}

				core::aabbox3d<f32> *selection_box
						= selected_active_object->getSelectionBox();
				// Box should exist because object was returned in the
				// first place
				assert(selection_box);

				v3f pos = selected_active_object->getPosition()-intToFloat(camera_offset,BS);

				core::aabbox3d<f32> box_on_map(
						selection_box->MinEdge + pos,
						selection_box->MaxEdge + pos
				);

				if (selected_active_object->doShowSelectionBox())
					hilightboxes.push_back(box_on_map);

				infotext = narrow_to_wide(selected_active_object->infoText());

				if (input->getLeftState()) {
					tooluse_t usage;
					content_t toolid = CONTENT_IGNORE;
					u16 tooldata = 0;
					InventoryList *mlist = local_inventory.getList("main");
					if (mlist != NULL) {
						InventoryItem *item = mlist->getItem(g_selected_item);
						if (item && (item->getContent()&CONTENT_TOOLITEM_MASK) == CONTENT_TOOLITEM_MASK) {
							ToolItem *titem = (ToolItem*)item;
							toolid = titem->getContent();
							tooldata = titem->getData();
						}
					}

					if (!get_tool_use(&usage,selected_active_object->getContent(),0,toolid,tooldata)) {
						left_punch = true;
						client.clickActiveObject(0, selected_active_object->getId(), g_selected_item);
						action_delay_counter = usage.delay;
					}
				}else if (input->getRightClicked()) {
					infostream<<"Right-clicked object"<<std::endl;
					client.clickActiveObject(1, selected_active_object->getId(), g_selected_item);
					action_delay_counter = 0.25;
				}
			}else{ // selected_object == NULL
				/*
					Find out which node we are pointing at
				*/

				bool nodefound = false;
				v3s16 nodepos;
				v3s16 neighbourpos;
				core::aabbox3d<f32> nodehilightbox;

				getPointedNode(&client, player_position,
						camera_direction, camera_position,
						nodefound, shootline,
						nodepos, neighbourpos, camera_offset,
						nodehilightbox, d);

				if (!nodefound) {
					if (nodepos_old != v3s16(-32768,-32768,-32768)) {
						dig_time = 0.0;
						nodepos_old = v3s16(-32768,-32768,-32768);
					}
					has_selected_node = false;
				}else{
					has_selected_node = true;
					if (nodepos != selected_node_pos)
						selected_node_crack = 0;
					selected_node_pos = nodepos;

					/*
						Check information text of node
					*/

					NodeMetadata *meta = client.getNodeMetadata(nodepos);
					if (meta)
						infotext = meta->infoText();

					/*
						Handle digging
					*/

					if (input->getLeftReleased())
						dig_time = 0.0;
					/*
						Visualize selection
					*/

					if (!highlight_selected_node)
						hilightboxes.push_back(nodehilightbox);

					if (nodepos != nodepos_old) {
						infostream<<"Pointing at ("<<nodepos.X<<","
								<<nodepos.Y<<","<<nodepos.Z<<")"<<std::endl;

						if (nodepos_old != v3s16(-32768,-32768,-32768)) {
							dig_time = 0.0;
							nodepos_old = v3s16(-32768,-32768,-32768);
						}
					}

					if (input->getLeftClicked() || (input->getLeftState() && nodepos != nodepos_old)) {
						infostream<<"Started digging"<<std::endl;
						client.groundAction(0, nodepos, neighbourpos, g_selected_item);
					}
					if (input->getLeftClicked())
						selected_node_crack = 0;
					if (input->getLeftState()) {
						MapNode n = client.getNode(nodepos);

						// Get tool name. Default is "" = bare hands
						content_t toolid = CONTENT_IGNORE;
						u16 tooldata = 0;
						InventoryList *mlist = local_inventory.getList("main");
						if (mlist != NULL) {
							InventoryItem *item = mlist->getItem(g_selected_item);
							if (item && (item->getContent()&CONTENT_TOOLITEM_MASK) == CONTENT_TOOLITEM_MASK) {
								ToolItem *titem = (ToolItem*)item;
								toolid = titem->getContent();
								tooldata = titem->getData();
							}
						}

						// Get digging properties for material and tool
						content_t material = n.getContent();
						uint16_t mineral = n.getMineral();
						tooluse_t usage;

						if (get_tool_use(&usage,material,mineral,toolid,tooldata))
							usage.diggable = false;

						float dig_time_complete = 0.0;

						if (!usage.diggable) {
							dig_time_complete = 10000000.0;
						}else{
							dig_time_complete = usage.data;
							if (enable_particles)
								addPunchingParticles(smgr, player, nodepos, content_features(n).tiles);

							if (dig_time_complete >= 0.001) {
								selected_node_crack = (u16)((float)CRACK_ANIMATION_LENGTH
										* dig_time/dig_time_complete);
							}else{
								selected_node_crack = CRACK_ANIMATION_LENGTH;
							}

							if (selected_node_crack >= CRACK_ANIMATION_LENGTH) {
								infostream<<"Digging completed"<<std::endl;
								client.groundAction(3, nodepos, neighbourpos, g_selected_item);
								selected_node_crack = 0;
								client.removeNode(nodepos);

								if (enable_particles)
									addDiggingParticles(smgr, player, nodepos, content_features(n).tiles);

								dig_time = 0.0;

								action_delay_counter = usage.delay;
							}
						}

						dig_time += dtime;

						camera.setDigging(0);  // left click animation
					}


					if (input->wasKeyDown(getKeySetting(VLKC_EXAMINE))) {
						// If metadata provides an inventory view, activate it
						if (meta && meta->getDrawSpecString(client.getLocalPlayer()) != "") {
							infostream<<"Launching custom inventory view"<<std::endl;

							InventoryLocation inventoryloc;
							inventoryloc.setNodeMeta(nodepos);

							/* Create menu */

							GUIFormSpecMenu* const menu = new GUIFormSpecMenu(guienv, guiroot, -1, &g_menumgr, &client);
							
							menu->setFormSpec(meta->getDrawSpecString(client.getLocalPlayer()), inventoryloc);
							menu->setFormIO(new NodeMetadataFormIO(nodepos, &client));
							menu->drop();
							{
								std::string sound = content_features(client.getEnv().getMap().getNodeNoEx(nodepos).getContent()).sound_access;
								if (sound == "")
									sound = "open-menu";
								client.playSound(sound,0);
							}
							client.setFormState(true);
						}else{
							MapNode nn = client.getNode(nodepos);
							v3s16 aa = content_features(nn).onact_also_affects;
							if (aa != v3s16(0,0,0)) {
								v3s16 npos = nodepos+nn.getEffectedRotation();
								NodeMetadata *ameta = client.getNodeMetadata(npos);
								if (ameta && ameta->getDrawSpecString(client.getLocalPlayer()) != "") {
									infostream<<"Launching custom inventory view"<<std::endl;

									InventoryLocation inventoryloc;
									inventoryloc.setNodeMeta(nodepos);

									/* Create menu */

									GUIFormSpecMenu* const menu =
									    new GUIFormSpecMenu(guienv, guiroot, -1, &g_menumgr, &client);
									
									menu->setFormSpec(ameta->getDrawSpecString(client.getLocalPlayer()), inventoryloc);
									menu->setFormIO(new NodeMetadataFormIO(npos, &client));
									menu->drop();
									{
										std::string sound = content_features(nn.getContent()).sound_access;
										if (sound == "")
											sound = "open-menu";
										client.playSound(sound,0);
									}
									client.setFormState(true);
								}
							}
						}
					}
					if (input->getRightClicked()) {
						client.groundAction(1, nodepos, neighbourpos, g_selected_item);
						camera.setDigging(1);  // right click animation
					}

					nodepos_old = nodepos;
				}

			} // selected_object == NULL
		}

		// this lets us hold down use to eat, and limits to 2 items per second
		if (input->wasKeyDown(getKeySetting(VLKC_USE))) {
			if (action_delay_counter <= 0.0) {
				client.useItem();
				/* TODO: this should come from content*_features */
				action_delay_counter = 0.25;
			}
		}

		if (left_punch || (input->getLeftClicked() && !left_punch_muted))
			camera.setDigging(0); // left click animation

		input->resetLeftClicked();
		input->resetRightClicked();

		if (input->getLeftReleased()) {
			infostream<<"Left button released (stopped digging)"
					<<std::endl;
			client.groundAction(2, v3s16(0,0,0), v3s16(0,0,0), 0);
		}
		if (input->getRightReleased()) {
			// Nothing here
		}

		input->resetLeftReleased();
		input->resetRightReleased();

		float fog_range = 0.0;
		if (draw_control.range_all) {
			fog_range = 100000*BS;
		}else{
			fog_range = (draw_control.wanted_range+MAP_BLOCKSIZE)*(BS*1.5);
			fog_range *= 0.9;
		}

		u32 daynight_ratio = client.getEnv().getDayNightRatio();
		float time_brightness = (float)decode_light((daynight_ratio * LIGHT_SUN) / 1000) / 255.0;
		float direct_brightness = 0;
		bool sunlight_seen = false;
		uint8_t biome = BIOME_UNKNOWN;
		{
			v3f pp = client.getLocalPlayer()->getPosition();
			v3s16 ppos = floatToInt(pp,BS);
			MapBlock *block = client.getEnv().getMap().getBlockNoCreateNoEx(getNodeBlockPos(ppos));
			if (block != NULL)
				biome = block->getBiome();
		}
		if (biome == BIOME_SPACE || free_move) {
			direct_brightness = time_brightness;
			sunlight_seen = true;
		}else{
			ScopeProfiler sp(g_profiler, "Detecting background light", SPT_AVG);
			float old_brightness = sky->getBrightness();
			direct_brightness = (float)client.getEnv().getClientMap().getBackgroundBrightness(
				MYMIN(fog_range*1.2, 60*BS),
				daynight_ratio,
				(int)(old_brightness*255.5),
				&sunlight_seen
			);
			direct_brightness /= 255.0;
		}

		time_of_day = client.getEnv().getTimeOfDayF();
		float maxsm = 0.05;
		if (
			fabs(time_of_day - time_of_day_smooth) > maxsm
			&& fabs(time_of_day - time_of_day_smooth + 1.0) > maxsm
			&& fabs(time_of_day - time_of_day_smooth - 1.0) > maxsm
		)
			time_of_day_smooth = time_of_day;
		float todsm = 0.05;
		if (time_of_day_smooth > 0.8 && time_of_day < 0.2) {
			time_of_day_smooth = time_of_day_smooth * (1.0-todsm) + (time_of_day+1.0) * todsm;
		}else{
			time_of_day_smooth = time_of_day_smooth * (1.0-todsm) + time_of_day * todsm;
		}

		float moon_phase = client.getEnv().getMoonPhase();

		sky->update(time_of_day_smooth, moon_phase, time_brightness, direct_brightness, sunlight_seen, biome);

		video::SColor bgcolor = sky->getBgColor();
		video::SColor skycolor = sky->getSkyColor();

		/*
			Update clouds
		*/
		if (clouds) {
			if (sky->getCloudsVisible()) {
				clouds->setVisible(true);
				clouds->step(dtime);
				clouds->update(v2f(player_position.X, player_position.Z), sky->getCloudColor());
			}else{
				clouds->setVisible(false);
			}
		}

		/*
			Update particles
		*/
		allparticles_step(dtime, client.getEnv());
		allparticlespawners_step(dtime, client.getEnv());

		/*
			Fog
		*/

		if (enable_fog && !force_fog_off) {
			driver->setFog(
				bgcolor,
				video::EFT_FOG_LINEAR,
				fog_range*0.4,
				fog_range*1.0,
				0.01,
				false, // pixel fog
				false // range fog
			);
		}else{
			driver->setFog(
				bgcolor,
				video::EFT_FOG_LINEAR,
				100000*BS,
				110000*BS,
				0.01,
				false, // pixel fog
				false // range fog
			);
		}

		float client_rtt = client.getRTT();
		if (client_rtt < -1000) {
			error_message = narrow_to_wide(gettext("Disconnected (Network Timeout)"));
			break;
		}

		/*
			Update gui stuff (0ms)
		*/
		const char program_name_and_version[] = "Voxelands " VERSION_STRING;
		if (show_debug) {
			static float drawtime_avg = 0;
			drawtime_avg = drawtime_avg * 0.95 + (float)drawtime*0.05;

			{
				char temptext[300];
				snprintf(
					temptext,
					300,
					"%s (R: range_all=%i)"
					" drawtime=%.0f, dtime_jitter = % .1f %%"
					", v_range = %.1f, RTT = %.3f",
					program_name_and_version,
					draw_control.range_all,
					drawtime_avg,
					dtime_jitter1_max_fraction * 100.0,
					draw_control.wanted_range,
					client_rtt
				);

				guitext->setText(narrow_to_wide(temptext).c_str());
				guitext->setVisible(true);
			}
			{
				char temptext[300];
				snprintf(temptext, 300,
						"(% .1f, % .1f, % .1f)"
						" (yaw = %.1f)",
						player_position.X/BS,
						player_position.Y/BS,
						player_position.Z/BS,
						wrapDegrees_0_360(camera_yaw));

				guitext2->setText(narrow_to_wide(temptext).c_str());
				guitext2->setVisible(true);
			}
		}else if (show_hud || show_chat) {
			char temptext[300];
			snprintf(temptext, 300,
					"(% .1f, % .1f, % .1f)"
					" (yaw = %.1f)",
					player_position.X/BS,
					player_position.Y/BS,
					player_position.Z/BS,
					wrapDegrees_0_360(camera_yaw));

			guitext2->setText(narrow_to_wide(temptext).c_str());
			guitext2->setVisible(true);
			guitext->setText(narrow_to_wide(program_name_and_version).c_str());
			guitext->setVisible(true);
		}else{
			guitext->setVisible(false);
			guitext2->setVisible(false);
		}

		if (!show_debug && g_menumgr.menuCount() == 0) {
			guitext_info->setText(infotext.c_str());
			guitext_info->setVisible(show_hud);
		}else{
			guitext_info->setVisible(false);
		}

		/*
			Get chat messages from client
		*/
		{
			std::wstring message;
			/* get new messages */
			while (client.getChatMessage(message)) {
				chatline_add(&chat_lines,message,0.0);
			}
		}
		if (chat_lines) {
			ref_t *ref;
			ref_t *refn;
			ChatLine *line;
			std::wstring whole;
			s16 line_number = 0;

			/* first, remove old status messages */
			ref = chat_lines;
			while (ref) {
				line = (ChatLine*)ref->ref;
				if (line->age < -50) {
					line->age += dtime;
					if (line->age > -100.0) {
						refn = ref;
						ref = ref->next;
						chat_lines = (ref_t*)list_remove(&chat_lines,refn);
						delete line;
						free(refn);
						continue;
					}
				}else{
					line_number++;
				}
				ref = ref->next;
			}

			/* second, remove old and excess chat messages */
			ref = chat_lines;
			while (ref) {
				line = (ChatLine*)ref->ref;
				if (line->age > -50) {
					line_number--;
					line->age += dtime;
					float allowed_age = (6-line_number) * 60.0;

					if (line->age > allowed_age) {
						refn = ref;
						ref = ref->next;
						chat_lines = (ref_t*)list_remove(&chat_lines,refn);
						delete line;
						free(refn);
						continue;
					}
				}
				whole += line->text + L'\n';
				ref = ref->next;
			}
			guitext_chat->setText(whole.c_str());

			// Update gui element size and position

			s32 chat_y = 5+(2*text_height);
			core::rect<s32> rect(
				10,
				chat_y,
				screensize.X - 10,
				chat_y + guitext_chat->getTextHeight()
			);

			guitext_chat->setRelativePosition(rect);

			// Don't show chat if empty or profiler or debug is enabled
			guitext_chat->setVisible(chat_lines != NULL && show_chat && show_profiler == 0);
		}else{
			guitext_chat->setVisible(false);
		}

		/*
			Inventory
		*/

		static u16 old_selected_item = 65535;
		if (client.getLocalInventoryUpdated() || g_selected_item != old_selected_item) {
			client.selectPlayerItem(g_selected_item);
			old_selected_item = g_selected_item;
			client.getLocalInventory(local_inventory);

			// Update wielded tool
			InventoryList *mlist = local_inventory.getList("main");
			InventoryItem *item = NULL;
			if (mlist != NULL)
				item = mlist->getItem(g_selected_item);
			camera.wield(item);
		}

		/*
			Send actions returned by the inventory menu
		*/
		while (inventory_action_queue.size() != 0) {
			InventoryAction *a = inventory_action_queue.pop_front();

			client.sendInventoryAction(a);
			// Eat it
			delete a;
		}

		/*
			Drawing begins
		*/

		TimeTaker drawtimer("Drawing");


		{
			TimeTaker timer("beginScene");
			driver->beginScene(true, true, skycolor);
			timer.stop(true);
		}

		{
			TimeTaker timer("smgr");
			smgr->drawAll();
			timer.stop(true);
		}

		{

		driver->setTransform(video::ETS_WORLD, core::IdentityMatrix);

		if (show_hud) {
			/* TODO: this should get nodes from the client, for other players' cracks */
			std::vector<SelectedNode> selected_nodes;
			if (has_selected_node) {
				MapNode snode = client.getEnv().getMap().getNodeNoEx(selected_node_pos,NULL);
				selected_nodes.push_back(
					SelectedNode(selected_node_pos,selected_node_crack,highlight_selected_node,snode.getContent())
				);
				MapNode nn = client.getNode(selected_node_pos);
				v3s16 aa = content_features(nn).onact_also_affects;
				if (aa != v3s16(0,0,0)) {
					v3s16 spos = selected_node_pos+nn.getEffectedRotation();
					snode = client.getEnv().getMap().getNodeNoEx(spos,NULL);
					selected_nodes.push_back(
						SelectedNode(spos,selected_node_crack,highlight_selected_node,snode.getContent())
					);
				}
			}

			if (selected_nodes.size() > 0)
				selection_draw(driver,client,camera.getOffset(),selected_nodes);

			/* draw old-style selection boxes */
			if (hilightboxes.size()) {
				video::SMaterial m;
				m.Thickness = 3;
				m.Lighting = false;
				driver->setMaterial(m);
				for (core::list<aabb3f>::Iterator i=hilightboxes.begin(); i != hilightboxes.end(); i++) {
					driver->draw3DBox(*i, video::SColor(255,0,0,0));
				}
			}

			/*
				Wielded tool
			*/
			// Warning: This clears the Z buffer.
			camera.drawWieldedTool();
		}

		/*
			Post effects
		*/
		{
			client.renderPostFx();
		}

		/*
			Frametime log
		*/
		if (show_debug_frametime) {
			s32 x = 10;
			s32 y = screensize.Y - 10;
			for (core::list<float>::Iterator i = frametime_log.begin(); i != frametime_log.end(); i++) {
				driver->draw2DLine(
					v2s32(x,y),
					v2s32(x,y+(*i)*1000),
					video::SColor(255,255,255,255)
				);
				x++;
			}
		}

		} // timer

		/*
			Draw gui
		*/
		// 0-1ms
		guienv->drawAll();

		/*
			Draw hotbar
		*/
		if (show_hud) {
			s32 hunger = 0;
			if (client.getServerHunger())
				hunger = client.getHunger();
			if (old_hotbar) {
				if (g_menumgr.menuCount() < 1) {
					client.setFormState(false);
					/*
						Draw crosshair
					*/
					driver->draw2DLine(
						displaycenter - core::vector2d<s32>(10,0),
						displaycenter + core::vector2d<s32>(10,0),
						video::SColor(255,255,255,255)
					);
					driver->draw2DLine(
						displaycenter - core::vector2d<s32>(0,10),
						displaycenter + core::vector2d<s32>(0,10),
						video::SColor(255,255,255,255)
					);
				}
				hud_draw_old(
					driver,
					font,
					v2s32(screensize.X/2,screensize.Y),
					&local_inventory,
					client.getHP()/5,
					client.getAir()/5,
					hunger/5
				);
			}else{
				int crosshair = 1;
				if (g_menumgr.menuCount() > 0) {
					crosshair = 0;
				}else{
					client.setFormState(false);
					if (client.getPointedContent() != CONTENT_IGNORE) {
						crosshair = 2;
						if (!has_selected_node)
							crosshair = 3;
					}
				}
				MapNode snode;
				uint8_t biome = BIOME_UNKNOWN;
				v3s16 spos = v3s16(0,0,0);
				if (show_debug) {
					if (has_selected_node) {
						spos = selected_node_pos;
					}else{
						v3f pp = client.getLocalPlayer()->getPosition();
						spos = floatToInt(pp,BS);
					}
					MapBlock *block = client.getEnv().getMap().getBlockNoCreateNoEx(getNodeBlockPos(spos));
					snode = client.getEnv().getMap().getNodeNoEx(spos,NULL);
					if (block != NULL)
						biome = block->getBiome();
				}

				LocalPlayer *p = client.getLocalPlayer();

				hud_draw(
					driver,
					font,
					v2s32(screensize.X,screensize.Y),
					show_index,
					&local_inventory,
					client.getServerDamage(),
					client.getHP(),
					(p->last_damage&0xFF00)>>8,
					(p->last_damage&0x00FF),
					p->cold_effectf,
					client.getServerSuffocation(),
					client.getAir(),
					client.getServerHunger(),
					hunger,
					client.getEnergy(),
					p->energy_effectf,
					crosshair,
					show_debug,
					has_selected_node,
					spos,
					snode,
					biome,
					client.getEnv().getTime()
				);
			}
		}

		/*
			Damage flash
		*/
		if (damage_flash_timer > 0.0) {
			damage_flash_timer -= dtime;

			video::SColor color(128,255,0,0);
			driver->draw2DRectangle(color,
					core::rect<s32>(0,0,screensize.X,screensize.Y),
					NULL);
		}

		// Clear Z buffer
		driver->clearZBuffer();

		/*
			End scene
		*/
		{
			TimeTaker timer("endScene");
			endSceneX(driver);
			timer.stop(true);
		}

		drawtime = drawtimer.stop(true);

		/*
			End of drawing
		*/

		static s16 lastFPS = 0;
		u16 fps = (1.0/dtime_avg1);

		if (lastFPS != fps) {
			core::stringw str = L"Voxelands [";
			str += driver->getName();
			str += "] FPS=";
			str += fps;

			device->setWindowCaption(str.c_str());
			lastFPS = fps;
		}
	}

	/*
		Drop stuff
	*/
	if (clouds)
		clouds->drop();

	clear_particles();

	bridge_register_client(NULL);

	/*
		Draw a "shutting down" screen, which will be shown while the map
		generator and other stuff quits
	*/
	{
		drawLoadingScreen(device,narrow_to_wide(gettext("Shutting down...")).c_str());
	}
}
