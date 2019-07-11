
#include "stream.h"

struct _StrmBufQue {
    StrmBuffer* head;
    StrmBuffer* tail;
    StrmBuffer* pool;

    int num;
    int size;
    int space;
    int length;

    int writes;
    int reads;
};

static StrmBufPSI* int_bufpsi_create  (int size, int num);
static void        int_bufpsi_reset   (StrmBufPSI* sbp);
static int         int_bufpsi_parse   (StrmBufPSI* sbp, ts_psi_t ts_psi);
static void        int_bufpsi_delete  (StrmBufPSI* sbp);

StrmBuffer* strm_buf_malloc(int size)
{
    StrmBuffer *sb = (StrmBuffer *)IND_CALLOC(sizeof(StrmBuffer), 1);
    if (!sb)
        goto Err;

    sb->buf = (char*)IND_MALLOC(size);
    if (!sb->buf)
        goto Err;
    sb->size = size;

    return sb;
Err:
    if (sb)
        IND_FREE(sb);
    return NULL;
}

void strm_buf_free(StrmBuffer* sb)
{
    if (!sb)
        return;
    IND_FREE(sb->buf);
    IND_FREE(sb);
}

void strm_buf_release(StrmBuffer *sb)
{
    StrmBuffer *next;

    while (sb) {
        next = sb->next;
        strm_buf_free(sb);
        sb = next;
    }
}

StrmBufQue* strm_bufque_create(int size, int num)
{
    int i;
    StrmBuffer *sb;
    StrmBufQue *sbq = NULL;

    if (num <= 0 || size <= 0)
        LOG_STRM_ERROUT("num = %d, size = %d\n", num, size);
    LOG_STRM_PRINTF("num = %d, size = %d\n", num, size);

    sbq = (StrmBufQue *)IND_CALLOC(sizeof(StrmBufQue), 1);
    if (!sbq)
        goto Err;
    sbq->num = num;
    sbq->size = size;
    sbq->space = size * num;

    for (i = 0; i < num; i ++) {
        sb = strm_buf_malloc(size);
        if (!sb)
            goto Err;
        sb->next = sbq->pool;
        sbq->pool = sb;
    }

    return sbq;
Err:
    if (sbq) {
        strm_buf_release(sbq->pool);
        IND_FREE(sbq);
    }
    return NULL;
}

void strm_bufque_delete(StrmBufQue* sbq)
{
    if (!sbq)
        return;
    LOG_STRM_PRINTF("num = %d, size = %d\n", sbq->num, sbq->size);

    strm_buf_release(sbq->head);
    strm_buf_release(sbq->pool);

    IND_FREE(sbq);
}

void strm_bufque_reset(StrmBufQue* sbq)
{
    if (sbq->head) {
        sbq->tail->next = sbq->pool;
        sbq->pool = sbq->head;
        sbq->head = NULL;
        sbq->tail = NULL;
    }
    sbq->space = sbq->size * sbq->num;
    sbq->length = 0;

    sbq->reads = 0;
    sbq->writes = 0;
}

int strm_bufque_space(StrmBufQue* sbq)
{
    return sbq->space;
}

int strm_bufque_length(StrmBufQue* sbq)
{
    return sbq->length;
}

void strm_bufque_push(StrmBufQue* sbq, StrmBuffer **psb)
{
    StrmBuffer *sb, *tail;

    sb = *psb;

    if (sb->size != sbq->size)
        LOG_STRM_ERROUT("size = %d / %d\n", sb->size, sbq->size);
    if (sb->off < 0 || sb->len <= 0 || sb->off + sb->len > sb->size)
        LOG_STRM_ERROUT("off = %d, len = %d / size = %d\n", sb->off, sb->len, sb->size);

    if (!sbq->pool) {
        LOG_STRM_PRINTF("sbq = %p, size = %d, num = %d\n", sbq, sbq->size, sbq->num);
        LOG_STRM_ERROR("OVERFLOW! writes = %d, reads = %d, num = %d\n", sbq->writes, sbq->reads, sbq->num);
        strm_bufque_reset(sbq);
    }

    tail = sbq->tail;
    if (tail)
        tail->next = sb;
    else
        sbq->head = sb;
    sbq->tail = sb;
    sb->next = NULL;

    sbq->space -= sb->size;
    sbq->length += sb->len;

    sbq->writes++;

    sb = sbq->pool;
    sbq->pool = sb->next;
    sb->next = NULL;

    *psb = sb;
Err:
    return;
}

