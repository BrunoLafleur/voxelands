/************************************************************************
* Minetest-c55
* Copyright (C) 2010-2011 celeron55, Perttu Ahola <celeron55@gmail.com>
*
* main.cpp
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

#ifdef NDEBUG
	// Disable unit tests
	#define ENABLE_TESTS 0
#else
	// Enable unit tests
	#define ENABLE_TESTS 1
#endif

#ifdef _MSC_VER
	#pragma comment(lib, "Irrlicht.lib")
	//#pragma comment(lib, "jthread.lib")
	#pragma comment(lib, "zlibwapi.lib")
	#pragma comment(lib, "Shell32.lib")
	// This would get rid of the console window
	//#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")
#endif

#include "irrlicht.h" // createDevice

#include "main.h"
#include "mainmenumanager.h"
#include <iostream>
#include <fstream>
#include <locale.h>
#include "common_irrlicht.h"
#include "debug.h"
#include "test.h"
#include "server.h"
#include "constants.h"
#include "porting.h"
#include "gettime.h"
#include "guiMessageMenu.h"
#include "config.h"
#include "guiMainMenu.h"
#include "mineral.h"
#include "game.h"
#include "keycode.h"
#include "tile.h"
#include "intl.h"
#include "profiler.h"
#include "log.h"
// for the init functions
#include "content_craft.h"
#include "content_clothesitem.h"
#include "content_craftitem.h"
#include "content_toolitem.h"
#include "content_mob.h"
#include "path.h"
#include "gui_colours.h"
#include "character_creator.h"
#include "thread.h"
#if USE_FREETYPE
#include "xCGUITTFont.h"
#endif
#include "sound.h"

// This makes textures
ITextureSource *g_texturesource = NULL;

// Global profiler
Profiler main_profiler;
Profiler *g_profiler = &main_profiler;

/*
	Random stuff
*/

/*
	mainmenumanager.h
*/

gui::IGUIEnvironment* guienv = NULL;
gui::IGUIStaticText *guiroot = NULL;
MainMenuManager g_menumgr;

bool noMenuActive()
{
	return (g_menumgr.menuCount() == 0);
}

// Passed to menus to allow disconnecting and exiting
MainGameCallback *g_gamecallback = NULL;

/*
	Debug streams
*/

// Connection
std::ostream *dout_con_ptr = &dummyout;
std::ostream *derr_con_ptr = &verbosestream;
//std::ostream *dout_con_ptr = &infostream;
//std::ostream *derr_con_ptr = &errorstream;

// Server
std::ostream *dout_server_ptr = &infostream;
std::ostream *derr_server_ptr = &errorstream;

// Client
std::ostream *dout_client_ptr = &infostream;
std::ostream *derr_client_ptr = &errorstream;

/*
	gettime.h implementation
*/

// A small helper class
class TimeGetter
{
public:
	virtual ~TimeGetter() {};
	virtual u32 getTime() = 0;
};

// A precise irrlicht one
class IrrlichtTimeGetter: public TimeGetter
{
public:
	IrrlichtTimeGetter(IrrlichtDevice *device):
		m_device(device)
	{}
	u32 getTime()
	{
		if(m_device == NULL)
			return 0;
		return m_device->getTimer()->getRealTime();
	}
private:
	IrrlichtDevice *m_device;
};
// Not so precise one which works without irrlicht
class SimpleTimeGetter: public TimeGetter
{
public:
	u32 getTime()
	{
		return porting::getTimeMs();
	}
};

// A pointer to a global instance of the time getter
// TODO: why?
TimeGetter *g_timegetter = NULL;

u32 getTimeMs()
{
	if(g_timegetter == NULL)
		return 0;
	return g_timegetter->getTime();
}

/*
	Event handler for Irrlicht

	NOTE: Everything possible should be moved out from here,
	      probably to InputHandler and the_game
*/

