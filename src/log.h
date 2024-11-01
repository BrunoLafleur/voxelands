/************************************************************************
* Minetest-c55
* Copyright (C) 2011 celeron55, Perttu Ahola <celeron55@gmail.com>
*
* log.h
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

#ifndef LOG_HEADER
#define LOG_HEADER

#include <string>
#include "jmutex.h"

/*
	Use this for logging everything.

	If you need to explicitly print something, use dstream or cout or cerr.
*/

enum LogMessageLevel {
	LMT_ERROR, /* Something failed ("invalid map data on disk, block (2,2,1)") */
	LMT_ACTION, /* In-game actions ("celeron55 placed block at (12,4,-5)") */
	LMT_INFO, /* More deep info ("saving map on disk (only_modified=true)") */
	LMT_VERBOSE, /* Flood-style ("loaded block (2,2,2) from disk") */
	LMT_NUM_VALUES,
};

class ILogOutput
{
public:
	virtual ~ILogOutput() {};
	
	/* line: Full line with timestamp, level and thread */
	virtual void printLog(const std::string &line){};
	/* line: Only actual printed text */
	virtual void printLog(enum LogMessageLevel lev, const std::string &line){};
};

void log_add_output(ILogOutput *out, enum LogMessageLevel lev);
void log_add_output_maxlev(ILogOutput *out, enum LogMessageLevel lev);
void log_add_output_all_levs(ILogOutput *out);

void log_register_thread(const std::string &name);

extern std::ostream errorstream;
extern std::ostream actionstream;
extern std::ostream infostream;
extern std::ostream verbosestream;
extern jthread::JMutex log_mutex;

#endif

