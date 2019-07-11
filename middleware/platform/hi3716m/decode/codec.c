#include "codec.h"
#include "YX_codec_porting.h"

#include "mid/mid_time.h"

#include "AppSetting.h"
#include "SysSetting.h"
#include "SettingEnum.h"
#include "SettingListener.h"

#include "Assertions.h"
#include "config/pathConfig.h"

#include "independs/ind_ts.h"
#include "app/Assertions.h"
#include "independs/ind_mem.h"
#include "Verimatrix.h"

#include "libzebra.h"

#include "pthread.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static void codec_teletext_show(int teletext);

enum {
    CODEC_STATE_CLOSE = 0,
    CODEC_STATE_OPEN,
    CODEC_STATE_PLAY,
    CODEC_STATE_PAUSE,
    CODEC_STATE_TPLAY
};

enum {
    CODEC_CMD_OPEN = 1,
    CODEC_CMD_PSI,
    CODEC_CMD_CAT,
    CODEC_CMD_CA_OPEN,
    CODEC_CMD_CA_CLOSE,//5
    CODEC_CMD_PAUSE,
    CODEC_CMD_IPTV,
    CODEC_CMD_VOD,
    CODEC_CMD_RESET,
    CODEC_CMD_FLUSH,
    CODEC_CMD_TPLAY,//10
    CODEC_CMD_RECT,
    CODEC_CMD_CLOSE,
    CODEC_CMD_SMOOTH
};


#define SAVESTREAM
//#define DUMMYDECODE

#define    DUMMY_SIZE        (128*1024)

#define MOSAIC_NUM        9

struct StrmCodec {
    int    index;
    int    state;
    int    magic;
    int    show;
    int    open;

    int msgfd[2];

    pthread_mutex_t mutex;

    struct ts_psi   arg_psi;

    struct ts_ca    arg_ca;

    int        arg_x;
    int        arg_y;
    int        arg_w;
    int        arg_h;

    struct ts_psi   ts_psi;

    struct ts_ca    ts_ca;

    char            pmt_buf[TS_SECTION_SIZE];
    int             pmt_len;
    char            cat_buf[TS_SECTION_SIZE];
    int             cat_len;

    char*           decode_buf;
    int             decode_len;
    uint            decode_pts;
    uint            decode_buf_phyaddr;

    int     x;
    int     y;
    int     w;
    int     h;

    int     playing;
    int     iframe;
    int     video;

    int     pcm_handle;

#ifdef SAVESTREAM
    int     save_flg;//enable标记
    FILE*   save_fp;
    int     save_sn;
#endif

#ifdef DUMMYDECODE
    char*   dummy_buf;
    int     dummy_size;
#endif

    int     alternative;

    Codec_interface*     codec_interface;
};

struct StrmCodec0 {
    struct ts_dr_subtitle    arg_subtitle;
    struct ts_dr_teletext    arg_teletext;
    struct ts_dr_subtitle    ts_subtitle;
    struct ts_dr_teletext    ts_teletext;

    int         track;
    uint32_t    track_pid;

    int         teletext;
    int         teletext_default;

    int         subtitle;
    int         subtitle_show;
    int         subtitle_default;
};

enum {
    CODEC_MODE_NORMAL = 0,
    CODEC_MODE_MOSAIC,
    CODEC_MODE_FLASH,
    CODEC_MODE_ZEBRA,
    CODEC_MODE_ZEBRA_PCM,
    CODEC_MODE_CLOUD,
    CODEC_MODE_LOCKED /* broadcom平台新sdk待机时若是马赛克或PIP已初始化，会出现问题 */
};


static int g_msc_audio_key;
static int g_msc_audio_flag;
static ts_mosaic_t g_msc_array = NULL;

static int g_mode = CODEC_MODE_NORMAL;
static int g_state = CODEC_STATE_CLOSE;
static int g_sync = YX_DECODER_ERROR_NONE;
static int g_iptv_flag = 0;

static pthread_mutex_t    g_mutex = PTHREAD_MUTEX_INITIALIZER;//保护g_mode

static char g_default_audio[4] = {"chi"};
static char g_default_subtitle[4] = {"chi"};
static struct StrmCodec *g_codecs[2] = {NULL, NULL};
static struct StrmCodec0 *g_codec0 = NULL;
static struct Codec_interface *g_codecs_interface[2] = {NULL, NULL};

extern int ys_vmdrm_stream_setcontrolset(int h, int v);
extern int ys_vmdrm_stream_getcontrolset(int h);


int codec_flush_open(char* url);
int codec_flush_stop(void);

#define PTHREAD_MUTEX_BEGIN( )                                      \
    pthread_mutex_lock(&g_mutex);                                   \
    if (CODEC_MODE_LOCKED == g_mode || CODEC_MODE_FLASH == g_mode)  \
        ERR_OUT("++++ g_mode = %d\n", g_mode)

#define PTHREAD_MUTEX_NORMAL( )                                     \
    pthread_mutex_lock(&g_mutex);                                   \
    if (g_mode != CODEC_MODE_NORMAL || codec->pcm_handle)           \
        ERR_OUT("g_mode = %d, pcm_handle = %d!\n", g_mode, codec->pcm_handle)

#define PTHREAD_MUTEX_PCM( )                                         \
    struct StrmCodec *codec = g_codecs[pIndex];                       \
    pthread_mutex_lock(&g_mutex);                                    \
    if (g_mode != CODEC_MODE_NORMAL || codec->pcm_handle == 0)       \
        ERR_OUT("g_mode = %d, pcm_handle = %d!\n", g_mode, codec->pcm_handle)

#define PTHREAD_MUTEX_UNLOCK( )         \
    pthread_mutex_unlock(&g_mutex);     \
    return 0;                           \
Err:                                    \
    pthread_mutex_unlock(&g_mutex);     \
    return -1

#define PTHREAD_MUTEX_MOSAIC( )         \
    pthread_mutex_lock(&g_mutex);       \
    if (g_mode != CODEC_MODE_MOSAIC)    \
        ERR_OUT("normal mode!\n")

#define PTHREAD_MUTEX_ZEBRA( )          \
    pthread_mutex_lock(&g_mutex);       \
    if (g_mode != CODEC_MODE_ZEBRA)     \
        ERR_OUT("normal mode!\n")

#define PTHREAD_MUTEX_CLOUD( )          \
    pthread_mutex_lock(&g_mutex);       \
    if (g_mode != CODEC_MODE_CLOUD)     \
        ERR_OUT("normal mode!\n")

#define PTHREAD_MUTEX_ZEBRA_PCM( )      \
    pthread_mutex_lock(&g_mutex);       \
    if (g_mode != CODEC_MODE_ZEBRA_PCM) \
        ERR_OUT("normal mode!\n")

static pthread_t g_thread[2] = { -1, -1};

static void codec_on_open(struct StrmCodec* codec);
static void codec_on_psi(struct StrmCodec* codec);
static void codec_on_cat(struct StrmCodec* codec);
static void codec_on_ca_open(struct StrmCodec* codec);
static void codec_on_ca_close(struct StrmCodec* codec);
static void codec_on_pause(struct StrmCodec* codec);
static void codec_on_resume(struct StrmCodec* codec, int iptv);
static void codec_on_reset(struct StrmCodec* codec);
static void codec_on_flush(struct StrmCodec* codec);
static void codec_on_tplay(struct StrmCodec* codec);
static void codec_on_rect(struct StrmCodec* codec);
static void codec_on_close(struct StrmCodec* codec, int smooth);

static void codec_mosaic_on_open(struct StrmCodec* codec);
static void codec_mosaic_on_close(struct StrmCodec* codec);

static void int_on_close(struct StrmCodec* codec, int clear);
static void codec_on_close_pcm(struct StrmCodec *codec);
static void codec_on_close_mosaic(void);
static void codec_on_close_zebra(void);
static void codec_on_close_zebra_pcm(void);



static int g_mute = 0;
static int g_volume = 0;
static uint32_t g_volumeClk = 0;//静音定时

static void codec_setMute(int mute)
{
#ifdef ANDROID
    appSettingSetInt("mute", mute);
#else
    yhw_aout_setMute(mute);
#endif
}

int mid_audio_mute_set(int mute)
{
    if(mute) {
        yhw_aout_setMute(1);
        g_mute = 1;
    } else {
        if (g_volume > 0)
            yhw_aout_setMute(0);
        g_mute = 0;
    }
    g_volumeClk = 0;
    return 0;
}

int mid_audio_mute_get(void)
{
    return g_mute;
}

static int muteListenFunc(const char* name, const char* value)
{
    int mute = atoi(value);
    mid_audio_mute_set(mute);
    return 0;
}

int mid_audio_volume_set(int vol)
{
	if(vol < 0 || vol > AUDIO_VOLUME_MAX) {
		ERR_PRN("vol = %d\n", vol);
		return -1;
	}

	if(vol > 0) {
		yhw_aout_setMute(0);
		g_mute = 0;
	} else {
		yhw_aout_setMute(1);
		g_mute = 1;
	}

	yhw_aout_setVolume(vol);
	g_volume = vol;
	g_volumeClk = 0;
	return 0;
}

int mid_audio_volume_get(void)
{
	return g_volume;
}


static int codec_create(int idx, void* callback);
static void yhw_board_init_setting_callback(void *p);


int codec_init(int pnmode)
{
    if(g_msc_array)
        return -1;
    g_msc_array = (ts_mosaic_t)malloc(sizeof(struct ts_mosaic) * MOSAIC_NUM);

    g_codec0 = (struct StrmCodec0 *)calloc(sizeof(struct StrmCodec0), 1);

    yhw_board_setInitSettingsCallback(yhw_board_init_setting_callback);
    yhw_board_init();

#ifdef ANDROID
    int handle = IPTVMiddleware_GetSurfaceHandle();
    if (handle != 0) {
        ygp_layer_setDisplaySurface(handle);
    }
#endif

    LogUserOperDebug("Set hdcp mode to close\n"); // (%d)\n", ys_get_hdcp_failure_mode());
    yhw_vout_setHdcpMode(0); // 0 is close, 1 is open
    LogUserOperDebug("Set hdcp failure mode to YX_HDMI_TO_SD\n"); // (%d)\n", ys_get_hdcp_failure_mode());
    yhw_vout_setHdcpFailureMode(YX_HDMI_TO_SD);
    LogUserOperDebug("Set Macrovision mode to NONE\n");
    yhw_vout_setMacrovision(0); // 0 is NONE, 1 is Agc, 2 is Agc2Lines, 3 is Agc4Lines
    LogUserOperDebug("Hdcp key burn status(%d)\n", yhw_board_getHdcpKeyBurnStatus());

    codec_create(0, YX_SDK_codec_construct_master);
    // set decoder trick mode
    YX_TRICK_MODE_SETTING trick_mode_setting;

    trick_mode_setting.b_ffw_socket_no_flush = 0;
    trick_mode_setting.b_pause_no_flush = 1;
    //trick_mode_setting.ffw_decode_mode = 0;
    trick_mode_setting.ffw_decode_mode = YX_DECODE_MODE_I;
    trick_mode_setting.pause_decode_mode = 2;
    ymm_decoder_setTrickMode(&trick_mode_setting);

    codec_create(1, YX_SDK_codec_construct_pip);

    ymm_decoder_setErrorHandleMode(YX_DECODER_ERROR_NONE);
    {
        int vol = 0;
        appSettingGetInt("volume", &vol, 0);
        mid_audio_volume_set(vol);
    }

    SettingListenerRegist("mute", 0, muteListenFunc);
    return 0;
}






