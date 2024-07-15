#ifndef _THREAD_H_
#define _THREAD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "array.h"

#ifdef WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

#ifndef _HAVE_THREADID_TYPE
#define _HAVE_THREADID_TYPE
#ifdef WIN32
typedef DWORD threadid_t;
#else
typedef pthread_t threadid_t;
#endif
#endif

#ifndef _HAVE_THREAD_TYPE
#define _HAVE_THREAD_TYPE
#include "array.h"
typedef struct thread_s {
	struct thread_s *prev;
	struct thread_s *next;
#ifndef WIN32
	pthread_t thread;
	pthread_attr_t attr;
#else
	HANDLE thread;
#endif
	unsigned int state;
	int exit;
	void *(*func)();
	array_t *args;
} thread_t;
#endif

#ifndef _HAVE_MUTEX_TYPE
#define _HAVE_MUTEX_TYPE
typedef struct mutex_s {
	struct mutex_s *prev;
	struct mutex_s *next;
#ifndef WIN32
	pthread_mutexattr_t attr;
	pthread_mutex_t mut;
#else
	CRITICAL_SECTION mut;
#endif
	threadid_t id;
	int count;
} mutex_t;
#endif

/* defined in thread.c */
int thread_init(void);
int thread_equal(threadid_t t1, threadid_t t2);
int thread_is_main(void);
thread_t *thread_create(void *(*func)(), array_t *args);
void thread_free(thread_t *t);
void thread_exit(thread_t *t, int state);
void thread_stop(thread_t *t);
int thread_wake(thread_t *t);
void thread_wait(thread_t *t);
threadid_t thread_get_current_id(void);
mutex_t *mutex_create(void);
void mutex_free(mutex_t *m);
void mutex_lock(mutex_t *m);
int mutex_trylock(mutex_t *m);
void mutex_unlock(mutex_t *m);
void mutex_unlock_complete(mutex_t *m);

#ifdef __cplusplus
}
#endif

#endif
