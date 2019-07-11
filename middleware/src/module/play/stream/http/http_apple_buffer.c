
/**************************************--**************************************
    Copyright (c) 2008-2009, Yuxing Software Corporation
    All Rights Reserved
    Confidential Property of Yuxing Softwate Corporation

    Revision History:

    Created: 2009-10-22 9:30:31 by liujianhua

 **************************************--**************************************/

#include "http_apple.h"

#if SUPPORTE_HD == 1

#define APPLE_PUSH_10MS     200
#if (defined(Sichuan) && defined(hi3716m))
#define APPLE_BUFQUE_NUM    20
#else
#define APPLE_BUFQUE_NUM    5//最多只存5个分片
#endif

#define APPLE_CRIPT_SIZE    (1316 * 196)

typedef struct tagStrmBufHead StrmBufHead;
struct tagStrmBufHead {
    StrmBuffer* head;
    StrmBuffer* tail;

    int         index;//音轨序号
    int         length;
};

typedef struct tagAppleBuffer AppleBuffer;
struct tagAppleBuffer {
    StrmBufHead head;
    StrmBufHead switch_head;

    int         valid;

    int         recv_len;
    int         push_len;
    int         push_step;
};

typedef struct tagAppleMuxQue AppleMuxQue;
struct tagAppleMuxQue {
    AppleBuffer array[HAPPLE_TRACK_NUM];

    int valid;
    int bitrate;
    int duration;
    int sequence;
};

struct tagAppleBufQue {
    StrmBuffer* pool;

    int         elemSize;
    int         totalNum;
    int         currentNum;

    AppleMuxQue matrix[APPLE_BUFQUE_NUM];

    int         ab_index;
    int         amq_index;

    int         prev_length;
    int         prev_bitrate;
    int         prev_duration;

    int         push_step;
    int         push_range;
    int         push_length;

    int         pmt_flag;
    uint32_t    pmt_counter;
};

static void strm_bufhead_free(AppleBufQue *abq, StrmBuffer *sb)
{
    if (1 == sb->off) {
        strm_buf_free(sb);
    } else {
        sb->next = abq->pool;
        abq->pool = sb;
        abq->currentNum++;
    }
}

static void strm_bufhead_empty(AppleBufQue *abq, StrmBufHead *sbh)
{
     StrmBuffer *sb, *next;

    sb = sbh->head;
    while (sb) {
        next = sb->next;
        strm_bufhead_free(abq, sb);
        sb = next;
    }

    sbh->head = NULL;
    sbh->tail = NULL;
}

static int strm_bufhead_fill(AppleBufQue *abq, StrmBufHead *sbh, char* buf, int len)
{
    int l;
    StrmBuffer *sb;

    strm_bufhead_empty(abq, sbh);

    sbh->length = len;
    while (len > 0) {
        if (len > STREAM_BUFFER_SIZE)
            l = STREAM_BUFFER_SIZE;
        else
            l = len;

        if (abq->pool) {
            sb = abq->pool;
            abq->pool = sb->next;
            abq->currentNum--;
            sb->off = 0;
        } else {
            sb = strm_buf_malloc(STREAM_BUFFER_SIZE);
            if (!sb)
                LOG_STRM_ERROUT("strm_buf_malloc\n");
            sb->off = 1;
        }
        IND_MEMCPY(sb->buf, buf, l);
        sb->len = l;
        buf += l;
        len -= l;

        sb->next = NULL;
        if (sbh->tail)
            sbh->tail->next = sb;
        else
            sbh->head = sb;
        sbh->tail = sb;
    }

    return 0;
Err:
    strm_bufhead_empty(abq, sbh);
    return -1;
}

