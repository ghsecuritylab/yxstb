#include <string.h>
#include "sha.h"
#include "aes.h"
#include "yx_crypto.h"
#include "tr069_api.h"
#include "aesCrypto.h"
#include "nm_dbg.h"
#include "tr069_interface.h"
void generateAESkey(unsigned char *key)
{
    unsigned char temp[33] ={'\0'};
    //char mac[13] ={'\0'};
    char serialOrMac[33] = {'\0'};
    char initvector[9] ="99991231";
    char tr069_passwd[128] = {'\0'} ;
    char tRandom[128] = {'\0'} ;
    int i;
    SHA256_CTX TMS_keys;
    SHA256_Init(&TMS_keys);

    tr069_port_getValue("Device.DeviceInfo.SerialNumber", serialOrMac, 32);
    tr069_api_getValue("Opaque", tRandom, 128);
    getAcsPassword(tr069_passwd);
	//nm_msg_level(LOG_DEBUG, "serialOrMac = %s:Password = %s:Opaque = %s\n",serialOrMac, tr069_passwd,tRandom);

    if((strlen(serialOrMac) == 0) ||(strlen(tr069_passwd) == 0 )||strlen(tRandom) == 0){
        nm_msg("uckey_para is error,please checking\n");
        return 1;
    }else{
        SHA256_Update(&TMS_keys,serialOrMac,strlen(serialOrMac));
        SHA256_Update(&TMS_keys,initvector,strlen(initvector));
        SHA256_Update(&TMS_keys,tr069_passwd,strlen(tr069_passwd));
        SHA256_Update(&TMS_keys, tRandom , strlen(tRandom));

        SHA256_Final(temp, &TMS_keys);
  
        strncpy(key, temp, 32);
        key[32] = '\0';
        /* nm_msg_level(LOG_DEBUG, "key[%s], tmp[%s]", key, temp); */
        return ;
    }
}

int aes_tms(const char * data, int length, int encrypt, char * out)
{/*{{{*/
	AES_KEY aes[1];
	int             KeyBits = 128;		// Should be one of 128,192,256
	unsigned char   ucBuf1[256];
	unsigned char   temp[256] ={'\0'};
	unsigned char   key[256] = {'\0'};
	int             len;
	int             i;

	if(data == NULL || out == NULL || length < 0)
		return -1;
	//key = AESKEY;
	generateAESkey(key);
	if (!strlen(key))
		nm_msg("aes_key error ! !!!!");
	len = length;
	if( encrypt )
	{
		for(i=0; i < len / 16; i++)
		{
			memcpy(temp, data + i * 16, 16);
			AES_set_encrypt_key(key, KeyBits, aes);
			AES_encrypt(temp, ucBuf1, aes);
			memcpy(out + i * 16, ucBuf1, 16);
		}

		if(i * 16 < len)
		{
			int     d = len - i * 16;
			memset(temp, (16 - d), 16);
			memcpy(temp, data + i * 16, d);
			AES_set_encrypt_key(key, KeyBits, aes);
			AES_encrypt(temp, ucBuf1, aes);
			memcpy(out + i * 16, ucBuf1, 16);
		} else {
			memset(temp, 16, 16);
			AES_set_encrypt_key(key, KeyBits, aes);
			AES_encrypt(temp, ucBuf1, aes);
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
			AES_set_decrypt_key(key, KeyBits, aes);
			AES_decrypt(temp, ucBuf1, aes);
			memcpy(result + i * 16, ucBuf1, 16);
		}
		//int     l = i * 16 - result[i * 16 - 1];
		int     l = i * 16 - (result[i * 16 - 1] & 0xff);
		if ( l < 0 ) {
			memcpy( out , data , length );
			free(result);
			return length;
		} else {
			memcpy(out, result, l);
			free(result);
			return l;
		}
        }
        return 0;
}/*}}}*/

int decryptACSCiphertext(const char * input, char * output)
{
    int ret = 0;

    if(input == NULL || output == NULL)
        return -1;
    if((strlen(input) < 16) || ((strlen(input) % 4) != 0) || (strncasecmp(input, "ftp://", 6) == 0)) {
        strcpy(output, input);
        return 0;
    }
    char *temp = malloc(strlen(input) * 2);

    ret =  yx_base64_decode(input, strlen(input), temp);
    if(ret <= 0) {
        nm_msg("the buffer is not a base64encoder string\n");
        free(temp);
        return -1;
    }
    ret = aes_tms(temp, ret, 0, output);
    output[ret] = '\0';
    //nm_msg_level(LOG_DEBUG, "decrypt input = %s\n decrypt temp =%s\n decrypt output =%s\n", input, temp, output);
    free(temp);

    return 0;

}
int encryptACSCiphertext(const char * input, char * output)
{
    if(input == NULL || output == NULL)
        return -1;

    char *  temp = malloc((strlen(input) / 16 + 1) * 32 + 4);
    memset(temp, 0, ((strlen(input) / 16 + 1) * 32 + 4));
    int     ret = aes_tms(input, strlen(input), 1, temp);
    ret =  yx_base64_encode(temp, strlen(temp), output);
    if(ret <= 0) {
        free(temp);
        return -1;
    }
    //nm_msg_level(LOG_DEBUG, "encrypt input = %s\n encrypt temp =%s\n encrypt output =%s\n", input, temp, output);
    free(temp);

    return 0;
}

