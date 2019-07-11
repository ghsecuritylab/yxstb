
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "app/Assertions.h"
#include "ind_ts.h"
#include "ind_mem.h"

#include <stdint.h>

struct ts_section {
    uint32_t        cnt;
    uint32_t        length;
    uint32_t        len;
    uint8_t*      section;
    uint8_t       buf[TS_SECTION_SIZE];
};
typedef struct ts_section* ts_section_t;

struct ts_parse {
    ts_psi_t    psi;
    uint32_t        video_pts;
    uint32_t        audio_pts;

    uint32_t        prog_number;

    struct ts_section    cat;
    struct ts_section    pmt;

    int         pmt_hdmv;/* Blu-Ray */

    uint32_t        pmt_len;
    uint8_t       pmt_buf[TS_SECTION_SIZE];
    uint32_t        pmt_crc;

    uint32_t        cat_len;
    uint8_t       cat_buf[TS_SECTION_SIZE];
    uint32_t        cat_crc;

    int         err_num;
};

static uint32_t g_crc32_table[256] =
{
  0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9,
  0x130476dc, 0x17c56b6b, 0x1a864db2, 0x1e475005,
  0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
  0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd,
  0x4c11db70, 0x48d0c6c7, 0x4593e01e, 0x4152fda9,
  0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
  0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011,
  0x791d4014, 0x7ddc5da3, 0x709f7b7a, 0x745e66cd,
  0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
  0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5,
  0xbe2b5b58, 0xbaea46ef, 0xb7a96036, 0xb3687d81,
  0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
  0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49,
  0xc7361b4c, 0xc3f706fb, 0xceb42022, 0xca753d95,
  0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
  0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d,
  0x34867077, 0x30476dc0, 0x3d044b19, 0x39c556ae,
  0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
  0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16,
  0x018aeb13, 0x054bf6a4, 0x0808d07d, 0x0cc9cdca,
  0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
  0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02,
  0x5e9f46bf, 0x5a5e5b08, 0x571d7dd1, 0x53dc6066,
  0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
  0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e,
  0xbfa1b04b, 0xbb60adfc, 0xb6238b25, 0xb2e29692,
  0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
  0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a,
  0xe0b41de7, 0xe4750050, 0xe9362689, 0xedf73b3e,
  0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
  0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686,
  0xd5b88683, 0xd1799b34, 0xdc3abded, 0xd8fba05a,
  0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
  0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb,
  0x4f040d56, 0x4bc510e1, 0x46863638, 0x42472b8f,
  0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
  0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47,
  0x36194d42, 0x32d850f5, 0x3f9b762c, 0x3b5a6b9b,
  0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
  0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623,
  0xf12f560e, 0xf5ee4bb9, 0xf8ad6d60, 0xfc6c70d7,
  0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
  0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f,
  0xc423cd6a, 0xc0e2d0dd, 0xcda1f604, 0xc960ebb3,
  0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
  0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b,
  0x9b3660c6, 0x9ff77d71, 0x92b45ba8, 0x9675461f,
  0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
  0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640,
  0x4e8ee645, 0x4a4ffbf2, 0x470cdd2b, 0x43cdc09c,
  0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
  0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24,
  0x119b4be9, 0x155a565e, 0x18197087, 0x1cd86d30,
  0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
  0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088,
  0x2497d08d, 0x2056cd3a, 0x2d15ebe3, 0x29d4f654,
  0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
  0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c,
  0xe3a1cbc1, 0xe760d676, 0xea23f0af, 0xeee2ed18,
  0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
  0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0,
  0x9abc8bd5, 0x9e7d9662, 0x933eb0bb, 0x97ffad0c,
  0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
  0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4
};

uint32_t ind_ts_crc32(const uint8_t *buf, uint32_t len)
{
    uint32_t i;
    /* Check the CRC_32 if b_syntax_indicator is 0 */
    uint32_t crc = 0xffffffff;

    for (i = 0; i < len; i ++)
        crc = (crc << 8) ^ g_crc32_table[(crc >> 24) ^ (buf[i])];
    return crc;
}

static int int_parse_crc(ts_parse_t parse, uint8_t *buf, int len, uint32_t* pcrc)
{
    uint32_t crc0, crc1;

    if (len <= 4 || len > TS_SECTION_SIZE)
        ERR_OUT("len = %d\n", len);
    crc0 = ind_ts_crc32(buf, len - 4);
    crc1 = ((uint32_t)(buf[len - 4]) << 24) 
         | ((uint32_t)(buf[len - 3]) << 16)
         | ((uint32_t)(buf[len - 2]) << 8)
         | ((uint32_t)(buf[len - 1]) << 0);
    if (crc0 != crc1)
        ERR_OUT("crc0 = 0x%08x, crc1 = 0x%08x\n", crc0, crc1);

    if (pcrc)
        *pcrc = crc0;

    return 0;
Err:
    parse->err_num ++;
    return -1;
}

