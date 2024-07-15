#ifndef _COMMON_H_
#define _COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef LINUX
#ifdef __linux__
#define LINUX
#endif
#endif

#ifdef LINUX

#ifdef _POSIX_C_SOURCE
#undef _POSIX_C_SOURCE
#endif
#define _POSIX_C_SOURCE 200112L

#ifdef _XOPEN_SOURCE
#undef _XOPEN_SOURCE
#endif
#define _XOPEN_SOURCE 700

#ifdef _DEFAULT_SOURCE
#undef _DEFAULT_SOURCE
#endif
#define _DEFAULT_SOURCE
#else

#ifdef _WIN32

#ifndef WIN32
#define WIN32
#endif

#include <windows.h>

#endif
#endif

#include <stdlib.h>
#include <stdint.h>

#include "array.h"
#include "file.h"

#include "config.h"

#ifndef _HAVE_V3_TYPE
#define _HAVE_V3_TYPE
typedef struct v3_s {
	float x;
	float y;
	float z;
} __attribute__((packed)) v3_t;
#endif

#ifndef _HAVE_COMMAND_CONTEXT_TYPE
#define _HAVE_COMMAND_CONTEXT_TYPE
typedef struct command_context_s {
	char player[256];
	uint64_t privs;
	uint32_t flags;

	char reply[2048];

	/* bridge stuff TODO: remove */
	void* bridge_server;
	void* bridge_env;
	void* bridge_player;
} command_context_t;
#endif

#ifndef _HAVE_WORDLIST_TYPE
#define _HAVE_WORDLIST_TYPE
typedef struct worldlist_s {
	struct worldlist_s *prev;
	struct worldlist_s *next;
	char* name;
	char* path;
	char* mode;
	char* version;
	int8_t compat;
} worldlist_t;
#endif

#define CN_ERROR	0x01
#define CN_WARN		0x02
#define CN_ACTION	0x03
#define CN_CHAT		0x04
#define CN_INFO		0x05
#define CN_DEBUG	0x06

#define SEND_TO_SENDER (1<<0)
#define SEND_TO_OTHERS (1<<1)
#define SEND_NO_PREFIX (1<<2)

/* defined in string.c */
char* trim(char* str);
/*char* strdup(const char* str);*/
int str_sanitise(char* dest, int size, char* str);
int strappend(char* dest, int size, char* str);
int parse_bool(char* str);
int str_tov3t(char* str, v3_t *v);
int str_topwd(char* name, char* pass, char* buff, int size);

/* defined in config.c */
char* config_get(char* name);
int config_get_int(char* name);
int64_t config_get_int64(char* name);
float config_get_float(char* name);
int config_get_bool(char* name);
int config_get_v3t(char* name, v3_t *value);
void config_set(char* name, char* value);
int config_set_command(command_context_t *ctx, array_t *args);
void config_set_int(char* name, int value);
void config_set_int64(char* name, int64_t value);
void config_set_float(char* name, float value);
void config_set_default(char* name, char* value, int (*setter)(char* v));
void config_set_default_int(char* name, int value, int (*setter)(char* v));
void config_set_default_float(char* name, float value, int (*setter)(char* v));
void config_load(char* type, char* file);
int config_load_command(command_context_t *ctx, array_t *args);
int config_ignore_command(command_context_t *ctx, array_t *args);
void config_init(int argc, char** argv);
void config_save(char* section, char* type, char* file);
void config_clear(char* section);

/* defined in config_default.c */
void config_default_init(void);
void config_default_creative(void);
void config_default_survival(void);
int config_default_gamemode(char* mode);

/* defined in log.c */
int log_minlevel_setter(char* v);
int log_maxlevel_setter(char* v);
int log_sminlevel_setter(char* v);
int log_smaxlevel_setter(char* v);
int log_cminlevel_setter(char* v);
int log_cmaxlevel_setter(char* v);
int log_file_setter(char* v);
void vlprint(uint8_t type, char* str);
void vlprintf(uint8_t type, char* fmt,...);