int strm_bufque_pop(StrmBufQue* sbq, StrmBuffer **psb)
{
    StrmBuffer *sb, *head;

    sb = *psb;
    if (sb->size != sbq->size)
        LOG_STRM_ERROUT("size = %d / %d\n", sb->size, sbq->size);

    head = sbq->head;
    if (!head)
        LOG_STRM_ERROUT("empty! writes = %d, reads = %d\n", sbq->writes, sbq->reads);

    sbq->head = head->next;
    if (!sbq->head)
        sbq->tail = NULL;
    head->next = NULL;

    sbq->space += head->size;
    sbq->length -= head->len;

    sbq->reads++;

    sb->next = sbq->pool;
    sbq->pool = sb;

    *psb = head;

    return 0;
Err:
    return -1;
}

static StrmBufPSI* int_bufpsi_create(int size, int num)
{
    StrmBufPSI* sbp;

    sbp = (StrmBufPSI *)IND_CALLOC(sizeof(StrmBufPSI), 1);
    if (!sbp)
        LOG_STRM_ERROUT("calloc\n");

    sbp->sbq = strm_bufque_create(size, num);
    if (!sbp->sbq)
        LOG_STRM_ERROUT("strm_bufque_create\n");

    sbp->ts_psi.dr_subtitle = &sbp->ts_subtitle;
    sbp->ts_psi.dr_teletext = &sbp->ts_teletext;

    sbp->ts_parse = ts_parse_create(&sbp->ts_psi);
    if (!sbp->ts_parse)
        goto Err;

    return sbp;
Err:
    int_bufpsi_delete(sbp);
    return 0;
}

static void int_bufpsi_delete(StrmBufPSI* sbp)
{
    if (!sbp)
        return;

    if (sbp->sbq)
        strm_bufque_delete(sbp->sbq);

    if (sbp->ts_parse)
        ts_parse_delete(sbp->ts_parse);

    IND_FREE(sbp);
}

static int int_bufpsi_parse(StrmBufPSI* sbp, ts_psi_t ts_psi)
{
    if (!sbp->psi_flg) {
        StrmBuffer* sb = sbp->sb;
        StrmBufQue* sbq = sbp->sbq;

        if (!sb) {
            sb = sbq->head;
        } else {
            if (!sb->next)
                return 0;
            sb = sb->next;
        }

        while (sb) {
            if (1 == ts_parse_psi(sbp->ts_parse, (uint8_t *)(sb->buf + sb->off), sb->len, NULL)) {
                if (sbp->ts_psi.video_pid || sbp->ts_psi.audio_num >= 0) {
                    sbp->psi_flg = 1;
                    break;
                }
            }

            sbp->sb = sb;
            sb = sb->next;
        }

        if (!sbp->psi_flg)
            return 0;
    }

    ts_psi_copy(ts_psi, &sbp->ts_psi);

    return 1;
}

static void int_bufpsi_reset(StrmBufPSI* sbp)
{
    strm_bufque_reset(sbp->sbq);

    if (!sbp->psi_flg)
        ts_parse_reset(sbp->ts_parse);
    sbp->sb = NULL;

}

StrmBufPlay* strm_bufplay_create(int size, int num)
{
    StrmBufPlay* sbp;

    sbp = (StrmBufPlay *)IND_CALLOC(sizeof(StrmBufPlay), 1);
    if (!sbp)
        LOG_STRM_ERROUT("calloc\n");

    sbp->sbpsi = int_bufpsi_create(size, num);
    if (!sbp->sbpsi)
        LOG_STRM_ERROUT("int_bufpsi_create\n");

    sbp->sb = strm_buf_malloc(size);
    if (!sbp->sb)
        goto Err;

    sbp->sbq = sbp->sbpsi->sbq;

    return sbp;
Err:
    strm_bufplay_delete(sbp);
    return 0;
}
void strm_bufplay_delete(StrmBufPlay* sbp)
{
    if (!sbp)
        return;

    if (sbp->sbpsi)
        int_bufpsi_delete(sbp->sbpsi);
    if (sbp->sb)
        strm_buf_free(sbp->sb);

    IND_FREE(sbp);
}

