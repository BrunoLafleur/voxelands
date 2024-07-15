/************************************************************************
* path.c
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
#include "file.h"
#include "path.h"
#include "list.h"

#ifdef WIN32
#include <windows.h>
#else
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <libgen.h>
#include <dirent.h>
#include <errno.h>
#include <ftw.h>
#endif
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static struct {
	char* cwd;		/* current working directory */
	char* data_custom;	/* set by config data_path */
	char* data_user;	/* ~/.local/share/voxelands */
	char* data_global;	/* /usr/share/voxelands */
	char* data;		/* ./data if it exists */
	char* world;		/* data_user + /worlds/ + world name */
	char* home;		/* ~/. */
	char* config;		/* ~/.config/voxelands */
	char* screenshot;	/* set by config screenshot_path */
} path = {
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};

static int path_check(char* base,const char* rel)
{
	int l;
	char path[2048];
#ifndef WIN32
	struct stat st;
#else
	DWORD atts;
#endif

	if (!rel) {
		if (base) {
			l = snprintf(path,2048,"%s",base);
		}else{
			return -1;
		}
	}else if (base) {
		l = snprintf(path,2048,"%s/%s",base,rel);
	}else{
		l = snprintf(path,2048,"%s",rel);
	}
	if (l >= 2048)
		return -1;

#ifndef WIN32
	if (stat(path,&st) != 0)
		return 0;
	if ((st.st_mode&S_IFMT) == S_IFDIR)
		return 2;
	if ((st.st_mode&S_IFMT) == S_IFREG)
		return 1;
#else
	atts = GetFileAttributes(path);
	if (atts == INVALID_FILE_ATTRIBUTES)
		return 0;
	if (atts == FILE_ATTRIBUTE_DIRECTORY)
		return 2;
	if (atts == FILE_ATTRIBUTE_NORMAL)
		return 1;
#endif

	return 0;
}

static char* path_set(char* base,const char* rel, char* buff, int size)
{
	int l;
	char path[2048];

	if (!base && !rel)
		return NULL;

	if (base) {
		if (rel) {
			l = snprintf(path,2048,"%s/%s",base,rel);
		}else{
			l = snprintf(path,2048,"%s",base);
		}
	}else{
		l = snprintf(path,2048,"%s",rel);
	}
	if (l >= 2048)
		return NULL;

	if (!buff)
		return strdup(path);

	if (size <= l)
		return NULL;

	if (!strcpy(buff,path))
		return NULL;

	return buff;
}

static int dir_create(char* p);
static int dir_create(char* path)
{
#ifdef WIN32
	if (CreateDirectory(path, NULL))
		return 0;
	if (GetLastError() == ERROR_ALREADY_EXISTS)
		return 0;
#else
	mode_t process_mask = umask(0);
	mode_t mode = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH;
	char* q;
	char* r = NULL;
	char* p = NULL;
	char* up = NULL;
	int ret = 1;
	if (!strcmp(path, ".") || !strcmp(path, "/")) {
		umask(process_mask);
		return 0;
	}

	if ((p = strdup(path)) == NULL) {
		umask(process_mask);
		return 1;
	}

	if ((q = strdup(path)) == NULL) {
		umask(process_mask);
		return 1;
	}

	if ((r = dirname(q)) == NULL)
		goto out;

	if ((up = strdup(r)) == NULL) {
		umask(process_mask);
		return 1;
	}

	errno = 0;
	if ((dir_create(up) == 1) && (errno != EEXIST))
		goto out;

	if ((mkdir(p, mode) == -1) && (errno != EEXIST)) {
		ret = 1;
	}else{
		ret = 0;
	}

out:
	umask(process_mask);
	if (up)
		free(up);
	free(q);
	free(p);
	return ret;
#endif
	return 1;
}

#ifndef WIN32
int unlink_cb(const char* fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
	return remove(fpath);
}
#endif

