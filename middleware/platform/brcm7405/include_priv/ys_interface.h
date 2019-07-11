/***************************************************************************
 *     Copyright (c) 2005-2006, Yuxing Software Corporation
 *     All Rights Reserved
 *     Confidential Property of Yuxing Softwate Corporation
 *
 * $ys_Workfile: ys_interface.h $
 * $ys_Revision: 01 $
 * $ys_Date: 5/30/06 4:51p $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $ys_Log: /lib/interface/common/ys_interface.h $
 *
 * 1. create by zhangting  05/30/2006
 *
 ***************************************************************************/

#ifndef __YS_INTERFACE_H__
#define __YS_INTERFACE_H__

#ifdef DMALLOC
#include "dmalloc.h"
#endif

#include "yxuserinput.h"

#define MAX_PARTITION_NUM	20
#define UDP_PKT_SIZE		1316
#define MAX_READ_LEN		UDP_PKT_SIZE * 160

#define IP_RECV_TIMEOUT_USEC		250000
#define IP_RECV_BURST_USEC			(IP_RECV_TIMEOUT_USEC/5)

/* Max Supported Jitter for the IPSTB Configuration */
#define IP_NETWORK_JITTER 300

typedef struct __D_YsStream  YsStream;

typedef enum
{
	MACROVISION_NONE = 0,
	MACROVISION_LEVEL_1,
	MACROVISION_LEVEL_2,
	MACROVISION_LEVEL_3

}MACROVISION_LEVEL;

typedef enum
{
	CGMSA_DISABLE = -1,
	CGMSA_COPY_FREELY = 0,
	CGMSA_COPY_NO_MORE,
	CGMSA_COPY_ONCE,
	CGMSA_COPY_NEVER

}CGMSA_LEVEL;

typedef enum
{
	SPDIF_OFF = 0,
	PASSTHROUGH,
	DOWNMIX,
}SPDIF_OUT_MODE;

typedef enum
{
	GRAPHIC_CLONE_MODE_NONE = 0,
	GRAPHIC_CLONE_MODE_HD_TO_SD,
	GRAPHIC_CLONE_MODE_SD_TO_HD,
}GRAPHIC_CLONE_MODE;

typedef enum
{
	TUNER_MODE_NONE = 0,
	TUNER_MODE_ANALOG,
	TUNER_MODE_DTMB,
	TUNER_MODE_DVBT,
	TUNER_MODE_DVBS,
	TUNER_MODE_DVBC,
	TUNER_MODE_ALL,
}TUNER_MDOE;

typedef enum
{
	HDMI_AUDIO_COMPRESSED = 0,
	HDMI_AUDIO_PCM,
	HDMI_AUDIO_PCM_6CH,

	HDMI_AUDIO_OFF,
}HDMI_AUDIO_MODE;

typedef enum
{
	DAC_OUTPUT_COMPRESSED = 0,
	DAC_OUTPUT_DOWNMIX,
}DAC_OUTPUT_MODE;

typedef enum
{
	HDMI_COLOR_SPACE_RGB = 0,
	HDMI_COLOR_SPACE_YUV,
}HDMI_COLOR_SPACE;

typedef struct
{
	YX_TV_SYSTEM         	   hd_tvsystem;
	YX_TV_SYSTEM             sd_tvsystem;
	YX_DISPLAY_ASPECT_RATIO  aspect_ratio;
	YX_ASPECT_MODE   	       hd_aspect_mode;
	YX_ASPECT_MODE   	       sd_aspect_mode;
	SPDIF_OUT_MODE           spdif_mode;
	int                      hdmi_edid_format_valid;
	int                      b_init;
	int                      screen_width;
	int                      screen_height;
	int                      display_num;
	int                      hdmi;
	int                      enable_hd;
	int                      video_clone;
	GRAPHIC_CLONE_MODE       graphic_clone;
	int                      graphic_offset_x;
	int                      graphic_offset_y;
	int                      graphic_buffer_num;
	int                      enable_analog_hd;
	int                      hdcp_mode;
	int	                     mem_size;
	int                      tuner_mode;
	int	                     macrovision_mode;
	HDMI_AUDIO_MODE          hdmi_audio_mode;
	DAC_OUTPUT_MODE          dac_output_mode;
	int                      playbuffer_num;
	int                      discard_parse_psibuf;
	YX_COMPONET_TYPE 				   component_type;
	int                      spdif_output_disable;
	int                      dac_output_disable;
	int                      hdmi_output_disable;
	int                      component_output_disable;
	int                      svideo_output_disable;
	int                      composite_output_disable;
	int                      b_have_pvr;
	int                      hdcp_failure_mode;
	YX_COLOR_MODE            color_mode;
	int                      ac3_license;
	int                      dts_license;
	int                      deinterlace;
	YX_AVSYNC_MODE           avsync_mode;
	YX_AVSYNC_MODE           dvb_syncmode;
} YS_SETTINGS;

