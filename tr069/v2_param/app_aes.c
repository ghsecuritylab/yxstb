
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "openssl/aes.h"

#include "app_aes.h"

static unsigned char LOCALKEY[]=  "Hybroad Vision..";

static int aes(const char * data, int length, int encrypt, char * out)
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

    strncpy((char*)key, (char*)LOCALKEY, key_len);
    key[key_len] = '\0';

    //printf("key = %s\n", key);
	if (!strlen((char*)key))
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

int Data2Hex(const void * data, int length, char * out)
{
    int i;
    unsigned char c;
    const unsigned char *p = data;

    for(i = 0; i < length; i++) {
        c = p[i];
        sprintf(&out[i * 2], "%02x", c & 0xff);
    }
    out[length * 2] = '\0';
    return length * 2 ;
}

inline static int char2int(int c)
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

int Hex2Data(const char *hex, void *data)
{
    int len = strlen(hex);
    int i;
    int d;
    int c;

    if(len % 2 != 0) {
        return -1;
    }

    for(i = 0; i < len / 2; i++) {
        c = hex[i * 2];
        d = char2int(c) * 16;
        c = hex[i * 2 + 1];
        d += char2int(c);
        *((char *)data + i) = (char)(d & 0xff);
    }
    return i;
}

int app_aes_encrypt(const char * input, char * output)
{
    if(input == NULL || output == NULL)
        return -1;
#if !defined(ANDROID)
    char *temp = malloc((strlen(input) / 16 + 1) * 32 + 1);
    int ret = aes(input, strlen(input), 1, temp);

    if(ret < 0) {
        printf("error!ret = %d\n", ret);
        free(temp);
        return -1;
    }
    Data2Hex(temp, ret, output);
    free(temp);
#else
    strcpy(output, input);
#endif
    return 0;
}

int app_aes_decrypt(const char *input, char *output)
{
#if !defined(ANDROID)
    if(input == NULL || output == NULL)
        return -1;

    if(strlen(input) < 32 || strlen(input) % 32 != 0) {
        strcpy(output, input);
        return 0;
    }

    char *temp = malloc(strlen(input) * 2);
    int ret = Hex2Data(input, temp);
    ret = aes(temp, ret, 0, output);

    if (ret >= 0)
        output[ret] = '\0';

    free(temp);

    return ret;
#else
    strcpy(output, input);
    return strlen(output) + 1;
#endif
}
