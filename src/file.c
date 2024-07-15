/************************************************************************
* file.c
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>

#define FILE_BUFF_STEP 1024

static int file_extend(file_t *file, int min)
{
	unsigned char* b;
	int inc = min;
	int ttl;
	if (min < FILE_BUFF_STEP)
		inc = FILE_BUFF_STEP;
	ttl = file->size+inc;

	b = realloc(file->data,ttl);
	if (!b)
		return 0;

	file->data = b;
	file->size = ttl;

	return inc;
}

/* load a file into memory */
file_t *file_load(char* type, char* name)
{
	file_t *ft;
	FILE *f;
	char* fn;
	char* path;

	if (type) {
		fn = name;
		path = path_get(type,name,1,NULL,0);
	}else{
		path = strdup(name);
		fn = strrchr(name,'/');
		if (!fn)
			return NULL;
		fn++;
	}

	if (!path)
		return NULL;

	f = fopen(path, "rb");
	if (!f) {
		free(path);
		return NULL;
	}

	ft = malloc(sizeof(file_t));
	if (!ft) {
		free(path);
		fclose(f);
		return NULL;
	}

	ft->pos = 0;
	ft->modified = 0;

	fseek(f, 0, SEEK_END);
	ft->len = ftell(f);
	fseek(f, 0, SEEK_SET);

	ft->size = ft->len+1;

	ft->data = malloc(ft->size);
	if (!ft->data) {
		free(path);
		fclose(f);
		free(ft);
		return NULL;
	}

	if (ft->len != fread(ft->data, 1, ft->len, f)) {
		free(path);
		fclose(f);
		free(ft->data);
		free(ft);
		return NULL;
	}

	/* this lets us do string operations on text files */
	ft->data[ft->len] = 0;

	fclose(f);

	ft->name = strdup(fn);
	ft->path = path;

	return ft;
}

/* load a file into memory */
file_t *file_create(char* type, char *name)
{
	file_t *ft;
	char* fn;
	char* path;

	if (name) {
		if (type) {
			fn = name;
			path = path_get(type,name,0,NULL,0);
		}else{
			path = strdup(name);
			fn = strrchr(name,'/');
			if (!fn)
				return NULL;
			fn++;
		}
	}else{
		fn = NULL;
		path = NULL;
	}

	ft = malloc(sizeof(file_t));
	if (!ft) {
		free(path);
		return NULL;
	}

	ft->pos = 0;
	ft->len = 0;
	ft->size = 0;
	ft->modified = 0;

	ft->data = NULL;

	if (fn)
		ft->name = strdup(fn);
	ft->path = path;

	return ft;
}

/* free a loaded file */
void file_free(file_t *file)
{
	if (!file)
		return;

	if (file->data)
		free(file->data);

	if (file->name)
		free(file->name);

	free(file);
}

/* flush file data to disk */
void file_flush(file_t *file)
{
	FILE *f;
	if (!file || !file->modified || !file->path)
		return;

	if (!path_exists(file->path)) {
		if (!path_create(NULL,file->path))
			return;
	}

	f = fopen(file->path, "wb");
	if (!f)
		return;

	if (file->len) {
		if (fwrite(file->data,1,file->len,f) != file->len) {
			fclose(f);
			return;
		}
	}

	fclose(f);
	file->modified = 0;
}

/* find the next occurance of value from offset in file, return it's position
 * relative to offset */
int file_find(file_t *file, int offset, unsigned char value)
{
	int r;
	if (!file)
		return -1;

	for (r=offset; r<file->len; r++) {
		if (file->data[r] == value)
			return (r-offset);
	}
	return -1;
}

/* find the next occurance of value from offset in file, return it's position
 * relative to offset */
int file_strfind(file_t *file, int offset, char* value)
{
	int r;
	char* v;
	if (!file || file->len <= offset)
		return -1;

	v = strstr((char*)(file->data+offset),value);
	if (!v)
		return -1;

	r = (v-(char*)file->data);
	if (r < 0)
		return -1;

	return (r-offset);
}

