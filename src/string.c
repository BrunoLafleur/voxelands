/************************************************************************
* string.c
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
#define VLCRYPTO_EXPOSE_LIBRARY_FUNCTIONS
#include "crypto.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* remove whitespace from beginning and end of a string */
char* trim(char* str)
{
	int l;

	while (*str && isspace(*str)) {
		str++;
	}

	l = strlen(str);
	while (--l > -1 && (!str[l] || isspace(str[l]))) {
		str[l] = 0;
	}

	return str;
}

/* allocate memory and copy a string *
char* strdup(const char* str)
{
	char* r;
	int l;

	if (!str)
		return NULL;

	l = strlen(str);
	l++;

	r = malloc(l);
	if (!r)
		return NULL;

	if (!memcpy(r,str,l)) {
		free(r);
		return NULL;
	}

	return r;
}*/

/* append str to dest, only if there's room for all of it */
int strappend(char* dest, int size, char* str)
{
	int l;

	l = strlen(dest);
	l += strlen(str);

	if (l >= size)
		return 0;

	strcat(dest,str);

	return l;
}

/* makes a string safe for use as a file or directory name */
int str_sanitise(char* dest, int size, char* str)
{
	int o = 0;
	int i = 0;
	int lws = 0;

	for (; o<size; i++) {
		if (!str[i]) {
			dest[o] = 0;
			return o;
		}
		if (isalnum(str[i]) || str[i] == '-') {
			lws = 0;
			dest[o++] = str[i];
		}else if (!lws && o) {
			dest[o++] = '_';
			lws = 1;
		}
	}

	return -1;
}

/* parse a string into a bool true/false 1/0 */
int parse_bool(char* str)
{
	if (str) {
		if (!strcmp(str,"true"))
			return 1;
		if (!strcmp(str,"yes"))
			return 1;
		if (!strcmp(str,"on"))
			return 1;
		if (!strcmp(str,"enabled"))
			return 1;
		if (!strcmp(str,"active"))
			return 1;
		if (strtol(str,NULL,10))
			return 1;
	}

	return 0;
}

/* parse a string to a v3_t */
int str_tov3t(char* str, v3_t *v)
{
	char buff[256];
	char* b;
	char* s1;
	char* s2;
	char *e;

	if (!str)
		return 1;

	strncpy(buff,str,256);

	b = strchr(buff,'(');
	if (!b)
		return 1;
	b++;

	s1 = strchr(b,',');
	if (!s1)
		return 1;
	*s1 = 0;
	s1++;

	s2 = strchr(s1,',');
	if (!s2)
		return 1;
	*s2 = 0;
	s2++;

	e = strchr(s2,')');
	if (!e)
		return 1;
	*e = 0;

	v->x = strtof(b,NULL);
	v->y = strtof(s1,NULL);
	v->z = strtof(s2,NULL);

	return 0;
}

int str_topwd(char* name, char* pass, char* buff, int size)
{
	char b1[256];
	unsigned char b2[20];
	if (!buff || size < 28)
		return 1;

	if (!name || !name[0]) {
		buff[0] = 0;
		return 0;
	}

	if (!pass || !pass[0]) {
		buff[0] = 0;
		return 0;
	}

	if (snprintf(b1,256,"%s%s",name,pass) >= 256)
		return 1;

	if (sha1(b1,b2))
		return 1;

	if (!base64_lencode((char*)b2,20,buff,size))
		return 1;

	return 0;
}
