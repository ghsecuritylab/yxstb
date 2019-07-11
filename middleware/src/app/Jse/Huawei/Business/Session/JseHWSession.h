#ifndef JseHWSession_h
#define JseHWSession_h

#ifdef __cplusplus

int JseHWSessionInit();

#endif //__cplusplus

#ifdef __cplusplus
extern "C" {
#endif

int jseAuthFailCountGet();
void jseAuthFailInfoGet(char(*buf)[256], int size);
void jseAuthFailInfoSet(const char *url, int err_no);

#ifdef __cplusplus
}
#endif


#endif // JseHWSession_h

