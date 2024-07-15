/************************************************************************
* sound_mumble.c
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
#define _VL_SOUND_EXPOSE_INTERNAL
#include "sound.h"

#ifndef WIN32
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <wchar.h>
#endif

typedef struct linkedmem_s {
#ifdef WIN32
	UINT32 uiVersion;
	DWORD uiTick;
#else
	uint32_t uiVersion;
	uint32_t uiTick;
#endif
	float fAvatarPosition[3];
	float fAvatarFront[3];
	float fAvatarTop[3];
	wchar_t name[256];
	float fCameraPosition[3];
	float fCameraFront[3];
	float fCameraTop[3];
	wchar_t	identity[256];
#ifdef WIN32
	UINT32 context_len;
#else
	uint32_t context_len;
#endif
	unsigned char context[256];
	wchar_t description[2048];
} linkedmem_t;

static struct {
	float dtime_cumulative;
	linkedmem_t *link;
} mumble_data = {
	0.0,
	NULL
};

void sound_mumble_init()
{
#ifdef WIN32
	HANDLE hMapObject = OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, L"MumbleLink");
	if (hMapObject == NULL)
		return;

	mumble_data.link = (linkedmem_t*)MapViewOfFile(hMapObject, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(linkedmem_t));
	if (!mumble_data.link)
		return;

	CloseHandle(hMapObject);
	hMapObject = NULL;
#else
	char memname[256];
	int shmfd = -1;

	snprintf(memname, 256, "/MumbleLink.%d", getuid());

	shmfd = shm_open(memname, O_RDWR, S_IRUSR | S_IWUSR);
	if (shmfd < 0)
		return;

	mumble_data.link = (linkedmem_t*)(mmap(NULL, sizeof(linkedmem_t), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd,0));
	if (mumble_data.link == (void*)(-1)) {
		mumble_data.link = NULL;
		return;
	}
#endif

	sound_mumble_set_ident("singleplayer");
	sound_mumble_set_context(NULL,0);
}

int sound_mumble_set_ident(char* id)
{
	wchar_t ident[256];
	int wl;
	int l;
	if (!mumble_data.link || !id)
		return 0;

	l = strlen(id);
	if (l>255)
		l = 255;

	wl = mbstowcs(ident, id, l);
	if (wl < 0)
		return 1;

	ident[l] = 0;

	wcsncpy(mumble_data.link->identity, ident, 256);

	return 0;
}

void sound_mumble_set_context(char* ctx, int len)
{
	if (!mumble_data.link)
		return;
	/* Context should be equal for players which should be able to
	 * hear each other positional and differ for those who shouldn't
	 * (e.g. it could contain the server+port and team) */
	if (!ctx || len < 1) {
		memcpy(mumble_data.link->context, "VoxelandsPlayer", 15);
		mumble_data.link->context_len = 15;
		return;
	}
	memcpy(mumble_data.link->context, ctx, len);
	mumble_data.link->context_len = len;
}

void sound_mumble_step(float dtime, v3_t *pos, v3_t *at, v3_t *up)
{
	if (!mumble_data.link)
		return;

	mumble_data.dtime_cumulative += dtime;

	if (mumble_data.link->uiVersion != 2) {
		wcsncpy(mumble_data.link->name, L"Voxelands", 256);
		wcsncpy(mumble_data.link->description, L"Voxelands Mumble Position Audio Link.", 2048);
		mumble_data.link->uiVersion = 2;
	}

	mumble_data.link->uiTick++;

	/* Position of the avatar */
	mumble_data.link->fAvatarPosition[0] = pos->x;
	mumble_data.link->fAvatarPosition[1] = pos->y;
	mumble_data.link->fAvatarPosition[2] = pos->z;

	/* Unit vector pointing out of the avatar's eyes aka "At"-vector. */
	mumble_data.link->fAvatarFront[0] = at->x;
	mumble_data.link->fAvatarFront[1] = at->y;
	mumble_data.link->fAvatarFront[2] = at->z;

	/* Unit vector pointing out of the top of the avatar's head aka "Up"-vector. */
	mumble_data.link->fAvatarTop[0] = -up->x;
	mumble_data.link->fAvatarTop[1] = -up->y;
	mumble_data.link->fAvatarTop[2] = -up->z;

	/* Same as avatar but for the camera. */
	mumble_data.link->fCameraPosition[0] = pos->x;
	mumble_data.link->fCameraPosition[1] = pos->y;
	mumble_data.link->fCameraPosition[2] = pos->z;

	mumble_data.link->fCameraFront[0] = at->x;
	mumble_data.link->fCameraFront[1] = at->y;
	mumble_data.link->fCameraFront[2] = at->z;

	mumble_data.link->fCameraTop[0] = -up->x;
	mumble_data.link->fCameraTop[1] = -up->y;
	mumble_data.link->fCameraTop[2] = -up->z;
}
