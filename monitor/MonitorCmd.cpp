#include <openssl/md5.h>
#include <openssl/aes.h>

#include "MonitorConfig.h"
#include "MonitorCmd.h"
#include "pathConfig.h"
#include "MonitorTimer.h"
#include "ParseCmd.h"

#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#define IND_MALLOC(size)			malloc(size)
#define IND_DALLOC(size)			malloc(size)
#define IND_CALLOC(size, n)			calloc(size, n)
#define IND_STRDUP(ptr)				strdup(ptr)
#define IND_REALLOC(ptr, size)		realloc(ptr, size)
#define IND_FREE(ptr)				free(ptr)
#define IND_MEMCPY(dest,src, n)		memcpy(dest,src, n)
#define IND_MEMSET(s, ch, n)		memset(s, ch, n)
#define IND_STRCPY(dest, src)		strcpy(dest, src)
#define IND_STRNCPY(dest, src, n)	strncpy(dest, src, n)


extern "C"
{
void sendMessageToEPGBrowser(int what, int arg1, int arg2, unsigned int pDelayMillis);
void yx_md5_get_key_monitor(char *md5key, char *array, unsigned char *huaweipriv, int flag, int updateflag);
int NativeHandlerGetState();
static int app_aes_decrypt_monitor(const char * input, char * output);

}
void monitorTimerDelayConnect(int param);


extern int            already_connected; //stb_monitor is connected or not
extern unsigned char  g_monitor_aes_keys[17];
extern int            initialize_state;
MonitorCmd*           MonitorCmd::m_monitorCmd;
static unsigned char  LOCALKEY[] = "Hybroad Vision..";
static unsigned char  g_local_aes_keys[USER_LEN];
extern int            g_connectTimes;
extern int            MonitorConnectFlag;

MonitorCmd::MonitorCmd()
{
	InitCmdMap();


}

MonitorCmd::~MonitorCmd()
{
	//
	std::map<std::string, ParseCmd*>::iterator iter;
	for (iter = m_cmdMap.begin(); iter != m_cmdMap.end(); iter++)
	{
		delete iter->second;
	}

	delete m_monitorCmd;
	m_monitorCmd = NULL;
}


MonitorCmd* MonitorCmd::GetInstance()
{
	if (m_monitorCmd != NULL)
		return m_monitorCmd;

	m_monitorCmd = new MonitorCmd();
	return m_monitorCmd;
}


int MonitorCmd::InitCmdMap()
{
	m_cmdMap["read"] 		= new ParseRead();
	m_cmdMap["write"] 		= new ParseWrite();
	m_cmdMap["ioctl"] 		= new ParseIoctl();
	m_cmdMap["inform"] 		= new ParseInform();
	m_cmdMap["connect"] 	= new ParseConnect();
	m_cmdMap["initialize"] 	= new ParseInitialize();

   return 0;
}


