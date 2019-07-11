#ifndef __TRUNK_EC2108_C27_IPSTB_SRC_APP_PROGRAM_C20_HttpINIT_H_
#define __TRUNK_EC2108_C27_IPSTB_SRC_APP_PROGRAM_C20_HttpINIT_H_

#ifdef __cplusplus

#include "Message.h"
#include "MessageTypes.h"
#include "MessageHandler.h"

namespace Hippo {
class HttpInit : public MessageHandler {
public:
	HttpInit();
	~HttpInit();
	void httpRequestCreat(void);
	virtual void handleMessage(Message *msg);

};

} // namespace Hippo

#endif // __cplusplus

extern "C" int HttpRequestInit();

#endif
