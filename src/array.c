/************************************************************************
* array.c
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

#include "common.h"
#include "array.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>


static int array_next_size(int s)
{
	s /= 16;
	s = (s+1)*16;
	return s;
}

/* initialises an array - useful for those created on the stack */
void array_init(array_t *a, uint32_t type)
{
	a->type = type;
	a->length = 0;
	a->size = 0;
	a->data = NULL;
}

/* create a new array of type */
array_t *array_create(uint32_t type)
{
	array_t *a;

	a = malloc(sizeof(array_t));
	a->type = type;
	a->length = 0;
	a->size = 0;
	a->data = NULL;

	return a;
}

/* create a new array as a copy of a */
array_t *array_copy(array_t *a)
{
	array_t *r = array_create(ARRAY_TYPE_STRING);

	if (a) {
		int i;
		r->type = a->type;
		switch (a->type) {
		case ARRAY_TYPE_STRING:
			for (i=0; i<a->length; i++) {
				array_push_string(r,((char**)(a->data))[i]);
			}
			break;
		case ARRAY_TYPE_FLOAT:
			for (i=0; i<a->length; i++) {
				array_push_float(r,((float*)(a->data))[i]);
			}
			break;
		case ARRAY_TYPE_INT:
			for (i=0; i<a->length; i++) {
				array_push_int(r,((uint32_t*)(a->data))[i]);
			}
			break;
		case ARRAY_TYPE_PTR:
			for (i=0; i<a->length; i++) {
				array_push_ptr(r,((uint8_t**)(a->data))[i]);
			}
			break;
		default:;
		}
	}

	return r;
}

/* compare two arrays */
int array_cmp(array_t *a1, array_t *a2)
{
	int i;
	char** c1;
	char** c2;
	int *i1;
	int *i2;
	float *f1;
	float *f2;
	if (!a1 || !a2)
		return -1;
	if (a1->length != a2->length)
		return -1;

	if (a1->type == ARRAY_TYPE_STRING && a2->type == ARRAY_TYPE_STRING) {
		c1 = a1->data;
		c2 = a2->data;
		for (i=0; i<a1->length; i++) {
			if (!c1[i] || !c2[i] || strcmp(c1[i],c2[i]))
				return 1;
		}
	}else if (a1->type == ARRAY_TYPE_STRING && a2->type == ARRAY_TYPE_INT) {
		c1 = a1->data;
		i2 = a2->data;
		for (i=0; i<a1->length; i++) {
			if (!c1[i] || i2[i] != strtol(c1[i],NULL,10))
				return 1;
		}
	}else if (a1->type == ARRAY_TYPE_STRING && a2->type == ARRAY_TYPE_FLOAT) {
		c1 = a1->data;
		f2 = a2->data;
		for (i=0; i<a1->length; i++) {
			if (!c1[i] || f2[i] != strtof(c1[i],NULL))
				return 1;
		}
	}else if (a1->type == ARRAY_TYPE_INT && a2->type == ARRAY_TYPE_STRING) {
		i1 = a1->data;
		c2 = a2->data;
		for (i=0; i<a1->length; i++) {
			if (!c2[i] || i1[i] != strtol(c2[i],NULL,10))
				return 1;
		}
	}else if (a1->type == ARRAY_TYPE_FLOAT && a2->type == ARRAY_TYPE_STRING) {
		f1 = a1->data;
		c2 = a2->data;
		for (i=0; i<a1->length; i++) {
			if (!c2[i] || f1[i] != strtof(c2[i],NULL))
				return 1;
		}
	}else if (a1->type == ARRAY_TYPE_INT && a2->type == ARRAY_TYPE_INT) {
		i1 = a1->data;
		i2 = a2->data;
		for (i=0; i<a1->length; i++) {
			if (i1[i] != i2[i])
				return 1;
		}
	}else if (a1->type == ARRAY_TYPE_FLOAT && a2->type == ARRAY_TYPE_FLOAT) {
		f1 = a1->data;
		f2 = a2->data;
		for (i=0; i<a1->length; i++) {
			if (f1[i] != f2[i])
				return 1;
		}
	}else if (a1->type == ARRAY_TYPE_INT && a2->type == ARRAY_TYPE_FLOAT) {
		i1 = a1->data;
		f2 = a2->data;
		for (i=0; i<a1->length; i++) {
			if (i1[i] != (int)f2[i])
				return 1;
		}
	}else if (a1->type == ARRAY_TYPE_FLOAT && a2->type == ARRAY_TYPE_INT) {
		f1 = a1->data;
		i2 = a2->data;
		for (i=0; i<a1->length; i++) {
			if ((int)f1[i] != i2[i])
				return 1;
		}
	}else if (a1->type == ARRAY_TYPE_PTR && a2->type == ARRAY_TYPE_PTR) {
		c1 = a1->data;
		c2 = a2->data;
		for (i=0; i<a1->length; i++) {
			if (c1[i] != c2[i])
				return 1;
		}
	}

	return 0;
}

