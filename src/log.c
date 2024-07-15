/************************************************************************
* log.c
* voxelands - 3d voxel world sandbox game
* Copyright (C) Lisa 'darkrose' Milne 2016 <lisa@ltmnet.com>
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
************************************************************************/

#include "common.h"
#include "path.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

static struct {
	int min_level;
	int max_level;
	int system_min_level;
	int system_max_level;
	int console_min_level;
	int console_max_level;
	char* logfile;
	char* levels[7];
} logdata = {
	1,
	5,
	4,
	5,
	4,
	5,
	NULL,
	{"none","error","warn","action","chat","info","debug"}
};

static void level_setter(const char* v, int *l, int d)
{
	int i;
	if (v) {
		for (i=0; i<7; i++) {
			if (!strcmp(logdata.levels[i],v)) {
				*l = i;
				return;
			}
		}
	}

	*l = d;
}

/* config setters */
int log_minlevel_setter(const char* v)
{
	level_setter(v,&logdata.min_level,1);
	return 0;
}
int log_maxlevel_setter(const char* v)
{
	level_setter(v,&logdata.max_level,5);
	return 0;
}
int log_sminlevel_setter(const char* v)
{
	level_setter(v,&logdata.system_min_level,4);
	return 0;
}
int log_smaxlevel_setter(const char* v)
{
	level_setter(v,&logdata.system_max_level,5);
	return 0;
}
int log_cminlevel_setter(const char* v)
{
	level_setter(v,&logdata.console_min_level,4);
	return 0;
}
int log_cmaxlevel_setter(const char* v)
{
	level_setter(v,&logdata.console_max_level,5);
	return 0;
}
int log_file_setter(const char* v)
{
	if (logdata.logfile)
		free(logdata.logfile);
	logdata.logfile = NULL;

	if (!v)
		return 0;

	logdata.logfile = path_get(NULL,v,0,NULL,0);

	return 0;
}

/* print text to game and system consoles */
void vlprint(uint8_t type,const char* str)
{
	char buff[1024];
	char* b = buff;
	int s = 1024;

	if (!str)
		return;

	/* if it's not logged, don't process it */
	if (
		!type
		|| (
			(type < logdata.min_level || type > logdata.max_level)
			&& (type < logdata.system_min_level || type > logdata.system_max_level)
			&& (type < logdata.console_min_level || type > logdata.console_max_level)
		)
	)
		return;

	switch (type) {
	case 1:
		strcpy(buff,"ERROR: ");
		b += 7;
		s -= 7;
		break;
	case 2:
		strcpy(buff,"WARNING: ");
		b += 9;
		s -= 9;
		break;
	case 3:
		strcpy(buff,"ACTION: ");
		b += 8;
		s -= 8;
		break;
	case 4:
		strcpy(buff,"CHAT: ");
		b += 6;
		s -= 6;
		break;
	case 6:
		strcpy(buff,"DEBUG: ");
		b += 7;
		s -= 7;
		break;
	default:;
	}

	if (snprintf(b,s,"%s",str) >= s)
		return;

	/* TODO: log to system console */
	if (type >= logdata.system_min_level && type <= logdata.system_max_level)
		printf("%s\n",buff);
		/*sys_console_print(buff,1);*/
	/* TODO: log to game console */
	/*if (type >= logdata.console_min_level && type <= logdata.console_max_level)
		console_put(buff);*/

	if (type < logdata.min_level || type > logdata.max_level)
		return;

	if (logdata.logfile) {
		/* the only time raw file calls are used, for speed */
		FILE *f = fopen(logdata.logfile,"a");
		if (!f)
			return;

		fputs(buff,f);
		fclose(f);
	}
}

/* print formatted text to game and system consoles */
void vlprintf(uint8_t type,const char* fmt,...)
{
	char buff[1024];
	va_list ap;

	if (!fmt)
		return;

	/* if it's not logged, don't process it */
	if (
		!type
		|| (
			(type < logdata.min_level || type > logdata.max_level)
			&& (type < logdata.system_min_level || type > logdata.system_max_level)
			&& (type < logdata.console_min_level || type > logdata.console_max_level)
		)
	)
		return;

	va_start(ap, fmt);
	if (vsnprintf(buff, 1024, fmt, ap) >= 1024) {
		va_end(ap);
		return;
	}

	va_end(ap);

	vlprint(type,buff);
}
