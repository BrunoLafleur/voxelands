/************************************************************************
* Minetest-c55
* Copyright (C) 2010-2011 celeron55, Perttu Ahola <celeron55@gmail.com>
* Copyright (C) 2011 Ciaran Gultnieks <ciaran@ciarang.com>
*
* servercommand.cpp
* voxelands - 3d voxel world sandbox game
* Copyright (C) Lisa 'darkrose' Milne 2014 <lisa@ltmnet.com>
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
*
* License updated from GPLv2 or later to GPLv3 or later by Lisa Milne
* for Voxelands.
************************************************************************/

#include "common.h"
#include "array.h"
#include "auth.h"
#include "ban.h"

#include <string.h>

int command_status(command_context_t *ctx, array_t *args)
{
	char buff[1024];
	if (bridge_server_get_status(ctx,buff,1024))
		return 1;

	command_print(ctx,SEND_TO_SENDER,CN_INFO,"%s",buff);

	return 0;
}

int command_me(command_context_t *ctx, array_t *args)
{
	char* str;

	if (!ctx || !args || !args->length)
		return 1;

	str = array_join(args," ",0);
	if (!str)
		return 1;

	command_print(ctx,SEND_TO_OTHERS | SEND_NO_PREFIX, CN_CHAT, "* %s %s",ctx->player,str);

	free(str);
	return 0;
}

int command_privs(command_context_t *ctx, array_t *args)
{
	char* str;
	char buff[256];
	uint64_t privs = 0;
	if (!args || !args->length) {
		if (!ctx) {
			/* assume server console */
			privs = PRIV_ALL;
		}else{
			/* Show our own real privs, without any adjustments made for admin status */
			privs = auth_getprivs(ctx->player);
		}

		if (auth_privs2str(privs,buff,256) < 0)
			buff[0] = 0;

		command_print(ctx,SEND_TO_SENDER,CN_INFO,"%s",buff);

		return 0;
	}

	if (ctx && (ctx->privs&PRIV_PRIVS) == 0) {
		command_print(ctx,SEND_TO_SENDER,CN_WARN,"You don't have permission to do that");
		return 1;
	}

	str = array_get_string(args,0);
	if (!str) {
		command_print(ctx,SEND_TO_SENDER,CN_WARN,"You don't have permission to do that");
		return 1;
	}

	if (bridge_env_check_player(ctx,str) == 0) {
		command_print(ctx,SEND_TO_SENDER,CN_WARN,"No such player");
		return 1;
	}

	privs = auth_getprivs(str);

	if (!privs) {
		command_print(ctx,SEND_TO_SENDER,CN_CHAT,"none");
		return 0;
	}

	if (auth_privs2str(privs,buff,256) < 0)
		buff[0] = 0;

	command_print(ctx,SEND_TO_SENDER,CN_INFO,"%s",buff);
	return 0;
}

int command_grant(command_context_t *ctx, array_t *args)
{
	char* name;
	char* str;
	uint64_t privs;
	uint64_t newprivs = 0;
	char buff[256];
	int i;

	if (ctx && (ctx->privs&PRIV_PRIVS) == 0) {
		command_print(ctx,SEND_TO_SENDER,CN_WARN,"You don't have permission to do that");
		return 1;
	}

	if (!args || args->length < 2) {
		command_print(ctx,SEND_TO_SENDER,CN_WARN,"Missing parameter");
		return 1;
	}

	name = array_get_string(args,0);
	if (!name) {
		command_print(ctx,SEND_TO_SENDER,CN_WARN,"You don't have permission to do that");
		return 1;
	}

	if (bridge_env_check_player(ctx,name) == 0) {
		command_print(ctx,SEND_TO_SENDER,CN_WARN,"No such player");
		return 1;
	}

	privs = auth_getprivs(name);
	if (privs == PRIV_INVALID) {
		command_print(ctx,SEND_TO_SENDER,CN_WARN,"No such player");
		return 1;
	}

	for (i=1; i<args->length; i++) {
		str = array_get_string(args,i);
		if (!str)
			break;

		privs = auth_str2privs(str);
		if (privs == PRIV_INVALID) {
			command_print(ctx,SEND_TO_SENDER,CN_WARN,"Invalid privileges specified");
			return 1;
		}

		newprivs |= privs;
	}

	if (!newprivs || newprivs == PRIV_INVALID) {
		command_print(ctx,SEND_TO_SENDER,CN_WARN,"Invalid privileges specified");
		return 1;
	}

	privs |= newprivs;

	auth_setprivs(name,privs);

	privs = auth_getprivs(name);

	if (auth_privs2str(newprivs,buff,256) >= 0) {
		char *n = "Server Admin";
		if (ctx)
			n = ctx->player;
		bridge_server_notify_player(ctx,name,"%s granted you the privilege \"%s\"",n,buff);
	}

	if (auth_privs2str(privs,buff,256) < 0) {
		strcpy(buff,"???");
	}

	command_print(ctx,SEND_TO_SENDER,CN_INFO,"Privileges changed to \"%s\"",buff);

	return 0;
}