void strm_bufplay_reset(StrmBufPlay* sbp)
{
    int_bufpsi_reset(sbp->sbpsi);

    sbp->ca_sb = NULL;
    sbp->ca_off = 0;

    sbp->pcr_sb = NULL;
    sbp->pcr_off = 0;
    sbp->pcr_last = 0;
    sbp->pcr_value = 0;

    sbp->sb->len = 0;
}

int strm_bufplay_psi_get(StrmBufPlay* sbp, ts_psi_t ts_psi)
{
    int ret = int_bufpsi_parse(sbp->sbpsi, ts_psi);
    if (1 == ret)
        sbp->pcr_pid = ts_psi->pcr_pid;

    return ret;
}

void strm_bufplay_psi_set(StrmBufPlay* sbp, ts_psi_t ts_psi)
{
    ts_psi_copy(&sbp->sbpsi->ts_psi, ts_psi);
    sbp->sbpsi->psi_flg = 1;

    sbp->pcr_pid = ts_psi->pcr_pid;

    sbp->pcr_sb = NULL;
    sbp->pcr_off = 0;
    sbp->pcr_value = 0;
}

void strm_bufplay_ca(StrmBufPlay* sbp, StrmBuffer *sb)
{
    int valid;
    uint32_t pid;
    uint8_t *ubuf;
    ts_psi_t psi;
    StrmBufQue* sbq;
    StrmBuffer *ca_sb;
    StrmBufPSI* sbpsi = sbp->sbpsi;

    sb->len = 0;
    sb->off = 0;

    if (!sbpsi->psi_flg)
        return;

    psi = &sbpsi->ts_psi;
    if (!sbp->ca_flg) {
        int num = psi->ecm_num;

        sbp->system_id = 0;
        if (num > 0) {
            int i;
            uint32_t mask;
            struct ts_ca ca, *ecm_array;

            ecm_array = psi->ecm_array;
            if (num > 0) {
                for (i = 0; i < num; i ++) {
                    ca = ecm_array[i];
                    mask = (uint32_t)(ca.system_id & 0xff00);
                    if (mask == 0x0b00 || mask == 0x1800)
                        break;
                }
                if (i >= num)
                    ca = ecm_array[0];
            }
            sbp->ca_flg = 1;
            sbp->system_id = ca.system_id;
        }
        if (!sbp->ca_flg)
            return;
    }

    ca_sb = sbp->ca_sb;
    sbq = sbpsi->sbq;

    if (!ca_sb) {
        ca_sb = sbq->head;
        if (!ca_sb)
            return;
        sbp->ca_sb = ca_sb;
        sbp->ca_off = 0;
    }

    while (sb->len + 188 <= sb->size) {
        if (sbp->ca_off >= ca_sb->len) {
            ca_sb = ca_sb->next;
            if (!ca_sb)
                return;
            sbp->ca_sb = ca_sb;
            sbp->ca_off = 0;
        }

        valid = 0;
        ubuf = (uint8_t*)(ca_sb->buf + ca_sb->off + sbp->ca_off);
        pid =(((uint32_t)ubuf[1] & 0x1f) << 8) + ubuf[2];

        {
            uint32_t mask;
            int i, num;
            ts_ca_t array;

            num = psi->ecm_num;
            if (num > 0) {
                array = psi->ecm_array;
                for (i = 0; i < num; i ++) {
                    if (pid == (uint32_t)array[i].pid) {
                        valid = 1;
                        break;
                    }
                }
            }

            mask = sbp->system_id & 0xff00;
            if (valid == 0 && (mask == 0x0b00 || mask == 0x1800)) {
                if (pid == 0 || pid == 1) {//PAT CAT
                    valid = 1;
                } else if (pid == (uint32_t)psi->pmt_pid) {//PMT
                    valid = 1;
                } else {
                    num = psi->emm_num;
                    if (num > 0) {
                        array = psi->emm_array;
                        for (i = 0; i < num; i ++) {
                            if (pid == (uint32_t)array[i].pid) {
                                valid = 1;
                                break;
                            }
                        }
                    }
                }
            }
        }
        if (valid == 1) {
            IND_MEMCPY(sb->buf + sb->len, ubuf, 188);
            sb->len += 188;
        }

        sbp->ca_off += 188;
    }
}

