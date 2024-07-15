#ifndef _CONTENT_LIST_H
#define _CONTENT_LIST_H

#ifdef __cplusplus
extern "C" {
#endif

#include "array.h"

#include <stdint.h>

#ifndef _HAVE_CONTENT_TYPE
#define _HAVE_CONTENT_TYPE
typedef uint16_t content_t;
#endif

#ifndef _HAVE_LISTDATA_TYPE
#define _HAVE_LISTDATA_TYPE
typedef struct listdata_s {
	struct listdata_s *prev;
	struct listdata_s *next;
	content_t content;
	uint16_t count;
	uint16_t data;
} listdata_t;
#endif

#ifndef _HAVE_CONTENTLIST_TYPE
#define _HAVE_CONTENTLIST_TYPE
typedef struct contentlist_s {
	struct contentlist_s *prev;
	struct contentlist_s *next;
	char* name;
	uint32_t h;
	uint32_t count;
	listdata_t *data;
} contentlist_t;
#endif

void content_list_add(const char* name, content_t c, uint16_t count, uint16_t data);
contentlist_t *content_list_get(const char* name);

#ifdef __cplusplus
}
#endif

#endif
