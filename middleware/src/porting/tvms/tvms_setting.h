#ifndef _TVMS_SETTING_H__
#define _TVMS_SETTING_H__


#ifdef __cplusplus
extern "C" {
#endif
void tvms_config_init(void);
void tvms_config_load(int reset);
void tvms_config_save(void);
int 	tvms_conf_tvmsgwip_set( const char *buff);
int 	tvms_conf_tvmsheartbiturl_set( const char *buff);
int     tvms_conf_tvmsgwip_get(char *buff);
int 	tvms_conf_tvmsheartbiturl_get(char *buff);
int 	tvms_conf_tvmsvodheartbiturl_set( const char *buff);
int 	tvms_conf_tvmsvodheartbiturl_get(char *buff);
int 	tvms_conf_tvmsheartbitinterval_set(int tvmsheartbitinterval);
int 	tvms_conf_tvmsheartbitinterval_get(int *tvmsheartbitinterval);
int 	tvms_conf_tvmsdelaylength_set(int delaylength);
int 	tvms_conf_tvmsdelaylength_get(int *delaylength);
void tvms_mediaurl_mediacode(char *mediacode, const char * mediaurl);


#ifdef __cplusplus
}
#endif

#endif
