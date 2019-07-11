#ifndef cryptoFunc_h
#define cryptoFunc_h

#ifdef __cplusplus
extern "C" {
#endif

int pkcs5Padding(const char *plainData, int plainLen, char *paddedData, int paddedLen, int blockSize);
int pkcs5PaddingRemove(char *plainData, int plainLen);

int aesEcbEncrypt(char *plain, int plainLen, char *key, char *encrypted, int encryptedLen);
int aesEcbDecrypt(char *encrypted, int encryptedLen, char *key, char *plain, int plainLen);

int aesCbcEncrypt(char *plain, int plainLen, char *key, char *vector, char *encrypted, int * encryptedLen);
int aesCbcDecrypt(char *encrypted, int encryptedLen, char *key, char *vector, char *plain, int *plainLen);

int md5Encypt(char **array, int num, char *enc, int len, int flag);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //cryptoFunc_h
