/************************************************************************
* Minetest-c55
* Copyright (C) 2011 celeron55, Perttu Ahola <celeron55@gmail.com>
*
* log.cpp
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

#include "log.h"

#include <map>
#include <list>
#include <sstream>
#include "threads.h"
#include "debug.h"
#include "gettime.h"
#include "jmutex.h"
#include "jmutexautolock.h"

std::list<ILogOutput*> log_outputs[LMT_NUM_VALUES];
std::map<threadid_t, std::string> log_threadnames;

void log_add_output(ILogOutput *out, enum LogMessageLevel lev)
{
	log_outputs[lev].push_back(out);
}

void log_add_output_maxlev(ILogOutput *out, enum LogMessageLevel lev)
{
	for(int i=0; i<=lev; i++)
		log_outputs[i].push_back(out);
}

void log_add_output_all_levs(ILogOutput *out)
{
	for(int i=0; i<LMT_NUM_VALUES; i++)
		log_outputs[i].push_back(out);
}

void log_register_thread(const std::string &name)
{
	threadid_t id = get_current_thread_id();
	log_threadnames[id] = name;
}

static std::string get_lev_string(enum LogMessageLevel lev)
{
	switch(lev){
	case LMT_ERROR:
		return "ERROR";
	case LMT_ACTION:
		return "ACTION";
	case LMT_INFO:
		return "INFO";
	case LMT_VERBOSE:
		return "VERBOSE";
	case LMT_NUM_VALUES:
		break;
	}
	return "(unknown level)";
}

static void log_printline(enum LogMessageLevel lev, const std::string &text)
{
	std::string threadname = "(unknown thread)";
	std::map<threadid_t, std::string>::const_iterator i;
	
	log_mutex.Lock();
	i = log_threadnames.find(get_current_thread_id());
	if(i != log_threadnames.end())
		threadname = i->second;
	log_mutex.Unlock();

	std::string levelname = get_lev_string(lev);
	std::ostringstream os(std::ios_base::binary);
	os<<getTimestamp()<<": "<<levelname<<"["<<threadname<<"]: "<<text;
	for(std::list<ILogOutput*>::iterator i = log_outputs[lev].begin();
			i != log_outputs[lev].end(); i++){
		ILogOutput *out = *i;
		out->printLog(os.str());
		out->printLog(lev, text);
	}
}

class Logbuf : public std::streambuf
{
public:
	Logbuf(enum LogMessageLevel lev):
			m_lev(lev),m_buf(),m_mutex()
	{
		m_mutex.Init();
		assert(m_mutex.IsInitialized());
	}

	~Logbuf()
	{
	}

	int overflow(int c)
	{
		bufchar(c);
		return c;
	}
	std::streamsize xsputn(const char *s, std::streamsize n)
	{
		for(int i=0; i<n; i++)
			bufchar(s[i]);
		return n;
	}

	void bufchar(char c)
	{
	    	JMutexAutoLock lock(m_mutex);
		if(c == '\n' || c == '\r'){
			if(m_buf != "")
			    log_printline(m_lev, m_buf);
			m_buf = "";
			return;
		}
		m_buf += c;
	}

private:
	enum LogMessageLevel m_lev;
	std::string m_buf;
	JMutex m_mutex;
};

JMutex log_mutex;
Logbuf errorbuf(LMT_ERROR);
Logbuf actionbuf(LMT_ACTION);
Logbuf infobuf(LMT_INFO);
Logbuf verbosebuf(LMT_VERBOSE);
std::ostream errorstream(&errorbuf);
std::ostream actionstream(&actionbuf);
std::ostream infostream(&infobuf);
std::ostream verbosestream(&verbosebuf);


