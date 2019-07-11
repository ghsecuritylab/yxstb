
#include <iostream>

#include "NativeHandlerRunningC10.h"
#include "NativeHandlerAssertions.h"

#include "Message.h"
#include "SystemManager.h"
#include "Business.h"
#include "Session.h"

#include "UltraPlayer.h"
#include "UltraPlayerWebPage.h"
#include "Hippo_Context.h"
#include "Hippo_MediaPlayer.h"
#include "BrowserAgentTakin.h"
#include "SysSetting.h"

#include "mid_stream.h"
#include "stream_port.h"
#include "codec.h"
#include "SystemManager.h"
#include "UltraPlayer.h"
#include "irkey_tab.h"

#include "KeyDispatcher.h"
#include "ProgressBarDialog.h"
#include "BootImagesShow.h"
#include "app_sys.h"

#include "libzebra.h"

#include "MessageTypes.h"
#include "MessageValueNetwork.h"

extern "C" int TAKIN_browser_editingEnabled(int *handle, int *x, int *y, int *w, int *h);

namespace Hippo {

NativeHandlerRunningC10::NativeHandlerRunningC10()
{
}

NativeHandlerRunningC10::~NativeHandlerRunningC10()
{
}

void
NativeHandlerRunningC10::onActive()
{
    /* 启用键值表功能 */
    if(!mDialog)
        keyDispatcher().setEnabled(true);

    //Network mask set
    DONT_SET(NMB_SHOW_ERROR_CODE);
    DONT_SET(NMB_SHOW_BOTTOM_ICON);
    DONT_SET(NMB_CONNECTOK_DOMENU);
    DONT_SET(NMB_REJOIN_WIRELESS);
}

void
NativeHandlerRunningC10::onUnactive()
{
    /* 关闭键值表功能 */
    keyDispatcher().setEnabled(false);

    //Network mask clear
    DONT_ZERO();
}

bool
NativeHandlerRunningC10::handleMessage(Message *msg)
{
    int speed = stream_port_get_rate();

    if(MessageType_Repaint != msg->what)
        NATIVEHANDLER_LOG("what(0x%x), info(0x%x & 0x%x)\n", msg->what, msg->arg1, msg->arg2);

    if(mDialog)
        return NativeHandlerPublicC10::handleMessage(msg);

	SystemManager &sysManager = systemManager();
    UltraPlayer *player = sysManager.obtainMainPlayer();

#if defined(Jiangsu)
    if(MessageType_Network == msg->what) {
        switch (msg->arg1) {
            case MV_Network_PhysicalDown:
                sendMessageToNativeHandler(MessageType_Network, MV_Network_PhysicalDown_OnOpenError, 0, 0);
                break;
            default:
                break;
        }
    }
#endif

    switch (msg->arg1) {
        case EIS_IRKEY_VOLUME_UP:{
            if(player){
                int tVolume = player->GetVolume( );

        		if(tVolume < AUDIO_VOLUME_MAX){
        		    tVolume += 5;
        		}
        	    player->SetVolume(tVolume);
        	    sysManager.releaseMainPlayer(player);
                return true;
            }
            break;
        }
        case EIS_IRKEY_VOLUME_DOWN:{
            if(player){
                int tVolume = player->GetVolume( );

                if(tVolume > 0){
                    tVolume -= 5;
                }
                player->SetVolume(tVolume);
                sysManager.releaseMainPlayer(player);
                return true;
            }
            break;
        }
        case EIS_IRKEY_VOLUME_MUTE:{
            if(player){
                if(player->GetMute()){
                    player->SetMute(0);
                }
                else{
                    player->SetMute(1);
                }
                sysManager.releaseMainPlayer(player);
                return true;
            }
            break;
        }
        case EIS_IRKEY_TRACK:
        case EIS_IRKEY_AUDIO:
        case EIS_IRKEY_AUDIO_MODE:{
            if(player){
                int tAudioChannel = player->GetChannel();

                tAudioChannel += 1;
                NATIVEHANDLER_LOG("Set audio channel to %d\n", tAudioChannel);
                player->SetChannel(tAudioChannel);
                sysManager.releaseMainPlayer(player);
                return true;
            }
            break;
        }
        case EIS_IRKEY_PLAY: {
            int type;

            if(player) {
                type = player->getMediaType();

                NATIVEHANDLER_LOG("Player type(%d); iptv<->%d,iptv2<->%d,vod<->%d\n", type, APP_TYPE_IPTV, APP_TYPE_IPTV2, APP_TYPE_VOD);
                if(type == APP_TYPE_IPTV || type == APP_TYPE_IPTV2 || type == APP_TYPE_VOD) {
                    int ret = stream_port_get_state();

                    NATIVEHANDLER_LOG("playstate = %d\n", ret);
                    if(ret == STRM_STATE_PLAY || ret == STRM_STATE_IPTV) {
                        player->pause();
                        player->onPause();
                        keyDispatcher().setEnabled(false);
                        if(!mProgressBarDialog) {
                            ProgressBarDialog * dlg = new ProgressBarDialog();
                            mProgressBarDialog = dlg;
                            dlg->setHandler(this);
                            mProgressBarDialog->draw();
                        }
                    } else if(ret == STRM_STATE_PAUSE) {
                        if(mProgressBarDialog) {
                            mProgressBarDialog->Close();
                        }
                        player->resume();
                    } else if(ret == STRM_STATE_FAST) {
                        player->resume();
                        mProgressBarDialog->Close();
                    }
                    systemManager().releaseMainPlayer(player);
                    return true;
                }
            }
            break;
        }
        case EIS_IRKEY_FASTFORWARD: {
            int type;

            if(player) {
                type = player->getMediaType();

               if(type == APP_TYPE_IPTV || type == APP_TYPE_IPTV2 || type == APP_TYPE_VOD) {
                   int ret = stream_port_get_state();

                   NATIVEHANDLER_LOG("playstate = %d\n", ret);
                   if(ret != STRM_STATE_CLOSE) {
                       if(speed <= 0)
                           speed = 1;
                       speed *= 2;
                       if(speed > 32)
                           speed = 2;

                       NATIVEHANDLER_LOG("FastForward speed(%d)\n", speed);
                       player->fastForward(speed);
                       keyDispatcher().setEnabled(false);
                       if(!mProgressBarDialog) {
                           ProgressBarDialog *dlg = new ProgressBarDialog();

                           mProgressBarDialog = dlg;
                           dlg->setHandler(this);
                           mProgressBarDialog->draw();
                       }
                       systemManager().releaseMainPlayer(player);
                       return true;
                   }
               }
            }
            break;
        }
        case EIS_IRKEY_REWIND: {
            int type;

            if(player) {
                type = player->getMediaType();

                if(type == APP_TYPE_IPTV || type == APP_TYPE_IPTV2 || type == APP_TYPE_VOD) {
                    int ret = stream_port_get_state();

                    NATIVEHANDLER_LOG("Current playstate(%d)\n", ret);
                    if(ret != STRM_STATE_CLOSE) {
                        if(speed >= 0)
                            speed = -1;
                        speed *= 2;
                        if(speed < -32)
                            speed = -2;
                        NATIVEHANDLER_LOG("FastRewind speed(%d)\n", speed);
                        player->fastRewind(speed);
                        keyDispatcher().setEnabled(false);
                        NATIVEHANDLER_LOG("FastRewind speed(%d)\n", speed);
                        if(!mProgressBarDialog) {
                            ProgressBarDialog *dlg = new ProgressBarDialog();

                            mProgressBarDialog = dlg;
                            dlg->setHandler(this);
                            mProgressBarDialog->draw();
                        }
                        systemManager().releaseMainPlayer(player);
                        return true;
                    }
                }
            }
            break;
        }
        case EIS_IRKEY_PAGE_UP: {
            int type;

            if(player) {
                type = player->getMediaType();
                if(type == APP_TYPE_IPTV || type == APP_TYPE_IPTV2 || type == APP_TYPE_VOD) {
                    player->SeekToStart();
                    systemManager().releaseMainPlayer(player);
                    return true;
                }
            }
            break;
        }
        case EIS_IRKEY_PAGE_DOWN: {
            int type;

            if(player) {
                type = player->getMediaType();

                if(type == APP_TYPE_IPTV || type == APP_TYPE_IPTV2 || type == APP_TYPE_VOD) {
#ifdef Liaoning
                    player->seekEnd(); // 一键到尾立即结束
#else
                    player->stop(); // 一键到尾播放3秒
#endif
                    systemManager().releaseMainPlayer(player);
                    return true;
                }
            }
            break;
        }
        case EIS_IRKEY_BACK:
        case EIS_IRKEY_STOP: {
            int type;

            if(player) {
                type = player->getMediaType();

                if(type == APP_TYPE_IPTV || type == APP_TYPE_IPTV2) {
                    int ret = stream_port_get_state();

                    NATIVEHANDLER_LOG("playstate = %d,%d\n", ret, type);
                    if (type == APP_TYPE_IPTV || type == APP_TYPE_IPTV2) {
                        if (ret == STRM_STATE_IPTV) {
				 			int changeVideoMode = 0;
	                        sysSettingGetInt("changevideomode", &changeVideoMode, 0);
                            if (!changeVideoMode){            //0 is black screen 1 is last frame 2 is smooth change
                                player->close(UltraPlayer::BlackScreenMode);
                            }
                            else {
                                if (UltraPlayer::getVideoClearFlag()){
                                    player->close(UltraPlayer::BlackScreenMode);
                                    UltraPlayer::setVideoClearFlag(0);
                                }
                                else{
                                    player->close(UltraPlayer::LastFrameMode);
                                }
                            }
                        }
                        else {
#ifdef Liaoning
                            player->seekEnd(); // 一键到尾立即结束
#else
                            player->stop(); // 一键到尾播放3秒
#endif
                        }
                    }
                    else {
						int changeVideoMode = 0;
	                    sysSettingGetInt("changevideomode", &changeVideoMode, 0);
                        if (!changeVideoMode){            //0 is black screen 1 is last frame 2 is smooth change
                            player->close(UltraPlayer::BlackScreenMode);
                        }
                        else {
                            if (UltraPlayer::getVideoClearFlag()){
                                player->close(UltraPlayer::BlackScreenMode);
                               UltraPlayer::setVideoClearFlag(0);
                            }
                            else{
                                player->close(UltraPlayer::LastFrameMode);
                            }
                        }
                    }
                    systemManager().releaseMainPlayer(player);
                    return true;
                }
            }
            break;
        }
        default:{
            break;
		}
    }
    systemManager().releaseMainPlayer(player);

    if (msg->what == MessageType_System && msg->arg1 == HM_CLOSEDIALOG) {
        if(mProgressBarDialog) {
            if (mProgressBarDialog->onClose()) {
                delete mProgressBarDialog;
                mProgressBarDialog = 0;
                keyDispatcher().setEnabled(true);
            }
        }
    }
    if(msg->what == MessageType_KeyDown) {
        if(mProgressBarDialog) {  //5E=
            bool status = mProgressBarDialog->handleMessage(msg);
            if(status)
                return status;
        }
    }
    return NativeHandlerPublicC10::handleMessage(msg);
}

bool
NativeHandlerRunningC10::onPlayPause()
{
    SystemManager &sysManager = systemManager();
    UltraPlayer *player = NULL;

    player = sysManager.obtainMainPlayer();
    if(player){
        player->pause();
        player->onPause();
    }
	sysManager.releaseMainPlayer(player);
    return true;
}

bool
NativeHandlerRunningC10::onLeft(Message *msg)
{
    SystemManager &sysManager = systemManager();
    UltraPlayer *player = NULL;

    player = sysManager.obtainMainPlayer();
    if(player){
        if(1 == business().getKeyCtrl() && STRM_STATE_PAUSE != player->mCurrentStatus)
            msg->arg1 = EIS_IRKEY_VOLUME_DOWN;
    }
	sysManager.releaseMainPlayer(player);
    return false;
}

bool
NativeHandlerRunningC10::onRight(Message *msg)
{
    SystemManager &sysManager = systemManager();
    UltraPlayer *player = NULL;

    player = sysManager.obtainMainPlayer();
    if(player){
        if(1 == business().getKeyCtrl() && STRM_STATE_PAUSE != player->mCurrentStatus)
            msg->arg1 = EIS_IRKEY_VOLUME_DOWN;
    }
	sysManager.releaseMainPlayer(player);
    return false;
}

#if defined (LIAONING_SD)
bool
NativeHandlerRunningC10::doSwitch()//no play openconfig
{
    int keymod = NativeHandlerGetState();
    SystemManager &sysManager = systemManager();
    UltraPlayer *player = sysManager.obtainMainPlayer();
    bool ret = false;

    if(player == NULL){
        webchannelFlagSet(0);
        NativeHandlerPublicC10::doOpenConfigPage();
        ret = true;
    }else if(keymod == 2 && player->mDisplayMode != 0){
        ret = false;
    }else{
        webchannelFlagSet(0);
        NativeHandlerPublicC10::doOpenConfigPage();
        ret = true;
   }
    sysManager.releaseMainPlayer(player);
    return ret;
}
#endif

#ifdef Liaoning
bool
NativeHandlerRunningC10::doBack(){
    SystemManager &sysManager = systemManager();
    UltraPlayer *player = sysManager.obtainMainPlayer();
    int x = 0, y = 0, w = 0, h = 0;
    bool ret = false;

    if(player != NULL){
        if( session().getPlatform() == PLATFORM_ZTE
            && !TAKIN_browser_editingEnabled((int *)(epgBrowserAgentGetHandle()), &x, &y, &w, &h)
            && APP_TYPE_VOD == player->getMediaType()
            && 1 == player->mDisplayMode){
            sendMessageToNativeHandler(MessageType_Unknow, VK_FINAL, 0, 0);
            ret = true;
        }
    }
    sysManager.releaseMainPlayer(player);
    return ret;
}
#endif

bool
NativeHandlerRunningC10::doMenu()
{
#if defined(Jiangsu)
    if(PLATFORM_ZTE == session().getPlatform())
        return false;
#endif
    return NativeHandlerPublicC10::doMenu();
}

} // namespace Hippo
