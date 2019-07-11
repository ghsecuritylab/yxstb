
#include "UpgradeWidget.h"
#include "ViewAssertions.h"
#include "UpgradeManager.h"
#include "LayerMixerDevice.h"
#include "Canvas.h"
#include "SystemManager.h"

#include "UtilityTools.h"
#include "mid_fpanel.h"
#include "libzebra.h"
#include "sys_basic_macro.h"

#include "NetworkFunctions.h"

//upgrade
#define GIF_UPGRADE_UNZIP_PATH             SYS_IMG_PATH_ROOT"/upgrade/unzip.gif"
#define GIF_UPGRADE_UPGRADING_PATH         SYS_IMG_PATH_ROOT"/upgrade/upgrading.gif"
#define GIF_UPGRADE_USB_UPGRADING_PATH     SYS_IMG_PATH_ROOT"/upgrade/usb_upgrading.gif"
#define GIF_UPGRADE_DOWNLOADING_PATH       SYS_IMG_PATH_ROOT"/upgrade/downloading.gif"

//error code
#define GIF_UPGRADE_ERRCODE_10040_PATH     SYS_IMG_PATH_ROOT"/errorcode/upgrade/errorCode_10040.png"
#define GIF_UPGRADE_ERRCODE_10041_PATH     SYS_IMG_PATH_ROOT"/errorcode/upgrade/errorCode_10041.png"
#define GIF_UPGRADE_ERRCODE_10042_PATH     SYS_IMG_PATH_ROOT"/errorcode/upgrade/errorCode_10042.png"

//upgrade error
#define GIF_UPGRADE_UPGRADEERR_PATH        SYS_IMG_PATH_ROOT"/upgrade/upgradeErr_bg.gif"
#define PNG_UPGRADE_UPGRADEERR_1_PATH      SYS_IMG_PATH_ROOT"/upgrade/upgradeErr_unzip.png"
#define PNG_UPGRADE_UPGRADEERR_2_PATH      SYS_IMG_PATH_ROOT"/upgrade/upgradeError_version.png"

