#include "Tr069SQMConfiguration.h"

#include "Tr069FunctionCall.h"

#if defined(SQM_INCLUDED)
#include "sqm_port.h"
#endif

#include "Assertions.h"

#include <stdio.h>
#include <stdlib.h>

Tr069Call* g_tr69SQMConfiguration = new Tr069SQMConfiguration();


#if defined(SQM_INCLUDED)
/*------------------------------------------------------------------------------
	�õ�SQM����Զ�̵Ķ˿�
 ------------------------------------------------------------------------------*/
static int getTr069PortSQMLisenPort(char* value, unsigned int size)
{
    return -1;
}


/*------------------------------------------------------------------------------
	����SQM����Զ�̵Ķ˿�
 ------------------------------------------------------------------------------*/
static int setTr069PortSQMLisenPort(char* value, unsigned int size)
{
    LogUserOperDebug("set SQMLisenPort = %d\n", atoi(value));
	sqm_set_listen_port((unsigned int)atoi(value));

    return 0;
}



/*------------------------------------------------------------------------------
	�õ�SQM�������Ķ˿�
 ------------------------------------------------------------------------------*/
static int getTr069PortSQMServerPort(char* value, unsigned int size)
{
    return -1;
}

/*------------------------------------------------------------------------------
	����SQM�������Ķ˿�
 ------------------------------------------------------------------------------*/
static int setTr069PortSQMServerPort(char* value, unsigned int size)
{
    LogUserOperDebug("set SQMServerPort = %d\n", atoi(value));
    sqm_set_server_port((unsigned int)atoi(value));

    return 0;
}
#endif


/*------------------------------------------------------------------------------
 *  ���¶����ע�ᵽ��root.Device.XXX.SQMConfiguration
 *   * XXX������X_CTC_IPTV��X_00E0FC
 ------------------------------------------------------------------------------*/
Tr069SQMConfiguration::Tr069SQMConfiguration()
	: Tr069GroupCall("SQMConfiguration")
{
#if defined(SQM_INCLUDED)
    // HW��CTC ���� fun1 - fun2
    Tr069Call* fun1  = new Tr069FunctionCall("SQMLisenPort", getTr069PortSQMLisenPort, setTr069PortSQMLisenPort);
    regist(fun1->name(), fun1);

    Tr069Call* fun2  = new Tr069FunctionCall("SQMServerPort", getTr069PortSQMServerPort, setTr069PortSQMServerPort);
    regist(fun2->name(), fun2);
#endif
}

Tr069SQMConfiguration::~Tr069SQMConfiguration()
{
}