static int strm_bufhead_push(AppleBufQue* abq, StrmBufHead* sbh, HAppleTrack* track, char* buf, int len)
{
    int l, bytes;
    StrmBuffer *sb;

    bytes = 0;
    while (len > 0 && abq->pool) {
        sb = abq->pool;
        abq->pool = sb->next;
        abq->currentNum--;

        if (len > STREAM_BUFFER_SIZE)
            l = STREAM_BUFFER_SIZE;
        else
            l = len;

        if (track->ha->alternative_audio) {
            int i;
            uint32_t pid;
            uint8_t *ubuf;
            AppleTask* task;
            struct ts_psi *psi;

            task = &track->task;
            psi = &task->ts_psi;

            if (!task->ts_pid) {
                HttpApple *ha = track->ha;

                if (1 == ts_parse_psi(task->ts_parse, (uint8_t*)buf, l, NULL)) {
                    switch(track->index) {
                    case HAPPLE_TRACK_VIDEO:
                        task->ts_pid = psi->video_pid;
                        ha->ts_psi.video_type = psi->video_type;
                        break;
                    case HAPPLE_TRACK_AUDIO:
                        task->ts_pid = psi->audio_pid[0];
                        for (i = 0; i < ha->ts_psi.audio_num; i++)
                            ha->ts_psi.audio_type[i] = psi->audio_type[0];
                         break;
                    case HAPPLE_TRACK_SUBTITLE:
                        task->ts_pid = psi->dr_subtitle->subtitle_pid[0];
                        {
                            ts_dr_subtitle_t dr_subtitle = ha->ts_psi.dr_subtitle;
                            for (i = 0; i < dr_subtitle->subtitle_num; i++) {
                                dr_subtitle->subtitle[i].type = psi->dr_subtitle->subtitle[0].type;
                                dr_subtitle->subtitle[i].composition = psi->dr_subtitle->subtitle[0].composition;
                                dr_subtitle->subtitle[i].ancillary = psi->dr_subtitle->subtitle[0].ancillary;
                            }
                        }
                        break;
                    default:
                        break;
                    }
                }
                LOG_STRM_PRINTF("ts_pid = %d\n", task->ts_pid);
            }
            for (i = 0; i < l; i += 188) {
                ubuf = (uint8_t*)(buf + i);
                pid = (((uint)ubuf[1] & 0x1f) << 8) + ubuf[2];//program_number
                if (pid && pid == task->ts_pid) {
                    ubuf[1] = (ubuf[1] & 0xe0) | (unsigned char)(track->pid >> 8);
                    ubuf[2] = (unsigned char)track->pid;
                } else {
                    ubuf[1] = 0x1F;
                    ubuf[2] = 0xFF;
                    ubuf[3] = 0x00;
                    memset (ubuf + 4, 0xff, 184);
                }
            }
        }

        IND_MEMCPY(sb->buf, buf, l);
        buf += l;
        len -= l;

        sb->len = l;
        sb->off = 0;

        bytes += l;

        sb->next = NULL;
        if (sbh->tail)
            sbh->tail->next = sb;
        else
            sbh->head = sb;
        sbh->tail = sb;
    }

    return bytes;
}

static StrmBuffer* strm_bufhead_pop(StrmBufHead *sbh)
{
    StrmBuffer *sb = sbh->head;

    sbh->head = sb->next;
    if (!sbh->head)
        sbh->tail = NULL;

    return sb;
}

AppleBufQue* apple_bufque_create(int size, int num)
{
    int i;
    StrmBuffer *sb;
    AppleBufQue *abq;

    abq = (AppleBufQue*)IND_CALLOC(sizeof(AppleBufQue), 1);
    if (!abq)
        LOG_STRM_ERROUT("malloc AppleBufQue\n");

    for (i = 0; i < num; i++) {
        sb = strm_buf_malloc(size);
        if (!sb)
            goto Err;
        sb->next = abq->pool;
        abq->pool = sb;
    }
    abq->elemSize = size;
    abq->totalNum = num;
    abq->currentNum = num;

    return abq;
Err:
    if (abq)
        apple_bufque_delete(abq);
    return NULL;
}

void apple_bufque_reset(AppleBufQue* abq)
{
    int i, j, idx;
    StrmBuffer *pool;
    AppleBuffer *array;

    idx = abq->amq_index;
    if (idx >= APPLE_BUFQUE_NUM)
        idx = APPLE_BUFQUE_NUM - 1;

    for (i = 0; i <= idx; i++) {
        array = abq->matrix[i].array;
        for (j = 0; j < HAPPLE_TRACK_NUM; j++) {
            strm_bufhead_empty(abq, &array[j].head);
            strm_bufhead_empty(abq, &array[j].switch_head);
        }
    }

    pool = abq->pool;
    IND_MEMSET(abq, 0, sizeof(AppleBufQue));
    abq->pool = pool;
    abq->matrix[0].sequence = -1;

    abq->currentNum = abq->totalNum;
}

int apple_bufque_space(AppleBufQue* abq)
{
    return abq->currentNum * abq->elemSize;
}