static void* codec_task(void *arg)
{
    uint cmd, magic;
    int fd, ret;
    fd_set rset;
    struct timeval tv;

    struct StrmCodec *codec = (struct StrmCodec *)arg;

    fd = codec->msgfd[0];
    for(;;) {
        tv.tv_sec = 3600;
        tv.tv_usec = 0;
        if (0 == codec->index && g_volumeClk) {
            uint32_t clk = mid_10ms( );
            if (g_volumeClk <= clk) {
                PRINTF("++++ mute = %d\n", g_mute);
                if (0 == g_mute)
                    codec_setMute(0);
                g_volumeClk = 0;
            } else {
                clk -= g_volumeClk;
                if (clk >= 100) {
                    tv.tv_sec = 1;
                } else {
                    tv.tv_sec = 0;
                    tv.tv_usec = clk * 10000;
                }
            }
        }

        FD_ZERO(&rset);
        FD_SET(fd, &rset);

        ret = select(fd + 1, &rset, NULL, NULL, &tv);
        if(g_mode == CODEC_MODE_NORMAL && codec->pcm_handle == 0) {
            pthread_mutex_lock(&codec->mutex);
            if(codec->magic == 0 && codec->state != CODEC_STATE_CLOSE && codec->state != CODEC_STATE_OPEN)
                codec->codec_interface->decoder_pts(&codec->decode_pts);
            pthread_mutex_unlock(&codec->mutex);
        }
        if(ret != 1)
            continue;

        ret = read(fd, &magic, 4);
        if(ret != 4) {
            ERR_PRN("read ret = %d\n", ret);
            continue;
        }

        cmd = magic >> 16;
        magic = magic & 0xffff;

        DBG_PRN("#%d, cmd = %d, magic = %d\n", codec->index, cmd, magic);

        switch(cmd) {
        case CODEC_CMD_OPEN:
            codec_on_open(codec);
            break;
        case CODEC_CMD_PSI:
            codec_on_psi(codec);
            break;
        case CODEC_CMD_CAT:
            codec_on_cat(codec);
            break;
        case CODEC_CMD_CA_OPEN:
            codec_on_ca_open(codec);
            break;
        case CODEC_CMD_CA_CLOSE:
            codec_on_ca_close(codec);
            break;
        case CODEC_CMD_PAUSE:
            codec_on_pause(codec);
            break;
        case CODEC_CMD_IPTV:
            codec_on_resume(codec, 1);
            break;
        case CODEC_CMD_VOD:
            codec_on_resume(codec, 0);
            break;
        case CODEC_CMD_RESET:
            codec_on_reset(codec);
            break;
        case CODEC_CMD_FLUSH:
            codec_on_flush(codec);
            break;
        case CODEC_CMD_TPLAY:
            codec_on_tplay(codec);
            break;
        case CODEC_CMD_RECT:
            codec_on_rect(codec);
            break;
        case CODEC_CMD_CLOSE:
            codec_on_close(codec, 0);
            break;
        case CODEC_CMD_SMOOTH:
            codec_on_close(codec, 1);
            break;
        default:
            ERR_PRN("cmd = %d\n", cmd);
            break;
        }
        pthread_mutex_lock(&codec->mutex);
        if(magic == (codec->magic & 0xffff))
            codec->magic = 0;
        pthread_mutex_unlock(&codec->mutex);
    }

    return NULL;
}

static void yhw_board_init_setting_callback(void *p)
{
    if(p == NULL)
        ERR_OUT("NULL");

    YX_SETTINGS *p_setting = (YX_SETTINGS *)p;
    int macrovision = 0;
    int teletext = 0;
	int hdmiNegotiation = 0;
	int aspectRatio = 0;
	int hdcpEnable = 1;
	char DolbyMode[2] = {0};
	int SPDIFAudioFormat = 0;
    int HDMIAudioFormat = 0;

    appSettingGetInt("macrovision", &macrovision, 0);
    appSettingGetInt("Teletext", &teletext, 0);
	appSettingGetInt("hdmi_negotiation", &hdmiNegotiation, 0);
	appSettingGetInt("hd_aspect_mode", &aspectRatio, 0);
	appSettingGetInt("HDCPEnableDefault", &hdcpEnable, 0);
	sysSettingGetInt("SPDIFAudioFormat", &SPDIFAudioFormat, 0);
    sysSettingGetInt("HDMIAudioFormat", &HDMIAudioFormat, 0);
    sysSettingGetString("DolbyMode", DolbyMode, 2, 0);


    p_setting->macrovision_mode = macrovision;
    p_setting->hd_aspect_ratio = 1; // 0 is 4:3, 1 is 16:9, 2 is auto
    p_setting->sd_aspect_ratio = 1;
    if(aspectRatio) {
        p_setting->hd_aspect_mode = 2; // 0 is letterbox, 1 is panscan, 2 is full, 3 is zoom, 4 is full noliner
        p_setting->sd_aspect_mode = 2;
    } else {
        p_setting->hd_aspect_mode = 0;
        p_setting->sd_aspect_mode = 0;
    }

    p_setting->hdcp_mode = hdcpEnable;
    p_setting->hdcp_failure_mode = 2;
    p_setting->hdmi_edid_format_valid = hdmiNegotiation;
    if(atoi(DolbyMode)) {
        p_setting->hdmi_audio_self_adaption_type = HDMI_AUDIO_AUTOSET_AUTO;
    } else {
        p_setting->hdmi_audio_self_adaption_type = HDMI_AUDIO_AUTOSET_MANUAL;
    }

    switch(SPDIFAudioFormat) {
    case PCM:
        p_setting->spdif_mode = YX_SPDIF_PCM;
        break;
    case OUT_CLOSE:
        p_setting->spdif_mode = YX_SPDIF_OFF;
        break;
    case PASS_THROUGH:
    default:
        p_setting->spdif_mode = YX_SPDIF_PASSTHOUGH;
        break;
    }

    switch(HDMIAudioFormat) {
    case PCM:
        p_setting->hdmi_audio_mode = YX_HDMI_AUDIO_PCM;
        break;
    case OUT_CLOSE:
        p_setting->hdmi_audio_mode = YX_HDMI_AUDIO_OFF;
        break;
    case PASS_THROUGH:
    default:
        p_setting->hdmi_audio_mode = YX_HDMI_AUDIO_PASSTHROUGH;
        break;
    }

    ymm_decoder_setTeletextMode(teletext);
    // PRINTF("macrovision_mode :%d\n",p_setting->macrovision_mode);
    PRINTF("hd_aspect_ratio :%d\n", p_setting->hd_aspect_ratio);
    PRINTF("hd_aspect_mode :%d\n", p_setting->hd_aspect_mode);
    // PRINTF("hdcp_mode :%d\n",p_setting->hdcp_mode);
    PRINTF("hdmi_edid_format_valid :%d\n", p_setting->hdmi_edid_format_valid);
Err:
    return;
}

static int codec_create(int idx, void* callback)
{
    struct StrmCodec *codec;
    struct Codec_interface *temp_interface;

    if(g_codecs[idx])
        ERR_OUT("already inited\n");

    codec = (struct StrmCodec *)IND_MALLOC(sizeof(struct StrmCodec));
    if(codec == NULL)
        ERR_OUT("malloc\n");
    memset(codec, 0, sizeof(struct StrmCodec));
    codec->index = idx;
    pthread_mutex_init(&codec->mutex, NULL);

    if (0 == idx) {
        codec->arg_psi.dr_subtitle = &g_codec0->arg_subtitle;
        codec->arg_psi.dr_teletext = &g_codec0->arg_teletext;

        codec->ts_psi.dr_subtitle = &g_codec0->ts_subtitle;
        codec->ts_psi.dr_teletext = &g_codec0->ts_teletext;
    }

    if(g_codecs_interface[idx])
        ERR_OUT("already inited interface\n");

    temp_interface = (struct Codec_interface *)IND_MALLOC(sizeof(Codec_interface));
    if(temp_interface == NULL)
        ERR_OUT("malloc\n");

    codec->codec_interface = (Codec_interface*)temp_interface;

    if(callback)
        YX_codec_create(codec->codec_interface, (void*)callback);

    g_codecs[idx] = codec;
    g_codecs_interface[idx] = temp_interface;

    pipe(codec->msgfd);
    pthread_create(&g_thread[idx], NULL, codec_task, codec);

#ifdef DUMMYDECODE
    if(0 == idx)
        codec->dummy_size = DUMMY_SIZE;
    else
        codec->dummy_size = DUMMY_SIZE / 4;
    codec->dummy_buf = (char *)IND_MALLOC(codec->dummy_size);
#endif//DUMMYDECODE

    return 0;
Err:
    return -1;
}

void codec_iframe(int pIndex, int iframe)
{
    PRINTF("++++ iframe = %d\n", iframe);

    struct StrmCodec *codec = g_codecs[0];
    pthread_mutex_lock(&g_mutex);
    codec->iframe = iframe;
    pthread_mutex_unlock(&g_mutex);
}

void codec_default_language(int pIndex, char* language)
{
}

void codec_default_audio(int pIndex, char* language)
{
    memcpy(g_default_audio, language, 3);
}

void codec_default_subtile(int pIndex, char* language)
{
    memcpy(g_default_subtitle, language, 3);
}

static int g_changemode = 1;

/* 设置切台模式 */
/* 0 切台黑屏 */
/* 1 切台时保留最后一帧 */
/* 2 切台时不停止播放并保留最后一帧 */
/* 3 切台时黑屏但是关掉同步 */
int codec_changemode(int mode)
{
    extern int ymm_decoder_setStartMode(int mode); // hisi mode取值0-50，设置预同步时间1-500ms。大于50,开启慢同步，预同步时间都为2秒。startmode传的值小于等于0，就是预同步2000ms，关闭慢同步

    PRINTF("Change mode(%d)\n", mode);
    if(mode < 0 || mode > 3)
        return -1;
    //之前同步有问题，请更新sdk。Before the AV synchronization has a problem, please update the SDK.
    ymm_decoder_setStartModeType(1); // 是否用户可自定义设置(Whether the user can customize Setting)，default 0 is use sdk setting。
    ymm_decoder_setStartMode(0); // 设置预同步时间(Set the synchronization time,0 is close, 1—100 is 100ms—10000ms)
    ymm_decoder_setStartModeEffect(1, 1); // 是否打开慢同步模式和快速输出(Whether to open slow AV sync mode and the output quickly)。
    g_changemode = mode;
    return 0;
}

static void codec_on_clear_normal(struct StrmCodec* codec, int clear)
{
    PRINTF("Index(%d) state(%d) playing(%d) video(%d) show(%d) clear(%d)\n", codec->index, codec->state, codec->playing, codec->video, codec->show, clear);

    if(codec->state != CODEC_STATE_CLOSE && codec->state != CODEC_STATE_OPEN) {
        if(codec->pcm_handle)
            codec_on_close_pcm(codec);
        else
            int_on_close(codec, 1);

    } else if(codec->playing || codec->video) {
        if(codec->playing == 0) {
            YX_MPEG_INFO mpeg_info;

            memset(&mpeg_info, 0, sizeof(mpeg_info));
            mpeg_info.stream_type = YX_STREAM_TS;
            mpeg_info.video_num = 1;
            mpeg_info.video_pid[0] = 100;
            mpeg_info.video_type[0] = YX_VIDEO_TYPE_H264;
            codec->codec_interface->decoder_start(&mpeg_info);
        }
        codec->codec_interface->decoder_stopMode(0);
        codec->codec_interface->decoder_stop();
        codec->playing = 0;
        codec->video = 0;
        codec->state = CODEC_STATE_CLOSE;
    } else if(codec->state == CODEC_STATE_OPEN) {
        codec->state = CODEC_STATE_CLOSE;
    }

#ifdef INCLUDE_PIP
    if(1 == codec->index && 1 == codec->show && 1 == clear) {
        ymm_pip_set_show(0);
        PRINTF("decoder hide\n");
        codec->show = 0;
    }
#endif
}

static void codec_on_close_all(void)
{
    pthread_mutex_lock(&g_codecs[0]->mutex);
    pthread_mutex_lock(&g_codecs[1]->mutex);

    switch(g_mode) {
    case CODEC_MODE_NORMAL:
        codec_on_clear_normal(g_codecs[0], 1);
        codec_on_clear_normal(g_codecs[1], 1);
        break;
    case CODEC_MODE_MOSAIC:
        codec_on_close_mosaic();

        PRINTF("SDK: ymm_mosaic_close\n");
        ymm_mosaic_close();

        PRINTF("SDK: ymm_pip_init\n");
#ifdef INCLUDE_PIP
        ymm_pip_init();
        ymm_pip_set_show(0);
#endif
        g_codecs[1]->show = 0;
        break;
    case CODEC_MODE_ZEBRA:
        codec_on_close_zebra();
        break;
    case CODEC_MODE_ZEBRA_PCM:
        codec_on_close_zebra_pcm();
        break;
    case CODEC_MODE_CLOUD:
        ymm_decoder_setStopMode(0);
        ymm_decoder_stopBuffer( );
        break;
    default:
        WARN_PRN("mode = %d\n", g_mode);
        break;
    }

    g_mode = CODEC_MODE_NORMAL;

    pthread_mutex_unlock(&g_codecs[1]->mutex);
    pthread_mutex_unlock(&g_codecs[0]->mutex);
}

