/************************************************************************
* Minetest-c55
* Copyright (C) 2010 celeron55, Perttu Ahola <celeron55@gmail.com>
*
* servermain.cpp
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

#ifndef SERVER
	#ifdef _WIN32
		#pragma error ("For a server build, SERVER must be defined globally")
	#else
		#error "For a server build, SERVER must be defined globally"
	#endif
#endif

#ifdef NDEBUG
	// Disable unit tests
	#define ENABLE_TESTS 0
#else
	// Enable unit tests
	#define ENABLE_TESTS 1
#endif

#ifdef _MSC_VER
#pragma comment(lib, "jthread.lib")
#pragma comment(lib, "zlibwapi.lib")
#endif

#include <iostream>
#include <fstream>
#include <time.h>
#include <jmutexautolock.h>
#include <locale.h>
#include "common_irrlicht.h"
#include "debug.h"
#include "map.h"
#include "player.h"
#include "main.h"
#include "test.h"
#include "environment.h"
#include "server.h"
#include "serialization.h"
#include "constants.h"
#include "strfnd.h"
#include "porting.h"
#include "config.h"
#include "mineral.h"
#include "profiler.h"
#include "log.h"
// for the init functions
#include "content_craft.h"
#include "content_clothesitem.h"
#include "content_craftitem.h"
#include "content_toolitem.h"
#include "content_mob.h"
#include "http.h"
#include "thread.h"
#include "path.h"

// Global profiler
Profiler main_profiler;
Profiler *g_profiler = &main_profiler;

// A dummy thing
ITextureSource *g_texturesource = NULL;

/*
	Debug streams
*/

// Connection
std::ostream *dout_con_ptr = &dummyout;
std::ostream *derr_con_ptr = &verbosestream;

// Server
std::ostream *dout_server_ptr = &infostream;
std::ostream *derr_server_ptr = &errorstream;

// Client
std::ostream *dout_client_ptr = &infostream;
std::ostream *derr_client_ptr = &errorstream;

/*
	gettime.h implementation
*/

u32 getTimeMs()
{
	/*
		Use imprecise system calls directly (from porting.h)
	*/
	return porting::getTimeMs();
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

	log_add_output_maxlev(&main_stderr_log_out, LMT_ACTION);
	log_add_output_all_levs(&main_dstream_no_stderr_log_out);

	log_register_thread("main");

	// Set locale. This is for forcing '.' as the decimal point.
	std::locale::global(std::locale("C"));
	// This enables printing all characters in bitmap font
	setlocale(LC_CTYPE, "en_US");

	/*
		Low-level initialization
	*/

	bool disable_stderr = false;
#ifdef _WIN32
	disable_stderr = true;
#endif

	porting::signal_handler_init();
	bool &kill = *porting::signal_handler_killstatus();

	thread_init();
	path_init();

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

	try
	{

	/*
		Basic initialization
	*/

	// Initialize sockets
	sockets_init();
	atexit(sockets_cleanup);

	// Initialize random seed
	srand(time(0));
	mysrand(time(0));

	// Initialize stuff

	crafting::initCrafting();
	content_clothesitem_init();
	content_craftitem_init();
	content_toolitem_init();
	content_mob_init();
	init_mapnode();
	init_mineral();

	/*
		Check parameters
	*/

	std::cout<<std::endl<<std::endl;
	std::cout
	<<"___    ___                 __"<<std::endl
	<<"\\  \\  /  /______  __ ____ |  | ___   ____  ___  ______"<<std::endl
	<<" \\  \\/  /  _  \\ \\/ // __ \\|  |/ _ \\ /    \\|   \\/  ___/"<<std::endl
	<<"  \\    /| |_| |    \\  ___/|  |  _  \\   |  \\ |  |___ \\"<<std::endl
	<<"   \\  / \\___  >_/\\  > __  >__|_/ \\  >__|  /___/____  >"<<std::endl
	<<"    \\/      \\/    \\/    \\/        \\/    \\/         \\/"<<std::endl
	<<std::endl;

	std::cout<<std::endl;

	world_init(NULL);

	// Create server
	Server server;
	server.start();
	HTTPServer http_server(server);
	if (config_get_bool("server.net.http"))
		http_server.start();

	// Run server
	dedicated_server_loop(server, kill);
	http_server.stop();
	world_exit();

	} //try
	catch(con::PeerNotFoundException &e)
	{
		errorstream<<"Connection timed out."<<std::endl;
	}

	END_DEBUG_EXCEPTION_HANDLER(errorstream)

	debugstreams_deinit();

	return 0;
}

//END