static int int_parse_pat(uint8_t *buf, ts_parse_t parse)
{
    int len, l;
    uint32_t num, pid;
    struct ts_psi* psi = parse->psi;

    if (buf[3] & 0x20)//有调整字段
        len = 188 - (5 + buf[4]);
    else
        len = 188 - 4;
    buf += (188 - len);

    if (len < 1)
        ERR_OUT("len = %d\n", len);
    l = 1 + buf[0];//pointer_field
    len -= l;
    buf += l;
    if (len < 3)
        ERR_OUT("len = %d\n", len);

    l =(((uint32_t)buf[1] & 0x0f) << 8) + buf[2];//section_length
    if (len < l + 3)
        ERR_OUT("len = %d, length = %d\n", len, l);
    len = l + 3;
    if (len < 13)
        ERR_OUT("len = %d\n", len);

    if (int_parse_crc(parse, buf, len, NULL))
        ERR_OUT("int_parse_crc\n");

    len -= 8;
    buf += 8;

    for ( ;len >= 4 + 4; len -= 4, buf += 4) {
        num =(((uint32_t)buf[0] & 0x1f) << 8) + buf[1];//program_number
        if (num == 0)
            continue;
        pid =(((uint32_t)buf[2] & 0x1f) << 8) + buf[3];//program_map_PID
        if (pid != psi->pmt_pid || num != parse->prog_number) {
            parse->prog_number = num;
            parse->pmt.cnt = 16;
            psi->pmt_pid = pid;
        }
        return 0;
    }

    ERR_OUT("pmt_pid not found\n");
Err:
    return -1;
}

static void int_parse_dr_pmt(ts_parse_t parse, uint8_t *buf, uint32_t len)
{
    uint32_t tag, length;
    struct ts_psi* psi = parse->psi;

    while(len >= 2) {
        tag = (uint32_t)buf[0];
        length = (uint32_t)buf[1];

        buf += 2;
        len -= 2;
        if (length > len) {
            tag = 0;
            length = len;
        }

        switch(tag) {
        case 0x09:
            if (length >= 4) {
                if (psi->ecm_num < TS_ECM_NUM) {
                    ts_ca_t ca = &psi->ecm_array[psi->ecm_num];
                    ca->system_id = ((unsigned short)buf[0] << 8) | buf[1];
                    ca->pid = ((unsigned short)(buf[2] & 0x1f) << 8) | buf[3];
                    psi->ecm_num ++;
                } else {
                    WARN_PRN("ecm_num = %d\n", psi->ecm_num);
                }
            }
            break;
        case 0x05:
            if (length == 4) {
                //PRINTF("HDMV\n");
                if (memcmp(buf, "HDMV", 4) == 0)/* Blu-Ray */
                    parse->pmt_hdmv = 1;
            }
            break;
        default:
            break;
        }
        buf += length;
        len -= length;
    }
}

static int int_parse_dr_iso639(uint8_t *buf, uint32_t len, ts_iso693_t iso693, struct ts_psi* psi)
{
    uint32_t tag, length;

    if (iso693)
        IND_MEMSET(iso693, 0, sizeof(struct ts_iso693));

    while(len >= 2) {
        tag = (uint32_t)buf[0];
        length = (uint32_t)buf[1];

        buf += 2;
        len -= 2;
        if (len < length)
            ERR_OUT("len = %d, length = %d\n", len, length);

        if (iso693 && tag == 0x0A) {//iso639
            IND_MEMCPY(iso693->language, buf, 3);
            iso693->type = buf[3];
        } else if (psi && tag == 0x09) {
            if (psi->ecm_num < TS_ECM_NUM) {
                ts_ca_t ca = &psi->ecm_array[psi->ecm_num];
                ca->system_id = ((unsigned short)buf[0] << 8) | buf[1];
                ca->pid = ((unsigned short)(buf[2] & 0x1f) << 8) | buf[3];
                psi->ecm_num ++;
            } else {
                WARN_PRN("ecm_num = %d\n", psi->ecm_num);
            }
        }

        buf += length;
        len -= length;
    }

    return 0;
Err:
    return -1;
}

static int int_parse_dr_subtitle(uint8_t *buf, uint32_t len, struct ts_subtitle* subtitle)
{
    uint32_t tag, length;

    while(len >= 2) {
        tag = (uint32_t)buf[0];
        length = (uint32_t)buf[1];

        buf += 2;
        len -= 2;
        if (len < length)
            ERR_OUT("len = %d, length = %d\n", len, length);

        if (tag == 0x59) {
            IND_MEMCPY(subtitle->language, buf, 3);
            subtitle->type = buf[3];
            subtitle->composition = ((unsigned short)(buf[4]) << 8) | buf[5];
            subtitle->ancillary = ((unsigned short)(buf[6]) << 8) | buf[7];
            return 1;
        }

        buf += length;
        len -= length;
    }

    return 0;
Err:
    return -1;
}

