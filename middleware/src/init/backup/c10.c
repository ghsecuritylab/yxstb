
#include "mid_stream.h"
#include "preconfig.h"

void app_stream_init_private_standard(void)
{
    /*	HUAWEI_C10��ֻ�Խ����˾ֵ㣬�������²���
    	mid_stream_standard(RTSP_STANDARD_CTC_SHANGHAI);
    	����Ҫ��Ϊ���ԣ��������²���
    	mid_stream_standard(RTSP_STANDARD_HUAWEI);
    	��mid_stream_standard(RTSP_STANDARD_CTC_GUANGDONG);
    	������user-agent��clock/nptʱ��
    	��������30�룬��������
    	mid_stream_heartbit_period(30);
    */
    mid_stream_heartbit_period(30);
    mid_stream_standard(RTSP_STANDARD_CTC_GUANGDONG);
    mid_stream_set_size(4600 * 1316, 8192 * 1316);

#if _HW_BASE_VER_ >= 57
    mid_stream_nat(1);
#endif
#if _HW_BASE_VER_ >= 58
    mid_stream_rrs_timeout(2000);
#endif

    return ;
}

#ifdef HWVERSION
char * HwSoftwareVersion(int type)
{
    if (type)
        return HWVERSION;
    else
        return "IPTV STB "HWVERSION;
}
#endif

