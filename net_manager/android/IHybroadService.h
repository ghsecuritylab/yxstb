#ifndef ANDROID_IHYBROADSERVICE_H
#define ANDROID_IHYBROADSERVICE_H

#include <utils/RefBase.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>
#include <utils/String8.h>
#include "ICallbackListener.h"

namespace android {
	enum {
		TRANSACTION_arpping = IBinder::FIRST_CALL_TRANSACTION,
		TRANSACTION_ping,
		TRANSACTION_setListener,
		TRANSACTION_nm_getvalue,
		TRANSACTION_nm_setvalue,
		TRANSACTION_sendToMonitor		
	};

class IHybroadService: public IInterface {
public:
	DECLARE_META_INTERFACE(HybroadService);

	virtual int arpping(const char* ipaddr) = 0;
	virtual int ping(const char* ipaddr) = 0;
	virtual int setListener(const sp<ICallbackListener> &listener)=0;
	virtual char * nm_get_value(char *name) = 0;
	virtual void nm_set_value(char *name, char *str, unsigned int str_len)=0;	
	
};

class BnHybroadService: public BnInterface<IHybroadService> {
public:
    virtual status_t    onTransact(uint32_t code, const Parcel &data, Parcel *reply, uint32_t flags = 0);

};

}

#endif
