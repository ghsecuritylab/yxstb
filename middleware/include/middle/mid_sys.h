#ifndef __MID_SYS_H__
#define __MID_SYS_H__

#ifdef __cplusplus
extern "C" {
#endif

enum {
	VideoFormat_UNKNOWN = -1,
	VideoFormat_NTSC = 0,
	VideoFormat_PAL,
	VideoFormat_SECAM,
	VideoFormat_480P,
	VideoFormat_576P,
	VideoFormat_720P50HZ,
	VideoFormat_720P60HZ,
	VideoFormat_1080I50HZ,
	VideoFormat_1080I60HZ,
	VideoFormat_1080P24HZ,
	VideoFormat_1080P25HZ,
	VideoFormat_1080P30HZ,
	VideoFormat_1080P50HZ,
	VideoFormat_1080P60HZ,
	VideoFormat_24P,
	VideoFormat_MAX
};

enum {
	AspectRatio_4x3 = 0,
	AspectRatio_16x9,
	AspectRatio_MAX
};

enum VisualizationType
{
    ASPECT_RATIO = 0,
    VIDEO_TYPE,
    AUDIO_TYPE,
    DECODER_FIELD_MODE,
    TRANCPORT_TYPE,
};

enum CollectInfoTYpe
{
    InfoTYpe_unknown = -1,
    InfoTYpe_stream_information,
    InfoTYpe_PIP_stream_information,
    InfoTYpe_OTT_information,
    InfoTYpe_MAX,
};


void mid_sys_serial_set(char *sn); // 17
void mid_sys_serial(char *buf); // 32
// void mid_sys_stbtype_get (char *buf);

void mid_sys_video_show(int flag);
void mid_sys_videoFormat_set(int fmt, int SetToUboot);
void mid_sys_aspectRatioMode_set(int pMode);
void mid_sys_Macrovision_set (int level);
int mid_sys_Macrovision_get (void);
int mid_sys_hdcp_enableflag_set(int enable);
int mid_sys_hdcp_enableflag_get(void);
int mid_sys_cgmsa_enableflag_set(int enable);
int mid_sys_cgmsa_enableflag_get(void);

int mid_sys_HDMIConnect_status_get(void);
int mid_sys_getVisualizationInfo(char *VisualizationInfo, int size);
int mid_sys_setCollectInfoType(int type);
int mid_sys_getCollectInfoType(void);
int mid_sys_getVisualizationStreamInfo(char *visualizationInfo, int size, int infoType);
void mid_sys_setStartCollectDebugInfo(int flag);
int mid_sys_getStartCollectDebugInfo(void);
void startCollectVisualizationDataThread();


#ifdef __cplusplus
}
#endif

#endif//__MID_SYS_H__

