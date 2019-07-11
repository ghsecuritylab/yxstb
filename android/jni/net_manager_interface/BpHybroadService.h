
#ifndef ANDROID_BPHYBROADSERVICE_H
#define ANDROID_BPHYBROADSERVICE_H

#include <binder/Parcel.h>
#include "IHybroadService.h"

namespace android {
	
	class BpHybroadService: public BpInterface<IHybroadService> {
		public:
			BpHybroadService(const sp<IBinder>& impl);

			virtual int arpping(const char* ipaddr);
			virtual int ping(const char* ipaddr);
			virtual int setListener(const sp<ICallbackListener> &listener);
			virtual char * nm_get_value(char *name);
			virtual void nm_set_value(char *name, char *str, unsigned int str_len);			 
            virtual int sendToMonitor(char *name, char *value, int len);
    };
}
#endif