void strm_bufplay_pop(StrmBufPlay* sbp)
{
    StrmBufQue* sbq = sbp->sbpsi->sbq;

    sbp->sb->len = 0;
    if (0 == strm_bufque_pop(sbq, &sbp->sb)) {
        int n;

        n = sbp->sb->len / 188;
        //sbp->pop += n;

        if (!sbq->head || sbp->pcr_off < n) {
            sbp->pcr_sb = NULL;
            sbp->pcr_off = 0;
            sbp->pcr_value = 0;
        } else {
            sbp->pcr_off -= n;
        }
    }
}

void strm_bufplay_pcr(StrmBufPlay* sbp, int* poff, uint32_t* pvalue)
{
    int i, n;
    uint32_t pid, pcr;
    uint8_t *pkt;

    if (0 == sbp->pcr_value) {
        StrmBuffer *sb = sbp->pcr_sb;
        StrmBufQue* sbq = sbp->sbpsi->sbq;

        if (!sb)
            sb = sbq->head;
        else
            sb = sb->next;

        pcr = 0;
        while (sb) {
            sbp->pcr_sb = sb;
    
            pkt = (uint8_t*)(sb->buf + sb->off);
            n = sb->len / 188;
            for (i = 0; i < n; i ++, pkt += 188) {
                pid = ((pkt[1]&0x1F)<<8) | pkt[2];
                if (pid != sbp->pcr_pid)
                    continue;

                pcr = ts_pcr_parse188(pkt);
                if (pcr && (pcr <= sbp->pcr_last || pcr > sbp->pcr_last + 2250)) {//45000.0 * 0.05 = 2250
                    //sbp->pcr_off += i;
                    //printf("++++ off = %d / %d, pcr = %d\n", sbp->pcr_off, sbp->pop + sbp->pcr_off, pcr - sbp->pcr_last);
                    sbp->pcr_last = pcr;
                    sbp->pcr_value = pcr;
                    break;
                }
            }
            if (pcr)
                break;
            sbp->pcr_off += n;
            sb = sb->next;
        }
    }

    *poff = sbp->pcr_off;
    *pvalue = sbp->pcr_value;
}

/*
struct _StrmBufOrder {
    StrmBuffer* pool;
    StrmBuffer* rtp_sb;
    uint32_t    rtp_seq;
    StrmBuffer* exp_sb;
    uint32_t    exp_seq;
};
typedef struct _StrmBufOrder StrmBufOrder;
*/

StrmBufOrder* strm_bufOrder_create(int size)
{
    int i;
    StrmBuffer* sb;
    StrmBufOrder* sbo = NULL;

    sbo = (StrmBufOrder*)calloc(sizeof(StrmBufOrder), 1);
    if (!sbo)
        goto Err;

    for (i = 0; i < SBO_STRMBUFFER_NUM; i++) {
        sb = strm_buf_malloc(size);
        if (!sb)
            goto Err;
        sb->next = sbo->pool;
        sbo->pool = sb;
    }
    sbo->rtp_seq = RTP_INVALID_SEQ;

    return sbo;
Err:
    strm_bufOrder_delete(sbo);
    return NULL;
}

void strm_bufOrder_reset(StrmBufOrder* sbo)
{
    StrmBuffer *sb, *pool;

    pool = sbo->rtp_head;
    sbo->rtp_head = NULL;
    sbo->rtp_seq = RTP_INVALID_SEQ;

    while (pool) {
        sb = pool;
        pool = pool->next;
        sb->next = sbo->pool;
        sbo->pool = sb;
    }

    pool = sbo->exp_head;
    sbo->exp_head = NULL;
    sbo->exp_tail = NULL;

    while (pool) {
        sb = pool;
        pool = pool->next;
        sb->next = sbo->pool;
        sbo->pool = sb;
    }
}

void strm_bufOrder_delete(StrmBufOrder* sbo)
{
    StrmBuffer *sb, *pool;

    pool = sbo->rtp_head;
    while (pool) {
        sb = pool;
        pool = pool->next;
        strm_buf_free(sb);
    }
    pool = sbo->exp_head;
    while (pool) {
        sb = pool;
        pool = pool->next;
        strm_buf_free(sb);
    }
    pool = sbo->pool;
    while (pool) {
        sb = pool;
        pool = sb->next;
        strm_buf_free(sb);
    }
    free(sbo);
}

