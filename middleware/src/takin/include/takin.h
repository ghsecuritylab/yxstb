#ifndef _TAKIN_H_
#define _TAKIN_H_

#ifdef __cplusplus
extern "C"{
#endif

int BrowserStatisticHTTPFailNumbersGet();
void BrowserStatisticHTTPFailInfoGet(char(*)[256], int);
int BrowserStatisticHTTPReqNumbersGet();

#ifdef __cplusplus
}
#endif
#endif
