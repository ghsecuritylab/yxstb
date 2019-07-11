
#include <sys/stat.h>

#include "ConfigPartition.h"
#include "mid_sys.h"
#include "mid_stream.h"
#include "libzebra.h"
#include "Tr069.h"

#include "Assertions.h"


#define YX_STBTYPE_FILE "/root/.st111"
#define YX_STBTYPE_LEN 512

static int visualizationCollectInfoType = InfoTYpe_unknown;
static int isStartCollectDebugInfo = 0;

void mid_sys_serial_set(char *sn) // The serial number lenght is 17.
{
    SetSerialCode(sn);
    return;
}

void mid_sys_serial(char *buf)
{
    char *sn = GetSerialCode(NULL);

    if(!buf || !sn)
        return;

    mid_SN17toSN32(sn, buf);
    LogUserOperDebug("Now ,i will use the 32 serial(%s) for stbid\n", buf);
    return ;
}

#if 0
// 请用StbInfo::STB::Model()
// 机顶盒型号在config.xml里定义。

void mid_sys_stbtype_get(char *buf)
{
    if(!buf) {
        LogUserOperError("buf is NULL! \n");
        return;
    }

    // Add change the entry box model, through read from the file
    FILE *fd = fopen(YX_STBTYPE_FILE, "r");
    if(fd) {
        char stb_type_buffer[YX_STBTYPE_LEN] = {0};
        if(fread(stb_type_buffer, 1, YX_STBTYPE_LEN, fd) == YX_STBTYPE_LEN) {
            if(stb_type_buffer[501] == 8
               && (!strcmp(stb_type_buffer, "EC1308G")
                   || !strcmp(stb_type_buffer, "EC1308G_pub")
                   || !strcmp(stb_type_buffer, "EC1308H_pub")
                   || !strcmp(stb_type_buffer, "EC1309G_pub"))) {
                strcpy(buf, stb_type_buffer);
                fclose(fd);
                return;
            }
        }
    }
#if defined(Jiangsu)
    strcpy(buf, "EC1308");
#else
    strcpy(buf, "EC1308H_pub");
#endif
    if(fd)
        fclose(fd);
    return;
}
#endif

/*flag 为1显示视频层，flag为0隐藏视频层*/
void mid_sys_video_show(int flag)
{
    yx_drv_videoout_show(flag);
    return;
}

void mid_sys_videoFormat_set(int fmt, int SetToUboot)
{
    LogUserOperError("Can't support this function !\n");
    return;
}

void mid_sys_aspectRatioMode_set(int pMode)
{
    LogUserOperError("Can't support this function !\n");
    return ;
}

void mid_sys_Macrovision_set(int level)
{
    LogUserOperError("Can't support this function !\n");
    return;
}

int mid_sys_Macrovision_get()
{
    LogUserOperError("Can't support this function !\n");
    return -1;
}

int  mid_sys_hdcp_enableflag_set(int enable)
{
    LogUserOperError("Can't support this function !\n");
    return -1;
}

int mid_sys_hdcp_enableflag_get(void)
{
    LogUserOperError("Can't support this function !\n");
    return -1;
}

int mid_sys_cgmsa_enableflag_set(int enable)
{
    LogUserOperError("Can't support this function !\n");
    return -1;
}

int mid_sys_cgmsa_enableflag_get(void)
{
    LogUserOperError("Can't support this function !\n");
    return -1;
}

int mid_sys_HDMIConnect_status_get(void)
{
    LogUserOperError("Can't support this function !\n");
    return 0;
}

int mid_sys_setCollectInfoType(int type)
{
    if (type < InfoTYpe_unknown || type >= InfoTYpe_MAX)
        ERR_OUT("set type error, type[%d]", type);

    visualizationCollectInfoType = type;
    return 0;
Err:
    return -1;
}

int mid_sys_getCollectInfoType(void)
{
    return visualizationCollectInfoType;
}

