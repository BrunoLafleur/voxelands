/************************************************************************
* Minetest-c55
* Copyright (C) 2010 celeron55, Perttu Ahola <celeron55@gmail.com>
*
* porting.cpp
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

/*
	Random portability stuff

	See comments in porting.h
*/

#include "porting.h"
#include "config.h"
#include "debug.h"
#include "player.h"
#include "errno.h"

#ifdef __APPLE__
	#include "CoreFoundation/CoreFoundation.h"
#endif

namespace porting
{

/*
	Signal handler (grabs Ctrl-C on POSIX systems)
*/

bool g_killed = false;

bool * signal_handler_killstatus(void)
{
	return &g_killed;
}

#if !defined(_WIN32) // POSIX
	#include <signal.h>

void sigint_handler(int sig)
{
	if(g_killed == false)
	{
		dstream<<DTIME<<"INFO: sigint_handler(): "
				<<"Ctrl-C pressed, shutting down."<<std::endl;

		dstream<<DTIME<<"INFO: sigint_handler(): "
				<<"Printing debug stacks"<<std::endl;
		debug_stacks_print();

		g_killed = true;
	}
	else
	{
		(void)signal(SIGINT, SIG_DFL);
	}
}

void signal_handler_init(void)
{
	dstream<<"signal_handler_init()"<<std::endl;
	(void)signal(SIGINT, sigint_handler);
}

#else // _WIN32
	#include <signal.h>
	#include <windows.h>

BOOL WINAPI event_handler(DWORD sig)
{
	switch (sig) {
	case CTRL_C_EVENT:
	case CTRL_CLOSE_EVENT:
	case CTRL_LOGOFF_EVENT:
	case CTRL_SHUTDOWN_EVENT:
		if (g_killed == false) {
			dstream<<DTIME<<"INFO: event_handler(): "
					<<"Ctrl+C, Close Event, Logoff Event or Shutdown Event, shutting down."<<std::endl;
			dstream<<DTIME<<"INFO: event_handler(): "
					<<"Printing debug stacks"<<std::endl;
			debug_stacks_print();

			g_killed = true;
		}else{
			(void)signal(SIGINT, SIG_DFL);
		}
		break;
	case CTRL_BREAK_EVENT:
		break;
	}

	return TRUE;
}

void signal_handler_init(void)
{
	dstream<<"signal_handler_init()"<<std::endl;
	SetConsoleCtrlHandler( (PHANDLER_ROUTINE)event_handler,TRUE);
}

#endif

std::string getUser()
{
	std::string user("someone");
#ifdef _WIN32

	char buff[1024];
	int size = 1024;

	if (GetUserName(buff,LPDWORD(&size)))
		user = std::string(buff);
#else
	char* u = getenv("USER");
	if (u)
		user = std::string(u);
#endif
	return string_allowify(user,PLAYERNAME_ALLOWED_CHARS);
}

} //namespace porting