typedef enum
{
	PVR = 0,
	UDP,
	RTSP,
	HTTP,
	FTP
} PLAY_PROTOCOL;



typedef enum
{
	BUFFER_NORMAL = 0,
	BUFFER_UNDERFLOW,
	BUFFER_OVERFLOW,
}BUFFER_STATUS;

typedef enum
{
	IP_MODE = 0,
	TV_MODE,
	DVB_MODE,
}PLAY_MODE;

typedef enum
{
	LZMA = 0,
	RAW,
	GZIP,
	STILL,
	JPEG,
} COMPRESS_MODE;

typedef struct
{
	short tv_system;
	short width;
	short height;
	short color_mode;
	short compress_mode;
	short progress_bar_x;
	short progress_bar_y;
	short progress_bar_width;
	short progress_bar_height;
	short progress_bar_style;
	unsigned int progress_bar_color;
	int size;
	char  reserve[4];
}OSD_INFO;

typedef struct
{
	unsigned int checksum;
	char product_id[12];
	unsigned int pcb_version;
	unsigned int chip_version;
	char product_serial_code[16];
	unsigned int net_mac_base_addr;
	unsigned char net_mac_addr[8];
	unsigned int user_size;
	unsigned int sys_mem;
	char reserve[4];
	OSD_INFO osd1_info;
	char product_serial_code1[32];
}YS_CFG_INFO;



typedef struct
{
	int bootflag;
	char partition_name[12];
	unsigned int partition_offset;
	unsigned int partition_size;
	int software_main_version;
	int software_sub_version;
	int software_svn_version;
}YS_PARTITION_INFO;

typedef enum
{
	PARTITION_BOOT_NORMAL=0,
	PARTITION_BOOT_BACKUP,
	PARTITION_BOOT_NEW,
}YS_PARTITION_BOOTFLAG;

typedef struct
{
	unsigned int checksum;
	int partition_num;
	YS_PARTITION_INFO partition_info[MAX_PARTITION_NUM];
	char reserve[16];
}YS_PARTITION_TABLE;

typedef struct
{
	char tag;
	unsigned char volume;
	unsigned char tv_system;
	unsigned char aspect;
	short graphic_width;
	short graphic_height;
	char graphic_offset_x;
	char graphic_offset_y;
	unsigned char spdif_hdmi;
	unsigned char box_mode;
	unsigned char hdcp_macvision;
	unsigned char cgms;
	unsigned char debug_mode;
	char runlevel;
	char boot_params;
	unsigned char mem_size;
	unsigned char tuner_mode;
	unsigned char frontpanel_version;
	unsigned char frontpanel_upgrade;
	char reserve[11];
	unsigned char PPPoE[8];
}YS_EEPROM_PARAMETER;

typedef enum
{
	EEPROM_TAG = 0,
	EEPROM_VOLUME = 1,
	EEPROM_TV_SYSTEM = 2,
	EEPROM_ASPECT = 3,
	EEPROM_GRAPHIC_WIDTH = 4,
	EEPROM_GRAPHIC_HEIGHT = 6,
	EEPROM_GRAPHIC_OFFSET_X = 8,
	EEPROM_GRAPHIC_OFFSET_Y = 9,
	EEPROM_SPDIF_HDMI = 10,
	EEPROM_BOX_MODE = 11,
	EEPROM_HDCP_MACVISION = 12,
	EEPROM_CGMS = 13,
	EEPROM_DEBUG_MODE = 14,
	EEPROM_RUN_LEVEL = 15,
	EEPROM_BOOT_PARAM = 16,
	EEPROM_MEM_SIZE = 17,
	EEPROM_TUNER_MODE = 18,
	EEPROM_FRONTPANEL_VERSION = 19,
	EEPROM_FRONTPANEL_UPGRADE = 20,
	EEPROM_FLASH_UPDATE = 21,
	EEPROM_PPPOE = 32,
	EEPROM_NVRAM_STARTUP = 40,
	EEPROM_NVRAM_KERNEL_INDEX = 41,
	EEPROM_NVRAM_ROOTFS_INDEX = 42,
	EEPROM_NVRAM_BAKKERNEL_INDEX = 43,
	EEPROM_NVRAM_BAKROOTFS_INDEX = 44,
	EEPROM_NVRAM_BOOTTIME = 45,
	EEPROM_NVRAM_BOOTOK = 46,
	EEPROM_NVRAM_KERNEL0_VERSION = 47,
	EEPROM_NVRAM_ROOTFS0_VERSION = 48,
	EEPROM_NVRAM_KERNEL1_VERSION = 49,
	EEPROM_NVRAM_ROOTFS1_VERSION = 50,
	EEPROM_NVRAM_CFE_VERSION = 51,
	EEPROM_NVRAM_CONFIG0_VERSION = 52,
	EEPROM_NVRAM_CONFIG1_VERSION = 53,
	EEPROM_NVRAM_KERNEL_NEW_FLAG = 54,
	EEPROM_NVRAM_ROOTFS_NEW_FLAG = 55,
	EEPROM_NVRAM_MAX = 60,
}YS_EEPROM_OFFSET;