class MyEventReceiver : public IEventReceiver
{
public:
	// This is the one method that we have to implement
	virtual bool OnEvent(const SEvent& event)
	{
		/*
			React to nothing here if a menu is active
		*/
		if(noMenuActive() == false)
		{
			return false;
		}

		// Remember whether each key is down or up
		if(event.EventType == irr::EET_KEY_INPUT_EVENT)
		{
			if(event.KeyInput.PressedDown) {
				keyIsDown.set(event.KeyInput);
				keyWasDown.set(event.KeyInput);
			} else {
				keyIsDown.unset(event.KeyInput);
			}
		}

		if(event.EventType == irr::EET_MOUSE_INPUT_EVENT)
		{
			if(noMenuActive() == false)
			{
				left_active = false;
				middle_active = false;
				right_active = false;
			}
			else
			{
				left_active = event.MouseInput.isLeftPressed();
				middle_active = event.MouseInput.isMiddlePressed();
				right_active = event.MouseInput.isRightPressed();

				if(event.MouseInput.Event == EMIE_LMOUSE_PRESSED_DOWN)
				{
					leftclicked = true;
				}
				if(event.MouseInput.Event == EMIE_RMOUSE_PRESSED_DOWN)
				{
					rightclicked = true;
				}
				if(event.MouseInput.Event == EMIE_LMOUSE_LEFT_UP)
				{
					leftreleased = true;
				}
				if(event.MouseInput.Event == EMIE_RMOUSE_LEFT_UP)
				{
					rightreleased = true;
				}
				if(event.MouseInput.Event == EMIE_MOUSE_WHEEL)
				{
					mouse_wheel += event.MouseInput.Wheel;
				}
			}
		}

		return false;
	}

	bool IsKeyDown(const KeyPress &keyCode) const
	{
		return keyIsDown[keyCode];
	}

	// Checks whether a key was down and resets the state
	bool WasKeyDown(const KeyPress &keyCode)
	{
		bool b = keyWasDown[keyCode];
		if (b)
			keyWasDown.unset(keyCode);
		return b;
	}

	s32 getMouseWheel()
	{
		s32 a = mouse_wheel;
		mouse_wheel = 0;
		return a;
	}

	void clearInput()
	{
		keyIsDown.clear();
		keyWasDown.clear();

		leftclicked = false;
		rightclicked = false;
		leftreleased = false;
		rightreleased = false;

		left_active = false;
		middle_active = false;
		right_active = false;

		mouse_wheel = 0;
	}

	MyEventReceiver()
	{
		clearInput();
	}

	bool leftclicked;
	bool rightclicked;
	bool leftreleased;
	bool rightreleased;

	bool left_active;
	bool middle_active;
	bool right_active;

	s32 mouse_wheel;

private:
	// The current state of keys
	KeyList keyIsDown;
	// Whether a key has been pressed or not
	KeyList keyWasDown;
};

/*
	Separated input handler
*/

class RealInputHandler : public InputHandler
{
public:
	RealInputHandler(IrrlichtDevice *device, MyEventReceiver *receiver):
		m_device(device),
		m_receiver(receiver)
	{
	}
	virtual bool isKeyDown(const KeyPress &keyCode)
	{
		return m_receiver->IsKeyDown(keyCode);
	}
	virtual bool wasKeyDown(const KeyPress &keyCode)
	{
		return m_receiver->WasKeyDown(keyCode);
	}
	virtual v2s32 getMousePos()
	{
		return m_device->getCursorControl()->getPosition();
	}
	virtual void setMousePos(s32 x, s32 y)
	{
		m_device->getCursorControl()->setPosition(x, y);
	}

	virtual bool getLeftState()
	{
		return m_receiver->left_active;
	}
	virtual bool getRightState()
	{
		return m_receiver->right_active;
	}

	virtual bool getLeftClicked()
	{
		return m_receiver->leftclicked;
	}
	virtual bool getRightClicked()
	{
		return m_receiver->rightclicked;
	}
	virtual void resetLeftClicked()
	{
		m_receiver->leftclicked = false;
	}
	virtual void resetRightClicked()
	{
		m_receiver->rightclicked = false;
	}

	virtual bool getLeftReleased()
	{
		return m_receiver->leftreleased;
	}
	virtual bool getRightReleased()
	{
		return m_receiver->rightreleased;
	}
	virtual void resetLeftReleased()
	{
		m_receiver->leftreleased = false;
	}
	virtual void resetRightReleased()
	{
		m_receiver->rightreleased = false;
	}