/*
第一层命令解析
*/
int MonitorCmd::CmdSort(const moni_buf_t pBuf)
{
    int i 	= 0;
	int ret = 0;
	int msglen 	= 0;
    int funclen = 0;
    char func[64] = {0};
    char *pFunc = NULL;
	char *pRest = NULL;
    moni_buf send_buf;

    memset(&send_buf, 0, sizeof(send_buf));

    printf("Command(%s)\n", pBuf->buf);

    if((pBuf == NULL) || !strlen(pBuf->buf)) {
        printf("The monitor recv cm is NULL\n");
        return -1;
    }

	pFunc = pBuf->buf + IDENTIFY_CODE_LEN + SESSIONID_LEN;
    pRest = strchr(pFunc, '^');

    if(pRest == NULL) {
        printf("The monitor recv cm is not ^\n");
        return -1;
    }

    funclen = strlen(pFunc) - strlen(pRest);
    if(funclen > 63) {
        funclen = 63;
    }

    strncpy(func, pFunc, funclen);

    //默认不连接工具
    if (MonitorConnectFlag == 0) {
        printf("MonitorConnectFlag = %d can not connect!\n", MonitorConnectFlag);
        return 0;
    }

    if (g_connectTimes <= 0 && m_cmdMap["initialize"] != NULL) {
        ((ParseInitialize *)m_cmdMap["initialize"])->sendLogErrorMsg(&send_buf);
        return -1;
    }

    if(MonitorInitializeStateGet() == 0) {
        if(0 != MonitorIdentifyCheck(pBuf->buf)) {
            printf("The monitor recv cm is wrong, the md5 check is wrong\n");
            sprintf(pBuf->buf + 8, "%d%s", MSG_CONNECT_CHECK_ERROR, func);
            pBuf->len = 8 + 3 + funclen;
            MonitorManager::GetInstance()->monitor_cmd_response(pBuf);

            //add
            if (g_connectTimes > 0)
                g_connectTimes--;
            if (g_connectTimes == 0)
                monitorTimerCreate(180, 1, monitorTimerDelayConnect, NULL);
            //end
            return 0;
        } 
        g_connectTimes = 5;
    }

    //xuke if(1 != already_connected) {
    //xuke    already_connected = 1;
    //xuke    sendMessageToEPGBrowser(MessageType_Prompt, 0, 1, 0); // LinkDisplay
    //xuke}

    msglen += IDENTIFY_CODE_LEN;
    msglen += SESSIONID_LEN;

	//
	std::string cmd_temp(func);

	if (m_cmdMap.find(cmd_temp) == m_cmdMap.end()){
        printf("No comand :%s, or havn't implement!!!\n", func);
        ret = MSG_COMMAND_UNDEFINE_ERROR;
	} else {
		//解析命令
		ret = (this->m_cmdMap[cmd_temp])->Parse(pBuf->buf+msglen, pBuf);
        if (ret != 2)
		    MonitorManager::GetInstance()->monitor_cmd_response(pBuf);
        return 0;
    }

    sprintf(pBuf->buf, "%d%s", ret, func);
    pBuf->len = 3 + funclen;
    if(pBuf->extend != NULL) {
        sprintf(pBuf->buf + pBuf->len, "^");
        pBuf->len += 1;
    }

    MonitorManager::GetInstance()->monitor_cmd_response(pBuf);
    return 0;
}


int MonitorCmd::MonitorInitializeStateGet()
{
	return initialize_state ;
}

void MonitorCmd::MonitorInitializeStateSet(int state)
{
    if(MonitorInitializeStateGet() != state)
        initialize_state = state ;
}




/***********************************************************
 * Check client message Identify code validity
 *   rightful return 0
 * 客户端报文格式
 *      TCP head + Identify code + sessionID + Function call + "^" + Call value + "^" + Parameter + "^" + data + "\0"
 *      说明：每个字段中，不包括+、引号和空格， ^加在前一个字段后面。Data后面跟着的”\0”为结束符，注意”\0”是整个数据的结束符，
 *      当data过长，一个包里无法装载时，data会被分片，”\0”只跟在最后一个分片的data后面。报文封装字段以此为规则拼装，由协议栈打包发送。
 * Identify code生成规则：
 *      Identify code=md5(sessionID+key)取前8字节（byte），值为大写。其中key=huawei。
 *      注意，md5对像不包括“+”号；
 *  初始化连接时的sessionID根据连接的用户名和密码生成，生成规则如下：
 *      1) 当机顶盒软件版本连接需要用户名和密码时， 客户端将用户名和密码拼成字符串，
 *      进行MD5取前16字节(byte)传送，用户名可以为空；当界面上输入空格时，认为是没有内容。
 *      2) 当机顶盒软件版本连接不需要用户名和密码时，则使用，则使用128位的0。在机顶盒上也是按此规则生成。
 *  机顶盒会做sessionID进行校验，如果验证不通过，会有错误返回。
***********************************************************/
int MonitorCmd::MonitorIdentifyCheck(char *pIdentify)
{
    char out[IDENTIFY_CODE_LEN + 1] = {0};
    char tSessionId[SESSIONID_LEN +6 + 1] = {0};
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
        yx_md5_get_key_monitor(tSessionId, in, NULL, 4, 0); // 0 卤铆脢戮虏禄麓贸脨麓, 1 卤铆脢戮麓贸脨麓
    }
    printf("SessionId is %s\n", tSessionId);

    strcat(tSessionId, "huawei");
    yx_md5_get_key_monitor(out, tSessionId, NULL, 3, 1);

    printf("%s\n", out);
    return strncmp(out, pIdentify, IDENTIFY_CODE_LEN);

}

