
#ifndef YX_CRYPTO_H
#define YX_CRYPTO_H

#ifdef  __cplusplus
extern "C" {
#endif

#ifndef ERROR
#define ERROR printf
#endif

#ifndef PRINTF
//#define PRINTF printf
#define PRINTF(x...)
#endif


int yx_aes_ecb_encrypt(char *plain_txt, int plain_txt_len, char *key, char *encrypted_txt, int * encrypted_txt_len);
int yx_aes_ecb_decrypt(char *encrypted_txt, int encrypted_txt_len, char *key, char *plain_txt, int *plain_txt_len);
int yx_aes_cbc_encrypt(char *plain_txt, int plain_txt_len, char *key, char *vector, char *encrypted_txt, int * encrypted_txt_len);
int yx_aes_cbc_decrypt(char *encrypted_txt, int encrypted_txt_len, char *key, char *vector, char *plain_txt, int *plain_txt_len);
int yx_base64_encode(char *in, int inLen, char *out);
int yx_base64_decode(char *in, int inLen, char *out);
int hex_get_from_char(unsigned char *in, int in_len, unsigned char *hex_out_buf, int hex_out_buf_len);
int char_get_from_hex(unsigned char *hex, int hex_len, unsigned char *out_buf, int out_buf_len);
int aes(const char * data, int length, int encrypt, char * out);



/**********************************************************************************
*  source:用来MD5加密的数据
*  huaweipriv:是否需要传入华为私有的数据 “99991231”
*  flag: 1表示24位，2表示32位
*  updateflag :1:表示变大写，0表示不变化
*  MD5加密
************************************************************************************/

void yx_md5_get_key1(char *md5key, char *array, unsigned char *huaweipriv, int flag, int updateflag);

/**********************************************************************************
*    deskey: 24位的3DES的key
*
*
*
*  设置3DES的key1,key2,key3
************************************************************************************/

void yx_set_3des_key(char *deskey);

/**********************************************************************************
*  in: 加密后的数据 必须是8的倍数
*  out:解密后的数据，程序中默认加上结束符
*  inlen:加密数据的长度（strlen）
*   *outlen:储存传出的out的长度
*   返回值： -1失败。
*   注意：函数本身不判断out区是否太小，导致越界的问题
************************************************************************************/
int yx_3des_decrypt(char *in, char *out, int inlen, int *outlen);
/**********************************************************************************
*  in: 原始未加密数据
*  out:加密后的数据，原始长度，未做任何处理
*  inlen:原始未加密数据的长度（strlen）
*   *outlen:储存传出的out的长度
*   返回值： -1失败。
*   注意：函数本身不判断out区是否太小，导致越界的问题
************************************************************************************/
int yx_3des_encrypt(char *in, char *out, int inlen, int *outlen);

#ifdef  __cplusplus
}
#endif

#endif /* !HEADER_CRYPTO_H */
