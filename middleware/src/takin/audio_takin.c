#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "jvm_porting.h"

#include "ind_mem.h"
#include "TAKIN_event_type.h"

#include "Assertions.h"


static int fd_data = -1;
static int fd_cmd  = -1;
static int audio_thread_run = 0;
static pthread_t audio_thread_handle = 0;
static JvmMusicInfo JvmBackGroundMusic = {0};
static JvmMusicInfo JvmInstantMusic = {0};
static int JvmMusicLoading=0;
static int JvmBackGroundMusicLoading = 0;
static int JvmInstantMusicLoading = 0;

static void *JvmMusicThread(void *arg);
static int JvmMusicLoad(JvmMusicMode pMusicMode);
static int JvmMusicPlay(JvmMusicMode pMusicMode);
static int JvmMusicStop(JvmMusicMode pMusicMode);
static void *BackGroundMusicPushData(void *arg);
static void *InstantMusicPushData(void *arg);

static void *JvmMusicThread(void *arg)
{
    unsigned char buffer[8] = {0};
    int ret = 0;
    fd_set rfds;
    struct timeval tv;
    int tJvmCmd = 0;

    while(audio_thread_run) {
        FD_ZERO(&rfds);
        FD_SET(fd_cmd,  &rfds);
        tv.tv_sec   = 0;
        tv.tv_usec  = 100000;
        ret = select(fd_cmd + 1, &rfds, NULL, NULL, &tv);
        if(ret < 0) {                   // select error
            if(errno != EINTR) {
                perror("select");
                return NULL;
            }
            continue;
        } else if(ret == 0) {           // timeout
            continue;
        }
        if(FD_ISSET(fd_cmd, &rfds)) {
            ret = read(fd_cmd, buffer, 8);
            LogUserOperDebug("### jvm audio cmd 0x%02x %02x %02x %02x %02x %02x %02x %02x\n", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7]);
            if(ret > 0) {
                tJvmCmd = (buffer[0] << 8) + buffer[1];
                LogUserOperDebug("###tJvmCmd=%X\n", tJvmCmd);
                switch(tJvmCmd) {
                case 0x8003: {      //JVM_AUDIO_START:{
                    if(JvmMusicLoading != 0 || JvmInstantMusic.mStatus != 0) {
                        JvmMusicLoading = 0;
                        JvmMusicStop(MUSIC_MODE_INSTANT);
                    }
                    if(JvmInstantMusic.mSize == 0) {
                        JvmInstantMusic.mSize = (buffer[2] << 24) + (buffer[3] << 16) + (buffer[4] << 8) + buffer[5];
                        JvmInstantMusic.mLoops = buffer[6];
                        JvmInstantMusic.mType = buffer[7];
                        JvmInstantMusic.mBuffer = IND_MALLOC(JvmInstantMusic.mSize);
                        IND_MEMSET(JvmInstantMusic.mBuffer, 0, JvmInstantMusic.mSize);
                    }
                    LogUserOperDebug("Jvm Instant music info size %d, loops %d, type %d\n", JvmInstantMusic.mSize, JvmInstantMusic.mLoops, JvmInstantMusic.mType);
                    JvmMusicLoading = 1;
                    JvmMusicLoad(MUSIC_MODE_INSTANT);
                    break;
                }
                case 0x8004: {      //JVM_AUDIO_STOP:{
                	 if(JvmInstantMusicLoading==1){
                    JvmMusicLoading = 0;
                    JvmMusicStop(MUSIC_MODE_INSTANT);
			}
                    break;
                }
                case 0x8005: {      //JVM_BGAUDIO_START:{
                    if(JvmMusicLoading != 0 || JvmBackGroundMusic.mStatus != 0) {
                        JvmMusicLoading = 0;
                        JvmMusicStop(MUSIC_MODE_BACKGROUND);
                    }
                    if(JvmBackGroundMusic.mSize == 0) {
                        JvmBackGroundMusic.mSize = (buffer[2] << 24) + (buffer[3] << 16) + (buffer[4] << 8) + buffer[5];
                        JvmBackGroundMusic.mLoops = buffer[6];
                        JvmBackGroundMusic.mType = buffer[7];
                        JvmBackGroundMusic.mBuffer = IND_MALLOC(JvmBackGroundMusic.mSize);
                        IND_MEMSET(JvmBackGroundMusic.mBuffer, 0, JvmBackGroundMusic.mSize);
                    }
                    JvmMusicLoading = 1;
                    JvmMusicLoad(MUSIC_MODE_BACKGROUND);
                    break;
                }
                case 0x8006: {      //JVM_BGAUDIO_STOP:{
                 if(JvmBackGroundMusicLoading==1){
                    JvmMusicLoading = 0;
                    JvmMusicStop(MUSIC_MODE_BACKGROUND);
                 	}
                    break;
                }
                default: {
                    LogUserOperDebug("Jvm music cmd(0x%x) unknown !\n", tJvmCmd);
                    break;
                }
                }
            } else {
                perror("read");
            }
        }
    }
    return NULL;
}