/* destroy an array */
void array_free(array_t *a, int freestruct)
{
	int i;
	if (!a)
		return;
	if (a->type == ARRAY_TYPE_STRING) {
		char** p = a->data;
		for (i=0; i<a->length; i++ ) {
			if (p[i])
				free(p[i]);
		}
	}
	free(a->data);
	if (freestruct)
		free(a);
}

/* push an int onto an array */
int array_push_int(array_t *a, uint32_t v)
{
	uint32_t *p;
	if (a->type == ARRAY_TYPE_STRING) {
		char sv[20];
		sprintf(sv,"%u",v);
		return array_push_string(a,sv);
	}else if (a->type == ARRAY_TYPE_FLOAT) {
		return array_push_float(a,(float)v);
	}else if (a->type != ARRAY_TYPE_INT) {
		return 1;
	}

	a->length++;

	p = a->data;

	if (a->size < a->length) {
		int s = array_next_size(a->length);
		p = realloc(a->data,sizeof(uint32_t)*s);
		if (!p) {
			a->length--;
			return 1;
		}
		a->size = s;
	}

	p[a->length-1] = v;
	a->data = p;

	return 0;
}

/* push a float onto an array */
int array_push_float(array_t *a, float v)
{
	float *p;
	if (a->type == ARRAY_TYPE_STRING) {
		char sv[20];
		sprintf(sv,"%f",v);
		return array_push_string(a,sv);
	}else if (a->type == ARRAY_TYPE_INT) {
		return array_push_int(a,(int)v);
	}else if (a->type != ARRAY_TYPE_FLOAT) {
		return 1;
	}

	a->length++;

	p = a->data;

	if (a->size < a->length) {
		int s = array_next_size(a->length);
		p = realloc(a->data,sizeof(float)*s);
		if (!p) {
			a->length--;
			return 1;
		}
		a->size = s;
	}

	p[a->length-1] = v;
	a->data = p;

	return 0;
}

/* push a string onto an array */
int array_push_string(array_t *a, char* v)
{
	char** p;
	if (a->type != ARRAY_TYPE_STRING)
		return 1;

	a->length++;

	p = a->data;

	if (a->size < a->length) {
		int s = array_next_size(a->length);
		p = realloc(a->data,sizeof(char*)*s);
		if (!p) {
			a->length--;
			return 1;
		}
		a->size = s;
	}

	if (v) {
		p[a->length-1] = strdup(v);
	}else{
		p[a->length-1] = NULL;
	}
	a->data = p;

	return 0;
}

/* push a pointer onto an array */
int array_push_ptr(array_t *a, void *v)
{
	char** p;
	if (a->type != ARRAY_TYPE_PTR)
		return 1;

	a->length++;

	p = a->data;

	if (a->size < a->length) {
		int s = array_next_size(a->length);
		p = realloc(a->data,sizeof(char*)*s);
		if (!p) {
			a->length--;
			return 1;
		}
		a->size = s;
	}

	p[a->length-1] = v;

	a->data = p;

	return 0;
}