void codec_lock(void)
{
    PRINTF("State(%d , %d)\n", g_codecs[0]->state, g_codecs[1]->state);

    pthread_mutex_lock(&g_mutex);

    if(g_mode == CODEC_MODE_FLASH) {
        WARN_PRN("CODEC_MODE_FLASH\n");
    } else if(g_mode != CODEC_MODE_LOCKED) {
        codec_on_close_all();
        PRINTF("SDK: ymm_pip_uninit\n");
#ifdef INCLUDE_PIP
        ymm_pip_uninit();
#endif
        g_mode = CODEC_MODE_LOCKED;
    }

    pthread_mutex_unlock(&g_mutex);
}

void codec_unlock(void)
{
    pthread_mutex_lock(&g_mutex);
    PRINTF("g_mode(%d)\n", g_mode);

    if(g_mode == CODEC_MODE_FLASH) {
        WARN_PRN("CODEC_MODE_FLASH\n");
    } else {
        pthread_mutex_lock(&g_codecs[0]->mutex);
        pthread_mutex_lock(&g_codecs[1]->mutex);

        if(g_mode == CODEC_MODE_LOCKED) {
            PRINTF("SDK: ymm_pip_init\n");
#ifdef INCLUDE_PIP
            ymm_pip_init();
            ymm_pip_set_show(0);
#endif
            g_codecs[1]->show = 0;
            g_mode = CODEC_MODE_NORMAL;
        } else {
            WARN_PRN("g_mode(%d)\n", g_mode);
        }

        pthread_mutex_unlock(&g_codecs[1]->mutex);
        pthread_mutex_unlock(&g_codecs[0]->mutex);
    }
    pthread_mutex_unlock(&g_mutex);
}

void codec_buf_info(int *pSize, int *pLen)
{
	struct StrmCodec *codec = g_codecs[0];

    *pSize = 0;
    *pLen = 0;
	pthread_mutex_lock(&codec->mutex);
	if (g_mode != CODEC_MODE_NORMAL || codec->state <= CODEC_STATE_OPEN)
		goto Err;

	if(codec->ts_psi.video_pid) {
		YX_VIDEO_DECODER_STATUS status;
		status.fifo_size = 0;
		status.fifo_depth = 0;
		ymm_decoder_getVideoStatus(&status);
        *pSize = status.fifo_size;
        *pLen = status.fifo_depth;
	} else {
		YX_AUDIO_DECODER_STATUS status;
		status.fifo_size = 0;
		status.fifo_depth = 0;
		ymm_decoder_getAudioStatus(&status);
        *pSize = status.fifo_size;
        *pLen = status.fifo_depth;
	}

Err:
	pthread_mutex_unlock(&codec->mutex);
}

int codec_buf_get(int pIndex, char **pbuf, int *plen)
{
    struct StrmCodec *codec = g_codecs[pIndex];

    pthread_mutex_lock(&codec->mutex);

#ifdef DUMMYDECODE
    *pbuf = codec->dummy_buf;
    *plen = codec->dummy_size;
#else
    *pbuf = NULL;
    *plen = 0;
    if(g_mode != CODEC_MODE_NORMAL || codec->magic || codec->state <= CODEC_STATE_OPEN)
        goto Err;

    codec->decode_buf = NULL;

    if(codec->codec_interface->decoder_getbuffer(&codec->decode_buf, &codec->decode_len, &codec->decode_buf_phyaddr))
        ERR_OUT("decoder_getbuffer\n");

    *pbuf = codec->decode_buf;
    *plen = codec->decode_len;

Err:
#endif//DUMMYDECODE
    pthread_mutex_unlock(&codec->mutex);
    return 0;
}

#ifdef MQM_PACKETS_MODE
extern int mqm_packets_feeding(int packet_type, void *data, int len);
#endif

int codec_buf_put(int pIndex, int len)
{
    struct StrmCodec *codec = g_codecs[pIndex];

	int wRet = 0;

    pthread_mutex_lock(&codec->mutex);

#ifdef DUMMYDECODE
#else
    if(codec->magic || codec->state <= CODEC_STATE_OPEN)
        ERR_OUT("#%d magic = %u, state = %d\n", pIndex, codec->magic, codec->state);

    if(codec->decode_buf == NULL || len > codec->decode_len)
        ERR_OUT("#%d decode_buf = %p, len = %d / %d\n", pIndex, codec->decode_buf, len, codec->decode_len);

#ifdef MQM_PACKETS_MODE
    if(codec->state == CODEC_STATE_PLAY && pIndex != 2)
        mqm_packets_feeding(1, codec->decode_buf, len);
#endif

    if(codec->ts_ca.system_id == 0x5601 && codec->ts_ca.pid) {
        wRet = CA_VM_DECRYPT_STREAM(pIndex, codec->decode_buf, len, codec->arg_ca.pid, codec->decode_buf_phyaddr);
		if(wRet != 0)
			ERR_OUT("decrypt_buffer_error\n");
        /*
                FILE *fp = NULL;
                int len = 0;
                fp = fopen("/mnt/usb1/ca_data.txt","ab+");
                if( NULL == fp ){
                    printf("\n_________fopen error____\n");
                } else {
                    len = fwrite(codec->decode_buf,1,strlen(codec->decode_buf),fp);
                    printf("\n_____fwrite : len = %d___\n",len);
                }
                if( NULL != fp )
                    fclose(fp);
        */
    }

    if(codec->codec_interface->decoder_putbuffer(codec->decode_buf, len))
        ERR_OUT("decoder_putbuffer\n");

#ifdef SAVESTREAM
    if(codec->save_fp)
        fwrite(codec->decode_buf, 1, len, codec->save_fp);
#endif

Err:
    codec->decode_buf = NULL;
#endif//DUMMYDECODE
    pthread_mutex_unlock(&codec->mutex);
    return 0;
}

#ifdef SAVESTREAM
static void save_close(struct StrmCodec *codec)
{
    if(codec->save_fp) {
        fclose(codec->save_fp);
        PRINTF("save_fp = %p\n", codec->save_fp);
        codec->save_fp = NULL;
        sync();
    }
}

static void save_open(struct StrmCodec *codec)
{
    char filename[64];
    int len;
    save_close(codec);

    codec->save_sn ++;
    len = sprintf(filename, DEFAULT_EXTERNAL_DATAPATH);
    if(codec->index == 1)
        len += sprintf(filename + len, "pip_");

    if(codec->state == CODEC_STATE_TPLAY)
        len += sprintf(filename + len, "t");

    len += sprintf(filename + len, "play_%d_%d.ts", codec->index, codec->save_sn);

    codec->save_fp = fopen(filename, "wb");
    PRINTF("filename = %s, save_fp = %p\n", filename, codec->save_fp);
}

int codec_save(int pIndex, int enable)
{
    struct StrmCodec *codec = g_codecs[pIndex];

    PRINTF("Index %d state = %d, enable = %d / %d\n", pIndex, codec->state, enable, codec->save_flg);
    if(enable != 0)
        enable = 1;

    pthread_mutex_lock(&codec->mutex);

    if(codec->save_flg == enable)
        goto End;

    if(codec->save_flg) {
        save_close(codec);
    } else {
        if(codec->state == CODEC_STATE_PLAY || codec->state == CODEC_STATE_TPLAY)
            save_open(codec);
    }
    codec->save_flg = enable;

End:
    pthread_mutex_unlock(&codec->mutex);

    return 0;
}
#endif

static void codec_cmd(struct StrmCodec *codec, int cmd)
{
    int magic;

    if(CODEC_MODE_LOCKED == g_mode || CODEC_MODE_FLASH == g_mode) {
        ERR_PRN("g_mode = %d\n", g_mode);
        return;
    }

    codec->magic ++;

    magic = (cmd << 16) | (codec->magic & 0xffff);
    write(codec->msgfd[1], &magic, 4);
}

int codec_ca_cat(int pIndex, char *cat_buf, int cat_len)
{
    return 0;
}

int codec_ca_update(int pIndex, ts_ca_t ca, char *pmt_buf, int pmt_len)
{
    struct StrmCodec *codec = g_codecs[pIndex];

    PRINTF("Index(%d) cstate(%d)\n", pIndex, codec->state);

    pthread_mutex_lock(&codec->mutex);
    if(ca && ca->system_id > 0) {
        codec->arg_ca.system_id = ca->system_id;
        codec->arg_ca.pid = ca->pid;

        if(pmt_buf && pmt_len > 0) {
            memcpy(codec->pmt_buf, pmt_buf, pmt_len);
            codec->pmt_len = pmt_len;
        } else {
            codec->pmt_len = 0;
        }
        codec_cmd(codec, CODEC_CMD_CA_OPEN);
    } else {
        codec_cmd(codec, CODEC_CMD_CA_CLOSE);
    }
    pthread_mutex_unlock(&codec->mutex);

    return 0;
}
/*
int codec_ca_update(int pIndex, unsigned int system_id, char *pmt_buf, int pmt_len)
{
    struct StrmCodec *codec = g_codecs[pIndex];

    PRINTF("++++ %d cstate = %d\n", pIndex, codec->state);

    pthread_mutex_lock(&codec->mutex);
    if (system_id > 0) {
        codec->arg_system_id = system_id;
        if (pmt_buf && pmt_len > 0) {
            memcpy(codec->pmt_buf, pmt_buf, pmt_len);
            codec->pmt_len = pmt_len;
        } else {
            codec->pmt_len = 0;
        }
        codec_cmd(codec, CODEC_CMD_CA_OPEN);
    } else {
        codec_cmd(codec, CODEC_CMD_CA_CLOSE);
    }
    pthread_mutex_unlock(&codec->mutex);

    return 0;
}
*/

static void int_on_ca_close(struct StrmCodec* codec);
/*
    Irdeto CA 的需求，PMT发生变化时通知下层
 */
static void int_on_ca_open(struct StrmCodec* codec)
{
    int tIndex = codec->index;
    //unsigned int system_id = codec->arg_system_id;

    unsigned int system_id = codec->arg_ca.system_id;
    unsigned int ecmpid = codec->arg_ca.pid;
    PRINTF("%d system_id = %04x ecmpid = %04x cat_len = %d\n", tIndex, system_id, ecmpid, codec->cat_len);

    if(system_id != 0x0604)
        int_on_ca_close(codec);

    if(system_id == 0x5601/* VERIMATRIX */) {
        CA_VM_RESET_STREAM(tIndex);
    } else {
        PRINTF("%d: Unknown!\n", tIndex);
    }
    codec->ts_ca.system_id = system_id;
    codec->ts_ca.pid = ecmpid;
}

static void int_on_ca_close(struct StrmCodec* codec)
{
    int tIndex = codec->index;
    unsigned int system_id = codec->ts_ca.system_id;

    if(system_id == 0)
        return;

    PRINTF("%d system_id = %04x\n", tIndex, system_id);

    if(system_id == 0x5601/* VERIMATRIX */) {

    } else {
        PRINTF("#%d: Unknown!\n", tIndex);
    }
    codec->ts_ca.system_id = 0;
}

static void codec_on_cat(struct StrmCodec* codec)
{
}

static void codec_on_ca_open(struct StrmCodec* codec)
{
    pthread_mutex_lock(&g_mutex);
    if(g_mode != CODEC_MODE_NORMAL || codec->pcm_handle || codec->state == CODEC_STATE_CLOSE || codec->state == CODEC_STATE_OPEN)
        ERR_OUT("g_mode = %d, state = %d\n", g_mode, codec->state);
    int_on_ca_open(codec);
Err:
    pthread_mutex_unlock(&g_mutex);
}

static void codec_on_ca_close(struct StrmCodec* codec)
{
    pthread_mutex_lock(&g_mutex);
    if(g_mode == CODEC_MODE_NORMAL)
        int_on_ca_close(codec);
    pthread_mutex_unlock(&g_mutex);
}

/* Irdeto CA 的需求，检测CA是否成功 */
int codec_ca_check(int idx)
{
    int ret = 0;
    struct StrmCodec *codec = g_codecs[idx];

    pthread_mutex_lock(&codec->mutex);

    if(g_mode != CODEC_MODE_NORMAL || codec->magic)
        goto Err;

    if(codec->ts_ca.system_id == 0x5601/* VERIMATRIX */) {
        CA_VM_CHECK_KEYISREADY(idx, &ret);
    } else {
        ret = 1;
    }
Err:
    pthread_mutex_unlock(&codec->mutex);
    return ret;
}

int codec_emm(int pIndex, int flag)
{
    return 0;
}