int command_revoke(command_context_t *ctx, array_t *args)
{
	char* name;
	char* str;
	uint64_t privs;
	uint64_t newprivs = 0;
	char buff[256];
	int i;

	if (ctx && (ctx->privs&PRIV_PRIVS) == 0) {
		command_print(ctx,SEND_TO_SENDER,CN_WARN,"You don't have permission to do that");
		return 1;
	}

	if (!args || args->length < 2) {
		command_print(ctx,SEND_TO_SENDER,CN_WARN,"Missing parameter");
		return 1;
	}

	name = array_get_string(args,0);
	if (!name) {
		command_print(ctx,SEND_TO_SENDER,CN_WARN,"You don't have permission to do that");
		return 1;
	}

	if (bridge_env_check_player(ctx,name) == 0) {
		command_print(ctx,SEND_TO_SENDER,CN_WARN,"No such player");
		return 1;
	}

	privs = auth_getprivs(name);
	if (privs == PRIV_INVALID) {
		command_print(ctx,SEND_TO_SENDER,CN_WARN,"No such player");
		return 1;
	}

	for (i=1; i<args->length; i++) {
		str = array_get_string(args,i);
		if (!str)
			break;

		privs = auth_str2privs(str);
		if (privs == PRIV_INVALID) {
			command_print(ctx,SEND_TO_SENDER,CN_WARN,"Invalid privileges specified");
			return 1;
		}

		newprivs |= privs;
	}

	if (!newprivs || newprivs == PRIV_INVALID) {
		command_print(ctx,SEND_TO_SENDER,CN_WARN,"Invalid privileges specified");
		return 1;
	}

	privs &= ~newprivs;

	auth_setprivs(name,privs);

	privs = auth_getprivs(name);

	if (auth_privs2str(newprivs,buff,256) >= 0) {
		char *n = "Server Admin";
		if (ctx)
			n = ctx->player;
		bridge_server_notify_player(ctx,name,"%s revoked from you the privilige \"%s\"",n,buff);
	}

	if (auth_privs2str(privs,buff,256) < 0) {
		strcpy(buff,"???");
	}

	command_print(ctx,SEND_TO_SENDER,CN_INFO,"Privileges changed to \"%s\"",buff);

	return 0;
}

int command_time(command_context_t *ctx, array_t *args)
{
	char* str;
	uint32_t time;

	if (ctx && (ctx->privs&PRIV_SETTIME) == 0) {
		command_print(ctx,SEND_TO_SENDER,CN_WARN,"You don't have permission to do that");
		return 1;
	}

	if (!args || !args->length) {
		command_print(ctx,SEND_TO_SENDER,CN_WARN,"Missing parameter");
		return 1;
	}

	str = array_get_string(args,0);
	if (!str) {
		command_print(ctx,SEND_TO_SENDER,CN_WARN,"Missing parameter");
		return 1;
	}

	time = strtol(str,NULL,10);

	time %= 24000;

	if (bridge_server_settime(ctx,time)) {
		command_print(ctx,SEND_TO_SENDER,CN_WARN,"Unable to set time");
		return 1;
	}

	str = "Server Admin";
	if (ctx)
		str = ctx->player;

	command_print(ctx,SEND_TO_OTHERS|SEND_TO_SENDER,CN_INFO,"%s sets time to %u",str,time);

	return 0;
}

int command_shutdown(command_context_t *ctx, array_t *args)
{
	if (ctx && (ctx->privs&PRIV_SERVER) == 0) {
		command_print(ctx,SEND_TO_SENDER,CN_WARN,"You don't have permission to do that");
		return 1;
	}

	if (bridge_server_shutdown(ctx)) {
		command_print(ctx,SEND_TO_SENDER,CN_WARN,"Unable to shutdown");
		return 1;
	}

	command_print(ctx,SEND_TO_OTHERS|SEND_TO_SENDER,CN_INFO,"Server shutting down (operator request)");

	return 0;
}

