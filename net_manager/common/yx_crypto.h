
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
*  source:����MD5���ܵ�����
*  huaweipriv:�Ƿ���Ҫ���뻪Ϊ˽�е����� ��99991231��
*  flag: 1��ʾ24λ��2��ʾ32λ
*  updateflag :1:��ʾ���д��0��ʾ���仯
*  MD5����
************************************************************************************/

void yx_md5_get_key1(char *md5key, char *array, unsigned char *huaweipriv, int flag, int updateflag);

/**********************************************************************************
*    deskey: 24λ��3DES��key
*
*
*
*  ����3DES��key1,key2,key3
************************************************************************************/

void yx_set_3des_key(char *deskey);

/**********************************************************************************
*  in: ���ܺ������ ������8�ı���
*  out:���ܺ�����ݣ�������Ĭ�ϼ��Ͻ�����
*  inlen:�������ݵĳ��ȣ�strlen��
*   *outlen:���洫����out�ĳ���
*   ����ֵ�� -1ʧ�ܡ�
*   ע�⣺���������ж�out���Ƿ�̫С������Խ�������
************************************************************************************/
int yx_3des_decrypt(char *in, char *out, int inlen, int *outlen);
/**********************************************************************************
*  in: ԭʼδ��������
*  out:���ܺ�����ݣ�ԭʼ���ȣ�δ���κδ���
*  inlen:ԭʼδ�������ݵĳ��ȣ�strlen��
*   *outlen:���洫����out�ĳ���
*   ����ֵ�� -1ʧ�ܡ�
*   ע�⣺���������ж�out���Ƿ�̫С������Խ�������
************************************************************************************/
int yx_3des_encrypt(char *in, char *out, int inlen, int *outlen);

#ifdef  __cplusplus
}
#endif

#endif /* !HEADER_CRYPTO_H */