/* 由mid_stream_open直接调用，用于同步设置，入音轨 */
void codec_prepare(void)
{
    g_codec0->track_pid = 0;
    g_codec0->teletext_default = -1;
   // if(0 != g_codec0->subtitle_show && 1 != g_codec0->subtitle_show)
   //     g_codec0->subtitle_show = 0;
    g_codec0->subtitle_show = 1;
    g_codec0->subtitle_default = -1;
}

void codec_open(int idx, int iptv)
{
    struct StrmCodec *codec = g_codecs[idx];

    PRINTF("Index %d state = %d, iptv = %d\n", idx, codec->state, iptv);

    if (0 == idx)
        g_iptv_flag = iptv;

#ifdef INCLUDE_PIP
#else
    if (0 != idx)
        return;
#endif
    codec->open = 1;
    codec->alternative = 0;

    pthread_mutex_lock(&codec->mutex);
    codec->decode_pts = 0;
    codec->decode_buf = NULL;
    codec_cmd(codec, CODEC_CMD_OPEN);
    pthread_mutex_unlock(&codec->mutex);
}

void codec_alternative_set(int alternative)
{
    g_codecs[0]->alternative = alternative;
}

int codec_alternative_get(int* paudio_index, int* psubtitle_index)
{
    struct ts_psi *psi;
    struct StrmCodec *codec = g_codecs[0];

    *paudio_index = -1;
    *psubtitle_index = -1;

    PTHREAD_MUTEX_NORMAL( );

    if (codec->state == CODEC_STATE_CLOSE || codec->state == CODEC_STATE_OPEN)
        goto Err;

    psi = &codec->ts_psi;

    if (psi->audio_num > 0)
        *paudio_index = g_codec0->track;

    if (psi->dr_subtitle && psi->dr_subtitle->subtitle_num > 0)
        *psubtitle_index = g_codec0->subtitle;

    PTHREAD_MUTEX_UNLOCK( );
}

int codec_dvbs(int pIndex, DvbCaParam_t param)
{
    return 0;
}

int codec_psi(int pIndex, struct ts_psi* psi)
{
    struct StrmCodec *codec = g_codecs[pIndex];

    PRINTF("Index %d cstate = %d, changemode = %d, sync = %d\n", pIndex, codec->state, g_changemode, sync);

    pthread_mutex_lock(&codec->mutex);
    ts_psi_copy(&codec->arg_psi, psi);
    codec_cmd(codec, CODEC_CMD_PSI);
    pthread_mutex_unlock(&codec->mutex);

    return 0;
}

int codec_pts(int idx, uint *ppts)
{
    struct StrmCodec *codec = g_codecs[idx];

    pthread_mutex_lock(&codec->mutex);
    if(codec->magic == 0 && codec->state != CODEC_STATE_CLOSE && codec->state != CODEC_STATE_OPEN) {
        uint32_t pts = 0;
        codec->codec_interface->decoder_pts(&pts);
        if (pts) {
            if (0 == idx && g_volumeClk) {
                PRINTF("++++ mute = %d\n", g_mute);
                if (0 == g_mute)
                    codec_setMute(0);
                g_volumeClk = 0;
            }
            codec->decode_pts = pts;
        }
    }
    *ppts = codec->decode_pts;
    pthread_mutex_unlock(&codec->mutex);

    return 0;
}

int codec_video_width(int pIndex)
{
    int width = 0;
    struct StrmCodec *codec = g_codecs[pIndex];

    pthread_mutex_lock(&codec->mutex);
    if(g_mode == CODEC_MODE_NORMAL && codec->magic == 0 && codec->state != CODEC_STATE_CLOSE && codec->state != CODEC_STATE_OPEN) {
        YX_VIDEO_DECODER_STATUS status;

        status.source_width = 0;
        if(pIndex == 1) {
#ifdef INCLUDE_PIP
            ymm_pip_getVideoStatus(&status);
#endif
        } else {
            ymm_decoder_getVideoStatus(&status);
        }
        width = status.source_width;
    }
    pthread_mutex_unlock(&codec->mutex);

    return width;
}

int codec_pause(int idx)
{
    struct StrmCodec *codec;

    if(idx)
        return -1;
    codec = g_codecs[0];

    PRINTF("state(%d)\n", codec->state);

    pthread_mutex_lock(&codec->mutex);
    codec_cmd(codec, CODEC_CMD_PAUSE);
    pthread_mutex_unlock(&codec->mutex);
    return 0;
}

int codec_resume(int idx, int iptv)
{
    struct StrmCodec *codec;

    if(idx)
        return -1;
    codec = g_codecs[0];

    PRINTF("Index(%d) state(%d) iptv(%d)\n", idx, codec->state, iptv);

    pthread_mutex_lock(&codec->mutex);
    if (iptv)
        codec_cmd(codec, CODEC_CMD_IPTV);
    else
        codec_cmd(codec, CODEC_CMD_VOD);
    pthread_mutex_unlock(&codec->mutex);

    return 0;
}

int codec_reset(int idx, int caReset)
{
    struct StrmCodec *codec;

    if(idx)
        return -1;
    codec = g_codecs[0];
    PRINTF("State(%d)\n", codec->state);

    pthread_mutex_lock(&codec->mutex);
    codec->decode_pts = 0;
    codec->decode_buf = NULL;
    if(caReset)
        codec_cmd(codec, CODEC_CMD_RESET);
    else
        codec_cmd(codec, CODEC_CMD_FLUSH);
    pthread_mutex_unlock(&codec->mutex);

    return 0;
}

int codec_tplay(int idx)
{
    struct StrmCodec *codec;

    if(idx)
        return -1;
    codec = g_codecs[0];
    PRINTF("State(%d)\n", codec->state);

    pthread_mutex_lock(&codec->mutex);
    codec_cmd(codec, CODEC_CMD_TPLAY);
    pthread_mutex_unlock(&codec->mutex);

    return 0;
}

int codec_rect(int pIndex, int x, int y, int w, int h)
{
    struct StrmCodec *codec = g_codecs[pIndex];

    PRINTF("Index(%d) state(%d)\n", pIndex, codec->state);

    pthread_mutex_lock(&codec->mutex);
    codec->arg_x = x;
    codec->arg_y = y;
    codec->arg_w = w;
    codec->arg_h = h;
    codec_cmd(codec, CODEC_CMD_RECT);
    pthread_mutex_unlock(&codec->mutex);

    return 0;
}

int codec_close(int pIndex, int clear)
{
    struct StrmCodec *codec = g_codecs[pIndex];

    PRINTF("Index(%d) state(%d) clear(%d)\n", pIndex, codec->state, clear);

    codec->open = 0;

    if(clear > 0) {
        pthread_mutex_lock(&g_mutex);
        if(g_mode == CODEC_MODE_NORMAL) {
            pthread_mutex_lock(&codec->mutex);
            codec_on_clear_normal(codec, clear);
            pthread_mutex_unlock(&codec->mutex);
        } else if(0 == pIndex && CODEC_MODE_MOSAIC == g_mode) {
            codec_on_close_all();
        }
        pthread_mutex_unlock(&g_mutex);
    } else {
        pthread_mutex_lock(&codec->mutex);
        if (clear < 0)
            codec_cmd(codec, CODEC_CMD_SMOOTH);
        else
            codec_cmd(codec, CODEC_CMD_CLOSE);
        pthread_mutex_unlock(&codec->mutex);
    }

    return 0;
}

static void int_on_open(struct StrmCodec* codec)
{
    if(g_mode == CODEC_MODE_NORMAL) {
        if(codec->pcm_handle)
            codec_on_close_pcm(codec);
        else
            int_on_close(codec, 0);
    } else {
        codec_on_close_all();
    }

    if(codec->open == 0)
        ERR_OUT("Index(%d) open is zero\n", codec->index);

#ifdef INCLUDE_PIP
    if(codec->index == 1 && codec->show == 0) {
        ymm_pip_set_show(1);
        PRINTF("PIP decoder show\n");
        codec->show = 1;
    }
#endif

    if(codec->playing) {
        codec->codec_interface->decoder_stopMode(1);
        codec->codec_interface->decoder_stop();
        codec->playing = 0;
    }

    if(0 == codec->index) {
        g_codec0->track = 0;
        g_codec0->subtitle = -1;
        g_codec0->teletext = -1;
    }

    codec->decode_buf = NULL;

    codec->ts_ca.system_id = 0;

    DBG_PRN("Index(%d) CODEC_STATE_OPEN\n", codec->index);
    codec->state = CODEC_STATE_OPEN;
Err:
    return;
}

static void codec_on_open(struct StrmCodec* codec)
{
    PRINTF("Index(%d) changemode(%d) state(%d)\n", codec->index, g_changemode, codec->state);

    PTHREAD_MUTEX_BEGIN();

    int_on_open(codec);

    if (0 == codec->index) {
        if (g_iptv_flag)
            ymm_decoder_setAvsyncMode(YX_AVSYNC_MODE_PCR);
        else
            ymm_decoder_setAvsyncMode(YX_AVSYNC_MODE_AUDIO);
    }
Err:
    pthread_mutex_unlock(&g_mutex);
}

#ifndef DUMMYDECODE
static void parse_psi(struct StrmCodec* codec, YX_MPEG_INFO* mpeg)
{
    int i;
	struct ts_psi* psi = &codec->ts_psi;

    /* 0表示TS流，暂时不支持其他流 */
    mpeg->stream_type = YX_STREAM_TS;
    if(psi->video_pid) {
        PRINTF("video: pid = %d, type = %d\n", psi->video_pid, psi->video_type);
        mpeg->video_num = 1;
        mpeg->video_pid[0] = psi->video_pid;
        mpeg->video_type[0] = ys_get_video_codec_from_iso(psi->video_type);
    }
    mpeg->audio_num = psi->audio_num;

    for(i = 0; i < mpeg->audio_num; i++) {
        PRINTF("audio: pid = %d, type = %d, lang = %s\n", psi->audio_pid[i], psi->audio_type[i], psi->audio_iso693[i].language);
        mpeg->audio_pid[i] = psi->audio_pid[i];
        if(ISO_EXT_DTS_AUDIO == psi->audio_type[i])
            mpeg->audio_type[i] = YX_AUDIO_TYPE_DTS;
        else if(ISO_IEC_AC3_AUDIO == psi->audio_type[i])
            mpeg->audio_type[i] = YX_AUDIO_TYPE_AC3;
        else if(ISO_EXT_AC3_AUDIO == psi->audio_type[i])
            mpeg->audio_type[i] = YX_AUDIO_TYPE_AC3PLUS;
        else
            mpeg->audio_type[i] = ys_get_audio_codec_from_iso(psi->audio_type[i]);

        memcpy(mpeg->audio_language[i], psi->audio_iso693[i].language, 3);
        mpeg->audio_language[i][3] = psi->audio_iso693[i].type;
    }

    if(psi->dr_subtitle) {
        struct ts_dr_subtitle *dr_subtitle = psi->dr_subtitle;

        mpeg->subtitle_num = dr_subtitle->subtitle_num;
        for(i = 0; i < mpeg->subtitle_num; i++) {
            mpeg->subtitle_pid[i] = dr_subtitle->subtitle_pid[i];
            memcpy(mpeg->subtitle_language[i], dr_subtitle->subtitle[i].language, 3);
            mpeg->subtitle_type[i] = dr_subtitle->subtitle[i].type;
            mpeg->subtitle_composition_page_id[i] = dr_subtitle->subtitle[i].composition;
            mpeg->subtitle_ancillary_page_id[i] = dr_subtitle->subtitle[i].ancillary;
            PRINTF("subtitle_pid = %d\n", mpeg->subtitle_pid[i]);
            PRINTF("subtitle_language = %c%c%c\n", mpeg->subtitle_language[i][0], mpeg->subtitle_language[i][1], mpeg->subtitle_language[i][2]);
        }
    }

    if(psi->dr_teletext) {
        struct ts_dr_teletext *dr_teletext = psi->dr_teletext;
        mpeg->teletext_pid = dr_teletext->pid;
        mpeg->teletext_page_num = dr_teletext->page_num;
        PRINTF("teletext_pid = %d, teletext_page_num = %d\n", mpeg->teletext_pid, mpeg->teletext_page_num);
        for(i = 0; i < mpeg->teletext_page_num; i++) {
            memcpy(mpeg->teletext_page_language[i], dr_teletext->page[i].language, 3);
            mpeg->teletext_page_type[i] = dr_teletext->page[i].type;
            mpeg->teletext_page_magazine_number[i] = dr_teletext->page[i].magazine;
            mpeg->teletext_page_number[i] = dr_teletext->page[i].page;
            PRINTF("teletext_page_language = %c%c%c\n", mpeg->teletext_page_language[i][0], mpeg->teletext_page_language[i][1], mpeg->teletext_page_language[i][2]);
        }
    }

    if(psi->ecm_num > 0) {
        struct ts_ca *ca = &psi->ecm_array[0];

        if(ca->system_id == 0x5601) { /*Verimatrix CA*/
            mpeg->ECMPID = ca->pid;
            mpeg->i_ca_sys_id = ca->system_id;
        }
    }

    mpeg->pcr_pid = psi->pcr_pid;

    if(psi->video_pid == 0)
        psi->video_type = 0;

    if (0 == codec->index) {
        int track = 0;
        if(psi->audio_num == 0) {
            psi->audio_pid[0] = 0;
            psi->audio_type[0] = 0;
        } else {
            track = -1;

            if (g_codec0->track_pid) {
                for (i = 0; i < psi->audio_num; i ++) {
                    if (g_codec0->track_pid == psi->audio_pid[i])
                        break;
                }
                if (i < psi->audio_num)
                    track = i;
            }

    		if (-1 == track) {
                for(i = 0; i < psi->audio_num; i ++) {
                    if(memcmp(psi->audio_iso693[i].language, g_default_audio, 3) == 0) {
                        track = i;
                        break;
                    }
                }
                if (-1 == track)
                    track = 0;
            }
        }

        g_codec0->track = track;
        mpeg->audio_index = (unsigned short)track;
        PRINTF("default_audio = %s, track = %d\n", g_default_audio, track);
    }
}

