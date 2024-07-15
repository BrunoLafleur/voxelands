/************************************************************************
* sound_util.c
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

void sound_play_step(Map *map, v3f pos, int foot, float gain)
{
	v3_t vp;
	v3s16 p = floatToInt(pos,BS);
	MapNode n = map->getNodeNoEx(p);
	ContentFeatures *f = &content_features(n);
	if (f->type == CMT_AIR) {
		p.Y--;
		n = map->getNodeNoEx(p);
		f = &content_features(n);
	}

	std::string snd("");

	if (f->sound_step != "") {
		snd = f->sound_step;
	}else{
		switch (f->type) {
		case CMT_PLANT:
			snd = "plant-step";
			break;
		case CMT_DIRT:
			snd = "dirt-step";
			break;
		case CMT_STONE:
			snd = "stone-step";
			break;
		case CMT_LIQUID:
			snd = "liquid-step";
			break;
		case CMT_TREE:
		case CMT_WOOD:
			snd = "wood-step";
			break;
		case CMT_GLASS:
			snd = "glass-step";
			break;
		default:;
		}
	}

	if (snd == "")
		return;

	if (foot == 0) {
		snd += "-left";
	}else{
		snd += "-right";
	}

	vp.x = pos.X;
	vp.y = pos.Y;
	vp.z = pos.Z;

	sound_play_effect((char*)snd.c_str(),1.0,0,&vp);
}

void sound_play_dig(content_t c, v3f pos)
{
	v3_t vp;
	ContentFeatures *f;

	if (c == CONTENT_IGNORE)
		return;

	vp.x = pos.X;
	vp.y = pos.Y;
	vp.z = pos.Z;

	f = &content_features(c);

	if (f->sound_dig != "") {
		sound_play_effect((char*)f->sound_dig.c_str(),1.0,0,&vp);
		return;
	}
	switch (f->type) {
	case CMT_PLANT:
		sound_play_effect("plant-dig",1.0,0,&vp);
		break;
	case CMT_DIRT:
		sound_play_effect("dirt-dig",1.0,0,&vp);
		break;
	case CMT_STONE:
		sound_play_effect("stone-dig",1.0,0,&vp);
		break;
	case CMT_LIQUID:
		sound_play_effect("liquid-dig",1.0,0,&vp);
		break;
	case CMT_TREE:
	case CMT_WOOD:
		sound_play_effect("wood-dig",1.0,0,&vp);
		break;
	case CMT_GLASS:
		sound_play_effect("glass-dig",1.0,0,&vp);
		break;
	default:
		sound_play_effect("miss-dig",1.0,0,&vp);
	}
}