static int int_parse_dr_teletext(uint8_t *buf, uint32_t len, struct ts_dr_teletext *dr_teletext)
{
    uint32_t tag, length;

    while(len >= 2) {
        tag = (uint32_t)buf[0];
        length = (uint32_t)buf[1];

        buf += 2;
        len -= 2;
        if (len < length)
            ERR_OUT("len = %d, length = %d\n", len, length);

        if (tag == 0x56) {
            dr_teletext->page_num = 0;
            while(length >= 5 && dr_teletext->page_num < 64) {
                struct ts_teletextpage *page = &dr_teletext->page[dr_teletext->page_num];
                IND_MEMCPY(page->language, buf, 3);
                page->type = buf[3] >> 3;
                page->magazine = buf[3] & 0x07;
                page->page = buf[4];
                dr_teletext->page_num ++;

                buf += 5;
                length -= 5;
            }
            return 1;
        }

        buf += length;
        len -= length;
    }

    return 0;
Err:
    return -1;
}

static int int_parse_dr_a52ac3(uint8_t *buf, uint32_t len)
{
    uint32_t tag, length;

    while(len >= 2) {
        tag = (uint32_t)buf[0];
        length = (uint32_t)buf[1];

        buf += 2;
        len -= 2;
        if (len < length)
            ERR_OUT("len = %d, length = %d\n", len, length);

        if (tag == 0x6A || tag == 0x81)//A52 参照VLC中的 modules\demux\ts.c
            return ISO_IEC_AC3_AUDIO;
        if (tag == 0x7A)//AC3
            return ISO_EXT_AC3_AUDIO;

        buf += length;
        len -= length;
    }

    return 0;
Err:
    return -1;
}

static int int_parse_pes_data(struct ts_psi* psi, uint32_t pid, uint8_t *buf, uint32_t len)
{
    int type;
    //PRINTF("descriptor_tag = 0x%02x, descriptor_length = %d\n", (uint32_t)buf[0], (uint32_t)buf[1]);

    if (psi->dr_subtitle) {
        struct ts_subtitle subtitle;
        struct ts_dr_subtitle *dr_subtitle = psi->dr_subtitle;

        if (int_parse_dr_subtitle(buf, len, &subtitle) == 1) {
            if (dr_subtitle->subtitle_num >= TS_SUBTITLE_NUM)
                return 0;
            dr_subtitle->subtitle_pid[dr_subtitle->subtitle_num] = pid;
            dr_subtitle->subtitle[dr_subtitle->subtitle_num] = subtitle;
            dr_subtitle->subtitle_num ++;
            return 0;
        }
    }
    if (psi->dr_teletext) {
        struct ts_dr_teletext *dr_teletext = psi->dr_teletext;

        if (int_parse_dr_teletext(buf, len, dr_teletext) == 1) {
            dr_teletext->pid = pid;
            return 0;
        }
    }
    type = int_parse_dr_a52ac3(buf, len);
    if (type > 0) {
        if (psi->audio_num >= TS_AUDIO_NUM)
            return 0;
        psi->audio_type[psi->audio_num] = type;
        psi->audio_pid[psi->audio_num] = pid;
        int_parse_dr_iso639(buf, len, &psi->audio_iso693[psi->audio_num], psi);
        psi->audio_num ++;
        return 0;
    }

    return 0;
}

static int int_parse_section(ts_parse_t parse, uint8_t *buf, uint32_t table_id, ts_section_t sct)
{
    uint32_t cnt, len, l;

    cnt = (uint32_t)(buf[3] & 0x0f);

    if (buf[1] & 0x40) {//start indicator
        sct->cnt = cnt;

        sct->length = 0;
        sct->len = 0;
    } else {
        if (sct->cnt >= 16)
            return 0;

        if ((sct->cnt + 1) % 16 != cnt) {
            WARN_PRN("cnt = %d, cnt = %d\n", sct->cnt, cnt);
            sct->cnt = 16;
            return 0;
        }
        sct->cnt = cnt;

        if (sct->length == 0)
            return 0;
    }

    if (buf[3] & 0x20) {//有调整字段
        len = 188 - 5;
        if (len <= buf[4]) {
            WARN_PRN("len = %d / %d\n", len, (uint32_t)buf[4]);
            return 0;
        }
        len -= buf[4];
    } else {
        len = 188 - 4;
    }
    buf += (188 - len);

    if (sct->length == 0) {
        l = 1 + buf[0];//pointer_field
        len -= l;
        buf += l;

        if (len < 3)
            ERR_OUT("len = %d\n", len);

        if ((uint32_t)buf[0] != table_id)
            ERR_OUT("table_id = %02x / %02x\n", (uint32_t)buf[0], table_id);
        if ((buf[1] & 0x80) == 0)
            ERR_OUT("not section_syntax_indicator\n");
        sct->length =(((uint32_t)buf[1] & 0x0f) << 8) + buf[2] + 3;
        if (sct->length > TS_SECTION_SIZE)
            ERR_OUT("pmt_length = %d\n", sct->length);

        sct->len = 0;

        if (sct->length <= len) {
            sct->section = buf;
            return 1;
        }
        sct->section = sct->buf;
    }

    l = sct->length - sct->len;
    if (len > l)
        len = l;
    IND_MEMCPY(sct->buf + sct->len, buf, len);
    sct->len += len;

    if (sct->len >= sct->length)
        return 1;

    return 0;
Err:
    parse->err_num ++;
    sct->length = 0;
    sct->len = 0;
    return -1;
}

