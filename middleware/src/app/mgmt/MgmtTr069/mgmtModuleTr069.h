#ifndef _MGMTMODULETR069_H_
#define _MGMTMODULETR069_H_

#ifdef __cplusplus
#include <map>
#include <string>
#endif

typedef void (*mgmtSetIntFunc)(int value);
typedef void (*mgmtSetUintFunc)(unsigned int value);
typedef void (*mgmtSetStringFunc)(char *value);
typedef int (*mgmtGetIntFunc)(void);
typedef unsigned int (*mgmtGetUintFunc)(void);
typedef void (*mgmtGetStringFunc)(char *value, int size);

struct ParamMapFunc
{			
    union {
        mgmtGetIntFunc getint;
        mgmtGetUintFunc getuint;
        mgmtGetStringFunc getstring;
    } getfunc;

    union{
        mgmtSetIntFunc setint;
        mgmtSetUintFunc setuint;
        mgmtSetStringFunc setstring;
    } setfunc;

    int paramtype;
};

typedef enum{
    INT_TYPE,
    UINT_TYPE,
    STRING_TYPE
}tr069_param_type;

#ifdef __cplusplus
extern "C" {
#endif

int tmsReadConfig(const char * szParm, char * pBuf, int iLen);
int tmsWriteConfig(const char * szParm, char * pBuf, int iLen);

int mgmtModuleTr069Init();

#ifdef __cplusplus
}
#endif

#endif
