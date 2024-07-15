/************************************************************************
* list.c
* tausaga - environmental survival game
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

#include "list.h"

#include <stdlib.h>

/* comparison functions:
 * return 0 to insert
 * return 1 to insert later
 * return -1 to not insert at all
 */
static int list_cmp_default(void *e1, void *e2)
{
	return ((*((int*)((ref_t*)e1)->ref))<(*((int*)((ref_t*)e2)->ref)));
}

/* get the element count in a list */
int list_count(void *list)
{
	int c = 0;
	ref_t *l = *((ref_t**)list);

	while (l) {
		c++;
		l = l->next;
	}

	return c;
}

/* get the last element in a list */
void *list_last(void *list)
{
	ref_t *l = *((ref_t**)list);
	if (l) {
		while (l->next) {
			l = l->next;
		}
	}

	return l;
}

/* push an element onto a list */
void *list_push(void *list, void *el)
{
	ref_t *l = *((ref_t**)list);
	ref_t *e = el;

	if (l) {
		while (l->next) {
			l = l->next;
		}
		l->next = e;
		e->prev = l;
		e->next = NULL;
		return *((ref_t**)list);
	}

	*((ref_t**)list) = e;
	e->next = NULL;
	e->prev = NULL;

	return e;
}

/* append a list to another list */
void *list_append(void *list, void *el)
{
	ref_t *l = *((ref_t**)list);
	ref_t *e = el;

	if (l) {
		while (l->next) {
			l = l->next;
		}
		l->next = e;
		e->prev = l;
		return *((ref_t**)list);
	}

	*((ref_t**)list) = e;
	e->prev = NULL;

	return e;
}

/* shove an element onto the start of a list */
void *list_shove(void *list, void *el)
{
	ref_t *l = *((ref_t**)list);
	ref_t *e = el;

	if (l)
		l->prev = e;

	*((ref_t**)list) = e;
	e->next = l;
	e->prev = NULL;

	return e;
}

/* remove an element form a list */
void *list_remove(void *list, void *el)
{
	ref_t *l = *((ref_t**)list);
	ref_t *e = el;

	if (!e->prev) {
		*(ref_t**)list = e->next;
		l = *((ref_t**)list);
		if (l)
			l->prev = NULL;

		return l;
	}

	e->prev->next = e->next;
	if (e->next)
		e->next->prev = e->prev;

	return l;
}

/* remove the last element from a list */
void *list_pop(void *list)
{
	ref_t *e = *((ref_t**)list);

	if (!e)
		return NULL;

	while (e->next) {
		e = e->next;
	}

	if (e->prev) {
		e->prev->next = e->next;
	}else{
		*((ref_t**)list) = e->next;
	}

	if (e->next)
		e->next->prev = e->prev;

	return e;
}

/* remove the first element from a list */
void *list_pull(void *list)
{
	ref_t *e = *((ref_t**)list);

	if (!e)
		return NULL;

	*((ref_t**)list) = e->next;
	if (*((ref_t**)list))
		e->next->prev = NULL;

	return e;
}

/* insert an element into a list before ne */
void *list_insert(void *list, void *el, void *ne)
{
	ref_t *l = *((ref_t**)list);
	ref_t *e = el;
	ref_t *n = ne;

	if (n == l || !n)
		return list_shove(list,el);

	e->prev = n->prev;
	e->next = n;
	n->prev = e;

	return l;
}

/* insert an element into a list based on a compare function - return 1 to insert before */
void *list_insert_cmp(void *list, void *el, int (*list_cmp)(void *e1, void *e2))
{
	ref_t *l = *((ref_t**)list);
	ref_t *e = el;
	ref_t *p = l;
	ref_t *r = NULL;
	int c;

	if (!l)
		return list_push(list,el);

	if (!list_cmp)
		list_cmp = list_cmp_default;

	((ref_t*)el)->next = NULL;
	((ref_t*)el)->prev = NULL;

	while (p && (c = list_cmp(e,p) > 0)) {
		r = p;
		p = p->next;
	}

	if (c < 0)
		return l;

	if (!r)
		return list_shove(list,el);

	e->prev = r;
	e->next = r->next;
	r->next = e;
	if (e->next)
		e->next->prev = e;

	return l;
}

/* resort a list, based on a compare function */
void *list_resort_cmp(void *list, int (*list_cmp)(void *e1, void* e2))
{
	ref_t *l = *((ref_t**)list);
	ref_t *e = l;
	ref_t *n;

	l = NULL;

	if (!list_cmp)
		list_cmp = list_cmp_default;

	while (e) {
		n = e->next;
		l = list_insert_cmp(&l,e,list_cmp);
		e = n;
	}

	return l;
}