static int int_parse_pmt(uint8_t *buf, ts_parse_t parse)
{
    uint32_t l, len, length;
    uint32_t type, pid, crc;
    struct ts_psi* psi = parse->psi;

    struct ts_dr_dvdspu *dr_dvdspu;

    //PRINTF("%02x %02x %02x %02x %02x %02x %02x %02x, pmt_len = %d / %d\n", (uint32_t)buf[0], (uint32_t)buf[1], (uint32_t)buf[2], (uint32_t)buf[3], (uint32_t)buf[4], (uint32_t)buf[5], (uint32_t)buf[6], (uint32_t)buf[7], h->pmt_len, h->pmt_length);

    if (int_parse_section(parse, buf, 2, &parse->pmt) != 1)
        return 0;

    buf = parse->pmt.section;
    len = parse->pmt.length;

    parse->pmt.cnt = 16;
    parse->pmt_hdmv = 0;

    if (int_parse_crc(parse, buf, len, &crc))
        ERR_OUT("int_parse_crc\n");

    if (parse->pmt_len != len || parse->pmt_crc != crc) {
        IND_MEMCPY(parse->pmt_buf, buf, len);
        parse->pmt_len = len;
        parse->pmt_crc = crc;
    }

    psi->pmt_crc = crc;

    buf += 3;
    len -= 3;

    pid = (((uint32_t)buf[0] & 0x1f) << 8) + buf[1];//program_number
    //PRINTF("@@@@@@@@: prog_number = %d / %d\n", (uint32_t)l, (uint32_t)parse->prog_number);
    if (pid != parse->prog_number) //2010-12-20 21:42:12 这会导致某些片源播放不出来 辽林现网又出现了该问题
        goto Err;

    if (0) {
        uint32_t current_next_indicator, section_number, last_section_number;
        current_next_indicator = (uint32_t)buf[2] & 0x01;
        section_number = (uint32_t)buf[3];
        last_section_number = (uint32_t)buf[4];
        PRINTF("current_next_indicator = %d\n", current_next_indicator);
        PRINTF("section_number = %d\n", section_number);
        PRINTF("last_section_number = %d\n", last_section_number);
    }

    //psi->program_number = (((uint16_t)buf[0] & 0x1f) << 8) + buf[1];
    //psi->pmt_version = ((uint32_t)buf[2] & 0x3e) >> 1;
    psi->pcr_pid = (((uint32_t)buf[5] & 0x1f) << 8) + buf[6];//PCR_PID
    l =(((uint32_t)buf[7] & 0x0f) << 8) + buf[8];//program_info_length
    buf += 9;
    len -= 9;

    if (len < l + 4)
        ERR_OUT("len = %d, length = %d\n", len, l);

    psi->ecm_num = 0;

    if (l > 0) {
        int_parse_dr_pmt(parse, buf, l);
        buf += l;
        len -= l;
    }

    psi->video_pid = 0;
    psi->audio_num = 0;

    if (psi->dr_subtitle)
        psi->dr_subtitle->subtitle_num = 0;
    if (psi->dr_teletext)
        psi->dr_teletext->page_num = 0;
    if (psi->dr_dvdspu)
        psi->dr_dvdspu->dvdspu_num = 0;

    while(len >= 5 + 4) {
        type = buf[0];
        pid = (((uint32_t)buf[1] & 0x1f) << 8) + buf[2];
        length =(((uint32_t)buf[3] & 0x0f) << 8) + buf[4];
        //PRINTF("@@@@@@@@: stream_type = %d, elementary_PID = %d, ES_info_length = %d\n", type, pid, l);
        len -= 5;
        buf += 5;

        if (type == ISO_IEC_11172_AUDIO)
            type = ISO_IEC_13818_3_AUDIO;
        switch(type)
        {
        case ISO_IEC_11172_VIDEO:
        case ISO_IEC_13818_2_VIDEO:
        case ISO_IEC_MPEG4_VIDEO:
        case ISO_IEC_H264:
        case ISO_IEC_H264_SVC:
        case ISO_IEC_H264_MVC:
        case ISO_IEC_AVS_VIDEO:
        case ISO_IEC_VC1:
        case ISO_IEC_VC1_SM:
        case ISO_IEC_DIVX:
            psi->video_type = type;
            psi->video_pid = pid;
            int_parse_dr_iso639(buf, length, NULL, psi);
            break;
        case ISO_IEC_13818_3_AUDIO:
        case ISO_IEC_13818_7_AUDIO:
        case ISO_IEC_MPEG4_AUDIO:
        case ISO_IEC_AVS_AUDIO:
        case ISO_IEC_AC3_AUDIO:
Audio:
            if (psi->audio_num >= TS_AUDIO_NUM) {
                //WARN_PRN("audio_num = %d\n", psi->audio_num);
                break;
            }
            psi->audio_type[psi->audio_num] = type;
            psi->audio_pid[psi->audio_num] = pid;
            int_parse_dr_iso639(buf, length, &psi->audio_iso693[psi->audio_num], psi);
            psi->audio_num ++;
            break;
        case 0x82:/* DVD_SPU (sub) DVD字幕实际无应用，我们不支持*/
            if (parse->pmt_hdmv) {
                type = ISO_EXT_DTS_AUDIO;
                goto Audio;
            }

            //PRINTF("descriptor_tag = 0x%02x, descriptor_length = %d\n", (uint32_t)buf[0], (uint32_t)buf[1]);
            dr_dvdspu = psi->dr_dvdspu;
            if (dr_dvdspu == NULL)
                break;
            if (dr_dvdspu->dvdspu_num >= TS_SUBTITLE_NUM) {
                //WARN_PRN("subtitle_num = %d\n", psi->dvdspu_num);
                break;
            }
            dr_dvdspu->dvdspu_pid[dr_dvdspu->dvdspu_num] = pid;
            int_parse_dr_iso639(buf, length, &dr_dvdspu->dvdspu_iso693[dr_dvdspu->dvdspu_num], NULL);
            dr_dvdspu->dvdspu_num ++;
            break;

        case 0x85: /* DTS-HD High resolution audio */
        case 0x86: /* DTS-HD Master audio */
        case 0xA2: /* Secondary DTS audio */
            if (parse->pmt_hdmv) {
                type = ISO_EXT_DTS_AUDIO;
                goto Audio;
            }
            break;

        case 0x84: /* E-AC3 */
        case 0xA1: /* Secondary E-AC3 */
            if (parse->pmt_hdmv) {
                type = ISO_EXT_AC3_AUDIO;
                goto Audio;
            }
            break;

        case ISO_IEC_PES_DATA:
            int_parse_pes_data(psi, pid, buf, length);
            break;
        case ISO_IEC_13818_1_PRI:
            break;
        case ISO_IEC_13818_6_A:
            break;
        case ISO_IEC_13818_6_B:
            break;
        case ISO_IEC_13818_6_C:
            break;
        case ISO_IEC_13818_6_D:
            break;
        default:
            //PRINTF("stream type = %d\n", type);
            break;
        }

        if (len < length + 4)
            ERR_OUT("len = %d, l = %d\n", len, length);
        len -= length;
        buf += length;
    }
    if (len != 4)
        ERR_OUT("len = %d\n", len);

    parse->pmt.cnt = 16;
    return 1;
Err:
    parse->pmt.cnt = 16;
    return 0;
}

