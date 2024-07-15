/************************************************************************
* sound.c
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
#include "list.h"
#include "file.h"
#define _VL_SOUND_EXPOSE_INTERNAL
#include "sound.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct {
	int init;
	float volume;
	struct {
		float volume;
		float fade;
		float fadev;
		sound_t *sounds;
		sound_instance_t *playing;
	} effects;
	struct {
		float volume;
		sound_t *sounds;
		sound_instance_t *playing;
	} music;
	ALCdevice *device;
	ALCcontext *context;
} sound_data = {
	0,
	0.5,
	{
		0.5,
		-1.0,
		1.0,
		NULL,
		NULL
	},
	{
		0.5,
		NULL,
		NULL
	},
	NULL,
	NULL
};

/* initialise sound */
int sound_init()
{
	/* initialise audio */
	sound_data.device = alcOpenDevice(NULL);
	if (!sound_data.device)
		return 1;

	sound_data.context = alcCreateContext(sound_data.device, NULL);

	if (!sound_data.context) {
		alcCloseDevice(sound_data.device);
		return 1;
	}

	if (!alcMakeContextCurrent(sound_data.context) || (alcGetError(sound_data.device) != ALC_NO_ERROR)) {
 		alcDestroyContext(sound_data.context);
		alcCloseDevice(sound_data.device);
		return 1;
	}

	alDistanceModel(AL_EXPONENT_DISTANCE);
	alListenerf(AL_GAIN, 1.0);

#if USE_MUMBLE == 1
	if (config_get_bool("client.sound.mumble"))
		sound_mumble_init();
#endif

	sound_data.init = 1;

	/* sounds must be mono sounds, stereo will not work right!
	 * exceptions: background music */

	/* walking */
	/* CMT_DIRT */
	sound_load_effect("step_dirt.1.ogg","dirt-step-left");
	sound_load_effect("step_dirt.2.ogg","dirt-step-right");
	/* CMT_STONE */
	sound_load_effect("step_stone.1.ogg","stone-step-left");
	sound_load_effect("step_stone.2.ogg","stone-step-right");
	sound_load_effect("step_stone.3.ogg","stone-step-left");
	sound_load_effect("step_stone.4.ogg","stone-step-right");
	/* CMT_PLANT */
	sound_load_effect("step_plant.1.ogg","plant-step-left");
	sound_load_effect("step_plant.2.ogg","plant-step-right");
	/* CMT_LIQUID */
	sound_load_effect("step_liquid.1.ogg","liquid-step-left");
	sound_load_effect("step_liquid.2.ogg","liquid-step-right");
	/* CMT_WOOD */
	sound_load_effect("step_wood.1.ogg","wood-step-left");
	sound_load_effect("step_wood.2.ogg","wood-step-right");
	sound_load_effect("step_wood.3.ogg","wood-step-left");
	sound_load_effect("step_wood.4.ogg","wood-step-right");
	/* CMT_GLASS */
	sound_load_effect("step_glass.1.ogg","glass-step-left");
	sound_load_effect("step_glass.1.ogg","glass-step-right");
	/* special for grass */
	sound_load_effect("step_grass.1.ogg","grass-step-left");
	sound_load_effect("step_grass.2.ogg","grass-step-right");

	/* digging */
	/* CMT_DIRT */
	sound_load_effect("dig_dirt.1.ogg","dirt-dig");
	/* CMT_STONE */
	sound_load_effect("dig_stone.1.ogg","stone-dig");
	/* CMT_PLANT */
	sound_load_effect("dig_plant.1.ogg","plant-dig");
	/* CMT_LIQUID */
	sound_load_effect("dig_liquid.1.ogg","liquid-dig");
	/* CMT_WOOD */
	sound_load_effect("dig_wood.1.ogg","wood-dig");
	/* CMT_GLASS */
	sound_load_effect("dig_glass.1.ogg","glass-dig");
	/* mobs */
	sound_load_effect("dig_mob.ogg","mob-dig");
	/* miss */
	sound_load_effect("dig_miss.ogg","miss-dig");

	/* placing */
	sound_load_effect("place_node.1.ogg","place");
	sound_load_effect("place_node.2.ogg","place");
	sound_load_effect("place_node.3.ogg","place");
	/* CMT_DIRT */
	/* CMT_STONE */
	/* CMT_PLANT */
	/* CMT_LIQUID */
	sound_load_effect("place_liquid.1.ogg","liquid-place");
	/* CMT_WOOD */
	/* CMT_GLASS */

	/* open formspec */
	sound_load_effect("open_menu.ogg","open-menu");
	sound_load_effect("open_book.ogg","open-book");
	sound_load_effect("open_chest.ogg","open-chest");

	/* environment and node sounds */
	sound_load_effect("env_piston.ogg","env-piston");
	sound_load_effect("env_dooropen.ogg","env-dooropen");
	sound_load_effect("env_doorclose.ogg","env-doorclose");
	sound_load_effect("env_fire.ogg","env-fire");
	sound_load_effect("env_lava.ogg","env-lava");
	sound_load_effect("env_water.ogg","env-water");
	sound_load_effect("env_steam.ogg","env-steam");
	sound_load_effect("env_tnt.ogg","env-tnt");
	sound_load_effect("env_teleport.ogg","env-teleport");

	/* mobs */
	sound_load_effect("mob_oerkki_spawn.ogg","mob-oerkki-spawn");
	sound_load_effect("mob_wolf_hit.ogg","mob-wolf-hit");
	sound_load_effect("mob_wolf_spawn.ogg","mob-wolf-spawn");
	sound_load_effect("mob_sheep_env.ogg","mob-sheep-env");
	sound_load_effect("mob_ducksheep_env.ogg","mob-ducksheep-env");
	sound_load_effect("mob_deer_env.ogg","mob-deer-env");

	/* special */
	sound_load_effect("wield_item.ogg","wield");
	sound_load_effect("low_energy_F.ogg","low-energy-F");
	sound_load_effect("low_energy_M.ogg","low-energy-M");
	sound_load_effect("player_hurt_F.ogg","player-hurt-F");
	sound_load_effect("player_hurt_M.ogg","player-hurt-M");

	/* use */
	sound_load_effect("use_eat.ogg","use-eat");
	sound_load_effect("use_drink.ogg","use-drink");

	/* menu backgrounds */
	sound_load_music("bg_mainmenu.ogg","bg-mainmenu");
	sound_load_music("bg_charcreator.ogg","bg-charcreator");

	return 0;
}

