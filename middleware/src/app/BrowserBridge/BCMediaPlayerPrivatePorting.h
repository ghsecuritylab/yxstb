#ifndef __MEDIA_PLAYER_PRIVATE_PORTING_H__
#define __MEDIA_PLAYER_PRIVATE_PORTING_H__

//初始化一路视频播放器并返回播放器ID号。
int MediaPlayerPrivateInit(void);

//清除一路视频播放器，并释放所有资源。
void MediaPlayerPrivateFinal(int playerID);

//加载一路视频，其url可能是http,https或其它类型。
void MediaPlayerPrivateLoad(int playerID, char* url);

//视频窗口大小或位置改变，设置视频窗口。
void MediaPlayerPrivateSizeChanged(int playerID, int x, int y, int w, int h);

//播放一路视频，播放结束需返回相应的虚拟键值。
void MediaPlayerPrivatePlay(int playerID);

//暂停一路视频。
void MediaPlayerPrivatePause(int playerID);

//得到码流中视频的实际宽度。
int MediaPlayerPrivateWidth(int playerID);

//得到码流中视频的实际高度。
int MediaPlayerPrivateHeight(int playerID);

//得到码流中是否包含视频。
int MediaPlayerPrivateHasVideo(int playerID);

//得到码流中是否包含音频。
int MediaPlayerPrivateHasAudio(int playerID);

//得到视频的总时长，单位为秒。如果得不到视频的部时长，返回inf
//如果当前没有找开的视频或出错，返回0。
float MediaPlayerPrivateDuration(int playerID);

//得到视频已播放的时长，单位为秒。
float MediaPlayerPrivateCurrentTime(int playerID);

//seek，单位为秒。
void MediaPlayerPrivateSeek(int playerID, float pTime);

//判断当前是否正在进行seek，seek返回１，否则返回0。
int MediaPlayerPrivateSeeking(int playerID);

//设置音量，volume范围为0~1。
void MediaPlayerPrivateSetVolume(int playerID, float volume);

//静音，mute为１静音，为0不静音。
void MediaPlayerPrivateSetMute(int playerID, int mute);

//设置播放速度。
void MediaPlayerPrivateSetRate(int playerID, float rate);

//判断平台是否支持某种码流，可能的码流类型如下：。
//H.264 Simple baseline profile video (main and extended video compatible) level 3 and Low-Complexity AAC audio in MP4 container:
//type='video/mp4; codecs="avc1.42E01E, mp4a.40.2"'
// 
//H.264 Extended profile video (baseline-compatible) level 3 and Low-Complexity AAC audio in MP4 container:
//type='video/mp4; codecs="avc1.58A01E, mp4a.40.2"'
// 
//H.264 Main profile video level 3 and Low-Complexity AAC audio in MP4 container
//type='video/mp4; codecs="avc1.4D401E, mp4a.40.2"'
// 
//H.264 ‘High’ profile video (incompatible with main, baseline, or extended profiles) level 3 and Low-Complexity AAC audio in MP4 container
//type='video/mp4; codecs="avc1.64001E, mp4a.40.2"'
// 
//MPEG-4 Visual Simple Profile Level 0 video and Low-Complexity AAC audio in MP4 container
//type='video/mp4; codecs="mp4v.20.8, mp4a.40.2"'
// 
//MPEG-4 Advanced Simple Profile Level 0 video and Low-Complexity AAC audio in MP4 container
//type='video/mp4; codecs="mp4v.20.240, mp4a.40.2"'
// 
//MPEG-4 Visual Simple Profile Level 0 video and AMR audio in 3GPP container
//type='video/3gpp; codecs="mp4v.20.8, samr"'
// 
//Theora video and Vorbis audio in Ogg container
//type='video/ogg; codecs="theora, vorbis"'
// 
//Theora video and Speex audio in Ogg container
//type='video/ogg; codecs="theora, speex"'
// 
//Dirac video and Vorbis audio in Ogg container
//type='video/ogg; codecs="dirac, vorbis"'
// 
//Theora video and Vorbis audio in Matroska container
//type='video/x-matroska; codecs="theora, vorbis"'
// 
//Webm format
//type='video/webm; codecs="vp8, vorbis"'
int MediaPlayerPrivateSupportsType(const char* type, const char* codecs);


//得到平台支持的码流类型，类型定义同上。
void MediaPlayerPrivateGetSupportedTypes(char* supportedTypes);

//得到当前从网络加载码流的状态，状态值定义如下：
//0:Empty.
//1:Idle.
//2:Loading.
//3:Loaded.
//4:FormatError.
//5:NetworkError.
//6:DecodeError.
int MediaPlayerPrivateNetworkState(int playerID);

//得到当前码流的就绪状态，状态值定义如下：
//0:HaveNothing
//1:HaveMetaData
//2:HaveCurrentData
//3:HaveFutureData
//4:HaveEnoughData
int MediaPlayerPrivateReadyState(int playerID);

//得到最大可缓存的数据大小，暂不实现。
float MediaPlayerPrivateMaxTimeBuffered(int playerID);

//得到最大可以seek的时长，暂不实现。
float MediaPlayerPrivateMaxTimeSeekable(int playerID);

//得到最大可加载的数据时长，暂不实现。
float MediaPlayerPrivateMaxTimeLoaded(int playerID);

//得到当前已经下载的字节数。
unsigned int MediaPlayerPrivateBytesLoaded(int playerID);

//得到码流的总字节数。
unsigned int MediaPlayerPrivateTotalBytes(int playerID);

#endif