static void int_parse_cat(uint8_t *buf, ts_parse_t parse)
{
    uint32_t len, l, crc;
    struct ts_psi* psi = parse->psi;

    if (int_parse_section(parse, buf, 1, &parse->cat) != 1)
        return;

    buf = parse->cat.section;
    len = parse->cat.length;

    if (int_parse_crc(parse, buf, len, &crc))
        ERR_OUT("int_parse_crc\n");

    if (parse->cat_len != len || parse->pmt_crc != crc) {
        IND_MEMCPY(parse->cat_buf, buf, len);
        parse->cat_len = len;
        parse->cat_crc = crc;
    }

    //psi->cat_crc = crc;
    //psi->cat_version = ((uint32_t)buf[5] & 0x3e) >> 1;

    psi->emm_num = 0;

    if (len >= 8 + 6) {
        uint32_t tag;

        buf += 8;
        len -= 8;

        while(len >= 6) {
            tag = (uint32_t)buf[0];
            l = (uint32_t)buf[1];
            buf += 2;
            len -= 2;

            if (tag == 0x09 && l >= 4) {
                if (psi->emm_num < TS_EMM_NUM) {
                    ts_ca_t ca = &psi->emm_array[psi->emm_num];
                    ca->system_id = ((unsigned short)buf[0] << 8) | buf[1];
                    ca->pid = ((unsigned short)(buf[2] & 0x1f) << 8) | buf[3];
                    if ((ca->system_id & 0xff00) == 0x0b00)/* CONAX */
                        psi->emm_num ++;
                } else {
                    WARN_PRN("emm_num = %d\n", psi->emm_num);
                }
            }

            if (l >= len)
                break;
            buf += l;
            len -= l;
        }
    }

Err:
    parse->cat.length = 0;
}

