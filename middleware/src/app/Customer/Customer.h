#ifndef _Customer_H_
#define _Customer_H_

#ifdef __cplusplus
extern "C" {
#endif

int DeviceConfig();
int ResourceJsRegist();
int GraphicsConfig();
int LogoShow(int pShow, unsigned char *pImage, int pImageLen);

#ifdef __cplusplus
}
#endif

#endif // _Customer_H_
