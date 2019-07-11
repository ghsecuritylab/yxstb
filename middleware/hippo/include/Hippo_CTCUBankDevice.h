#ifndef __HIPPO_CTCUBankDevice_H
#define __HIPPO_CTCUBankDevice_H

#include <Hippo_HString.h>
namespace Hippo{

class HActiveObjectClient;
class CTCUBankDevice {
public:
	CTCUBankDevice( HActiveObjectClient* client, const char* );
	~CTCUBankDevice( );
	char* ioctl( HString& cmdJson );
	bool deleteEvent( int eventId );
	bool addEvent( int eventId );
	static int onEvent(int eventId, char *value, void *handle);
	bool set( HString& ioStr, HString& wrStr );
	const char* get( HString& ioStr );
private:
	HActiveObjectClient* m_client;
	HString m_name;
};


}


#endif