void apple_bufque_delete(AppleBufQue* abq)
{
    StrmBuffer *sb, *next;

    if (!abq)
        return;

    apple_bufque_reset(abq);

    sb = abq->pool;
    while (sb) {
        next = sb->next;
        strm_buf_free(sb);
        sb = next;
    }

    IND_FREE(abq);
}

static void int_ts_pat(char *packet, unsigned int conter, unsigned int pmtpid)
{
    unsigned int crc;
    unsigned char *p;

    ind_ts_header(packet, 0, 1, conter);
    packet[4] = 0;//pointer_field

    p = (unsigned char *)(packet + 5);

    p[0] = 0x00;//table_id -- program_association_section
    p[1] = 0x80 | 0x30;//section_syntax_indicator + reserved
    p[2] = 0x0d;//section_length
    p[3] = 0x26;
    p[4] = 0x11;//transport_stream_id
    p[5] = 0xc0 | 0x08 | 0x01;//reserved + version_number + current_next_indicator
    p[6] = 0x00;//section_number
    p[7] = 0x00;//last_section_number

    p[ 8] = 0x00;
    p[ 9] = 0x01;//program_number
    p[10] = 0xe0 | (unsigned char)(pmtpid >> 8);//reserved
    p[11] = (unsigned char)(pmtpid & 0xff);//program_map_PID

    crc = ind_ts_crc32(p, 12);
    p[12] = (unsigned char)(crc >> 24);
    p[13] = (unsigned char)(crc >> 16);
    p[14] = (unsigned char)(crc >> 8);
    p[15] = (unsigned char)(crc >> 0);//CRC

    IND_MEMSET(p + 16, 0xff, 183 - 16);
}

static void int_ts_pmt(char *packet, unsigned int conter, struct ts_psi* psi)
{
    unsigned int i, proglen, off, crc;
    unsigned char *p;

    ind_ts_header(packet, psi->pmt_pid, 1, conter);
    packet[4] = 0;//pointer_field

    p = (unsigned char *)(packet + 5);

    p[0] = 0x02;//table_id -- TS_program_map_section
    p[1] = 0x80 | 0x30;//section_syntax_indicator + reserved
    p[3] = 0x00;
    p[4] = 0x01;//program_number
    p[5] = 0xc0 | 0x02 | 0x01;//reserved + version_number + current_next_indicator
    p[6] = 0x00;//section_number
    p[7] = 0x00;//last_section_number

    p[ 8] = 0xe0 | (psi->pcr_pid >> 8);//reserved
    p[ 9] = (unsigned char)psi->pcr_pid;//PCR_PID
    p[10] = 0xf0;//reserved
    p[11] = 0x00;//program_info_length

    off = 12;
    if (psi->video_pid) {
        p[off ++] = (unsigned char)psi->video_type;//stream_type
        p[off ++] = 0xe0 | (psi->video_pid >> 8);//reserved
        p[off ++] = (unsigned char)psi->video_pid;//elementary_PID
        p[off ++] = 0xf0;//reserved
        p[off ++] = 0x00;//ES_info_length
    }
    for (i = 0; i < psi->audio_num; i ++) {
        p[off ++] = (unsigned char)psi->audio_type[i];//stream_type
        p[off ++] = 0xe0 | (psi->audio_pid[i] >> 8);//reserved
        p[off ++] = (unsigned char)psi->audio_pid[i];//elementary_PID
        p[off ++] = 0xf0;//reserved
        p[off ++] = 0x06;//ES_info_length
        p[off ++] = 0x0a;//descriptor_tag
        p[off ++] = 0x04;//descriptor_length
        p[off ++] = psi->audio_iso693[i].language[0];
        p[off ++] = psi->audio_iso693[i].language[1];
        p[off ++] = psi->audio_iso693[i].language[2];
        p[off ++] = psi->audio_iso693[i].type;
    }
    if (psi->dr_subtitle) {
        struct ts_subtitle* subtitle;
        ts_dr_subtitle_t dr_subtitle = psi->dr_subtitle;

        for (i = 0; i < dr_subtitle->subtitle_num; i ++) {
            subtitle = &dr_subtitle->subtitle[i];
            p[off ++] = ISO_IEC_PES_DATA;//stream_type
            p[off ++] = 0xe0 | (dr_subtitle->subtitle_pid[i] >> 8);//reserved
            p[off ++] = (unsigned char)dr_subtitle->subtitle_pid[i];//elementary_PID
            p[off ++] = 0xf0;//reserved
            p[off ++] = 0x0a;//ES_info_length
            p[off ++] = 0x59;//descriptor_tag
            p[off ++] = 0x08;//descriptor_length
            p[off ++] = subtitle->language[0];
            p[off ++] = subtitle->language[1];
            p[off ++] = subtitle->language[2];
            p[off ++] = 0x01;
            p[off ++] = 0x00;
            p[off ++] = 0x02;
            p[off ++] = 0x00;
            p[off ++] = 0x02;
        }
    }

    proglen = off - 12;
    p[2] = (unsigned char)(9 + proglen + 4);//section_length

    crc = ind_ts_crc32(p, off);
    p[off ++] = (unsigned char)(crc >> 24);
    p[off ++] = (unsigned char)(crc >> 16);
    p[off ++] = (unsigned char)(crc >> 8);
    p[off ++] = (unsigned char)(crc >> 0);//CRC

    IND_MEMSET(p + off, 0xff, 183 - off);
}

