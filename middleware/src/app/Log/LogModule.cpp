#include "Assertions.h"
#include "LogModule.h"
#include <string.h>
#include <stdio.h>
#include "ConfigFileParser.h"
#include "LogModuleHuawei.h"

static struct LogModule* g_moduleListHead = 0;

LogModule::LogModule(const char* name, int& flag, int& level)
	: m_name(name)
	, m_flag(flag)
	, m_level(level)
{
    m_next = g_moduleListHead;
    g_moduleListHead = this;
}

LogModule::~LogModule()
{
}


extern "C" 
void logModuleInit()
{
    logModuleCustomerInit();

    Hippo::ConfigFileParser cfg; //printf log when stb reboot
    int level = -1;
    if (0 == cfg.fileOpen(DEBUG_CONFIG_FILE)) {
        //field: extend
        std::string ftpserver = cfg.GetVarStr(DEBUG_FIELD_EXTEND, "ftpserver");
        if (!ftpserver.empty()) {
            huaweiSetLogFTPServer(ftpserver.c_str());
            huaweiSetLogOutPutType(1);
        }
        std::string udpserver = cfg.GetVarStr(DEBUG_FIELD_EXTEND, "udpserver");
        if (!udpserver.empty()) {
            huaweiSetLogUDPServer((char*)udpserver.c_str());
            huaweiSetLogOutPutType(2);
        }
        huaweiLog();
        //field: debug
        int style = cfg.GetVarInt(DEBUG_FIELD_NAME, "style");
        if (style != INVALID_VALUE)
            logSetExtensionStyle(style);
        /* Level */
        struct LogModule* module = g_moduleListHead;
        while (module) {
            level = cfg.GetVarInt(DEBUG_FIELD_NAME, module->m_name);
            if (level != INVALID_VALUE) 
                module->m_level = level;
            module = module->m_next;
        }
    }
}

extern "C" 
int setModuleFlag(const char* name, int flag)
{
    struct LogModule* module = g_moduleListHead;

    while (module) {
        if (!strcmp(module->m_name, name)) {
            module->m_flag |= flag;
            return 0;
        }
        module = module->m_next;
    }

    return -1;
}

extern "C" 
int clearModuleFlag(const char* name, unsigned int flag)
{
    struct LogModule* module = g_moduleListHead;

    while (module) {
        if (!strcmp(module->m_name, name)) {
            module->m_flag &= ~flag;
            return 0;
        }
        module = module->m_next;
    }

    return -1;
}

extern "C" 
int setModuleLevel(const char* name, int level)
{
    struct LogModule* module = g_moduleListHead;

    while (module) {
        if (!strcmp(module->m_name, name)) {
            int old = module->m_level;
            module->m_level = level;
            return old;
        }
        module = module->m_next;
    }

    return -1;
}

extern "C" 
int getModuleLevel(const char* name)
{
    struct LogModule* module = g_moduleListHead;

    while (module) {
        if (!strcmp(module->m_name, name))
            return module->m_level;
        module = module->m_next;
    }

    return -1;
}

extern "C"
void setModulesLevel(int flag, int level)
{
    struct LogModule* module = g_moduleListHead;

    while (module) {
        if (module->m_flag & flag)
            module->m_level = level;
        module = module->m_next;
    }
}

extern "C" 
void clearModulesLevel( )
{
    struct LogModule* module = g_moduleListHead;
    while (module) {
        module->m_level = 0;
        module = module->m_next;
    }
}

extern "C" 
char* getModulesNames(char* names, int len)
{
    if (names && len > 0) {
        int sumlen = 0;
        struct LogModule* module = g_moduleListHead;
        while (module) {
            if (len > sumlen)
                sumlen += snprintf(names + sumlen, len - sumlen, "%s ", module->m_name);
            module = module->m_next;
        }
        names[sumlen] = 0;
    }
    return names;
}