/* exit sound */
void sound_exit()
{
	if (!sound_data.init)
		return;

	sound_stop(0);
	alcMakeContextCurrent(NULL);
	alcDestroyContext(sound_data.context);
	alcCloseDevice(sound_data.device);

	sound_data.init = 0;

	/* TODO: unload sounds */
}

/* do stuff */
void sound_process(float dtime)
{
	ALint state;
	float v;
	sound_instance_t *d;
	sound_instance_t *i;

	/* factor = dtime / fade */

	if (sound_data.effects.fade > -0.5) {
		v = dtime/sound_data.effects.fade;
		sound_data.effects.fadev -= v*sound_data.effects.volume*sound_data.volume;
		if (sound_data.effects.fadev < 0.01)
			sound_stop_effects(0);
	}
	i = sound_data.effects.playing;
	while (i) {
		alGetSourcei(i->id, AL_SOURCE_STATE, &state);
		if (state != AL_PLAYING) {
			d = i;
			i = i->next;
			sound_data.effects.playing = list_remove(&sound_data.effects.playing,d);
			alDeleteSources(1, &d->id);
			free(d);
			continue;
		}
		if (i->fade > -0.5) {
			if (i->fade > 0.01 && i->volume > 0.01) {
				v = dtime/i->fade;
				i->volume -= v*sound_data.music.volume*sound_data.volume;
				if (i->volume > 0.01)
					alSourcef(i->id, AL_GAIN, i->volume);
			}else{
				d = i;
				i = i->next;
				sound_data.effects.playing = list_remove(&sound_data.effects.playing,d);
				alDeleteSources(1, &d->id);
				free(d);
				continue;
			}
		}
		if (sound_data.effects.fade > -0.5)
			alSourcef(i->id, AL_GAIN, sound_data.effects.fadev);
		i = i->next;
	}
}

/* */
void sound_step(float dtime, v3_t *pos, v3_t *at, v3_t *up)
{
	float orientation[6] = {0.0,0.0,0.0,0.0,0.0,0.0};

	sound_process(dtime);

	if (pos) {
		alListener3f(AL_POSITION, pos->x,pos->y,pos->z);
	}else{
		alListener3f(AL_POSITION, 0.0,0.0,0.0);
	}

	alListener3f(AL_VELOCITY, 0.0,0.0,0.0);

	if (at) {
		orientation[0] = at->x;
		orientation[1] = at->y;
		orientation[2] = at->z;
	}

	if (up) {
		orientation[3] = -up->x;
		orientation[4] = -up->y;
		orientation[5] = -up->z;
	}

	alListenerfv(AL_ORIENTATION, orientation);

#if USE_MUMBLE == 1
	if (pos && at && up)
		sound_mumble_step(dtime,pos,at,up);
#endif
}