typedef enum
{
	HDMI_NORMAL = 0,
	HDMI_BLUE_SCREEN,
	HDMI_TO_SD,
	HDMI_GRAPHIC_ONLY,
	HDMI_FAILURE_MODE_MAX = HDMI_GRAPHIC_ONLY,
}YS_HDCP_FAILURE_MODE;

typedef struct
{
	unsigned int source_width;
	unsigned int source_height;
	int binterlaced;
	YX_VIDEO_FRAME_RATE frame_rate;
	YX_ASPECT_RATIO aspect_ratio;
	unsigned int audio_sample_rate;
	unsigned int audio_bitrate;
	YX_TV_SYSTEM video_format;
	YX_AUDIO_TYPE audio_format;

	unsigned int video_pts;
	unsigned int audio_pts;
	unsigned int video_stc;
	unsigned int audio_stc;

	unsigned long video_fifo_depth;
	unsigned long video_fifo_size;
	unsigned long audio_fifo_depth;
	unsigned long audio_fifo_size;

	YX_TIME_CODE time_code;

	int bdisco_event;
	int bunderflow_event;

}YS_DECODER_STATUS;

typedef enum
{
	YS_TUNER_TYPE_UNKNOWN = 0,
	YS_TUNER_TYPE_QAM,
	YS_TUNER_TYPE_SATELLITE,
	YS_TUNER_TYPE_OOB,
	YS_TUNER_TYPE_VSB,
	YS_TUNER_TYPE_OFDM,

	YS_TUNER_TYPE_ANALOG,

}YS_TUNER_TYPE;

typedef enum
{
	YS_TUNER_QAM_MODE_16 = 0,
	YS_TUNER_QAM_MODE_32,
	YS_TUNER_QAM_MODE_64,
	YS_TUNER_QAM_MODE_128,
	YS_TUNER_QAM_MODE_256,
	YS_TUNER_QAM_MODE_512,
	YS_TUNER_QAM_MODE_1024,
	YS_TUNER_QAM_MODE_2048,
	YS_TUNER_QAM_MODE_4096,
	YS_TUNER_QAM_MODE_AUTO_64_256,

	YS_TUNER_QAM_MODE_UNKNOWN,
}YS_TUNER_QAM_MODE;

typedef enum
{
	YS_TUNER_SATELLITE_MODE_DVB = 0,
	YS_TUNER_SATELLITE_MODE_DSS,
	YS_TUNER_SATELLITE_MODE_DCII,
	YS_TUNER_SATELLITE_MODE_TURBO_QPSK,
	YS_TUNER_SATELLITE_MODE_TURBO_8PSK,
	YS_TUNER_SATELLITE_MODE_QPSK_LDPC,
	YS_TUNER_SATELLITE_MODE_8PSK_LDPC,

 	YS_TUNER_SATELLITE_MODE_UNKNOWN,
}YS_TUNER_SATELLITE_MODE;

typedef struct
{
	unsigned int frequency;
	YS_TUNER_QAM_MODE mode;
	int symbol_rate;
	int b_terrestrial;

}YS_TUNER_QAM_PARAM;

typedef struct
{
	unsigned int frequency;
	YS_TUNER_SATELLITE_MODE mode;
	unsigned int symbol_rate;
	unsigned int carrier_offset;
	unsigned int search_range;
	unsigned int ldpc_pilot;
	unsigned int ldpc_pilot_pll;

}YS_TUNER_SATELLITE_PARAM;

typedef struct
{
	int b_locked;

	int signal_quality;
	int signal_intensity;

}YS_TUNER_STATUS;

typedef enum
{
	YS_MESSAGE_TYPE_TS = 0,
	YS_MESSAGE_TYPE_PES,
	YS_MESSAGE_TYPE_PSI

}YS_MESSAGE_TYPE;

#define YS_MESSAGE_FILTER_SIZE 16

typedef struct
{
	unsigned char mask[YS_MESSAGE_FILTER_SIZE];
	unsigned char coefficient[YS_MESSAGE_FILTER_SIZE];
	unsigned char exclusion[YS_MESSAGE_FILTER_SIZE];

}YS_MESSAGE_FILTER;

