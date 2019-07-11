#include <netinet/in.h>
#include <errno.h>

#include "libzebra.h"
#include "cloud_api.h"
#include "codec.h"

extern int codec_cloud_open(YX_MPEG_INFO* mpeg);
extern int codec_cloud_close(void);
extern void codec_cloud_get(char **pbuf, int *plen);
extern void codec_cloud_put(int len);

C_RESULT stb_AVInjectTSData(IN C_U8 *pInjectData, IN C_U32 uDataLen)
{
    int len;
    char *buf;

    len = 0;
    while (uDataLen > 0) {
        codec_cloud_get(&buf, &len);

        if (len > uDataLen)
            len = uDataLen;
        len = len - len % 188;
        if (len <= 0)
            return CLOUD_FAILURE;
        memcpy(buf, pInjectData, len);
        pInjectData += len;
        uDataLen -= len;

        codec_cloud_put(len);
    }
    return CLOUD_OK;
}

C_RESULT CStb_AVPlayTSOverIP(IN C_PIDParam const *pPidParams, OUT CStb_AVInjectTSData *InjectTSDataFun)
{
    YX_MPEG_INFO mpeginfo;

    memset(&mpeginfo, 0, sizeof(mpeginfo));
    mpeginfo.stream_type = YX_STREAM_TS;

    mpeginfo.video_num = 1;
    mpeginfo.video_pid[0] = pPidParams->uVideoPid;
    if (VideoType_MPEG2 == pPidParams->uVideoType)
        mpeginfo.video_type[0] = YX_VIDEO_TYPE_MPEG2;
    else
        mpeginfo.video_type[0] = YX_VIDEO_TYPE_H264;

    mpeginfo.audio_num = 1;
    mpeginfo.audio_pid[0] = pPidParams->uAudioPid;
    if(AudioType_MPEG2 == pPidParams->uAudioType)
        mpeginfo.audio_type[0] = YX_AUDIO_TYPE_MPEG;
    else if(AudioType_MPEG2_AAC == pPidParams->uAudioType)
        mpeginfo.audio_type[0] = YX_AUDIO_TYPE_AAC;
    else
        mpeginfo.audio_type[0] = YX_AUDIO_TYPE_MP3;

    codec_cloud_open(&mpeginfo);

    *InjectTSDataFun = stb_AVInjectTSData;

    return CLOUD_OK;
}

void CStb_AVStop(void)
{
    codec_cloud_close( );
}

C_RESULT CStb_SetVideoWindow(IN C_U32 uX, IN C_U32 uY, IN C_U32 uWidth, IN C_U32 uHeight)
{
    int s_width, s_height;

    ygp_layer_getScreenSize(&s_width, &s_height);

    if (uX >= s_width || uWidth > s_width
            || uY >= s_height || uHeight > s_height
            || uX + uWidth > s_width || uY + uHeight > s_height)
        return CLOUD_FAILURE;

    yhw_vout_setVideoDisplayRect(uX, uY, uWidth, uHeight);

    return CLOUD_OK;
}

C_RESULT CStb_GetPacketLossRate (OUT C_U32 *pRate)
{
    *pRate = 0;
    return CLOUD_OK;
}

C_DecoderStatusCode CStb_GetDecoderStatus(IN C_U8 uDecoderType)
{
    return DecoderStatusCode_OK;
}

C_RESULT CStb_AVPlayByPid(IN C_FreqParam const *pFreq, IN C_PIDParam const *pPidParams )
{
	 return CLOUD_OK;
}

C_RESULT CStb_AVPlayByProgNo(IN C_FreqParam  const *pFreq, IN C_U32 uProgNo)
{
	return CLOUD_OK;
}

C_RESULT CStb_AVSetCW(IN C_U8 uType, IN C_U8 const *pEven, IN C_U8 const *pOdd, IN C_U8 uKeyLen)
{
	return CLOUD_OK;
}

C_U8 CStb_GetVolume(void)
{
	CLOUD_LOG_TRACE("CStb_GetVolume v_vol_c =\n");

	return mid_audio_volume_get();
}

