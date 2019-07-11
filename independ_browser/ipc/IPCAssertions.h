#ifndef _IPCASSERTIONS_H_
#define _IPCASSERTIONS_H_

#include <string.h>

extern int gIpcModuleLevel;

#define LogIpcStack(param...) \
do{\
    if (gIpcModuleLevel == -1) {\
        printf("Stack  [%s %s:%d]: ", strrchr(__FILE__, '/') + 1, __func__, __LINE__);\
        printf(param);\
    }\
}while(0)

#define LogIpcDebug(param...) \
do{\
    if (gIpcModuleLevel >= 3) {\
        printf("Normal [%s %s:%d]: ", strrchr(__FILE__, '/') + 1, __func__, __LINE__);\
        printf(param);\
    }\
}while(0)

#define LogIpcWarn(param...) \
do{\
    if (gIpcModuleLevel >= 2) {\
        printf("Warn!  [%s %s:%d]: ", strrchr(__FILE__, '/') + 1, __func__, __LINE__);\
        printf(param);\
    }\
}while(0)

#define LogIpcError(param...) \
do{\
    if (gIpcModuleLevel >= 1) {\
        printf("Error! [%s %s:%d]: ", strrchr(__FILE__, '/') + 1, __func__, __LINE__);\
        printf(param);\
    }\
}while(0)

#define LogIpcTest(param...) \
do{\
    if (gIpcModuleLevel >= 0) {\
        printf("Test!  [%s %s:%d]: ", strrchr(__FILE__, '/') + 1, __func__, __LINE__);\
        printf(param);\
    }\
}while(0)

#endif //_IpcASSERTIONS_H_