typedef struct
{
	int parse_band;
	int pid;
	int buffer_size;
	int use_message_filter;
	int DisablePsiCrc;
	YS_MESSAGE_TYPE type;

	YS_MESSAGE_FILTER filter;

}YS_MESSAGE_SETTINGS;

typedef enum
{
	YS_DVB_SIGNAL_NONE = 0,
	YS_DVB_SIGNAL_WEAK,
	YS_DVB_SIGNAL_NORMAL,
}YS_DVB_SIGNAL;

int ys_board_init(void);
int ys_board_uninit(void);
int ys_init(void);
int ys_uninit(void);
int ys_set_init_settings(YS_SETTINGS *settings);
int ys_get_default_settings(YS_SETTINGS *settings);
int ys_get_settings(YS_SETTINGS *settings);

int ys_set_audiopid(int audio_pid);
int ys_get_audiopid(int *audio_pid);
int ys_get_audiopid_by_index(int index,int * audio_pid);
int ys_get_audiopid_index(int * index);
int ys_get_num_of_audio(void);
int ys_set_audio_channels(int audio_channels);

int ys_audio_mute(int mute,int temp_set);
int ys_get_mute_status(void);

int ys_get_macrovision_level(int *level);
int ys_set_aspectratio(int as_ratio);
int ys_get_aspectratio(int *as_ratio);
int ys_set_aspectratio_mode(int mode);
int ys_get_aspectratio_mode(int *mode);
int ys_get_video_handle(void);
int ys_set_video_show(int show);
int ys_get_video_show(void);
int ys_set_video_show_mode(YX_DISPLAY_MODE mode);
int ys_get_video_show_mode(void);
int ys_set_hdcp_mode(int mode);
int ys_get_hdcp_mode(void);
int ys_set_hdcp_status(int status);
int ys_get_hdcp_status(int *status);
int ys_set_spdif_output_mode(int mode);
int ys_set_hdmi_audio_mode(int mode);
int ys_set_dac_output_mode(int mode);
int ys_set_video_zorder(int zorder);
int ys_get_video_zorder(int *zorder);
int ys_set_component_type(int type);
int ys_get_component_type(int *type);
int ys_set_hdmi_colorspace(int color_space);
int ys_get_hdmi_colorspace(int *color_space);
int ys_get_video_fullscreen(int *full);
int ys_get_spdif_output_mode(void);
int ys_get_hdmi_audio_mode(void);


//add by lhw
int ys_get_playpumpbuffer_statuslevel();
int ys_get_buffer_size();
int ys_set_mms_flush_flag(int flag);

int ys_start_mpegplay(char *addr,int port);
int ys_stop_mpegplay(void);

int ys_get_decode_finished(int *b_finished);
int ys_wait_playpump_callback(int timeout_ms);
int ys_get_playstatus(void);
int ys_set_playstatus(int playstatus,int rate);
int ys_set_livemode(int blive_mode);
int ys_get_livemode(int *blive_mode);
int ys_set_play_watermark(int nBytes);

int ys_audio_start(int audio_type);
int ys_audio_stop(void);
int ys_pcm_start(int sample_rate,int bits_per_sample,int channels);
int ys_pcm_wait_callback(void);
int ys_pcm_get_status(int handle,int *bStart,int *noplay_bytes);

int ys_surface_create(int width, int height, YX_COLOR_MODE colorMode, int visible, int *surfaceHandle);
int ys_surface_copy(int dst_surfaceHandle, int dst_x ,int dst_y,int dst_w,int dst_h,
					   int src_surfaceHandle, int src_x, int src_y,int src_w,int src_h);
int ys_surface_sync_middlefb( void );
void ys_surface_sync_realfb( void );

int ys_graphic_create(YX_COLOR_MODE mode, int *graphic_handle);
int ys_graphic_destroy(void);
int ys_graphic_set_clone_mode(int mode);
int ys_graphic_get_enabled(int *b_enabled);
int ys_graphic_set_enabled(int b_enabled);

int ys_get_mtdinfo_from_name(char *part_name,int *index,int *size,int *earse_size );
int ys_flash_write(char *part_name,char *databuf,int offset,int len);
int ys_flash_read(char *part_name,char *databuf,int offset,int len);
int ys_flash_erase(char *part_name,int offset,int len);
int ys_flash_update(char *part_name,char *databuf,int offset,int len);
int ys_flash_update_fullpartiton(char *part_name,char *databuf,int len);
int ys_flash_get_update_bytes(void);
int ys_flash_set_update_start(void);
int ys_flash_erase_fullpartition(char *part_name);
int ys_flash_verify(char *part_name,char *databuf,int offset,int len);
int ys_flash_get_badblock_num(char *part_name);
int ys_flash_set_write_oob(int writeoob);
int ys_flash_get_block_oobsize(char *part_name);