void monitorTimerDelayConnect(int param)
{
    g_connectTimes = 5;
}


//#ifdef ANDROID
void _UseDefaultMonitorInfo(char * user, int userlen, char * pswd, int pswdlen)
{
    if (user && userlen > 0)
        snprintf(user, userlen, "huawei");
    if (pswd && pswdlen > 0)
        snprintf(pswd, pswdlen, ".287aW");
}


#ifdef ANDROID
void getMonitorInfo(char * user, int userlen, char * pswd, int pswdlen)
{
    FILE*   fp = fopen(CONFIG_FILE_DIR"/monitorpasswd", "rb");
    if (!fp) {
        _UseDefaultMonitorInfo(user, userlen, pswd, pswdlen);
        return;
    }
    int     len;
    fseek(fp, 0, SEEK_END);
    len = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    int     total;
    char*   buffer = (char*)IND_MALLOC(len + 1);
    IND_MEMSET(buffer, 0, len + 1);
    total = 0;
    while (total < len) {
        int bytes_read = fread(buffer + total, 1, len - total, fp);
        if (bytes_read <= 0) {
            fclose(fp);
            IND_FREE(buffer);
            _UseDefaultMonitorInfo(user, userlen, pswd, pswdlen);
            return;
        }
        total += bytes_read;
    }
    fclose(fp);

    char* p = strchr(buffer, ':');
    if (!p) {
        _UseDefaultMonitorInfo(user, userlen, pswd, pswdlen);
        IND_FREE(buffer);
        return;
    }
    *p = '\0';
    p++;
    if (pswd && pswdlen > 0) {
        char * buf = (char*)IND_MALLOC(strlen(p));
        IND_MEMSET(buf, 0, strlen(p));
        app_aes_decrypt_monitor(p, buf);
        snprintf(pswd, pswdlen, "%s", buf);
        IND_FREE(buf);
    }
    if (user && userlen > 0) {
        snprintf(user, userlen, "%s", buffer);
    }
    IND_FREE(buffer);
    return;
}
#endif

int char2int(int c)
{
        if(c >= '0' && c <= '9') {
                c = c - '0';
        } else if(c >= 'a' && c <= 'z') {
                c = c - 'a' + 10;
        } else if(c >= 'A' && c <= 'Z') {
                c = c - 'A' + 10;
        }
        return c;
}


int Hex2Data(const char * hex, void * data)
{
        int             len = strlen(hex);
        int             i;
        int             d;
        int             c;

        if(len % 2 != 0)
        {
                return -1;
        }

        for(i=0; i<len/2; i++)
        {
                c = hex[i * 2];
                d = char2int(c) * 16;
                c = hex[i * 2 + 1];
                d += char2int(c);
                *((char *)data + i) = (char)(d & 0xff);
        }
        return i;
}

int aes(const char * data, int length, int encrypt, char * out);

static int app_aes_decrypt_monitor(const char * input, char * output)
{

	if(input == NULL || output == NULL)
		return -1;

	if(strlen(input) < 32 || strlen(input) % 32 != 0) {
		IND_STRCPY(output, input);
		return 0;
	}
	char *temp = (char *)IND_MALLOC(strlen(input) * 2);
	int ret = Hex2Data(input, temp);
	ret = aes(temp, ret, 0, output);
	if (ret >=0 )
		output[ret] = '\0';
	IND_FREE(temp);
	return ret;

}