/* push a pointer onto an array if it isn't already in there */
int array_push_unique_ptr(array_t *a, void *v)
{
	if (array_find_ptr(a,v) > -1)
		return 0;

	return array_push_ptr(a,v);
}

/* push a colour onto an array *
int array_push_colour(array_t *a, colour_t *c)
{
	int r = 0;
	r += array_push_float(a,((float)c->r)/255.0);
	r += array_push_float(a,((float)c->g)/255.0);
	r += array_push_float(a,((float)c->b)/255.0);
	r += array_push_float(a,((float)c->a)/255.0);
	return r;
}

* push a v3_t onto an array *
int array_push_v3t(array_t *a, v3_t *v)
{
	int r = 0;
	r += array_push_float(a,v->x);
	r += array_push_float(a,v->y);
	r += array_push_float(a,v->z);
	return r;
}

* push a v2_t onto an array *
int array_push_v2t(array_t *a, v2_t *v)
{
	int r = 0;
	r += array_push_float(a,v->x);
	r += array_push_float(a,v->y);
	return r;
}

* set the value of array index i to an int value */
int array_set_int(array_t *a, uint32_t v, int i)
{
	uint32_t *p = a->data;
	if (a->type == ARRAY_TYPE_STRING) {
		char sv[20];
		sprintf(sv,"%u",v);
		return array_set_string(a,sv,i);
	}else if (a->type == ARRAY_TYPE_FLOAT) {
		return array_set_float(a,(float)v,i);
	}else if (a->type != ARRAY_TYPE_INT) {
		return 1;
	}

	if (a->size <= i) {
		int k;
		int l = array_next_size(i+1);

		p = realloc(a->data,sizeof(uint32_t)*l);
		if (!p)
			return 1;
		for (k=a->length; k<l; k++) {
			p[k] = 0;
		}
		a->data = p;
		a->size = l;
	}

	if (a->length <= i)
		a->length = i+1;

	p[i] = v;

	return 0;
}

/* set the value of array index i to a float value */
int array_set_float(array_t *a, float v, int i)
{
	float *p = a->data;
	if (a->type == ARRAY_TYPE_STRING) {
		char sv[20];
		sprintf(sv,"%f",v);
		return array_set_string(a,sv,i);
	}else if (a->type == ARRAY_TYPE_INT) {
		return array_set_float(a,(uint32_t)v,i);
	}else if (a->type != ARRAY_TYPE_FLOAT) {
		return 1;
	}

	if (a->size <= i) {
		int k;
		int l = array_next_size(i+1);

		p = realloc(a->data,sizeof(float)*l);
		if (!p)
			return 1;
		for (k=a->length; k<l; k++) {
			p[k] = 0.0;
		}
		a->data = p;
		a->size = l;
	}
	if (a->length <= i)
		a->length = i+1;

	p[i] = v;

	return 0;
}

/* set the value of array index i to a string value */
int array_set_string(array_t *a, char* v, int i)
{
	char** p = a->data;
	if (a->type != ARRAY_TYPE_STRING)
		return 1;

	if (a->size <= i) {
		int k;
		int l = array_next_size(i+1);

		p = realloc(a->data,sizeof(char*)*l);
		if (!p)
			return 1;
		for (k=a->length; k<l; k++) {
			p[k] = NULL;
		}
		a->data = p;
		a->size = l;
	}
	if (a->length <= i)
		a->length = i+1;

	if (p[i])
		free(p[i]);

	if (v) {
		p[i] = strdup(v);
	}else{
		p[i] = NULL;
	}

	return 0;
}

/* set the value of array index i to a ponter value */
int array_set_ptr(array_t *a, void* v, int i)
{
	char** p = a->data;
	if (a->type != ARRAY_TYPE_PTR)
		return 1;

	if (a->size <= i) {
		int k;
		int l = array_next_size(i+1);

		p = realloc(a->data,sizeof(char*)*l);
		if (!p)
			return 1;
		for (k=a->length; k<l; k++) {
			p[k] = NULL;
		}
		a->data = p;
		a->size = l;
	}
	if (a->length <= i)
		a->length = i+1;

	p[i] = v;

	return 0;
}

