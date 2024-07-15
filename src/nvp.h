#ifndef _NVP_H_
#define _NVP_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _HAVE_V3_TYPE
struct v3_s;
#endif

#ifndef _HAVE_NVP_TYPE
#define _HAVE_NVP_TYPE
typedef struct nvp_s {
	struct nvp_s *prev;
	struct nvp_s *next;
	char* name;
	unsigned int h;
	char* value;
	void *data;
} nvp_t;
#endif

/* defined in nvp.c */
void nvp_free(nvp_t **list, int data);
nvp_t *nvp_get(nvp_t **list,const char* name);
char* nvp_get_str(nvp_t **list,const char* name);
int nvp_get_int(nvp_t **list,const char* name);
float nvp_get_float(nvp_t **list,const char* name);
int nvp_get_bool(nvp_t **list,const char* name);
void *nvp_get_data(nvp_t **list,const char* name);
void nvp_set(nvp_t **list,const char* name,const char* value, void *data);
void nvp_set_int(nvp_t **list,const char* name, int value);
void nvp_set_float(nvp_t **list,const char* name, float value);
void nvp_set_v3t(nvp_t **list,const char* name, struct v3_s *value);
void nvp_from_str(nvp_t **list,const char* str);
int nvp_to_str(nvp_t **list, char* buff, int size);

#ifdef __cplusplus
}
#endif

#endif
