/**********************************************************
    Copyright (c) 2008-2009, Yuxing Software Corporation
    All Rights Reserved
    Confidential Property of Yuxing Softwate Corporation

    Revision History:

    Created: 2009-10-22 9:30:31 by liujianhua

 **********************************************************/

#include "stream.h"

static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

uint32_t mid_stream_open(int idx, const char* url, APP_TYPE apptype, int shiftlen)
{
    LOG_STRM_DEBUG("#%d apptype = %d, shiftlen = %d\n", idx, apptype, shiftlen);
    return mid_stream_open_range(idx, url, apptype, shiftlen, -1, 0);
}

uint32_t mid_stream_open_vodAdv(int idx, struct VODAdv* vodadv, int starttime)
{
    LOG_STRM_DEBUG("#%d adv_num = %d\n", idx, vodadv->adv_num);
    return mid_stream_open_range(idx, (char*)vodadv, APP_TYPE_VODADV, starttime, -1, 0);
}

void mid_stream_save(int flag)
{
    LOG_STRM_PRINTF("\n");
    int_stream_cmd(STREAM_CMD_TEST_SAVE, flag, 0, 0, 0);
}

void mid_stream_bandwidth(int bandwidth)
{
    LOG_STRM_PRINTF("\n");
    int_stream_cmd(STREAM_CMD_TEST_BANDWIDTH, bandwidth, 0, 0, 0);
}

void mid_stream_timeshift_open(void)
{
    LOG_STRM_PRINTF("\n");
    int_stream_cmd(STREAM_CMD_TIMESHIFT_OPEN, 0, 0, 0, 0);
}

void mid_stream_timeshift_close(void)
{
    LOG_STRM_PRINTF("\n");
    int_stream_cmd(STREAM_CMD_TIMESHIFT_CLOSE, 0, 0, 0, 0);
}

static int g_timeshift_jump = 0;
void mid_stream_timeshift_jump(int jump)
{
    g_timeshift_jump = jump;
}

int int_stream_timeshift_jump(void)
{
    return g_timeshift_jump;
}

static uint32_t g_rrs_timeout = 2000;
void mid_stream_rrs_timeout(int ms)
{
    if (ms < 0 || ms > 60 * 1000)
        LOG_STRM_ERROUT("ms = %d\n", ms);
    LOG_STRM_PRINTF("ms = %d\n", ms);

    g_rrs_timeout = ms;

Err:
    return;
}

uint32_t int_stream_rrs_timeout(void)
{
    return g_rrs_timeout / 10;
}

static int g_bufferrate = 0;
void int_back_hls_bufferrate(int bufferrate)
{
    g_bufferrate = bufferrate;
}
int mid_stream_hls_buffrate(void)
{
    return g_bufferrate;
}

static unsigned int g_recordrate = 0;
void int_back_hls_recordrate(unsigned int recordrate)
{
    g_recordrate = recordrate;
}
unsigned int mid_stream_hls_recordrate(void)
{
    return g_recordrate;
}

//单位：比特每秒
static unsigned int g_playrate = 0;
void int_back_hls_playrate(unsigned int playrate)
{
    g_playrate = playrate;
}
unsigned int mid_stream_hls_playrate(void)
{
    return g_playrate;
}

#define RTSP_METHOD_SIZE   16
typedef struct {
    char url[STREAM_URL_SIZE];
    char method[RTSP_METHOD_SIZE];
} RTSPInfo;

static RTSPInfo *g_rtspInfoArray = NULL;
void int_back_rtspURL(int idx, char *url)
{
    if (0 != idx && 1 != idx)
        return;
    if (!url )
        return;
    
    if (!g_rtspInfoArray)
        g_rtspInfoArray = (RTSPInfo*)IND_CALLOC(sizeof(RTSPInfo) * 2, 1);

    if (!g_rtspInfoArray)
        return;

    strcpy(g_rtspInfoArray[idx].url, url);
}

void int_back_rtspMethod(int idx, char *method)
{
    if (0 != idx && 1 != idx)
        return;
    if (!method || strlen(method) >= RTSP_METHOD_SIZE)
        return;
    if (!g_rtspInfoArray) {
        g_rtspInfoArray = (RTSPInfo*)IND_CALLOC(sizeof(RTSPInfo) * 2, 1);
        if (!g_rtspInfoArray)
            return;
    }

    strcpy(g_rtspInfoArray[idx].method, method);
}