namespace Hippo {
static WidgetSource UpgradebgSource= {StandardScreen::S576, 0, 0, 720, 576, 0, (void *)GIF_UPGRADE_UPGRADING_PATH, 0};
static WidgetSource Error1Source = {StandardScreen::S576, 255, 208 , 300, 150, 0, (void *)GIF_UPGRADE_ERRCODE_10040_PATH, 0};
static WidgetSource Error2Source = {StandardScreen::S576, 255, 208 , 300, 150, 0, (void *)GIF_UPGRADE_ERRCODE_10041_PATH, 0};
static WidgetSource Error3Source = {StandardScreen::S576, 255, 208 , 300, 150, 0, (void *)GIF_UPGRADE_ERRCODE_10042_PATH, 0};
static WidgetSource Error4Source = {StandardScreen::S576, 255, 208 , 300, 150, 0, (void *)PNG_UPGRADE_UPGRADEERR_1_PATH, 0};
static WidgetSource Error5Source = {StandardScreen::S576, 255, 208 , 300, 150, 0, (void *)PNG_UPGRADE_UPGRADEERR_2_PATH, 0};
static WidgetSource ErrorBackgroundSource = {StandardScreen::S576, 0, 0, 720, 576, 0, (void *)GIF_UPGRADE_UPGRADEERR_PATH, 0};

UpgradeWidget::UpgradeWidget()
    : Widget(&UpgradebgSource)
    , mProgress(0)
    , mState(0)
    , mReboot(false)
{

}

UpgradeWidget::~UpgradeWidget()
{
}

int
UpgradeWidget::setProgress(int progress)
{
    if (progress < 0)
        return 0;
    mProgress = progress;
    inval(NULL);

    return 0;
}

int
UpgradeWidget::getProgress()
{
    return mProgress;
}

int
UpgradeWidget::setUpgradeState(int state)
{
    Message *message = NULL;

    mState = state;
    mProgress = 0;
    switch (state) {
    case UpgradeManager::UMMI_GET_VERSION_FAILED:
    case UpgradeManager::UMMI_UPGRADE_VERSION_FAILED:
    case UpgradeManager::UMMI_UPGRADE_SAME_VERSION:
    case UpgradeManager::UMMI_UPGRADE_UNZIP_FAILED:
    case UpgradeManager::UMMI_UPGRADE_VERSION_WRONG:
        message = obtainMessage(MC_SHOW_ERROR, state, 0);
        sendMessage(message);
        return 0;
    default:
        break;
    }
    inval(NULL);
    return 0;
}

int
UpgradeWidget::getUpgradeState()
{
    return mState;
}

void
UpgradeWidget::onDraw(Canvas* canvas)
{
    Rect bounds;
    Rect progress;
    Rect rect;
    cairo_text_extents_t extents;
    double x, y;

    char precent[32] = {0};
    char stbIP[64] = {0};
    char messageError[512] = {0};
    char msgBuf[32] = {0};
    char *msgPos = NULL;
    int count = 0;
    int h = 0;
    int w = 0;
    int i = 0;
    unsigned char *imageBackGround = NULL;
    char ifname[URL_LEN] = { 0 };
    char ifaddr[URL_LEN] = { 0 };
    network_default_ifname(ifname, URL_LEN);

    cairo_t* cr = canvas->mCairo;
    getLocalBounds(&bounds);

    switch (mState) {
    case UpgradeManager::UMMI_UPGRADE_SHOW_STBIP:
        sprintf(stbIP, "STB IP : %s", network_address_get(ifname, ifaddr, URL_LEN));
    #ifdef Sichuan
        progress.set((int)84*1280/720, (int)448*720/576, (int)205*1280/720, (int)473*720/576);
        cairo_set_source_rgb(canvas->mCairo, 255, 255, 255);
    #else
        progress.set(84, 448, 205, 473);
        cairo_set_source_rgb(canvas->mCairo, 255, 255, 255);
    #endif
        drawText(canvas, &progress, stbIP);
        return;
    case UpgradeManager::UMMI_UPGRADE_UNZIP:
		strcpy((char *)imageBackGround, GIF_UPGRADE_UNZIP_PATH);
		VIEW_LOG("drawImage [%s] start\n", (unsigned char*)imageBackGround);
		{
			Image img((unsigned char*)imageBackGround);
			drawImage(canvas, &bounds, &img); //draw
		}
        cairo_save(cr);
        cairo_scale(cr, (double)width() / (double)(mSource->w), (double)height() / (double)(mSource->h));
        cairo_set_source_rgba(cr, (double)244/255.0, (double)82/255.0, (double)3/255.0, (double)255/255.0);
        w = 80 + 560 * mProgress / 100;
        rect.set(80, 290, w, 304);
        cairo_rectangle(cr, rect.fLeft, rect.fTop, rect.width(), rect.height());
        cairo_fill(cr);
        cairo_restore(cr);
        sendEmptyMessageDelayed(MC_UZIP_PROGRESS, 500);
        return;
    case UpgradeManager::UMMI_UPGRADE_VERSION_START:
		strcpy((char *)imageBackGround, GIF_UPGRADE_UPGRADING_PATH);
        break;
    case UpgradeManager::UMMI_UPGRADE_UDISK_VERSION:
        removeMessages(MC_UZIP_PROGRESS);
		strcpy((char *)imageBackGround, GIF_UPGRADE_USB_UPGRADING_PATH);
        break;
    case UpgradeManager::UMMI_UPGRADE_VERSION_SUCCEED:
		sendEmptyMessageDelayed(MC_REBOOT, 3000);
		return;
    default:
		strcpy((char *)imageBackGround, GIF_UPGRADE_DOWNLOADING_PATH);
        break;
    }
	VIEW_LOG("drawImage [%s] start\n", (unsigned char*)imageBackGround);
	{
		Image img((unsigned char*)imageBackGround);
		drawImage(canvas, &bounds, &img); //draw
	}
    sprintf(precent, "%d", mProgress);
    precent[strlen(precent)] = '%';
    cairo_save(cr);
    cairo_scale(cr, (double)width() / (double)(mSource->w), (double)height() / (double)(mSource->h));
    cairo_set_source_rgba(cr, (double)244/255.0, (double)82/255.0, (double)3/255.0, (double)255/255.0);
    w = 81 + 558 * mProgress / 100;
    rect.set(81, 478, w, 493);
    cairo_rectangle(cr, rect.fLeft, rect.fTop, rect.width(), rect.height());
    cairo_fill(cr);

    progress.set(84, 448, 105, 473);
    cairo_set_source_rgb(canvas->mCairo, 255, 255, 255);
    drawText(canvas, &progress, precent);
    cairo_restore(cr);

    return ;
}

void
UpgradeWidget::showErrorMessage(int status)
{

    WidgetSource* errorInfo;

    removeMessages(MC_UZIP_PROGRESS);
    switch (status) {
    case UpgradeManager::UMMI_GET_VERSION_FAILED:
        errorInfo = &Error1Source;
        break;
    case UpgradeManager::UMMI_UPGRADE_VERSION_WRONG:
    case UpgradeManager::UMMI_UPGRADE_VERSION_FAILED:
        errorInfo = &Error2Source;
        break;
    case UpgradeManager::UMMI_UPGRADE_UNZIP_FAILED:
        errorInfo = &Error4Source;
        break;
    case UpgradeManager::UMMI_UPGRADE_SAME_VERSION:
        errorInfo = &Error5Source;
        break;
    default:
        errorInfo = &Error3Source;
        break;
    }

    mErrorInfo = new Icon(errorInfo);
    mErrorBackground = new Icon(&ErrorBackgroundSource);
    LayerMixerDevice::Layer* layer = systemManager().mixer().topLayer();

    layer->attachChildToFront(mErrorBackground);
    layer->attachChildToFront(mErrorInfo);

    mErrorBackground->setVisibleP(true);
    mErrorInfo->setVisibleP(true);

    //mErrorInfo->detachFromParent();
    //delete mErrorInfo;
    mReboot = true;
    sendEmptyMessageDelayed(MC_REBOOT, 3000);

}

void
UpgradeWidget::handleMessage(Message* msg)
{
    switch(msg->what) {
    case MC_UZIP_PROGRESS:
        if (mProgress == 100)
            mProgress = 0;
        else
            mProgress += 10;
        setProgress(mProgress);
        break;
    case MC_REBOOT:
#ifdef INCLUDE_LITTLESYSTEM
        if (yhw_upgrade_rebootType() == 1 || mReboot) {
            yhw_board_setRunlevel(1);
            mid_fpanel_reboot();
        } else {
            removeFile(UPGRADE_PATH);
            removeFile("/var/startshell");
            exit(0);
        }
#endif
        break;
    case MC_SHOW_ERROR:
        showErrorMessage(msg->arg1);
        break;
    default:
        break;
    }

    return;
}

} // namespace Hippo


