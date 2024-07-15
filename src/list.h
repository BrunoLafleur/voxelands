#ifndef _LIST_H_
#define _LIST_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _HAVE_REF_TYPE
#define _HAVE_REF_TYPE
typedef struct ref_s {
	struct ref_s *prev;
	struct ref_s *next;
	void *ref;
} ref_t;
#endif

/* defined in list.c */
int list_count(void *list);
void *list_last(void *list);
void *list_push(void *list, void *el);
void *list_append(void *list, void *el);
void *list_shove(void *list, void *el);
void *list_remove(void *list, void *el);
void *list_pop(void *list);
void *list_pull(void *list);
void *list_insert(void *list, void *el, void *n);
void *list_insert_cmp(void *list, void *el, int (*list_cmp)(void *e1, void *e2));
void *list_resort_cmp(void *list, int (*list_cmp)(void *e1, void* e2));

#ifdef __cplusplus
}
#endif

#endif