static int JvmMusicLoad(JvmMusicMode pMusicMode)
{
    int ret = 0, tReadedLen = 0;
    fd_set rfds;
    struct timeval tv;
    JvmMusicInfo *tJvmMusic = NULL;

    if(pMusicMode == MUSIC_MODE_BACKGROUND) {
        tJvmMusic = &JvmBackGroundMusic;
    } else {
        tJvmMusic = &JvmInstantMusic;
    }
    while(JvmMusicLoading) {
        FD_ZERO(&rfds);
        FD_SET(fd_data,  &rfds);
        tv.tv_sec   = 0;
        tv.tv_usec  = 100000;
        ret = select(fd_data + 1, &rfds, NULL, NULL, &tv);
        if(ret < 0) {                   // select error
            if(errno != EINTR) {
                perror("select");
                return -1;
            }
            continue;
        } else if(ret == 0) {           // timeout
            if (tReadedLen >= tJvmMusic->mSize)
                break;
            continue;
        }
        if(FD_ISSET(fd_data, &rfds)) {
            if (tReadedLen >= tJvmMusic->mSize) {
                char buffer[4097];
                ret = read(fd_data, buffer, 4097);
            } else {
                ret = read(fd_data, tJvmMusic->mBuffer + tReadedLen, tJvmMusic->mSize - tReadedLen);
                if(ret > 0) {
                    tReadedLen += ret;
                } else if(ret < 0) {
                    perror("JVM read music data !");
                    return -1;
                }
                LogUserOperDebug("Jvm music data readed %d <-> size %d\n", tReadedLen, tJvmMusic->mSize);
                // if(tReadedLen >= tJvmMusic->mSize) {
                    // break;
                // }
            }
        }
    }
    JvmMusicLoading = 0;
    JvmMusicPlay(pMusicMode);
    return 0;
}

static int JvmMusicPlay(JvmMusicMode pMusicMode)
{
    JvmMusicInfo *tJvmMusic = NULL;

    LogUserOperDebug("pMusicMode=%d\n",pMusicMode);
    LogUserOperDebug("#%s# jvm music mode %d\n", __func__, pMusicMode);
    if(pMusicMode == MUSIC_MODE_BACKGROUND) {
        tJvmMusic = &JvmBackGroundMusic;
        tJvmMusic->mPlayerHandle = 0;
	 JvmBackGroundMusicLoading=1;
    } else {
        tJvmMusic = &JvmInstantMusic;
        tJvmMusic->mPlayerHandle = 1;
	 JvmInstantMusicLoading=1;
    }
    LogUserOperDebug("Jvm music type(%d) mode(%d) status(%d) \n", tJvmMusic->mType, pMusicMode, tJvmMusic->mStatus);
    LogUserOperDebug("Jvm music info size %d, loops %d\n", tJvmMusic->mSize, tJvmMusic->mLoops);
    if(tJvmMusic->mStatus) {
        JvmMusicStop(pMusicMode);
    }
    if(pMusicMode == MUSIC_MODE_BACKGROUND) {
        mid_stream_close(tJvmMusic->mPlayerHandle, 0);
    }
    if(tJvmMusic->mType == MUSIC_TYPE_WAV) {
        if(strncmp(tJvmMusic->mBuffer, "RIFF", 4) == 0) {
            PPCMFILEHEADER  header = NULL;
            header = (PPCMFILEHEADER)tJvmMusic->mBuffer;
            if(header->phBitsPerSample != 8 && header->phBitsPerSample != 16) {
                LogUserOperDebug("#%s#jvm pcm data error!\n", __func__);
                return -1;
            }
            tJvmMusic->mixpcm.sampleRate = header->phSampleRate;
            tJvmMusic->mixpcm.bitWidth = header->phBitsPerSample;
            tJvmMusic->mixpcm.channels = header->phChannels;
            tJvmMusic->magic = mid_stream_open(tJvmMusic->mPlayerHandle, (char*) & (tJvmMusic->mixpcm), APP_TYPE_MIX_PCM, 0);
            mid_stream_sync(tJvmMusic->mPlayerHandle, 1000);
            int len = mid_stream_mix_space(tJvmMusic->mPlayerHandle, tJvmMusic->magic);
            LogUserOperDebug("@ len = %d\n", len);
            tJvmMusic->mSize = tJvmMusic->mSize - sizeof(PCMFILEHEADER) + sizeof(header->data);
            IND_MEMCPY(tJvmMusic->mBuffer, header->data, tJvmMusic->mSize);
        }
    } else if(tJvmMusic->mType == MUSIC_TYPE_MP3) {
		tJvmMusic->magic = mid_stream_open(tJvmMusic->mPlayerHandle, (char*) & (tJvmMusic->mixpcm), APP_TYPE_MIX_MP3, 0);
		mid_stream_sync(tJvmMusic->mPlayerHandle, 1000);
		int len = mid_stream_mix_space(tJvmMusic->mPlayerHandle, tJvmMusic->magic);
		LogUserOperDebug("@ len = %d\n", len);
	}
    tJvmMusic->mStatus = 1;
    if(pMusicMode == MUSIC_MODE_BACKGROUND) {
        if(tJvmMusic->mPlayerThread == 0) {
            LogUserOperDebug("#%s# jvm music mode %d:%p create play thread\n", __func__, pMusicMode, &pMusicMode);
            pthread_create(&(tJvmMusic->mPlayerThread), NULL, BackGroundMusicPushData, (void *)&pMusicMode);
        }
    } else {
        if(tJvmMusic->mPlayerThread == 0) {
            LogUserOperDebug("#%s# jvm music mode %d:%p create play thread\n", __func__, pMusicMode, &pMusicMode);
            pthread_create(&(tJvmMusic->mPlayerThread), NULL, InstantMusicPushData, (void *)&pMusicMode);
        }
    }
    return 0;
}