	virtual s32 getMouseWheel()
	{
		return m_receiver->getMouseWheel();
	}

	void clear()
	{
		m_receiver->clearInput();
	}
private:
	IrrlichtDevice *m_device;
	MyEventReceiver *m_receiver;
};

class RandomInputHandler : public InputHandler
{
public:
	RandomInputHandler()
	{
		leftdown = false;
		rightdown = false;
		leftclicked = false;
		rightclicked = false;
		leftreleased = false;
		rightreleased = false;
		keydown.clear();
	}
	virtual bool isKeyDown(const KeyPress &keyCode)
	{
		return keydown[keyCode];
	}
	virtual bool wasKeyDown(const KeyPress &keyCode)
	{
		return false;
	}
	virtual v2s32 getMousePos()
	{
		return mousepos;
	}
	virtual void setMousePos(s32 x, s32 y)
	{
		mousepos = v2s32(x,y);
	}

	virtual bool getLeftState()
	{
		return leftdown;
	}
	virtual bool getRightState()
	{
		return rightdown;
	}

	virtual bool getLeftClicked()
	{
		return leftclicked;
	}
	virtual bool getRightClicked()
	{
		return rightclicked;
	}
	virtual void resetLeftClicked()
	{
		leftclicked = false;
	}
	virtual void resetRightClicked()
	{
		rightclicked = false;
	}

	virtual bool getLeftReleased()
	{
		return leftreleased;
	}
	virtual bool getRightReleased()
	{
		return rightreleased;
	}
	virtual void resetLeftReleased()
	{
		leftreleased = false;
	}
	virtual void resetRightReleased()
	{
		rightreleased = false;
	}

	virtual s32 getMouseWheel()
	{
		return 0;
	}

	virtual void step(float dtime)
	{
		{
			static float counter1 = 0;
			counter1 -= dtime;
			if(counter1 < 0.0)
			{
				counter1 = 0.1*Rand(1, 40);
				keydown.toggle(getKeySetting(VLKC_JUMP));
			}
		}
		{
			static float counter1 = 0;
			counter1 -= dtime;
			if(counter1 < 0.0)
			{
				counter1 = 0.1*Rand(1, 40);
				keydown.toggle(getKeySetting(VLKC_USE));
			}
		}
		{
			static float counter1 = 0;
			counter1 -= dtime;
			if(counter1 < 0.0)
			{
				counter1 = 0.1*Rand(1, 40);
				keydown.toggle(getKeySetting(VLKC_FORWARD));
			}
		}
		{
			static float counter1 = 0;
			counter1 -= dtime;
			if(counter1 < 0.0)
			{
				counter1 = 0.1*Rand(1, 40);
				keydown.toggle(getKeySetting(VLKC_LEFT));
			}
		}
		{
			static float counter1 = 0;
			counter1 -= dtime;
			if(counter1 < 0.0)
			{
				counter1 = 0.1*Rand(1, 20);
				mousespeed = v2s32(Rand(-20,20), Rand(-15,20));
			}
		}
		{
			static float counter1 = 0;
			counter1 -= dtime;
			if(counter1 < 0.0)
			{
				counter1 = 0.1*Rand(1, 30);
				leftdown = !leftdown;
				if(leftdown)
					leftclicked = true;
				if(!leftdown)
					leftreleased = true;
			}
		}
		{
			static float counter1 = 0;
			counter1 -= dtime;
			if(counter1 < 0.0)
			{
				counter1 = 0.1*Rand(1, 15);
				rightdown = !rightdown;
				if(rightdown)
					rightclicked = true;
				if(!rightdown)
					rightreleased = true;
			}
		}
		mousepos += mousespeed;
	}

	s32 Rand(s32 min, s32 max)
	{
		return (myrand()%(max-min+1))+min;
	}
private:
	KeyList keydown;
	v2s32 mousepos;
	v2s32 mousespeed;
	bool leftdown;
	bool rightdown;
	bool leftclicked;
	bool rightclicked;
	bool leftreleased;
	bool rightreleased;
};