static void int_on_psi(struct StrmCodec* codec)
{
    struct ts_psi* psi;
    YX_MPEG_INFO mpeg_info;

    psi = &codec->ts_psi;

    memset(&mpeg_info, 0, sizeof(YX_MPEG_INFO));
    parse_psi(codec, &mpeg_info);

    codec->ts_ca.system_id = mpeg_info.i_ca_sys_id;
    codec->ts_ca.pid = mpeg_info.ECMPID;

    PRINTF("audio_index(%d) playing(%d)\n", mpeg_info.audio_index, codec->playing);

    int_on_ca_close(codec);

    if(codec->playing) {
        codec->codec_interface->decoder_stopMode(1);
        codec->codec_interface->decoder_stop();
        codec->playing = 0;
    }

    PRINTF("Index(%d)\n", codec->index);
    if(0 == codec->index) {
        PRINTF("YX_DECODER_ERROR_NONE\n");
        ymm_decoder_setErrorHandleMode(YX_DECODER_ERROR_NONE);
        g_sync = YX_DECODER_ERROR_NONE;
    }

    if(mpeg_info.video_num > 0)
        codec->video = 1;
    codec->codec_interface->decoder_start(&mpeg_info);
    codec->playing = 1;

    if(0 == codec->index && codec->iframe)
        ymm_decoder_setMode(YX_FAST_FORWARD);
    codec->codec_interface->decoder_pts(NULL);

    if(psi->dr_teletext && psi->dr_teletext->page_num > 0) {
        ymm_decoder_startTeleText();
        g_codec0->teletext = 64;
    }

    PRINTF("Index(%d) CODEC_STATE_PLAY\n", codec->index);
    codec->state = CODEC_STATE_PLAY;

    if (0 == codec->index) {
        int i, subtitle;
        struct ts_dr_subtitle *dr_subtitle = psi->dr_subtitle;

        if(dr_subtitle && dr_subtitle->subtitle_num > 0) {
            int i, subtitle;

            if (g_codec0->subtitle_default >= 0 && g_codec0->subtitle_default < dr_subtitle->subtitle_num) {
                subtitle = g_codec0->subtitle_default;
            } else {
                subtitle = 0;
                for(i = 0; i < psi->dr_subtitle->subtitle_num; i ++) {
                    if(memcmp(psi->dr_subtitle->subtitle[i].language, g_default_subtitle, 3) == 0) {
                        subtitle = i;
                        break;
                    }
                }
            }
            ymm_decoder_setSubtitlePID(psi->dr_subtitle->subtitle_pid[subtitle]);

            if (g_codec0->subtitle_show)
                ymm_decoder_showSubtitle();

            g_codec0->subtitle = subtitle;
            PRINTF("Default subtitle(%s), subtitle(%d)\n", g_default_subtitle, subtitle);
        }
    }

#ifdef SAVESTREAM
    if(codec->save_flg == 1)
        save_open(codec);
#endif
}
#endif//DUMMYDECODE

static void codec_on_psi(struct StrmCodec* codec)
{
#ifndef DUMMYDECODE
    pthread_mutex_lock(&g_mutex);

    PRINTF("++++ %d cstate = %d g_changemode = %d\n", codec->index, codec->state, g_changemode);

    if(g_mode != CODEC_MODE_NORMAL || codec->pcm_handle || codec->state != CODEC_STATE_OPEN)
        END_OUT("mode = %d, state = %d\n", g_mode, codec->state);

    pthread_mutex_lock(&codec->mutex);
    ts_psi_copy(&codec->ts_psi, &codec->arg_psi);
    pthread_mutex_unlock(&codec->mutex);

    int_on_psi(codec);

    if (0 == codec->index && codec->ts_psi.video_pid) {
        PRINTF("++++ mute = %d\n", g_mute);
        if (0 == g_mute) {
            codec_setMute(1);
            g_volumeClk = mid_10ms( ) + 150;
        }
    }

#ifdef SAVESTREAM
    if(codec->save_flg == 1)
        save_open(codec);
#endif

End:
    pthread_mutex_unlock(&g_mutex);
#endif//DUMMYDECODE
}

static void codec_on_pause(struct StrmCodec* codec)
{
    if(codec->index)
        return;
    PRINTF("++++ state = %d\n", codec->state);

    pthread_mutex_lock(&g_mutex);

    if(g_mode != CODEC_MODE_NORMAL || codec->pcm_handle || codec->state == CODEC_STATE_CLOSE || codec->state == CODEC_STATE_OPEN)
        END_OUT("state = %d\n", codec->state);

    ymm_decoder_setMode(YX_PAUSE);

    codec->state = CODEC_STATE_PAUSE;

End:
    pthread_mutex_unlock(&g_mutex);
}

static void codec_on_resume(struct StrmCodec* codec, int iptv)
{
    if(codec->index)
        return;
    PRINTF("++++ %d state = %d, iptv = %d / %d\n", codec->index, codec->state, iptv, g_iptv_flag);

    pthread_mutex_lock(&g_mutex);

    if(g_mode != CODEC_MODE_NORMAL || codec->pcm_handle || codec->state == CODEC_STATE_CLOSE || codec->state == CODEC_STATE_OPEN)
        END_OUT("state = %d\n", codec->state);

    if(0 == codec->index && g_sync != YX_DECODER_ERROR_NONE) {
        PRINTF("++++ YX_DECODER_ERROR_NONE\n");
        ymm_decoder_setErrorHandleMode(YX_DECODER_ERROR_NONE);
        g_sync = YX_DECODER_ERROR_NONE;
    }

    if (0 == codec->index && iptv != g_iptv_flag) {
        if (iptv)
            ymm_decoder_setAvsyncMode(YX_AVSYNC_MODE_PCR);
        else
            ymm_decoder_setAvsyncMode(YX_AVSYNC_MODE_AUDIO);
        g_iptv_flag = iptv;

        int_on_close(codec, 2);
        int_on_open(codec);
        int_on_psi(codec);
    } else {
        ymm_decoder_setMode(YX_NORMAL_PLAY);
    }

    codec->state = CODEC_STATE_PLAY;

End:
    pthread_mutex_unlock(&g_mutex);
}

static void codec_on_reset(struct StrmCodec* codec)
{
    if(codec->index)
        return;
    PRINTF("++++ state = %d\n", codec->state);

    pthread_mutex_lock(&g_mutex);

    if(g_mode != CODEC_MODE_NORMAL || codec->pcm_handle || codec->state == CODEC_STATE_CLOSE)
        END_OUT("#%d codec->state = %d!\n", codec->index, codec->state);

    if(codec->state == CODEC_STATE_OPEN)
        goto End;
    ymm_decoder_flush();
    if(codec->ts_ca.system_id == 0x5601/* VERIMATRIX */)
        CA_VM_RESET_STREAM(codec->index);

    codec->decode_buf = NULL;

End:
    pthread_mutex_unlock(&g_mutex);
}

static void codec_on_flush(struct StrmCodec* codec)
{
    if(codec->index)
        return;
    PRINTF("++++ state = %d\n", codec->state);

    pthread_mutex_lock(&g_mutex);

    if(g_mode != CODEC_MODE_NORMAL || codec->pcm_handle || codec->state == CODEC_STATE_CLOSE)
        END_OUT("#%d codec->state = %d!\n", codec->index, codec->state);

    if(codec->state != CODEC_STATE_OPEN)
        ymm_decoder_flush();

    codec->decode_buf = NULL;

End:
    pthread_mutex_unlock(&g_mutex);
}

static void codec_on_tplay(struct StrmCodec* codec)
{
    if(codec->index)
        return;
    PRINTF("++++ state = %d\n", codec->state);

    pthread_mutex_lock(&g_mutex);

    if(g_mode != CODEC_MODE_NORMAL || codec->pcm_handle || codec->state == CODEC_STATE_CLOSE || codec->state == CODEC_STATE_OPEN)
        END_OUT("state = %d\n", codec->state);

    PRINTF("++++ index = %d, g_sync = %d\n", codec->index, g_sync);
    if(0 == codec->index && g_sync != YX_DECODER_ERROR_PICTURE) {
        PRINTF("++++ YX_DECODER_ERROR_PICTURE\n");
        ymm_decoder_setErrorHandleMode(YX_DECODER_ERROR_NONE);
        g_sync = YX_DECODER_ERROR_PICTURE;
    }
    ymm_decoder_setMode(YX_FAST_FORWARD);

    codec->state = CODEC_STATE_TPLAY;

#ifdef SAVESTREAM
    if(codec->save_flg == 1)
        save_open(codec);
#endif

End:
    pthread_mutex_unlock(&g_mutex);
}

static void int_on_close(struct StrmCodec* codec, int clear)
{
    if(0 == codec->index && g_sync != YX_DECODER_ERROR_NONE) {
        PRINTF("++++ YX_DECODER_ERROR_NONE\n");
        ymm_decoder_setErrorHandleMode(YX_DECODER_ERROR_NONE);
        g_sync = YX_DECODER_ERROR_NONE;
    }

    int_on_ca_close(codec);

    if(codec->state == CODEC_STATE_CLOSE)
        return;

    if (0 == codec->index && g_volumeClk) {
        PRINTF("++++ mute = %d\n", g_mute);
        if (0 == g_mute)
            codec_setMute(0);
        g_volumeClk = 0;
    }

    if(0 == codec->index && codec->state != CODEC_STATE_OPEN) {
        if(g_codec0->subtitle_show && g_codec0->subtitle >= 0)
            ymm_decoder_hideSubtitle();

        if(g_codec0->teletext >= 0) {
            if(g_codec0->teletext != 64)
                codec_teletext_show(0);
            ymm_decoder_stopTeleText();
        }
    }

    /* 增加pip的控制代码 */

    if(codec->playing) {
        if(codec->state == CODEC_STATE_PAUSE || codec->state == CODEC_STATE_TPLAY)
            codec->playing = 2;

        if(2 != g_changemode || 1 == clear || 0 == codec->ts_psi.video_pid) {

            if(1 == clear || (0 == clear && (0 == codec->ts_psi.video_pid || 0 == g_changemode || 3 == g_changemode))) {
                codec->codec_interface->decoder_stopMode(0);
                codec->video = 0;
            } else {
                codec->codec_interface->decoder_stopMode(1);
            }

            codec->codec_interface->decoder_stop();
            codec->playing = 0;
        }
    }

#ifdef SAVESTREAM
    save_close(codec);
#endif

#ifdef INCLUDE_PIP
    if(1 == codec->index && 1 == clear && 1 == codec->show) {
        ymm_pip_set_show(0);
        PRINTF("decoder hide\n");
        codec->show = 0;
    }
#endif

    PRINTF("#%d CODEC_STATE_CLOSE clear = %d, video_pid = %d\n", codec->index, clear, codec->ts_psi.video_pid);

    codec->state = CODEC_STATE_CLOSE;
}