/* initialises the common paths */
int path_init()
{
	char buff[2048];

#ifndef WIN32

#ifdef DATA_PATH
	path.data_global = strdup(DATA_PATH);
#else
	path.data_global = strdup("/usr/games/voxelands");
#endif

	if (getcwd(buff,2048)) {
		path.cwd = strdup(buff);
	}else{
		path.cwd = strdup(".");
	}

	path.home = getenv("HOME");
	if (path.home) {
		path.home = strdup(path.home);
	}else{
		path.home = strdup(path.cwd);
	}

	path.data_user = getenv("XDG_DATA_HOME");
	if (path.data_user) {
		path.data_user = path_set(path.data_user,"voxelands",NULL,0);
	}else if (path.home) {
		path.data_user = path_set(path.home,".local/share/voxelands",NULL,0);
	}else{
		path.data_user = path_set(path.cwd,"data",NULL,0);
	}
	path_create(NULL,path.data_user);

	path.config = getenv("XDG_CONFIG_HOME");
	if (path.config) {
		path.config = path_set(path.config,"voxelands",NULL,0);
	}else if (path.home) {
		path.config = path_set(path.home,".config/voxelands",NULL,0);
	}else{
		path.config = strdup(path.cwd);
	}
	path_create(NULL,path.config);
#else
	/* TODO: windows, and mac? */
	DWORD buflen = 2048;
	int i;
	int k;
	DWORD len;
	// Find path of executable and set path_data relative to it
	len = GetModuleFileName(GetModuleHandle(NULL), buff, buflen);
	if (len >= buflen)
		return 1;

	for (i=0; i<len; i++) {
		if (buff[i] == '\\')
			buff[i] = '/';
	}
	for (k=0; k<2; k++) {
		for (i--; i>=0; i--) {
			if (buff[i] == '/')
				break;
		}
		buff[i] = 0;
	}

	path.cwd = strdup(buff);
	path.data_global = strdup(path.cwd);
	path.home = strdup(path.cwd);
	path.data_user = path_set(path.cwd,"data",NULL,0);
	path.config = strdup(path.cwd);
	path_create(NULL,path.config);

#endif

	if (snprintf(buff,2048,"%s/data",path.cwd) < 2048 && path_check(NULL,buff) == 2) {
		path.data = strdup(buff);
#ifndef WIN32
	}else{
		char* a = NULL;
		char* b = strstr(path.cwd,"/bin");
		while (b) {
			a = b;
			b = strstr(a+1,"/bin");
		}
		if (a) {
			*a = 0;
			if (snprintf(buff,2048,"%s/data",path.cwd) < 2048 && path_check(NULL,buff) == 2)
				path.data = strdup(buff);
			*a = '/';
		}
#endif
	}

	return 0;
}

/* frees all the paths */
void path_exit()
{
	if (path.cwd)
		free(path.cwd);
	path.cwd = NULL;

	if (path.data_custom)
		free(path.data_custom);
	path.data_custom = NULL;

	if (path.data_user)
		free(path.data_user);
	path.data_user = NULL;

	if (path.data_global)
		free(path.data_global);
	path.data_global = NULL;

	if (path.data)
		free(path.data);
	path.data = NULL;

	if (path.world)
		free(path.world);
	path.world = NULL;

	if (path.home)
		free(path.home);
	path.home = NULL;

	if (path.config)
		free(path.config);
	path.config = NULL;

	if (path.screenshot)
		free(path.screenshot);
	path.screenshot = NULL;
}

/* sets path.data_custom */
int path_custom_setter(const char* p)
{
	if (path.data_custom)
		free(path.data_custom);
	path.data_custom = NULL;

	if (p)
		path.data_custom = strdup(p);

	return 0;
}

/* sets path.screenshot */
int path_screenshot_setter(const char* p)
{
	if (path.screenshot)
		free(path.screenshot);
	path.screenshot = NULL;

	if (p)
		path.screenshot = strdup(p);

	return 0;
}

/* sets the world path to user_data + /worlds/ + p, creates the path if necessary */
int path_world_setter(const char* p)
{
	int c;
	char buff[2048];
	char* base = path.data_user;

	if (path.world)
		free(path.world);
	path.world = NULL;

	if (!p)
		return 1;

	if (p[0] == '/') {
		path.world = strdup(p);
	}else{
		if (snprintf(buff,2048,"worlds/%s",p) >= 2048)
			return 1;

		if (!base)
			base = path.data;
		if (!base)
			base = path.home;
		if (!base)
			base = path.cwd;
		if (!base)
			return 1;

		path.world = path_set(base,buff,NULL,0);
	}

	c = path_check(path.world,NULL);
	if (c == 2)
		return 0;
	if (c == 0)
		return dir_create(path.world);
	return 1;
}

/*
 * get the full path for a file/directory
 * type is the usual "texture" "model" etc
 * file is the file name
 * must_exist is pretty self explanatory
 * buff is a buffer to write the path into, if NULL allocate
 * size is the size of buff
 *
 * returns the path or NULL if either:
 *	must_exist is non-zero and the path doesn't exist
 *	buff is not NULL and too small to hold the full path
 */