int nvram_getTunerInfo(unsigned int *addr,int *size);
int nvram_getAppInfo(unsigned int *addr, int *size);
int nvram_burn(unsigned int offset,char *buffer,int size);
int nvram_read(unsigned int offset,char *buffer,int size);

int ys_get_playpumpbuffer_status(BUFFER_STATUS *buffer_status,int *free_size);
int ys_set_playpumpbuffer_undermark(int mark);
int ys_set_playpumpbuffer_overmark(int mark);

void ys_set_socket_flush(int flag);
int ys_set_playmode(int play_mode);
int ys_get_playmode(void);

unsigned int ys_cfg_get_checksum(void *buffer,int len);
unsigned int ys_cfg_string_to_num(char *str);
int ys_cfg_get_partition_table(YS_PARTITION_TABLE *partition_table);
int ys_cfg_set_partition_table(YS_PARTITION_TABLE *partition_table);
int ys_cfg_get_software_info(char *partition_name,char *info);
int ys_cfg_get_serial_code(void *info);
int ys_cfg_set_serial_code(void *info);
int ys_cfg_get_mac_addr(void *addr);
int ys_cfg_set_mac_addr(void *addr);
int ys_cfg_get_mac_addr_from_serialcode(char *serialcode,unsigned char *mac_addr);
int ys_cfg_get_hdcp_key(void *buffer,int size,int *key_size);
int ys_cfg_set_hdcp_key(void *buffer,int size);
int ys_cfg_get_hardware_info( YX_HW_INFO *info );

int sntp_add_server(char *servername);
int sntp_del_server(char *servername);
char *sntp_get_server(void);
int sntp_set_server(char *servername);
int sntp_get_timezone(void);
int sntp_set_timezone(int timezone);
int sntp_update_time(void);
int sntp_set_timeout(int msec);

int ys_send_padt(void);

int http_fetch(const char *url_tmp, char **fileBuf);
int http_fetch_get_error(void);
int ys_tcp_init(void);

int ys_set_runlevel(unsigned char level);
int ys_set_factory_mode(void);

int ys_backdoor_run(void);
int ys_backdoor_set_callback(int (*backdoor)(void));

int ys_subtitle_start(void);
int ys_subtitle_stop(void);
void ys_subtitle_filter(char *buffer,int size);
int ys_subtitle_get_show(void);
int ys_subtitle_set_show(int show);
int ys_subtitle_get_pid(int *pid);
//int ys_subtitle_get_index(int *index);
int ys_subtitle_set_pid(unsigned short pid);
int ys_subtitle_set_index(int index);

int ys_get_av_mute(void);
int ys_set_av_mute(int bmute);

int ys_set_hdcp_failure_mode(int mode);
int ys_get_hdcp_failure_mode(void);

int ys_set_output_interface_status(int output_interface,int status);
int ys_get_output_interface_status(int output_interface,int *status);
int ys_set_hd_mode(int mode);
int ys_get_hd_mode(int *mode);
int ys_get_channelchange_time(int *millisecond);
int ys_set_component_output_mode(int hd_mode);
int ys_get_component_output_mode(int *hd_mode);
int ys_get_system_run_time(unsigned int *sec,unsigned int *msec);
int ys_set_output_interface_attribute(int type, int output_attribute);
int ys_get_output_interface_attribute(int type, int *output_attribute);
int ys_set_output_interface_resolution(int type, int resolution);
int ys_get_output_interface_resolution(int type, int *resolution);

int ys_front_turn_off(void);
int ys_front_set_power_light(int on);
int ys_front_show_text(char *text);
int ys_front_hide_text(void);
int ys_front_show_scrolling_text(char *text,int millisecond);
int ys_front_hide_scrolling_text(void);
int ys_front_set_net(int on);
int ys_front_update_clock(int timezone);
int ys_front_display_clock(void);
int ys_front_hide_clock(void);
int ys_front_power_control(void);
int ys_front_set_power_invalid(int value);
int ys_front_get_status(void);//add by lhw
int ys_front_set_poweroff_enable(int value);
int ys_front_set_pause(int value);

int ys_get_develop_mode(void);

int ys_get_stream_type(void);
int ys_set_stream_type(int type);
int ys_set_reparser_psi(int to_playstatus);