// These are defined global so that they're not optimized too much.
// Can't change them to volatile.
s16 temp16;
f32 tempf;
v3f tempv3f1;
v3f tempv3f2;
std::string tempstring;
std::string tempstring2;

void SpeedTests()
{
	{
		dstream<<"The following test should take around 20ms."<<std::endl;
		TimeTaker timer("Testing std::string speed");
		const u32 jj = 10000;
		for(u32 j=0; j<jj; j++)
		{
			tempstring = "";
			tempstring2 = "";
			const u32 ii = 10;
			for(u32 i=0; i<ii; i++){
				tempstring2 += "asd";
			}
			for(u32 i=0; i<ii+1; i++){
				tempstring += "asd";
				if(tempstring == tempstring2)
					break;
			}
		}
	}

	dstream<<"All of the following tests should take around 100ms each."
			<<std::endl;

	{
		TimeTaker timer("Testing floating-point conversion speed");
		tempf = 0.001;
		for(u32 i=0; i<4000000; i++){
			temp16 += tempf;
			tempf += 0.001;
		}
	}

	{
		TimeTaker timer("Testing floating-point vector speed");

		tempv3f1 = v3f(1,2,3);
		tempv3f2 = v3f(4,5,6);
		for(u32 i=0; i<10000000; i++){
			tempf += tempv3f1.dotProduct(tempv3f2);
			tempv3f2 += v3f(7,8,9);
		}
	}

	{
		TimeTaker timer("Testing core::map speed");

		core::map<v2s16, f32> map1;
		tempf = -324;
		const s16 ii=300;
		for(s16 y=0; y<ii; y++){
			for(s16 x=0; x<ii; x++){
				map1.insert(v2s16(x,y), tempf);
				tempf += 1;
			}
		}
		for(s16 y=ii-1; y>=0; y--){
			for(s16 x=0; x<ii; x++){
				tempf = map1[v2s16(x,y)];
			}
		}
	}

	{
		dstream<<"Around 5000/ms should do well here."<<std::endl;
		TimeTaker timer("Testing mutex speed");

		JMutex m;
		m.Init();
		u32 n = 0;
		u32 i = 0;
		do{
			n += 10000;
			for(; i<n; i++){
				m.Lock();
				m.Unlock();
			}
		}
		// Do at least 10ms
		while(timer.getTime() < 10);

		u32 dtime = timer.stop();
		u32 per_ms = n / dtime;
		dstream<<"Done. "<<dtime<<"ms, "
				<<per_ms<<"/ms"<<std::endl;
	}
}

void drawMenuBackground(video::IVideoDriver* driver)
{
	char buff[1024];
	core::dimension2d<s32> screensize(driver->getScreenSize());

	video::ITexture *background = NULL;
	if (path_get((char*)"texture",(char*)"menubg0.png",1,buff,1024))
		background = driver->getTexture(buff);

	if (!background)
		return;

	core::dimension2d<s32> texturesize(background->getSize());

	core::rect<s32> image(0,0,texturesize.Width,texturesize.Height);
	core::rect<s32> pos(
		(texturesize.Width/2)-(screensize.Width/2),
		(texturesize.Height/2)-(screensize.Height/2),
		texturesize.Width,
		texturesize.Height
	);
	driver->draw2DImage(
		background,
		image,
		pos,
		NULL,
		NULL,
		true
	);
}

class StderrLogOutput: public ILogOutput
{
public:
	/* line: Full line with timestamp, level and thread */
	void printLog(const std::string &line)
	{
		std::cerr<<line<<std::endl;
	}
} main_stderr_log_out;

class DstreamNoStderrLogOutput: public ILogOutput
{
public:
	/* line: Full line with timestamp, level and thread */
	void printLog(const std::string &line)
	{
		dstream_no_stderr<<line<<std::endl;
	}
} main_dstream_no_stderr_log_out;