int command_teleport(command_context_t *ctx, array_t *args)
{
	char* str;
	v3_t pos;

	if (ctx && (ctx->privs&PRIV_SERVER) == 0) {
		command_print(ctx,SEND_TO_SENDER,CN_WARN,"You don't have permission to do that");
		return 1;
	}

	if (!args || !args->length) {
		command_print(ctx,SEND_TO_SENDER,CN_WARN,"Missing parameter");
		return 1;
	}

	str = array_get_string(args,0);
	if (!str) {
		command_print(ctx,SEND_TO_SENDER,CN_WARN,"Missing parameter");
		return 1;
	}

	if (!str_tov3t(str,&pos)) {
		pos.x *= 10.0;
		pos.y *= 10.0;
		pos.z *= 10.0;
	}else if (bridge_env_check_player(ctx,str) || bridge_env_player_pos(ctx,str,&pos)) {
		command_print(ctx,SEND_TO_SENDER,CN_WARN,"Teleport to where?");
		return 1;
	}

	if (bridge_move_player(ctx,&pos)) {
		command_print(ctx,SEND_TO_SENDER,CN_WARN,"Unable to teleport");
		return 1;
	}

	vlprintf(CN_ACTION,"%s teleports to %s",ctx->player,str);

	command_print(ctx,SEND_TO_SENDER,CN_INFO,"Teleported to %s",str);

	return 0;
}

int command_ban(command_context_t *ctx, array_t *args)
{
	char* name;
	char buff[256];

	if (ctx && (ctx->privs&PRIV_BAN) == 0) {
		command_print(ctx,SEND_TO_SENDER,CN_WARN,"You don't have permission to do that");
		return 1;
	}

	if (!args || !args->length) {
		command_print(ctx,SEND_TO_SENDER,CN_WARN,"Missing parameter");
		return 1;
	}

	name = array_get_string(args,0);
	if (!name) {
		command_print(ctx,SEND_TO_SENDER,CN_WARN,"You don't have permission to do that");
		return 1;
	}

	if (bridge_env_check_player(ctx,name) == 0) {
		command_print(ctx,SEND_TO_SENDER,CN_WARN,"No such player");
		return 1;
	}

	if (bridge_server_get_player_ip_or_name(ctx,name,buff,256)) {
		command_print(ctx,SEND_TO_SENDER,CN_WARN,"No such player");
		return 1;
	}

	bridge_server_notify_player(ctx,name,"You have been banned from this server");

	ban_add(buff,name);

	command_print(ctx,SEND_TO_SENDER,CN_INFO,"Banned %s (%s)",name,buff);

	return 0;
}

int command_unban(command_context_t *ctx, array_t *args)
{
	char* name;
	if (ctx && (ctx->privs&PRIV_BAN) == 0) {
		command_print(ctx,SEND_TO_SENDER,CN_WARN,"You don't have permission to do that");
		return 1;
	}

	if (!args || !args->length) {
		command_print(ctx,SEND_TO_SENDER,CN_WARN,"Missing parameter");
		return 1;
	}

	name = array_get_string(args,0);
	if (!name) {
		command_print(ctx,SEND_TO_SENDER,CN_WARN,"You don't have permission to do that");
		return 1;
	}

	ban_remove(name);

	command_print(ctx,SEND_TO_SENDER,CN_INFO,"Unbanned %s",name);

	return 0;
}

int command_adduser(command_context_t *ctx, array_t *args)
{
	char* name;
	char* pass;
	if (ctx && (ctx->privs&PRIV_BAN) == 0) {
		command_print(ctx,SEND_TO_SENDER,CN_WARN,"You don't have permission to do that");
		return 1;
	}

	if (!args || args->length != 2) {
		command_print(ctx,SEND_TO_SENDER,CN_WARN,"Missing parameter");
		return 1;
	}

	name = array_get_string(args,0);
	pass = array_get_string(args,1);
	if (!name || !pass) {
		command_print(ctx,SEND_TO_SENDER,CN_WARN,"You don't have permission to do that");
		return 1;
	}

	if (bridge_env_check_player(ctx,name) != 0) {
		command_print(ctx,SEND_TO_SENDER,CN_WARN,"Player '%s' already exists",name);
		return 1;
	}

	if (bridge_server_add_player(ctx,name,pass)) {
		command_print(ctx,SEND_TO_SENDER,CN_WARN,"Unable to add player");
		return 1;
	}

	command_print(ctx,SEND_TO_SENDER,CN_INFO,"Added user %s",name);

	return 0;
}

int command_clearobjects(command_context_t *ctx, array_t *args)
{
	if (ctx && (ctx->privs&PRIV_SERVER) == 0) {
		command_print(ctx,SEND_TO_SENDER,CN_WARN,"You don't have permission to do that");
		return 1;
	}

	bridge_server_notify_player(ctx,NULL,"Clearing all objects, server may lag for a while.");

	bridge_env_clear_objects(ctx);

	command_print(ctx,SEND_TO_SENDER|SEND_TO_OTHERS,CN_INFO,"Clearing objects complete.");

	return 0;
}

