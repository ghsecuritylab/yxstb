
#ifdef  __cplusplus
extern "C" {
#endif

#include <stdio.h>

typedef  int (*proxy_get_value_func)(char *name, char *str, unsigned int size); 
typedef  void (*proxy_set_value_func)(char *name, char *str, unsigned int val); 

proxy_get_value_func proxy_get_value_call_get();
proxy_set_value_func proxy_set_value_call_get();
proxy_set_value_func proxySendToMonitor();


typedef int (*nm_io_callback)( const char * szParm, char * pBuf, int iLen);
typedef char* (*nm_notify_callback)(int eMsgType, const char* szParm1, const char* szParm2);
typedef void (*nm_log_export_callback)(const char* pszFile, int lLine, const char* pszFunc, int lThreadId, int enLogType, int enLogLevel, const char* pszModule, const char* format, ...);
typedef void (*get_value_func)(char *name, char *str, unsigned int pval);
typedef void (*set_value_func)(char *name, char *str, unsigned int pval);
typedef void (*get_capability_func)(char *capability);

int nm_read_callback_set(nm_io_callback read_callback);
int nm_write_callback_set(nm_io_callback write_callback);
int nm_notify_callback_set(nm_notify_callback pfnCallback);
int nm_log_callback_set(nm_log_export_callback  pfnCallback);
int nm_getvalue_callback_set(get_value_func pfnCallback);
int nm_setvalue_callback_set(set_value_func pfnCallback);
int nm_capability_callback_set(get_capability_func pfnCallback);
int nm_interface_init();
#ifdef  __cplusplus
}
#endif
