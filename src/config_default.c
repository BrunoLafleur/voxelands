/************************************************************************
* config_default.c
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
#include "path.h"
#ifndef SERVER
#include "sound.h"
#endif

#include <string.h>

void config_default_init()
{
	config_set_default("log.min_level","error",log_minlevel_setter);
	config_set_default("log.max_level","info",log_maxlevel_setter);
	config_set_default("log.system.min_level","chat",log_sminlevel_setter);
	config_set_default("log.system.max_level","info",log_smaxlevel_setter);
	config_set_default("log.console.min_level","chat",log_cminlevel_setter);
	config_set_default("log.console.max_level","info",log_cmaxlevel_setter);

	config_set_default("path.log",NULL,log_file_setter);
	config_set_default("path.data.custom",NULL,path_custom_setter);
	config_set_default("path.screenshot",NULL,path_screenshot_setter);
	config_set_default("world.path",NULL,path_world_setter);

	config_set_default("debug.profiler.interval","0",NULL);
	config_set_default("debug.show","false",NULL);

#ifndef SERVER
	config_set_default("client.video.anisotropic","false",NULL);
	config_set_default("client.video.bilinear","false",NULL);
	config_set_default("client.video.trilinear","false",NULL);
	config_set_default("client.video.mipmaps","true",NULL);
	config_set_default("client.video.fullscreen","false",NULL);
	config_set_default("client.video.fullscreen.bpp","24",NULL);
	config_set_default("client.video.fullscreen.fsaa","0",NULL);
	config_set_default("client.video.vsync","false",NULL);
	config_set_default("client.video.size.width","1024",NULL);
	config_set_default("client.video.size.height","600",NULL);
	config_set_default("client.video.driver","opengl",NULL);
	config_set_default("client.video.hpfpu","true",NULL);
	
#if USE_AUDIO == 1
	config_set_default("client.sound.volume","50",sound_master_setter);
	config_set_default("client.sound.volume.effects","50",sound_effects_setter);
	config_set_default("client.sound.volume.music","50",sound_music_setter);
#else
	config_set_default("client.sound.volume","50",NULL);
	config_set_default("client.sound.volume.effects","50",NULL);
	config_set_default("client.sound.volume.music","50",NULL);
#endif
	config_set_default("client.sound.mumble","true",NULL);
#if USE_MUMBLE == 0
# if USE_AUDIO == 1
	config_set_default("client.name",NULL,sound_mumble_set_ident);
# else
	config_set_default("client.name",NULL,NULL);
# endif
#endif

	config_set_default("client.graphics.mesh.lod","3",NULL);
	config_set_default("client.graphics.texture.animations","false",NULL);
	config_set_default("client.graphics.texture.atlas","true",NULL);
	config_set_default("client.graphics.texture.lod","3",NULL);
	config_set_default("client.graphics.light.lod","3",NULL);
	config_set_default("client.graphics.light.fog","true",NULL);
	config_set_default("client.graphics.fov","72",NULL);
	config_set_default("client.graphics.fps.min","30",NULL);
	config_set_default("client.graphics.fps.max","60",NULL);
	config_set_default("client.graphics.bobbing","true",NULL);
	config_set_default("client.graphics.bobbing.amount","1.0",NULL);
	config_set_default("client.graphics.range.min","15",NULL);
	config_set_default("client.graphics.range.max","300",NULL);
	config_set_default("client.graphics.particles","true",NULL);
	config_set_default("client.graphics.clouds","true",NULL);
	config_set_default("client.graphics.water.opaque","false",NULL);
	config_set_default("client.graphics.selection","highlight",NULL);

	config_set_default("client.ui.mainmenu.tab","credits",NULL);
	config_set_default("client.ui.hud.old","false",NULL);
	config_set_default("client.ui.hud.wieldindex","false",NULL);
	config_set_default("client.ui.mouse.invert","false",NULL);
	config_set_default("client.ui.mouse.sensitivity","0.2",NULL);
	config_set_default("client.ui.font","unifont.ttf",NULL);
	config_set_default("client.ui.font.size","14",NULL);

	config_set_default("client.chunk.timeout","600",NULL);


	config_set_default("keymap_forward","KEY_KEY_W",NULL);
	config_set_default("keymap_backward","KEY_KEY_S",NULL);
	config_set_default("keymap_left","KEY_KEY_A",NULL);
	config_set_default("keymap_right","KEY_KEY_D",NULL);
	config_set_default("keymap_jump","KEY_SPACE",NULL);
	config_set_default("keymap_sneak","KEY_LSHIFT",NULL);
	config_set_default("keymap_inventory","KEY_KEY_I",NULL);
	config_set_default("keymap_examine","KEY_KEY_Q",NULL);
	config_set_default("keymap_use","KEY_KEY_H",NULL);
	config_set_default("keymap_chat","KEY_KEY_T",NULL);
	config_set_default("keymap_cmd","/",NULL);
	config_set_default("keymap_rangeselect","KEY_KEY_O",NULL);
	config_set_default("keymap_freemove","KEY_KEY_K",NULL);
	config_set_default("keymap_up","KEY_KEY_R",NULL);
	config_set_default("keymap_down","KEY_KEY_F",NULL);
	config_set_default("keymap_run","KEY_KEY_E",NULL);
	config_set_default("keymap_screenshot","KEY_F12",NULL);
	config_set_default("keymap_toggle_hud","KEY_F1",NULL);
	config_set_default("keymap_toggle_chat","KEY_F2",NULL);
	config_set_default("keymap_toggle_force_fog_off","KEY_F3",NULL);
	config_set_default("keymap_toggle_update_camera","KEY_F4",NULL);
	config_set_default("keymap_toggle_debug","KEY_F5",NULL);
	config_set_default("keymap_toggle_profiler","KEY_F6",NULL);
	config_set_default("keymap_increase_viewing_range_min","KEY_PRIOR",NULL);
	config_set_default("keymap_decrease_viewing_range_min","KEY_NEXT",NULL);
	config_set_default("keymap_select_prev",",",NULL);
	config_set_default("keymap_select_next",".",NULL);
	/* some (temporary) keys for debugging o.0 */
	config_set_default("keymap_print_debug_stacks","KEY_KEY_P",NULL);

	config_set_default("new_style_water","false",NULL);
	config_set_default("new_style_leaves","true",NULL);
	config_set_default("continuous_forward","false",NULL);
	config_set_default("invisible_stone","false",NULL);