#ifdef ENABLE_VISUALINFO
static char* transition_Visualization_Info(YX_VISUALIZATION_INFO info, int type, int AudioCodecNum)
{
    static char tempInfo[128] = {0};
    int len = sizeof(tempInfo);

    memset(tempInfo, 0, sizeof(tempInfo));
    switch(type){
    case ASPECT_RATIO:
        switch(info.VideoAspect){
        case YX_ASPECT_RATIO_1x1:
            strncpy(tempInfo, "1:1", len);
            break;
        case YX_ASPECT_RATIO_4x3:
            strncpy(tempInfo, "4:3", len);
            break;
        case YX_ASPECT_RATIO_16x9:
            strncpy(tempInfo, "16:9", len);
            break;
        case YX_ASPECT_RATIO_221x1:
            strncpy(tempInfo, "221:1", len);
            break;
        case YX_ASPECT_RATIO_15x9:
            strncpy(tempInfo, "15:9", len);
            break;
        case YX_ASPECT_RATIO_SAR:
            strncpy(tempInfo, "SAR", len);
            break;
        case YX_ASPECT_RATIO_UNKOWN:
        default:
            strncpy(tempInfo, "Unknown", len);
            break;
        }
        break;
    case VIDEO_TYPE:
        switch(info.VideoCodec) {
        case YX_VIDEO_TYPE_MPEG1:
            strncpy(tempInfo, "MPEG1", len);
            break;
        case YX_VIDEO_TYPE_MPEG2:
            strncpy(tempInfo, "MPEG2", len);
            break;
        case YX_VIDEO_TYPE_H264:
            strncpy(tempInfo, "H.264", len);
            break;
        case YX_VIDEO_TYPE_H263:
            strncpy(tempInfo, "H.263", len);
            break;
        case YX_VIDEO_TYPE_VC1:
            strncpy(tempInfo, "VC1", len);
            break;
        case YX_VIDEO_TYPE_VC1_SM:
            strncpy(tempInfo, "VC1_SM", len);
            break;
        case YX_VIDEO_TYPE_MPEG4PART2:
            strncpy(tempInfo, "MPEG4PART2", len);
            break;
        case YX_VIDEO_TYPE_DIVX311:
            strncpy(tempInfo, "DIVX311", len);
            break;
        case YX_VIDEO_TYPE_AVS:
            strncpy(tempInfo, "AVS", len);
            break;
        case YX_VIDEO_TYPE_NONE:
        default:
            strncpy(tempInfo, "Unknown", len);
            break;
        }
        break;
    case DECODER_FIELD_MODE:
        switch(info.PictureCode) {
        case YX_DECODER_BOTTOM_FIELD_MODE:
            strncpy(tempInfo, "BottomField", len);
            break;
        case YX_DECODER_TOP_FIELD_MODE:
            strncpy(tempInfo, "TopField", len);
            break;
        case YX_DECODER_FRAME_MODE:
            strncpy(tempInfo, "Frame", len);
            break;
        case YX_DECODER_FIELD_MODE_UNKNOWN:
        default:
            strncpy(tempInfo, "Unknown", len);
            break;
       }
        break;
    case AUDIO_TYPE:
        switch(info.AudioCodec[AudioCodecNum]) {
        case YX_AUDIO_TYPE_PCM:
            strncpy(tempInfo, "PCM", len);
            break;
        case YX_AUDIO_TYPE_MPEG:
            strncpy(tempInfo, "MPEG", len);
            break;
        case YX_AUDIO_TYPE_MP3:
            strncpy(tempInfo, "MP3", len);
            break;
        case YX_AUDIO_TYPE_AAC:
            strncpy(tempInfo, "AAC", len);
            break;
        case YX_AUDIO_TYPE_AACPLUS:
            strncpy(tempInfo, "AACPLUS", len);
            break;
        case YX_AUDIO_TYPE_AC3:
            strncpy(tempInfo, "AC3", len);
            break;
        case YX_AUDIO_TYPE_AC3PLUS:
            strncpy(tempInfo, "AC3PLUS", len);
            break;
        case YX_AUDIO_TYPE_DTS:
            strncpy(tempInfo, "DTS", len);
            break;
        case YX_AUDIO_TYPE_LPCM_DVD:
            strncpy(tempInfo, "LPCM_DVD", len);
            break;
        case YX_AUDIO_TYPE_LPCM_HDDVD:
            strncpy(tempInfo, "LPCM_HDDVD", len);
            break;
        case YX_AUDIO_TYPE_LPCM_BLURAY:
            strncpy(tempInfo, "LPCM_BLURAY", len);
            break;
        case YX_AUDIO_TYPE_DTSHD:
            strncpy(tempInfo, "DTSHD", len);
            break;
        case YX_AUDIO_TYPE_WMASTD:
            strncpy(tempInfo, "WMASTD", len);
            break;
        case YX_AUDIO_TYPE_WMAPRO:
            strncpy(tempInfo, "WMAPRO", len);
            break;
        case YX_AUDIO_TYPE_AVS:
            strncpy(tempInfo, "AVS", len);
            break;
        case YX_AUDIO_TYPE_AACPLUS_ADTS:
            strncpy(tempInfo, "AACPLUS_ADTS", len);
            break;
        case YX_AUDIO_TYPE_WAVPCM:
            strncpy(tempInfo, "WAVPCM", len);
            break;
        case YX_AUDIO_TYPE_NONE:
        default:
            strncpy(tempInfo, "Unknown", len);
            break;
        }
        break;
    case TRANCPORT_TYPE:{
        int transportProtocol = 0;
        transportProtocol = mid_stream_getInt("TransportProtocol", 0);
        switch(transportProtocol){
        case 0:
            strncpy(tempInfo, "TCP", len);
            break;
        case 1:
            strncpy(tempInfo, "UDP", len);
            break;
        case 2:
            strncpy(tempInfo, "RTP OVER TCP", len);
            break;
        case 3:
            strncpy(tempInfo, "RTP OVER UDP", len);
            break;
        default:
            strncpy(tempInfo, "Unknown", len);
            break;
        }
         break;
    }
    default:
        LogUserOperDebug("not support, type:%d\n", type);
        break;
    }

    return tempInfo;
}
#endif

