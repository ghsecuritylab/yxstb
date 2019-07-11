#include "Tr069Config.h"

#include "Tr069GroupCall.h"
#include "Tr069FunctionCall.h"
#include <string.h>

#define IND_STRCPY(dest, src)		strcpy(dest, src)

/*------------------------------------------------------------------------------
	必须贯穿机顶盒重启的任意用户数据。
 ------------------------------------------------------------------------------*/
static 
int tr069_PersistentData_Read(char *str, unsigned int length)
{
    IND_STRCPY(str, "xiangyang");
    return 0;
}

static 
int tr069_PersistentData_Write(char *str, unsigned int length)
{
    return 0;
}

static 
int tr069_ConfigFile_Read(char *str, unsigned int length)
{
    IND_STRCPY(str, "xiangyang_configfile");
    return 0;
}

static 
int tr069_ConfigFile_Write(char *str, unsigned int length)
{
    return 0;
}

Tr069Config::Tr069Config()
	: Tr069GroupCall("Config")
{   
    Tr069Call* data  = new Tr069FunctionCall("PersistentData",   tr069_PersistentData_Read,    tr069_PersistentData_Write);
    Tr069Call* file  = new Tr069FunctionCall("ConfigFile",       tr069_ConfigFile_Read,        tr069_ConfigFile_Write);

    regist(data->name(), data);
    regist(file->name(), file);
}

Tr069Config::~Tr069Config()
{
}
