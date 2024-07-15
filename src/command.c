/************************************************************************
* command.c
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
#include "nvp.h"
#include "array.h"
#include "thread.h"
#include "file.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

typedef struct setter_s {
	int (*func)(command_context_t *ctx, array_t *args);
	int (*help)();
	uint8_t client;
} setter_t;

static struct {
	nvp_t *list;
	mutex_t *mutex;
} command_data = {
	NULL,
	NULL
};

static int command_alias(command_context_t *ctx, array_t *args)
{
	char* a;
	char* c;
	nvp_t *n;

	if (!args) {
		n = command_data.list;
		while (n) {
			if (n->value)
				command_print(ctx, 0, CN_INFO, "alias: %s %s",n->name,n->value);
			n = n->next;
		}
		return 0;
	}

	a = array_get_string(args,0);
	c = array_join(args," ",1);

	n = nvp_get(&command_data.list,a);
	if (!n) {
		if (c)
			nvp_set(&command_data.list,a,c,NULL);
		return 0;
	}

	if (!n->value) {
		command_print(ctx, 0, CN_WARN, "alias: Cannot unalias built-in command '%s'",a);
		return 1;
	}

	nvp_set(&command_data.list,a,c,NULL);


	return 0;
}

static int command_help(command_context_t *ctx, array_t *args)
{
	nvp_t *n;
	setter_t *s;
	if (args) {
		char* c = array_get_string(args,0);
		if (!c)
			return 1;

		n = nvp_get(&command_data.list,c);
		if (!n)
			return 1;

		s = n->data;
		if (s && s->help) {
			/* TODO: implement the help() functionality */
			s->help();
			return 0;
		}

		command_print(ctx, 0, CN_INFO, "%s [arguments]",c);
		return 0;
	}
	return 0;
}

/*
	global		- settings that are used by the server and client, and aren't per-world
	log		- settings related to logging
	path		- settings related to file paths
	server		- settings used soley by the server, and aren't per-world
	client		- settings used soley by the client (never per-world)
	debug		- debug related settings used by client and/or server
	world		- per-world settings
*/

/* returns 0 for client, 1 for server, and -1 for both */
static int command_set_detect(char* name)
{
	if (!name)
		return 0;

	if (!strncmp(name,"client.",7))
		return 0;
	if (!strncmp(name,"path.",5))
		return 0;
	if (!strncmp(name,"debug.",6))
		return -1;
	if (!strncmp(name,"log.",4))
		return -1;
	if (!strncmp(name,"global.",7))
		return -1;

	return 1;
}

static int command_send_str(char* name, char* args)
{
#ifndef SERVER
	char buff[1024];

	if (snprintf(buff,1024,"/%s %s",name,args) >= 1024)
		return 1;

	return bridge_client_send_msg(buff);
#else
	command_print(NULL, 0, CN_WARN, "Server-side command not sent: '%s'",name);
#endif
	return 1;
}

static int command_send(char* name, array_t *args)
{
	char* str;
	int r;

	str = array_join(args," ", 0);
	if (!str)
		return 1;

	r = command_send_str(name,str);
	free(str);
	return r;
}

static int command_set(command_context_t *ctx, array_t *args)
{
	char* n;
	int v;

	n = array_get_string(args,0);
	if (!n)
		return 1;

	if (!ctx) {
		v = command_set_detect(n);
		if (v) {
			command_send("set",args);
			if (v == 1)
				return 0;
		}
	}

	config_set_command(ctx,args);

	return 0;
}

static int command_toggle(command_context_t *ctx, array_t *args)
{
	char* n;
	int v;

	n = array_get_string(args,0);
	if (!n)
		return 1;

	if (!ctx) {
		v = command_set_detect(n);
		if (v) {
			command_send("toggle",args);
			if (v == 1)
				return 0;
		}
	}

	v = config_get_bool(n);
	config_set_int(n,!v);

	return 0;
}

static int command_unset(command_context_t *ctx, array_t *args)
{
	char* n;
	int v;

	n = array_get_string(args,0);
	if (!n)
		return 1;

	if (!ctx) {
		v = command_set_detect(n);
		if (v) {
			command_send("unset",args);
			if (v == 1)
				return 0;
		}
	}

	config_set_command(ctx,args);

	return 0;
}

void command_print(command_context_t *ctx, uint16_t flags, uint8_t type, char* str, ...)
{
	int l;
	char buff[1024];
	va_list ap;
	va_start(ap, str);
	l = vsnprintf(buff,1024,str,ap);
	va_end(ap);

	if (l >= 1024) {
		if (ctx)
			ctx->flags = 0;
		return;
	}

	if (ctx) {
		if (type < CN_ACTION || !flags) {
			ctx->flags = SEND_TO_SENDER;
		}else{
			ctx->flags = flags;
		}
		strcpy(ctx->reply,buff);
	}

	vlprint(type,buff);
}

