#include "Tr069WLAN.h"

#include "Tr069FunctionCall.h"


static int getAppTr069PortWLANuselan(char* str, unsigned int val)
{
    return 0;
}
static int setAppTr069PortWLANuselan(char* str, unsigned int val)
{
    return 0;
}

static int getAppTr069PortWLANEncryOnOff(char* str, unsigned int val)
{
    return 0;
}

static int setAppTr069PortWLANEncryOnOff(char* str, unsigned int val)
{
    return 0;
}

static int getAppTr069PortWLANEssID(char* str, unsigned int val)
{
    return 0;
}

static int setAppTr069PortWLANEssID(char* str, unsigned int val)
{
    return 0;
}

static int getAppTr069PortWLANKeyVector(char* str, unsigned int val)
{
    return 0;
}

static int setAppTr069PortWLANKeyVector(char* str, unsigned int val)
{
    return 0;
}

static int getAppTr069PortWLANRfMode(char* str, unsigned int val)
{
    return 0;
}

static int getAppTr069PortWLANEncryKeyID(char* str, unsigned int val)
{
    return 0;
}

static int setAppTr069PortWLANEncryKeyID(char* str, unsigned int val)
{
    return 0;
}

static int getAppTr069PortWLANAuthMode(char* str, unsigned int val)
{
    return 0;
}

static int setAppTr069PortWLANAuthMode(char* str, unsigned int val)
{
    return 0;
}


Tr069WLAN::Tr069WLAN()
	: Tr069GroupCall("WLAN")
{
#ifdef TR069_LIANCHUANG

    Tr069Call* UseWLAN         = new Tr069FunctionCall("UseWLAN", getAppTr069PortWLANuselan, setAppTr069PortWLANuselan);
    Tr069Call* EncryOnOff      = new Tr069FunctionCall("EncryOnOff", getAppTr069PortWLANEncryOnOff, setAppTr069PortWLANEncryOnOff);   
    Tr069Call* EssID           = new Tr069FunctionCall("EssID", getAppTr069PortWLANEssID, setAppTr069PortWLANEssID);
    Tr069Call* KeyVector       = new Tr069FunctionCall("KeyVector", getAppTr069PortWLANKeyVector, setAppTr069PortWLANKeyVector);
    Tr069Call* RfMode          = new Tr069FunctionCall("RfMode", getAppTr069PortWLANRfMode, NULL);
    Tr069Call* WLANEncryKeyID  = new Tr069FunctionCall("EncryKeyID", getAppTr069PortWLANEncryKeyID, setAppTr069PortWLANEncryKeyID);
    Tr069Call* WLANAuthMode    = new Tr069FunctionCall("AuthMode", getAppTr069PortWLANAuthMode, setAppTr069PortWLANAuthMode);

    
    regist(UseWLAN->name(), UseWLAN);
    regist(EncryOnOff->name(), EncryOnOff);
    regist(EssID->name(), EssID);
    regist(KeyVector->name(), KeyVector);
    regist(RfMode->name(), RfMode);
    regist(WLANEncryKeyID->name(), WLANEncryKeyID);
    regist(WLANAuthMode->name(), WLANAuthMode);

#endif
}

Tr069WLAN::~Tr069WLAN()
{
}