/* get/set sound effect volume */
float sound_volume_effects(float v)
{
	if (v > -0.5) {
		if (v > 1.0)
			v = 1.0;
		sound_data.effects.volume = v;

		if (sound_data.init) {
			float ev;
			sound_instance_t *i = sound_data.effects.playing;

			ev = v*sound_data.volume;

			while (i) {
				alSourcef(i->id, AL_GAIN, ev);
				i = i->next;
			}
		}
	}
	return sound_data.effects.volume;
}

/* get/set sound music volume */
float sound_volume_music(float v)
{
	if (v > -0.5) {
		if (v > 1.0)
			v = 1.0;
		sound_data.music.volume = v;

		if (sound_data.init && sound_data.music.playing) {
			float ev;

			ev = v*sound_data.volume;

			alSourcef(sound_data.music.playing->id, AL_GAIN, ev);

		}
	}
	return sound_data.music.volume;
}

/* get/set sound master volume */
float sound_volume_master(float v)
{
	if (v > -0.5) {
		if (v > 1.0)
			v = 1.0;
		sound_data.volume = v;
		sound_volume_effects(sound_data.effects.volume);
		sound_volume_music(sound_data.music.volume);
	}
	return sound_data.volume;
}

/* load a sound identified by token from a file */
static sound_t *sound_load(char* file, char* token)
{
	file_t *f;
	sound_t *e = malloc(sizeof(sound_t));
	e->file = strdup(file);
	e->token = strdup(token);
	e->data = NULL;
	e->d_len = 0;

	f = file_load("sound",file);

	if (sound_is_ogg(f)) {
		if (sound_load_ogg(f,e))
			goto sound_load_fail;
	}else if (sound_is_wav(f)) {
		if (sound_load_wav(f,e))
			goto sound_load_fail;
	}else{
		goto sound_load_fail;
	}

	file_free(f);

	return e;

sound_load_fail:
	file_free(f);
	free(e->file);
	free(e->token);
	if (e->data)
		free(e->data);
	free(e);

	vlprintf(CN_ERROR, "failed to load sound data for file '%s'",file);

	return NULL;
}

/* load a sound effect identifed by token */
int sound_load_effect(char* file, char* token)
{
	sound_t *e = sound_load(file,token);
	if (!e)
		return 1;

	sound_data.effects.sounds = list_push(&sound_data.effects.sounds,e);

	return 0;
}

/* load music identifed by token */
int sound_load_music(char* file, char* token)
{
	sound_t *e = sound_load(file,token);
	if (!e)
		return 1;

	sound_data.music.sounds = list_push(&sound_data.music.sounds,e);

	return 0;
}

/* unload a sound effect */
void sound_free_effect(char* token)
{
	sound_t *e = sound_data.effects.sounds;

	if (!e)
		return;

	while (e) {
		if (!strcmp(e->token,token))
			break;
		e = e->next;
	}

	if (!e)
		return;

	sound_data.effects.sounds = list_remove(&sound_data.effects.sounds,e);

	alDeleteBuffers(1,&e->id);

	free(e->data);
	free(e->file);
	free(e->token);
	free(e);
}

/* unload a music */
void sound_free_music(char* token)
{
	sound_t *e = sound_data.music.sounds;
	if (!e)
		return;

	while (e) {
		if (!strcmp(e->token,token))
			break;
		e = e->next;
	}

	if (!e)
		return;

	sound_data.music.sounds = list_remove(&sound_data.music.sounds,e);

	alDeleteBuffers(1,&e->id);

	free(e->data);
	free(e->file);
	free(e->token);
	free(e);
}

/* play sound effect */
uint32_t sound_play_effect(const char* token, float volume, uint8_t loop, v3_t *pos)
{
	sound_instance_t *i;
	sound_t *e = sound_data.effects.sounds;
	if (!sound_data.init || !e)
		return 0;

	while (e) {
		if (!strcmp(e->token,token))
			break;
		e = e->next;
	}

	if (!e)
		return 0;

	i = malloc(sizeof(sound_instance_t));
	if (!i)
		return 0;

	alGenSources(1, &i->id);
	alSourcei(i->id, AL_BUFFER, e->id);

	i->volume = volume;
	i->fade = -1.0;

	if (pos) {
		alSourcei(i->id, AL_SOURCE_RELATIVE, AL_FALSE);
		alSource3f(i->id, AL_POSITION, pos->x, pos->y, pos->z);
		alSourcef(i->id, AL_REFERENCE_DISTANCE, 30.0);
	}else{
		alSourcei(i->id, AL_SOURCE_RELATIVE, AL_TRUE);
		alSource3f(i->id, AL_POSITION, 0, 0, 0);
	}

	alSource3f(i->id, AL_VELOCITY, 0, 0, 0);
	if (loop) {
		alSourcei(i->id, AL_LOOPING, AL_TRUE);
	}else{
		alSourcei(i->id, AL_LOOPING, AL_FALSE);
	}
	alSourcef(i->id, AL_GAIN, sound_data.effects.volume*sound_data.volume*volume);
	alSourcePlay(i->id);

	if (alGetError() != AL_NO_ERROR) {
		alDeleteSources(1, &i->id);
		free(i);
		return 0;
	}

	sound_data.effects.playing = list_push(&sound_data.effects.playing,i);

	return i->id;
}