/* initialise commands */
int command_init()
{
	command_data.mutex = mutex_create();
	command_add("help",command_help,1);
	command_add("set",command_set,1);
	command_add("toggle",command_toggle,1);
	command_add("unset",command_unset,1);
	command_add("exec",config_load_command,1);
	command_add("ignore",config_ignore_command,1);
	command_add("alias",command_alias,1);

	command_add("status",command_status,0);
	command_add("me",command_me,0);
	command_add("privs",command_privs,0);
	command_add("grant",command_grant,0);
	command_add("revoke",command_revoke,0);
	command_add("time",command_time,0);
	command_add("shutdown",command_shutdown,0);
	command_add("teleport",command_teleport,0);
	command_add("ban",command_ban,0);
	command_add("unban",command_unban,0);
	command_add("adduser",command_adduser,0);
	command_add("clearobjects",command_clearobjects,0);
	command_add("setpassword",command_setpassword,0);
/*	command_add("bind",event_bind);


	command_add("jump",control_jump);
	command_add("sneak",control_sneak);
	command_add("inventory",control_inventory);
	command_add("examine",control_examine);
	command_add("use",control_use);
	command_add("chat",control_chat);
	command_add("fly",control_fly);
	command_add("up",control_up);
	command_add("down",control_down);
	command_add("run",control_run);
	command_add("dig",control_dig);
	command_add("place",control_place);
	command_add("wield0",control_wield0);
	command_add("wield1",control_wield1);
	command_add("wield2",control_wield2);
	command_add("wield3",control_wield3);
	command_add("wield4",control_wield4);
	command_add("wield5",control_wield5);
	command_add("wield6",control_wield6);
	command_add("wield7",control_wield7);
	command_add("wieldnext",control_wieldnext);
	command_add("wieldprev",control_wieldprev);
	command_add("console",control_console);
	command_add("capture",control_capture);
 */

	return 0;
}

/* register a new command function */
int command_add(char* name, int (*func)(command_context_t *ctx, array_t *args), uint8_t clientside)
{
	nvp_t *n;
	setter_t *s;

	n = nvp_get(&command_data.list,name);
	if (n && n->data) {
		s = n->data;
		if (s->func)
			return 1;
	}

	s = malloc(sizeof(setter_t));
	if (!s)
		return 1;
	s->func = func;
	s->help = NULL;
	s->client = clientside;

	nvp_set(&command_data.list,name,NULL,s);

	return 0;
}

/* apply a command */
int command_apply(command_context_t *ctx, char* name, char* value)
{
	int r;
	setter_t *s;
	nvp_t *n;
	array_t *args;
	int (*func)(command_context_t *ctx, array_t *args);

	mutex_lock(command_data.mutex);

	n = nvp_get(&command_data.list,name);
	if (!n) {
		mutex_unlock(command_data.mutex);
		command_print(ctx, 0, CN_WARN, "Unknown command: '%s'",name);
		return 1;
	}

	if (n->value) {
		char buff[256];
		strcpy(buff,n->value);
		mutex_unlock(command_data.mutex);
		if (value) {
			return command_execf(ctx,"%s %s",buff,value);
		}else{
			return command_exec(ctx,buff);
		}
	}

	if (!n->data) {
		mutex_unlock(command_data.mutex);
		command_print(ctx, 0, CN_WARN, "Invalid command: '%s'",name);
		return 1;
	}

	s = n->data;

	if (!s->func) {
		mutex_unlock(command_data.mutex);
		command_print(ctx, 0, CN_WARN, "Invalid command: '%s'",name);
		return 1;
	}

	if (!s->client && !ctx) {
		mutex_unlock(command_data.mutex);
		command_send_str(name,value);
		return 0;
	}

	func = s->func;

	mutex_unlock(command_data.mutex);

	args = array_split(value," ",1);

	r = func(ctx,args);

	array_free(args,1);

	return r;
}

/* execute a command */
int command_exec(command_context_t *ctx, char* str)
{
	int r;
	char* cmd = str;
	char* spc;

	if (!cmd || !cmd[0])
		return 1;

	spc = strchr(cmd,' ');

	if (cmd[0] == '/')
		cmd++;

	vlprintf(CN_INFO, "%s",cmd);

	if (spc && spc[0]) {
		char* args = spc+1;
		*spc = 0;

		r = command_apply(ctx,cmd,args);

		*spc = ' ';
	}else{
		r = command_apply(ctx,cmd,NULL);
	}

	return r;
}

/* execute a command from a formatted string */
int command_execf(command_context_t *ctx, char* str, ...)
{
	int l;
	char buff[1024];
	va_list ap;
	va_start(ap, str);
	l = vsnprintf(buff,1024,str,ap);
	va_end(ap);

	if (l >= 1024)
		return 1;

	return command_exec(ctx,buff);
}

/* save alias to file */
void command_save(file_t *f)
{
	nvp_t *n = command_data.list;
	while (n) {
		if (!n->data)
			file_writef(f,"alias %s %s\n",n->name,n->value);
		n = n->next;
	}
}
