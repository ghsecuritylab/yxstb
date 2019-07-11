#ifndef _ITCPlayerCBridge_H_
#define _ITCPlayerCBridge_H_

/* 为c程序加的一些函数，尽量少用 */

#ifdef __cplusplus
extern "C" {
#endif

int ITCPlayerOpen(int index, char *streamUrl, int type);
int ITCPlayerPlay(int index, unsigned int startTime);
int ITCPlayerSeek(int index, unsigned int playTime);
int ITCPlayerFastForward(int index, int speed);
int ITCPlayerFastRewind(int index, int speed);
int ITCPlayerPause(int index);
int ITCPlayerResume(int index);
int ITCPlayerStop(int index);
int ITCPlayerClose(int index);

#ifdef __cplusplus
}
#endif

#endif // _ITCPlayerCBridge_H_
