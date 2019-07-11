
#ifndef __extendConfig_h__
#define __extendConfig_h__

#ifdef __cplusplus
extern "C" {
#endif

void extendConfigInit(void);

int extendConfigInsetObject(const char *paramname);
int extendConfigInsetInt(const char *paramname, int *addr);
int extendConfigInsetUnsigned(const char *paramname, unsigned int *addr);
int extendConfigInsetString(const char *paramname, char *addr, int len);

void extendConfigRead(const char *rootname);
void extendConfigWrite(const char *rootname);

#ifdef __cplusplus
}
#endif

#endif//__extendConfig_h__