void mid_stream_get_rtspInfo(int idx, char *url, char *method)
{
    RTSPInfo *info;

    if (!url)
        return;
    url[0] = 0;
    if (method)
        method[0] = 0;

    if (0 != idx && 1 != idx)
        return;

    if (!g_rtspInfoArray)
        return;
    info = &g_rtspInfoArray[idx];
    strcpy(url, info->url);
    if (method)
        strcpy(method, info->method);
}

static int gAudioChannels = 0;
static int gAudioChannels1 = 0;
static int* gAudioBitRateArray = NULL;
static int* gAudioBitRateArray1 = NULL;
void int_steam_setAudioChannels(int idx, int audioChannels)
{
    pthread_mutex_lock(&g_mutex);
    if (1 == idx) {
        gAudioChannels1 = audioChannels;
        if (gAudioBitRateArray1) {
            free(gAudioBitRateArray1);
            gAudioBitRateArray1 = NULL;
        }
        if (audioChannels > 0)
            gAudioBitRateArray1 = (int*)calloc(sizeof(int), audioChannels);
    } else {
        gAudioChannels = audioChannels;
        if (gAudioBitRateArray) {
            free(gAudioBitRateArray);
            gAudioBitRateArray = NULL;
        }
        if (audioChannels > 0)
            gAudioBitRateArray = (int*)calloc(sizeof(int), audioChannels);
    }
    pthread_mutex_unlock(&g_mutex);
}

static int gTransportProtocol = 0;
static int gTransportProtocol1 = 0;
void int_steam_setTransportProtocol(int idx, int transportProtocol)
{
    if (1 == idx)
        gTransportProtocol1 = transportProtocol;
    else
        gTransportProtocol = transportProtocol;
}

void int_steam_setAudioBitRate(int idx, int audioIndex, int audioBitRate)
{
    pthread_mutex_lock(&g_mutex);
    if (1 == idx) {
        if (audioIndex >= 0 && audioIndex < gAudioChannels1)
            gAudioBitRateArray1[audioIndex] = audioBitRate;
    } else {
        if (audioIndex >= 0 && audioIndex < gAudioChannels)
            gAudioBitRateArray[audioIndex] = audioBitRate;
    }
    pthread_mutex_unlock(&g_mutex);
}

static long long gPacketLost = 0;
static long long gPacketLost1 = 0;
void int_steam_setPacketLost(int idx, int packetLost)
{
    pthread_mutex_lock(&g_mutex);
    if (packetLost > 0) {
        if (1 == idx)
            gPacketLost1 += packetLost;
        else
            gPacketLost += packetLost;
    } else {
        LOG_STRM_PRINTF("Reset PacketLost%d\n", idx);
        if (1 == idx)
            gPacketLost1 = 0;
        else
            gPacketLost = 0;
    }
    pthread_mutex_unlock(&g_mutex);
}

static long long gContinuityError = 0;
static long long gContinuityError1 = 0;
void int_steam_setContinuityError(int idx, int continuityError)
{
    pthread_mutex_lock(&g_mutex);
    if (continuityError > 0) {
        if (1 == idx)
            gContinuityError1 += continuityError;
        else
            gContinuityError += continuityError;
    } else {
        LOG_STRM_PRINTF("Reset ContinuityError%d\n", idx);
        if (1 == idx)
            gContinuityError1 = 0;
        else
            gContinuityError = 0;
    }
    pthread_mutex_unlock(&g_mutex);
}

static int gDownloadRate = 0;
void int_steam_setDownloadRate(int downloadRate)
{
    gDownloadRate = downloadRate;
}

static int gRemainPlaytime = 0;
void int_steam_setRemainPlaytime(int remainPlaytime)
{
    gRemainPlaytime = remainPlaytime;
}

static int gCurBufferSize = 0;
void int_steam_setCurBufferSize(int curBufferSize)
{
    gCurBufferSize = curBufferSize;
}

static int gToalBufferSize = 0;
void int_steam_setToalBufferSize(int toalBufferSize)
{
    gToalBufferSize = toalBufferSize;
}

static int gStreamNum = 0;
static int *gStreamBandwith = NULL;
static char **gStreamURL = NULL;

