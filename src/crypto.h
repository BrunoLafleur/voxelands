#ifndef _CRYPTO_H_
#define _CRYPTO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

uint32_t hash(const char* str);
char* base64_encode(const char* str);
char* base64_decode(const char* str);
int sha1(char* str, unsigned char buff[20]);

#ifdef VLCRYPTO_EXPOSE_LIBRARY_FUNCTIONS
/* defined in base64.c */
int base64_lencode(const char* source, size_t sourcelen, char *target, size_t targetlen);
size_t base64_ldecode(const char* source, char *target, size_t targetlen);
#endif

#ifdef __cplusplus
}
#endif

#endif