char* judge_data_plus_or_minus(unsigned int data)
{
    static char tempInfo[128] = {0};
    memset(tempInfo, 0, sizeof(tempInfo));
    if ((int)data < 0)
        strncpy(tempInfo, "Unknown", sizeof(tempInfo));
    else
        snprintf(tempInfo, sizeof(tempInfo), "%u", data);
    tempInfo[strlen(tempInfo)] = '\0';

    return tempInfo;
}

int mid_sys_getVisualizationStreamInfo(char *visualizationInfo, int size, int infoType)
{
#ifdef ENABLE_VISUALINFO
    YX_VISUALIZATION_INFO info;
    char tempBuf[2048] = {0}, temp[128] = {0};
    int len = 0, length = sizeof(tempBuf);;

     if (!visualizationInfo)
       ERR_OUT("VisualizationInfo is NULL\n");

     if (0 > infoType || 1 < infoType)
         ERR_OUT("type err, type:%d\n", infoType);


    memset(&info, 0, sizeof(info));
    int result = ymm_decoder_getVisualizationInfo(infoType, &info); //Success is 0, otherwise is not zero
    if (result)
        ERR_OUT("VisualizationInfo get from sdk error, result:%d\n", result);

    len += snprintf(tempBuf + len, length, "CPU=%s%%\n", judge_data_plus_or_minus(info.CpuUsed));
    len += snprintf(tempBuf + len, length, "MEM=%s%%\n", judge_data_plus_or_minus(info.SysMemUsed));
    len += snprintf(tempBuf + len, length, "VideoCodec=%s\n", transition_Visualization_Info(info, VIDEO_TYPE, 0)); //YX_VIDEO_TYPE VideoCodec;

    if ((int)info.VideoWidth <= 0 || (int)info.VideoHeight <= 0)
        len += snprintf(tempBuf + len, length, "VideoResolution=Unknown\n");
    else {
        len += snprintf(tempBuf + len, length, "VideoResolution=%s*", judge_data_plus_or_minus(info.VideoWidth));
        len += snprintf(tempBuf + len, length, "%s\n", judge_data_plus_or_minus(info.VideoHeight));
    }
    len += snprintf(tempBuf + len, length, "VideoAspect=%s\n", transition_Visualization_Info(info, ASPECT_RATIO, 0)); //YX_ASPECT_RATIO VideoAspect;
    len += snprintf(tempBuf + len, length, "PictureCode=%s\n", transition_Visualization_Info(info, DECODER_FIELD_MODE, 0)); //YX_DECODER_FIELD_MODE

    if (1 == info.AudioNum) {
        len += snprintf(tempBuf + len, length, "AudioCodec=%s\n", transition_Visualization_Info(info, AUDIO_TYPE, 0));  //YX_AUDIO_TYPE AudioCodec
        if (InfoTYpe_stream_information == infoType)
            len += snprintf(tempBuf + len, length, "AudioBitRate=%dkbps\n", mid_stream_getInt("AudioBitRate", 0) / 1024);
         else
            len += snprintf(tempBuf + len, length, "AudioBitRate=%dkbps\n",mid_stream_getInt("AudioBitRate1", 0) / 1024);

        len += snprintf(tempBuf + len, length, "AudioChannels=Unknown\n");

        if ((int)info.AudioSamplingRate[0] < 0)
            len += snprintf(tempBuf + len, length, "AudioSamplingRate=Unknown\n");
        else
            len += snprintf(tempBuf + len, length, "AudioSamplingRate=%sKHz\n", judge_data_plus_or_minus(info.AudioSamplingRate[0] / 1024));
        if (!strlen(&info.SubtitleLanguage[0][4]))
            len += snprintf(tempBuf + len, length, "SubtitleLanguage=Unknown\n");
        else
            len += snprintf(tempBuf + len, length, "SubtitleLanguage=%s\n", &info.SubtitleLanguage[0][4]);
    } else if (-1 == (int)info.AudioNum || 0 == (int)info.AudioNum) {
        len += snprintf(tempBuf + len, length, "AudioCodec=Unknown\n");  //YX_AUDIO_TYPE AudioCodec
        len += snprintf(tempBuf + len, length, "AudioBitRate=Unknown\n");
        len += snprintf(tempBuf + len, length, "AudioChannels=Unknown\n");
        len += snprintf(tempBuf + len, length, "AudioSamplingRate=Unknown\n");
        len += snprintf(tempBuf + len, length, "SubtitleLanguage=Unknown\n");
    } else {
        int i;
        for(i = 1; i <= 2; i++) { //如果三音轨，默认显示前两个音轨
            len += snprintf(tempBuf + len, length, "Audio%dCodec=%s\n", i, transition_Visualization_Info(info, AUDIO_TYPE, i - 1));  //YX_AUDIO_TYPE AudioCodec
            if (InfoTYpe_stream_information == infoType)
                len += snprintf(tempBuf + len, length, "Audio%dBitRate=%dkbps\n", i, mid_stream_getInt("AudioBitRate", (i - 1)) / 1024);
             else
                len += snprintf(tempBuf + len, length, "Audio%dBitRate=%dkbps\n", i, mid_stream_getInt("AudioBitRate1", (i - 1)) / 1024);

            len += snprintf(tempBuf + len, length, "Audio%dChannels=Unknown\n", i);

            if ((int)info.AudioSamplingRate[i - 1] < 0)
                len += snprintf(tempBuf + len, length, "AudioSamplingRate=Unknown\n");
            else
                len += snprintf(tempBuf + len, length, "Audio%dSamplingRate=%sKHz\n", i, judge_data_plus_or_minus(info.AudioSamplingRate[i - 1] / 1024));

            if (!strlen(&info.SubtitleLanguage[i - 1][4]))
                len += snprintf(tempBuf + len, length, "Subtitle%dLanguage=Unknown\n", i);
            else
                len += snprintf(tempBuf + len, length, "Subtitle%dLanguage=%s\n", i, &info.SubtitleLanguage[i - 1][4]);
        }
    }

    if (InfoTYpe_stream_information == infoType){
       mid_stream_getString("PacketLost", 0, temp, sizeof(temp));
       len += snprintf(tempBuf + len, length, "PacketLost=%s\n", temp);
    } else {
       mid_stream_getString("PacketLost1", 0, temp, sizeof(temp));
       len += snprintf(tempBuf + len, length, "PacketLost=%s\n", temp);
    }
    len += snprintf(tempBuf + len, length, "PacketDisorder=%d\n", get_sqa_errInfo(1));
    len += snprintf(tempBuf + len, length, "StreamDF=Unknown\n"); //Sqm library no interface, assignment unknown.
    len += snprintf(tempBuf + len, length, "TransportProtocol=%s\n", transition_Visualization_Info(info, TRANCPORT_TYPE, 0));
    len += snprintf(tempBuf + len, length, "ContinuityError=%s\n", judge_data_plus_or_minus(info.ContinuityError));
    len += snprintf(tempBuf + len, length, "SynchronisationError=%s\n", judge_data_plus_or_minus(info.SynchronisationError));
    len += snprintf(tempBuf + len, length, "EcmError=Unknown\n"); //CA library no interface, assignment unknown.
    len += snprintf(tempBuf + len, length, "DiffAvPlayTime=%dms\n", (int)info.DiffAvPlayTime);

    len += snprintf(tempBuf + len, length, "VideoDecoderBufSize=%s\n", judge_data_plus_or_minus(info.VideoBufSize));
    len += snprintf(tempBuf + len, length, "VideoDecoderUsedSize=%s\n", judge_data_plus_or_minus(info.VideoUsedSize));
    len += snprintf(tempBuf + len, length, "AudioDecoderBufSize=%s\n", judge_data_plus_or_minus(info.AudioBufSize));
    len += snprintf(tempBuf + len, length, "AudioDecoderUsedSize=%s\n", judge_data_plus_or_minus(info.AudioUsedSize));
    len += snprintf(tempBuf + len, length, "VideoDecoderError=%s\n", judge_data_plus_or_minus(info.VideoDecodeError));
    len += snprintf(tempBuf + len, length, "VideoDecoderDrop=%s\n", judge_data_plus_or_minus(info.VideoDecodeDrop));
    len += snprintf(tempBuf + len, length, "VideoDecoderUnderflow=%s\n", judge_data_plus_or_minus(info.VideoDecodeUnderflow));
    len += snprintf(tempBuf + len, length, "VideoDecoderPtsError=%s\n", judge_data_plus_or_minus(info.VideoDecodePtsError));
    len += snprintf(tempBuf + len, length, "AudioDecoderError=%s\n", judge_data_plus_or_minus(info.AudioDecodeError));
    len += snprintf(tempBuf + len, length, "AudioDecoderDrop=%s\n", judge_data_plus_or_minus(info.AudioDecodeDrop));
    len += snprintf(tempBuf + len, length, "AudioDecoderUnderflow=%s\n", judge_data_plus_or_minus(info.AudioDecodeUnderflow));
    len += snprintf(tempBuf + len, length, "AudioDecoderPtsError=%s\n", judge_data_plus_or_minus(info.AudioDecodePtsError));

    strncpy(visualizationInfo, tempBuf, size - 1);

    return 0;

Err:
    memset(tempBuf, 0, sizeof(tempBuf));
    snprintf(tempBuf, length, "CPU=Unknown\nMEM=Unknown\nVideoCodec=Unknown\nVideoResolution=Unknown\n\
VideoAspect=Unknown\nPictureCode=Unknown\nAudioCodec=Unknown\nAudioBitRate=Unknown\nAudioChannels=Unknown\n\
AudioSamplingRate=Unknown\nSubtitleLanguage=Unknown\nPacketLost=Unknown\nPacketDisorder=Unknown\n\
StreamDF=Unknown\nTransportProtocol=Unknown\nContinuityError=Unknown\nSynchronisationError=Unknown\n\
EcmError=Unknown\nDiffAvPlayTime=Unknown\nVideoBufSize=Unknown\nVideoUsedSize=Unknown\nAudioBufSize=Unknown\n\
AudioUsedSize=Unknown\nVideoDecoderError=Unknown\nVideoDecoderDrop=Unknown\nVideoDecoderUnderflow=Unknown\n\
VideoDecoderPtsError=Unknown\nAudioDecoderError=Unknown\nAudioDecoderDrop=Unknown\nAudioDecoderUnderflow=Unknown\n\
AudioDecoderPtsError=Unknown\n");
    if (visualizationInfo)
        strncpy(visualizationInfo, tempBuf, size - 1);
#endif
    return -1;
}

