#ifndef Business_h
#define Business_h

#ifdef __cplusplus

class Business {
public:
    Business();
    ~Business();

	int changeUrlFormatFromDomainToIp(char* , int);

    int setEncryToken(const char *);
	char* getEncryToken();

	char* getCTCAuthInfo(char* , int);
	char* getCUAuthInfo(char* , int );

    void setEDSJoinFlag(const int);
    int getEDSJoinFlag(void);
    void setEPGReadyFlag(int);
    int getEPGReadyFlag();

    int setKeyCtrl(int);
    int getKeyCtrl();

    void setServiceMode(char *);
    char* getServiceMode();

private:
    static char s_encryToken[36];
    static int s_joinFlag;
    static char s_serviceMode[10];
    static int s_EPGReadyFlag;

#if defined (HUAWEI_C10)
    static int s_leftrightkeyFlag;
#elif defined (HUAWEI_C20)
    static int s_leftrightkeyFlag;
#else
    should define s_leftrightkeyFlag.
#endif
};

Business &business();

#endif // __cplusplus

#ifdef __cplusplus
extern "C" {
#endif

void BusinessSetEDSJoinFlag(const int);
int BusinessGetKeyCtrl();
int BusinessGetEPGReadyFlag();
char* BusinessGetServiceMode();

#ifdef __cplusplus
}
#endif //__cplusplus

#endif // Business_h