/* read from a file buffer to dst */
int file_read(file_t *file, void* dst, int size)
{
	int len;
	if (!file || !file->len)
		return -1;

	len = file->len - file->pos;
	if (size < len) {
		len = size;
	}

	if (len > 0) {
		memcpy(dst, file->data+file->pos, len);
		file->pos += len;
	}

	return len;
}

/* read a line from a file buffer to dst */
int file_readline(file_t *file, char* dst, int size)
{
	int len;
	char* b;
	char* e;
	if (!file || !file->len)
		return -1;
	if (file->pos >= file->len)
		return -1;

	b = (char*)(file->data+file->pos);

	e = strchr(b,'\n');
	if (e) {
		len = (e-b);
		file->pos += len+1;
	}else{
		len = strlen(b);
		file->pos += len;
	}

	if (len >= size)
		len = size-1;

	memcpy(dst, b, len);
	dst[len] = 0;

	if (file->pos > file->len)
		file->pos = file->len;

	return len;
}

/* read integer from a file buffer */
int file_read_int(file_t *file)
{
	int32_t r;
	file_read(file,&r,4);
	return r;
}

/* read short integer from a file buffer */
int16_t file_read_short(file_t *file)
{
	int16_t r;
	file_read(file,&r,2);
	return r;
}

/* read char from a file buffer */
char file_read_char(file_t *file)
{
	char r = 0;
	file_read(file,&r,1);
	return r;
}

/* read unsigned integer from a file buffer */
uint32_t file_read_uint(file_t *file)
{
	uint32_t r;
	file_read(file,&r,4);
	return r;
}

/* read float from a file buffer */
float file_read_float(file_t *file)
{
	float r;
	file_read(file,&r,4);
	return r;
}

/* seek to a position in a file buffer */
int file_seek(file_t *file, int offset, int origin)
{
	if (!file)
		return -1;

	switch (origin) {
	case SEEK_SET:
		if (file->len < offset)
			offset = file->len;

		file->pos = offset;
		break;
	case SEEK_CUR:
		offset += file->pos;
		if (file->len < offset)
			offset = file->len;

		file->pos = offset;
		break;
	case SEEK_END:
		file->pos = file->len;
		break;
	default:;
	};

	if (file->pos < 0)
		file->pos = 0;

	return 0;
}

/* get the position in a file buffer */
int file_tell(file_t *file)
{
	if (!file)
		return -1;

	return file->pos;
}

/* get the pointer to the current file position */
void *file_get(file_t *file)
{
	return file->data+file->pos;
}

/* write data to a file buffer */
int file_write(file_t *file, void *buff, int size)
{
	if (!file || !buff || size)
		return -1;

	if (file->size <= (file->pos+size+1)) {
		int inc = file->pos+size+2;
		if (file_extend(file,inc) < inc)
			return 0;
	}

	if (!memcpy(file->data+file->pos,buff,size)) {
		file->data[file->pos] = 0;
		return 0;
	}

	file->pos += size;
	if (file->pos >= file->len) {
		file->len = file->pos;
		file->data[file->pos] = 0;
	}
	file->modified = 1;

	return size;
}

/* write a formatted string to a file buffer (printf style) */
int file_writef(file_t *file, char* fmt, ...)
{
	va_list ap;
	int l;
	int s;
	if (!file)
		return -1;

	if (!file->size || !file->data) {
		if (file_extend(file,FILE_BUFF_STEP) <= 0)
			return 0;
	}

	/* basically "keep trying till it all fits" */
	while (1) {
		va_start(ap, fmt);
		s = (file->size-1)-file->pos;
		l = vsnprintf((char*)(file->data+file->pos), s, fmt, ap);
		va_end(ap);
		if (l < s)
			break;
		if (file_extend(file,s+FILE_BUFF_STEP) <= 0) {
			if (file->data)
				file->data[file->len] = 0;
			return 0;
		}
	}

	file->pos += l;
	if (file->pos >= file->len) {
		file->len = file->pos;
		file->data[file->pos] = 0;
	}
	file->modified = 1;

	return l;
}