static int JvmMusicStop(JvmMusicMode pMusicMode)
{
    JvmMusicInfo *tJvmMusic = NULL;

    LogUserOperDebug("pMusicMode=%d\n",pMusicMode);
    if(pMusicMode == MUSIC_MODE_BACKGROUND) {
	 if(JvmBackGroundMusicLoading==0)
	 	return 0;
        tJvmMusic = &JvmBackGroundMusic;
	 JvmBackGroundMusicLoading=0;
    } else {
    	 if(JvmInstantMusicLoading==0)
	 	return 0;
        tJvmMusic = &JvmInstantMusic;
	 JvmInstantMusicLoading=0;
    }
    LogUserOperDebug("#%s# music mode %d, status %d\n", __func__, pMusicMode, tJvmMusic->mStatus);
    if(tJvmMusic->mStatus) {
        tJvmMusic->mStatus = 0;
        pthread_join(tJvmMusic->mPlayerThread, NULL);
    }
    mid_stream_close(tJvmMusic->mPlayerHandle, 0);
    LogUserOperDebug("mPlayerHandle=%d\n", tJvmMusic->mPlayerHandle);
    LogUserOperDebug("#%s# mid_stream_close ok \n", __func__);
    if(!tJvmMusic->mBuffer) {
        IND_FREE(tJvmMusic->mBuffer);
    }
    IND_MEMSET(tJvmMusic, 0, sizeof(JvmMusicInfo));
    LogUserOperDebug("#%s# free buffer ok \n", __func__);
    return 0;
}

static void *BackGroundMusicPushData(void *arg)
{
    char *tPlayBuffer = NULL;
    int tPlayBufferSize = 0;
    int tPushDataSize = 0;
    int tSentDataSize = 0;
    int ret = 0;

    LogUserOperDebug("#%s# Jvm music info size %d, loops %d, type %d, status %d\n", __func__, JvmBackGroundMusic.mSize, JvmBackGroundMusic.mLoops, JvmBackGroundMusic.mType, JvmBackGroundMusic.mStatus);
    while(JvmBackGroundMusic.mStatus) {
        //  ret = ymm_audio_PCMGetBuffer(JvmBackGroundMusic.mPlayerHandle, &tPlayBuffer, &tPlayBufferSize);
        tPlayBufferSize = mid_stream_mix_space(JvmBackGroundMusic.mPlayerHandle, JvmBackGroundMusic.magic);
        if(tPlayBufferSize <= 0) {
            LogUserOperDebug("### Jvm get decoder buffer failure! ret=%d,tPlayBufferSize=%d\n", ret, tPlayBufferSize);
            LogUserOperDebug("#%s# Jvm music info size %d, loops %d, type %d, status %d\n", __func__, JvmBackGroundMusic.mSize, JvmBackGroundMusic.mLoops, JvmBackGroundMusic.mType, JvmBackGroundMusic.mStatus);
            usleep(1000 * 1000);
            continue;
        }
        if((tPlayBufferSize + tPushDataSize) > JvmBackGroundMusic.mSize)
            tSentDataSize = JvmBackGroundMusic.mSize - tPushDataSize;
        else
            tSentDataSize = tPlayBufferSize;
        tPlayBuffer = JvmBackGroundMusic.mBuffer + tPushDataSize;
        mid_stream_mix_push(JvmBackGroundMusic.mPlayerHandle, JvmBackGroundMusic.magic, tPlayBuffer, tSentDataSize);
        tPushDataSize += tSentDataSize;
        if(tPushDataSize >= JvmBackGroundMusic.mSize) {
            LogUserOperDebug("#%s# loops %d tPushDataSize %d, tSentDataSize %d\n", __func__, JvmBackGroundMusic.mLoops, tPushDataSize, tSentDataSize);
            tPushDataSize = 0;
            if(JvmBackGroundMusic.mLoops >= 0) {
                JvmBackGroundMusic.mLoops --;
            }
            if(JvmBackGroundMusic.mLoops == 0) {
                break;
            }
        }
    }
//    JvmMusicStop(MUSIC_MODE_BACKGROUND); //非循环播放增加此接口，会至少声音还没解码完成就关闭了
    return NULL;
}

