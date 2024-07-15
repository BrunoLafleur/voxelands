#ifndef _INTL_H_
#define _INTL_H_

#ifdef __cplusplus
extern "C" {
#endif

char* gettext(const char *s);
char* ngettext(const char* s1, const char* s2, int n);
void intl_init();

#ifndef SERVER
/* Initialise KeyNamesLang array */
void init_KeyNamesLang();
#endif

#ifdef __cplusplus
}
#endif

#endif
