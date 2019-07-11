
#ifndef __INT_PVR_H__
#define __INT_PVR_H__

//10����1����Ƭ
#define IFRAME_CLK_INTERVAL        40

//SECOND_PER_FRAGMENT���ֵΪ600
#define FRAGMENT_TIME_DEFAULT        600
#define FRAGMENT_TIME_CURRENT        60

typedef struct tagPvrSInfo {
    uint32_t        id;
    uint32_t        base_id;
    int         version;
    int         encrypt;//����
    int         networkid;
    int         record;//¼�ƴ�
    int         breaktype;//ʱ��ʱbreaktypeΪ-1
    int         pcr;
    long long   key;

    uint32_t        prev_clks;
    uint32_t        prev_date;

    uint32_t        time_len;
    uint32_t        time_base;
    uint32_t        time_bmark;
    uint32_t        time_length;

    int         fill_num;

    int         base_len;
    long long   byte_len;
    long long   byte_base;
    uint32_t        byte_rate;
    long long   byte_bmark;
    long long   byte_length;
} PvrSInfo;
typedef PvrSInfo* PvrSInfo_t;

#endif//__INT_PVR_H__