static int int_duration_push(HttpApple *ha)
{
    int length;
    long long duration;
    AppleMuxQue *amq;
    AppleBufQue *abq;

    abq = ha->abq;
    amq = &abq->matrix[0];
    length = strm_play_length(ha->strm_play);

    if (length > 0) {
        if (amq->valid) {
            AppleBuffer *ab = &amq->array[0];

            duration = (long long)amq->duration * ab->push_len / ab->head.length;

            if (length <= abq->push_length)
                duration = duration * length / abq->push_length;
            else
                duration += (long long)abq->prev_duration * (length - abq->push_length) / abq->prev_length;
        } else {
            if (abq->prev_length > 0)
                duration = (long long)abq->prev_duration * length / abq->prev_length;
            else
                duration = 0;
        }
    } else {
        duration = 0;
    }

    return (int)duration;
}

int apple_buffer_delay(HttpApple* ha)
{
    AppleBufQue *abq = ha->abq;

    if (abq->amq_index >= APPLE_BUFQUE_NUM - 1)
        return 1;
    if (abq->matrix[abq->amq_index].valid)
        return 1;

    return 0;
}

int apple_buffer_duration(HttpApple* ha)
{
    int i, idx;
    long long duration;

    AppleMuxQue *amq;
    AppleBufQue *abq;

    duration = int_duration_push(ha);

    abq = ha->abq;
    amq = &abq->matrix[0];
    if (amq->valid) {
        AppleBuffer *ab = &amq->array[0];

        duration += (long long)abq->matrix[0].duration * (ab->recv_len - ab->push_len) / ab->head.length;
    }

    idx = abq->amq_index;
    if (idx >= APPLE_BUFQUE_NUM)
        idx = APPLE_BUFQUE_NUM - 1;
    for (i = 1; i < idx; i++)
        duration += abq->matrix[i].duration;

    return (int)duration;
}

static void int_bufque_step(AppleBufQue *abq)
{
    int i;
    AppleBuffer *ab;
    AppleMuxQue *amq = &abq->matrix[0];

    abq->pmt_flag = 0;
    abq->pmt_counter = 0;
    for (i = 0; i < HAPPLE_TRACK_NUM; i ++) {
        ab = &amq->array[i];
        if (!ab->valid)
            continue;
        ab->push_step = (int)((long long)ab->head.length * abq->push_step / abq->push_range);
    }
}

static void int_push_ts(HttpApple *ha, int range)
{
    int i;
    StrmBuffer *sb;
    AppleBuffer *ab;
    AppleMuxQue *amq;
    AppleBufQue *abq;
    StrmBufHead *sbh;

    abq = ha->abq;
    amq = &abq->matrix[0];

    ab = &amq->array[0];
    sbh = &ab->head;

    while (range > 0 && strm_play_space(ha->strm_play) >= STREAM_BUFFER_SIZE * 2) {
        if ((ab->push_len < ab->push_step || ab->push_step >= ab->head.length) && sbh->head) {
            sb = strm_bufhead_pop(sbh);
            ab->push_len += sb->len;

            sb->off = 0;
            abq->push_length += sb->len;
            strm_play_push(ha->strm_play, ha->index, &sb);

            sb->next = abq->pool;
            abq->pool = sb;
            abq->currentNum++;
        } else {
            if (!sbh->head) {
                if (0 == abq->amq_index)
                    break;

                if (abq->push_step >= abq->push_range) {//下一个分片
                    abq->prev_length = abq->push_length;
                    abq->prev_bitrate = amq->bitrate;
                    abq->prev_duration = amq->duration;

                    abq->amq_index--;
                    abq->push_range = 0;

                    strm_bufhead_empty(abq, &ab->head);
                    for (i = 0; i < abq->amq_index; i++)
                         abq->matrix[i] = abq->matrix[i + 1];

                    if (i < APPLE_BUFQUE_NUM - 1) {
                        abq->matrix[i] = abq->matrix[i + 1];
                    } else {
                        memset(&abq->matrix[i], 0, sizeof(abq->matrix[i]));
                        abq->matrix[i].sequence = -1;
                    }

                    break;
                }
            }

            range--;
            abq->push_step++;
            int_bufque_step(abq);

            ab = &amq->array[0];
            sbh = &ab->head;
        }
    }
}

