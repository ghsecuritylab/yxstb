#include "cryptoFunc.h"
#include "openssl/aes.h"
#include "openssl/md5.h"
#include "charConvert.h"

#include <stdio.h>
#include <string.h>

#define MAX_LEN 4096

int pkcs5Padding(const char *plainData, int plainLlen, char *paddedData, int paddedLen, int block_size)
{
    int padLen = plainLlen % block_size;

    if (plainLlen > paddedLen)
        return -1;

    memcpy(paddedData, plainData, plainLlen);
    if (padLen) {
        padLen = block_size - padLen;
        int i;
        for(i = 0; i < padLen; i ++) {
            paddedData[plainLlen + i] = padLen;
        }
    } else {
        padLen = block_size;
        int i;
        for (i = 0; i < padLen; i ++) {
            paddedData[plainLlen + i] = padLen;
        }
    }
    return padLen + plainLlen;
}


int pkcs5PaddingRemove(char *plainData, int plainLlen)
{
    int paddingValue = plainData[plainLlen - 1];

    while (paddingValue > 0) {
        if (plainData[plainLlen - paddingValue] != plainData[plainLlen - 1])
            return -1;
        paddingValue --;
    }
    plainData[plainLlen - plainData[plainLlen - 1]] = '\0';
    return (plainLlen - plainData[plainLlen - 1]);
}

int aesEcbEncrypt(char *plain, int plainLen, char *key, char *encrypted, int encryptedLen)
{
    if (!plain || !key || !encrypted)
    	return -1;

    if (encryptedLen < ((plainLen & 0xfffffff0) + 16))
        return -1;

    int i;
    char input[MAX_LEN] = {0};
    AES_KEY aes_ks;

    encryptedLen = pkcs5Padding(plain, plainLen, input, MAX_LEN, 16);
    AES_set_encrypt_key((unsigned char*)key, 128, &aes_ks);
    for (i = 0; i < (encryptedLen >> 4); i++)
        AES_encrypt((unsigned char*)(input + (i << 4)), (unsigned char*)(encrypted + (i << 4)), &aes_ks);

    return encryptedLen;
}

int aesEcbDecrypt(char *encrypted, int encryptedLen, char *key, char *plain, int plainLen)
{
	if (!plain || !key || !encrypted)
    	return -1;

    if ((encryptedLen & 0x0000000f) || plainLen < (encryptedLen - 16))
               return -1;

    int i;
    AES_KEY aes_ks;

    AES_set_decrypt_key((unsigned char*)key, 128, &aes_ks);
	for (i = 0; i < (encryptedLen >> 4); i++)
        AES_decrypt((unsigned char*)(encrypted + (i << 4)), (unsigned char*)(plain + (i << 4)), &aes_ks);
    plainLen = pkcs5PaddingRemove(plain, encryptedLen);

    return plainLen;
}

int aesCbcEncrypt(char *plain, int plainLen, char *key, char *vector, char *encrypted, int * encryptedLen)
{
	if (!plain || !key || !encrypted || !vector)
    	return -1;
    if (plainLen > MAX_LEN)
        return -1;

    AES_KEY aes_ks;
    int len = 0;
    char input[MAX_LEN + 1] = {0};

    AES_set_encrypt_key((unsigned char*)key, 128, &aes_ks);
    len = pkcs5Padding(plain, plainLen, input, MAX_LEN + 1, 16);
    if (*encryptedLen < (len + 1))
        return -1;

    AES_cbc_encrypt((unsigned char*)input, (unsigned char*)encrypted, len, &aes_ks, (unsigned char*)vector , 1);
    *encryptedLen = len;
    return *encryptedLen;
}

int aesCbcDecrypt(char *encrypted, int encryptedLen, char *key, char *vector, char *plain, int *plainLen)
{
	if (!plain || !key || !encrypted)
    	return -1;

    if (*plainLen < encryptedLen)
        return -1;

    AES_KEY aes_ks;

    AES_set_decrypt_key((unsigned char*)key, 128, &aes_ks);
    AES_cbc_encrypt((unsigned char*)encrypted, (unsigned char*)plain, encryptedLen, &aes_ks, (unsigned char*)vector , 0);
    *plainLen = pkcs5PaddingRemove(plain, encryptedLen);
    return *plainLen;
}

//flag=1 huaweiEncrypt; flag=0;normal md5 encrypt
int md5Encypt(char **array, int num, char *enc, int len, int flag)
{
    if (!array || !enc)
        return -1;
    if (len < 16)
        return -1;

    int i = 0;
    MD5_CTX ctx;

    MD5_Init(&ctx);
    for(i = 0; i < num; i ++)
        MD5_Update(&ctx, (unsigned char *)array[i], strlen(array[i]));
    if (flag)//huawei encrypt
        MD5_Update(&ctx, (unsigned char *)"99991231", 8);
    MD5_Final((unsigned char*)enc, &ctx);
    return 0;
}

