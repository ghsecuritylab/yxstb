#ifndef __APP_PRESSURE_TEST_H__
#define __APP_PRESSURE_TEST_H__

int app_pressureTest_get_flag(void);
int app_pressureTest_get_keynum(void);


void app_pressureTest_timer(void);
void app_pressureTest_clear(void);
void app_pressureTest_set_flag(int flag);
void app_pressureTest_set_para( int interval, int keynum, char *key_str);
void app_pressureTest_deal_key(char *cmd_str);


#endif

