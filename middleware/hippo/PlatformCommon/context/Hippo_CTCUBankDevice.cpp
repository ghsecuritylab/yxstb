#include <Hippo_Debug.h>
#include <Hippo_CTCUBankDevice.h>
#include <base/Hippo_HEvent.h>

#if defined(PAY_SHELL)
#include "PayShell.h"
#endif

namespace Hippo{

#if defined(PAY_SHELL)
CTCUBankDevice::CTCUBankDevice( HActiveObjectClient* client, const char* name )
	:m_client(client)
{
	HIPPO_DEBUG( "create Hippo CTCUBankDevice, this=%#x\n", this );
	m_name = name;

	pay_shell_setHandle(this);
	JseParamWrite("UBank_construction", m_name.c_str( ), m_name.length());
}

CTCUBankDevice::~CTCUBankDevice( )
{
	HIPPO_DEBUG( "release Hippo CTCUBankDevice.\n" );
#if 0
	JseParamWrite("UBank_destructor", "UBank_destructor", strlen("UBank_destructor"));
    pay_shell_setHandle(NULL);
#endif
}

char* CTCUBankDevice::ioctl( HString& cmdJson )
{
	HIPPO_DEBUG( "run here. cmdJson=%s\n", cmdJson.c_str( ) );

    return pay_shell_ioctl(cmdJson.c_str( ));
}

bool CTCUBankDevice::deleteEvent( int eventId )
{
	HIPPO_DEBUG( "run here. eventId=%d\n", eventId );
	return true;
}

bool CTCUBankDevice::addEvent( int eventId )
{
	HIPPO_DEBUG( "run here. eventId=%d\n", eventId );
	return true;
}

int CTCUBankDevice::onEvent(int eventId, char *value, void *handle)
{
	if (!handle)
		return -1;

	HIPPO_DEBUG( "run here. eventId=%d, handle=%#x\n", eventId, handle);
	
	CTCUBankDevice* UBank = static_cast<CTCUBankDevice*>( handle );	

	HString jsonStr;
	HString m_value = value;
	HEvent event;

	event.value(m_value);
	event.code((long)eventId);
	
    if (UBank->m_client) {
    	HIPPO_DEBUG("run here.\n");
        UBank->m_client->HandleEvent( &event, jsonStr );
   	}

	return 0;
	
	
}

bool CTCUBankDevice::set( HString& ioStr, HString& wrStr )
{
	HIPPO_DEBUG( "run here. ioStr=%s, wrStr=%s\n", ioStr.c_str( ), wrStr.c_str() );
	if( ioStr.equalIgnoringCase( "call" ) ){
		if( wrStr.equalIgnoringCase( "UBank_onEvent" ) )
			CTCUBankDevice::onEvent( 1001, (char*)"test ", this );
	}
	return false;
}

const char* CTCUBankDevice::get( HString& ioStr )
{
	HIPPO_DEBUG( "run here. ioStr=%s\n", ioStr.c_str( ) );
	return NULL;
}

#else //defined(PAY_SHELL)

CTCUBankDevice::CTCUBankDevice( HActiveObjectClient* client, const char* name )
	:m_client(client)
{
}

CTCUBankDevice::~CTCUBankDevice( )
{
}

char* CTCUBankDevice::ioctl( HString& cmdJson )
{    
	return NULL;
}

bool CTCUBankDevice::deleteEvent( int eventId )
{
	return true;
}

bool CTCUBankDevice::addEvent( int eventId )
{
	return true;
}

int CTCUBankDevice::onEvent(int eventId, char *value, void *handle)
{
	return 0;	
}

bool CTCUBankDevice::set( HString& ioStr, HString& wrStr )
{
	return false;
}

const char* CTCUBankDevice::get( HString& ioStr )
{
	return NULL;
}
#endif

}

