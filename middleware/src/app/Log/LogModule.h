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

/* ���á����ģ��flag���������տͻ���Ҫ��ģ����顣*/
int setModuleFlag(const char*, int);
int clearModuleFlag(const char*, unsigned int);

/* ����ָ��ģ��Ĵ�ӡ�ȼ���*/
int setModuleLevel(const char*, int);
int getModuleLevel(const char*);

/* ����ĳһ��ģ��Ĵ�ӡ�ȼ�������ģ�鶼����ָ����flag��*/
void setModulesLevel(int, int);
void clearModulesLevel();
char* getModulesNames(char* names, int len);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _LogModule_H_
