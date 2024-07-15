/************************************************************************
* content_list.cpp
* voxelands - 3d voxel world sandbox game
* Copyright (C) Lisa Milne 2014-2017 <lisa@ltmnet.com>
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

#include <stdlib.h>
#include <string.h>

#include "content_list.h"
#include "list.h"
#include "crypto.h"

static struct {
	contentlist_t *lists;
} content_list_data = {
	NULL
};

static int content_list_insert_cmp(void *e1, void *e2)
{
	contentlist_t *n1 = e1;
	contentlist_t *n2 = e2;

	if (n2->h > n1->h)
		return 0;
	return 1;
}

static listdata_t *content_list_find(contentlist_t *list, content_t c, uint16_t data)
{
	listdata_t *d;

	d = list->data;
	while (d) {
		if (d->content == c && d->data == data)
			return d;
		d = d->next;
	}

	return NULL;
}

static contentlist_t *content_list_create(const char* name)
{
	contentlist_t *l;

	l = malloc(sizeof(contentlist_t));

	if (!l)
		return NULL;

	l->name = strdup(name);
	l->h = hash(name);
	l->data = NULL;
	l->count = 0;

	content_list_data.lists = list_insert_cmp(&content_list_data.lists,l,content_list_insert_cmp);

	return l;
}

static void content_list_append(contentlist_t *list, content_t c, uint16_t count, uint16_t data)
{
	listdata_t *d;

	d = malloc(sizeof(listdata_t));
	if (!d)
		return;

	d->content = c;
	d->count = count;
	d->data = data;

	list->data = list_push(&list->data,d);
}

void content_list_add(const char* name, content_t c, uint16_t count, uint16_t data)
{
	contentlist_t *l;
	listdata_t *d;

	l = content_list_get(name);
	if (!l)
		l = content_list_create(name);
	if (!l)
		return;

	d = content_list_find(l,c,data);
	if (!d) {
		content_list_append(l,c,count,data);
		return;
	}

	d->count = count;
}

contentlist_t *content_list_get(const char* name)
{
	contentlist_t *l;
	uint32_t h;

	l = content_list_data.lists;
	h = hash(name);

	while (l) {
		if (l->h == h && !strcmp(l->name,name))
			return l;
		l = l->next;
	}

	return NULL;
}