#endif

	/* game/world */
	config_set_default("world.player.inventory.starter","false",NULL);
	config_set_default("world.player.inventory.creative","false",NULL);
	config_set_default("world.player.inventory.droppable","true",NULL);
	config_set_default("world.player.inventory.keep","false",NULL);
	config_set_default("world.player.tool.wear","true",NULL);
	config_set_default("world.player.damage","true",NULL);
	config_set_default("world.player.suffocation","true",NULL);
	config_set_default("world.player.hunger","true",NULL);
	config_set_default("world.game.mob.spawn.level","destructive",NULL);
#ifndef SERVER
	config_set_default("world.player.lava","true",NULL);
	config_set_default("world.game.environment.tnt","true",NULL);
#else
	config_set_default("world.player.lava","false",NULL);
	config_set_default("world.game.environment.tnt","false",NULL);
#endif
	config_set_default("world.game.mode","survival",config_default_gamemode);
	config_set_default("world.game.borderstone.radius","5",NULL);
	config_set_default("world.game.mob.spawn.natural","true",NULL);
	config_set_default("world.game.environment.footprints","true",NULL);
	config_set_default("world.game.environment.fire.spread","false",NULL);
	config_set_default("world.game.environment.time.speed","96",NULL);
	config_set_default("world.game.environment.season","auto",NULL);
	config_set_default("world.game.motd","",NULL);
	config_set_default("world.map.type","default",NULL);

	/* server */
	config_set_default("world.server.chunk.range.active","2",NULL);
	config_set_default("world.server.chunk.range.send","7",NULL);
	config_set_default("world.server.chunk.range.generate","5",NULL);
	config_set_default("world.server.mob.range","3",NULL);
	config_set_default("world.server.client.version.strict","false",NULL);
	config_set_default("world.server.client.private","false",NULL);
	config_set_default("world.server.client.emptypwd","false",NULL);
	config_set_default("world.server.client.default.password","",NULL);
	config_set_default("world.server.client.default.privs","build, shout",NULL);
	config_set_default("world.server.client.max","20",NULL);
	config_set_default("world.server.admin","",NULL);

	config_set_default("server.net.client.queue.size","4",NULL);
	config_set_default("server.net.client.queue.delay","2.0",NULL);
	config_set_default("server.net.client.time.interval","5",NULL);
	config_set_default("server.net.client.object.interval","0.2",NULL);
	/* only enable http on the server, singleplayer doesn't need it */
#ifndef SERVER
	config_set_default("server.net.http","false",NULL);
#else
	config_set_default("server.net.http","true",NULL);
#endif
	config_set_default("server.net.chunk.max","20",NULL);
	config_set_default("server.chunk.timeout","19",NULL);
	config_set_default("server.save.interval","60",NULL);


	config_set_default("global.api.address","servers.voxelands.com",NULL);
#ifdef SERVER
	config_set_default("world.server.api.announce","true",NULL);
#else
	config_set_default("world.server.api.announce","false",NULL);
#endif
	config_set_default("world.server.name","Voxelands Server",NULL);
	config_set_default("world.server.address","",NULL);
	config_set_default("world.server.port","30000",NULL);

	config_set_default("server.world","default",NULL);
#ifndef SERVER
	config_set_default("client.world","default",NULL);
#endif

	config_default_survival();
}


void config_default_creative()
{
	config_set_default("world.player.inventory.creative","true",NULL);
	config_set_default("world.player.inventory.droppable","false",NULL);
	config_set_default("world.player.inventory.keep","true",NULL);

	config_set_default("world.player.tool.wear","false",NULL);
	config_set_default("world.player.damage","false",NULL);
	config_set_default("world.player.suffocation","false",NULL);
	config_set_default("world.player.hunger","false",NULL);

	config_set_default("world.game.mob.spawn.level","passive",NULL);
}
void config_default_survival()
{
	config_set_default("world.player.inventory.creative","false",NULL);
	config_set_default("world.player.inventory.droppable","true",NULL);
	config_set_default("world.player.inventory.keep","false",NULL);

	config_set_default("world.player.tool.wear","true",NULL);
	config_set_default("world.player.damage","true",NULL);
	config_set_default("world.player.suffocation","true",NULL);
	config_set_default("world.player.hunger","true",NULL);

	config_set_default("world.game.mob.spawn.level","destructive",NULL);
}

int config_default_gamemode(const char* mode)
{
	if (mode && !strcmp(mode,"creative")) {
		config_default_creative();
	}else{
		config_default_survival();
	}
	return 0;
}
