#ifndef _FILE_H_
#define _FILE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>

#ifndef _HAVE_FILE_TYPE
#define _HAVE_FILE_TYPE
typedef struct file_s {
	struct file_s *prev;
	struct file_s *next;
	char* path;
	char* name;
	unsigned char* data;
	int size;
	int len;
	int pos;
	int modified;
} file_t;
#endif

/* defined in file.c */
file_t *file_load(char* type, char *name);
file_t *file_create(char* type, char *name);
void file_free(file_t *file);
void file_flush(file_t *file);
int file_find(file_t *file, int offset, unsigned char value);
int file_strfind(file_t *file, int offset, char* value);
int file_read(file_t *file, void* dst, int size);
int file_readline(file_t *file, char* dst, int size);
int file_read_int(file_t *file);
int16_t file_read_short(file_t *file);
char file_read_char(file_t *file);
uint32_t file_read_uint(file_t *file);
float file_read_float(file_t *file);
int file_seek(file_t *file, int offset, int origin);
int file_tell(file_t *file);
void *file_get(file_t *file);
int file_write(file_t *file, void *buff, int size);
int file_writef(file_t *file, char* fmt, ...);

#ifdef __cplusplus
}
#endif

#endif
