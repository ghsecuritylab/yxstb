
#include <string.h>

#include "Customer.h"

#include "SystemManager.h"
#include "UltraPlayer.h"
#include "BrowserAgentTakin.h"

#ifdef HUAWEI_C10
#include "NativeHandlerPublicC10.h"
#endif

#include "Canvas.h"
#include "StandardScreen.h"
#include "StandardViewGroup.h"
#include "Widget.h"
#include "BrowserViewTakin.h"
#include "BrowserLayerZebra.h"
#include "OSDLayer.h"
#include "Icon.h"
#include "PlayStateWidget.h"
#include "ChannelNOWidget.h"
#include "ProgressBarWidget.h"
#include "AudioMuteWidget.h"
#include "AudioTrackWidget.h"
#include "AudioVolumeWidget.h"
#include "DolbyWidget.h"
#include "DolbyDownmixWidget.h"
#include "PromptWidget.h"
#include "LogoWidget.h"
#include "PicSetWidget.h"
#include "Assertions.h"

#include "cairo/cairo.h"
#include "ft2build.h"
#include "freetype/freetype.h"

#include "libzebra.h"

#include "SysSetting.h"

int topLayer = 0;
int middleLayer = 0;
int bottomLayer = 0;

//waitclock
#define GIF_WAITCLOCK_1_PATH SYS_IMG_PATH_ROOT"/loading/waitclock/clock1.gif"
#define GIF_WAITCLOCK_2_PATH SYS_IMG_PATH_ROOT"/loading/waitclock/clock2.gif"
#define GIF_WAITCLOCK_3_PATH SYS_IMG_PATH_ROOT"/loading/waitclock/clock3.gif"
#define GIF_WAITCLOCK_4_PATH SYS_IMG_PATH_ROOT"/loading/waitclock/clock4.gif"
#define GIF_WAITCLOCK_5_PATH SYS_IMG_PATH_ROOT"/loading/waitclock/clock5.gif"
#define GIF_WAITCLOCK_6_PATH SYS_IMG_PATH_ROOT"/loading/waitclock/clock6.gif"
#define GIF_WAITCLOCK_7_PATH SYS_IMG_PATH_ROOT"/loading/waitclock/clock7.gif"
#define GIF_WAITCLOCK_8_PATH SYS_IMG_PATH_ROOT"/loading/waitclock/clock8.gif"
#define GIF_WAITCLOCK_9_PATH SYS_IMG_PATH_ROOT"/loading/waitclock/clock9.gif"
#define GIF_WAITCLOCK_10_PATH SYS_IMG_PATH_ROOT"/loading/waitclock/clock10.gif"
#define GIF_WAITCLOCK_11_PATH SYS_IMG_PATH_ROOT"/loading/waitclock/clock11.gif"
#define GIF_WAITCLOCK_12_PATH SYS_IMG_PATH_ROOT"/loading/waitclock/clock12.gif"
#define GIF_WAITCLOCK_13_PATH SYS_IMG_PATH_ROOT"/loading/waitclock/clock13.gif"
#define GIF_WAITCLOCK_14_PATH SYS_IMG_PATH_ROOT"/loading/waitclock/clock14.gif"
#define GIF_WAITCLOCK_15_PATH SYS_IMG_PATH_ROOT"/loading/waitclock/clock15.gif"
#define GIF_WAITCLOCK_16_PATH SYS_IMG_PATH_ROOT"/loading/waitclock/clock16.gif"

//seek to
#define GIF_PROGRESS_BG_PATH SYS_IMG_PATH_ROOT"/playstate/seek/progress_bg.gif"

//dolby
#define PNG_DOLBY_DIGITALPLUS_PATH SYS_IMG_PATH_ROOT"/audio/Dolby_DigitalPlus.png"
#define PNG_DOLBY_DOWNMIX_PATH SYS_IMG_PATH_ROOT"/audio/Dolby_Downmix.png"

//audio mute
#define GIF_AUDIO_MUTE_ON_PATH SYS_IMG_PATH_ROOT"/audio/mute_on.gif"
#define GIF_AUDIO_MUTE_OFF_PATH SYS_IMG_PATH_ROOT"/audio/mute_off.gif"