/* defined in utf8.c */
int utf8_seqlen(char* str);
uint32_t utf8_nextchar(char* str, int *i);
uint32_t utf8_toutf32(char* src, int size);
int utf8_fromutf32(char *dest, int sz, uint32_t ch);
uint32_t utf16_toutf32(uint16_t *str);
int utf8_offset(char* str, int i);
int utf8_charindex(char* str, int o);
int utf8_strlen(char* str);
void utf8_inc(char* str, int *i);
void utf8_dec(char* str, int *i);
char* utf8_strchr(char* str, uint32_t ch, int *charn);
char* utf8_memchr(char* str, uint32_t ch, size_t sz, int *charn);

/* defined in sys_console.c */
void sys_console_print(char* str, int newline);
void sys_console_printf(char* fmt, ...);
void sys_console_init(void);
void sys_console_exit(void);

/* defined in time.c */
void time_init(void);
uint32_t time_ticks(void);
void delay(uint32_t ms);
float time_dtime(uint32_t last);
uint32_t interval_delay(uint32_t last, uint32_t hz);
uint32_t calc_fps(uint32_t prev, uint32_t current);

/* defined in command.c */
int command_init(void);
void command_print(command_context_t *ctx, uint16_t flags, uint8_t type, char* str, ...);
int command_add(char* name, int (*func)(command_context_t *ctx, array_t *args), uint8_t clientside);
int command_apply(command_context_t *ctx, char* name, char* value);
int command_exec(command_context_t *ctx, char* str);
int command_execf(command_context_t *ctx, char* str, ...);
void command_save(file_t *f);

/* defined in servercommand.c */
int command_status(command_context_t *ctx, array_t *args);
int command_me(command_context_t *ctx, array_t *args);
int command_privs(command_context_t *ctx, array_t *args);
int command_grant(command_context_t *ctx, array_t *args);
int command_revoke(command_context_t *ctx, array_t *args);
int command_time(command_context_t *ctx, array_t *args);
int command_shutdown(command_context_t *ctx, array_t *args);
int command_teleport(command_context_t *ctx, array_t *args);
int command_ban(command_context_t *ctx, array_t *args);
int command_unban(command_context_t *ctx, array_t *args);
int command_adduser(command_context_t *ctx, array_t *args);
int command_clearobjects(command_context_t *ctx, array_t *args);
int command_setpassword(command_context_t *ctx, array_t *args);

/* defined in world.c */
int world_create(char* name);
int world_load(char* name);
int world_import(char* path);
void world_unload(void);
int world_init(char* name);
void world_exit(void);
worldlist_t *world_list_get(void);
void world_list_free(worldlist_t *l);

#ifdef __cplusplus
}
#include <string>
class Client;
void bridge_register_client(Client *c);
std::string bridge_config_get(char* name);
#endif

/* bridge stuff TODO: remove */
#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif
EXTERNC int bridge_client_send_msg(char* str);
EXTERNC int bridge_server_get_status(command_context_t *ctx, char* buff, int size);
EXTERNC int bridge_server_notify_player(command_context_t *ctx, char* name, char* str, ...);
EXTERNC int bridge_server_settime(command_context_t *ctx, uint32_t time);
EXTERNC int bridge_server_shutdown(command_context_t *ctx);
EXTERNC int bridge_server_get_player_ip_or_name(command_context_t *ctx, char* name, char* buff, int size);
EXTERNC int bridge_server_add_player(command_context_t *ctx, char* name, char* pass);
EXTERNC int bridge_set_player_password(command_context_t *ctx, char* name, char* pass);
EXTERNC int bridge_env_check_player(command_context_t *ctx, char* name);
EXTERNC int bridge_env_player_pos(command_context_t *ctx, char* name, v3_t *pos);
EXTERNC int bridge_env_clear_objects(command_context_t *ctx);
EXTERNC int bridge_move_player(command_context_t *ctx, v3_t *pos);
EXTERNC unsigned char* bridge_sha1(char *str);

#endif