static void int_push_sb(HAppleTrack* track, StrmBuffer* sb)
{
    int ex;
    HttpApple *ha;
    AppleBufQue *abq;

    ha = track->ha;
    abq = ha->abq;

    if (1 == sb->off) {
        IND_MEMCPY(ha->sb->buf, sb->buf, sb->len);
        ha->sb->len = sb->len;
        ha->sb->off = 0;
        strm_buf_free(sb);
        sb = ha->sb;
        ex = 1;
    } else {
        ex = 0;
    }

    sb->off = 0;
    abq->push_length += sb->len;

    strm_play_push(ha->strm_play, ha->index, &sb);

    if (ex) {
        ha->sb = sb;
    } else {
        sb->next = abq->pool;
        abq->pool = sb;
        abq->currentNum++;
    }
}

static void int_push_track(HttpApple *ha, int range)
{
    int i;
    StrmBuffer *sb;
    AppleBuffer *ab;
    AppleMuxQue *amq;
    AppleBufQue *abq;
    StrmBufHead *sbh0;
    HAppleTrack *track;

    abq = ha->abq;
    amq = &abq->matrix[0];

    ab = &amq->array[abq->ab_index];
    sbh0 = &ab->head;
    track = ha->track_array[abq->ab_index];

    while (range > 0 && strm_play_space(ha->strm_play) >= STREAM_BUFFER_SIZE * 3) {
        if (0 == abq->pmt_flag) {
            sb = ha->sb;

            sb->len = 0;
            int_ts_pat(sb->buf + sb->len, abq->pmt_counter, ha->ts_psi.pmt_pid);
            sb->len += 188;
            int_ts_pmt(sb->buf + sb->len, abq->pmt_counter, &ha->ts_psi);
            sb->len += 188;
            abq->pmt_counter++;

            sb->off = 0;
            abq->push_length += sb->len;
            strm_play_push(ha->strm_play, ha->index, &ha->sb);

            abq->pmt_flag = 1;
        }

        if ((ab->push_len < ab->push_step || ab->push_step >= ab->head.length) && sbh0->head) {
            sb = strm_bufhead_pop(sbh0);
            ab->push_len += sb->len;

            int_push_sb(track, sb);
        } else {
            if (!sbh0->head) {
                if (abq->amq_index) {
                    strm_bufhead_empty(abq, &ab->switch_head);

                    if (abq->ab_index >= HAPPLE_TRACK_NUM - 1 && abq->push_step >= abq->push_range) {//下一个分片
                        abq->prev_length = abq->push_length;
                        abq->prev_bitrate = amq->bitrate;
                        abq->prev_duration = amq->duration;

                        abq->amq_index--;
                        abq->push_range = 0;

                        for (i = 0; i < HAPPLE_TRACK_NUM; i++) {
                            ab = &amq->array[i];
                            strm_bufhead_empty(abq, &ab->head);
                            strm_bufhead_empty(abq, &ab->switch_head);
                        }
                        for (i = 0; i < abq->amq_index; i++)
                                abq->matrix[i] = abq->matrix[i + 1];

                        if (i < APPLE_BUFQUE_NUM - 1) {
                            abq->matrix[i] = abq->matrix[i + 1];
                        } else {
                            memset(&abq->matrix[i], 0, sizeof(abq->matrix[i]));
                            abq->matrix[i].sequence = -1;
                        }
                        break;
                    }
                } else {// 0 == abq->amq_index 还未下载完全
                    if (track)
                       break;
                }
            }

            abq->ab_index++;
            if (abq->ab_index >= HAPPLE_TRACK_NUM) {
                abq->ab_index = 0;
                range--;
                abq->push_step++;
                int_bufque_step(abq);
                if (int_duration_push(ha) > APPLE_PUSH_10MS)
                    break;
            }
            ab = &amq->array[abq->ab_index];
            sbh0 = &ab->head;
            track = ha->track_array[abq->ab_index];
        }
    }
}