//audio track
#define GIF_AUDIO_TRACK_LR_PATH SYS_IMG_PATH_ROOT"/audio/track_lr.gif"
#define GIF_AUDIO_TRACK_LL_PATH SYS_IMG_PATH_ROOT"/audio/track_ll.gif"
#define GIF_AUDIO_TRACK_RR_PATH SYS_IMG_PATH_ROOT"/audio/track_rr.gif"
#define GIF_AUDIO_TRACK_RL_PATH SYS_IMG_PATH_ROOT"/audio/track_rl.gif"

//playstate
#define GIF_PLAYSTATE_LIVE_PATH SYS_IMG_PATH_ROOT"/playstate/live.gif"
#define GIF_PLAYSTATE_WATCH_PATH SYS_IMG_PATH_ROOT"/playstate/watch.gif"
#define GIF_PLAYSTATE_PAUSE_PATH SYS_IMG_PATH_ROOT"/playstate/pause.gif"
#define GIF_PLAYSTATE_PLAY_PATH SYS_IMG_PATH_ROOT"/playstate/play.gif"
#define GIF_PLAYSTATE_STOP_PATH SYS_IMG_PATH_ROOT"/playstate/stop.gif"

//fastforward
#define GIF_PLAYSTATE_FW2_PATH SYS_IMG_PATH_ROOT"/playstate/forward_2.gif"
#define GIF_PLAYSTATE_FW4_PATH SYS_IMG_PATH_ROOT"/playstate/forward_4.gif"
#define GIF_PLAYSTATE_FW8_PATH SYS_IMG_PATH_ROOT"/playstate/forward_8.gif"
#define GIF_PLAYSTATE_FW16_PATH SYS_IMG_PATH_ROOT"/playstate/forward_16.gif"
#define GIF_PLAYSTATE_FW32_PATH SYS_IMG_PATH_ROOT"/playstate/forward_32.gif"
#define GIF_PLAYSTATE_FWEND_PATH SYS_IMG_PATH_ROOT"/playstate/forward_end.gif"

//fastrewind
#define GIF_PLAYSTATE_RW2_PATH SYS_IMG_PATH_ROOT"/playstate/rewind_2.gif"
#define GIF_PLAYSTATE_RW4_PATH SYS_IMG_PATH_ROOT"/playstate/rewind_4.gif"
#define GIF_PLAYSTATE_RW8_PATH SYS_IMG_PATH_ROOT"/playstate/rewind_8.gif"
#define GIF_PLAYSTATE_RW16_PATH SYS_IMG_PATH_ROOT"/playstate/rewind_16.gif"
#define GIF_PLAYSTATE_RW32_PATH SYS_IMG_PATH_ROOT"/playstate/rewind_32.gif"
#define GIF_PLAYSTATE_RW2BEGIN_PATH SYS_IMG_PATH_ROOT"/playstate/rewind_2_begin.gif"




extern "C" cairo_font_face_t *cairo_ft_font_face_create_for_ft_face(void *face, int flags);
#if defined(hi3560e)
extern "C" int yx_drv_display_layerOrder_set(int order);
#endif


#if(SUPPORTE_HD)
static Hippo::WidgetSource gWaitClockSource   = {Hippo::StandardScreen::S720, 128, 598, 50, 50, 0, 0, 0}; //HD
#else
static Hippo::WidgetSource gWaitClockSource   = {Hippo::StandardScreen::S576, 64, 427, 50, 50, 0, 0, 0}; //SD
#endif
static Hippo::WidgetSource gPlayStateSource   = {Hippo::StandardScreen::S576, 600, 40, 69, 36, 0, 0, 0};
static Hippo::WidgetSource gChannelNOSource   = {Hippo::StandardScreen::S576, 100, 100, 200, 200, 0, 0, 0};
static Hippo::WidgetSource gAudioMuteSource   = {Hippo::StandardScreen::S576, 200, 500, 42, 36, 0, 0, 0};
static Hippo::WidgetSource gAudioTrackSource  = {Hippo::StandardScreen::S576, 158, 500, 42, 36, 0, 0, 0};
static Hippo::WidgetSource gAudioVolumeSource = {Hippo::StandardScreen::S576, 242, 500, 320,  36, 0, 0, 0};

static Hippo::WidgetSource gProgressBarSource = {Hippo::StandardScreen::S576, 40, 360, 640, 79, 0, (void *)GIF_PROGRESS_BG_PATH, 0};
static Hippo::WidgetSource gDolbySource = {Hippo::StandardScreen::S576, 30, 490, 71, 56, 0, (void *)PNG_DOLBY_DIGITALPLUS_PATH, 0};
static Hippo::WidgetSource gDolbyDownmixSource = {Hippo::StandardScreen::S576, 101, 518, 35, 28, 0, (void *)PNG_DOLBY_DOWNMIX_PATH, 0};