/* play music */
uint32_t sound_play_music(const char* token, float volume, uint8_t loop)
{
	sound_t *e = sound_data.music.sounds;
	if (!sound_data.init || !e)
		return 0;

	while (e) {
		if (!strcmp(e->token,token))
			break;
		e = e->next;
	}

	if (!e)
		return 0;

	if (sound_data.music.playing) {
		sound_data.effects.playing->fade = 1.0;
		sound_data.effects.playing = list_push(&sound_data.effects.playing,sound_data.music.playing);
	}

	sound_data.music.playing = malloc(sizeof(sound_instance_t));
	if (!sound_data.music.playing)
		return 0;

	alGenSources(1, &sound_data.music.playing->id);
	alSourcei(sound_data.music.playing->id, AL_BUFFER, e->id);

	sound_data.music.playing->volume = volume;
	sound_data.music.playing->fade = -1.0;

	alSourcei(sound_data.music.playing->id, AL_SOURCE_RELATIVE, AL_TRUE);
	alSource3f(sound_data.music.playing->id, AL_POSITION, 0, 0, 0);
	alSource3f(sound_data.music.playing->id, AL_VELOCITY, 0, 0, 0);
	if (loop) {
		alSourcei(sound_data.music.playing->id, AL_LOOPING, AL_TRUE);
	}else{
		alSourcei(sound_data.music.playing->id, AL_LOOPING, AL_FALSE);
	}
	alSourcef(sound_data.music.playing->id, AL_GAIN, sound_data.music.volume*sound_data.volume*volume);
	alSourcePlay(sound_data.music.playing->id);

	if (alGetError() != AL_NO_ERROR) {
		alDeleteSources(1, &sound_data.music.playing->id);
		free(sound_data.music.playing);
		return 0;
	}

	return sound_data.music.playing->id;
}

/* stop sound effects, optionally fading out in fade seconds */
void sound_stop_effects(int fade)
{
	if (fade) {
		sound_data.effects.fade = fade;
		sound_data.effects.fadev = sound_data.effects.volume;
	}else{
		sound_instance_t *i = sound_data.effects.playing;
		while (i) {
			alSourceStop(i->id);
			i = i->next;
		}
		sound_data.effects.fade = -1.0;
	}
}

/* stop music, optionally fading out in fade seconds */
void sound_stop_music(int fade)
{
	if (!sound_data.music.playing)
		return;

	if (!fade)
		alSourceStop(sound_data.music.playing->id);

	sound_data.music.playing->fade = fade;
	sound_data.music.playing->volume = sound_data.music.volume;
	sound_data.effects.playing = list_push(&sound_data.effects.playing,sound_data.music.playing);
	sound_data.music.playing = NULL;

}

void sound_stop_single(uint32_t id)
{
	sound_instance_t *i = sound_data.effects.playing;
	while (i) {
		if (i->id == id) {
			alSourceStop(i->id);
			break;
		}
		i = i->next;
	}
}

int sound_exists(uint32_t id)
{
	sound_instance_t *i = sound_data.effects.playing;
	while (i) {
		if (i->id == id)
			return 1;
		i = i->next;
	}
	return 0;
}

/* stop all sounds, optionally fading out in fade seconds */
void sound_stop(int fade)
{
	sound_stop_music(fade);
	sound_stop_effects(fade);
}

/* command setter for sound effects volume */
int sound_effects_setter(const char* value)
{
	float vf;
	int v = strtol(value,NULL,10);
	vf = (float)v/100.0;
	sound_volume_effects(vf);

	return 0;
}

/* command setter for music volume */
int sound_music_setter(const char* value)
{
	float vf;
	int v = strtol(value,NULL,10);
	vf = (float)v/100.0;
	sound_volume_music(vf);

	return 0;
}

/* command setter for master volume */
int sound_master_setter(const char* value)
{
	int v = strtol(value,NULL,10);
	if (v < 0)
		v = 0;
	if (v > 100)
		v = 100;

	sound_data.volume = (float)v/100.0;

	sound_volume_effects(sound_data.effects.volume);
	sound_volume_music(sound_data.music.volume);

	return 0;
}
