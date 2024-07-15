/************************************************************************
* auth.c
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
#include "auth.h"

#include "thread.h"
#include "nvp.h"
#include "file.h"
#include "path.h"

#include <stdlib.h>
#include <string.h>

static struct {
	char* file;
	mutex_t *mutex;
	nvp_t *data;
	char modified;
	char* privstr[PRIV_COUNT];
} auth = {
	NULL,
	NULL,
	NULL,
	0,
	{
		"build",
		"teleport",
		"settime",
		"privs",
		"server",
		"shout",
		"ban"
	}
};

/* convert privs into a human readable csv string */
int auth_privs2str(uint64_t privs, char* buff, int size)
{
	int i;
	int r = 0;
	int l;
	int o = 0;
	uint64_t p = 1;

	if (!buff || !size)
		return -1;

	buff[0] = 0;

	for (i=0; i<PRIV_COUNT; i++) {
		p = (1<<i);
		if ((privs&p) != p)
			continue;
		l = strlen(auth.privstr[i]);
		if ((o+l+2) >= size)
			continue;
		if (r) {
			strcpy(buff+o,",");
			o++;
		}
		strcpy(buff+o,auth.privstr[i]);
		o += l;
		r++;
	}

	return r;
}

/* convert a string to privs */
uint64_t auth_str2privs(const char* str)
{
	uint64_t privs = 0;
	char buff[256];
	int i;
	int j;
	int o = 0;
	char* n;

	if (!str)
		return 0;

	for (i=0; ; i++) {
		if (!str[i] || str[i] == ',') {
			buff[o] = 0;
			o = 0;
			n = trim(buff);
			for (j=0; j<PRIV_COUNT; j++) {
				if (strcmp(auth.privstr[j],n))
					continue;
				privs |= (1<<j);
				break;
			}
			if (j == PRIV_COUNT)
				return PRIV_INVALID;
			if (!str[i])
				break;
		}else if (o<255) {
			buff[o++] = str[i];
		}
	}

	return privs;
}

/* init auth system for the given file */
int auth_init(const char* file)
{
	char* path;
	if (!auth.mutex) {
		auth.mutex = mutex_create();
		if (!auth.mutex)
			return -1;
	}

	path = path_get("world",file,0,NULL,0);
	if (!path)
		return -1;

	if (auth.file) {
		if (!strcmp(auth.file,path)) {
			free(path);
			auth_load();
			return 0;
		}
		if (auth.file)
			free(auth.file);
		auth.file = NULL;

		nvp_free(&auth.data,1);
		auth.data = NULL;
	}

	auth.file = path;
	auth.modified = 0;

	auth_load();

	return 0;
}

/* free auth memory, reset auth struct */
void auth_exit()
{
	if (!auth.mutex)
		return;

	if (auth.modified)
		auth_save();

	if (auth.mutex)
		mutex_free(auth.mutex);
	auth.mutex = NULL;

	if (auth.file)
		free(auth.file);
	auth.file = NULL;

	if (auth.data)
		nvp_free(&auth.data,1);
	auth.data = NULL;

	auth.modified = 0;
}

/* load auth data from file */
void auth_load()
{
	file_t *f;
	char line[512];
	char* n;
	char* p;
	char* s;
	uint64_t privs;
	authdata_t *d;

	mutex_lock(auth.mutex);

	f = file_load(NULL,auth.file);
	if (!f) {
		mutex_unlock(auth.mutex);
		return;
	}

	while (file_readline(f,line,512) > 0) {

		n = line;
		p = strchr(n,':');
		if (!p)
			continue;
		*p = 0;
		p++;
		s = strchr(p,':');
		if (!s)
			continue;
		*s = 0;
		s++;

		privs = auth_str2privs(s);

		d = malloc(sizeof(authdata_t));
		if (!d)
			continue;

		strcpy(d->pwd,p);
		d->privs = privs;
		nvp_set(&auth.data,n,"",d);
	}

	mutex_unlock(auth.mutex);

	file_free(f);

	auth.modified = 0;
}

