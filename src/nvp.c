/************************************************************************
* nvp.c
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
#include "list.h"
#include "nvp.h"
#include "crypto.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/* comparison functions:
 * return 0 to insert
 * return 1 to insert later
 * return -1 to not insert at all
 */
static int nvp_insert_cmp(void *e1, void *e2)
{
	nvp_t *n1 = e1;
	nvp_t *n2 = e2;

	if (n2->h > n1->h)
		return 0;
	return 1;
}

/* free memory of an nvp list, if data is non-zero, data is also freed */
void nvp_free(nvp_t **list, int data)
{
	nvp_t *n;
	while ((n = list_pop(list))) {
		if (n->name)
			free(n->name);
		if (n->value)
			free(n->value);
		if (data && n->data)
			free(n->data);
	}
}

/* get a name/value pair */
nvp_t *nvp_get(nvp_t **list, char* name)
{
	nvp_t *c = *list;
	unsigned int h = hash(name);
	while (c) {
		if (c->h > h)
			return NULL;
		if (c->h == h && !strcmp(c->name,name))
			return c;
		c = c->next;
	}
	return NULL;
}

/* get the value of a name/value pair */
char* nvp_get_str(nvp_t **list, char* name)
{
	nvp_t *c = nvp_get(list,name);
	if (c)
		return c->value;
	return NULL;
}

/* get the value of a name/value pair as an int value */
int nvp_get_int(nvp_t **list, char* name)
{
	nvp_t *c = nvp_get(list,name);
	if (c)
		return strtol(c->value,NULL,10);
	return 0;
}

/* get the value of a name/value pair as a float value */
float nvp_get_float(nvp_t **list, char* name)
{
	nvp_t *c = nvp_get(list,name);
	if (c)
		return strtof(c->value,NULL);
	return 0.0;
}

/* get the value of a name/value pair as a boolean value */
int nvp_get_bool(nvp_t **list, char* name)
{
	nvp_t *c = nvp_get(list,name);
	if (c && (!strcmp(c->value,"1") || !strcmp(c->value,"true")))
		return 1;
	return 0;
}

/* get a name/value pair's data value */
void *nvp_get_data(nvp_t **list, char* name)
{
	nvp_t *c = nvp_get(list,name);
	if (c)
		return c->data;
	return NULL;
}

/* set the value of a name/value pair */
void nvp_set(nvp_t **list, char* name, char* value, void *data)
{
	nvp_t *c = *list;
	unsigned int h = hash(name);

	while (c) {
		if (c->h > h)
			break;
		if (c->h == h && !strcmp(c->name,name)) {
			if (c->value) {
				free(c->value);
				c->value = NULL;
			}
			if (value)
				c->value = strdup(value);
			if (data)
				c->data = data;
			return;
		}
		c = c->next;
	}

	if (!value && !data)
		return;

	c = malloc(sizeof(nvp_t));
	c->name = strdup(name);
	c->h = h;
	if (value) {
		c->value = strdup(value);
	}else{
		c->value = NULL;
	}
	c->data = data;

	*list = list_insert_cmp(list,c,nvp_insert_cmp);
}

/* set a name/value pair to an int value */
void nvp_set_int(nvp_t **list, char* name, int value)
{
	char str[20];
	sprintf(str,"%d",value);
	nvp_set(list,name,str,NULL);
}

/* set a name/value pair to a float value */
void nvp_set_float(nvp_t **list, char* name, float value)
{
	char str[20];
	sprintf(str,"%f",value);
	nvp_set(list,name,str,NULL);
}

/* set a name/value pair to a float value */
void nvp_set_v3t(nvp_t **list, char* name, v3_t *value)
{
	char str[128];
	sprintf(str,"(%f,%f,%f)",value->x,value->y,value->z);
	nvp_set(list,name,str,NULL);
}

/* parse a name=value string to an nvp list */
void nvp_from_str(nvp_t **list, char* str)
{
	char name[512];
	char value[512];
	uint8_t is_str = 0;
	uint8_t is_value = 0;
	int i;
	int o = 0;

	if (!str)
		return;

	for (i=0; str[i]; i++) {
		if (str[i] == '\r')
			continue;
		if (str[i] == '\n') {
			if (!is_value) {
				o = 0;
				continue;
			}
			is_value = 0;
			is_str = 0;
			value[o] = 0;
			o = 0;
			nvp_set(list,name,value,NULL);
			continue;
		}else if (is_value) {
			if (!o) {
				if (isspace(str[i]))
					continue;
				if (str[i] == '"') {
					is_str = 1;
					continue;
				}
			}

			if (is_str) {
				if (str[i] == '\\') {
					i++;
					if (!str[i] || str[i] == '\n') {
						is_value = 0;
						is_str = 0;
						value[o] = 0;
						o = 0;
						nvp_set(list,name,value,NULL);
						continue;
					}
				}else if (str[i] == '"') {
					is_value = 0;
					is_str = 0;
					value[o] = 0;
					o = 0;
					nvp_set(list,name,value,NULL);
					continue;
				}
			}

			if (o < 511)
				value[o++] = str[i];
			continue;
		}else if (str[i] == '=') {
			name[o] = 0;
			o = 0;
			is_value = 1;
			continue;
		}

		if (isspace(str[i]))
			continue;

		if (o < 511)
			name[o++] = str[i];
	}
}

/* write an nvp list to a name=value string */
int nvp_to_str(nvp_t **list, char* buff, int size)
{
	nvp_t *n;
	int r = 0;
	int ns;
	int vs;
	int i;

	if (!size)
		return -1;

	if (!list || !*list) {
		buff[0] = 0;
		return 0;
	}

	n = *list;

	while (n) {
		if (!n->name || !n->value) {
			n = n->next;
			continue;
		}

		ns = strlen(n->name);
		vs = strlen(n->value);

		if (ns+(vs*2)+6 > size-(r+1))
			return -2;

		for (i=0; i<ns; i++) {
			buff[r++] = n->name[i];
		}

		buff[r++] = ' ';
		buff[r++] = '=';
		buff[r++] = ' ';
		buff[r++] = '"';

		for (i=0; i<vs; i++) {
			if (n->value[i] == '"' || n->value[i] == '\\')
				buff[r++] = '\\';
			buff[r++] = n->value[i];
		}

		buff[r++] = '"';
		buff[r++] = '\n';

		n = n->next;
	}

	buff[r] = 0;

	return r;
}
