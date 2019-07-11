
#include "ParseCmd.h"
#include "MonitorDef.h"
#include "MonitorManager.h"
#include "MonitorConfig.h"

#include <ctype.h>
#include <openssl/aes.h>
#include <openssl/md5.h>
#include <openssl/bn.h>
#include <openssl/rsa.h>
#include "string.h"


extern unsigned char g_monitor_aes_keys[17];
extern int initialize_state;
int    g_connectTimes=5;

extern "C" {
void sendMessageToEPGBrowser(int what, int arg1, int arg2, unsigned int pDelayMillis);
void yx_md5_get_key_monitor(char *md5key, char *array, unsigned char *huaweipriv, int flag, int updateflag);
int NativeHandlerGetState();
}


//ParseInitialize Class
ParseInitialize::ParseInitialize()
{
	m_initMap["connection"] = &ParseInitialize::Connection;
}

ParseInitialize::~ParseInitialize()
{
}

int ParseInitialize::Exec(moni_buf_t pBuf)
{
	int ret = 0;
	int response_ret = 0;
	int key_run_state = 0;
	unsigned char key[KEY_LEN] = {'\0'};
    unsigned char output[OUTPUT_LEN] = {'\0'};
	std::string str_cmd;
	//
	do{
		str_cmd = m_szCmdInfo[1];
		m_iter = m_initMap.find(str_cmd);
		if (m_iter == m_initMap.end()){
			response_ret = MSG_PARAMETER_ERROR;
			key_run_state = 1;
			break;
		}

		if (strlen(m_szCmdInfo[1]) > 63){
			response_ret = MSG_COMMAND_FAIL_ERROR;
			break;
		}
		//
		ret = (this->*m_initMap[str_cmd])(key, output);

		if(ret != 0)
			return ret;

		response_ret = MSG_CONNECT_OK;
	}while(0);


    if(key_run_state == 1) {
        sprintf(pBuf->buf, "%dinitialize^connection", response_ret);
        pBuf->len = 3 + strlen("initialize^connection");
        if(pBuf->extend != NULL) {
            sprintf(pBuf->buf + pBuf->len, "^");
            pBuf->len += 1;
        }
        printf("Command response(%s) run state(%d)\n", pBuf->buf, key_run_state);
    } else {
        memcpy(pBuf->buf, output, OUTPUT_LEN-1);
        sprintf(pBuf->buf + OUTPUT_LEN-1, "%dinitialize^connection", response_ret);
        pBuf->len = OUTPUT_LEN-1 + 3 + strlen("initialize^connection");
        if(pBuf->extend != NULL) {
            sprintf(pBuf->buf + pBuf->len, "^");
            pBuf->len += 1;
        }
    }
    return 0;
}


int ParseInitialize::Connection(unsigned char *key, unsigned char *output)
{
	int ret = 0;

	ret = SetSessionID();
	if(ret != 0)
		return ret;

	MonitorInitializeStateSet(1);
	get_monitor_aes_keys(key, KEY_LEN);

	ret = random_number_encrypt(key, output);

	return ret;
}


void ParseInitialize::sendLogErrorMsg(moni_buf_t pBuf)
{
    unsigned char tmp_key[17] = {'\0'};
    unsigned char tmp_output[129] = {'\0'};

    get_monitor_aes_keys(tmp_key, sizeof(tmp_key));
    random_number_encrypt(tmp_key, tmp_output);

    memcpy(pBuf->buf, tmp_output, 128);
    sprintf(pBuf->buf + 128, "%dinitialize^connection^null", MSG_ACCOUNT_LOCKOUT);
    pBuf->len = 128 + 3 + strlen("initialize^connection^null");
    MonitorManager::GetInstance()->monitor_cmd_response(pBuf);

}

int ParseInitialize::SetSessionID()
{
	int i = 0 ;
    int tRandLen = 0;
    char tRandBuf[17] = {0};

    if(NULL == g_monitor_aes_keys)
        return 0;

    srand((unsigned)time(NULL));
    for(i = 0; i < 8; i++) {
        g_monitor_aes_keys[i] = (unsigned char)(rand() % 255);
        tRandLen += sprintf(tRandBuf + tRandLen, "%02x", g_monitor_aes_keys[i]);
    }

    for(i = 0; i < 8 * 2; i++)
        tRandBuf[i] = toupper(tRandBuf[i]);

    memset(g_monitor_aes_keys, 0, 17);
    strcpy((char *)g_monitor_aes_keys, tRandBuf);
    printf("g_monitor_aes_keys =%s \n", g_monitor_aes_keys);
    return 0;

}


int ParseInitialize::MonitorInitializeStateGet()
{
	return initialize_state ;
}

void ParseInitialize::MonitorInitializeStateSet(int state)
{
    if(MonitorInitializeStateGet() != state)
        initialize_state = state ;
}


int ParseInitialize::MonitorIdentifyCheck(char *pIdentify)
{
    char out[IDENTIFY_CODE_LEN + 1] = {0};
    char tSessionId[SESSIONID_LEN + 1] = {0};
    char in[256]  = {0};
    char user[64] = {0};
    char pass[64] = {0};

    getMonitorInfo(user, sizeof(user), pass, sizeof(pass));

    printf("pIdentify(%s)\n", pIdentify);
    //printf("monitor user(%s)password(%s)\n", user, pass);

    if(strlen(pass) == 0) {
        strncpy(tSessionId, (pIdentify + IDENTIFY_CODE_LEN), 16);
    } else {
        snprintf(in, 256, "%s%s", user, pass);
        yx_md5_get_key_monitor(tSessionId, in, NULL, 4, 0);
    }
    printf("SessionId is %s\n", tSessionId);

    strcat(tSessionId, "huawei");
    yx_md5_get_key_monitor(out, tSessionId, NULL, 3, 1);

    printf("%s\n", out);
    return strncmp(out, pIdentify, IDENTIFY_CODE_LEN);

}


int ParseInitialize::get_monitor_aes_keys(unsigned char *buf, int len)
{
	printf("len = %d \n", len);
    if(NULL == buf || len < 17)
        return 0;
    memcpy(buf, g_monitor_aes_keys, len - 1);
    buf[len] = '\0';
    return 0;
}

int ParseInitialize::random_number_encrypt(unsigned char *key, unsigned char *output)
{
    int length = RSA_rndom_number_encrypt((char *)key, output);
    printf("Length(%d)\n", length);
    if(length <= 0)
        return -1;
    output[length] = '\0';
    return 0;
}

int ParseInitialize::RSA_rndom_number_encrypt(char* input, unsigned char* output)
{
    BIGNUM *bnn = NULL;
	BIGNUM *bne = NULL;
    int flen = 0 ;

    if(input == NULL) {
        printf("input data is error!!\n");
        return -1;
    }

    bnn = BN_new();
    bne = BN_new();
    BN_hex2bn(&bnn, MODULUS);
    BN_set_word(bne, PUBLIC_EXPONENT);
    RSA* encrypt_rsa = RSA_new();
    encrypt_rsa->n = bnn;
    encrypt_rsa->e = bne;
    flen = strlen(input); //RSA_size(encrypt_rsa);

    int encrypt_size = RSA_public_encrypt(flen, (unsigned char*)input, output, encrypt_rsa, RSA_PKCS1_PADDING);
	printf("Encrypt size(%d)\n", encrypt_size);
    RSA_free(encrypt_rsa);
    return encrypt_size;
}