uint32_t ts_parse_pmt_prognum(uint8_t *buf)
{
    uint32_t prognum, len, l;

    if ((buf[1] & 0x40) == 0)//start indicator
        ERR_OUT("not start\n");

    if (buf[3] & 0x20)//有调整字段
        len = 188 - (5 + buf[4]);
    else
        len = 188 - 4;
    buf += (188 - len);
    if (len < 1)
        ERR_OUT("len = %d\n", len);

    l = 1 + buf[0];//pointer_field
    len -= l;
    buf += l;
    if (len < 5)
        ERR_OUT("len = %d\n", len);

    if (buf[0] != 0x02)
        ERR_OUT("not TS_program_map_section\n");
    if ((buf[1] & 0xCC) != 0x80)
        ERR_OUT("not section_syntax_indicator\n");

    prognum = (((uint32_t)buf[3] & 0x1f) << 8) + buf[4];//program_number

    return prognum;
Err:
    return -1;
}

ts_parse_t ts_parse_create(struct ts_psi* psi)
{
    ts_parse_t parse = (ts_parse_t)IND_CALLOC(sizeof(struct ts_parse), 1);
    if (parse == NULL)
        return NULL;
    parse->psi = psi;

    return parse;
}

void ts_parse_reset(ts_parse_t parse)
{
    if (parse == NULL)
        return;

    parse->psi->pmt_pid = 0;

    parse->cat.cnt = 16;
    parse->pmt.cnt = 16;

    parse->pmt_len = 0;

    parse->video_pts = 0;
    parse->audio_pts = 0;

    parse->err_num = 0;
}

int ts_parse_error(ts_parse_t parse)
{
    int err_num = parse->err_num;
    parse->err_num = 0;
    return err_num;
}

void ts_parse_delete(ts_parse_t parse)
{
    if (parse == NULL)
        return;
    IND_FREE(parse);
}

uint32_t ts_parse_pts(uint8_t *buf)
{
	uint8_t *p;
	int l;
	uint32_t pts;

	if (buf[0] != 0x47)
		return 0;

	if (buf[3] & 0x20) {//有调整字段
		l = 188 - (5 + buf[4]);
		if (l < 14)
			return 0;
	} else {
		l = 188 - 4;
	}

	p = buf + 188 - l;
	if (p[0] != 0 || p[1] != 0 || p[2] != 1)
		return 0;//非前导码
	if (p[3] < 0xC0 || p[3] > 0xEF)
		return 0;//非音视频流PES
	if (((p[7] & 0x80) == 0x80) && ((p[9] & 0x20) == 0x20)) {
		pts =	(uint32_t)(p[ 9] & 0x0E) << (32 - 3 - 1) |
				(uint32_t)(p[10]       ) << (29 - 7 - 1) |
				(uint32_t)(p[11] & 0xFE) << (21 - 7 - 1) |
				(uint32_t)(p[12]       ) << (14 - 7 - 1) |
				(uint32_t)(p[13] & 0xFE) >> 2;
		return pts;
	}
	return 0;
}

int ts_parse_psi(ts_parse_t parse, uint8_t *buf, int len, struct ts_pts* tsp)
{
    int n, ok, off;
    uint32_t pid;
    struct ts_psi* psi;

    ok = 0;

    if (parse == NULL)
        ERR_OUT("handle is NULL\n");

    psi = parse->psi;

    ok = 0;
    n = 0;

    if (len % 188)
        ERR_OUT("len = %d\n", len);
    for (off = 0; off < len; off += 188, buf += 188, n ++) {
        //检测TS包头
        if (buf[0] != 0x47)
            ERR_OUT("n = %d buf[0] = %x\n", n, (uint32_t)buf[0]);

        pid =(((uint32_t)buf[1] & 0x1f) << 8) + buf[2];
        if (pid == 0) {
            int_parse_pat(buf, parse);
        } else if (pid == 1) {
            int_parse_cat(buf, parse);
        } else if (pid == psi->pmt_pid) {
            if (int_parse_pmt(buf, parse) == 1)
                ok = 1;
        } else if (tsp) {
            uint32_t pts;

            if (pid == psi->pcr_pid && (buf[3] & 0x20))
                ts_pts_input_pcr(tsp, buf);

            if (buf[1] & 0x40) {
                if (psi->video_pid && pid == psi->video_pid) {
                    pts = ts_parse_pts(buf);
                    if (pts)
                        ts_pts_input(tsp, pts);
                }
                if (!psi->video_pid && psi->audio_num > 0 && pid == psi->audio_pid[0]) {
                    pts = ts_parse_pts(buf);
                    if (pts)
                        ts_pts_input(tsp, pts);
                }
            }
        }
    }

    return ok;
Err:
    return -1;
}

