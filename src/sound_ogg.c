/************************************************************************
* sound_ogg.c
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
#include "sound.h"
#include "file.h"

#include <stdlib.h>
#include <string.h>

#include <vorbis/vorbisfile.h>

static size_t sound_ogg_read(void* dst, size_t byte_size, size_t l, void* src)
{
	return file_read(src,dst,(l*byte_size));
}

static int sound_ogg_seek(void* src, ogg_int64_t offset, int origin)
{
	return file_seek(src,offset,origin);
}

static int sound_ogg_close(void* src)
{
	return 0;
}

static long sound_ogg_tell(void* src)
{
	return file_tell(src);
}

/* is it an ogg? */
int sound_is_ogg(file_t *f)
{
	int r;
	OggVorbis_File oggfile;
	ov_callbacks cb;

	cb.read_func = sound_ogg_read;
	cb.close_func = sound_ogg_close;
	cb.seek_func = sound_ogg_seek;
	cb.tell_func = sound_ogg_tell;

	r = !ov_test_callbacks(f, &oggfile, NULL, 0, cb);

	if (r)
		ov_clear(&oggfile);

	file_seek(f,0,SEEK_SET);

	return r;
}

/* load an ogg file */
int sound_load_ogg(file_t *f, sound_t *e)
{
	int endian = 0;
	int bitStream;
	int s;
	char buff[30000];
	vorbis_info *ogginfo;
	OggVorbis_File oggfile;
	ov_callbacks cb;

	cb.read_func = sound_ogg_read;
	cb.close_func = sound_ogg_close;
	cb.seek_func = sound_ogg_seek;
	cb.tell_func = sound_ogg_tell;

	if (ov_open_callbacks(f, &oggfile, NULL, 0, cb))
		return 1;

 	ogginfo = ov_info(&oggfile, -1);

	/* always use 16-bit samples */
	if (ogginfo->channels == 1) {
		e->format = AL_FORMAT_MONO16;
	}else{
		e->format = AL_FORMAT_STEREO16;
	}

	e->freq = ogginfo->rate;

	do{
		s = ov_read(&oggfile, buff, 30000, endian, 2, 1, &bitStream);
		if (s < 0) {
			ov_clear(&oggfile);
			return 1;
		}

		e->data = realloc(e->data,e->d_len+s);
		memcpy(e->data+e->d_len,buff,s);
		e->d_len += s;
	} while (s > 0);

	alGenBuffers(1, &e->id);
	alBufferData(e->id, e->format, e->data, e->d_len, e->freq);


	if (alGetError() != AL_NO_ERROR)
		return 1;

	ov_clear(&oggfile);

	return 0;
}