C_RESULT CStb_SetVolume(IN C_U8 uVol)
{
	CLOUD_LOG_TRACE("CStb_SetVolume v_vol_c = %d\n", uVol);

	mid_audio_volume_set(uVol);
	return CLOUD_OK;
}
C_RESULT CStb_SetMuteState(IN C_BOOL bMuteStatus)
{
	CLOUD_LOG_TRACE("CStb_SetMuteState muteStatus = %d\n", bMuteStatus);

	mid_audio_mute_set(bMuteStatus);
	return CLOUD_OK;
}

C_BOOL CStb_GetMuteState(void)
{
	CLOUD_LOG_TRACE("CStb_GetMuteState v_mute = \n");
	return mid_audio_mute_get();
}

C_RESULT CStb_GetTsSignalInfo(OUT C_TsSignal *pSignal)
{
	return CLOUD_OK;
}

static CStb_AVInjectTSData g_injectTSDataFun = NULL;

#define BUFFER_SIZE 1500
static void shell_cloud_loop(unsigned int localaddr, unsigned int multiaddr, unsigned int multport)
{
    int sock, length;
    unsigned char buffer[BUFFER_SIZE];
    struct sockaddr_in sin;
    struct ip_mreq ipmreq;

    fd_set fdset;
    struct timeval tv;

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = multiaddr;
    sin.sin_port = htons((ushort)multport);

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (bind(sock, (struct sockaddr *)&sin, sizeof(sin)))
        CLOUD_LOG_TRACE("@@@@@@@@ , err = %s\n", strerror(errno));

    ipmreq.imr_multiaddr.s_addr = multiaddr;
    ipmreq.imr_interface.s_addr = localaddr;
    if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&ipmreq, sizeof(ipmreq)) < 0)
        CLOUD_LOG_TRACE("@@@@@@@@ , err = %s\n", strerror(errno));

    FD_ZERO(&fdset);
    FD_SET(sock, &fdset);

    length = 0;

    for (;;) {
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        length = select(sock + 1, &fdset, NULL,  NULL, &tv);
        if (length <= 0)
            continue;

        length = recv(sock, buffer, BUFFER_SIZE, 0);
        //CLOUD_LOG_TRACE("@@@@@@@@ recvlen = %d---------------------\n", len);
        if (length <= 0)
            continue;

        g_injectTSDataFun(buffer, length);
    }

    ipmreq.imr_multiaddr.s_addr = multiaddr;
    ipmreq.imr_interface.s_addr = localaddr;
    if (setsockopt(sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char *)&ipmreq, sizeof(ipmreq)) < 0)
        CLOUD_LOG_TRACE("@@@@@@@@  err = %s\n", strerror(errno));

    close(sock);
}

int shell_cloud(int argc, char **argv)
{
    unsigned int multaddr, multport, localaddr;

    localaddr = inet_addr("110.1.1.153");
    multaddr = inet_addr("239.255.0.1");
    multport = 5001;

/*
    YX_MPEG_INFO mpeginfo;

    memset(&mpeginfo, 0, sizeof(mpeginfo));
    mpeginfo.stream_type = YX_STREAM_TS;

    mpeginfo.video_num = 1;
    mpeginfo.video_pid[0] = 0x45;
    mpeginfo.video_type[0] = YX_VIDEO_TYPE_H264;
    mpeginfo.audio_num = 1;
    mpeginfo.audio_pid[0] = 0x44;
    mpeginfo.audio_type[0] = YX_AUDIO_TYPE_MPEG;
 */

    {
        C_PIDParam pidParams;

        pidParams.uVideoPid = 0x45;
        pidParams.uVideoType = VideoType_H264;
        pidParams.uAudioPid = 0x44;
        pidParams.uAudioType = AudioType_MPEG2;

        CStb_AVPlayTSOverIP(&pidParams, &g_injectTSDataFun);
    }

    shell_cloud_loop(localaddr, multaddr, multport);

    codec_cloud_close( );
    return 0;
}
