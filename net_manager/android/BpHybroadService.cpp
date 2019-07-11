#include <binder/Parcel.h>
#include "BpHybroadService.h"
#include <utils/Log.h>
#include "nm_dbg.h"

namespace android {
	
	BpHybroadService::BpHybroadService(const sp<IBinder>& impl)
	    : BpInterface<IHybroadService>(impl) {
		LOGI("BpHybroadService::BpHybroadService()");
	}

	int BpHybroadService::arpping(const char* ipaddr) {
		Parcel data, reply;
		LOGI("START BpHybroadService::arpping()");
		data.writeInterfaceToken(IHybroadService::getInterfaceDescriptor());
		data.writeCString(ipaddr);
		remote()->transact(TRANSACTION_arpping, data, &reply);
		LOGI("END BpHybroadService::arpping");
		return reply.readInt32();
	}

	int BpHybroadService::ping(const char* ipaddr) {
		LOGI("START BpHybroadService::ping()");
		Parcel data, reply;
		data.writeInterfaceToken(IHybroadService::getInterfaceDescriptor());
		data.writeCString(ipaddr);
		remote()->transact(TRANSACTION_ping, data, &reply);
		LOGI("END BpHybroadService::ping()");
		return reply.readInt32();

	}
    
	int BpHybroadService::setListener(const sp<ICallbackListener> &listener){
		Parcel data, reply;
		LOGI("START BpHybroadService::setListener()");
		data.writeInterfaceToken(IHybroadService::getInterfaceDescriptor());
		data.writeStrongBinder(listener->asBinder());
		remote()->transact(TRANSACTION_setListener, data, &reply);
		LOGI("END BpHybroadService::setListener()");
		return reply.readInt32();
	}

	char * BpHybroadService::nm_get_value(char *name)
	{
		nm_msg_level(LOG_DEBUG, "BpCallbackListener::ReadCallback start\n");
		Parcel data, reply;

		data.writeInterfaceToken(IHybroadService::getInterfaceDescriptor());
		String16 name16(name);
		data.writeString16(name16);
		remote()->transact(TRANSACTION_nm_getvalue, data, &reply);

		int ret = reply.readInt32();
		nm_msg_level(LOG_DEBUG, "BpCallbackListener::ReadCallback ret=%d\n", ret);
		char *dst = (char *)malloc(sizeof(char) * 512 * 1024);
		String16 result = reply.readString16();
		int length = result.size();
		utf16_to_utf8(result.string(), length, dst);
		nm_msg_level(LOG_DEBUG, "BpCallbackListener::ReadCallback dst=%s\n", dst);
		return dst;
	}
	void BpHybroadService::nm_set_value(char *name, char *str, unsigned int str_len)
	{
		Parcel data, reply;
		
		data.writeInterfaceToken(IHybroadService::getInterfaceDescriptor());
		String16 name16(name);
		data.writeString16(name16);
		if(str){
			String16 str16(str);
			data.writeString16(str16);
		} else {
			String16 str16("");
			data.writeString16(str16);
		}
		data.writeInt32(str_len);
		remote()->transact(TRANSACTION_nm_setvalue, data, &reply);
		LOGI("END BpHybroadService::nm_set_value()");
		return;
	}

	void BpHybroadService::sendToMonitor(char *name, char *value, unsigned int len)
    {
        Parcel data, reply;
        if (!name || !value)
            return ;
        
		LOGI("START BpHybroadService::sendToMonitor(),%s,%s,%d", name, value, len);
		data.writeInterfaceToken(IHybroadService::getInterfaceDescriptor());
		String16 name16(name);
		data.writeString16(name16);
        String16 value16(value);
		data.writeString16(value16);
		data.writeInt32(len);
		remote()->transact(TRANSACTION_sendToMonitor, data, &reply);
		LOGI("END BpHybroadService::sendToMonitor()");
		return;
        
    }
}