char* path_get(const char* type,const char* file, int must_exist, char* buff, int size)
{
	char rel_path[1024];

	if (!file && !type)
		return NULL;

	if (file && file[0] == '/') {
		return path_set(NULL,file,buff,size);
	}else if (!type) {
		strcpy(rel_path,file);
	}else if (!strcmp(type,"world")) {
		if (path.world && (!must_exist || path_check(path.world,file)))
			return path_set(path.world,file,buff,size);
		return NULL;
	}else if (!strcmp(type,"player")) {
		int ck;
		if (!path.world)
			return NULL;
		ck = path_check(path.world,"players");
		if (ck == 1 || (!ck && must_exist))
			return NULL;

		if (file) {
			if (snprintf(rel_path,1024,"players/%s",file) >= 1024)
				return NULL;
		}else{
			strcpy(rel_path,"players");
		}

		return path_set(path.world,rel_path,buff,size);
	}else if (!strcmp(type,"worlds")) {
		char* base = path.data_user;
		if (!base)
			base = path.data;
		if (!base)
			base = path.home;
		if (!base)
			base = path.cwd;
		if (!base)
			return NULL;
		if (file) {
			if (snprintf(rel_path,1024,"worlds/%s",file) >= 1024)
				return NULL;
		}else{
			strcpy(rel_path,"worlds");
		}
		if (!must_exist || path_check(base,rel_path) == 2)
			return path_set(base,rel_path,buff,size);
		return NULL;
	}else if (!strcmp(type,"screenshot")) {
		if (path.screenshot) {
			return path_set(path.screenshot,file,buff,size);
		}else if (path.home) {
			return path_set(path.home,file,buff,size);
		}else if (path.data_user) {
			return path_set(path.data_user,file,buff,size);
		}else if (path.data_custom) {
			return path_set(path.data_custom,file,buff,size);
		}
		return NULL;
	}else if (!strcmp(type,"config")) {
		if (path.config && (!must_exist || path_check(path.config,file)))
			return path_set(path.config,file,buff,size);
		return NULL;
	}else if (!file) {
		return NULL;
	}else if (!strcmp(type,"model")) {
		snprintf(rel_path,1024,"models/%s",file);
	}else if (!strcmp(type,"texture")) {
		snprintf(rel_path,1024,"textures/%s",file);
	}else if (!strcmp(type,"shader")) {
		snprintf(rel_path,1024,"shaders/%s",file);
	}else if (!strcmp(type,"html")) {
		snprintf(rel_path,1024,"html/%s",file);
	}else if (!strcmp(type,"skin")) {
		snprintf(rel_path,1024,"textures/skins/%s",file);
	}else if (!strcmp(type,"sound")) {
		snprintf(rel_path,1024,"sounds/%s",file);
	}else if (!strcmp(type,"font")) {
		snprintf(rel_path,1024,"fonts/%s",file);
	}else if (!strncmp(type,"translation-",12)) {
		const char* lang = type+12;
		type = "translation";
		snprintf(rel_path,1024,"locale/%s/%s",lang,file);
	}else{
		strcpy(rel_path,file);
	}

	/* check from data_path */
	if (path.data_custom) {
		if (path_check(path.data_custom,rel_path))
			return path_set(path.data_custom,rel_path,buff,size);
	}

	/* check from user data directory */
	if (path.data_user) {
		if (path_check(path.data_user,rel_path))
			return path_set(path.data_user,rel_path,buff,size);
	}

	/* check from default data directory */
	if (path.data) {
		if (path_check(path.data,rel_path))
			return path_set(path.data,rel_path,buff,size);
	}

	/* check from global data directory */
	if (path.data_global) {
		if (path_check(path.data_global,rel_path))
			return path_set(path.data_global,rel_path,buff,size);
	}

	if (must_exist)
		return NULL;

	if (path.data)
		return path_set(path.data,rel_path,buff,size);
	if (path.data_user)
		return path_set(path.data_user,rel_path,buff,size);
	if (path.data_custom)
		return path_set(path.data_custom,rel_path,buff,size);

	return NULL;
}

/*
 * check if a path exists
 * returns:
 * 	-1 on error
 * 	0 if path does not exist
 * 	1 if path exists and is a file
 * 	2 if path exists and is a directory
 */
int path_exists(char* path)
{
	return path_check(NULL,path);
}

/*
 * create the full path for the type
 * assumes that files have a dot somewhere in their name
 * thus:
 * 	if file is NULL, creates the base path for the type
 * 	if file contains a dot, creates the base path for the type, and
 * 		an empty file along with any subdirectories
 * 	if file does not contain a dot, creates the base path for the
 * 		<type>/file
 *	if type is NULL, file must be an absolute path
 * returns 0 if successful
 */