int ys_stream_create( const char *filename, YsStream **ret_stream );
int ys_stream_fopen( YsStream  *stream, void **ret_file );
int ys_stream_seekable( YsStream *stream );
int ys_stream_remote( YsStream *stream );
unsigned long long ys_stream_offset( YsStream *stream );
unsigned long long ys_stream_length( YsStream *stream );
int ys_stream_wait( YsStream *stream,unsigned int length,struct timeval *tv );
int ys_stream_peek( YsStream *stream,unsigned int length, long long offset,void *buf, unsigned int *read_out );
int ys_stream_read( YsStream *stream,unsigned int length,void *buf,unsigned int *read_out );
int ys_stream_seek( YsStream *stream,long long offset);
void ys_stream_destroy( YsStream *stream );
int ys_stream_set_playstart(int bstart);
void ys_playstream_function(void *data);

int ys_stream_play(char *url,int noOfLoops);
int ys_stream_stop(void);
int ys_pcmfile_play(char *url,int noOfLoops);
int ys_pcmfile_stop(void);

int ys_set_mpeg_info(YX_MPEG_INFO *mpeg_info);
int ys_get_mpeg_info(YX_MPEG_INFO *mpeg_info);
/*get HW pid channel index that video stream used, can only be called after ymm_decoder_startAudio() */
int ys_get_audio_pidIndex(void);
/*get HW pid channel index that audio stream used, can only be called after ymm_decoder_startVideo() */
int ys_get_video_pidIndex(void);
int ys_get_pip_video_pidIndex(void);
int ys_get_pip_audio_pidIndex(void);
int ys_set_debug_buffer_mode(int on, int time_interval_ms, int minimum_percentage);

int ys_asf_get_mpeg_info(char *chunk,int chunk_size,YX_MPEG_INFO *mpeg_info);

int ys_changeto_brcm_videocodec(YX_VIDEO_TYPE video_type);
int ys_changeto_brcm_audioformat(YX_AUDIO_TYPE audio_type);

int ys_media_get_stream_info(char *buffer,int size, YX_MPEG_INFO *info);

int ys_get_telnet_module_pid(void);

int ys_get_asf_udp_packet(int sockethandle,char *buffer,int *size,int wait);
int ys_asf_start_udp_play(char *ip_addr, int port);
int ys_asf_stop_udp_play(int mode);
int ys_asf_set_packetfilter_callback(int(*packetFilter)(unsigned char *, int));
int ys_net_start_udp_receive(int sockethandle);
int ys_net_stop_udp_receive(void);
int ys_net_set_rtsp_max_packet_size(int size);
int ys_asf_set_playstatus(int playstatus,int rate);
int ys_get_asf_tcp_packet(int sockethandle,char *buffer,int *size,int wait);
int ys_net_start_rtsp_receive(int sockethandle);
int ys_net_stop_rtsp_receive(void);
int ys_net_get_socketfd_by_handle(int socket);

yresult yhw_drm_init(void);
yresult yhw_drm_init_license_state(void);
int yhw_drm_get_license_state(void);
yresult yhw_drm_get_message_from_license_state(int state, char** msg);
yresult yhw_drm_get_encryption_key(char *chunk, int chunk_size , void **encryption_key);
yresult yhw_drm_init_by_tmpfs(void);
yresult yhw_drm_init_pre_delivery(void);
yresult yhw_drm_uninit_pre_delivery(void);
yresult yhw_drm_get_client_info(char *info_buffer,int *info_len);
yresult yhw_drm_process_pre_delivery(char *url);
yresult yhw_drm_cleanup_license_store(void);
yresult yhw_drm_set_local_license_file(char *license_file);
yresult yhw_drm_encode_url(char *src_buf,int src_len,char *dest_buf,int *dest_len);
yresult yhw_drm_reset_by_tmpfs(void);
yresult yhw_drm_get_challenge(char **url, char **data, int *len);
yresult yhw_drm_get_encryption_key_by_response(char *respone_buffer, int len , void **encryption_key);

int ys_get_playpumpbuffer_statuslevel(void);
int ys_clear_pre_audio(void);
int ys_get_dac_output_mode(DAC_OUTPUT_MODE *dac_mode);
int ys_get_play_watermark(int *nBytes);
int ys_pcmplay_init(void);
int ys_get_buffer_size(void);
int ys_set_playstatus_only(int playstatus);
int ys_set_hdmi_colorspace(int color_space);
int ys_get_hdmi_colorspace(int *color_space);
int ys_set_video_mute(void);
int ys_set_video_unmute(void);

int ys_get_videopid(int *video_pid);