static void codec_on_close(struct StrmCodec* codec, int smooth)
{
    pthread_mutex_lock(&g_mutex);

    if(g_mode != CODEC_MODE_NORMAL || codec->pcm_handle)
        END_OUT("++++ %d g_mode = %d, pcm_handle = %d\n", codec->index, g_mode, codec->pcm_handle);

    PRINTF("++++ %d changemode = %d smooth = %d, state = %d\n", codec->index, g_changemode, smooth, codec->state);
    if (smooth)
        int_on_close(codec, -1);
    else
        int_on_close(codec, 0);
End:
    pthread_mutex_unlock(&g_mutex);
}

static void codec_on_rect(struct StrmCodec* codec)
{
    pthread_mutex_lock(&g_mutex);
    pthread_mutex_lock(&codec->mutex);
    codec->x = codec->arg_x;
    codec->y = codec->arg_y;
    codec->w = codec->arg_w;
    codec->h = codec->arg_h;
    pthread_mutex_unlock(&codec->mutex);
    PRINTF("++++ %d %d,%d,%d,%d\n", codec->index, codec->x, codec->y, codec->w, codec->h);

    if(codec->index == 0 || (codec->w > 0 && codec->h > 0))
        codec->codec_interface->decoder_rect(codec->x, codec->y, codec->w, codec->h);
    pthread_mutex_unlock(&g_mutex);
}

int codec_audio_track_set(int track)
{
    struct StrmCodec *codec = g_codecs[0];

    PRINTF("++++ track = %d\n", track);

    pthread_mutex_lock(&g_mutex);
    switch(g_mode) {
    case CODEC_MODE_NORMAL:
        if(codec->pcm_handle || codec->state == CODEC_STATE_CLOSE || codec->state == CODEC_STATE_OPEN)
            ERR_OUT("pcm_handle = %d, state = %d\n", codec->pcm_handle, codec->state);
        {
            struct ts_psi *psi = &codec->ts_psi;
            if(psi->audio_num > 0 && track >= 0 && track < psi->audio_num) {
                ymm_decoder_setAudioPID(psi->audio_pid[track]);
                g_codec0->track = track;
            }
        }
        break;
    case CODEC_MODE_ZEBRA: {
        int num = 0;
        //ymm_decoder_getNumOfAudio(&num);
        ymm_stream_playerGetNumOfAudio(0, &num);
        if(track < 0 || track >= num)
            ERR_OUT("track = %d, num = %d\n", track, num);
        //ymm_stream_setAudioPIDIndex(track);
        ymm_stream_playerSetAudioIndex(0, track);
    }
    break;
    default:
        ERR_OUT("g_mode = %d\n", g_mode);
    }

    PTHREAD_MUTEX_UNLOCK();
}

int codec_audio_track_get(int *ptrack)
{
    struct StrmCodec *codec = g_codecs[0];

    *ptrack = 0;
    pthread_mutex_lock(&g_mutex);

    switch(g_mode) {
    case CODEC_MODE_NORMAL:
        if(codec->pcm_handle || codec->state == CODEC_STATE_CLOSE || codec->state == CODEC_STATE_OPEN)
            ERR_OUT("pcm_handle = %d, state = %d\n", codec->pcm_handle, codec->state);
        *ptrack = g_codec0->track;
        break;
    case CODEC_MODE_ZEBRA:
        // ymm_decoder_getAudioPIDIndex(ptrack);
        ymm_stream_playerGetAudioIndex(0, ptrack);
        break;
    default:
        ERR_OUT("g_mode = %d\n", g_mode);
    }

    PTHREAD_MUTEX_UNLOCK();
}

int codec_audio_track_get_info(int track, char *info)
{
    struct StrmCodec *codec = g_codecs[0];

    if(info == NULL)
        return -1;

    memset(info, 0, 4);
    pthread_mutex_lock(&g_mutex);

    switch(g_mode) {
    case CODEC_MODE_NORMAL:
        if(codec->pcm_handle || codec->state == CODEC_STATE_CLOSE || codec->state == CODEC_STATE_OPEN)
            ERR_OUT("pcm_handle = %d, state = %d\n", codec->pcm_handle, codec->state);
        if(track >= 0 && track < codec->ts_psi.audio_num) {
            memcpy(info, codec->ts_psi.audio_iso693[track].language, 3);
            info[3] = 0;
        }
        break;
    case CODEC_MODE_ZEBRA: {
        char *planguage = NULL;
        ymm_decoder_getAudioLanguageFromIndex(track, &planguage);
        if(planguage)
            memcpy(info, planguage, 3);
    }
    break;
    default:
        ERR_OUT("g_mode = %d\n", g_mode);
    }

    PTHREAD_MUTEX_UNLOCK();
}

int codec_audio_track_get_pid(int track, int *pid)
{
    struct StrmCodec *codec = g_codecs[0];

    *pid = 0;
    pthread_mutex_lock(&g_mutex);

    switch(g_mode) {
    case CODEC_MODE_NORMAL:
        if(codec->pcm_handle || codec->state == CODEC_STATE_CLOSE || codec->state == CODEC_STATE_OPEN)
            ERR_OUT("pcm_handle = %d, state = %d\n", codec->pcm_handle, codec->state);
        if(track >= 0 && track < codec->ts_psi.audio_num)
            *pid = codec->ts_psi.audio_pid[track];
        break;
    case CODEC_MODE_ZEBRA:
        ymm_decoder_getAudioPIDFromIndex(track, pid);
        break;
    default:
        ERR_OUT("g_mode = %d\n", g_mode);
    }

    PTHREAD_MUTEX_UNLOCK();
}

int codec_audio_track_get_type(int track, int *ptype)
{
    struct StrmCodec *codec = g_codecs[0];

    *ptype = 0;
    pthread_mutex_lock(&g_mutex);

    switch(g_mode) {
    case CODEC_MODE_NORMAL:
        if(codec->pcm_handle || codec->state == CODEC_STATE_CLOSE || codec->state == CODEC_STATE_OPEN)
            ERR_OUT("pcm_handle = %d, state = %d\n", codec->pcm_handle, codec->state);
        if(track >= 0 && track < codec->ts_psi.audio_num) {
            int type = codec->ts_psi.audio_type[track];

            if(ISO_EXT_DTS_AUDIO == type)
                *ptype = YX_AUDIO_TYPE_DTS;
            else if(ISO_IEC_AC3_AUDIO == type)
                *ptype = YX_AUDIO_TYPE_AC3;
            else if(ISO_EXT_AC3_AUDIO == type)
                *ptype = YX_AUDIO_TYPE_AC3PLUS;
            else
                *ptype = ys_get_audio_codec_from_iso(type);
        }
        break;
    case CODEC_MODE_ZEBRA: {
        YX_MPEG_INFO mpeg_info;

        ymm_decoder_getMpegInfo(&mpeg_info);
        if(track >= 0 && track < mpeg_info.audio_num) {
            *ptype = mpeg_info.audio_type[track];
        }
        break;
    }
    default:
        break;
    }
    PRINTF("play mode(%d) audio format(%d)\n", g_mode, *ptype);
    PTHREAD_MUTEX_UNLOCK();
}

int codec_audio_track_num(int *pnum)
{
    struct StrmCodec *codec = g_codecs[0];

    *pnum = 0;
    pthread_mutex_lock(&g_mutex);

    switch(g_mode) {
    case CODEC_MODE_NORMAL:
        if(codec->pcm_handle || codec->state == CODEC_STATE_CLOSE || codec->state == CODEC_STATE_OPEN)
            ERR_OUT("pcm_handle = %d, state = %d\n", codec->pcm_handle, codec->state);
        *pnum = codec->ts_psi.audio_num;
        break;
    case CODEC_MODE_ZEBRA:
        ymm_decoder_getNumOfAudio(pnum);
        break;
    default:
        ERR_OUT("g_mode = %d\n", g_mode);
    }

    PTHREAD_MUTEX_UNLOCK();
}

int codec_subtitle_show_set(int flag)
{
    PTHREAD_MUTEX_BEGIN();
    if(g_mode == CODEC_MODE_NORMAL) {
        if(flag) {
            ymm_decoder_showSubtitle();
            g_codec0->subtitle_show = 1;
        } else {
            ymm_decoder_hideSubtitle();
            g_codec0->subtitle_show = 0;
        }
    } else if(g_mode == CODEC_MODE_ZEBRA) {
        ymm_stream_subPlayerSetShow(0, flag);
        g_codec0->subtitle_show = flag;
    }

    PTHREAD_MUTEX_UNLOCK();
}

int codec_subtitle_show_get(int flag)
{
    int show = 0;

    pthread_mutex_lock(&g_mutex);
    show = g_codec0->subtitle_show;
    pthread_mutex_unlock(&g_mutex);
    return show;
}

int codec_subtitle_set(int subtitle_index)
{
    PTHREAD_MUTEX_BEGIN();

    PRINTF("++++ subtitle = %d\n", subtitle_index);

    switch(g_mode) {
    case CODEC_MODE_NORMAL: {
        struct StrmCodec *codec = g_codecs[0];
        struct ts_dr_subtitle *dr_subtitle;

        if(codec->state == CODEC_STATE_CLOSE || codec->state == CODEC_STATE_OPEN)
            ERR_OUT("stoped!\n");

        dr_subtitle = codec->ts_psi.dr_subtitle;
        if(dr_subtitle == NULL)
            ERR_OUT("dr_subtitle is NULL\n");
        if(subtitle_index >= 0 && subtitle_index < dr_subtitle->subtitle_num) {
            ymm_decoder_setSubtitlePID(dr_subtitle->subtitle_pid[subtitle_index]);
            g_codec0->subtitle = subtitle_index;
            g_codec0->subtitle_default = subtitle_index;
        }
    }
    break;
    case CODEC_MODE_ZEBRA: {
        int subtitle_num;

        if(CODEC_STATE_CLOSE == g_state)
            ERR_OUT("stoped!\n");
        subtitle_num = 0;
        //ymm_decoder_getNumOfSubtitle(&subtitle_num);
        ymm_stream_subPlayerGetLanguageNum(0, &subtitle_num);
        if(subtitle_index >= 0 && subtitle_index < subtitle_num) {
            //ymm_decoder_setSubtitlePIDIndex(subtitle_index);
            ymm_stream_subPlayerSetLanguageByIndex(0, subtitle_index);
            g_codec0->subtitle = subtitle_index;
        }
    }
    break;
    default:
        ERR_OUT("g_mode = %d!\n", g_mode);
        break;
    }

    PTHREAD_MUTEX_UNLOCK();
}

int codec_subtitle_get(int *psubtitle)
{
    PTHREAD_MUTEX_BEGIN();
    *psubtitle = g_codec0->subtitle;
    PTHREAD_MUTEX_UNLOCK();
}

