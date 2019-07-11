
#ifndef __CODEC_H__
#define __CODEC_H__

#include "ind_ts.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define AUDIO_VOLUME_MAX 100
#define AUDIO_VOLUME_DEFAULT 50

int mid_audio_mute_set(int mute);
int mid_audio_mute_get(void);

int mid_audio_volume_set(int vol);
int mid_audio_volume_get(void);

/*
	pnmode ��ʼ��ʽ���ò���ֻ���ں�˼ƽ̨��������
 */
int codec_init(int pnmode);
/*
	�������ʱ���ٲ�����
	�ù���ֻ��3560E����ʵ��
 */
void codec_mini(int pIndex, int mini);


void codec_default_language(int pIndex, char* language);
void codec_default_audio(int pIndex, char* language);
void codec_default_subtile(int pIndex, char* language);

void codec_alternative_set(int alternative);
int  codec_alternative_get(int* paudio_index, int* psubtitle_index);


/*
	I֡����ģʽ���ù���ֻ���ڶ�λ����ʹ��
 */
void codec_iframe(int pIndex, int iframe);

void codec_ignore(int pIndex, int ignore);

/*
	���ò���������������ڶ�����죬��PSI��Ϣ�����������Ա�ǣ�
	���ſ�ʼʱ��ѡ��Ĭ��������Ӧ������
 */
void codec_language(int pIndex, char* language);

/*
	�����͸��������������ýӿ�ֻ���ڶ�λ
 */
int codec_save(int pIndex, int enable);

int codec_emm(int pIndex, int flag);

/*
	Ϊ��ʼ���ţ���δ��������ʱ�������ö����ӵĺ���
 */
void codec_prepare(void);
/*
	type
	0��VOD
 */
void codec_open(int pIndex, int iptv);

int codec_volume_set(int vol);
int codec_volume_get(void);

void codec_buf_info(int *pSize, int *pLen);
int codec_buf_get(int pIndex, char **pbuf, int *plen);
int codec_buf_put(int pIndex, int len);

int codec_pts(int pIndex, unsigned int* ppts);
int codec_video_width(int pIndex);

//#define codec_reset codec_reset_v1
int codec_reset(int pIndex, int caReset);
int codec_pause(int pIndex);

int codec_psi(int pIndex, struct ts_psi *psi);

#define codec_ca_update codec_ca_update_v2
int codec_ca_update(int pIndex, ts_ca_t ca, char *pmt_buf, int pmt_len);
int codec_ca_cat(int pIndex, char *pmt_buf, int pmt_len);
int codec_ca_check(int pIndex);

int codec_tplay(int pIndex);
int codec_resume(int pIndex, int iptv);
int codec_close(int pIndex, int clear);

void codec_track(void);
void codec_lock(void);
void codec_unlock(void);

int codec_rect(int pIndex, int x, int y, int width, int height);

int codec_audio_track_set(int track);
int codec_audio_track_get(int *ptrack);
int codec_audio_track_num(int *pnum);
int codec_audio_track_get_pid(int track, int *pid);
int codec_audio_track_get_info(int track, char *info);
int codec_audio_track_get_type(int track, int *ptype);
/*
	subtitle
 */
int codec_subtitle_show_set(int flag);
int codec_subtitle_set(int subtitle);
int codec_subtitle_get(int *psubtitle);
int codec_subtitle_lang(int subtitle, char *language);
int codec_subtitle_num(int *pnum);
int codec_subtitle_show_get(int flag);
int codec_subtitle_pid(int subtitle,unsigned short* pid);

/*
	teletext
 */
int codec_teletext_set(int teletext);
int codec_teletext_get(int *pteletext);
int codec_teletext_lang(int teletext, char *language);
int codec_teletext_num(int *pnum);

int codec_teletext_page(int page);
int codec_teletext_subpage(int page);

/* ������̨ģʽ */
/* 0				��̨���� */
/* 1				��̨ʱ�������һ֡  */
/* 2				��̨ʱ��ֹͣ���Ų��������һ֡  */
/* 3				��̨ʱ�������ǹص�ͬ�� */
int codec_changemode(int mode);
int codec_get_changemode( void );

/*
	�����˲���
 */
int codec_mosaic_open(void);
int codec_mosaic_elem_open(ts_mosaic_t ts_mosaic);
void codec_mosaic_elem_close(int key);

int codec_mosaic_push(char *buf, int len);
int codec_mosaic_close(void);

int codec_mosaic_set(int key);
int codec_mosaic_get(void);

int codec_mosaic_decript(int add_index, char* buf, int len);

void codec_mosaic_test(void);

/*
	FLASH����
 */
int codec_flash_open(char* url);
int codec_flash_close(void);

/*
	PCM����
 */
int codec_pcm_open(int pIndex, int sampleRate, int bitWidth, int channels);
int codec_pcm_close(int pIndex);
int codec_pcm_push(int pIndex, char *buf, int len);



int codec_zebra_open(int pIndex, char* url, int arg);
int codec_zebra_close(int pIndex);

typedef struct __ZebraPCM {
	int size;

	char url[2048];
	int sampleRate;
	int bitWidth;
	int channels;
	int b_signed;
	int b_bigendian;
} ZebraPCM;

int codec_zebra_pcm_open(int pIndex, ZebraPCM* zpcm, int arg);
int codec_zebra_pcm_close(int pIndex);


struct _DvbCaParam {
unsigned int networkid;
unsigned int samefreq;
unsigned int prognum;
unsigned int pmtpid;
};

typedef struct _DvbCaParam	DvbCaParam;
typedef struct _DvbCaParam*	DvbCaParam_t;

/*
	DVBS
	v2
		pmtpid = 1 ʱ����ʾʱ����ֱ���л���prognum=1Ϊֱ����prognum=2��ʾʱ��
 */
#define codec_dvbs codec_dvbs_v2
int codec_dvbs(int pIndex, DvbCaParam_t param);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //__CODEC_H__
