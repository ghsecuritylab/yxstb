#ifndef _LogModule_H_
#define _LogModule_H_

#ifdef __cplusplus

#include "config/pathConfig.h"

#define DEBUG_CONFIG_FILE DEFAULT_RAM_DATAPATH"/debug.ini"
#define DEBUG_FIELD_NAME "DEBUG"
#define DEBUG_FIELD_EXTEND "EXTEND"

//namespace Hippo {

struct LogModule {
	LogModule(const char*, int&, int&);
	~LogModule();

	const char* m_name;
	int&        m_flag;
	int&        m_level;
	LogModule*  m_next;
};

//} // namespace Hippo

extern "C" {
#endif // __cplusplus

void logModuleInit();
void logModuleCustomerInit();

/* 设置、清除模块flag，用来按照客户需要给模块分组。*/
int setModuleFlag(const char*, int);
int clearModuleFlag(const char*, unsigned int);

/* 设置指定模块的打印等级。*/
int setModuleLevel(const char*, int);
int getModuleLevel(const char*);

/* 设置某一类模块的打印等级，此类模块都带有指定的flag。*/
void setModulesLevel(int, int);
void clearModulesLevel();
char* getModulesNames(char* names, int len);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _LogModule_H_