void apple_buffer_push(HttpApple* ha)
{
    AppleBuffer *ab;
    AppleTask *task;
    AppleMuxQue *amq;
    AppleBufQue *abq;
    StrmBufHead *sbh;
    HAppleTrack *track, **track_array;

    abq = ha->abq;
    track_array = ha->track_array;

    if (ha->end_flg < 2 && abq->amq_index < APPLE_BUFQUE_NUM) {
        amq = &abq->matrix[abq->amq_index];

        if (amq->valid) {
            int i, len;
            char *buf;
            ts_buf_t ts_buf;

            for (i = HAPPLE_TRACK_NUM - 1; i >= 0; i--) {
                track = track_array[i];
                if (!track)
                    continue;

                task = &track->task;
                ts_buf = task->recv_ts_buf;
#ifdef USE_VERIMATRIX_OTT
                if (track->key_uri)
                    ts_buf = task->crypt_ts_buf;
#endif

                ab = &amq->array[i];
                sbh = &ab->head;

                for (;;) {
                    len = 0;
                    ts_buf_reload_get(ts_buf, &buf, &len);
                    if (len < 16) {
                        if (len <= 0 || ab->valid < 2)
                            break;
                    } else {
                        len -= len % 16;
                    }
#ifdef USE_VERIMATRIX_OTT
                    if (track->key_uri) {
                        if (len < 16) {
                            IND_MEMSET(buf + len, 0, 16 - len);
                            ymm_stream_cryptM2MStream(task->crypt_handle, buf, 16, buf);
                        } else {
                            ymm_stream_cryptM2MStream(task->crypt_handle, buf, len, buf);
                        }
                    }
#endif
#ifdef ENABLE_SAVE_APPLE
                    if (0 == i)
                        fwrite(buf, len, 1, ha->save_fp);
#endif
                    ts_buf_reload_mark(ts_buf, len);
                }

                while (abq->pool && ts_buf_length(ts_buf) > 0) {
                    len = 0;
                    ts_buf_read_get(ts_buf, &buf, &len);
                    len -= len % 188;
                    if (len < 188)
                        break;

                    len = strm_bufhead_push(abq, sbh, track, buf, len);

                    ts_buf_read_pop(ts_buf, len);
                    ab->recv_len += len;
                }
                if (abq->pool && ab->valid >= 2 && ab->switch_head.head) {
                    if (track->switch_index == ab->head.index || track->switch_index != ab->switch_head.index) {
                        strm_bufhead_empty(abq, &ab->switch_head);
                    } else if (abq->amq_index > 0) {
                        strm_bufhead_empty(abq, &ab->head);
                        ab->head = ab->switch_head;
                        memset(&ab->switch_head, 0, sizeof(ab->switch_head));
                    }

                }
            }

            if (abq->pool && amq->valid >= 2) {
                abq->amq_index++;
                LOG_STRM_PRINTF("amq_index = %d\n", abq->amq_index);
                if (abq->amq_index < APPLE_BUFQUE_NUM) {
                    memset(&abq->matrix[abq->amq_index], 0, sizeof(abq->matrix[abq->amq_index]));
                    abq->matrix[abq->amq_index].sequence = -1;
                }
            }
        }

        if (1 == ha->end_flg && abq->pool && 0 == abq->amq_index)
            ha->end_flg = 2;
    }

    if (ha->end_flg < 3) {
        int range, duration;

        if (0 == strm_play_get_psi(ha->strm_play)) {
            range = APPLE_PUSH_10MS / 10;
        } else {
            duration = int_duration_push(ha);
            if (duration < APPLE_PUSH_10MS)
                range = (APPLE_PUSH_10MS - duration) / 10;
            else
                range = 0;
        }
        if (range > 0) {
            amq = &abq->matrix[0];
            if (!amq->valid) {
                if (2 == ha->end_flg) {
                    strm_play_end(ha->strm_play, ha->index);
                    ha->end_flg = 3;
                }
                return;
            }

            if (0 == abq->push_range) {
                abq->ab_index = 0;

                abq->push_step = 1;
                abq->push_range = amq->duration / 10;
                abq->push_length = 0;

                int_bufque_step(abq);
            }

            if (ha->alternative_audio)
                int_push_track(ha, range);
            else
                int_push_ts(ha, range);
        }
    }
}

