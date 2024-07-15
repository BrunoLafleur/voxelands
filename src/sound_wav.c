/************************************************************************
* sound_wav.c
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

#include <string.h>

typedef struct wav_file_header_s {
	char riff[4]; /* 'RIFF' */
        int size;
	char wave[4]; /* 'WAVE' */
	char fmt[4]; /* 'fmt ' */
	int f_len;
	short int f_tag;
} __attribute__((packed)) wav_file_header_t;

typedef struct wav_format_header_s {
	short int channels;
	int freq;
	int bytes_per_second;
	short int block_align;
	short int bps;
	char data[4]; /* 'data' */
	int d_len;
} __attribute__((packed)) wav_format_header_t;

/* is it a wav file? */
int sound_is_wav(file_t *f)
{
	int r = 0;
	wav_file_header_t h;

	file_read(f,&h,sizeof(wav_file_header_t));

	if (!strncmp("RIFF",h.riff,4) && !strncmp("WAVE",h.wave,4))
		r = 1;

	file_seek(f,0,SEEK_SET);

	return r;
}

/* load a wav file */
int sound_load_wav(file_t *f, sound_t *e)
{
	wav_file_header_t h;
	wav_format_header_t fmt;
	unsigned char* data;

	file_read(f,&h,sizeof(wav_file_header_t));
	file_read(f,&fmt,sizeof(wav_format_header_t));
	data = file_get(f);

	if (h.f_tag != 1) {
		return 1;
	}else if (fmt.bps == 8) {
		if (fmt.channels == 1) {
			e->format = AL_FORMAT_MONO8;
		}else{
			e->format = AL_FORMAT_STEREO8;
		}
	}else if (fmt.bps == 16) {
		if (fmt.channels == 1) {
			e->format = AL_FORMAT_MONO16;
		}else{
			e->format = AL_FORMAT_STEREO16;
		}
	}else{
		return 1;
	}

	e->freq = fmt.freq;

	e->d_len = fmt.d_len;
	e->data = malloc(e->d_len);

	memcpy(e->data,data,e->d_len);

	alGenBuffers(1, &e->id);
	alBufferData(e->id, e->format, e->data, e->d_len, e->freq);


	if (alGetError() != AL_NO_ERROR)
		return 1;

	return 0;
}
