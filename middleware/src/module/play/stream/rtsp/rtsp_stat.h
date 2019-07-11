
/******************************************************************************
    RTSP ͳ��
 ******************************************************************************/

#ifndef __RTSP_STAT_H__
#define __RTSP_STAT_H__

struct RTSP;

struct __RTSPLost {
    //ͳ�ƶ���
    uint32_t    rtp_stat_total;
    uint32_t    rtp_stat_lost;

    int     rtp_lost_num;
    uint32_t    rtp_lost_clk;
    uint32_t    rtp_lost_seq0;
    uint32_t    rtp_lost_seq1;
    uint32_t    rtp_seq;
};
typedef struct __RTSPLost RTSPLost;
typedef struct __RTSPLost* RTSPLost_t;

struct __RTSPStat {
    //ͳ������
    int             stat_int_bitrate;//ͳ������
    uint32_t            stat_clock;
    uint32_t            stat_bytes;
    int             stat_bitrate;
    uint32_t            stat_bitrate_lost;
    uint32_t            stat_bitrate_total;

    //ͳ�ƶ�����
    int             stat_int_pklosts;//ͳ������

    //VOD��������
    uint32_t            stat_vodreq;
    uint32_t            stat_vodstop;
    uint32_t            stat_vodpause;
    uint32_t            stat_vodplay;

    RTSPLost        lost_rtp;
    int             lost_warn;
    //FEC, ARQ �Ⱦ���󶪰�ͳ��
    RTSPLost        lost_correct;

    //�ϱ�������
    CallBack_StatFlow    call_statflow;
};
typedef struct __RTSPStat RTSPStat;
typedef struct __RTSPStat* RTSPStat_t;

#ifdef __cplusplus
extern "C" {
#endif

void rtsp_stat_init(struct RTSP* rtsp);

void rtsp_stat_reset(RTSPLost_t lost, int clear);

void rtsp_stat_set(struct RTSP* rtsp, int flag);

void rtsp_stat_seq(struct RTSP* rtsp, RTSPLost_t lost, uint32_t seq);

void rtsp_stat_seq_ex(struct RTSP* rtsp, uint seq);

#ifdef __cplusplus
}
#endif

#endif//__RTSP_STAT_H__