static void int_steam_freetStream(void)
{
    if (gStreamNum > 0) {
        if (gStreamBandwith) {
            free(gStreamBandwith);
            gStreamBandwith = NULL;
        }
        if (gStreamURL) {
            int i;
            char *url;
            for (i = 0; i < gStreamNum; i++) {
                url = gStreamURL[i];
                if (url)
                    free(url);
            }
            free(gStreamURL);
            gStreamURL = NULL;
        }
        gStreamNum = 0;
    }
}
void int_steam_setStreamNum(int streamNum)
{
    pthread_mutex_lock(&g_mutex);
    int_steam_freetStream( );

    gStreamNum = streamNum;
    if (gStreamNum > 0) {
        gStreamBandwith = (int*)calloc(sizeof(int), streamNum);
        gStreamURL = (char**)calloc(sizeof(char*), streamNum);
    }
    pthread_mutex_unlock(&g_mutex);
}

void int_steam_setStreamInfo(int idx, int bandwith, char *url)
{
    LOG_STRM_PRINTF("stream[%d]: %s %d\n", idx, url, bandwith);
    pthread_mutex_lock(&g_mutex);
    if (idx >= 0 && idx < gStreamNum) {
        gStreamBandwith[idx] = bandwith;
        if (gStreamURL[idx])
            free(gStreamURL[idx]);
        gStreamURL[idx] = strdup(url);
    }
    pthread_mutex_unlock(&g_mutex);
}

static char *gCurSegment = NULL;
static char **gSegmentList = NULL;
static int gSegmentNum = 0;
#define SEGMENTNUM 20
void int_steam_setCurSegment(char *url)
{
    int i;

    if (url)
        LOG_STRM_PRINTF("new segment %s\n", url);
    else
        LOG_STRM_PRINTF("free segment\n");

    pthread_mutex_lock(&g_mutex);
    if (url) {
        if (!gSegmentList) {
            gSegmentList = (char**)calloc(sizeof(char*), SEGMENTNUM);
            if (!gSegmentList)
                goto End;
        }

        if (gCurSegment) {
            if (gSegmentNum < SEGMENTNUM) {
                gSegmentList[gSegmentNum] = gCurSegment;
                gSegmentNum++;
            } else {
                free(gSegmentList[0]);
                for (i = 0; i < SEGMENTNUM - 1; i++)
                    gSegmentList[i] = gSegmentList[i + 1];
                gSegmentList[SEGMENTNUM - 1] = gCurSegment;
            }
        }

        i = 0;
        while ('?' != url[i] && 0 != url[i])
            i++;
        gCurSegment = (char*)malloc(i + 1);
        if (gCurSegment) {
            if (i > 0)
                memcpy(gCurSegment, url, i);
            gCurSegment[i] = 0;
        }
    } else {
        if (gCurSegment) {
            free(gCurSegment);
            gCurSegment = NULL;
        }
        if (gSegmentList) {
            for (i = 0; i < gSegmentNum; i++)
                free(gSegmentList[i]);
            free(gSegmentList);
            gSegmentList = NULL;
            gSegmentNum = 0;
        }
    }

End:
    pthread_mutex_unlock(&g_mutex);
}

static int gPlayCounter = 0;
static int gAbnormalFlag = 1;
static int gAbnormalCounter = 0;

void int_steam_postPlay(void)
{
    pthread_mutex_lock(&g_mutex);
    gPlayCounter++;
    gAbnormalFlag = 0;
    pthread_mutex_unlock(&g_mutex);
}
void int_steam_postAbnormal(void)
{
    pthread_mutex_lock(&g_mutex);
    if (!gAbnormalFlag) {
        gAbnormalFlag = 1;
        gAbnormalCounter++;
    }
    pthread_mutex_unlock(&g_mutex);
}