#if defined(Sichuan)
static Hippo::WidgetSource gLogoSource = {Hippo::StandardScreen::S576, (720-640)/2, (576-530)/2, 640, 530, 0, NULL, 0};
#else
static Hippo::WidgetSource gLogoSource = {Hippo::StandardScreen::S576, 0, 0, 720, 576, 0, NULL, 0};
#endif

static Hippo::LogoWidget *gLogoShow = NULL;


static void
createWidgets()
{
	int pageWidth, pageHeight;

	sysSettingGetInt("pagewidth", &pageWidth, 0);
	sysSettingGetInt("pageheight", &pageHeight, 0);

    /* 创建在下层显示的部件 */
    gLogoShow = new Hippo::LogoWidget(&gLogoSource);

    /* 创建在中层显示的部件 */
#if !(defined(GRAPHIC_SINGLE_LAYER) || defined(BROWSER_INDEPENDENCE))
    Hippo::epgBrowserAgent().setView(new Hippo::BrowserView(middleLayer));
#else
    Hippo::epgBrowserAgent().setView(new Hippo::BrowserViewTakin(middleLayer));
#endif
    Hippo::epgBrowserAgent().mView->setContentSize(pageWidth, pageHeight);
    Hippo::epgBrowserAgent().mView->setVisibleP(true);

    /* 创建在上层显示的部件 */
    Hippo::UltraPlayer::mAudioVolume = new Hippo::AudioVolumeWidget(&gAudioVolumeSource);


#ifndef HUAWEI_C20
    Hippo::UltraPlayer::mDolbyIcon = new Hippo::DolbyWidget(&gDolbySource);
    Hippo::UltraPlayer::mDolbyDownmixIcon = new Hippo::DolbyDownmixWidget(&gDolbyDownmixSource);
#endif


    Hippo::UltraPlayer::mAudioMute = new Hippo::AudioMuteWidget(&gAudioMuteSource);
    Hippo::UltraPlayer::mAudioMute->setStateImage(Hippo::AudioMuteWidget::StateMute, (void *)GIF_AUDIO_MUTE_ON_PATH, 0);
    Hippo::UltraPlayer::mAudioMute->setStateImage(Hippo::AudioMuteWidget::StateUnmute, (void *)GIF_AUDIO_MUTE_OFF_PATH, 0);

    Hippo::UltraPlayer::mAudioTrack = new Hippo::AudioTrackWidget(&gAudioTrackSource);
    Hippo::UltraPlayer::mAudioTrack->setStateImage(Hippo::AudioTrackWidget::StateLR, (void *)GIF_AUDIO_TRACK_LR_PATH, 0);
    Hippo::UltraPlayer::mAudioTrack->setStateImage(Hippo::AudioTrackWidget::StateLL, (void *)GIF_AUDIO_TRACK_LL_PATH, 0);
    Hippo::UltraPlayer::mAudioTrack->setStateImage(Hippo::AudioTrackWidget::StateRR, (void *)GIF_AUDIO_TRACK_RR_PATH, 0);
    Hippo::UltraPlayer::mAudioTrack->setStateImage(Hippo::AudioTrackWidget::StateRL, (void *)GIF_AUDIO_TRACK_RL_PATH, 0);

    Hippo::UltraPlayer::mPlayState = new Hippo::PlayStateWidget(&gPlayStateSource);
    Hippo::UltraPlayer::mPlayState->setStateImage(Hippo::PlayStateWidget::StateLive, (void *)GIF_PLAYSTATE_LIVE_PATH, 0);
    Hippo::UltraPlayer::mPlayState->setStateImage(Hippo::PlayStateWidget::StateTimeShift, (void *)GIF_PLAYSTATE_WATCH_PATH, 0);
    Hippo::UltraPlayer::mPlayState->setStateImage(Hippo::PlayStateWidget::StatePlay, (void *)GIF_PLAYSTATE_PLAY_PATH, 0);
    Hippo::UltraPlayer::mPlayState->setStateImage(Hippo::PlayStateWidget::StatePause, (void *)GIF_PLAYSTATE_PAUSE_PATH, 0);
	
    Hippo::UltraPlayer::mPlayState->setStateImage(Hippo::PlayStateWidget::StateFW2, (void *)GIF_PLAYSTATE_FW2_PATH, 0);
    Hippo::UltraPlayer::mPlayState->setStateImage(Hippo::PlayStateWidget::StateFW4, (void *)GIF_PLAYSTATE_FW4_PATH, 0);
    Hippo::UltraPlayer::mPlayState->setStateImage(Hippo::PlayStateWidget::StateFW8, (void *)GIF_PLAYSTATE_FW8_PATH, 0);
    Hippo::UltraPlayer::mPlayState->setStateImage(Hippo::PlayStateWidget::StateFW16, (void *)GIF_PLAYSTATE_FW16_PATH, 0);
    Hippo::UltraPlayer::mPlayState->setStateImage(Hippo::PlayStateWidget::StateFW32, (void *)GIF_PLAYSTATE_FW32_PATH, 0);
    Hippo::UltraPlayer::mPlayState->setStateImage(Hippo::PlayStateWidget::StateNext, (void *)GIF_PLAYSTATE_FWEND_PATH, 0);
	
    Hippo::UltraPlayer::mPlayState->setStateImage(Hippo::PlayStateWidget::StateRW2, (void *)GIF_PLAYSTATE_RW2_PATH, 0);
    Hippo::UltraPlayer::mPlayState->setStateImage(Hippo::PlayStateWidget::StateRW4, (void *)GIF_PLAYSTATE_RW4_PATH, 0);
    Hippo::UltraPlayer::mPlayState->setStateImage(Hippo::PlayStateWidget::StateRW8, (void *)GIF_PLAYSTATE_RW8_PATH, 0);
    Hippo::UltraPlayer::mPlayState->setStateImage(Hippo::PlayStateWidget::StateRW16, (void *)GIF_PLAYSTATE_RW16_PATH, 0);
    Hippo::UltraPlayer::mPlayState->setStateImage(Hippo::PlayStateWidget::StateRW32, (void *)GIF_PLAYSTATE_RW32_PATH, 0);
    Hippo::UltraPlayer::mPlayState->setStateImage(Hippo::PlayStateWidget::StatePrevious, (void *)GIF_PLAYSTATE_RW2BEGIN_PATH, 0);
	
    Hippo::UltraPlayer::mPlayState->setStateImage(Hippo::PlayStateWidget::StateStop, (void *)GIF_PLAYSTATE_STOP_PATH, 0);

    Hippo::UltraPlayer::mProgressBar = new Hippo::ProgressBarWidget(&gProgressBarSource);

    Hippo::BrowserAgent::mWaitClock = new Hippo::PicSetWidget(&gWaitClockSource, 15);
    Hippo::BrowserAgent::mWaitClock->insert(0 , (void *)GIF_WAITCLOCK_1_PATH , 0);
    Hippo::BrowserAgent::mWaitClock->insert(1 , (void *)GIF_WAITCLOCK_2_PATH , 0);
    Hippo::BrowserAgent::mWaitClock->insert(2 , (void *)GIF_WAITCLOCK_3_PATH , 0);
    Hippo::BrowserAgent::mWaitClock->insert(3 , (void *)GIF_WAITCLOCK_4_PATH , 0);
    Hippo::BrowserAgent::mWaitClock->insert(4 , (void *)GIF_WAITCLOCK_5_PATH , 0);
    Hippo::BrowserAgent::mWaitClock->insert(5 , (void *)GIF_WAITCLOCK_6_PATH , 0);
    Hippo::BrowserAgent::mWaitClock->insert(6 , (void *)GIF_WAITCLOCK_7_PATH , 0);
    Hippo::BrowserAgent::mWaitClock->insert(7 , (void *)GIF_WAITCLOCK_8_PATH , 0);
    Hippo::BrowserAgent::mWaitClock->insert(8 , (void *)GIF_WAITCLOCK_9_PATH , 0);
    Hippo::BrowserAgent::mWaitClock->insert(9 , (void *)GIF_WAITCLOCK_10_PATH, 0);
    Hippo::BrowserAgent::mWaitClock->insert(10, (void *)GIF_WAITCLOCK_11_PATH, 0);
    Hippo::BrowserAgent::mWaitClock->insert(11, (void *)GIF_WAITCLOCK_12_PATH, 0);
    Hippo::BrowserAgent::mWaitClock->insert(12, (void *)GIF_WAITCLOCK_13_PATH, 0);
    Hippo::BrowserAgent::mWaitClock->insert(13, (void *)GIF_WAITCLOCK_14_PATH, 0);
    Hippo::BrowserAgent::mWaitClock->insert(14, (void *)GIF_WAITCLOCK_15_PATH, 0);
    Hippo::BrowserAgent::mWaitClock->insert(15, (void *)GIF_WAITCLOCK_16_PATH, 0);


    Hippo::BrowserAgent::mPrompt = new Hippo::PromptWidget();
}