int ts_parse_getpmt(ts_parse_t parse, char* pmt_buf, int size)
{
    int len = 0;

    if (parse == NULL || pmt_buf == NULL)
        ERR_OUT("parse = %p, pmt_buf = %p\n", parse, pmt_buf);

    if (parse->pmt_len <= 0)
        goto Err;

    len = parse->pmt_len;
    IND_MEMCPY(pmt_buf, parse->pmt_buf, len);

Err:
    return len;
}

int ts_parse_getcat(ts_parse_t parse, char* cat_buf, int size)
{
    int len = 0;

    if (parse == NULL || cat_buf == NULL)
        ERR_OUT("parse = %p, pmt_buf = %p\n", parse, cat_buf);

    if (parse->cat_len <= 0)
        goto Err;

    len = parse->cat_len;
    IND_MEMCPY(cat_buf, parse->cat_buf, len);

Err:
    return len;
}

int ts_cat_equal(struct ts_psi* src_psi, struct ts_psi* dst_psi)
{
    int i, j, num;

    if (src_psi->emm_num != dst_psi->emm_num)
        WARN_OUT("emm_num = %d / %d\n", src_psi->emm_num, dst_psi->emm_num);

    num = src_psi->emm_num;
    for (i = 0; i < num; i ++) {
        for (j = 0; j < num; j ++) {
            if (src_psi->emm_array[i].pid == dst_psi->emm_array[j].pid)
                break;
        }
        if (j >= num)
            WARN_OUT("src_psi->emm_pid[%d] = %d\n", i, src_psi->emm_array[i].pid);
    }

    return 1;
Warn:
    return 0;
}

int ts_psi_equal(struct ts_psi* srcpsi, struct ts_psi* dstpsi)
{
    int i;

    if (srcpsi == NULL || dstpsi == NULL)
        WARN_OUT("srcpsi = %p, dstpsi = %p\n", srcpsi, dstpsi);

    if (srcpsi->video_pid != dstpsi->video_pid)
        WARN_OUT("video_pid: src = %d, dst = %d\n", srcpsi->video_pid, dstpsi->video_pid);
    if (srcpsi->video_pid && srcpsi->video_type != dstpsi->video_type)
        WARN_OUT("video_pid: src = %d, dst = %d\n", srcpsi->video_type, dstpsi->video_type);

    if (srcpsi->audio_num != dstpsi->audio_num)
        WARN_OUT("audio_num: src = %d, dst = %d\n", srcpsi->audio_num, dstpsi->audio_num);
    for (i = 0; i < srcpsi->audio_num; i ++) {
        if (srcpsi->audio_pid[i] != dstpsi->audio_pid[i])
            WARN_OUT("audio_pid[%d]: src = %d, dst = %d\n", i, srcpsi->audio_pid[i], dstpsi->audio_pid[i]);
        if (srcpsi->audio_type[i] != dstpsi->audio_type[i])
            WARN_OUT("audio_type[%d]: src = %d, dst = %d\n", i, srcpsi->audio_type[i], dstpsi->audio_type[i]);
    }
    /*
    if (srcpsi->dr_subtitle && dstpsi->dr_subtitle && srcpsi->dr_subtitle->subtitle_num != dstpsi->dr_subtitle->subtitle_num)
        WARN_PRN("subtitle_num = %d / %d\n", srcpsi->dr_subtitle->subtitle_num, dstpsi->dr_subtitle->subtitle_num);
    if (srcpsi->dr_teletext && dstpsi->dr_teletext && srcpsi->dr_teletext->page_num != dstpsi->dr_teletext->page_num)
        WARN_PRN("page_num = %d / %d\n", srcpsi->dr_teletext->page_num, dstpsi->dr_teletext->page_num);
    */

    return 1;
Warn:
    return 0;
}