/* insert a pointer onto the first NULL index of an array */
int array_insert_ptr(array_t *a, void *v)
{
	int i;
	uint8_t** p;

	if (a->type != ARRAY_TYPE_PTR)
		return 1;

	p = a->data;

	for (i=0; i<a->length; i++) {
		if (!p[i]) {
			p[i] = v;
			return 0;
		}
	}

	return array_push_ptr(a,v);
}

/* pop an int off the top of an array */
uint32_t array_pop_int(array_t *a)
{
	if (a->type == ARRAY_TYPE_INT) {
		if (!a->length)
			return 0;
		a->length--;
		return ((uint32_t*)(a->data))[a->length];
	}else if (a->type == ARRAY_TYPE_FLOAT) {
		float v = array_pop_float(a);
		return (uint32_t)v;
	}else if (a->type == ARRAY_TYPE_STRING) {
		uint32_t v;
		char* s = array_pop_string(a);
		v = strtol(s,NULL,10);
		free(s);
		return v;
	}
	return 0;
}

/* pop a float off the top of an array */
float array_pop_float(array_t *a)
{
	if (a->type == ARRAY_TYPE_FLOAT) {
		if (!a->length)
			return 0;
		a->length--;
		return ((float*)(a->data))[a->length];
	}else if (a->type == ARRAY_TYPE_INT) {
		uint32_t v = array_pop_int(a);
		return (float)v;
	}else if (a->type == ARRAY_TYPE_STRING) {
		float v;
		char* s = array_pop_string(a);
		v = strtof(s,NULL);
		free(s);
		return v;
	}
	return 0.0;
}

/* pop a string off the top of an array */
char* array_pop_string(array_t *a)
{
	if (a->type == ARRAY_TYPE_FLOAT) {
		float v = array_pop_float(a);
		char sv[20];
		if (!a->length)
			return NULL;
		sprintf(sv,"%f",v);
		return strdup(sv);
	}else if (a->type == ARRAY_TYPE_INT) {
		uint32_t v = array_pop_int(a);
		char sv[20];
		if (!a->length)
			return NULL;
		sprintf(sv,"%u",v);
		return strdup(sv);
	}else if (a->type == ARRAY_TYPE_STRING) {
		if (!a->length)
			return NULL;
		a->length--;
		return ((char**)(a->data))[a->length];
	}
	return NULL;
}

/* pop a pointer off the top of an array */
void *array_pop_ptr(array_t *a)
{
	if (a->type == ARRAY_TYPE_PTR && a->length) {
		a->length--;
		return ((char**)(a->data))[a->length];
	}
	return NULL;
}

/* get an int value from an array */
uint32_t array_get_int(array_t *a, int i)
{
	if (a->type == ARRAY_TYPE_INT) {
		if (a->length <= i)
			return 0;
		return ((uint32_t*)(a->data))[i];
	}else if (a->type == ARRAY_TYPE_FLOAT) {
		float v = array_get_float(a,i);
		return (uint32_t)v;
	}else if (a->type == ARRAY_TYPE_STRING) {
		uint32_t v;
		char* s = array_get_string(a,i);
		v = strtol(s,NULL,10);
		free(s);
		return v;
	}
	return 0;
}

/* get a float value from an array */
float array_get_float(array_t *a, int i)
{
	if (a->type == ARRAY_TYPE_FLOAT) {
		if (a->length <= i)
			return 0.0;
		return ((float*)(a->data))[i];
	}else if (a->type == ARRAY_TYPE_INT) {
		uint32_t v = array_get_int(a,i);
		return (float)v;
	}else if (a->type == ARRAY_TYPE_STRING) {
		float v;
		char* s = array_get_string(a,i);
		v = strtof(s,NULL);
		free(s);
		return v;
	}
	return 0.0;
}