extern "C" int
GraphicsConfig()
{
    Hippo::StandardScreen *layer;
    Hippo::StandardViewGroup *group;
    cairo_surface_t *surface;
    cairo_t *cr;
    unsigned char *buffer;
    int pitch;
    bool isSoftLayer = false;
	int pageWidth, pageHeight;
    static FT_Library gLibrary;
    FT_Face     fontFace;

    // TODO: Fix path.
    // #ifdef ANDROID
    // #endif
    FT_Init_FreeType(&gLibrary);
    FT_Error ret = FT_New_Face(gLibrary, "/var/MicroHei.ttf",  0, &fontFace);
    if (ret != 0) {
        ret = FT_New_Face(gLibrary, "/root/MicroHei.ttf",  0, &fontFace);
    }
    if (ret != 0) {
        ret = FT_New_Face(gLibrary, CONFIG_FILE_DIR"/MicroHei.ttf",  0, &fontFace);
    }
    if (ret != 0) {
        LogUserOperError("***** Error ***** MicroHei.ttf is missing.\n");
        LogUserOperError("Will abort().\n");
        abort();
    }

    cairo_font_face_t *face = cairo_ft_font_face_create_for_ft_face(fontFace, 0);

#if defined(GRAPHIC_SINGLE_LAYER)

    /* 创建层 */
    if (ygp_layer_createGraphics(SCREEN_MAX_WIDTH, SCREEN_MAX_HEIGHT, 0, &middleLayer))
        return -1;
    ygp_layer_setZorder(middleLayer, 105);
    ygp_layer_setShow(middleLayer, 1);

    ygp_layer_getMemory(middleLayer, (int *)&buffer, &pitch);

    surface = cairo_image_surface_create_for_data(buffer, CAIRO_FORMAT_ARGB32, SCREEN_MAX_WIDTH, SCREEN_MAX_HEIGHT, pitch);
    cr = cairo_create(surface);
    cairo_set_font_face(cr, face);

    Hippo::systemManager().mixer().mCanvas = new Hippo::Canvas(cr);

    layer = new Hippo::StandardScreen(Hippo::StandardScreen::S720, Hippo::systemManager().mixer().mCanvas);
    layer->mPlatformLayer = middleLayer;
    layer->setCanvasMaxSize(SCREEN_MAX_WIDTH, SCREEN_MAX_HEIGHT);
    layer->setCanvasValidSize(SCREEN_MAX_WIDTH, SCREEN_MAX_HEIGHT, false);
    layer->calculateCanvasOffset();
    layer->setVisibleP(true);

    /* 下 */
    Hippo::OSDLayer *bottomOSDLayer = new Hippo::OSDLayer(Hippo::StandardScreen::S720);
    bottomOSDLayer->mPlatformLayer = middleLayer;
    bottomOSDLayer->setCanvasMaxSize(SCREEN_MAX_WIDTH, SCREEN_MAX_HEIGHT);
    bottomOSDLayer->setCanvasValidSize(SCREEN_MAX_WIDTH, SCREEN_MAX_HEIGHT, false);
    bottomOSDLayer->calculateCanvasOffset();
    bottomOSDLayer->setVisibleP(true);

    Hippo::systemManager().mixer().setBottomLayer(bottomOSDLayer);

    /* 中 */
    Hippo::BrowserLayerZebra *browserLayer;
    browserLayer = new Hippo::BrowserLayerZebra(Hippo::StandardScreen::S720);
    browserLayer->mPlatformLayer = middleLayer;
    browserLayer->setCanvasMaxSize(SCREEN_MAX_WIDTH, SCREEN_MAX_HEIGHT);
    browserLayer->setCanvasValidSize(SCREEN_MAX_WIDTH, SCREEN_MAX_HEIGHT, false);
    browserLayer->calculateCanvasOffset();
    browserLayer->setVisibleP(true);

    Hippo::systemManager().mixer().setMiddleLayer(browserLayer);

    /* 上 */
    Hippo::OSDLayer *topOSDLayer = new Hippo::OSDLayer(Hippo::StandardScreen::S720);
    topOSDLayer->mPlatformLayer = middleLayer;
    topOSDLayer->setCanvasMaxSize(SCREEN_MAX_WIDTH, SCREEN_MAX_HEIGHT);
    topOSDLayer->setCanvasValidSize(SCREEN_MAX_WIDTH, SCREEN_MAX_HEIGHT, false);
    topOSDLayer->calculateCanvasOffset();
    topOSDLayer->setVisibleP(true);

    Hippo::systemManager().mixer().setTopLayer(topOSDLayer);

#else /*GRAPHIC_SINGLE_LAYER*/

#if defined(hi3560e)
    yx_drv_display_layerOrder_set(3102); // 把zebra layer层显示在浏览器层之上
#endif

    /* 创建最下层 */
    if (ygp_layer_createGraphics(SCREEN_MAX_WIDTH, SCREEN_MAX_HEIGHT, 0, &bottomLayer))
        return -1;
	ygp_layer_setZorder(bottomLayer, 104);
	ygp_layer_setShow(bottomLayer, 1);

    ygp_layer_getMemory(bottomLayer, (int *)&buffer, &pitch);
    /*debug*/ //memset(buffer, 0xcf, SCREEN_MAX_WIDTH * SCREEN_MAX_HEIGHT * 4);
    surface = cairo_image_surface_create_for_data(buffer, CAIRO_FORMAT_ARGB32, SCREEN_MAX_WIDTH, SCREEN_MAX_HEIGHT, pitch);
    cr = cairo_create(surface);
    cairo_set_font_face(cr, face);

    layer = new Hippo::StandardScreen(Hippo::StandardScreen::S720, new Hippo::Canvas(cr));
    layer->mPlatformLayer = bottomLayer;
    layer->mSoftLayer = true;
    layer->setCanvasMaxSize(SCREEN_MAX_WIDTH, SCREEN_MAX_HEIGHT);
    layer->setCanvasValidSize(SCREEN_MAX_WIDTH, SCREEN_MAX_HEIGHT, false);
    layer->calculateCanvasOffset();
    layer->setVisibleP(true);

    Hippo::systemManager().mixer().setBottomLayer(layer);


    /* 创建最中层 */
#if defined(hi3560e)
    if (ygp_HWLayer_CreateGraphic(1, SCREEN_MAX_WIDTH, SCREEN_MAX_HEIGHT, 0, &middleLayer))
        return -1;

    ygp_HWLayer_GetMemory(middleLayer, (int *)&buffer, &pitch);

    isSoftLayer = false;
#else
    if (ygp_layer_createGraphics(SCREEN_MAX_WIDTH, SCREEN_MAX_HEIGHT, 0, &middleLayer))
        return -1;
    ygp_layer_setZorder(middleLayer, 105);
    ygp_layer_setShow(middleLayer, 1);

    ygp_layer_getMemory(middleLayer, (int *)&buffer, &pitch);

    isSoftLayer = true;
#endif

    /*debug*/ //memset(buffer, 0xcf, SCREEN_MAX_WIDTH * SCREEN_MAX_HEIGHT * 4);
    surface = cairo_image_surface_create_for_data(buffer, CAIRO_FORMAT_ARGB32, SCREEN_MAX_WIDTH, SCREEN_MAX_HEIGHT, pitch);
    cr = cairo_create(surface);
    cairo_set_font_face(cr, face);

    layer = new Hippo::BrowserLayer(Hippo::StandardScreen::S720, new Hippo::Canvas(cr));
    layer->mPlatformLayer = middleLayer;
    layer->mSoftLayer = isSoftLayer;
    layer->setCanvasMaxSize(SCREEN_MAX_WIDTH, SCREEN_MAX_HEIGHT);
    layer->setCanvasValidSize(SCREEN_MAX_WIDTH, SCREEN_MAX_HEIGHT, false);
    layer->calculateCanvasOffset();
    layer->setVisibleP(true);

    Hippo::systemManager().mixer().setMiddleLayer(layer);


    /* 创建最上层 */
    if (ygp_layer_createGraphics(SCREEN_MAX_WIDTH, SCREEN_MAX_HEIGHT, 0, &topLayer))
        return -1;
	ygp_layer_setZorder(topLayer, 210);
	ygp_layer_setShow(topLayer, 1);

    ygp_layer_getMemory(topLayer, (int *)&buffer, &pitch);
    /*debug*/ //memset(buffer, 0x8f, SCREEN_MAX_WIDTH * SCREEN_MAX_HEIGHT * 4);
    surface = cairo_image_surface_create_for_data(buffer, CAIRO_FORMAT_ARGB32, SCREEN_MAX_WIDTH, SCREEN_MAX_HEIGHT, pitch);
    cr = cairo_create(surface);
    cairo_set_font_face(cr, face);

    layer = new Hippo::StandardScreen(Hippo::StandardScreen::S720, new Hippo::Canvas(cr));
    layer->mPlatformLayer = topLayer;
    layer->mSoftLayer = true;
    layer->setCanvasMaxSize(SCREEN_MAX_WIDTH, SCREEN_MAX_HEIGHT);
    layer->setCanvasValidSize(SCREEN_MAX_WIDTH, SCREEN_MAX_HEIGHT, false);
    layer->calculateCanvasOffset();
    layer->setVisibleP(true);

    Hippo::systemManager().mixer().setTopLayer(layer);

#endif /*GRAPHIC_SINGLE_LAYER*/


    /* 创建小部件 */
    createWidgets();

    Hippo::LayerMixerDevice::Layer* parentLayer;
    /* 挂接在下层显示的部件 */
    parentLayer = Hippo::systemManager().mixer().bottomLayer();
    parentLayer->attachChildToFront(gLogoShow);

    /* 挂接在中层显示的部件 */
    parentLayer = Hippo::systemManager().mixer().middleLayer();
    parentLayer->attachChildToFront(Hippo::epgBrowserAgent().mView);

    /* 挂接在上层显示的部件 */
    parentLayer = Hippo::systemManager().mixer().topLayer();
    parentLayer->attachChildToFront(Hippo::UltraPlayer::mAudioVolume);
    parentLayer->attachChildToFront(Hippo::UltraPlayer::mAudioMute);
    parentLayer->attachChildToFront(Hippo::UltraPlayer::mAudioTrack);
    parentLayer->attachChildToFront(Hippo::UltraPlayer::mPlayState);
    parentLayer->attachChildToFront(Hippo::UltraPlayer::mProgressBar);
    parentLayer->attachChildToFront(Hippo::BrowserAgent::mWaitClock);
    parentLayer->attachChildToFront(Hippo::UltraPlayer::mDolbyIcon);
    parentLayer->attachChildToFront(Hippo::UltraPlayer::mDolbyDownmixIcon);
#if defined(GRAPHIC_SINGLE_LAYER)
    Hippo::systemManager().mixer().setStandard(Hippo::LayerMixerDevice::Layer::closeStandard(SCREEN_MAX_WIDTH, SCREEN_MAX_HEIGHT));
#else /*GRAPHIC_SINGLE_LAYER*/
	sysSettingGetInt("pagewidth", &pageWidth, 0);
	sysSettingGetInt("pageheight", &pageHeight, 0);
    Hippo::systemManager().mixer().setStandard(Hippo::LayerMixerDevice::Layer::closeStandard(pageWidth, pageHeight));
#endif /*GRAPHIC_SINGLE_LAYER*/

    return 0;
}

extern "C"
int LogoShow(int show, unsigned char *image, int imageLen)
{
    if (gLogoShow) {
        if (show) {
            gLogoShow->setLogo(image, imageLen);
            gLogoShow->setVisibleP(true);
        }
        else {
            gLogoShow->setVisibleP(false);
        }
        return 0;
    }
    else{
        return -1;
    }
}

extern "C"
int GraphicsRelease()
{
#if defined(GRAPHIC_SINGLE_LAYER)
    if(middleLayer){
        ygp_layer_destroyGraphics(middleLayer);
        middleLayer=0;
	}
#else
	if(topLayer){
     ygp_layer_destroyGraphics(topLayer);
     topLayer=0;
    }

    if(middleLayer){
     ygp_layer_destroyGraphics(middleLayer);
     middleLayer=0;
    }

    if(bottomLayer){
     ygp_layer_destroyGraphics(bottomLayer);
     bottomLayer=0;
    }
#endif
    return 0;
}


