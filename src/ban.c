/************************************************************************
* ban.c
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
#include "ban.h"

#include "file.h"
#include "thread.h"
#include "nvp.h"
#include "list.h"
#include "path.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct {
	char* file;
	mutex_t *mutex;
	nvp_t *data;
	char modified;
} ban = {
	NULL,
	NULL,
	NULL,
	0
};

/* init ban system for the given file */
int ban_init(char* file)
{
	char* path;
	if (!ban.mutex) {
		ban.mutex = mutex_create();
		if (!ban.mutex)
			return -1;
	}

	path = path_get("world",file,0,NULL,0);
	if (!path)
		return -1;

	if (ban.file) {
		if (!strcmp(ban.file,path)) {
			free(path);
			ban_load();
			return 0;
		}
		if (ban.file)
			free(ban.file);
		ban.file = NULL;

		nvp_free(&ban.data,1);
		ban.data = NULL;
	}

	ban.file = path;
	ban.modified = 0;

	ban_load();

	return 0;
}

/* free ban memory, reset ban struct */
void ban_exit()
{
	if (!ban.mutex)
		return;

	if (ban.modified)
		ban_save();

	mutex_free(ban.mutex);
	ban.mutex = NULL;

	if (ban.file)
		free(ban.file);
	ban.file = NULL;

	nvp_free(&ban.data,1);
	ban.data = NULL;

	ban.modified = 0;
}

/* load ban data from file */
void ban_load()
{
	file_t *f;
	char line[512];
	char* n;
	char* p;

	mutex_lock(ban.mutex);

	f = file_load(NULL,ban.file);
	if (!f) {
		mutex_unlock(ban.mutex);
		return;
	}

	while (file_readline(f,line,512) > 0) {

		p = line;
		n = strchr(p,'|');
		if (!n)
			continue;
		*n = 0;
		n++;

		if (!p[0])
			continue;

		nvp_set(&ban.data,p,n,NULL);
	}

	mutex_unlock(ban.mutex);

	file_free(f);

	ban.modified = 0;
}

/* save ban data to file */
void ban_save()
{
	file_t *f;
	nvp_t *n;

	if (!ban.modified)
		return;

	mutex_lock(ban.mutex);

	f = file_create(NULL,ban.file);
	if (!f)
		return;

	for (n=ban.data; n; n = n->next) {
		if (!n->name || !n->name[0])
			continue;
		file_writef(f,"%s|%s\n",n->name,n->value);
	}

	file_flush(f);

	mutex_unlock(ban.mutex);

	file_free(f);

	ban.modified = 0;
}

/* check if an ip address is banned */
int ban_ipbanned(char* ip)
{
	nvp_t *n;
	mutex_lock(ban.mutex);

	n = nvp_get(&ban.data,ip);

	mutex_unlock(ban.mutex);

	if (n)
		return 1;
	return 0;
}

/* get the discription of a ban, NULL for all */
int ban_description(char* ip_or_name, char* buff, int size)
{
	nvp_t *n;
	int o = 0;
	if (!buff || !size)
		return -1;

	buff[0] = 0;

	if (!ip_or_name) {
		char b1[256];
		int c = 0;
		int k;

		mutex_lock(ban.mutex);

		for (n=ban.data; n; n=n->next) {
			k = snprintf(b1,256,"%s|%s",n->name,n->value);
			if (k >= 256)
				continue;
			if (k+o+3 >= size)
				break;
			if (c) {
				buff[o++] = ',';
				buff[o++] = ' ';
				buff[o] = 0;
			}
			if (!strcpy(buff+o,b1))
				break;
			o += k;
			c++;
		}

		mutex_unlock(ban.mutex);

		buff[o] = 0;

		return c;
	}

	mutex_lock(ban.mutex);

	n = nvp_get(&ban.data,ip_or_name);

	if (!n) {
		n = ban.data;
		while (n) {
			if (!strcmp(n->value,ip_or_name))
				break;
			n = n->next;
		}
	}

	mutex_unlock(ban.mutex);

	if (!n)
		return 0;

	o = snprintf(buff,size,"%s|%s",n->name,n->value);
	if (o >= size)
		return -1;

	return 1;
}

/* get the name for a given ip ban */
char* ban_ip2name(char* ip)
{
	char* n;

	mutex_lock(ban.mutex);

	n = nvp_get_str(&ban.data,ip);

	mutex_unlock(ban.mutex);

	return n;
}

/* add a new ban */
void ban_add(char* ip, char* name)
{
	mutex_lock(ban.mutex);

	nvp_set(&ban.data,ip,name,NULL);

	mutex_unlock(ban.mutex);
}

/* remove an existing ban */
void ban_remove(char* ip_or_name)
{
	nvp_t *n;
	mutex_lock(ban.mutex);

	n = nvp_get(&ban.data,ip_or_name);

	if (!n) {
		for (n=ban.data; n; n=n->next) {
			if (!strcmp(n->value,ip_or_name))
				break;
		}
		if (!n) {
			mutex_unlock(ban.mutex);
			return;
		}
	}

	ban.data = list_remove(&ban.data,n);

	mutex_unlock(ban.mutex);

	if (n->name)
		free(n->name);
	if (n->value)
		free(n->value);
	free(n);
}
