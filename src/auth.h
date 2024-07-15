#ifndef _AUTH_H_
#define _AUTH_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define PRIV_NONE	0x0000
#define PRIV_BUILD	0x0001	/* build/interact */
#define PRIV_TELEPORT	0x0002	/* teleport */
#define PRIV_SETTIME	0x0004	/* set the time */
#define PRIV_PRIVS	0x0008	/* grant/revoke privs */
#define PRIV_SERVER	0x0010	/* server management (shutdown/settings/etc) */
#define PRIV_SHOUT	0x0020	/* chat */
#define PRIV_BAN	0x0040	/* ban/unban */

/* increment this if a priv is added */
#define PRIV_COUNT	7

#define PRIV_DEFAULT	(PRIV_BUILD|PRIV_SHOUT)
#define PRIV_ALL	0x7FFFFFFFFFFFFFFF
#define PRIV_INVALID	0x8000000000000000

#ifndef _HAVE_AUTHDATA_TYPE
#define _HAVE_AUTHDATA_TYPE
typedef struct authdata_s {
	char pwd[64];
	uint64_t privs;
} authdata_t;
#endif

/* defined in auth.c */
int auth_privs2str(uint64_t privs, char* buff, int size);
uint64_t auth_str2privs(char* str);
int auth_init(char* file);
void auth_exit(void);
void auth_load(void);
void auth_save(void);
int auth_exists(char* name);
void auth_set(char* name, authdata_t data);
void auth_add(char* name);
int auth_getpwd(char* name, char buff[64]);
void auth_setpwd(char* name, char* pwd);
uint64_t auth_getprivs(char* name);
void auth_setprivs(char* name, uint64_t privs);

#ifdef __cplusplus
}
#endif

#endif
