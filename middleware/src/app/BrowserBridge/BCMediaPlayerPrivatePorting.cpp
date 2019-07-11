#include <stdio.h>
#include <string.h>
#include "libzebra.h"
#include "VideoPlayerManager.h"
#include "VideoBrowserPlayer.h"

static int videoLayer;
static int networkState;
static int readyState;

using namespace Hippo;
static VideoPlayerManager* playerManager = NULL;

int MediaPlayerPrivateInit(void)
{
    printf("MediaPlayerPrivateInit\n");
    if (!playerManager)
        playerManager = new VideoPlayerManager();
    int playerID = playerManager->createVideoPlayerInstance();
    
    return playerID;
}

void MediaPlayerPrivateFinal(int playerID)
{
    playerManager->releaseVideoPlayerInstanceById(playerID);
}

void MediaPlayerPrivateLoad(int playerID, char* url)
{
    printf("--------- MediaPlayerPrivateLoad playerID = %d, url=%s\n", playerID, url);
    
    VideoBrowserPlayer* player = playerManager->getVideoPlayerInstanceById(playerID);
    player->play(url);

    /*int screenW, screenH;
    ymm_stream_setPreReadBufferSize(8*1024*1024);

    ygp_layer_createVideo(100, 100, &videoLayer);
    ymm_stream_playerStart(0, url, 0);
    ymm_stream_playerSetTrickMode(0, YX_PAUSE, 0);*/
}

void MediaPlayerPrivateSizeChanged(int playerID, int x, int y, int w, int h)
{
    int show=0;
    //ygp_layer_setDisplayPosition(videoLayer, 50, 50, 500, 400);
    ygp_layer_setDisplayPosition(videoLayer, x, y, w, h);
    ygp_layer_getShow(videoLayer, &show);
    if(!show)
    {
        ygp_layer_setZorder(videoLayer, 200);
        ygp_layer_setShow(videoLayer, 1);
        ygp_layer_getShow(videoLayer, &show);
    }
}

void MediaPlayerPrivatePlay(int playerID)
{
    printf("\n\n------------ MediaPlayerPrivatePlay--------\n\n");
    VideoBrowserPlayer* player = playerManager->getVideoPlayerInstanceById(playerID);
    player->resume();
//    ymm_stream_playerSetTrickMode(0, YX_NORMAL_PLAY, 0);
}

void MediaPlayerPrivatePause(int playerID)
{
    printf("------------ MediaPlayerPrivatePause\n");
    VideoBrowserPlayer* player = playerManager->getVideoPlayerInstanceById(playerID);
    player->pause();
 //   ymm_stream_playerSetTrickMode(0, YX_PAUSE, 0);
}

int MediaPlayerPrivateWidth(int playerID)
{
    return 0;
}

int MediaPlayerPrivateHeight(int playerID)
{
    return 0;
}

int MediaPlayerPrivateHasVideo(int playerID)
{
    return 1;
}

int MediaPlayerPrivateHasAudio(int playerID)
{
    return 1;
}

float MediaPlayerPrivateDuration(int playerID)
{
    unsigned int totalTime = 0;
/*    ymm_stream_playerGetTotalTime(0, &totalTime);
    totalTime /= 1000;
  //  printf("totalTime=%d\n", totalTime);
    if(totalTime == 0)
        return -1;*/
        
    VideoBrowserPlayer* player = playerManager->getVideoPlayerInstanceById(playerID);
    totalTime = player->getTotalTime();
    if (totalTime == 0)
        return -1;
             
    return totalTime;
}

float MediaPlayerPrivateCurrentTime(int playerID)
{
    unsigned int currentTime = 0;
   /* ymm_stream_playerGetPlaytime(0, &currentTime);
    currentTime /= 1000;
   // printf("currentTime=%d\n", currentTime);*/
 
    VideoBrowserPlayer* player = playerManager->getVideoPlayerInstanceById(playerID);
    currentTime = player->getCurrentTime();
    return currentTime;
}

unsigned MediaPlayerPrivateBytesLoaded(int playerID)
{
    int bytesLoaded;
/*    YX_Preload_Info preinfo;
    int duration = MediaPlayerPrivateDuration(playerID);
    if(duration &gt; 0)
    {
        ymm_stream_getPrebufferDownloadProgress(&preinfo);
        bytesLoaded = (MediaPlayerPrivateCurrentTime(playerID)+ preinfo.predownloadtime/1000)*1.0f/duration
                      * MediaPlayerPrivateTotalBytes(playerID); 
        printf("bytesLoaded = %d\n", bytesLoaded);
    }
    else
        bytesLoaded = 1;*/
    return bytesLoaded;
}

unsigned MediaPlayerPrivateTotalBytes(int playerID)
{
   /* YX_Preload_Info preinfo;
    ymm_stream_getPrebufferDownloadProgress(&preinfo);
    printf("totalBytes = %d\n", preinfo.totalsize);
    return preinfo.totalsize;*/
    return 0;
}

void MediaPlayerPrivateSeek(int playerID, float pTime)
{    
    return;
}

int MediaPlayerPrivateSeeking(int playerID)
{
    return 0;
}

void MediaPlayerPrivateSetVolume(int playerID, float volume)
{
    printf("volume=%f\n", volume);
    //yhw_aout_setVolume(volume);
    return;
}

void MediaPlayerPrivateSetMute(int playerID, int mute)
{
    yhw_aout_setMute(mute);
}

void MediaPlayerPrivateSetRate(int playerID, float rate)
{
    printf("---------- set rate:%f \n", rate);
    VideoBrowserPlayer* player = playerManager->getVideoPlayerInstanceById(playerID);
   
/*    if(rate == 1)
        ymm_stream_playerSetTrickMode(0, YX_NORMAL_PLAY, 0);
    else if(rate == 2)
        ymm_stream_playerSetTrickMode(0, YX_SLOW, 0);
    else
        ymm_stream_playerSetTrickMode(0, YX_FAST_FORWARD, 0);
*/
}

int MediaPlayerPrivateSupportsType(const char* type, const char* codecs)
{
    printf("-------- SupportsType, type: %s, codecs: %s\n", type, codecs);
    return 0;
}

void MediaPlayerPrivateGetSupportedTypes(char* supportedTypes)
{
    strcpy(supportedTypes, "video/mp4;video/aac");
    return;
}

int MediaPlayerPrivateNetworkState(int playerID)
{
    return -1;
}

int MediaPlayerPrivateReadyState(int playerID)
{
    return -1;
}