int main(int argc, char *argv[])
{
	/*
		Initialization
	*/
	log_mutex.Init();
	log_add_output_maxlev(&main_stderr_log_out, LMT_ACTION);
	log_add_output_all_levs(&main_dstream_no_stderr_log_out);
	log_register_thread("main");

	/*
		Low-level initialization
	*/

	bool disable_stderr = false;

	porting::signal_handler_init();
	bool &kill = *porting::signal_handler_killstatus();

	thread_init();
	path_init();
	intl_init();
	command_init();
	config_init(argc,argv);

	// Initialize debug streams
	{
		char buff[1024];
		if (!path_get(NULL,(char*)"debug.txt",0,buff,1024))
			return 1;
		debugstreams_init(disable_stderr, buff);
	}
	// Initialize debug stacks
	debug_stacks_init();

	DSTACK(__FUNCTION_NAME);

	// Init material properties table
	//initializeMaterialProperties();

	// Debug handler
	BEGIN_DEBUG_EXCEPTION_HANDLER

	// Print startup message
	actionstream<<PROJECT_NAME<<
			" with SER_FMT_VER_HIGHEST="<<(int)SER_FMT_VER_HIGHEST
			<<", "<<BUILD_INFO
			<<std::endl;

	// Initialize sockets
	sockets_init();
	atexit(sockets_cleanup);

	// Initialize random seed
	srand(time(0));
	mysrand(time(0));

	/*
		Pre-initialize some stuff with a dummy irrlicht wrapper.

		These are needed for unit tests at least.
	*/

	crafting::initCrafting();
	content_clothesitem_init();
	content_craftitem_init();
	content_toolitem_init();
	content_mob_init();
	// Initial call with g_texturesource not set.
	init_mapnode(NULL);
	// Must be called before g_texturesource is created
	// (for texture atlas making)
	init_mineral();

	/*
		Game parameters
	*/

	// Run dedicated server if asked to
	if (config_get_bool("server")) {
		DSTACK("Dedicated server branch");

		// Create time getter
		g_timegetter = new SimpleTimeGetter();

		const char* v = config_get("server.world");
		world_init(v);

		// Create server
		Server server;
		server.start();

		// Run server
		dedicated_server_loop(server, kill);

		world_exit();

		config_save(NULL,NULL,NULL);

		return 0;
	}

	/*
		Device initialization
	*/

	// Resolution selection

	bool fullscreen = config_get_bool("client.video.fullscreen");
	u16 screenW = config_get_int("client.video.size.width");
	u16 screenH = config_get_int("client.video.size.height");

	// bpp, fsaa, vsync

	bool vsync = config_get_bool("client.video.vsync");
	u16 bits = config_get_int("client.video.fullscreen.bpp");
	u16 fsaa = config_get_int("client.video.fullscreen.fsaa");

	// Determine driver

	video::E_DRIVER_TYPE driverType = video::EDT_OPENGL;

	{
		const char* v = config_get("client.video.driver");
		if (v && !strcmp(v,"d3d9"))
			driverType = video::EDT_DIRECT3D9;
	}

	/*
		Create device and exit if creation failed
	*/

	MyEventReceiver receiver;

	SIrrlichtCreationParameters params = SIrrlichtCreationParameters();
	params.DriverType    = driverType;
	params.WindowSize    = core::dimension2d<u32>(screenW, screenH);
	params.Bits          = bits;
	params.AntiAlias     = fsaa;
	params.Fullscreen    = fullscreen;
	params.Stencilbuffer = false;
	params.Vsync         = vsync;
	params.EventReceiver = &receiver;
	params.HighPrecisionFPU = config_get_bool("client.video.hpfpu");

	IrrlichtDevice * device = createDeviceEx(params);

	if (device == 0)
		return 1; // could not create selected driver.

	/*
		Continue initialization
	*/

	video::IVideoDriver* driver = device->getVideoDriver();

	// Disable mipmaps (because some of them look ugly)
	driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, false);

	// Set the window caption
	device->setWindowCaption(L"Voxelands");
	drawLoadingScreen(device,L"");

	// Create time getter
	g_timegetter = new IrrlichtTimeGetter(device);

	// Create game callback for menus
	g_gamecallback = new MainGameCallback(device);

	// Create texture source
	g_texturesource = new TextureSource(device);

	drawLoadingScreen(device,L"");

	device->setResizable(true);

	InputHandler *input = new RealInputHandler(device, &receiver);

	scene::ISceneManager* smgr = device->getSceneManager();

	guienv = device->getGUIEnvironment();
	gui::IGUISkin* skin = guienv->getSkin();
	gui::IGUIFont* font = NULL;
	{
		char buff[1024];
#if USE_FREETYPE
		uint16_t font_size = config_get_int("client.ui.font.size");
		const char* v = config_get("client.ui.font");
		if (!v)
			v = "unifont.ttf";
		if (path_get("font",v,1,buff,1024))
			font = gui::CGUITTFont::createTTFont(guienv, buff, font_size, true, true, 1, 128);
#else
		if (path_get("texture","fontlucida.png",1,buff,1024))
			font = guienv->getFont(buff);
#endif
	}
	if (font) {
		skin->setFont(font);
	}else{
		errorstream<<"WARNING: Font file was not found. Using default font."<<std::endl;
	}
	// If font was not found, this will get us one
	font = skin->getFont();
	assert(font);
	drawLoadingScreen(device,narrow_to_wide(gettext("Setting Up UI")));

	u32 text_height = font->getDimension(L"Hello, world!").Height;
	infostream<<"text_height="<<text_height<<std::endl;

	skin->setColor(gui::EGDC_3D_HIGH_LIGHT, GUI_3D_HIGH_LIGHT);
	skin->setColor(gui::EGDC_3D_SHADOW, GUI_3D_SHADOW);
	skin->setColor(gui::EGDC_HIGH_LIGHT, GUI_HIGH_LIGHT);
	skin->setColor(gui::EGDC_HIGH_LIGHT_TEXT, GUI_HIGH_LIGHT_TEXT);
	skin->setColor(gui::EGDC_BUTTON_TEXT, GUI_BUTTON_TEXT);

