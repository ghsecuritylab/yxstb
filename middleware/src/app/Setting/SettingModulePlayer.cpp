
#include "Assertions.h"
#include "SettingModulePlayer.h"

#include <stdio.h>

#include "Setting.h"
#include "AppSetting.h"
#include "SysSetting.h"

#include "configCustomer.h"

namespace Hippo {

static SettingModulePlayer g_SettingModulePlayer;

SettingModulePlayer::SettingModulePlayer()
    : SettingModule()
{
}

SettingModulePlayer::~SettingModulePlayer()
{
}

int
SettingModulePlayer::settingModuleRegister()
{
    sysSetting().add("[ PLAYER ]", "");
    LogSafeOperDebug("SettingModulePlayer::settingModuleRegister [%d]\n", __LINE__);
    sysSetting().add("videoformat", DEFAULT_VIDEO_FORMAT);
    sysSetting().add("hd_video_format", DEFAULT_HD_VIDEO_FORMAT); // television high definition out system. 0 is disable，
                                                  // 1 is 1080i50HZ，2 is 1080i60HZ，3 is 576i50HZ，4 is 480i60HZ.
    sysSetting().add("pppdmulticast", DEFAULT_PPPD_MULTICAST); // 1 -- with ppp head     0 -- no ppp head
    sysSetting().add("TransportProtocol", DEFAULT_TRANSPORT_PROTOCOL);
    sysSetting().add("SPDIFAudioFormat", DEFAULT_SPDIF_AUDIO_FORMAT); //0 forPCM，1 for PASS-THROUGH
    sysSetting().add("HDMIAudioFormat", DEFAULT_HDMI_AUDIO_FORMAT); //0 forPCM，1 for PASS-THROUGH
    sysSetting().add("Aniflicker", DEFAULT_ANI_FLICKER);  //0 for disable，1 for enable
    sysSetting().add("DolbyMode", DEFAULT_DOLBY_MODE); // 0 is manual mode, 1 is auto mode. Default value is auto, spdif and hdmi can't select.
    sysSetting().add("changevideomode", DEFAULT_CHANGE_VIDEO_MODE); //0,normal; 1,last picture; 2,smooth switch
    sysSetting().add("ca_flag", DEFAULT_CA_FLAG); //CA flag，indecate the CA property of user当前用户是否为CA用户
    sysSetting().add("OttAppleStoreSize", "");

    sysSetting().add("lastChannelPlay", DEFAULT_LAST_CHANNEL_PLAY); //0,EPG; 1, BTV; 2,DVB

    sysSetting().add("mqmcIp", DEFAULT_MQMC_IP);
    sysSetting().add("sqmListenPort", 37001);
    sysSetting().add("sqmServerPort", 37000);

    //sqa param
    sysSetting().add("fcc_port", DEFAULT_FCC_PORT);
    // sysSetting().add("fcc_mode", DEFAULT_FCC_MODE);
    sysSetting().add("fcc_time", DEFAULT_FCC_TIME);
    sysSetting().add("sqa_fcc", DEFAULT_SQA_FCC);
    sysSetting().add("sqa_ret", DEFAULT_SQA_RET);
    sysSetting().add("sqa_fec", DEFAULT_SQA_FEC);
    sysSetting().add("SQABufferHLevel", DEFAULT_SQA_BUFFER_H_LEVEL);
    sysSetting().add("SQABufferMLevel", DEFAULT_SQA_BUFFER_M_LEVEL);
    sysSetting().add("SQABufferLLevel", DEFAULT_SQA_BUFFER_L_LEVEL);
    sysSetting().add("RETSendPeriod", DEFAULT_RET_SEND_PERIOD);
    sysSetting().add("RSRTimeOut", DEFAULT_RSR_TIMEOUT);
    sysSetting().add("SCNTimeOut", DEFAULT_SCN_TIMEOUT);
    sysSetting().add("ChanCacheTime", DEFAULT_CHAN_CACHE_TIME);
    sysSetting().add("RETDelayTime", DEFAULT_RET_DELAY_TIME);
    sysSetting().add("RETInterval", DEFAULT_RET_INTERVAL);
    sysSetting().add("NATRHBinterval", DEFAULT_NAT_RHB_INTERVAL);
    sysSetting().add("DisorderTime", DEFAULT_DISORDER_TIME);

    /**********************  customer ********************/
    appSetting().add("[ PLAYER ]", "");

    appSetting().add("fcc_switch", DEFAULT_FCC_SWITCH);
    appSetting().add("macrovision", DEFAULT_MACRO_VISION);
    appSetting().add("Teletext", DEFAULT_TELETEXT);
    appSetting().add("hdcpkey", DEFAULT_HDCP_KEY);
    appSetting().add("HDCPEnableDefault", DEFAULT_HDCP_ENABLE); //0:close, canbe modifed by stbmonitor or parameter update
    appSetting().add("CGMSAEnableDefault", DEFAULT_CGMSA_ENABLE);
    appSetting().add("hdmi_negotiation", DEFAULT_HDMI_NEGOTIATION);
    appSetting().add("hd_aspect_mode", DEFAULT_HD_ASPECT_MODE); //0:YX_ASPECT_MODE_LETTERBOX  2: YX_ASPECT_MODE_FULL

    appSetting().add("ChannelIndex", "");

    appSetting().add("AllowBandwidth", DEFAULT_ALLOW_BANDWIDTH);
    appSetting().add("defaultAudioLanguage", DEFAULT_AUDIO_LANGUAGE);
    appSetting().add("defaultSubtitleLanguage", DEFAULT_SUBTITLE_LANGUAGE);
    appSetting().add("defaultAudioChannel", DEFAULT_AUDIO_CHANNEL);

    appSetting().add("lastChannelID", 0);
#ifdef HUAWEI_C20
    appSetting().add("lastChanDomain", 0);
#endif
    appSetting().add("mediaServerType", DEFAULT_MEDIA_SERVER_TYPE);

    appSetting().add("volume", DEAFULT_AUDIO_VOLUME);
    appSetting().add("mute", 0);
    appSetting().add("BGMusicEnable", -1); // 为了兼容原来/root/.bgmconfig文件，这里默认值先设为-1

#ifdef ENABLE_IGMPV3
    appSetting().add("igmpversion", DEFAULT_IGMP_VERSION);
    appSetting().add("exclude", DEFAULT_EXCLUDE);
#endif

    appSetting().add("localTimeShift_enable", DEFAULT_LOCAL_TIME_SHIFT_ENABLE);
    appSetting().add("localTimeShift_maxDuration", DEFAULT_LOCAL_TIME_SHIFT_MAX_DURATION);
    appSetting().add("localTImeShift_startmode", DEFAULT_LOCAL_TIME_SHIFT_START_MODE); // 1: start record when play ; 2: start record when pause
    appSetting().add("maxChannelBandwidth", DEFAULT_MAX_CHANNEL_BAND_WIDTH); // max bandwidth of stream
    appSetting().add("local_Service_Profile", "");

#ifdef INCLUDE_PIP
    appSetting().add("pipx", "");
    appSetting().add("pipy", "");
    appSetting().add("pipw", "");
    appSetting().add("piph", "");
    appSetting().add("pipchannelid", "");
#endif

    appSetting().add("nCAFlag", DEFAULT_N_CA_FLAG); //CA flag.0:not CA account, 1：SM,2:Irdeto hardware card, 3:Irdeto soft card, 4:Verimatrix
    //conflic with ca_flag?


    return 0;
}

} // namespace Hippo
extern "C"
int settingPlayer()
{
    return 0;
}

