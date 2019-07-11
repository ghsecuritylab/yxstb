#include "Tr069IMSInfo.h"

#include "Tr069FunctionCall.h"


#include <string.h>


/*------------------------------------------------------------------------------
 ------------------------------------------------------------------------------*/
static int getTr069PortSIPProxy(char* value, unsigned int size)
{
    if (size <= strlen("http://www.baidu.com/")) 
        return -1;    
    strcpy(value, "http://www.baidu.com/");
     
    return 0;
}

static int setTr069PortSIPProxy(char* value, unsigned int size)
{ 
    return 0;
}

/*------------------------------------------------------------------------------
 ------------------------------------------------------------------------------*/
static int getTr069PortSIPProxy_Port(char* value, unsigned int size)
{
    if (size <= strlen("5000")) 
        return -1;    
    strcpy(value, "5000");	
	
    return 0;
}

static int setTr069PortSIPProxy_Port(char* value, unsigned int size)
{ 
    return 0;
}


/*------------------------------------------------------------------------------
 ------------------------------------------------------------------------------*/
static int getTr069PortBACKUP_Proxy(char* value, unsigned int size)
{
    if (size <= strlen("http://www.google.com/")) 
        return -1;    
    strcpy(value, "http://www.google.com/");	
    
    return 0;
}

static int setTr069PortBACKUP_Proxy(char* value, unsigned int size)
{
    return 0;
}


/*------------------------------------------------------------------------------
 ------------------------------------------------------------------------------*/
static int getTr069PortBACKUP_Proxy_Port(char* value, unsigned int size)
{
    if (size <= strlen("5001")) 
        return -1;    
    strcpy(value, "5001");	
    
    return 0;
}

static int setTr069PortBACKUP_Proxy_Port(char* value, unsigned int size)
{ 
    return 0;
}


/*------------------------------------------------------------------------------
 ------------------------------------------------------------------------------*/
static int getTr069PortSIPAccount(char* value, unsigned int size)
{
    if (size <= strlen("admin")) 
        return -1;    
    strcpy(value, "admin");	
    
    return 0;
}

static int setTr069PortSIPAccount(char* value, unsigned int size)
{  
    return 0;
}


/*------------------------------------------------------------------------------
 ------------------------------------------------------------------------------*/
static int getTr069PortSIPPassword(char* value, unsigned int size)
{
    if (size <= strlen("123456")) 
        return -1;    
    strcpy(value, "123456");	

    return 0;
}

static int setTr069PortSIPPassword(char* value, unsigned int size)
{
    return 0;
}



/*------------------------------------------------------------------------------
 ------------------------------------------------------------------------------*/
static int getTr069PortImsDomain(char* value, unsigned int size)
{
    if (size <= strlen("http://www.xiangyang.com/")) 
        return -1;    
    strcpy(value, "http://www.xiangyang.com/");	
     
    return 0;
}

static int setTr069PortImsDomain(char* value, unsigned int size)
{  
    return 0;
}



/*------------------------------------------------------------------------------
 ------------------------------------------------------------------------------*/
static int getTr069PortBACDomainName(char* value, unsigned int size)
{
    if (size <= strlen("")) 
        return -1;    
    strcpy(value, "");	
 
    return 0;
}

static int setTr069PortBACDomainName(char* value, unsigned int size)
{
    return 0;
}



/*------------------------------------------------------------------------------
 ------------------------------------------------------------------------------*/
static int getTr069PortBACAddress1(char* value, unsigned int size)
{
    if (size <= strlen("")) 
        return -1;    
    strcpy(value, "");	

    return 0;
}

static int setTr069PortBACAddress1(char* value, unsigned int size)
{  
    return 0;
}



/*------------------------------------------------------------------------------
 ------------------------------------------------------------------------------*/
static int getTr069PortBACAddress2(char* value, unsigned int size)
{
    if (size <= strlen("")) 
        return -1;    
    strcpy(value, "");	
    return 0;
}

static int setTr069PortBACAddress2(char* value, unsigned int size)
{
    return 0;
}



/*------------------------------------------------------------------------------
 ------------------------------------------------------------------------------*/
static int getTr069PortIMSUserID(char* value, unsigned int size)
{
    if (size <= strlen("")) 
        return -1;    
    strcpy(value, "");	

    return 0;
}

static int setTr069PortIMSUserID(char* value, unsigned int size)
{
    return 0;
}



/*------------------------------------------------------------------------------
 ------------------------------------------------------------------------------*/
static int getTr069PortIMSPassword(char* value, unsigned int size)
{
    if (size <= strlen("")) 
        return -1;    
    strcpy(value, "");	

    return 0;
}

static int setTr069PortIMSPassword(char* value, unsigned int size)
{
    return 0;
}


Tr069IMSInfo::Tr069IMSInfo()
	: Tr069GroupCall("IMSInfo")
{
	
	/* 以下对象的注册到表root.Device.X_CTC_IPTV.IMSInfo  */	

#if defined(SHANGHAI_HD)||defined(SHANGHAI_SD)

    Tr069Call* fun1  = new Tr069FunctionCall("SIPProxy", getTr069PortSIPProxy, setTr069PortSIPProxy);
    regist(fun1->name(), fun1);    

  
    Tr069Call* fun2  = new Tr069FunctionCall("SIPProxy_Port", getTr069PortSIPProxy_Port, setTr069PortSIPProxy_Port);
    regist(fun2->name(), fun2);    
    
    Tr069Call* fun3  = new Tr069FunctionCall("BACKUP_Proxy", getTr069PortBACKUP_Proxy, setTr069PortBACKUP_Proxy);
    regist(fun3->name(), fun3);    
    
    Tr069Call* fun4  = new Tr069FunctionCall("BACKUP_Proxy_Port", getTr069PortBACKUP_Proxy_Port, setTr069PortBACKUP_Proxy_Port);
    regist(fun4->name(), fun4);    
    

    Tr069Call* fun5  = new Tr069FunctionCall("SIPAccount", getTr069PortSIPAccount, setTr069PortSIPAccount);
    regist(fun5->name(), fun5);    
    
    Tr069Call* fun6  = new Tr069FunctionCall("SIPPassword", getTr069PortSIPPassword, setTr069PortSIPPassword);
    regist(fun6->name(), fun6);    
    
    Tr069Call* fun7  = new Tr069FunctionCall("ImsDomain", getTr069PortImsDomain, setTr069PortImsDomain);
    regist(fun7->name(), fun7);    
#else
 
    Tr069Call* fun8  = new Tr069FunctionCall("BACDomainName", getTr069PortBACDomainName, setTr069PortBACDomainName);
    regist(fun8->name(), fun8);       
    
    Tr069Call* fun9  = new Tr069FunctionCall("BACAddress1", getTr069PortBACAddress1, setTr069PortBACAddress1);
    regist(fun9->name(), fun9);    
    
    Tr069Call* fun10  = new Tr069FunctionCall("BACAddress2", getTr069PortBACAddress2, setTr069PortBACAddress2);
    regist(fun10->name(), fun10);    
    
    Tr069Call* fun11  = new Tr069FunctionCall("UserID", getTr069PortIMSUserID, setTr069PortIMSUserID);
    regist(fun11->name(), fun11);    
    
    Tr069Call* fun12  = new Tr069FunctionCall("Password", getTr069PortIMSPassword, setTr069PortIMSPassword);
    regist(fun12->name(), fun12);    

#endif

}

Tr069IMSInfo::~Tr069IMSInfo()
{
}