int path_create(char* type, char* file)
{
	char path[2048];
	char* fn = NULL;
	FILE *f;

	if (!path_get(type,file,0,path,2048))
		return -1;

	if (file) {
		char* b = strrchr(file,'/');
		if (!b) {
			b = file;
		}else{
			b++;
		}
		fn = strchr(b,'.');
		if (fn) {
			fn = strrchr(path,'/');
			if (!fn)
				return -1;
			*fn = 0;
			fn++;
		}
	}

	if (dir_create(path))
		return -1;

	if (!fn)
		return 0;

	*fn = '/';

	f = fopen(path,"w");
	if (!f)
		return -1;

	fclose(f);

	return 0;
}

/* removes (recursively) the last node in the full path of <type>/file */
int path_remove(char* type, char* file)
{
	char path[2048];
	if (!path_get(type,file,1,path,2048))
		return 0;

#ifdef WIN32
	DWORD attributes = GetFileAttributes(path);

	/* delete if it's a file, or call recursive delete if a directory */
	if (attributes == INVALID_FILE_ATTRIBUTES)
		return -1;

	if (attributes == FILE_ATTRIBUTE_DIRECTORY) {
		char fpath[2048];
		dirlist_t *list;
		dirlist_t *n;
		int r = 0;
		list = path_dirlist(NULL,path);
		for (n=list; n != NULL && r == 0; n=n->next) {
			if (snprintf(fpath,2048,"%s/%s",path,n->name) >= 2048)
				r = -1;
			if (path_remove(NULL,fpath))
				r = -1;
		}
		while ((n = list_pop(&list))) {
			free(n->name);
			free(n);
		}
		if (r != 0)
			return r;
		if (RemoveDirectory(path))
			return 0;
	}else{
		if (DeleteFile(path))
			return 0;
	}
#else
	if (path[0] != '/' || path[1] == 0)
		return -1;
	/* file tree walk, calls the unlink_cb function on every file/directory */
	return nftw(path, unlink_cb, 64, FTW_DEPTH | FTW_PHYS);
#endif
	return -1;
}

dirlist_t *path_dirlist(char* type, char* file)
{
	dirlist_t *n;
	dirlist_t *list = NULL;
	char path[2048];
#ifdef WIN32
	dirlist_t nn;
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	DWORD dwError;
	LPTSTR DirSpec;
	INT retval;
#else
	DIR *dp;
	struct dirent *dirp;
#endif
	if (!path_get(type,file,1,path,2048))
		return NULL;
#ifdef WIN32
	DirSpec = malloc(MAX_PATH);

	if (!DirSpec) {
		retval = 1;
	}else if (strlen(path) > (MAX_PATH-2)) {
		retval = 3;
	}else{
		retval = 0;
		sprintf(DirSpec, "%s\\*", path);

		hFind = FindFirstFile(DirSpec, &FindFileData);

		if (hFind == INVALID_HANDLE_VALUE) {
			retval = -1;
		}else{
			nn.name = FindFileData.cFileName;
			nn.dir = FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
			if (n->name[0] != '.' || (n->name[1] && strcmp(n->name,".."))) {
				n = malloc(sizeof(dirlist_t));
				n->name = strdup(nn.name);
				n->dir = nn.dir;
				list = list_push(&list,n);
			}

			while (FindNextFile(hFind, &FindFileData) != 0) {
				nn.name = FindFileData.cFileName;
				nn.dir = FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
				if (n->name[0] != '.' || (n->name[1] && strcmp(n->name,".."))) {
					n = malloc(sizeof(dirlist_t));
					n->name = strdup(nn.name);
					n->dir = nn.dir;
					list = list_push(&list,n);
				}
			}

			dwError = GetLastError();
			FindClose(hFind);
			if (dwError != ERROR_NO_MORE_FILES)
				retval = -1;
		}
	}

	free(DirSpec);

	if (retval != 0 && list) {
		while ((n = list_pop(&list))) {
			free(n->name);
			free(n);
		}
	}
#else
	dp = opendir(path);
	if (!dp)
		return NULL;

	while ((dirp = readdir(dp)) != NULL) {
		if (dirp->d_name[0] == '.' && (!dirp->d_name[1] || !strcmp(dirp->d_name,"..")))
			continue;
		n = malloc(sizeof(dirlist_t));
		n->name = strdup(dirp->d_name);
		if (dirp->d_type == DT_DIR) {
			n->dir = 1;
		}else{
			n->dir = 0;
		}
		list = list_push(&list,n);
	}
	closedir(dp);
#endif
	return list;
}

void path_dirlist_free(dirlist_t *l)
{
	dirlist_t *w;
	if (!l)
		return;

	while ((w = list_pull(&l))) {
		if (w->name)
			free(w->name);
		free(w);
	}
}
