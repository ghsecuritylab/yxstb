
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "app/Assertions.h"
#include "ind_ts.h"

#include "ts_size.h"

static unsigned char bit8[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
uint32_t ts_u_v(struct u_bits *bits, int n)
{
	bits->ue = 0;
	while(n > 0) {
		bits->ue <<= 1;
		if (bits->buf[bits->idx] & bit8[bits->bit])
			bits->ue ++;

		if (bits->bit < 7) {
			bits->bit ++;
		} else {
			bits->idx ++;
			if (bits->idx >= bits->len)
				ERR_OUT("len = %d, idx = %d\n", bits->len, bits->idx);
			bits->bit = 0;
		}
		n --;
	}

	return 0;
Err:
	return -1;
}

int ts_ue_v(struct u_bits *bits)
{
	unsigned int b;
	int leadingZeroBits = -1; 

	for (b = 0; !b; leadingZeroBits ++) {
		if (ts_u_v(bits, 1))
			ERR_OUT("ts_u_v\n");
		b = bits->ue;
	}

	bits->ue = 0;
	if (ts_u_v(bits, leadingZeroBits))
		ERR_OUT("ts_u_v codeNum\n");
	bits->ue = (1 << leadingZeroBits) - 1 + bits->ue;

	return 0;
Err:
	return -1;
}

int ts_se_v(struct u_bits *bits)
{
	if (ts_ue_v(bits))
		ERR_OUT("ts_ue_v\n");

	if (bits->ue & 0x1)
		bits->se = (int)((bits->ue + 1) >> 2);
	else
		bits->se = -1 * (int)(bits->ue >> 2);

	return 0;
Err:
	return -1;
}

#if 0
static unsigned int _log2(unsigned int value)
{
	unsigned int n = 0;

	while(value) {
		value >>= 1;
		n ++;
	}

	return n;
}

static int parse_h264_scale(struct u_bits *bits, int sizeOfScalingList)
{
	int j;
	int delta_scale, lastScale, nextScale;

	lastScale = 8;
	nextScale = 8;

	for (j = 0; j < sizeOfScalingList; j ++) {
		if (nextScale != 0) {
			if (ts_se_v(bits))
				ERR_OUT("ts_se_v\n");
			delta_scale = bits->se;
			nextScale = (lastScale + delta_scale + 256) % 256;
		}
		lastScale = (nextScale == 0) ? lastScale : nextScale;
	}

	return 0;
Err:
	return -1;
}

static int parse_h264_seq(unsigned char *buf, int len, int* pwidth, int* pheight)
{
	unsigned int profile_idc;
	int i, width, height;
	struct u_bits bits;

	if (len <= 3)
		ERR_OUT("len = %d\n", len);
	profile_idc = (unsigned int)buf[0];
	if (profile_idc != 66 && profile_idc != 77 && profile_idc != 88 
		&& profile_idc != 100 && profile_idc != 110 && profile_idc != 122 && profile_idc != 144)
		ERR_OUT("profile_idc = %d\n", profile_idc);

	bits.buf = buf + 3;
	bits.len = len - 3;
	bits.idx = 0;
	bits.bit = 0;

	if (ts_ue_v(&bits))
		ERR_OUT("seq_parameter_set_id\n");

	if (profile_idc == 100 || profile_idc == 110 || profile_idc == 122 || profile_idc == 144) {
		if (ts_ue_v(&bits))
			ERR_OUT("chroma_format_idc\n");
		if (bits.ue == 3) {
			if (ts_u_v(&bits, 1))
				ERR_OUT("residual_colour_transform_flag\n");
		}
		if (ts_ue_v(&bits))
			ERR_OUT("bit_depth_luma_minus8\n");
		if (ts_ue_v(&bits))
			ERR_OUT("bit_depth_chroma_minus8\n");
		if (ts_u_v(&bits, 1))
			ERR_OUT("qpprime_y_zero_transform_bypass_flag\n");
		if (ts_u_v(&bits, 1))
			ERR_OUT("seq_scaling_matrix_present_flag\n");
		if (bits.ue) {
			for (i = 0; i < 8; i ++) {
				if (ts_u_v(&bits, 1))
					ERR_OUT("seq_scaling_list_present_flag[%d]\n", i);
				if (bits.ue) {
					if (i < 6) {
						if (parse_h264_scale(&bits, 16))
							ERR_OUT("parse_h264_scale %d\n", i);
					} else {
						if (parse_h264_scale(&bits, 64))
							ERR_OUT("parse_h264_scale %d\n", i);
					}
				}
			}
		}
	}

	if (ts_ue_v(&bits))
		ERR_OUT("log2_max_frame_num_minus4\n");
	if (ts_ue_v(&bits))
		ERR_OUT("pic_order_cnt_type\n");
	if (bits.ue == 0) {
		if (ts_ue_v(&bits))
			ERR_OUT("log2_max_pic_order_cnt_lsb_minus4\n");
	} else if (bits.ue == 1) {
		int num;
		if (ts_u_v(&bits, 1))
			ERR_OUT("delta_pic_order_always_zero_flag\n");
		if (ts_se_v(&bits))
			ERR_OUT("offset_for_non_ref_pic\n");
		if (ts_se_v(&bits))
			ERR_OUT("offset_for_top_to_bottom_field\n");
		if (ts_ue_v(&bits))
			ERR_OUT("num_ref_frames_in_pic_order_cnt_cycle\n");
		num = bits.ue;
		for (i = 0; i < num; i ++) {
			if (ts_se_v(&bits))
				ERR_OUT("offset_for_ref_frame[%d]\n", i);
		}
	}
	if (ts_ue_v(&bits))
		ERR_OUT("num_ref_frames\n");
	if (ts_u_v(&bits, 1))
		ERR_OUT("gaps_in_frame_num_value_allowed_flag\n");
	if (ts_ue_v(&bits))
		ERR_OUT("pic_width_in_mbs_minus1\n");
	width = (bits.ue + 1) * 16;
	PRINTF("pic_width_in_mbs_minus1 = %d / %d\n", bits.ue, width);
	if (pwidth)
		*pwidth = width;
	if (ts_ue_v(&bits))
		ERR_OUT("pic_height_in_map_units_minus1\n");
	height = (bits.ue + 1) * 16;
	PRINTF("pic_height_in_map_units_minus1 = %d / %d\n", bits.ue, height);
	if (pheight)
		*pheight = height;

	return 1;
Err:
	return -1;
}

static int parse_h264(unsigned char *buf, int len, int* pwidth, int* pheight)
{
	char ch;
	int i;
	int nal_prefix, nal_type;

	nal_prefix = 0;
	for (i = 0; i < len; i ++) {
		ch = buf[i];
		if (nal_prefix == 3) {
			nal_type = ch & 0x1f;

			if (nal_type <= 5)
				return 0;
			if (nal_type == 7)
				return parse_h264_seq(buf + i + 1, len - i - 1, pwidth, pheight);
			nal_prefix = 0;
		}
		switch(ch) {
		case 0:
			if (nal_prefix < 2)
				nal_prefix ++;
			break;
		case 1:
			if (nal_prefix == 2)
				nal_prefix = 3;
			else
				nal_prefix = 0;
			break;
		default:
			nal_prefix = 0;
			break;
		}
	}

	return 0;
}

static int parse_mpeg2(unsigned char *buf, int len, int* pwidth, int* pheight)
{
	unsigned int width, height;
	if (len < 7)
		ERR_OUT("len = %d\n", len);
	if (buf[0] != 0 || buf[1] != 0 || buf[2] != 1 || buf[3] != 0xB3)
		return 0;
	width = ((unsigned int)buf[4] << 4) + ((unsigned int)buf[5] >> 4);
	height = (((unsigned int)buf[5] & 0xf) << 8) + (unsigned int)buf[6];
	PRINTF("horizontal_size_value = %d\n", width);
	if (pwidth)
		*pwidth = width;
	PRINTF("vertical_size_value = %d\n", height);
	if (pheight)
		*pheight = height;

	return 1;
Err:
	return -1;
}

static int parse_mpeg4(unsigned char *buf, int len)
{
	unsigned int width, height, startcode;
	unsigned int vop_time_increment_resolution;
	struct u_bits bits;

	if (len < 6)
		ERR_OUT("len = %d\n", len);
	if (buf[0] != 0 || buf[1] != 0 || buf[2] != 1)
		return 0;
	for (; len > 4; len --, buf ++) {
		if (buf[0] != 0 || buf[1] != 0 || buf[2] != 1)
			continue;
		startcode = buf[3];
		if (startcode == 0xB0)
			PRINTF("Visual Object Sequence\n");
		else if (startcode == 0xB2)
			PRINTF("User Data\n");
		else if (startcode == 0xB3)
			PRINTF("Group of Video Object Plane\n");
		else if (startcode == 0xB5)
			PRINTF("Visual Object\n");
		else if (startcode == 0xB6)
			//PRINTF("Video Object Plane\n");
			return 0;
		else if (startcode <= 0x1F)
			PRINTF("Video Object\n");
		else if (startcode >= 0x20 && startcode <= 0x2F)
			//PRINTF("Visual Object Layer %02x\n", startcode);
			break;
		else {
			PRINTF("start = %02x\n", buf[3]);
			continue;
		}
	}
	if (len <= 4)
		ERR_OUT("len = %d\n", len);
	if ((buf[3] & 0xf0) != 0x20)
		ERR_OUT("start = %x\n", (buf[3] & 0xf0));

	bits.buf = buf + 4;
	bits.len = len - 4;
	bits.idx = 0;
	bits.bit = 0;

	if (ts_u_v(&bits, 1))
		ERR_OUT("random_accessible_vol\n");
	if (ts_u_v(&bits, 8))
		ERR_OUT("video_object_type_indication\n");
	if (ts_u_v(&bits, 1))
		ERR_OUT("is_object_layer_identifier\n");
	if (bits.ue) {
		if (ts_u_v(&bits, 4))
			ERR_OUT("video_object_layer_verid\n");
		if (ts_u_v(&bits, 3))
			ERR_OUT("video_object_layer_priority\n");
	}
	if (ts_u_v(&bits, 4))
		ERR_OUT("aspect_ratio_info\n");
	if (bits.ue == 15) {
		if (ts_u_v(&bits, 8))
			ERR_OUT("par_width\n");
		if (ts_u_v(&bits, 8))
			ERR_OUT("par_height\n");
	}
	if (ts_u_v(&bits, 1))
		ERR_OUT("vol_control_parameters\n");
	if (bits.ue) {
		if (ts_u_v(&bits, 2))
			ERR_OUT("chroma_format\n");
		if (ts_u_v(&bits, 1))
			ERR_OUT("low_delay\n");
		if (ts_u_v(&bits, 1))
			ERR_OUT("vbv_parameters\n");
		if (bits.ue) {
			if (ts_u_v(&bits, 15))
				ERR_OUT("first_half_bit_rate\n");
			if (ts_u_v(&bits, 1))
				ERR_OUT("marker_bit\n");
			if (ts_u_v(&bits, 15))
				ERR_OUT("latter_half_bit_rate\n");
			if (ts_u_v(&bits, 1))
				ERR_OUT("marker_bit\n");
			if (ts_u_v(&bits, 15))
				ERR_OUT("first_half_vbv_buffer_size\n");
			if (ts_u_v(&bits, 1))
				ERR_OUT("marker_bit\n");
			if (ts_u_v(&bits, 3))
				ERR_OUT("latter_half_vbv_buffer_size\n");
			if (ts_u_v(&bits, 11))
				ERR_OUT("first_half_vbv_occupancy\n");
			if (ts_u_v(&bits, 1))
				ERR_OUT("marker_bit\n");
			if (ts_u_v(&bits, 15))
				ERR_OUT("chroma_format\n");
			if (ts_u_v(&bits, 1))
				ERR_OUT("marker_bit\n");
		}
	}
	if (ts_u_v(&bits, 2))
		ERR_OUT("video_object_layer_shape\n");
	if (bits.ue != 0)
		ERR_OUT("video_object_layer_shape = %d\n", bits.ue);

	if (ts_u_v(&bits, 1))
		ERR_OUT("marker_bit\n");
	if (ts_u_v(&bits, 16))
		ERR_OUT("vop_time_increment_resolution\n");
	vop_time_increment_resolution = bits.ue;
	if (ts_u_v(&bits, 1))
		ERR_OUT("marker_bit\n");
	if (ts_u_v(&bits, 1))
		ERR_OUT("fixed_vop_rate\n");

	if (bits.ue) {
		int n = _log2(vop_time_increment_resolution);
		if (n == 0)
			n = 1;
		if (ts_u_v(&bits, n))
			ERR_OUT("fixed_vop_time_increment\n");
	}
	if (ts_u_v(&bits, 1))
		ERR_OUT("marker_bit\n");
	if (ts_u_v(&bits, 13))
		ERR_OUT("video_object_layer_width \n");
	width = bits.ue;
	PRINTF("video_object_layer_width = %d\n", width);
	if (ts_u_v(&bits, 1))
		ERR_OUT("marker_bit\n");
	if (ts_u_v(&bits, 13))
		ERR_OUT("video_object_layer_width \n");
	height = bits.ue;
	PRINTF("video_object_layer_height = %d\n", height);

	return width;
Err:
	return -1;
}

/*
	·ÖÎöTSÁ÷»­Ãæ³ß´ç
	-1£º´íÎó
	 0£º Î´·ÖÎö³ö
	 1£º·ÖÎö³ö³ß´ç
 */
int ts_size_parse(char *buf, int len, uint32_t video_pid, uint32_t video_type, int* pwidth, int* pheight)
{
	int ret = 0;
	int l, hl, off;
	unsigned char *p;
	unsigned int pid;

	if (video_pid == 0)
		ERR_OUT("parse not init\n");

	if (len % 188)
		ERR_OUT("len = %d\n", len);

	for (off = 0; off < len; off += 188, buf += 188) {
		p = (unsigned char *)buf;
		if (p[0] != 0x47)
			continue;

		pid =(((unsigned int)p[1] & 0x1f) << 8) + p[2];
		if (pid != video_pid)
			continue;

		if (p[3] & 0x20)//adaptation field
			l = 5 + p[4];
		else
			l = 4;

		if (p[1] & 0x40) {//start indicator
			p += l;

			if (p[0] != 0 || p[1] != 0 || p[2] != 1)
				ERR_OUT("prefix\n");

			if (p[3] < 0xC0 || p[3] > 0xEF)
				ERR_OUT("stream id\n");

			if (188 - l < 9 || (p[6] & 0xC0) != 0x80)
				ERR_OUT("PES header large? l = %d\n", l);

			hl = p[8] + 9;
			l += hl;
			if (188 < l)
				ERR_OUT("PES header too large? l = %d\n", l);

			switch(video_type) {
			case ISO_IEC_H264:
				ret = parse_h264((unsigned char*)(buf + l), 188 - l, pwidth, pheight);
				break;
			case ISO_IEC_MPEG4_VIDEO:
				ret = parse_mpeg4((unsigned char*)(buf + l), 188 - l, pwidth, pheight);
				break;
			case ISO_IEC_13818_2_VIDEO:
				ret = parse_mpeg2((unsigned char*)(buf + l), 188 - l, pwidth, pheight);
				break;
			default:
				ERR_OUT("unsupport video_type = %d\n", video_type);
			}
			//PRINTF("video_type = %d, ret = %d\n", video_type, ret);
			if (ret)
				return ret;
		}
	}

	return 0;
Err:
	return -1;
}
#endif

