
#ifndef __JVM_PORTING_H__
#define __JVM_PORTING_H__

#include "mid_stream.h"
#include <pthread.h> //for type pthread_t


#define JVM_BOOT_INFO  "/var/jvm_boot_info"
#define JVM_BOOT_INFO_OPP  "/var/jvm_oppsite_boot_info"
#define MAX_JVM_BOOT_INFO_LEN 1024
#define MAX_JVM_STRING_LEN 150
#define MAX_JVM_STRING_COUNT 18
#define AUDIO_FIFO_NAME "/var/audio_fifo"
#define AUDIO_FIFO_CMD  "/var/audio_cmd_fifo"

typedef struct      _PCMFILEHEADER {
    unsigned int        phChunkID;
    unsigned int        phChunkSize;
    unsigned int        phFormat;
    unsigned int        phSubchunk1ID;
    unsigned int        phSubchunk1Size;
    unsigned short      phAudioFormat;
    unsigned short      phChannels;
    unsigned int        phSampleRate;
    unsigned int        phByteRate;
    unsigned short      phBlockAlign;
    unsigned short      phBitsPerSample;
    unsigned int        phSubchunk2ID;
    unsigned int        phSubchunk2Size;
    unsigned char       data[4];
} PCMFILEHEADER, *PPCMFILEHEADER;

typedef enum {
    MUSIC_MODE_BACKGROUND = 0,
    MUSIC_MODE_INSTANT,
} JvmMusicMode;

typedef enum {
    MUSIC_TYPE_UNKNOWN = 0,
    MUSIC_TYPE_WAV,
    MUSIC_TYPE_MP3,
} JvmMusicType;

typedef struct {
    JvmMusicMode mMode;
    JvmMusicType mType;
    int mLoops;
    int mSize;
    int mPlayerHandle;
    int mStatus;
    pthread_t mPlayerThread;
    char *mBuffer;
    struct MixPCM mixpcm;
    int magic;
} JvmMusicInfo;

#ifdef __cplusplus
extern "C" {
#endif

void Jvm_Main_Running(void);
void Jvm_Main_Close(void);
int JVM_audio_open(void);
int JVM_audio_close(void);

#ifdef __cplusplus
}
#endif

#endif // __JVM_PORTING_H__