/* get a string value from an array */
char* array_get_string(array_t *a, int i)
{
	if (a->type == ARRAY_TYPE_FLOAT) {
		float v = array_get_float(a,i);
		char sv[20];
		sprintf(sv,"%f",v);
		return strdup(sv);
	}else if (a->type == ARRAY_TYPE_INT) {
		uint32_t v = array_get_int(a,i);
		char sv[20];
		sprintf(sv,"%u",v);
		return strdup(sv);
	}else if (a->type == ARRAY_TYPE_STRING) {
		if (a->length <= i)
			return NULL;
		return ((char**)(a->data))[i];
	}
	return NULL;
}

/* get a pointer from an array */
void *array_get_ptr(array_t *a, int i)
{
	if (a->type == ARRAY_TYPE_PTR && a->length > i) {
		return ((char**)(a->data))[i];
	}
	return NULL;
}

/* find the index of an int value in an array */
int array_find_int(array_t *a, uint32_t v)
{
	if (a->type == ARRAY_TYPE_INT) {
		int i;
		uint32_t *p = a->data;
		for (i=0; i<a->length; i++) {
			if (p[i] == v)
				return i;
		}
	}
	return -1;
}

/* find the index of a float value in an array */
int array_find_float(array_t *a, float v)
{
	if (a->type == ARRAY_TYPE_FLOAT) {
		int i;
		float *p = a->data;
		for (i=0; i<a->length; i++) {
			if (p[i] == v)
				return i;
		}
	}
	return -1;
}

/* find the index of a string value in an array */
int array_find_string(array_t *a, char* v)
{
	if (a->type == ARRAY_TYPE_STRING) {
		int i;
		char** p = a->data;
		for (i=0; i<a->length; i++) {
			if (!strcmp(p[i],v))
				return i;
		}
	}
	return -1;
}

/* find the index of a pointer in an array */
int array_find_ptr(array_t *a, void *v)
{
	if (a->type == ARRAY_TYPE_PTR) {
		int i;
		uint8_t* cv = v;
		uint8_t** p = a->data;
		for (i=0; i<a->length; i++) {
			if (p[i] == cv)
				return i;
		}
	}
	return -1;
}

/* remove a string value from an array */
int array_remove_string(array_t *a, char* v)
{
	if (a->type == ARRAY_TYPE_STRING) {
		int i;
		char** p = a->data;
		for (i=0; i<a->length; i++) {
			if (!strcmp(p[i],v)) {
				free(p[i]);
				break;
			}
		}
		a->length--;
		for (; i<a->length; i++) {
			p[i] = p[i+1];
		}
		return 0;
	}
	return 1;
}

/* split a string into an array, at a separator character */
/* TODO: if strings is non-zero, then don't split within "" or '' */
/* TODO: utf8 support */
array_t *array_split(char* str, char* s, int strings)
{
	char buff[1024];
	int i;
	int o = 0;
	int l;
	array_t *r;

	if (!str)
		return NULL;

	r = array_create(ARRAY_TYPE_STRING);

	l = strlen(s);

	for (i=0; str[i]; i++) {
		if (!strncmp(str+i,s,l)) {
			buff[o] = 0;
			array_push_string(r,buff);
			o = 0;
			i += l-1;
			continue;
		}
		buff[o++] = str[i];
	}
	buff[o] = 0;
	array_push_string(r,buff);

	return r;
}

/* join an array into a string, using a glue character */
char* array_join(array_t *a, char* glue, int start)
{
	char* str = NULL;
	int l = 0;
	if (!glue)
		glue = " ";
	if (a->type == ARRAY_TYPE_STRING) {
		int i;
		int gl = strlen(glue);
		for (i=start; i<a->length; i++) {
			l += strlen(((char**)(a->data))[i])+gl;
		}
		if (l) {
			str = malloc(l+1);
			str[0] = 0;
			for (i=start; i<a->length; i++) {
				if (i > start)
					strcat(str,glue);
				strcat(str,((char**)(a->data))[i]);
			}
		}
	}

	return str;
}
