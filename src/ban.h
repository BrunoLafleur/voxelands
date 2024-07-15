#ifndef _BAN_H_
#define _BAN_H_

#ifdef __cplusplus
extern "C" {
#endif

/* defined in ban.c */
int ban_init(const char* file);
void ban_exit(void);
void ban_load(void);
void ban_save(void);
int ban_ipbanned(char* ip);
int ban_description(char* ip_or_name, char* buff, int size);
char* ban_ip2name(char* ip);
void ban_add(char* ip, char* name);
void ban_remove(char* ip_or_name);

#ifdef __cplusplus
}
#endif

#endif