//static char eds3key[24] = {0};
/**********************************************************************************
*  source:用来MD5加密的数据
*  huaweipriv:是否需要传入华为私有的数据 “99991231”
*  flag: 1表示24位，2表示32位
*  updateflag :1:表示变大写，0表示不变化
*  MD5加密
************************************************************************************/
void yx_md5_get_key_monitor(char *md5key, char *array, unsigned char *huaweipriv, int flag, int updateflag)
{
    unsigned char digest[16] = {0};
    char buf[37] = {0};
    int i = 0, len = 0;
    int lenflag = 0;
    MD5_CTX ctx;

    if(flag == 1)
        lenflag = 12;
    else if(flag == 2)
        lenflag = 16;
    else if(flag == 3)
        lenflag = 4;
    else if(flag == 4)
        lenflag = 8;

    MD5_Init(&ctx);

    MD5_Update(&ctx, (const void *)array, strlen((char *)array));
    if(huaweipriv != NULL) //华为私有
        MD5_Update(&ctx,	(const void *)huaweipriv, strlen((char *)huaweipriv));

    MD5_Final(digest, &ctx);

    for(i = 0; i < lenflag; i ++) {
        len += sprintf(buf + len, "%02x", digest[i]);
    }
    printf("Create md5 len(%d) data(%s)\n", lenflag, buf);
    if(updateflag == 1) {
        for(i = 0; i < lenflag * 2; i++)
            buf [i] = toupper(buf[i]);
    }
    memcpy(md5key, buf, lenflag * 2);
    printf("return md5key(%s)\n", md5key);
    return;
}
//#else

//#endif//ANDROID


int app_local_aes_keys_set(unsigned char *key)
{
	int len =sizeof(LOCALKEY)/sizeof(LOCALKEY[0]);
	//Comparing an array to null is not useful.
	if(/*NULL == LOCALKEY||*/len >32)
	{
		printf ("The PlatformCode is NULL!\n");
		return 1;
	}
	if(strncmp((const char *)g_local_aes_keys, (const char *)LOCALKEY, len))
		strncpy((char *)g_local_aes_keys, (const char *)LOCALKEY, len);
	g_local_aes_keys[len] = '\0';
	strcpy((char *)key,(const char *)g_local_aes_keys);
	return 0;
}


int aes(const char * data, int length, int encrypt, char * out)
{/*{{{*/
	AES_KEY tAes[1];
	int             KeyBits = 128;		// Should be one of 128、192、256中的一个
	unsigned char   ucBuf1[64];
	unsigned char   temp[64] ={'\0'};
	unsigned char   key[256] = {'\0'};
	int             len;
	int             i;
    int key_len =sizeof(LOCALKEY)/sizeof(LOCALKEY[0]);
#if 0
        assert(data != NULL);
        assert(out != NULL);
        assert(length >= 0);
#endif
	if(data == NULL || out == NULL || length < 0)
		return -1;

//	app_local_aes_keys_set(key);

    strncpy((char *)key, (const char *)LOCALKEY, key_len);
    key[key_len] = '\0';

    printf("key = %s\n", key);
	if (!strlen((char *)key))
		printf("aes_key error ! !!!!");

	len = length;
	if(encrypt)
	{
		for(i=0; i < len / 16; i++)
		{
			memcpy(temp, data + i * 16, 16);
			AES_set_encrypt_key(key, KeyBits, tAes);
			AES_encrypt(temp, ucBuf1, tAes);
			memcpy(out + i * 16, ucBuf1, 16);
		}

		if(i * 16 < len)
		{
			int     d = len - i * 16;
			memset(temp, (16 - d), 16);
			memcpy(temp, data + i * 16, d);
			AES_set_encrypt_key(key, KeyBits, tAes);
			AES_encrypt(temp, ucBuf1, tAes);
			memcpy(out + i * 16, ucBuf1, 16);
		} else {
			memset(temp, 16, 16);
			AES_set_encrypt_key(key, KeyBits, tAes);
			AES_encrypt(temp, ucBuf1, tAes);
			memcpy(out + i * 16, ucBuf1, 16);
		}
		return (i * 16 + 16);
        } else {
               // assert(len % 16 == 0);
		if(len % 16 != 0)
			return -1;

		char *  result = (char *)malloc(len  + 1);

		for(i=0; i < len / 16; i++)
		{
			memcpy(temp, data + i * 16, 16);
			AES_set_decrypt_key(key, KeyBits, tAes);
			AES_decrypt(temp, ucBuf1, tAes);
			memcpy(result + i * 16, ucBuf1, 16);
		}
		int     l = i * 16 - (result[i * 16 - 1] & 0xff);
		if ( l < 0 )
			 memcpy( out , data , length );
		else
			memcpy(out, result, l);
		free(result);
		return l;
        }
        return 0;
}/*}}}*/