int codec_subtitle_lang(int subtitle_index, char *language)
{
    memset(language, 0, 4);

    PTHREAD_MUTEX_BEGIN();

    switch(g_mode) {
    case CODEC_MODE_NORMAL: {
        struct StrmCodec *codec = g_codecs[0];
        struct ts_dr_subtitle *dr_subtitle;

        dr_subtitle = codec->ts_psi.dr_subtitle;
        if(subtitle_index >= 0 && subtitle_index < dr_subtitle->subtitle_num)
            memcpy(language, dr_subtitle->subtitle[subtitle_index].language, 3);
    }
    break;
    case CODEC_MODE_ZEBRA: {
        char *subtitle_lang;
        int subtitle_num;

        subtitle_num = 0;
        //ymm_decoder_getNumOfSubtitle(&subtitle_num);
        ymm_stream_subPlayerGetLanguageNum(0, &subtitle_num);
        if(subtitle_index >= 0 && subtitle_index < subtitle_num) {
            subtitle_lang = NULL;
            //ymm_decoder_getSubtitleLangFromIndex(subtitle_index, &subtitle_lang);
            ymm_stream_subPlayerGetLanguageByIndex(0, subtitle_index, &subtitle_lang);
            if(subtitle_lang)
                memcpy(language, subtitle_lang, 3);
        }
    }
    break;
    default:
        ERR_OUT("g_mode = %d!\n", g_mode);
        break;
    }

    PTHREAD_MUTEX_UNLOCK();
}
int codec_subtitle_pid(int subtitle_index, unsigned short* pid)
{
    *pid = 0x1fff;
    PTHREAD_MUTEX_BEGIN();

    switch(g_mode) {
    case CODEC_MODE_NORMAL: {
        struct StrmCodec *codec = g_codecs[0];
        struct ts_dr_subtitle *dr_subtitle;

        dr_subtitle = codec->ts_psi.dr_subtitle;
        if(subtitle_index >= 0 && subtitle_index < dr_subtitle->subtitle_num)
            *pid = dr_subtitle->subtitle_pid[subtitle_index];
    }
    break;
    case CODEC_MODE_ZEBRA: {
        int subtitle_num = 0;

        ymm_stream_subPlayerGetLanguageNum(0, &subtitle_num);
        if(subtitle_index >= 0 && subtitle_index < subtitle_num) {
            //subtitle_pid = 0x1fff;
            //ymm_decoder_getSubtitlePIDFromIndex(subtitle_index, &subtitle_pid);
            //*pid = (unsigned short)((unsigned int)subtitle_pid);
            *pid = subtitle_index;
        }
    }
    break;
    default:
        ERR_OUT("g_mode = %d!\n", g_mode);
        break;
    }

    PTHREAD_MUTEX_UNLOCK();
}
int codec_subtitle_num(int *pnum)
{
    *pnum = 0;

    PTHREAD_MUTEX_BEGIN();

    switch(g_mode) {
    case CODEC_MODE_NORMAL: {
        struct StrmCodec *codec = g_codecs[0];

        if(codec->state != CODEC_STATE_CLOSE && codec->state != CODEC_STATE_OPEN)
            *pnum = codec->ts_psi.dr_subtitle->subtitle_num;
    }
    break;
    case CODEC_MODE_ZEBRA: {
        if(g_state != CODEC_STATE_CLOSE)
            ymm_stream_subPlayerGetLanguageNum(0, pnum);//ymm_decoder_getNumOfSubtitle(pnum);
    }
    break;
    default:
        ERR_OUT("g_mode = %d!\n", g_mode);
        break;
    }

    PTHREAD_MUTEX_UNLOCK();
}

/*
    -1 关闭
    0 ~ 63 选择特点teletext
    64 选择特默认teletext（上次关闭的）
 */
static void codec_teletext_show(int teletext)
{
    struct StrmCodec *codec = g_codecs[0];
    struct ts_dr_teletext *dr_teletext = codec->ts_psi.dr_teletext;

    if(teletext != 64 && teletext == g_codec0->teletext)
        return;

    if(dr_teletext == NULL || g_codec0->teletext < 0)
        ERR_OUT("dr_teletext = %p, ts_teletext = %d\n", dr_teletext, g_codec0->teletext);

    if(teletext == 64)
        teletext = g_codec0->teletext_default;
    if(teletext >= 0 && teletext < dr_teletext->page_num) {
        int magazine = dr_teletext->page[teletext].magazine;
        if(magazine < 0 || magazine > 7)
            ERR_OUT("magazine = %d\n", magazine);
        if(magazine == 0)
            magazine = 8;

        PRINTF("show teletext = %d / %d, magazine = %d\n", teletext, g_codec0->teletext, magazine);
        if(g_codec0->teletext == 64)
            ymm_decoder_setTeleTextShow(1);
        ymm_decoder_displayTeleTextPage(magazine * 100, -1);
        g_codec0->teletext = teletext;
    } else {
        PRINTF("hide teletext\n");
        if(g_codec0->teletext != 64)
            ymm_decoder_setTeleTextShow(0);
        if(g_codec0->teletext >= 0)
            g_codec0->teletext = 64;
    }
Err:
    return;
}

int codec_teletext_set(int teletext)
{
    struct StrmCodec *codec = g_codecs[0];
    PTHREAD_MUTEX_NORMAL();

    PRINTF("++++ teletext = %d\n", teletext);

    if(g_mode != CODEC_MODE_NORMAL || codec->state == CODEC_STATE_CLOSE || codec->state == CODEC_STATE_OPEN)
        ERR_OUT("stoped!\n");

    codec_teletext_show(teletext);
    if (teletext < 64)
        g_codec0->teletext_default = teletext;

    PTHREAD_MUTEX_UNLOCK();
}

int codec_teletext_get(int *pteletext)
{
    struct StrmCodec *codec = g_codecs[0];
    PTHREAD_MUTEX_NORMAL();

    *pteletext = g_codec0->teletext;

    PTHREAD_MUTEX_UNLOCK();
}

int codec_teletext_lang(int teletext, char *language)
{
    struct StrmCodec *codec = g_codecs[0];
    struct ts_dr_teletext *dr_teletext;

    PTHREAD_MUTEX_NORMAL();

    dr_teletext = codec->ts_psi.dr_teletext;
    if(dr_teletext && teletext >= 0 && teletext < dr_teletext->page_num) {
        memcpy(language, dr_teletext->page[teletext].language, 3);
        language[3] = 0;
    } else {
        memset(language, 0, 4);
    }
    PTHREAD_MUTEX_UNLOCK();
    return 0;
}

int codec_teletext_num(int *pnum)
{
    struct StrmCodec *codec = g_codecs[0];
    struct ts_dr_teletext *dr_teletext;

    PTHREAD_MUTEX_NORMAL();

    dr_teletext = codec->ts_psi.dr_teletext;
    if(dr_teletext == NULL || codec->state == CODEC_STATE_CLOSE || codec->state == CODEC_STATE_OPEN)
        *pnum = 0;
    else
        *pnum = dr_teletext->page_num;

    PTHREAD_MUTEX_UNLOCK();
}

int codec_teletext_page(int page)
{
    struct StrmCodec *codec = g_codecs[0];
    PTHREAD_MUTEX_NORMAL();

    PRINTF("++++ page = %d\n", page);

    if(g_codec0->teletext < 0)
        ERR_OUT("ts_teletext = %d\n", g_codec0->teletext);

    if(page == 0) {
        codec_teletext_show(-1);
        goto End;
    }

    if(page != -1 && page != 1 && (page < 100 || page > 899))
        ERR_OUT("page = %d\n", page);

    if(g_codec0->teletext == 64)
        codec_teletext_show(64);

    if(page == -1)
        ymm_decoder_displayTeleTextNextPage(-1);
    else if(page == 1)
        ymm_decoder_displayTeleTextNextPage(1);
    else if(page >= 100 && page <= 899)
        ymm_decoder_displayTeleTextPage(page, -1);

End:
    PTHREAD_MUTEX_UNLOCK();
}

int codec_teletext_subpage(int subpage)
{
    struct StrmCodec *codec = g_codecs[0];
    struct ts_dr_teletext *dr_teletext;

    PTHREAD_MUTEX_NORMAL();

    PRINTF("++++ subpage = %d\n", subpage);

    dr_teletext = codec->ts_psi.dr_teletext;
    if(dr_teletext == NULL || dr_teletext->page_num <= 0 || g_codec0->teletext < 0)
        ERR_OUT("dr_teletext = %p / %d\n", dr_teletext, g_codec0->teletext);

    if(subpage == 1)
        ymm_decoder_displayTeleTextNextSubPage(1);
    else if(subpage == -1)
        ymm_decoder_displayTeleTextNextSubPage(-1);
    else
        ERR_OUT("subpage = %d\n", subpage);

    PTHREAD_MUTEX_UNLOCK();
}

int codec_pcm_open(int pIndex, int sampleRate, int bitWidth, int channels)
{
    struct StrmCodec *codec = g_codecs[pIndex];

    PRINTF("++++++++\n");

    PTHREAD_MUTEX_BEGIN();

    if(g_mode == CODEC_MODE_NORMAL) {
        codec_on_clear_normal(codec, 1);
    } else {
        codec_on_close_all();
    }
    if(ymm_audio_PCMOpenByMixType(&codec->pcm_handle, pIndex, 100, 0))
        ERR_OUT("ymm_audio_PCMOpenByIndex\n");

    ymm_audio_PCMSet(codec->pcm_handle, sampleRate, bitWidth, channels);
    ymm_audio_PCMStart(codec->pcm_handle);

    codec->state = CODEC_STATE_OPEN;

    PTHREAD_MUTEX_UNLOCK();
}

static void codec_on_close_pcm(struct StrmCodec *codec)
{
    if(codec->state == CODEC_STATE_CLOSE)
        return;

    ymm_audio_PCMClose(codec->pcm_handle);
    codec->pcm_handle = 0;

    codec->state = CODEC_STATE_CLOSE;
}

int codec_pcm_close(int pIndex)
{
    PTHREAD_MUTEX_PCM();

    PRINTF("++++++++\n");
    codec_on_close_pcm(codec);

    PTHREAD_MUTEX_UNLOCK();
}

int codec_pcm_push(int pIndex, char* buf, int len)
{
    char* decode_buf;
    int decode_len;

    PTHREAD_MUTEX_PCM();

    if(codec->state != CODEC_STATE_OPEN)
        ERR_OUT("state = %d\n", codec->state);

    decode_len = 0;
    ymm_audio_PCMGetBuffer(codec->pcm_handle, (char **)&decode_buf, &decode_len);

    if(decode_len <= 0)
        goto Err;

    if(len > decode_len)
        len = decode_len;
    if(len <= 0)
        goto Err;

    memcpy(decode_buf, buf, len);
    ymm_audio_PCMReadComplete(codec->pcm_handle, len);

    pthread_mutex_unlock(&g_mutex);
    return len;
Err:
    pthread_mutex_unlock(&g_mutex);
    return 0;
}

int codec_mosaic_open(void)
{
    PRINTF("++++ g_mode = %d\n", g_mode);

    PTHREAD_MUTEX_BEGIN();

    if(CODEC_MODE_MOSAIC == g_mode) {
        codec_on_close_mosaic();
    } else {
        int ret;
        codec_on_close_all();
#ifdef INCLUDE_PIP
        printf("SDK: ymm_pip_uninit\n");
        ymm_pip_uninit();
#endif
        printf("SDK: ymm_mosaic_open\n");
        ret = ymm_mosaic_open(9);
        PRINTF("++++ ret = %d\n", ret);
    }

    g_state = CODEC_STATE_PLAY;
    g_msc_audio_key = -1;
    g_msc_audio_flag = 0;

    {
        int i;
        for(i = 0; i < MOSAIC_NUM; i ++)
            g_msc_array[i].key = -1;
    }

    g_mode = CODEC_MODE_MOSAIC;

    PTHREAD_MUTEX_UNLOCK();
}

static void int_mosaic_elem_set(void)
{
    int i, type;
    ts_mosaic_t msc;

    PRINTF("++++++++ key = %d\n", g_msc_audio_key);

    if(g_msc_audio_flag) {
        PRINTF("++++++++ ymm_mosaic_detachAudio\n");
        ymm_mosaic_detachAudio();
        g_msc_audio_flag = 0;
    }

    if(g_msc_audio_key < 0)
        return;

    for(i = 0; i < MOSAIC_NUM; i ++) {
        msc = &g_msc_array[i];
        if(g_msc_audio_key == msc->key) {
            if(msc->apid) {
                PRINTF("++++++++ ymm_mosaic_attachAudio\n");
                type = ys_get_audio_codec_from_iso(msc->atype);
                ymm_mosaic_attachAudioIndex(msc->apid, type, 0);
                PRINTF("@@@@@@@@: audio_pid = %d, audio_type = %d\n", msc->apid, type);
                g_msc_audio_flag = 1;
            }
            break;
        }
    }
}

int codec_mosaic_close(void)
{
    PRINTF("++++ state = %d\n", g_state);

    PTHREAD_MUTEX_MOSAIC();

    codec_on_close_mosaic();

    PTHREAD_MUTEX_UNLOCK();
}

static void int_mosaic_elem_close(int key)
{
    int i;
    ts_mosaic_t msc;

    if(key < 0)
        return;

    for(i = 0; i < MOSAIC_NUM; i ++) {
        msc = &g_msc_array[i];

        if(key != msc->key)
            continue;

        if(key == g_msc_audio_key && g_msc_audio_flag) {
            ymm_mosaic_detachAudio();
            g_msc_audio_flag = 0;
        }
        printf("SDK: ymm_mosaic_stopBufferByIndex=%d\n", i);
        ymm_mosaic_stopBufferByIndex(i);
        msc->key = -1;
    }
}

