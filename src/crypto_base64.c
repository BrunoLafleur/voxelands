/************************************************************************
* crypto_base64.c
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

#include "crypto.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>

static const char* b64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/* encode three bytes using base64 */
static void s_base64_encode_triple(unsigned char triple[3], unsigned char result[4])
{
	int tripleValue;
	int i;
	int v;

	tripleValue = triple[0];
	tripleValue *= 256;
	tripleValue += triple[1];
	tripleValue *= 256;
	tripleValue += triple[2];

	for (i=0; i<4; i++) {
		v = tripleValue%64;
		if (v < 0) {
			v *= -1;
		}
		if (v > 63) {
			result[3-i] = '=';
		}else{
			result[3-i] = b64_chars[v];
		}
		tripleValue /= 64;
	}
}

/* get the value of a base64 encoding character */
static int s_base64_char_value(unsigned char c)
{
	if (c >= 'A' && c <= 'Z')
		return c-'A';
	if (c >= 'a' && c <= 'z')
		return c-'a'+26;
	if (c >= '0' && c <= '9')
		return c-'0'+2*26;
	if (c == '+')
		return 2*26+10;
	if (c == '/')
		return 2*26+11;
	return -1;
}

/* decode a 4 char base64 encoded byte triple */
static int s_base64_decode_triple(unsigned char quadruple[4], unsigned char *result)
{
	int i, triple_value, bytes_to_decode = 3, only_equals_yet = 1;
	int char_value[4];

	for (i=0; i<4; i++) {
		char_value[i] = s_base64_char_value(quadruple[i]);
	}

	/* check if the characters are valid */
	for (i=3; i>=0; i--) {
		if (char_value[i]<0) {
			if (only_equals_yet && quadruple[i]=='=') {
				/* we will ignore this character anyway, make it something
				* that does not break our calculations */
				char_value[i]=0;
				bytes_to_decode--;
				continue;
			}
			return 0;
		}
		/* after we got a real character, no other '=' are allowed anymore */
		only_equals_yet = 0;
	}

	/* if we got "====" as input, bytes_to_decode is -1 */
	if (bytes_to_decode < 0)
		bytes_to_decode = 0;

	/* make one big value out of the partial values */
	triple_value = char_value[0];
	triple_value *= 64;
	triple_value += char_value[1];
	triple_value *= 64;
	triple_value += char_value[2];
	triple_value *= 64;
	triple_value += char_value[3];

	/* break the big value into bytes */
	for (i=bytes_to_decode; i<3; i++) {
		triple_value /= 256;
	}

	for (i=bytes_to_decode-1; i>=0; i--) {
		result[i] = triple_value%256;
		triple_value /= 256;
	}

	return bytes_to_decode;
}

/* encode an array of bytes using base64 */
int base64_lencode(const char* source, size_t sourcelen, char *target, size_t targetlen)
{
	/* check if the result will fit in the target buffer */
	if ((sourcelen+2)/3*4 > targetlen-1)
		return 0;

	/* encode all full triples */
	while (sourcelen >= 3) {
		s_base64_encode_triple((unsigned char*)source, (unsigned char*)target);
		sourcelen -= 3;
		source += 3;
		target += 4;
	}

	/* encode the last one or two characters */
	if (sourcelen > 0) {
		unsigned char temp[3];
		memset(temp, 0, sizeof(temp));
		memcpy(temp, source, sourcelen);
		s_base64_encode_triple((unsigned char*)temp, (unsigned char*)target);
		target[3] = '=';
		if (sourcelen == 1)
			target[2] = '=';

		target += 4;
	}

	/* terminate the string */
	target[0] = 0;

	return 1;
}

/* decode base64 encoded data */
size_t base64_ldecode(const char* source, char *target, size_t targetlen)
{
	char *src, *tmpptr;
	char quadruple[4], tmpresult[3];
	int i, tmplen = 3;
	size_t converted = 0;

	/* concatinate '===' to the source to handle unpadded base64 data */
	src = malloc(strlen(source)+5);
	if (!src)
		return -1;
	strcpy(src, source);
	strcat(src, "====");
	tmpptr = src;

	/* convert as long as we get a full result */
	while (tmplen == 3) {
		/* get 4 characters to convert */
		for (i=0; i<4; i++) {
			/* skip invalid characters - we won't reach the end */
			while (*tmpptr != '=' && s_base64_char_value(*tmpptr)<0) {
				tmpptr++;
			}

			quadruple[i] = *(tmpptr++);
		}

		/* convert the characters */
		tmplen = s_base64_decode_triple((unsigned char*)quadruple, (unsigned char*)tmpresult);

		/* check if the fit in the result buffer */
		if (targetlen < tmplen) {
			free(src);
			return -1;
		}

		/* put the partial result in the result buffer */
		memcpy(target, tmpresult, tmplen);
		target += tmplen;
		targetlen -= tmplen;
		converted += tmplen;
	}

	free(src);
	return converted;
}
