/************************************************************************
* world.c
* voxelands - 3d voxel world sandbox game
* Copyright (C) Lisa 'darkrose' Milne 2017 <lisa@ltmnet.com>
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
#include "list.h"

#include <string.h>

static int world_exists(char* name)
{
	char buff[2048];
	char nbuff[256];

	if (!name)
		return 0;

	if (str_sanitise(nbuff,256,name) < 1)
		return 0;

	if (!path_get("worlds",nbuff,1,buff,2048))
		return 0;

	return 1;
}

int world_create(char* name)
{
	char buff[2048];
	char nbuff[256];
	char nbuff1[256];
	int i;

	if (!name || !name[0])
		name = "New World";

	if (str_sanitise(nbuff,256,name) < 1)
		return 1;

	if (!path_get("worlds",nbuff,1,buff,2048)) {
		if (!path_get("worlds",nbuff,0,buff,2048))
			return 1;

		config_set("world.path",nbuff);
		config_set("world.name",name);
		config_set("world.version",VERSION_STRING);

		return path_create("world","players");
	}

	for (i=1; i<100; i++) {
		snprintf(nbuff1,256,"%s_%d",nbuff,i);
		if (!path_get("worlds",nbuff1,1,buff,2048)) {
			if (!path_get("worlds",nbuff1,0,buff,2048))
				return 1;

			config_set("world.path",nbuff1);
			snprintf(nbuff,256,"%s %d",name,i);
			config_set("world.name",nbuff);
			config_set("world.version",VERSION_STRING);

			return path_create("world","players");
		}
	}

	return 1;
}

int world_load(char* name)
{
	char buff[2048];
	char buff1[2048];
	char nbuff[256];
	char* v;

	config_clear("world");

	if (!name) {
#ifdef SERVER
		name = config_get("server.world");
#else
		name = config_get("client.world");
#endif
		if (!name)
			return 1;
	}

	if (snprintf(nbuff,256,"%s",name) >= 256)
		return 1;

	if (str_sanitise(buff,2048,nbuff) < 1)
		return 1;

	config_set("world.path",buff);
	config_set("world.name",nbuff);

	if (path_get("world","world.cfg",1,buff1,2048)) {
		config_load("world","world.cfg");
	}else{
		vlprintf(CN_WARN,"Unknown world config: using defaults");
		config_save("world","world","world.cfg");
	}

	v = config_get("world.path");
	/* world.path may have changed, if so then load world config from the new path */
	if (v && strcmp(v,buff))
		config_load("world","world.cfg");

	return 0;
}

/* imports a world from an absolute path, path = /path/to/world.cfg */
int world_import(char* path)
{
	char buff[2048];
	char pbuff[2048];
	char newp[256];
	char id[256];
	char* v;

	if (!path_exists(path))
		return 1;

	if (snprintf(buff,2048,"%s",path) >= 2048)
		return 1;

	v = strrchr(buff,'/');
	if (!v)
		return 1;

	*v = 0;

	config_set("world.path",buff);
	config_load("world","world.cfg");

	v = config_get("world.path");
	if (!v || !strcmp(v,buff) || !config_get("world.name")) {
		config_clear("world");
		return 1;
	}

	if (snprintf(id,256,"%s",v) >= 256) {
		config_clear("world");
		return 1;
	}

	if (snprintf(newp,256,"%s/world.cfg",id) >= 256) {
		config_clear("world");
		return 1;
	}

	if (path_get("worlds",newp,1,pbuff,2048)) {
		config_clear("world");
		return 1;
	}

	if (!path_get("worlds",newp,0,pbuff,2048)) {
		config_clear("world");
		return 1;
	}

	config_save("world",NULL,pbuff);
	config_clear("world");

	return world_load(id);
}

/* save the world data, then clear all world.* config */
void world_unload()
{
	config_save("world","world","world.cfg");
	config_clear("world");
}

/* initialise and/or create a world */
int world_init(char* name)
{
	char *v;

	if (!name) {
#ifdef SERVER
		name = config_get("server.world");
#else
		name = config_get("client.world");
#endif
		if (!name)
			return 1;
	}

	if (!world_exists(name)) {
		if (world_create(name))
			return 1;
	}

	if (world_load(name))
		return 1;

	v = config_get("world.name");

	config_set("world.version",VERSION_STRING);

#ifdef SERVER
	config_set("server.world",v);
#else
	config_set("client.world",v);
#endif

	/* TODO: init server/environment/etc */


	return 0;
}

/* shutdown, save, clear, and free the current world */
void world_exit()
{
	/* TODO: shutdown server/environment/etc */

	world_unload();
}

int8_t world_compatibility(char* version)
{
	return 1;
}

worldlist_t *world_list_get()
{
	dirlist_t *d;
	dirlist_t *e;
	worldlist_t *l = NULL;
	worldlist_t *w;
	char* n;
	char* v;
	char* m;

	d = path_dirlist("worlds",NULL);

	if (!d)
		return NULL;

	e = d;
	while (e) {
		world_load(e->name);
		n = config_get("world.name");
		v = config_get("world.version");
		m = config_get("world.game.mode");
		if (n) {
			w = malloc(sizeof(worldlist_t));
			if (w) {
				w->name = strdup(n);
				w->path = strdup(e->name);
				if (v) {
					w->version = strdup(v);
					w->compat = world_compatibility(w->version);
				}else{
					w->version = strdup(VERSION_STRING);
					w->compat = -1;
				}
				if (m) {
					w->mode = strdup(m);
				}else{
					w->mode = strdup("survival");
				}
				l = list_push(&l,w);
			}
		}
		config_clear("world");
		e = e->next;
	}

	path_dirlist_free(d);

	return l;
}

void world_list_free(worldlist_t *l)
{
	worldlist_t *w;
	if (!l)
		return;

	while ((w = list_pull(&l))) {
		if (w->name)
			free(w->name);
		if (w->path)
			free(w->path);
		if (w->mode)
			free(w->mode);
		if (w->version)
			free(w->version);
		free(w);
	}
}