int ys_dvb_tuner_open(int tuner_id);
int ys_dvb_tuner_close(int tuner_id);
int ys_dvb_tune_tuner(int tuner_handle, int tuner_type, void *param );
int ys_dvb_get_tuner_status(int tuner_handle, int tuner_type,YS_TUNER_STATUS *status);
int ys_dvb_get_tuner_default_parameter(int tuner_type, void *param );
int ys_dvb_message_start(YS_MESSAGE_SETTINGS *ys_settings);
int ys_dvb_message_stop(int message_handle);
int ys_dvb_message_wait_dataready(int message_handle,int time_out_ms);
int ys_dvb_message_get_buffer(int message_handle,char **buffer, int *buffer_size);
int ys_dvb_message_release_buffer(int message_handle,int used_count);
int ys_dvb_play_start(int play_band,YX_MPEG_INFO *mpeg_info);
int ys_dvb_play_stop(void);
int ys_dvb_set_record_all(int mode);
int ys_dvb_get_play_parse_band(int tuner_handle,int tuner_type);
int ys_dvb_get_playband(void);
int ys_dvb_get_record_parse_band(int tuner_handle,int tuner_type);
yresult ys_dvb_pip_play_start(int play_band,YX_MPEG_INFO *mpeg_info);
yresult ys_dvb_pip_play_stop(void);
int ys_dvb_get_pip_parse_band_from_play_band(int play_band);
int ys_dvb_get_pip_playband(void);
int ys_dvb_play_createTeletext(void);
int ys_dvb_play_setpidTeletext(int pid);
int ys_dvb_play_startTeletext(void);
int ys_dvb_play_stopTeletext(void);


/* for 6936u 3 tuners */
int ys_dvbt_initband(void);
int ys_dvbt_getband(int index);
int ys_dvbt_lockfreq(int index,unsigned int freq, int band);
int ys_dvbt_getsignal(int index,int *quality);

yresult ygp_layer_recreate_framebuffer(void);
yresult ygp_layer_recreate_framebuffer_sd(void);

yresult ymm_decode_getDecodeStatus(YS_DECODER_STATUS *status);

yresult yhw_power_av_block_init(void);
yresult yhw_power_av_block_uninit(void);
int yhw_power_get_av_block_handle(void);
yresult yhw_power_set_av_block_state(int state);
yresult yhw_power_get_av_block_state(int *state);
yresult yhw_power_get_cpu_frequency(unsigned int *frequency);
yresult yhw_power_set_cpu_frequency(int scale_freq);
yresult yhw_power_get_ddr_refresh_time(unsigned int *timeout);
yresult yhw_power_set_ddr_refresh_time(unsigned int timeout);
yresult yhw_power_get_enet_state(int *state);
yresult yhw_power_set_enet_state(int state);
yresult yhw_power_get_sata_state(int *state);
yresult yhw_power_set_sata_state(int state);
yresult yhw_power_get_usb_state(int *state);
yresult yhw_power_set_usb_state(int state);
yresult yhw_power_get_tp1_state(int *state);
yresult yhw_power_set_tp1_state(int state);

int ys_changeto_brcm_audiocodec(YX_AUDIO_TYPE audio_type);
int ys_changeto_brcm_videocodec(YX_VIDEO_TYPE video_type);
int ys_set_audio_pid_change(void);
int ys_set_audio_output(void);
int ys_get_audio_codec_from_iso(int iso_code);
int ys_get_video_codec_from_iso(int iso_code);
int ymm_decoder_clearPreAudioPID(void);
int ymm_still_decoder_init(void);
int ymm_still_decoder_close(void);

yresult ymm_decoder_getDecodeStatus(YS_DECODER_STATUS *status);
yresult ymm_decoder_setDolbyDownmixingStatus(int status);

int ymm_asf_startUDPStreaming(char *ip_addr, int port);
int ymm_asf_stopUDPStreaming(int mode);
int ymm_asf_startTCPStreaming(int socked_fd);
int ymm_asf_stopTCPStreaming(int mode);
int ys_asf_set_drm(int mode);
int ymm_asf_setStreamSessionInformation(char *response_buf,int buf_len);
int ymm_asf_SetPacketFilterCallback(int(*packetFilter)(unsigned char *, int));

int ys_decoder_start(void);
int ys_decoder_stop(void);

int ys_get_iptv_status(void);
int ys_get_decode_status_pts(int *videopts,int *audiopts);

int ys_pcm_open( int *handle );
int ys_pcm_start(int sample_rate,int bits_per_sample,int channels);
int ys_pcm_get_status(int handle,int *bStart,int *noplay_bytes);
int ys_pcm_get_playbuffer(int handle, char **buffer , int *buffer_size);
int ys_pcm_write_data(int handle,char *buffer ,int buffer_len);
int ys_pcm_wait_callback(void);
int ys_pcm_stop(int handle);

int ys_getserver_wmv(void);
int AsfParseHeaderInfo(char *buf,int data_len);
int ys_get_startvideopts(void);
int ys_get_startaudiopts(void);
int Set_Decoder_Play_Over(int number);
int GetStreamVideoNum(void);
int ys_get_decoder_play_over(void);
int ys_stream_get_playstart(void);
int SetHttp_Live_Flag(int flag);
int ys_stream_set_playstate(int playstate);
int ys_stream_get_playstate(void);

