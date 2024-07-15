/************************************************************************
* crypto.c
* tausaga - environmental survival game
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

uint32_t hash(const char *str_param)
{
	uint32_t hval = 0;
	uint32_t g;
	const char *s = str_param;

	while (*s) {
		hval <<= 4;
		hval += (unsigned char) *s++;
		g = hval & ((uint32_t) 0xf << 28);
		if (g != 0) {
			hval ^= g >> 24;
			hval ^= g;
		}
	}

	return hval;
}

/* base64 encode a string */
char* base64_encode(const char* str)
{
	int sl;
	int tl;
	char* ret;
	int el;
	if (!str)
		return NULL;

	sl = strlen(str);
	tl = ((sl+2)/3*4)+5;

	ret = malloc(tl);
	if (!ret)
		return NULL;

	el = base64_lencode(str, sl, ret, tl);
	if (!el) {
		free(ret);
		return NULL;
	}

	return ret;
}

/* decode a base64 string */
char* base64_decode(const char* str)
{
	int sl;
	int tl;
	char* ret;
	int dl;
	if (!str)
		return NULL;

	sl = strlen(str);
	tl = (sl/4*3)+5;
	ret = malloc(tl);
	if (!ret)
		return NULL;

	dl = base64_ldecode(str, ret, tl);
	if (!dl) {
		free(ret);
		return NULL;
	}
	ret[dl] = 0;

	return ret;
}

int sha1(char* str, unsigned char buff[20])
{
	unsigned char* digest;
	if (!str)
		return 1;

	digest = bridge_sha1(str);
	if (!digest)
		return 1;

	memcpy(buff,digest,20);

	free(digest);

	return 0;
}
