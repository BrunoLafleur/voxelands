/************************************************************************
* file.c
* voxelands - 3d voxel world sandbox game
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
#include "thread.h"
#include "list.h"
#include "array.h"

#include <stdlib.h>
#ifndef WIN32
#include <signal.h>
#endif

#define IO_THREAD_STOPPED	0
#define IO_THREAD_RUNNING	1

static struct {
	thread_t *threads;
	mutex_t *mutexes;
	threadid_t main;
} thread_data = {
	NULL,
	NULL
};

int thread_init()
{
	thread_data.main = thread_get_current_id();

	return 0;
}

int thread_equal(threadid_t t1, threadid_t t2)
{
#ifndef WIN32
	return pthread_equal(t1,t2);
#else
	if (t1 == t2)
		return 1;
#endif
	return 0;
}

int thread_is_main()
{
	threadid_t self = thread_get_current_id();

	return thread_equal(self,thread_data.main);
}

/* create a new thread */
thread_t *thread_create(void *(*func)(), array_t *args)
{
	thread_t *t = malloc(sizeof(thread_t));

	t->state = IO_THREAD_RUNNING;
	t->func = func;
	t->args = args;
	t->exit = 0;

	thread_data.threads = list_push((void**)&thread_data.threads,t);

#ifndef WIN32
	pthread_attr_init(&t->attr);
	pthread_attr_setdetachstate(&t->attr, PTHREAD_CREATE_JOINABLE);
	pthread_create(&t->thread, &t->attr, t->func, (void *)t);
#else
	t->thread = CreateThread(NULL, 0, t->func, (void*)t, 0, NULL);
#endif

	return t;
}

/* destroy a thread */
void thread_free(thread_t *t)
{
	if (!t)
		return;

	thread_stop(t);
#ifndef WIN32
	pthread_attr_destroy(&t->attr);
#endif
	if (t->args)
		array_free(t->args,1);
	free(t);
}

/* exit from a thread */
void thread_exit(thread_t *t, int state)
{
	if (!t)
		return;

	t->exit = state;
	t->state = IO_THREAD_STOPPED;
#ifndef WIN32
	pthread_exit(NULL);
#else
	ExitThread(state);
#endif
}

/* stop a thread */
void thread_stop(thread_t *t)
{
	if (!t)
		return;

	if (t->state == IO_THREAD_RUNNING) {
		t->state = IO_THREAD_STOPPED;
#ifndef WIN32
		pthread_kill(t->thread,SIGKILL);
#else
		TerminateThread(t->thread,0);
		CloseHandle(t->thread);
#endif
	}
}

/* restart a thread */
int thread_wake(thread_t *t)
{
	if (!t)
		return 0;

	if (t->state == IO_THREAD_RUNNING) {
#ifndef WIN32
		pthread_kill(t->thread,SIGCONT);
#else
		ResumeThread(t->thread);
#endif
	}else{
		t->state = IO_THREAD_RUNNING;
#ifndef WIN32
		pthread_create(&t->thread, &t->attr, t->func, (void *)t);
#else
		t->thread = CreateThread(NULL, 0, t->func, (void*)t, 0, NULL);
#endif
	}
	return 0;
}

/* wait for a thread to exit */
void thread_wait(thread_t *t)
{
	if (!t)
		return;

	if (t->state == IO_THREAD_RUNNING) {
#ifndef WIN32
		pthread_join(t->thread,NULL);
#else
		WaitForSingleObject(t->thread, 2000);
		CloseHandle(t->thread);
#endif
	}
	t->state = IO_THREAD_STOPPED;
}

threadid_t thread_get_current_id()
{
#ifdef WIN32
	return GetCurrentThreadId();
#else
	return pthread_self();
#endif
}

/* create a mutex */
mutex_t *mutex_create()
{
	mutex_t *m = malloc(sizeof(mutex_t));

#ifndef WIN32
	pthread_mutexattr_init(&m->attr);
	pthread_mutexattr_settype(&m->attr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&m->mut, &m->attr);
#else
	InitializeCriticalSection(&m->mut);
#endif

	m->id = thread_get_current_id();
	m->count = 0;

	thread_data.mutexes = list_push((void**)&thread_data.mutexes,m);

	return m;
}

/* destroy a mutex */
void mutex_free(mutex_t *m)
{
	mutex_unlock(m);
#ifndef WIN32
	pthread_mutex_destroy(&m->mut);
	pthread_mutexattr_destroy(&m->attr);
#else
	DeleteCriticalSection(&m->mut);
#endif

	thread_data.mutexes = list_remove((void**)&thread_data.mutexes,m);
	free(m);
}

/* lock a mutex */
void mutex_lock(mutex_t *m)
{
	if (!mutex_trylock(m))
		return;

	if (m->id == thread_get_current_id()) {
		m->count++;
		return;
	}
#ifndef WIN32
	pthread_mutex_lock(&m->mut);
#else
	EnterCriticalSection(&m->mut);
#endif
	m->id = thread_get_current_id();
	m->count = 1;
}

/* try to lock a mutex - return non-zero if not locked */
int mutex_trylock(mutex_t *m)
{
#ifndef WIN32
	if (pthread_mutex_trylock(&m->mut))
		return 1;
#else
	if (!TryEnterCriticalSection(&m->mut))
		return 1;
#endif
	m->id = thread_get_current_id();
	m->count = 1;

	return 0;
}

/* unlock a mutex */
void mutex_unlock(mutex_t *m)
{
	m->count--;
	if (m->count > 0)
		return;
#ifndef WIN32
	pthread_mutex_unlock(&m->mut);
#else
	LeaveCriticalSection(&m->mut);
#endif
}

static void *mutex_test_lock_t(thread_t *t)
{
	int r = mutex_trylock(((mutex_t**)(t->args->data))[0]);
	thread_exit(t,r);
	return NULL;
}

/* try to force a mutex to unlock */
void mutex_unlock_complete(mutex_t *m)
{
	array_t *a;
	thread_t *t;

	a = array_create(ARRAY_TYPE_PTR);
	array_push_ptr(a,m);
	t = thread_create(mutex_test_lock_t,a);
	thread_wait(t);
	while (t->exit) {
		thread_wake(t);
		thread_wait(t);
	}
}