void apple_buffer_slice_end(HAppleTrack* track)
{
    int i;
    HttpApple *ha;
    HLiveMedia *media;
    AppleBufQue *abq;
    HAppleTrack **track_array;

    ha = track->ha;

    track_array = ha->track_array;
    for(i = 0; i < HAPPLE_TRACK_NUM; i++) {
        track = track_array[i];
        if (!track)
            continue;
        if (LIVE_STATE_IDLE != track->state)
            return;
    }

    ha->load_sequence ++;

    abq = ha->abq;
    abq->matrix[abq->amq_index].valid = 2;

    apple_buffer_push(ha);

    media = ha->track_array[HAPPLE_TRACK_VIDEO]->task.media;
    if (media->slice_complete && ha->load_sequence >= media->x_sequence + media->slice_num && (0 == ha->servicetype || 3 == ha->servicetype)) {
        LOG_STRM_PRINTF("slice_complete\n");
        ha->end_flg = 1;
        return;
    }
    if (apple_buffer_delay(ha)) {
        ha->slice_clk = strm_httploop_clk(ha->loop) + ha->slice_duration * 100;
        return;
    }

    if (1 == ha->servicetype) {
        if (STRM_STATE_PLAY == ha->state) {
            if (http_apple_index_request(ha, 2))
                LOG_STRM_ERROR("#%d index_request\n", ha->index);
            ha->servicetype = 2;//直播转时移
        } else {
            //延迟半个分片周期请求
            apple_audio_m3u8_delay(ha);
        }
        return;
    }

    for(i = 0; i < HAPPLE_TRACK_NUM; i++) {
        track = track_array[i];
        if (!track)
            continue;
        media = track->task.media;
        if (ha->load_sequence >= media->x_sequence + media->slice_num) {
            apple_audio_m3u8_refresh(ha, 1);
            return;
        }
    }

    apple_audio_slice_download(ha);
}

void apple_buffer_slice_valid(HAppleTrack* track, int flag)
{
    HttpApple *ha;
    AppleBuffer *ab;
    AppleBufQue *abq;

    ha = track->ha;
    abq = ha->abq;

    switch (flag) {
    case 0:
        abq->matrix[abq->amq_index].sequence = ha->load_sequence;
        break;
    case 1:
        {
            int i, valid;
            AppleMuxQue *amq;

            amq = &abq->matrix[abq->amq_index];
            if (HAPPLE_TRACK_VIDEO == track->index) {
                if (!track->load_duration) {
                    LOG_STRM_ERROR("duration = %d\n", track->load_duration);
                    apple_audio_error(track);
                    return;
                }

                if (ha->stream)
                    amq->bitrate = ha->stream->bandwidth;
                amq->duration = track->load_duration * 100;
            }

            ab = &amq->array[track->index];
            ab->valid = 1;
            ab->head.length = track->load_length;

            valid = 1;
            for (i = 0; i < HAPPLE_TRACK_NUM; i++) {
                if (!ha->track_array[i])
                    continue;
                if (0 == amq->array[i].valid)
                    valid = 0;
            }
            if (valid)
                amq->valid = 1;
        }
        break;
    case 2:
        {
            ab = &abq->matrix[abq->amq_index].array[track->index];
            ab->valid = 2;
        }
        break;
    default:
        break;
    }
}

static void switch_check0(HAppleTrack* track, int idx)
{
    int i;
    AppleBuffer *ab;
    HLiveTrack *ltrak;
    AppleTask *task, *switch_task;
    AppleMuxQue *amq;
    AppleBufQue *abq;

    ltrak = track->ltrack;
    if (idx < 0 || idx >= ltrak->num || idx == track->switch_index)
        return;

    LOG_STRM_PRINTF("#%d %d > %d\n", track->index, track->switch_index, idx);
    track->switch_index = idx;

    abq = track->ha->abq;

    task = &track->task;
    switch_task = &track->switch_task;

    if ((switch_task->index < 0 && idx != task->index) || (switch_task->index >= 0 && idx != switch_task->index)) {
        if (switch_task->index >= 0)
            apple_switch_end(track);

        for (i = abq->amq_index; i > 0; i--) {
            if (i >= APPLE_BUFQUE_NUM)
                continue;
            if (0 == i && abq->push_range > 0 && abq->push_step + 20 > abq->push_range)
                continue;
            amq = &abq->matrix[i];
            if (amq->sequence < 0)
                continue;
            ab = &amq->array[track->index];
            if (idx == ab->head.index)
                continue;
            if (ab->switch_head.head && idx == ab->switch_head.index)
                continue;

            if (ab->switch_head.head)
                strm_bufhead_empty(abq, &ab->switch_head);
            switch_task->index = idx;
            track->switch_sequence = amq->sequence;
            apple_switch_begin(track);
            break;
        }
    }
}