int mid_stream_getInt(char *paramName, int arg)
{
    int ret = 0;
    if (!paramName)
        LOG_STRM_ERROUT("paramName is NULL\n");

    if ('A' == paramName[0]) {
        pthread_mutex_lock(&g_mutex);
        if (0 == strcmp("AbnormalInterval", paramName)) {
            if (gPlayCounter <= 0)
                ret = 0;
            else
                ret = gAbnormalCounter * 1000 / gPlayCounter;
        } else if (0 == strcmp("AudioBitRate", paramName)) {
            if (arg >= 0 && arg < gAudioChannels)
                ret = gAudioBitRateArray[arg];
            else
                LOG_STRM_ERROR("arg = %d / %d\n", arg, gAudioChannels);
        } else if (0 == strcmp("AudioBitRate1", paramName)) {
            if (arg >= 0 && arg < gAudioChannels1)
                ret = gAudioBitRateArray1[arg];
            else
                LOG_STRM_ERROR("arg = %d / %d\n", arg, gAudioChannels1);
        } else if (0 == strcmp("AudioChannels", paramName)) {
            ret = gAudioChannels;
        } else if (0 == strcmp("AudioChannels1", paramName)) {
            ret = gAudioChannels1;
        }
        pthread_mutex_unlock(&g_mutex);
        return ret;
    } else if ('C' == paramName[0]) {
        if (0 == strcmp("CurBufferSize", paramName))
            return gCurBufferSize;
    } else if ('D' == paramName[0]) {
        if (0 == strcmp("DownloadRate", paramName))
            return gDownloadRate;
    } else if ('P' == paramName[0]) {
        if (0 == strcmp("Playrate", paramName))
            return strm_play_byte_rate(NULL);
    } else if ('R' == paramName[0]) {
        if (0 == strcmp("RemainPlaytime", paramName))
            return gRemainPlaytime;
    } else if ('S' == paramName[0]) {
        pthread_mutex_lock(&g_mutex);
        if (0 == strcmp("StreamBandwith", paramName)) {
            if (arg >= 0 && arg < gStreamNum)
                ret = gStreamBandwith[arg];
            else
                LOG_STRM_ERROR("arg = %d / %d\n", arg, gStreamNum);
        } else if (0 == strcmp("StreamNum", paramName)) {
            ret = gStreamNum;
        } else if (0 == strcmp("SegmentNum", paramName)) {
            ret = gSegmentNum;
        }
        pthread_mutex_unlock(&g_mutex);
        return ret;
    } else if ('T' == paramName[0]) {
        if (0 == strcmp("ToalBufferSize", paramName))
            return gToalBufferSize;
        if (0 == strcmp("TransportProtocol1", paramName))
            return gTransportProtocol1;
        if (0 == strcmp("TransportProtocol", paramName))
            return gTransportProtocol;
    }
    LOG_STRM_ERROR("Unknown param '%s'\n", paramName);

Err:
    return 0;
}

void mid_stream_getString(char *paramName, int arg, char *buf, int size)
{
    int len;
    char *url;

    if (!buf || size <= 0)
        return;
    buf[0] = 0;

    if (!paramName)
        return;

    url = buf;//用于判断URL是否有效
    pthread_mutex_lock(&g_mutex);
    if ('C' == paramName[0]) {
        if (0 == strncmp("ContinuityError", paramName, 15)) {
            if ('1' == paramName[15])
                snprintf(buf, size, "%lld", gContinuityError1);
            else
                snprintf(buf, size, "%lld", gContinuityError);
            goto End;
        }
        if (0 == strcmp("CurSegment", paramName))
            url = gCurSegment;
    } else if ('P' == paramName[0]) {
        if (0 == strncmp("PacketLost", paramName, 10)) {
            if ('1' == paramName[10])
                snprintf(buf, size, "%lld", gPacketLost1);
            else
                snprintf(buf, size, "%lld", gPacketLost);
            goto End;
        }
    } else if ('S' == paramName[0]) {
        if (0 == strcmp("SegmentList", paramName)) {
            url = gCurSegment;
            if (arg >= 0 && arg < gSegmentNum)
                url = gSegmentList[arg];
            else
                LOG_STRM_ERROR("arg = %d / %d\n", arg, gSegmentNum);
        } else if (0 == strcmp("StreamURL", paramName)) {
            if (arg >= 0 && arg < gStreamNum)
                url = gStreamURL[arg];
            else
                LOG_STRM_ERROR("arg = %d / %d\n", arg, gStreamNum);
        }
    }
    if (url == buf) {
        LOG_STRM_ERROR("Unknown param '%s'\n", paramName);
    } else {
        if (url) {
            len = strlen(url);
            if (len >= size)
                len = size - 1;
            if (len > 0)
                memcpy(buf, url, len);
            buf[len] = 0;
        }
    }
End:
    pthread_mutex_unlock(&g_mutex);
}

void mid_stream_setRouteCallback(StreamRouteCall func)
{
    ind_net_setRouteCall(func);
}