int mid_sys_getVisualizatioOTTStreamInfo(char *visualizationInfo, int size)
{
#ifdef ENABLE_VISUALINFO
    char wifi_ssid[34] = {0}, wifi_channel[34] = {0}, temp[512] = {0}, tempBuf[2048] = {0};
    int len = 0, length = sizeof(tempBuf), streamNum = 0, segmentNum = 0, i = 0, netType = 1;
    int wQuality = 0, wLevel = 0, wNoise = 0;

    if (!visualizationInfo)
       ERR_OUT("VisualizationInfo is NULL\n");

    memset(tempBuf, 0, sizeof(tempBuf));
    sysSettingGetInt("nettype", &netType, 0);
    if (netType) {
        const char* devname = network_wifi_devname(); //TODO
        len += snprintf(tempBuf + len, length, "NetMode:Wifi\n");
        sysSettingGetString("wifi_ssid", wifi_ssid, sizeof(wifi_ssid), 0);
        len += snprintf(tempBuf + len, length, "SSID:%s\n", wifi_ssid);
        if(network_device_link_state(devname) >= 0)
            network_wifi_signal_get(devname, &wQuality, &wLevel, &wNoise);
        len += snprintf(tempBuf + len, length, "Signal:%ddbm\n", wQuality);
        sysSettingGetString("Wifi_channel", wifi_channel, sizeof(wifi_channel), 0);
        len += snprintf(tempBuf + len, length, "Channel:%s\n", wifi_channel);
        len += snprintf(tempBuf + len, length, "LinkSpeed: %dMbps\n",  network_linkspeed_get(devname) / 1024 / 1024);
    } else {
        len += snprintf(tempBuf + len, length, "NetMode:Eth\n");
        len += snprintf(tempBuf + len, length, "SSID:\n");
        len += snprintf(tempBuf + len, length, "Signal:\n");
        len += snprintf(tempBuf + len, length, "Channel:\n");
        len += snprintf(tempBuf + len, length, "LinkSpeed:100Mbps\n");
    }

    char ifname[32] = { 0 };
    int downloadrate = network_downloadrate_get(network_default_ifname(ifname, 32)) ;
    len += snprintf(tempBuf + len, length, "DownloadRate:%dKbps\n", downloadrate / 1024);
    len += snprintf(tempBuf + len, length, "PlayRate:%dbps\n", mid_stream_getInt("Playrate", 0) / 1024);
    len += snprintf(tempBuf + len, length, "RemainPlayTime:%ds\n", mid_stream_getInt("RemainPlaytime", 0));
    len += snprintf(tempBuf + len, length, "CurrentBufferSize:%dKB\n", mid_stream_getInt("CurBufferSize", 0) / 1024);
    len += snprintf(tempBuf + len, length, "TotalBufferSize:%dKB\n", mid_stream_getInt("ToalBufferSize", 0) / 1024);
    len += snprintf(tempBuf + len, length, "StreamInfo:\n");
    streamNum = mid_stream_getInt("StreamNum", 0);
    for(i = 0; i <streamNum; i++) {
        memset(temp, 0, sizeof(temp));
        mid_stream_getString("StreamURL", i, temp, sizeof(temp));
        len += snprintf(tempBuf + len, length, "BandWidth:%dbps URL:%s\n", mid_stream_getInt("StreamBandwith", i), temp);
        }
    len += snprintf(tempBuf + len, length, "CurrentDownloadSegment:\n");
    memset(temp, 0, sizeof(temp));
    mid_stream_getString("CurSegment", 0, temp, sizeof(temp));
    if(strlen(temp))
        len += snprintf(tempBuf + len, length, "%s\n", temp);
    len += snprintf(tempBuf + len, length, "SegmentListInfo:\n");

    segmentNum = mid_stream_getInt("SegmentNum", 0);
    if ( 20 < segmentNum)
        segmentNum = 20;
    for(i = 0; i < segmentNum; i++) {
        memset(temp, 0, sizeof(temp));
        mid_stream_getString("SegmentList", i, temp, sizeof(temp));
        len += snprintf(tempBuf + len, length, "%s\n", temp);
    }

    strncpy(visualizationInfo, tempBuf, size - 1);
    return 0;
#endif
Err:
    return -1;
}

int mid_sys_getVisualizationInfo(char *visualizationInfo, int size)
{
#ifdef ENABLE_VISUALINFO
    int infoType = -1;

    infoType = mid_sys_getCollectInfoType();

    switch(infoType) {
    case InfoTYpe_stream_information:
        mid_sys_getVisualizationStreamInfo(visualizationInfo, size, InfoTYpe_stream_information);
        break;
    case InfoTYpe_PIP_stream_information:
        mid_sys_getVisualizationStreamInfo(visualizationInfo, size, InfoTYpe_PIP_stream_information);
        break;
    case InfoTYpe_OTT_information:
        mid_sys_getVisualizatioOTTStreamInfo(visualizationInfo, size);
        break;
    default :
        mid_sys_getVisualizationStreamInfo(visualizationInfo, size, InfoTYpe_unknown);
        break;
    }
#endif
    return 0;
}

void mid_sys_setStartCollectDebugInfo(int flag)
{
    isStartCollectDebugInfo = flag;
}

int mid_sys_getStartCollectDebugInfo(void)
{
    return isStartCollectDebugInfo;
}


