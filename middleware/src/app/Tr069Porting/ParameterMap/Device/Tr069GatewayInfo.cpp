#include "Tr069GatewayInfo.h"

#include "Tr069GroupCall.h"
#include "Tr069FunctionCall.h"

/*------------------------------------------------------------------------------
	相关网关设备的OUI。
 ------------------------------------------------------------------------------*/
static 
int tr069_ManufacturerOUI_Read(char *str, unsigned int length)
{
    return 0;
}

static 
int tr069_ManufacturerOUI_Write(char *str, unsigned int length)
{
    return 0;
}

/*------------------------------------------------------------------------------
	用于标识相关网关设备的产品类型。
 ------------------------------------------------------------------------------*/
static 
int tr069_ProductClass_Read(char *str, unsigned int length)
{
    return 0;
}
static 
int tr069_ProductClass_Write(char *str, unsigned int length)
{
    return 0;
}

/*------------------------------------------------------------------------------
	用于标识相关网关设备的序列号。
 ------------------------------------------------------------------------------*/
static 
int tr069_SerialNumber_Read(char *str, unsigned int length)
{
    return 0;
}

static 
int tr069_SerialNumber_Write(char *str, unsigned int length)
{
    return 0;
}

Tr069GatewayInfo::Tr069GatewayInfo()
	: Tr069GroupCall("GatewayInfo")
{   

    Tr069Call* oui  = new Tr069FunctionCall("ManufacturerOUI",       tr069_ManufacturerOUI_Read,        tr069_ManufacturerOUI_Write);
    Tr069Call* product  = new Tr069FunctionCall("ProductClass",      tr069_ProductClass_Read,           tr069_ProductClass_Write);
    Tr069Call* serial  = new Tr069FunctionCall("SerialNumber",       tr069_SerialNumber_Read,           tr069_SerialNumber_Write);
   
    regist(oui->name(), oui);
    regist(product->name(), product);
    regist(serial->name(), serial);

}

Tr069GatewayInfo::~Tr069GatewayInfo()
{
}