int codec_mosaic_elem_open(ts_mosaic_t ts_mosaic)
{
    int i, ret, key, result = -1;

    PRINTF("++++ state = %d\n", g_state);

    PTHREAD_MUTEX_MOSAIC();

    key = ts_mosaic->key;
    if(key < 0 || NULL == ts_mosaic)
        ERR_OUT("++++ key = %d, ts_mosaic = %p\n", key, ts_mosaic);

    if(CODEC_STATE_CLOSE == g_state)
        ERR_OUT("++++ state = %d\n", g_state);

    int_mosaic_elem_close(key);

    for(i = 0; i < MOSAIC_NUM; i ++) {
        if(g_msc_array[i].key < 0)
            break;
    }
    if(i >= MOSAIC_NUM)
        ERR_OUT("++++ msc_array is FULL\n");

    g_msc_array[i] = *ts_mosaic;
    {
        YX_RECT rect;

        memset(&rect, 0, sizeof(rect));
        rect.x = ts_mosaic->x;
        rect.y = ts_mosaic->y;
        rect.w = ts_mosaic->width;
        rect.h = ts_mosaic->height;

        ret = ymm_mosaic_setDisplayRectByIndex(i, &rect);
        if(ret)
            WARN_PRN("ymm_mosaic_setDisplayRectByIndex ret = %d\n", ret);
    }

    if (0x5601 == g_msc_array[i].ts_ca.system_id)
        CA_VM_RESET_MASICSTREAM(i);

    {
        YX_MPEG_INFO mpeg_info;

        memset(&mpeg_info, 0, sizeof(mpeg_info));

        mpeg_info.stream_type = YX_STREAM_TS;
        if(ts_mosaic->vpid) {
            mpeg_info.video_num = 1;
            mpeg_info.video_pid[0] = ts_mosaic->vpid;
            mpeg_info.video_type[0] = ys_get_video_codec_from_iso(ts_mosaic->vtype);
        }
        if(ts_mosaic->apid) {
            mpeg_info.audio_num = 1;
            mpeg_info.audio_pid[0] = ts_mosaic->apid;
            if(ISO_EXT_DTS_AUDIO == ts_mosaic->atype)
                mpeg_info.audio_type[0] = YX_AUDIO_TYPE_DTS;
            else if(ISO_EXT_AC3_AUDIO == ts_mosaic->atype)
                mpeg_info.audio_type[0] = YX_AUDIO_TYPE_AC3PLUS;
            else
                mpeg_info.audio_type[0] = ys_get_audio_codec_from_iso(ts_mosaic->atype);
        }
        PRINTF("SDK: ymm_mosaic_startBufferByIndex=%d\n", i);
        ret = ymm_mosaic_startBufferByIndex(i, &mpeg_info);
        if(ret)
            WARN_PRN("ymm_mosaic_startBufferByIndex ret = %d\n", ret);

        PRINTF("++++++++[%d] key = %d video = 0x%x/%d, audio = 0x%x/%d (%d %d %d %d)\n", i, key, mpeg_info.video_pid[0], mpeg_info.video_type[0], mpeg_info.audio_pid[0], mpeg_info.audio_type[0], ts_mosaic->x, ts_mosaic->y, ts_mosaic->width, ts_mosaic->height);
    }

    if(key == g_msc_audio_key)
        int_mosaic_elem_set();

    result = 0;

Err:
    pthread_mutex_unlock(&g_mutex);
    return result;
}

void codec_mosaic_elem_close(int key)
{
    PRINTF("Key(%d)\n", key);

    PTHREAD_MUTEX_MOSAIC();

    if(key < 0)
        ERR_OUT("Key(%d)\n", key);

    if(CODEC_STATE_CLOSE == g_state)
        ERR_OUT("CODEC_STATE_CLOSE\n");

    int_mosaic_elem_close(key);

Err:
    pthread_mutex_unlock(&g_mutex);
    return;
}

static void codec_on_close_mosaic(void)
{
    int i, key;

    if(g_state == CODEC_STATE_CLOSE)
        return;

    for(i = 0; i < MOSAIC_NUM; i ++) {
        key = g_msc_array[i].key;
        if(key >= 0)
            int_mosaic_elem_close(key);
    }

    g_state = CODEC_STATE_CLOSE;
    return;
}

int codec_mosaic_set(int key)
{
    PRINTF("Key(%d)\n", key);

    pthread_mutex_lock(&g_mutex);

    if(key == g_msc_audio_key)
        goto End;

    g_msc_audio_key = key;

    if(g_mode == CODEC_MODE_MOSAIC && g_state == CODEC_STATE_PLAY)
        int_mosaic_elem_set();

End:
    pthread_mutex_unlock(&g_mutex);
    return 0;
}

int codec_mosaic_get(void)
{
    int key;

    pthread_mutex_lock(&g_mutex);
    key = g_msc_audio_key;
    pthread_mutex_unlock(&g_mutex);
    return key;
}

int codec_mosaic_decript(int key, char* buf, int len)
{
    int i;

    pthread_mutex_lock(&g_mutex);
    if (g_mode != CODEC_MODE_MOSAIC)
        goto Err;

    if (key < 0)
        goto Err;

    for (i = 0; i < MOSAIC_NUM; i ++) {
        if (key == g_msc_array[i].key)
            break;
    }
    if (i >= MOSAIC_NUM)
        goto Err;
    if (0x5601 == g_msc_array[i].ts_ca.system_id)
        CA_VM_DECRYPT_MOSIC(i, buf, len);

Err:
	pthread_mutex_unlock(&g_mutex);
	return len;
}

int codec_mosaic_push(char* buf, int len)
{
    int l, bytes = -1;
    char* buffer;

    PTHREAD_MUTEX_MOSAIC();

    bytes = 0;
    if(g_state != CODEC_STATE_PLAY)
        goto Err;

    while(len > 0) {
        l = -1;
        if(ymm_decoder_getBuffer(&buffer, &l))
            ERR_OUT("ymm_decoder_getBuffer\n");
        if(l <= 0) {
            WARN_PRN("l = %d\n", l);
            goto Err;
        }
        l = (l / 188) * 188;
        if(l > len)
            l = len;
        memcpy(buffer, buf, l);
        ymm_decoder_pushBuffer(buffer, l);
        buf += l;
        len -= l;
        bytes += l;
    }

Err:
    pthread_mutex_unlock(&g_mutex);
    return bytes;
}

int codec_set_hd_AspectRation(int aspect, int ratio)
{
    if (aspect < YX_ASPECT_MODE_LETTERBOX
        || aspect > YX_ASPECT_MODE_FULL_NOLINER
        || ratio < YX_DISPLAY_ASPECT_RATIO_4x3
        || ratio > YX_DISPLAY_ASPECT_RATIO_16x9) {
        PRINTF("Aspect Ration  ERROR!\n");
        return -1;
    }

    yhw_vout_setHDAspectRatio(aspect, ratio);
    return 0;
}

int codec_set_sd_AspectRation(int aspect, int ratio)
{
    if (aspect < YX_ASPECT_MODE_LETTERBOX
        || aspect > YX_ASPECT_MODE_FULL_NOLINER
        || ratio < YX_DISPLAY_ASPECT_RATIO_4x3
        || ratio > YX_DISPLAY_ASPECT_RATIO_16x9) {
        PRINTF("Aspect Ration  ERROR!\n");
        return -1;
    }
    yhw_vout_setSDAspectRatio(aspect, ratio);
    return 0;
}

#ifdef INCLUDE_FLASHPLAY
int codec_flash_open(char* url)
{
    int ret = -1;

    PRINTF("Url(%s)\n", url);

    PTHREAD_MUTEX_BEGIN();

    codec_on_close_all();
    g_mode = CODEC_MODE_FLASH;

    pthread_mutex_unlock(&g_mutex);

    //添加flash播放代码
    extern void *flashlite_task(void * flash);
    extern int flashlite_stop(void);

    flashlite_task(NULL);
    flashlite_stop();

    pthread_mutex_lock(&g_mutex);
    g_mode = CODEC_MODE_NORMAL;
    ret = 0;
Err:
    pthread_mutex_unlock(&g_mutex);
    return ret;
}

int codec_flash_close(void)
{
    return 0;
}
#else
int codec_flash_open(char* url)
{
    return 0;
}

int codec_flash_close(void)
{
    return 0;
}
#endif

int codec_zebra_open(int pIndex, char *url, int arg)
{
    PRINTF("Url(%s)\n", url);

    PTHREAD_MUTEX_BEGIN();
    if(g_mode == CODEC_MODE_ZEBRA) {
        codec_on_close_zebra();
    } else {
        codec_on_close_all();
        ymm_decoder_setAvsyncMode(YX_AVSYNC_MODE_AUDIO);
    }

    PRINTF("ymm_stream_playerstart\n");
    ymm_stream_playerStart(0, url, 0);

    g_state = CODEC_STATE_PLAY;
    g_mode = CODEC_MODE_ZEBRA;

    pthread_mutex_unlock(&g_mutex);
Err:
    return 0;
}

static void codec_on_close_zebra(void)
{
    if(g_state == CODEC_STATE_CLOSE)
        return;

    ymm_stream_playerStop(0);
    g_state = CODEC_STATE_CLOSE;
    return;
}

int codec_zebra_close(int pIndex)
{
    PTHREAD_MUTEX_ZEBRA();
    PRINTF("State(%d)\n", g_state);
    codec_on_close_zebra();
    PTHREAD_MUTEX_UNLOCK();
    return 0;
}

int codec_zebra_pcm_open(int pIndex, ZebraPCM* zpcm, int arg)
{
    int ret = -1;

    PRINTF("Url(%s)\n", zpcm->url);

    PTHREAD_MUTEX_BEGIN();

    if(g_mode == CODEC_MODE_ZEBRA_PCM)
        codec_on_close_zebra_pcm();
    else
        codec_on_close_all();

    ret = ymm_audio_playPCMStream(0, zpcm->url, 0, zpcm->sampleRate, zpcm->bitWidth, zpcm->channels, zpcm->b_signed, zpcm->b_bigendian);

    g_state = CODEC_STATE_PLAY;
    g_mode = CODEC_MODE_ZEBRA_PCM;
Err:
    pthread_mutex_unlock(&g_mutex);
    return ret;
}

static void codec_on_close_zebra_pcm(void)
{
    if(g_state == CODEC_STATE_CLOSE)
        return;

    ymm_audio_stopPCMStream(0);
    g_state = CODEC_STATE_CLOSE;
    return;
}

int codec_zebra_pcm_close(int pIndex)
{
    PTHREAD_MUTEX_ZEBRA_PCM();
    PRINTF("State(%d)\n", g_state);
    codec_on_close_zebra_pcm();
    PTHREAD_MUTEX_UNLOCK();
    return 0;
}

int codec_cloud_open(YX_MPEG_INFO* mpeg)
{
    PRINTF("++++++++\n");

    PTHREAD_MUTEX_BEGIN( );

    codec_on_close_all( );

    ymm_decoder_setAvsyncMode(YX_AVSYNC_MODE_DISABLED);

    ymm_decoder_setMpegInfo(mpeg);
    ymm_decoder_startBuffer( );

    g_mode = CODEC_MODE_CLOUD;
    g_codecs[0]->decode_buf = NULL;

    PTHREAD_MUTEX_UNLOCK( );
}

int codec_cloud_close(void)
{
    PTHREAD_MUTEX_CLOUD( );

    PRINTF("++++++++\n");
    ymm_decoder_setStopMode(0);
    ymm_decoder_stopBuffer( );
    g_mode = CODEC_MODE_NORMAL;

    PTHREAD_MUTEX_UNLOCK( );
}


void codec_cloud_get(char **pbuf, int *plen)
{
    struct StrmCodec *codec = g_codecs[0];

    *pbuf = NULL;
    *plen = 0;

    pthread_mutex_lock(&g_mutex);

    if (g_mode != CODEC_MODE_CLOUD)
        goto Err;

    codec->decode_buf = NULL;
    if (ymm_decoder_getBuffer(&codec->decode_buf, &codec->decode_len))
        ERR_OUT("ymm_decoder_getBuffer\n");

    *pbuf = codec->decode_buf;
    *plen = codec->decode_len;

Err:
    pthread_mutex_unlock(&g_mutex);
}

void codec_cloud_put(int len)
{
    struct StrmCodec *codec = g_codecs[0];

    pthread_mutex_lock(&codec->mutex);

    if (g_mode != CODEC_MODE_CLOUD || !codec->decode_buf)
        goto Err;

    if (ymm_decoder_pushBuffer(codec->decode_buf, len))
        ERR_OUT("ymm_decoder_pushBuffer\n");

Err:
    codec->decode_buf = NULL;
    pthread_mutex_unlock(&codec->mutex);
}