int command_setpassword(command_context_t *ctx, array_t *args)
{
	char* name;
	char* pass;
	char buff[256];
	if (ctx && (ctx->privs&PRIV_SERVER) == 0) {
		command_print(ctx,SEND_TO_SENDER,CN_WARN,"You don't have permission to do that");
		return 1;
	}

	if (!args || args->length != 2) {
		command_print(ctx,SEND_TO_SENDER,CN_WARN,"Missing parameter");
		return 1;
	}

	name = array_get_string(args,0);
	pass = array_get_string(args,1);
	if (!name || !pass) {
		command_print(ctx,SEND_TO_SENDER,CN_WARN,"You don't have permission to do that");
		return 1;
	}

	if (bridge_env_check_player(ctx,name) == 0) {
		command_print(ctx,SEND_TO_SENDER,CN_WARN,"No such player");
		return 1;
	}

	if (str_topwd(name,pass,buff,256)) {
		command_print(ctx,SEND_TO_SENDER,CN_WARN,"Invalid password");
		return 1;
	}

	if (bridge_set_player_password(ctx,name,buff)) {
		command_print(ctx,SEND_TO_SENDER,CN_WARN,"Unable to set password");
		return 1;
	}

	command_print(ctx,SEND_TO_SENDER,CN_INFO,"Password set for %s",name);

	return 0;
}

/*
void cmd_help(std::wostringstream &os,
	ServerCommandContext *ctx)
{
	uint64_t privs = ctx->privs;

	if (ctx->parms.size() > 1) {
		if ((privs&PRIV_SERVER) == PRIV_SERVER) {
			if (ctx->parms[1] == L"shutdown") {
				os<<L"-!- /#shutdown - shutdown the server";
				return;
			}else if (ctx->parms[1] == L"clearobjects") {
				os<<L"-!- /clearobjects - remove active objects from the world, may take a long time";
				return;
			}else if (ctx->parms[1] == L"setting") {
				os<<L"-!- /#setting <SETTING> [= VALUE] - change or view a config setting";
				return;
			}else if (ctx->parms[1] == L"setpassword") {
				os<<L"-!- /setpassword <PLAYERNAME> <PASSWORD> - change or set a player's password";
				return;
			}
		}
		if ((privs&PRIV_SETTIME) == PRIV_SETTIME && ctx->parms[1] == L"time") {
			os<<L"-!- /time <0-23999> - set the game time";
			return;
		}
		if ((privs&PRIV_TELEPORT) == PRIV_TELEPORT && ctx->parms[1] == L"teleport") {
			os<<L"-!- /teleport <X>,<Y>,<Z> - teleport to coordinates <X>,<Y>,<Z>";
			return;
		}
		if ((privs&PRIV_PRIVS) == PRIV_PRIVS) {
			if (ctx->parms[1] == L"grant") {
				os<<L"-!- /grant <PLAYERNAME> <PRIVILEGE> - grant a new privilege to a player";
				return;
			}else if (ctx->parms[1] == L"revoke") {
				os<<L"-!- /revoke <PLAYERNAME> <PRIVILEGE> - remove privilege from a player";
				return;
			}
		}
		if ((privs&PRIV_BAN) == PRIV_BAN) {
			if (ctx->parms[1] == L"ban") {
				os<<L"-!- /ban <PLAYERNAME OR IP ADDRESS> - ban a player from the server";
				return;
			}else if (ctx->parms[1] == L"unban") {
				os<<L"-!- /unban <PLAYERNAME OR IP ADDRESS> - remove a player's ban from the server";
				return;
			}else if (ctx->parms[1] == L"adduser") {
				os<<L"-!- /adduser <PLAYERNAME> <PASSWORD> - add a new player with the specified password";
				return;
			}
		}
		if (ctx->parms[1] == L"privs") {
			if ((privs&PRIV_PRIVS) == PRIV_PRIVS) {
				os<<L"-!- /privs [PLAYERNAME] - view the server privileges of a player";
				return;
			}
			os<<L"-!- /privs - view your server privileges";
			return;
		}
		if (ctx->parms[1] == L"status") {
			os<<L"-!- /status - view the server's status and welcome message";
			return;
		}
		if (ctx->parms[1] == L"help") {
			os<<L"-!- /help [COMMAND] - get help for a command or a list of commands";
			return;
		}
	}

	os<<L"-!- Available commands: ";
	os<<L"help status privs";
	if ((privs&PRIV_SERVER) == PRIV_SERVER)
		os<<L" shutdown setting clearobjects setpassword";
	if ((privs&PRIV_SETTIME) == PRIV_SETTIME)
		os<<L" time";
	if ((privs&PRIV_TELEPORT) == PRIV_TELEPORT)
		os<<L" teleport";
	if ((privs&PRIV_PRIVS) == PRIV_PRIVS)
		os<<L" grant revoke";
	if ((privs&PRIV_BAN) == PRIV_BAN)
		os<<L" ban unban";
}
*/