int ts_psi_copy(struct ts_psi* dst_psi, struct ts_psi* src_psi)
{
    int i, num;

    if (src_psi == NULL || dst_psi == NULL)
        WARN_OUT("srcpsi = %p, dstpsi = %p\n", src_psi, dst_psi);

    dst_psi->pcr_pid    = src_psi->pcr_pid;
    dst_psi->pmt_pid    = src_psi->pmt_pid;

    dst_psi->video_pid    = src_psi->video_pid;
    dst_psi->video_type    = src_psi->video_type;
    dst_psi->audio_num    = src_psi->audio_num;

    num = src_psi->audio_num;
    for (i = 0; i < num; i ++) {
        dst_psi->audio_pid[i]    = src_psi->audio_pid[i];
        dst_psi->audio_type[i]    = src_psi->audio_type[i];
        dst_psi->audio_iso693[i]= src_psi->audio_iso693[i];
    }

    {
        struct ts_dr_subtitle *src_subtitle, *dst_subtitle;

        dst_subtitle = dst_psi->dr_subtitle;
        src_subtitle = src_psi->dr_subtitle;

        if (dst_subtitle) {
            if (src_subtitle) {
                num = src_subtitle->subtitle_num;
                dst_subtitle->subtitle_num = num;
                for (i = 0; i < num; i ++) {
                    dst_subtitle->subtitle_pid[i]    = src_subtitle->subtitle_pid[i];
                    dst_subtitle->subtitle[i]        = src_subtitle->subtitle[i];
                }
            } else {
                dst_subtitle->subtitle_num = 0;
            }
        }
    }

    {
        struct ts_dr_teletext *dst_teletext, *src_teletext;
    
        dst_teletext = dst_psi->dr_teletext;
        src_teletext = src_psi->dr_teletext;
        if (dst_teletext) {
            if (src_teletext) {
                dst_teletext->pid = src_teletext->pid;
                dst_teletext->page_num = src_teletext->page_num;
                num = src_teletext->page_num;
                for (i = 0; i < num; i ++)
                    dst_teletext->page[i] = src_teletext->page[i];
            } else {
                dst_teletext->pid = 0;
                dst_teletext->page_num = 0;
            }
        }
    }

    dst_psi->pmt_crc = src_psi->pmt_crc;

    {
        num = src_psi->ecm_num;
        dst_psi->ecm_num    = num;
        for (i = 0; i < num; i ++)
            dst_psi->ecm_array[i] = src_psi->ecm_array[i];

        num = src_psi->emm_num;
        dst_psi->emm_num = num;
        for (i = 0; i < num; i ++)
            dst_psi->emm_array[i] = src_psi->emm_array[i];
    }

    return 1;
Warn:
    return 0;
}

void ts_psi_print(struct ts_psi* psi)
{
    int i;

    if (psi == NULL)
        return;
    PRINTF("video_pid = %d\n", psi->video_pid);
    PRINTF("video_type = %d\n", psi->video_type);

    PRINTF("audio_num = %d\n", psi->audio_num);
    for (i = 0; i < psi->audio_num; i ++) {
        PRINTF("audio_pid[%d] = %d\n", i, psi->audio_pid[i]);
        PRINTF("audio_type[%d] = %d\n", i, psi->audio_type[i]);
    }
    if (psi->ecm_num > 0)
        PRINTF("ca system_id = %d\n", (uint32_t)psi->ecm_array[0].system_id);

    PRINTF("pcr_pid = %d\n", psi->pcr_pid);
    PRINTF("pmt_pid = %d\n", psi->pmt_pid);
}


/*
    加密流检测是帧数据（可以检测I帧）加密还是TS负载层加密
    -1：为TS负载
    0：正在检测
    1：帧数据层
 */
int ts_index_check(char *buf, int len, uint32_t vpid, uint32_t apid)
{
    int l, hl, off;
    uint8_t *p;
    uint32_t pid;

    if (len % 188)
        WARN_OUT("len = %d\n", len);

    for (off = 0; off < len; off += 188) {
        p = (uint8_t *)(buf + off);
        if (p[0] != 0x47)
            WARN_OUT("ts sync byte!\n");
        l = 188;

        pid =(((uint32_t)p[1] & 0x1f) << 8) + p[2];
        if (vpid) {
            if (pid != vpid)
                continue;
        } else {
            if (pid != apid)
                continue;
        }

        if ((p[1] & 0x40) == 0) //start indicator
            continue;

        if (p[3] & 0x20) //adaptation field
            hl = 5 + (int)((uint32_t)p[4]);
        else
            hl = 4;
        p += hl;
        l -= hl;

        if (l < 14) {//pes 负载过短
            WARN_PRN("l = %d\n", l);
            continue;
        }

        if (p[0] != 0 || p[1] != 0 || p[2] != 1)
            WARN_OUT("prefix1 %02x%02x%02x\n", p[0], p[1], p[2]);

        if (p[3] < 0xC0 || p[3] > 0xEF)
            WARN_OUT("stream id\n");

        if ((p[6] & 0xC0) != 0x80)
            WARN_OUT("PES header error\n");

        hl = (int)((uint32_t)p[8]) + 9;
        p += hl;
        l -= hl;

        if (l < 4)
            WARN_OUT("l = %d\n", l);

        if (vpid == 0)
            return 1;

        if (p[0] != 0 || p[1] != 0 || (p[2] != 0 && p[2] != 1))
            WARN_OUT("prefix2 %02x%02x%02x\n", p[0], p[1], p[2]);

        return 1;
    }

    return 0;
Warn:
    return -1;
}
