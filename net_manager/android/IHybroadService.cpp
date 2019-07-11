
#include <stdint.h>
#include <sys/types.h>

#include <binder/Parcel.h>
#include "IHybroadService.h"
#include "BpHybroadService.h"

namespace android {

IMPLEMENT_META_INTERFACE(HybroadService, "com.hybroad.stb.IHybroadService");

status_t BnHybroadService::onTransact(uint32_t code, const Parcel &data, Parcel *reply, uint32_t flags) {
	const char *str;
	int id;
	LOGI("START BnHybroadService::onTransact");
	switch (code) {
	case TRANSACTION_arpping: {
		str = data.readCString();
		reply->writeInt32(0);
		reply->writeInt32(arpping(str));
		return NO_ERROR;
		break;
	}

	case TRANSACTION_ping: {
		str = data.readCString();
		reply->writeInt32(0);
		reply->writeInt32(ping(str));
		return NO_ERROR;
		break;
	}

	case TRANSACTION_setListener: {
		setListener(interface_cast < ICallbackListener > (data.readStrongBinder()));
		reply->writeInt32(0);
		reply->writeInt32(1);
		return NO_ERROR;
		break;
	}

	default:
		return BBinder::onTransact(code, data, reply, flags);
	}

    LOGI("END BnHybroadService::onTransact");
    return 0;
  }
  
}