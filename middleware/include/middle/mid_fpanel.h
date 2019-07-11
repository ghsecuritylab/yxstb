#ifndef __MID_FPANEL_H__
#define __MID_FPANEL_H__

#ifdef __cplusplus
extern "C" {
#endif

#define PPPOE_EXIT_TIMEOUT 2000


int mid_fpanel_powerled(int pColor);
int mid_fpanel_netled(int pOnOff);

void mid_fpanel_reboot(void);
// �����, ���ô˽ӿں�,ϵͳ����ػ�״̬. �����Դȫ���ر�. added by teddy. 2011-2-12 14:21:10
void mid_fpanel_poweroffdeep( );
unsigned int mid_real_standbyFlag_get(void);

void mid_fpanel_standby_set(int flag);
int mid_fpanel_standby_get(void);

void mid_fpanel_showtext(char *string, int millisecond);
void mid_fpanel_hide(void);

#ifdef __cplusplus
}
#endif

#endif /* __MID_FPANEL_H__ */

