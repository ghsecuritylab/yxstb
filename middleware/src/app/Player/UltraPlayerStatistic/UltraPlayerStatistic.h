#ifndef _UltraPlayerStatistic_H_
#define _UltraPlayerStatistic_H_

#include <string.h>
#include <pthread.h>

#define STATISTIC_INFO_FAID_NUM     10
#define STATISTIC_INFO_FAID_SIZE    256
#define STATISTIC_INFO_MULT_SIZE    64

#ifdef __cplusplus

namespace Hippo {

class UltraPlayerStatistic {    /*for statistic */
public :
    UltraPlayerStatistic(){};
    ~UltraPlayerStatistic(){};

public:
    char* getAbendInfo(int index){return s_abendInfo[index];}

    int getUnderflowNum(){return s_StreamTimeOutNum;}
    int setUnderflowNum(int num){return s_StreamTimeOutNum = num;}
    int getTotalAbendDuration(){return s_StreamTimeOutTotalDuration;}
    int getAbendDurationClock(){return s_StreamTimeOutClock;}
    int getAbendDurationMax(){return s_StreamTimeOutMaxDuration;}
    int getAbendDuration(int index){return s_StreamTimeOutDuration[index];}
    int getBufferDecNmb(){return s_bufferIncreaseNum;}
    int getBufferIncNmb(){return s_bufferDecreaseNum;}
    int getplayErrorNumbers(){return s_playErrorNum;}
    char* getPlayErrorInfo(int index){return s_playErrorInfo[index];}
    int getStreamGapNumbers(){return s_StreamTimeOutNum;}
    char* getStreamGapEvent(int index){return s_streamTimeOutEvent[index];}
    int getTotalChannelSwitchTime(){return s_switchChannelTotalTime;}
    int getChannelSwitchNumbers(){return s_switchChannelNum;}
    int getChannelSwitchWorstTime(){return s_channelSwitchWorstTime;}
    int getChannelSwitchBestTime(){return s_channelSwitchBestTime;}

    static int app_abend_duration(void);
    static void streamAbendend(void);
    static void mutexInit();
#ifdef HUAWEI_C10
    static void streamPostFlow(int mult, int width, int flow);
#endif
    static void streamAbendFail(char *url);
    static void streamPostOk(int mult, int rrt);
    static void streamPostFail(int mult, char *url, int err_no);
    static void streamStreamgap(void);
    static void streamChannelzap(int ms);

    static pthread_mutex_t* getMutex();
protected:
    /*for tr069 statistic*/
    static pthread_mutex_t* s_mutex;
    static char s_abendInfo[STATISTIC_INFO_FAID_NUM][STATISTIC_INFO_FAID_SIZE]; // no use todo
    static int s_StreamTimeOutNum;
    static int s_StreamTimeOutTotalDuration;
    static int s_StreamTimeOutClock;
    static int s_StreamTimeOutMaxDuration;
    static int s_StreamTimeOutDuration[STATISTIC_INFO_FAID_NUM];
    static int s_switchChannelTotalTime;
    static int s_switchChannelNum;
    static int s_channelSwitchWorstTime; //ju no use
    static int s_channelSwitchBestTime; //ju no use
    static int s_bufferIncreaseNum;
    static int s_bufferDecreaseNum;
    static int s_playErrorNum;
    static char s_playErrorInfo[STATISTIC_INFO_FAID_NUM][STATISTIC_INFO_FAID_SIZE];
    static char s_streamTimeOutEvent[STATISTIC_INFO_FAID_NUM][STATISTIC_INFO_FAID_SIZE];
};

} //namespace Hippo

extern "C"
{
void stream_port_post_abend_fail(char *url);
void stream_port_post_abend_end(void);
void stream_port_post_channelzap(int ms);
void stream_port_post_streamgap(void);
void stream_port_post_ok(int mult, int rrt);
void stream_port_post_fail(int mult, char *url, int err_no);
void stream_port_post_flow(int mult, int width, int flow);

void stream_port_post_httpinfo(char *info_buf, int info_len);
void stream_port_post_rstpinfo(int multflg, char *info_buf, int info_len);
}

#endif // __cplusplus

#endif  // _UltraPlayerStatistic_H_