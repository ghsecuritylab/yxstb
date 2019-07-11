#include "HttpInit.h"
#include "ProgramAssertions.h"

#include "ChannelListHttpCall.h"

#include "string.h"
#include "app_heartbit.h"
#include "Assertions.h"
#include "mid/mid_tools.h"

namespace Hippo {

HttpInit::HttpInit()
{

}

HttpInit::~HttpInit()
{

}

void
HttpInit::handleMessage(Message * msg)
{
	char requestMessage[1024];
	int type;
	memset(requestMessage, 0, sizeof(requestMessage));

	if(msg->what == MessageType_Timer){
		if(httpRequestgetMsg(requestMessage,&type) <= 0){
			Message *message = this->obtainMessage(MessageType_Timer);
			this->sendMessageDelayed(message, 500);
			return;
		}

		ChannelListHttpCall *requestTheard = new ChannelListHttpCall();
		requestTheard->requstChannelList(requestMessage, type);
		PROGRAM_LOG("HttpInit::handleMessage type = %d\n", type);
		Message *message = this->obtainMessage(MessageType_Timer);
		this->sendMessageDelayed(message, 500);

	}

}

void
HttpInit::httpRequestCreat(void)
{
	httpRequestMsgCreat();

	Message *message = this->obtainMessage(MessageType_Timer);
	this->sendMessageDelayed(message, 500);
}

}

extern "C" int
HttpRequestInit()
{
	Hippo::HttpInit *p = new (Hippo::HttpInit)();
	p->httpRequestCreat();

	return 0;

}