static void int_bufOrder_push(StrmBufOrder* sbo, StrmBuffer *sb)
{
    uint32_t seq = sb->off;

    if (RTP_INVALID_SEQ == sbo->rtp_seq) {
        sbo->rtp_head = sb;
        sb->next = NULL;
        sbo->rtp_seq = seq;
        return;
    }

    if ((seq >= sbo->rtp_seq && seq < sbo->rtp_seq + SBO_SEQUENCE_DIFF) || (seq < sbo->rtp_seq && seq + RTP_INVALID_SEQ < sbo->rtp_seq + SBO_SEQUENCE_DIFF)) {
        StrmBuffer *sbuf, *prev;

        sbuf = sbo->rtp_head;
        prev = NULL;
        while (sbuf) {
            if (seq == sbuf->off) {
                //printf("s0:%d ", seq);
                sb->next = sbo->pool;
                sbo->pool = sb;
                return;
            }
            if ((seq < sbuf->off && seq + SBO_SEQUENCE_DIFF > sbuf->off) || (seq > sbuf->off && seq + SBO_SEQUENCE_DIFF > sbuf->off + RTP_INVALID_SEQ))
                break;

            prev = sbuf;
            sbuf = sbuf->next;
        }
        sb->next = sbuf;
        if (prev)
            prev->next = sb;
        else
            sbo->rtp_head = sb;
        //printf("i:%d ", seq);
        return;
    }

    if ((seq < sbo->rtp_seq && seq + SBO_SEQUENCE_DIFF >= sbo->rtp_seq) || (seq > sbo->rtp_seq && seq + SBO_SEQUENCE_DIFF >= sbo->rtp_seq + RTP_INVALID_SEQ)) {
        //printf("s1:%d ", seq);
        sb->next = sbo->pool;
        sbo->pool = sb;
        return;
    }

    //printf("e:%d ", seq);
    sb->next = NULL;
    if (sbo->exp_tail)
        sbo->exp_tail->next = sb;
    else
        sbo->exp_head = sb;
    sbo->exp_tail = sb;
}

void strm_bufOrder_push(StrmBufOrder* sbo, StrmBuffer **psb)
{
    int hdr;
    uint32_t seq;
    StrmBuffer *sb;

    if (!sbo->pool) {
        LOG_STRM_ERROR("pool empty!\n");
        return;
    }

    sb = *psb;
    hdr = ind_rtp_parse(sb->buf, sb->len, &seq);
    if (hdr <= 0)
        return;

    sb->off = seq;
    int_bufOrder_push(sbo, sb);

    sb = sbo->pool;
    sbo->pool = sb->next;
    sb->next = NULL;

    sb->off = 0;
    sb->len = 0;
    *psb = sb;
}

void strm_bufOrder_pop(StrmBufOrder* sbo, StrmBuffer **psb)
{
    StrmBuffer* sb, *pool;

    sb = *psb;
    sb->len = 0;

    if (!sbo->rtp_head && !sbo->pool) {
        //printf("r0 ");
        sbo->rtp_seq = RTP_INVALID_SEQ;

        pool = sbo->exp_head;

        sbo->exp_head = NULL;
        sbo->exp_tail = NULL;

        while (pool) {
            sb = pool;
            pool = pool->next;
            sb->next = NULL;
            int_bufOrder_push(sbo, sb);
        }
    }

    if (sbo->rtp_head) {
        uint32_t seq = sbo->rtp_head->off;

        if (seq != sbo->rtp_seq && !sbo->pool) {
            //printf("j:%d>%d ", sbo->rtp_seq, seq);
            sbo->rtp_seq = seq;
        }
        if (seq == sbo->rtp_seq) {
            sb = *psb;
            sb->next = sbo->pool;
            sbo->pool = sb;

            sb = sbo->rtp_head;
            sbo->rtp_head = sb->next;
            sb->next = NULL;
            sb->off = 0;
            *psb = sb;
            //printf("o:%d ", seq);

            sbo->rtp_seq = (seq + 1) % RTP_INVALID_SEQ;

            pool = sbo->exp_head;
            if (pool) {
                sbo->exp_head = NULL;
                sbo->exp_tail = NULL;

                while (pool) {
                    sb = pool;
                    pool = pool->next;
                    //printf("s2:%d ", sb->off);
                    sb->next = sbo->pool;
                    sbo->pool = sb;
                }
            }
        } else {
            //printf("s:%d,%d ", seq, sbo->rtp_seq);
        }
    }
}
