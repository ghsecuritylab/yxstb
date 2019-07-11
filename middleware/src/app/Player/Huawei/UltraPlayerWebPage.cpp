
#include "UltraPlayerWebPage.h"

#include "ProgramChannel.h"
#include "BrowserAgent.h"


namespace Hippo {

UltraPlayerWebPage::UltraPlayerWebPage(UltraPlayerClient *client, BrowserPlayerReporter *pReporter, ProgramChannel *pProgram)
	: UltraPlayerMultiple(client, pReporter, pProgram)
	, mProgram(pProgram)
{
    mProgram->ref();
    mIsFake = 1;
}

UltraPlayerWebPage::~UltraPlayerWebPage()
{
    mProgram->unref();
}

int 
UltraPlayerWebPage::play(unsigned int)
{
    epgBrowserAgent().openUrl((char *)mProgram->GetChanURL().c_str());
    return 0;
}

} // namespace Hippo

static int gWebchannelFlag = 0; // 用于标识是否正在播放webchannel的状态

// 设置是否进入webchannel状态
extern "C" int
webchannelFlagSet(int flag)
{
    gWebchannelFlag = flag;
    return 0;
}

// 获取webchannel状态
extern "C" int
webchannelFlagGet(void)
{
    return gWebchannelFlag;
}
