
#include "UltraPlayerBGMusic.h"
#include "UltraPlayerAssertions.h"

#include "NativeHandler.h"

#include "AppSetting.h"
#include "SysSetting.h"
#include "config/pathConfig.h"

#include <ctype.h>
#include <stdio.h>
#include <unistd.h>

#include "app_heartbit.h"
#include "mid/mid_timer.h"
#include "Assertions.h"
#include "mid_stream.h"

extern char* global_cookies;

namespace Hippo {

#define BGM_CONFIG_FILE CONFIG_FILE_DIR"/.bgmconfig"

#define	BGM_URL_MAX			1024

typedef struct{
    int     bgm_init;
	short 	bgm_valid;
	char 	bgm_url[BGM_URL_MAX];
	int		bgm_status;
} BGM_INFO;

enum{
	//BGM_STATUS_CONFLICT_OTHER_PLAY = -1,
	//BGM_STATUS_NOTREADY = 0,
	//BGM_STATUS_IDLE,
	BGM_STATUS_PLAYING = 0,
	BGM_STATUS_STOP
};

static BGM_INFO bgm_info = {0, 0, {0}, 0};

static void stateCall(int pIndex, STRM_STATE state, int rate, unsigned int magic, int callarg)
{
}

static void msgCall(int pIndex, STRM_MSG msg, int arg, unsigned int magic, int callarg)
{
}

static int getBGMusicEnable()
{
    int valid_read = -1;
    FILE *bgm_fd = NULL;

    appSettingGetInt("BGMusicEnable", &valid_read, 0);

    if(-1 == valid_read){
        bgm_fd = fopen(BGM_CONFIG_FILE, "r");
    	if(bgm_fd){
    		if(fread((char *)(&valid_read), 1, sizeof(int), bgm_fd) != sizeof(int)){
    			valid_read = -1;
    		}
            fclose(bgm_fd);

            if(valid_read == -1)
                valid_read = 1;
            appSettingSetInt("BGMusicEnable", valid_read);
        }
    }

    return valid_read;
}

static void setBGMusicEnable(int value)
{
    appSettingSetInt("BGMusicEnable", value);

    if(access(BGM_CONFIG_FILE, F_OK) == 0){
        FILE *bgm_fd = NULL;

        bgm_fd = fopen(BGM_CONFIG_FILE, "w");
        if(bgm_fd){
        	fwrite((char *)(&value), 1, sizeof(int), bgm_fd);
        	fclose(bgm_fd);
        }
    }

    return;
}

static void bgm_init()
{
	int valid_read = -1;

	if(bgm_info.bgm_init == 1){
	    return ;
	}

    valid_read = getBGMusicEnable();
	bgm_info.bgm_valid = valid_read;

	memset(bgm_info.bgm_url, 0, BGM_URL_MAX);
	bgm_info.bgm_status = BGM_STATUS_STOP;
	bgm_info.bgm_init = 1;
	return;
}

static void bgm_music_play(int arg)
{
	PLAYER_LOG("Background music play current state(%d), url(%s), upgrade state(%d)\n", bgm_info.bgm_status, bgm_info.bgm_url, defNativeHandler().getState());
	if(bgm_info.bgm_status == BGM_STATUS_PLAYING || !isprint(bgm_info.bgm_url[0])) {
		return;
	}
	if(bgm_info.bgm_url[0] != 0 && NativeHandler::Running == defNativeHandler().getState()
        ){
		char buf[256] = {0};

		if(global_cookies){
			snprintf(buf, 128, "%s", global_cookies);
			PLAYER_LOG("Background music cookie buff = %s\n",buf);
		}
		mid_stream_hmpa_cookie(0, buf);
		PLAYER_LOG("BackGround Music Start play !\n");
		mid_stream_open(0, bgm_info.bgm_url, APP_TYPE_HTTP_MPA, 1);
		mid_stream_set_call(0, stateCall, msgCall, (int)0);
		bgm_info.bgm_status = BGM_STATUS_PLAYING;
	}
	return;
}

UltraPlayerBGMusic::UltraPlayerBGMusic(UltraPlayerClient *client, BrowserPlayerReporter *pReporter, Program *pProgram, int delay)
	: UltraPlayer(client, pReporter, pProgram)
	, mDelay(delay)
{
    bgm_init();
}

UltraPlayerBGMusic::~UltraPlayerBGMusic()
{
	PLAYER_LOG("Brackground music stop from current status(%d)\n", bgm_info.bgm_status);
	mid_timer_delete(bgm_music_play, 0);
	if(bgm_info.bgm_status == BGM_STATUS_PLAYING){
		mid_stream_close(0, 1);
		bgm_info.bgm_status = BGM_STATUS_STOP;
	}
	return ;
}

int
UltraPlayerBGMusic::play(unsigned int)
{
	PLAYER_LOG("Background music on-off(%d), url(%s)\n", bgm_info.bgm_valid, bgm_info.bgm_url);
	if(bgm_info.bgm_valid){
    	mid_timer_delete(bgm_music_play, 0);
    	if(mDelay){
    		mid_timer_create(mDelay, 1, bgm_music_play, 0);
    		mMediaType = APP_TYPE_HTTP_MPA;
    	}else{
    	    mMediaType = APP_TYPE_HTTP_MPA;
    		bgm_music_play(0);
    	}
    }
	return 0;
}

int
UltraPlayerBGMusic::stop()
{
	PLAYER_LOG("Brackground music stop from current status(%d)\n", bgm_info.bgm_status);
	mid_timer_delete(bgm_music_play, 0);
	if(bgm_info.bgm_status == BGM_STATUS_PLAYING){
		mid_stream_close(0, 1);
		bgm_info.bgm_status = BGM_STATUS_STOP;
	}
	return 0;
}

int
UltraPlayerBGMusic::close(int mode)
{
	PLAYER_LOG("Brackground music close from current status(%d)\n", bgm_info.bgm_status);
	mid_timer_delete(bgm_music_play, 0);
    UltraPlayer::ClearAllIcon();
	if(bgm_info.bgm_status == BGM_STATUS_PLAYING){
		mid_stream_close(0, 1);
		bgm_info.bgm_status = BGM_STATUS_STOP;
	}
	return 0;
}

int
UltraPlayerBGMusic::setUrl(const char *url)
{
	char full_url_start[] = "http://";
	char epg_url[1024] = {0};
	char *p = NULL, *q = NULL;

	if(bgm_info.bgm_init == 0){
		bgm_init();
	}
	if(!url){
		bgm_info.bgm_url[0] = 0;
		PLAYER_LOG_ERROR("Background music url is NULL !!\n");
		return -1;
	}
	memset(bgm_info.bgm_url, 0, BGM_URL_MAX);
	//if url is not full
	if(strncmp(url, full_url_start, strlen(full_url_start))){
		PLAYER_LOG("BackGround Music url is not full, url(%s)-<->full url(%s)\n", url, full_url_start);
        appSettingGetString("epg", epg_url, sizeof(epg_url), 0);
        if (epg_url[0] == '\0') {
            PLAYER_LOG_ERROR("No EPGUrl. bgMusic won't start.\n");
            return -1;
        }
		p = epg_url;
		p += strlen(full_url_start);
		q = strchr(p, '/');
		*q = 0;
		PLAYER_LOG("BackGround Music epg set url(%s)\n", epg_url);
		strcpy(bgm_info.bgm_url, epg_url);
		if(url[0] != '/')
			strcat(bgm_info.bgm_url, "/");
		strcat(bgm_info.bgm_url, url);
		PLAYER_LOG("BackGround Music completed url(%s)\n", bgm_info.bgm_url);
	}else{
		strcpy(bgm_info.bgm_url, url);
	}
	return 0;
}

int
UltraPlayerBGMusic::getUrl(char *url)
{
	if(url == NULL){
	    PLAYER_LOG_ERROR("BackGround Music get url input params is NULL !!\n");
		return -1;
	}
	strcpy(url, bgm_info.bgm_url);
	return 0;
}

int
UltraPlayerBGMusic::enable(int pEnable)
{
	bgm_info.bgm_valid = pEnable;

    setBGMusicEnable(pEnable);

	if(bgm_info.bgm_valid){
	    bgm_music_play(0);
	}
	else{
	    mid_timer_delete(bgm_music_play, 0);
    	if(bgm_info.bgm_status == BGM_STATUS_PLAYING){
    		mid_stream_close(0, 1);
    		bgm_info.bgm_status = BGM_STATUS_STOP;
    	}
	}
	return 0;
}

int
UltraPlayerBGMusic::isEnable()
{
	return bgm_info.bgm_valid;
}

void
UltraPlayerBGMusic::handleMessage(Message *msg)
{
    if(msg->what >= MessageType_ConfigSave && msg->what <= MessageType_ClearAllIcon){
        return UltraPlayer::handleMessage(msg);
    }
    return ;
}
} // namespace Hippo

