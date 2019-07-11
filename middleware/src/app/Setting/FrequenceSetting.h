#ifndef FrequenceSetting_h
#define FrequenceSetting_h

#ifdef __cplusplus
extern "C" {
#endif

void frequenceSettingInit(void);
int sysMulticastGet(int pIndex, char *buf);
int sysMulticastSet(int pIndex, char *buf);
int sysPppoeparamGet(char *buf);
int sysPppoeparamSet(char* buf);

#ifdef ENABLE_IGMPV3
int sysSrcipGet(int pIndex, char *buf);
int sysSrcipSet(int pIndex, char* buf);
#endif

#ifdef __cplusplus
}
#endif

#endif //FrequenceSetting_h
