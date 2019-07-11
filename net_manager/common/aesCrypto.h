#ifndef _aesCrypto_h_
#define _aesCrypto_h_
#ifdef __cplusplus
extern "C" {
#endif
void  generateAESkey(unsigned char *key);
int decryptACSCiphertext(const char * input, char * output);
int encryptACSCiphertext(const char * input, char * output);
#ifdef __cplusplus
}
#endif
#endif
