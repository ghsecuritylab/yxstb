/**********************************************************
    Copyright (c) 2008-2009, Yuxing Software Corporation
    All Rights Reserved
    Confidential Property of Yuxing Softwate Corporation

    Revision History:

    Created: 2009-10-22 9:30:31 by liujianhua

 **********************************************************/

#ifndef __STRM_BUFFER_H__
#define __STRM_BUFFER_H__

typedef struct _StrmBuffer StrmBuffer;
typedef struct _StrmBufQue StrmBufQue;
typedef struct _StrmBufPSI StrmBufPSI;
typedef struct _StrmBufPlay StrmBufPlay;


struct _StrmBuffer {
    StrmBuffer* next;

    char*   buf;
    int     size;
    int     len;
    int     off;
};
StrmBuffer* strm_buf_malloc(int size);
void strm_buf_free(StrmBuffer* sb);
// Õ∑≈∂”¡–
void strm_buf_release(StrmBuffer *sb);

StrmBufQue* strm_bufque_create  (int size, int num);
void        strm_bufque_delete  (StrmBufQue* sbq);
int         strm_bufque_space   (StrmBufQue* sbq);
int         strm_bufque_length  (StrmBufQue* sbq);
void        strm_bufque_reset   (StrmBufQue* sbq);
void        strm_bufque_push    (StrmBufQue* sbq, StrmBuffer **psb);
int         strm_bufque_pop     (StrmBufQue* sbq, StrmBuffer **psb);

struct _StrmBufPSI {
    StrmBufQue* sbq;
    StrmBuffer* sb;

    int         psi_flg;
    ts_parse_t  ts_parse;

    struct ts_psi           ts_psi;
    struct ts_dr_subtitle   ts_subtitle;
    struct ts_dr_teletext   ts_teletext;
};

struct _StrmBufPlay {
    StrmBuffer* sb;
    StrmBufQue* sbq;
    StrmBufPSI* sbpsi;

    StrmBuffer* ca_sb;
    int         ca_off;
    int         ca_flg;
    uint32_t    system_id;

    StrmBuffer* pcr_sb;
    uint32_t    pcr_pid;
    int         pcr_off;
    uint32_t    pcr_last;
    uint32_t    pcr_value;

    //int       pop;
};

#define SBO_STRMBUFFER_NUM  10
#define SBO_SEQUENCE_DIFF   100
struct _StrmBufOrder {
    StrmBuffer*     pool;

    uint32_t        rtp_seq;
    StrmBuffer*     rtp_head;

    StrmBuffer*     exp_head;
    StrmBuffer*     exp_tail;
};
typedef struct _StrmBufOrder StrmBufOrder;


StrmBufPlay*    strm_bufplay_create (int size, int num);
void            strm_bufplay_delete (StrmBufPlay* sbp);
void            strm_bufplay_reset  (StrmBufPlay* sbp);
int             strm_bufplay_psi_get(StrmBufPlay* sbp, ts_psi_t ts_psi);
void            strm_bufplay_psi_set(StrmBufPlay* sbp, ts_psi_t ts_psi);
void            strm_bufplay_ca     (StrmBufPlay* sbp, StrmBuffer* psb);
void            strm_bufplay_pop    (StrmBufPlay* sbp);
void            strm_bufplay_pcr    (StrmBufPlay* sbp, int* poff, uint32_t* pvalue);

StrmBufOrder* strm_bufOrder_create(int size);
void strm_bufOrder_reset(StrmBufOrder* sbo);
void strm_bufOrder_delete(StrmBufOrder* sbo);
void strm_bufOrder_push(StrmBufOrder* sbo, StrmBuffer **psb);
void strm_bufOrder_pop(StrmBufOrder* sbo, StrmBuffer **psb);

#endif//__STRM_BUFFER_H__