#if (IRRLICHT_VERSION_MAJOR >= 1 && IRRLICHT_VERSION_MINOR >= 8) || IRRLICHT_VERSION_MAJOR >= 2
	// Irrlicht 1.8 input colours
	skin->setColor(gui::EGDC_3D_FACE, GUI_3D_FACE);
	skin->setColor(gui::EGDC_3D_DARK_SHADOW, GUI_3D_DARK_SHADOW);
	skin->setColor(gui::EGDC_EDITABLE, GUI_EDITABLE);
	skin->setColor(gui::EGDC_FOCUSED_EDITABLE, GUI_FOCUSED_EDITABLE);
#endif

	/*
		Preload some textures and stuff
	*/

	drawLoadingScreen(device,narrow_to_wide(gettext("Loading MapNodes")));
	init_mapnode(device); // Second call with g_texturesource set
	drawLoadingScreen(device,narrow_to_wide(gettext("Loading Creatures")));
	content_mob_init();
	// preloading this reduces some hud flicker
	g_texturesource->getTextureId("crack.png");

	drawLoadingScreen(device,narrow_to_wide(gettext("Setting Up Sound")));

#if USE_AUDIO == 1
	sound_init();
#endif

	/*
		GUI stuff
	*/

	/*
		If an error occurs, this is set to something and the
		menu-game loop is restarted. It is then displayed before
		the menu.
	*/
	std::wstring error_message = L"";

	// The password entered during the menu screen,
	std::string password;

	// if there's no chardef then put the player directly into the character creator
	bool character_creator = true;
	{
		const char* v = config_get("client.character");
		if (v && v[0])
			character_creator = false;
	}

	/*
		Menu-game loop
	*/
	while (device->run() && kill == false) {

		// This is used for catching disconnects
		try{

			/*
				Clear everything from the GUIEnvironment
			*/
			guienv->clear();

			/*
				We need some kind of a root node to be able to add
				custom gui elements directly on the screen.
				Otherwise they won't be automatically drawn.
			*/
			guiroot = guienv->addStaticText(L"",core::rect<s32>(0, 0, 10000, 10000));

			/*
				Out-of-game menu loop.
				Loop quits when menu returns proper parameters.
				break out immediately if we should be in the character creator
			*/
			while (kill == false && character_creator == false) {
				// Cursor can be non-visible when coming from the game
				device->getCursorControl()->setVisible(true);
				// Some stuff are left to scene manager when coming from the game
				// (map at least?)
				smgr->clear();

				// Initialize menu data
				std::string playername = "";
				{
					const char* v = config_get("client.name");
					if (v)
						playername = std::string(v);
				}
				if (playername == "")
					playername = porting::getUser();
				MainMenuData menudata;
				menudata.address = L"";
				menudata.name = narrow_to_wide(playername);
				menudata.port = L"";
				menudata.game_mode = narrow_to_wide(bridge_config_get("world.game.mode"));
				menudata.max_mob_level = narrow_to_wide(bridge_config_get("world.game.mob.spawn.level"));
				menudata.initial_inventory = config_get_bool("world.player.inventory.starter");
				menudata.infinite_inventory = config_get_bool("world.player.inventory.creative");
				menudata.droppable_inventory = config_get_bool("world.player.inventory.droppable");
				menudata.death_drops_inventory = config_get_bool("world.player.inventory.keep");
				menudata.enable_damage = config_get_bool("world.player.damage");
				menudata.suffocation = config_get_bool("world.player.suffocation");
				menudata.hunger = config_get_bool("world.player.hunger");
				menudata.tool_wear = config_get_bool("world.player.tool.wear");
				menudata.unsafe_fire = config_get_bool("world.game.environment.fire.spread");
				menudata.delete_map = false;
				menudata.clear_map = false;
				menudata.use_fixed_seed = false;
				menudata.fixed_seed = narrow_to_wide(bridge_config_get("world.seed"));
				if (menudata.fixed_seed != L"")
					menudata.use_fixed_seed = true;
				menudata.map_type = config_get("world.map.type");

				GUIMainMenu* const menu = new GUIMainMenu(
					guienv,
					guiroot,
					-1,
					&g_menumgr,
					&menudata,
					g_gamecallback
				);
				menu->allowFocusRemoval(true);

				if (error_message != L"") {
					errorstream<<"error_message = "
							<<wide_to_narrow(error_message)<<std::endl;

					GUIMessageMenu* const menu2 =
							new GUIMessageMenu(guienv, guiroot, -1,
								&g_menumgr, error_message.c_str());
					menu2->drop();
					error_message = L"";
				}

				video::IVideoDriver* driver = device->getVideoDriver();

				infostream<<"Created main menu"<<std::endl;

#if USE_AUDIO == 1
				sound_play_music("bg-mainmenu",1.0,1);
#endif

				while (device->run() && kill == false) {
					if (menu->getStatus() == true)
						break;
					if (g_gamecallback->startgame_requested) {
						g_gamecallback->startgame_requested = false;
						menu->quitMenu();
						break;
					}

					driver->beginScene(true, true, video::SColor(255,170,230,255));

					drawMenuBackground(driver);

					guienv->drawAll();

					driver->endScene();

#if USE_AUDIO == 1
					sound_step(0.02,NULL,NULL,NULL);
#endif

					// On some computers framerate doesn't seem to be
					// automatically limited
					sleep_ms(25);
				}

#if USE_AUDIO == 1
				sound_stop_music(1.0);
#endif

				// Break out of menu-game loop to shut down cleanly
				if (device->run() == false || kill == true)
					break;

				infostream<<"Dropping main menu"<<std::endl;

				menu->drop(); // Don't use after.

				character_creator = menudata.character_creator;

				if (character_creator)
					break;
				g_gamecallback->startgame_requested = false;

				// Delete map if requested
				if (menudata.delete_map) {
					if (path_remove((char*)"world",NULL)) {
						error_message = L"Map deletion failed";
						continue;
					}
					if (menudata.use_fixed_seed)
						config_set("world.map.seed",(char*)wide_to_narrow(menudata.fixed_seed).c_str());
					config_set("world.map_type",(char*)menudata.map_type.c_str());
				}else if (menudata.clear_map) {
					if (path_remove((char*)"world",(char*)"map.sqlite")) {
						error_message = L"Map clearing failed";
						continue;
					}
				}

				playername = wide_to_narrow(menudata.name);

				password = translatePassword(playername, menudata.password);

				//infostream<<"Main: password hash: '"<<password<<"'"<<std::endl;

				config_set("world.server.address",(char*)wide_to_narrow(menudata.address).c_str());
				int newport = mywstoi(menudata.port);
				if (newport != 0)
					config_set_int("world.server.port",newport);
				config_set("world.game.mode", (char*)wide_to_narrow(menudata.game_mode).c_str());
				config_set("world.game.mob.spawn.level", (char*)wide_to_narrow(menudata.max_mob_level).c_str());
				config_set_int("world.player.inventory.starter", menudata.initial_inventory);
				config_set_int("world.player.inventory.creative", menudata.infinite_inventory);
				config_set_int("world.player.inventory.droppable", menudata.droppable_inventory);
				config_set_int("world.player.inventory.keep", menudata.death_drops_inventory);
				config_set_int("world.player.damage", menudata.enable_damage);
				config_set_int("world.player.suffocation", menudata.suffocation);
				config_set_int("world.player.hunger", menudata.hunger);
				config_set_int("world.player.tool.wear", menudata.tool_wear);
				config_set_int("world.game.environment.fire.spread", menudata.unsafe_fire);

				// Save settings
				config_set("client.name", (char*)playername.c_str());
				config_set("world.server.admin",(char*)playername.c_str());

				config_save(NULL,NULL,NULL);

				// Continue to game
				break;
			}

			// Break out of menu-game loop to shut down cleanly
			if (device->run() == false)
				break;

			if (character_creator) {
				character_creator = false;
				video::IVideoDriver* driver = device->getVideoDriver();

				GUICharDefMenu* const menu = new GUICharDefMenu(device, guienv, guiroot, -1, &g_menumgr);
				menu->allowFocusRemoval(true);

#if USE_AUDIO == 1
				sound_play_music("bg-charcreator",1.0,1);
#endif

				while (device->run() && kill == false) {
					if (menu->getStatus() == true)
						break;

					driver->beginScene(true, true, video::SColor(255,0,0,0));

					smgr->drawAll();
					guienv->drawAll();

					driver->endScene();

#if USE_AUDIO == 1
					sound_step(0.02,NULL,NULL,NULL);
#endif

					// On some computers framerate doesn't seem to be
					// automatically limited
					sleep_ms(25);
				}

#if USE_AUDIO == 1
				sound_stop_music(1.0);
#endif

				menu->drop();
				continue;
			}
			// Initialize mapnode again to enable changed graphics settings
			init_mapnode(device);

			/*
				Run game
			*/
			{
				const char* v = config_get("world.server.address");
				if (!v || !v[0]) {
					if (!world_init(NULL)) {
						the_game(
							kill,
							input,
							device,
							font,
							password,
							error_message
						);
						world_exit();
					}
				}else{
					the_game(
						kill,
						input,
						device,
						font,
						password,
						error_message
					);
					config_clear("world");
				}
			}
#if USE_AUDIO == 1
			sound_stop_effects(0);
#endif

		} //try
		catch(con::PeerNotFoundException &e)
		{
			errorstream<<"Connection error (timed out?)"<<std::endl;
			error_message = L"Connection error (timed out?)";
		}
		catch(SocketException &e)
		{
			errorstream<<"Socket error (port already in use?)"<<std::endl;
			error_message = L"Socket error (port already in use?)";
		}
#ifdef NDEBUG
		catch(std::exception &e)
		{
			std::string narrow_message = "Some exception, what()=\"";
			narrow_message += e.what();
			narrow_message += "\"";
			errorstream<<narrow_message<<std::endl;
			error_message = narrow_to_wide(narrow_message);
		}
#endif

	} // Menu-game loop

	config_save(NULL,NULL,NULL);

	delete input;

#if USE_AUDIO == 1
	sound_exit();
#endif

	/*
		In the end, delete the Irrlicht device.
	*/
	device->drop();

	END_DEBUG_EXCEPTION_HANDLER(errorstream)

	debugstreams_deinit();

	return 0;
}

//END