yresult yhw_input_init(void);
yresult yhw_input_uninit(void);
void set_front_readkey(int readkey);
int updateSend(unsigned char *pCommand,unsigned command_len);
void sfpPowerLedOpen(int ledstatus);
void sfpPowerOff(void);
void sfpPowerReset(unsigned char delay);
int sfpGetVersion(unsigned char *psoftversion,unsigned char *phwdversion);
int sfpSetTime(unsigned char *address);
int sfpGetTime(unsigned char *fronthour,unsigned char *frontminute,unsigned char *frontsecond);


int ys_get_iptv_status();

int ys_make_dos_filesystem(int argc, char **argv);
int ys_get_formating_progress(void);
void ys_set_formating_progress(int Progress);

int ys_mms_set_water_mark(void);

void GBToUnicode(const char *gbStr,unsigned int *uniStr);
int UTF8ToUnicode(const char *utf8Str,unsigned int *uniStr);
int big5_mbtowc (unsigned int *pwc, unsigned char *s, int n);
int gb18030_mbtowc (int conv, unsigned int *pwc, const unsigned char *s, int n);

int DTTStart(void);
void DTTStop(void);
int PVROpen(void);
int PVRClose(void);

int ys_setrunlevel(int argc, char *runlevel[]);
int ys_setmemsize(int argc, char *memsize[]);
int ys_exit(int argc, char *memsize);

void yhw_env_serialVerify( void );

yresult ymm_dvb_pause(void);

int ys_management(void);

int ys_set_decoder_start(int DecodeStart);
int ys_get_decoder_start(void);
int ys_get_playpump_start(void);

yresult ymm_pip_get_decoder_start(int *b_start);
yresult ymm_pip_get_playpump_start(int *b_start);
int ys_set_audio_swap(void);

void ys_set_first_pts(void *data);
int ys_get_audio_output_status(YX_MPEG_INFO *mpeg_info, int *use_pcm, int *use_compressed);

int ys_set_real_time_check_pmt(int b_check);
int ys_get_real_time_check_pmt(void);

void yhw_board_systemEvent( SYSTEM_EVENT event );

int get_mtd_name( char *name, int type, char *mtd_name );

void ymm_demux_init(void);

int check_jpeg(FILE *fp);
int get_jpeg_info(char *path, int *w, int *h);
int load_jpeg(char *path, int w, int h, yx_pic_t **pp);
int check_png(FILE * fp);
int get_png_info(char *path, int *w, int *h);
int load_png(char *path, int w, int h, yx_pic_t **pp);
int check_gif(char *path);
int get_gif_info(char *path, int *w, int *h);
int load_gif(char *path, int w, int h, yx_pic_t **pp);
int check_bmp(FILE *fp);
int get_bmp_info(char *path, int *w, int *h);
int load_bmp(char *path, int w, int h, yx_pic_t **pp);

int get_pic_info(char *path, int *w, int *h);
int load_pic(char *path, int w, int h, yx_pic_t **pp);
void unload_pic(void);

int yhw_env_init(void);
int yu_init(void);
int ygp_font_freetypeUninit(void);

yresult ymm_still_decoder_init(void);
yresult ymm_still_decoder_close(void);

void set_fps_count(int sum);

extern YX_CA_TYPE yx_ca_type;
extern int ys_vmdrm_init_context(int context);
extern int ys_vmdrm_set_stream(int h, int pump, int vindex, int *aindex, int anum);
extern int ys_vmdrm_hw_set_ecm_buf(int h, char *buf, int len);
extern int ys_vmdrm_set_ecm_pid(int h, int pid);
extern int ys_vmdrm_reset_context(int h);
extern int ys_vmdrm_stream_stop(int h);
extern int ys_vmdrm_gethandle(void);

yx_pic_t* yx_get_mem_pic_p(void);

extern void yx_set_dl_ca_name(char *name);

yresult ymm_decoder_videoDecoderOpen(void);
yresult ymm_decoder_videoDecoderClose(void);
yresult ymm_stream_playerSetLoops(int index, int loops);
yresult ymm_decoder_audioDecoderOpen(void);
yresult ymm_decoder_audioDecoderClose(void);
int ys_set_audio_all_change(void);
yresult ymm_stream_playerSetDataPause(int index, int b_pause);
yresult ymm_stream_playerSeekToOffset(int index, off64_t seek_off);

int ymm_decoder_getCAType(void);
void ymm_deocder_SetCAType(int type);

extern int ys_vmdrm_reset_context(int h);
extern int ys_vmdrm_stream_stop(int h);
extern int ymm_vmdrm_setECMBuf(int h, char *buf, int len);

void parse_jpeg(const char *filename, void *callback_data);

#endif
