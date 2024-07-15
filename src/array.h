#ifndef _ARRAY_H_
#define _ARRAY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#ifndef _HAVE_ARRAY_TYPE
#define _HAVE_ARRAY_TYPE
typedef struct array_s {
        uint32_t type;
        uint32_t length;
        uint32_t size;
        void *data;
} array_t;
#endif


#define ARRAY_TYPE_NONE		0
#define ARRAY_TYPE_INT		1
#define ARRAY_TYPE_STRING	2
#define ARRAY_TYPE_FLOAT	3
#define ARRAY_TYPE_PTR		4

/* defined in array.c */
void array_init(array_t *a, uint32_t type);
array_t *array_create(uint32_t type);
array_t *array_copy(array_t *a);
int array_cmp(array_t *a1, array_t *a2);
void array_free(array_t *a, int freestruct);
int array_push_int(array_t *a, uint32_t v);
int array_push_float(array_t *a, float v);
int array_push_string(array_t *a, char* v);
int array_push_ptr(array_t *a, void *v);
int array_push_unique_ptr(array_t *a, void *v);
/*
int array_push_colour(array_t *a, colour_t *c);
int array_push_v3t(array_t *a, v3_t *v);
int array_push_v2t(array_t *a, v2_t *v);
*/
int array_set_int(array_t *a, uint32_t v, int i);
int array_set_float(array_t *a, float v, int i);
int array_set_string(array_t *a, char* v, int i);
int array_set_ptr(array_t *a, void* v, int i);
int array_insert_ptr(array_t *a, void *v);
uint32_t array_pop_int(array_t *a);
float array_pop_float(array_t *a);
char* array_pop_string(array_t *a);
void *array_pop_ptr(array_t *a);
uint32_t array_get_int(array_t *a, int i);
float array_get_float(array_t *a, int i);
char* array_get_string(array_t *a, int i);
void *array_get_ptr(array_t *a, int i);
int array_find_int(array_t *a, uint32_t v);
int array_find_float(array_t *a, float v);
int array_find_string(array_t *a, char* v);
int array_find_ptr(array_t *a, void *v);
int array_remove_string(array_t *a, char* v);
array_t *array_split(char* str, char* s, int strings);
char* array_join(array_t *a, char* glue, int start);

#ifdef __cplusplus
}
#endif

#endif