static void *InstantMusicPushData(void *arg)
{
    char *tPlayBuffer = NULL;
    int tPlayBufferSize = 0;
    int tPushDataSize = 0;
    int tSentDataSize = 0;
    int ret = 0;

    LogUserOperDebug("%s Jvm music info size %d, loops %d, type %d, status %d\n", __func__, JvmInstantMusic.mSize, JvmInstantMusic.mLoops, JvmInstantMusic.mType, JvmInstantMusic.mStatus);
    while(JvmInstantMusic.mStatus) {
        // ret = ymm_audio_PCMGetBuffer(JvmInstantMusic.mPlayerHandle, &tPlayBuffer, &tPlayBufferSize);
        tPlayBufferSize = mid_stream_mix_space(JvmInstantMusic.mPlayerHandle, JvmInstantMusic.magic);
        if(tPlayBufferSize <= 0) {
            LogUserOperDebug("### Jvm get decoder buffer failure! ret=%d,tPlayBufferSize=%d\n", ret, tPlayBufferSize);
            LogUserOperDebug("#%s# Jvm music info size %d, loops %d, type %d, status %d\n", __func__, JvmInstantMusic.mSize, JvmInstantMusic.mLoops, JvmInstantMusic.mType, JvmInstantMusic.mStatus);
            usleep(100 * 1000);
            continue;
        }
        if((tPlayBufferSize + tPushDataSize) > JvmInstantMusic.mSize)
            tSentDataSize = JvmInstantMusic.mSize - tPushDataSize;
        else
            tSentDataSize = tPlayBufferSize;
        tPlayBuffer = JvmInstantMusic.mBuffer + tPushDataSize;
        mid_stream_mix_push(JvmInstantMusic.mPlayerHandle, JvmInstantMusic.magic, tPlayBuffer, tSentDataSize);
        tPushDataSize += tSentDataSize;
        if(tPushDataSize >= JvmInstantMusic.mSize) {
            LogUserOperDebug("#%s# loops %d tPushDataSize %d, tSentDataSize %d\n", __func__, JvmInstantMusic.mLoops, tPushDataSize, tSentDataSize);
            tPushDataSize = 0;
            if(JvmInstantMusic.mLoops >= 0) {
                JvmInstantMusic.mLoops --;
            }
            if(JvmInstantMusic.mLoops == 0) {
                break;
            }
        }
    }
//  JvmMusicStop(MUSIC_MODE_INSTANT);                       //非循环播放增加此接口，会至少声音还没解码完成就关闭了
    return NULL;
}

int JVM_audio_open(void)
{
    umask(0);
    if (mknod(AUDIO_FIFO_NAME, S_IFIFO | 0666, 0))                /*creat a fifo*/
        perror("mknode err\n");
    if (mknod(AUDIO_FIFO_CMD, S_IFIFO | 0666, 0))                 /*creat a fifo*/
        perror("mknode err\n");

    fd_data = open(AUDIO_FIFO_NAME, 0666);
    fd_cmd  = open(AUDIO_FIFO_CMD, 0666);
    if((fd_data == -1) || (fd_cmd == -1)) {
        perror("audio_pipe: create named fifo fault:");
    } else {
        audio_thread_run = 1;
        pthread_create(&audio_thread_handle, NULL, JvmMusicThread, NULL);
    }
    return 0;
}

int JVM_audio_close(void)
{
    audio_thread_run = 0;
    pthread_join(audio_thread_handle, NULL);

    /* 先关掉线程,再关音乐 */
    JvmMusicStop(MUSIC_MODE_BACKGROUND);
    JvmMusicStop(MUSIC_MODE_INSTANT);

    close(fd_data);
    close(fd_cmd);
    unlink(AUDIO_FIFO_NAME);
    unlink(AUDIO_FIFO_CMD);
    fd_data = -1;
    fd_cmd  = -1;
    return 0;
}