/* save auth data to file */
void auth_save()
{
	file_t *f;
	nvp_t *n;
	char s[512];

	if (!auth.modified)
		return;

	mutex_lock(auth.mutex);

	f = file_create(NULL,auth.file);
	if (!f) {
		mutex_unlock(auth.mutex);
		return;
	}

	for (n=auth.data; n; n = n->next) {
		if (!n->name || !n->name[0])
			continue;
		if (auth_privs2str(((authdata_t*)n->data)->privs,s,512) < 0)
			s[0] = 0;
		file_writef(f,"%s:%s:%s\n",n->name,((authdata_t*)n->data)->pwd,s);
	}

	file_flush(f);

	mutex_unlock(auth.mutex);

	file_free(f);

	auth.modified = 0;
}

int auth_exists(char* name)
{
	nvp_t *n;
	if (!name)
		return 0;

	mutex_lock(auth.mutex);

	n = nvp_get(&auth.data,name);

	mutex_unlock(auth.mutex);

	if (!n)
		return 0;
	return 1;
}

void auth_set(char* name, authdata_t data)
{
	nvp_t *n;
	if (!name)
		return;

	mutex_lock(auth.mutex);

	n = nvp_get(&auth.data,name);

	if (n) {
		strcpy(((authdata_t*)n->data)->pwd,data.pwd);
		((authdata_t*)n->data)->privs = data.privs;
	}else{
		authdata_t *d = malloc(sizeof(authdata_t));
		if (!d) {
			mutex_unlock(auth.mutex);
			return;
		}

		strcpy(d->pwd,data.pwd);
		d->privs = data.privs;
		nvp_set(&auth.data,name,"",d);
	}

	mutex_unlock(auth.mutex);

	auth.modified = 1;
}

void auth_add(char* name)
{
	authdata_t d;
	d.pwd[0] = 0;
	d.privs = PRIV_DEFAULT;

	auth_set(name,d);
}

int auth_getpwd(char* name, char buff[64])
{
	nvp_t *n;

	buff[0] = 0;

	if (!name)
		return -1;

	mutex_lock(auth.mutex);

	n = nvp_get(&auth.data,name);

	if (!n) {
		mutex_unlock(auth.mutex);
		return -1;
	}

	strcpy(buff,((authdata_t*)n->data)->pwd);

	mutex_unlock(auth.mutex);

	return 0;
}

void auth_setpwd(char* name, char* pwd)
{
	nvp_t *n;
	if (!name)
		return;

	mutex_lock(auth.mutex);

	n = nvp_get(&auth.data,name);

	if (n) {
		strcpy(((authdata_t*)n->data)->pwd,pwd);
	}else{
		authdata_t *d = malloc(sizeof(authdata_t));
		if (!d) {
			mutex_unlock(auth.mutex);
			return;
		}

		strcpy(d->pwd,pwd);
		d->privs = PRIV_DEFAULT;
		nvp_set(&auth.data,name,"",d);
	}

	mutex_unlock(auth.mutex);

	auth.modified = 1;
}

uint64_t auth_getprivs(char* name)
{
	nvp_t *n;
	uint64_t privs = PRIV_INVALID;

	if (!name)
		return PRIV_INVALID;

	mutex_lock(auth.mutex);

	n = nvp_get(&auth.data,name);

	if (!n) {
		mutex_unlock(auth.mutex);
		return PRIV_INVALID;
	}

	privs = ((authdata_t*)n->data)->privs;

	mutex_unlock(auth.mutex);

	return privs;
}

void auth_setprivs(char* name, uint64_t privs)
{
	nvp_t *n;
	if (!name)
		return;

	mutex_lock(auth.mutex);

	n = nvp_get(&auth.data,name);

	if (n) {
		((authdata_t*)n->data)->privs = privs;
	}else{
		authdata_t *d = malloc(sizeof(authdata_t));
		if (!d) {
			mutex_unlock(auth.mutex);
			return;
		}

		d->pwd[0] = 0;
		d->privs = privs;
		nvp_set(&auth.data,name,"",d);
	}

	mutex_unlock(auth.mutex);

	auth.modified = 1;
}
