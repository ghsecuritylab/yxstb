
#include "tr069_port_ctc.h"

struct MonitorPost {
    int mdiMLR;
    int mdiDF;
    int jitter;
    int responseDelay;
    int channelSwitchDelay;
    int authNumbers;
    int authFailNumbers;
    int multiReqNumbers;
    int vodReqNumbers;
    int multiFailNumbers;
    int vodFailNumbers;
    int bufUnderFlowNumbers;
    int bufOverFlowNumbers;
};
struct MonitorPost g_monitorPostData = {0};

void monitor_cycle_statistics(int flag)
{
    switch (flag) {
    case FLAG_AUTHNUM:
        g_monitorPostData.authNumbers++;
        break;
    case FLAG_AUTHFAILNUM:
        g_monitorPostData.authFailNumbers++;
        break;
    case FLAG_MULTIREQNUM:
        g_monitorPostData.multiReqNumbers++;
        break;
    case FLAG_VODREQNUM:
        g_monitorPostData.vodReqNumbers++;
        break;
    case FLAG_MULTIFAILNUM:
        g_monitorPostData.multiFailNumbers++;
        break;
    case FLAG_VODFAILNUM:
        g_monitorPostData.vodFailNumbers++;
        break;
    case FLAG_BUFINCNUM:
        g_monitorPostData.bufOverFlowNumbers++;
        break;
    case FLAG_BUFDECNUM:
        g_monitorPostData.bufUnderFlowNumbers++;
        break;
    default:
        break;
    }

    return;
}
int app_tr069_port_Monitor_get_AuthNumbers(void)
{
    g_monitorPostData.authNumbers = 0;
    return g_monitorPostData.authNumbers;
}

int app_tr069_port_Monitor_get_AuthFailNumbers(void)
{
    g_monitorPostData.authFailNumbers = 0;
    return g_monitorPostData.authFailNumbers;
}

int app_tr069_port_Monitor_get_MultiReqNumbers(void)
{

    return g_monitorPostData.multiReqNumbers;
}

int app_tr069_port_Monitor_get_VodReqNumbers(void)
{
    return g_monitorPostData.vodReqNumbers;
}

int app_tr069_port_Monitor_get_MultiFailNumbers(void)
{
    return g_monitorPostData.multiFailNumbers;
}

int app_tr069_port_Monitor_get_VodFailNumbers(void)
{
    return g_monitorPostData.vodFailNumbers;
}

int app_tr069_port_Monitor_get_BufUnderFlowNumbers(void)
{
    return g_monitorPostData.bufUnderFlowNumbers;
}

int app_tr069_port_Monitor_get_BufOverFlowNumbers(void)
{
    return g_monitorPostData.bufOverFlowNumbers;
}

int app_tr069_port_Monitor_get_ResponseDelay(void)
{
	return g_monitorPostData.responseDelay;
}

int app_tr069_port_Monitor_get_ChannelSwitchDelay(void)
{
	return g_monitorPostData.channelSwitchDelay;
}

int monitor_post_play(int type, int delay, const char *channelName, const char *channelAddress)
{
    if (type == TYPE_VOD)
        g_monitorPostData.responseDelay = delay;
    else
        g_monitorPostData.channelSwitchDelay = delay;
    return 0;
}