static void switch_check1(HAppleTrack* track)
{
    int i;
    AppleBuffer *ab;
    AppleMuxQue *amq;
    AppleBufQue *abq;

    abq = track->ha->abq;

    for (i = abq->amq_index; i > 0; i--) {
        if (i >= APPLE_BUFQUE_NUM)
            continue;
        amq = &abq->matrix[i];
        if (amq->sequence < 0)
            continue;
        ab = &amq->array[track->index];
        if (track->switch_index == ab->head.index)
            continue;
        if (ab->switch_head.head && track->switch_index == ab->switch_head.index)
            continue;

        if (ab->switch_head.head)
            strm_bufhead_empty(abq, &ab->switch_head);
        if (0 == i && abq->push_range > 0 && abq->push_step + 20 > abq->push_range)
            continue;
        track->switch_sequence = amq->sequence;
        break;
    }
}

void apple_buffer_switch(HAppleTrack* track, char* buf, int len)
{
    int i;
    AppleBuffer *ab;
    AppleMuxQue *amq;
    AppleBufQue *abq;
    StrmBufHead *sbh;

    abq = track->ha->abq;

    for (i = abq->amq_index; i >= 0; i--) {
        if (i >= APPLE_BUFQUE_NUM)
            continue;
        amq = &abq->matrix[i];
        if (amq->sequence == track->switch_sequence)
            break;
    }
    if (i < 0)
        LOG_STRM_ERROUT("#%d timeout!\n", track->index);

    track->switch_sequence = -1;

    ab = &abq->matrix[i].array[track->index];
    if (i == abq->amq_index)
        sbh = &ab->switch_head;
    else if (i > 0)
        sbh = &ab->head;
    else if (abq->push_step + 10 < abq->push_range)
        sbh = &ab->switch_head;
    else
        sbh = NULL;
    LOG_STRM_PRINTF("#%d i = %d / %d, sbh = %p\n", track->index, i, abq->amq_index, sbh);

    if (sbh) {
        if (strm_bufhead_fill(abq, sbh, buf, len)) {
            HttpApple *ha = track->ha;

            strm_httploop_break(ha->loop);
            stream_post_msg(ha->index, STRM_MSG_OPEN_ERROR, 0);
            LOG_STRM_ERROUT("strm_bufhead_fill\n");
        }
        sbh->index = track->switch_task.index;
    }

    switch_check1(track);

Err:
    return;
}

void apple_buffer_1000ms(HttpApple* ha)
{
    int length, bitrate, audio_index, subtitle_index;
    AppleBufQue *abq;
    HAppleTrack *track;

    abq = ha->abq;
    length = strm_play_length(ha->strm_play);

    if (length <= abq->push_length)
        bitrate = abq->matrix[0].bitrate;
    else
        bitrate = abq->prev_bitrate;

    int_back_hls_playrate(bitrate);

    audio_index = -1;
    subtitle_index = -1;
    codec_alternative_get(&audio_index, &subtitle_index);
    {
        int diff, buffer, duration;

        diff = strm_play_diff(ha->strm_play);
        buffer = strm_play_buffer(ha->strm_play);
        duration = int_duration_push(ha);
        LOG_STRM_DEBUG("#%d duration = %d, buffer = %d, diff = %d, audio = %d, subtitle = %d\n",
            ha->index, duration, buffer, diff, audio_index, subtitle_index);
    }

    track = ha->track_array[HAPPLE_TRACK_VIDEO];
    if (ha->alternative_audio && STREAM_INDEX_PIP != ha->index && track->state != LIVE_STATE_INDEX) {
        track = ha->track_array[HAPPLE_TRACK_AUDIO];
        if (track)
            switch_check0(track, audio_index);
        track = ha->track_array[HAPPLE_TRACK_SUBTITLE];
        if (track)
            switch_check0(track, subtitle_index);
    }
}

#endif//#if SUPPORTE_HD == 1
