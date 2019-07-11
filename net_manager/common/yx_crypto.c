#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "yx_crypto.h"
#include "aes.h"
#include "evp.h"
#include "md5.h"
#include "des.h"


#define MAX_LEN 4096

#if 0
unsigned char bcd_char(unsigned char data)
{
    switch(data) {
    case 0x0:
        return '0';
    case 0x1:
        return '1';
    case 0x2:
        return '2';
    case 0x3:
        return '3';
    case 0x4:
        return '4';
    case 0x5:
        return '5';
    case 0x6:
        return '6';
    case 0x7:
        return '7';
    case 0x8:
        return '8';
    case 0x9:
        return '9';
    case 0xa:
        return 'A';
    case 0xb:
        return 'B';
    case 0xc:
        return 'C';
    case 0xd:
        return 'D';
    case 0xe:
        return 'E';
    case 0xf:
        return 'F';
    }
}

static void bcd_get(unsigned char data, unsigned char bcd_data[2])
{
    unsigned char split_data = 0;

    split_data = (data & 0xf0) >> 4;
    bcd_data[0] = bcd_char(split_data);
    split_data = data & 0xf;
    bcd_data[1] = bcd_char(split_data);
}
#endif

static unsigned char hex_get(unsigned char data)
{
    if(data >= '0' && data <= '9')
        return data - '0';
    if(data >= 'A' && data <= 'F')
        return data - 'A' + 10;
    return 0;
}

static unsigned char hex_get_from_bcd(unsigned char bcd_data[2])
{
    unsigned char ret;

    ret = hex_get(bcd_data[0]);
    ret = ret << 4;
    ret += hex_get(bcd_data[1]);
    return ret;
}

int char_get_from_hex(unsigned char *hex, int hex_len, unsigned char *out_buf, int out_buf_len)
{
    int i;

    if((hex_len * 2 + 1) > out_buf_len) {
        //PRINTF("out len is too small:hex_len:%d, out_buf_len is %d\n", hex_len, out_buf_len);
        return  -1;
    }
    for(i = 0; i < hex_len; i++) {
        //bcd_get(hex[i], &out_buf[i * 2]);
        sprintf(&out_buf[2 * i], "%02X", hex[i]);
    }
    out_buf[hex_len * 2 + 1] = 0;
    //PRINTF("out buf is %s\n ", out_buf);
    return 0;
}
int hex_get_from_char(unsigned char *in, int in_len, unsigned char *hex_out_buf, int hex_out_buf_len)
{
    int i;

    if(in_len / 2 > hex_out_buf_len) {
        //PRINTF("hex_out_buf_len is small; in_len :%d, hex_out_buf_len :%d\n", in_len, hex_out_buf_len);
        return -1;
    }
    for(i = 0; i < in_len / 2; i ++) {
        hex_out_buf[i] = hex_get_from_bcd(&in[i * 2]);
    }
    return 0;
}

static int pkcs5_padding(const char *plain_data, int plain_data_len, char *padded_data, int padded_data_len, int block_size)
{
    int pad_len = plain_data_len % block_size;

    if(plain_data_len > padded_data_len) {
        //PRINTF("error to padding:%d:%d\n", plain_data_len, padded_data_len);
        return -1;
    }
    memcpy(padded_data, plain_data, plain_data_len);
    if(pad_len) {
        pad_len = block_size - pad_len;
        int i;
        for(i = 0; i < pad_len; i ++) {
            padded_data[plain_data_len + i] = pad_len;
        }
    }
    return pad_len + plain_data_len;
}


static int pkcs5_padding_remove(char *plain_data, int plain_data_len)//这里要保证plain_data有结束位存在，即plain_data得buf长度=plain_data_len + 1;否则会越界。
{
    if(plain_data[plain_data_len - 1] <= 16 && plain_data[plain_data_len - 1] >= 0) {
        int padding_value = plain_data[plain_data_len - 1];
        while(padding_value > 0) {
            if(plain_data[plain_data_len - padding_value] != plain_data[plain_data_len - 1])
                return -1;
            padding_value --;
        }
        plain_data[plain_data_len - plain_data[plain_data_len - 1]] = 0;
        return (plain_data_len - plain_data[plain_data_len - 1]);
    } else
        plain_data[plain_data_len] = 0;
    return plain_data_len;
}

int yx_aes_ecb_encrypt(char *plain_txt, int plain_txt_len, char *key, char *encrypted_txt, int *encrypted_txt_len)
{

    int encrypt_len = 0;
    int inc_len = 0;
    char input[MAX_LEN] = {0};
    char output[MAX_LEN] = {0};
    AES_KEY aes_ks;
    int mem_len = plain_txt_len + 16; //padding最多会补16位

    assert(plain_txt != NULL);
    assert(key != NULL);
    assert(encrypted_txt != NULL);
    AES_set_encrypt_key(key, 128, &aes_ks);
    if(plain_txt_len > MAX_LEN) {
      printf("app_aes_ecb_encrypt error :plain_txt_len :%d is  too longer for buf:%d\n", plain_txt_len, MAX_LEN);
      return -1;
    }
    encrypt_len = pkcs5_padding(plain_txt, plain_txt_len, input, mem_len, 16);
    if(*encrypted_txt_len < (encrypt_len  + 1) || encrypt_len < 0) {
      printf("error!encrypt_len < 0 or app_aes_ecb_encrypt encrypted_txt_len:%d is less than  plain_txt_len * 2 + 1 :%d\n", *encrypted_txt_len, (encrypt_len  + 1));
      return -1;
    }
    inc_len = 0;
    while(inc_len < encrypt_len) {
      AES_encrypt(&input[inc_len], &output[inc_len], &aes_ks);
      inc_len += 16;
    }

    memcpy(encrypted_txt, output, encrypt_len);
  //  *encrypted_txt_len = encrypt_len ;

    return encrypt_len;
}

int yx_aes_ecb_decrypt(char *encrypted_txt, int encrypted_txt_len, char *key, char *plain_txt, int *plain_txt_len)
{
    int encrypt_len = 0;
    int inc_len = 0;
    AES_KEY aes_ks;

    assert(plain_txt != NULL);
    assert(key != NULL);
    assert(encrypted_txt != NULL);
    if(*plain_txt_len <= encrypted_txt_len) {
        printf("app_aes_ecb_decrypt plain_txt_len :%d is less than encrypted_txt_len :%d\n", *plain_txt_len, encrypted_txt_len);
        return -1;
    }

    AES_set_decrypt_key(key, 128, &aes_ks);
    inc_len = 0;
    encrypt_len = encrypted_txt_len ;
    while(inc_len < encrypt_len) {
        AES_decrypt(&encrypted_txt[inc_len], &plain_txt[inc_len], &aes_ks);
        inc_len += 16;
    }

    *plain_txt_len = pkcs5_padding_remove(plain_txt, encrypt_len);
    return *plain_txt_len;
}


int yx_aes_cbc_encrypt(char *plain_txt, int plain_txt_len, char *key, char *vector, char *encrypted_txt, int * encrypted_txt_len)
{
    AES_KEY aes_ks;
    int encrypt_len = 0;
    char input[MAX_LEN + 1] = {0};
    int tmp_len;
    char output[MAX_LEN + 1] = {0};

    assert(plain_txt != NULL);
    assert(key != NULL);
    assert(encrypted_txt != NULL);
    assert(vector != NULL);
    tmp_len = plain_txt_len + 9;
    AES_set_encrypt_key(key, 128, &aes_ks);
    if(plain_txt_len > MAX_LEN) {
        //PRINTF("error :plain_txt_len :%d is  too longer for buf:%d\n", plain_txt_len, MAX_LEN);
        return -1;
    }
    encrypt_len = pkcs5_padding(plain_txt, plain_txt_len, input, tmp_len, 16);
    if(*encrypted_txt_len < (encrypt_len * 2  + 1)) {
        //PRINTF("encrypted_txt_len:%d is less than  plain_txt_len * 2 + 1 :%d\n", *encrypted_txt_len, (encrypt_len * 2 + 1));
        return -1;
    }
    AES_cbc_encrypt(input, output, encrypt_len, &aes_ks, vector , 1);
    char_get_from_hex(output, encrypt_len, encrypted_txt, *encrypted_txt_len);
    *encrypted_txt_len = encrypt_len * 2;
    return 0;
}

int yx_aes_cbc_decrypt(char *encrypted_txt, int encrypted_txt_len, char *key, char *vector, char *plain_txt, int *plain_txt_len)
{
    AES_KEY aes_ks;
    char hex[MAX_LEN] = {0};

    assert(plain_txt != NULL);
    assert(key != NULL);
    assert(encrypted_txt != NULL);
    if(*plain_txt_len < encrypted_txt_len / 2) {
        //PRINTF("plain_txt_len :%d is less than encrypted_txt_len / 2:%d\n", *plain_txt_len, encrypted_txt_len / 2);
        return -1;
    }
    if(encrypted_txt_len / 2 > MAX_LEN) {
        //PRINTF("error :encrypted_txt_len :%d is  too longer for buf:%d\n", encrypted_txt_len, MAX_LEN);
        return -1;
    }
    hex_get_from_char(encrypted_txt, encrypted_txt_len, hex, encrypted_txt_len / 2);
    AES_set_decrypt_key(key, 128, &aes_ks);
    AES_cbc_encrypt(hex, plain_txt, *plain_txt_len, &aes_ks, vector , 0);
    pkcs5_padding_remove(plain_txt, encrypted_txt_len / 2);
    return 0;
}

static unsigned char LOCALKEY[]=  "Hybroad Vision..";//?薷谋?????钥

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

    strncpy(key, LOCALKEY, key_len);
    key[key_len] = '\0';

    //printf("key = %s\n", key);
	if (!strlen(key))
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


int yx_base64_encode(char *in, int inLen, char *out)
{
    int ret = 0;

    if((NULL == in) || (NULL == out)) {
        printf("yx_base64_encode err: in or out is null!\n");
        return -1;
    }

   ret= EVP_EncodeBlock(out, in, inLen);
   if(-1 == ret){
        printf("yx_base64_encode err: encode error!\n");
        return -1;
   }

    return ret;
}

int yx_base64_decode(char *in, int inLen, char *out)
{
    int ret = 0;

    if((NULL == in) || (NULL == out)) {
        printf("yx_base64_decode err: in or out is null!\n");
        return -1;
    }

    ret = EVP_DecodeBlock(out, in, inLen);
    if(-1 == ret){
        printf("yx_base64_decode err: decode error!\n");
        return -1;
    }

    while(out[ret - 1] == '\0')  ret --;

    return ret;
}


/**********************************************************************************
*  source:用来MD5加密的数据
*  huaweipriv:是否需要传入华为私有的数据 “99991231”
*  flag: 1表示24位，2表示32位
*  updateflag :1:表示变大写，0表示不变化
*  MD5加密
************************************************************************************/
void yx_md5_get_key1(char *md5key, char *array, unsigned char *huaweipriv, int flag, int updateflag)
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

    MD5_Update(&ctx, (unsigned char *)array, strlen((char *)array));
    if(huaweipriv != NULL) //华为私有
        MD5_Update(&ctx,	huaweipriv, strlen((char *)huaweipriv));

    MD5_Final(digest, &ctx);

    for(i = 0; i < lenflag; i ++) {
        len += sprintf(buf + len, "%02x", digest[i]);
    }
    PRINTF("Create md5 len(%d) data(%s)\n", lenflag, buf);
    if(updateflag == 1) {
        for(i = 0; i < lenflag * 2; i++)
            buf [i] = toupper(buf[i]);
    }
    memcpy(md5key, buf, lenflag * 2);
    PRINTF("return md5key(%s)\n", md5key);
    return;
}

/**********************************************************************************
*    deskey: 24位的3DES的key
*
*
*
*  设置3DES的key1,key2,key3
************************************************************************************/
static DES_key_schedule ks1, ks2, ks3;
void yx_set_3des_key(char *deskey)
{
    int code_len , i/*,k*/;
    char Password[25] = {0};
    memcpy(Password, deskey, 24);

    DES_cblock key1, key2, key3;

    code_len = 24 - strlen(Password);
    PRINTF("code_len = %d\n", code_len);
    if(code_len != 0)
        for(i = 0; i < code_len; i++)
            strcat(Password, "0");

    for(i = 0; i < 24; i++) {
        if(i < 8) key1[i] = Password[i];
        else if(i < 16) key2[i - 8] = Password[i];
        else key3[i - 16] = Password[i];
        /*k= ( int ) i/8;
        switch( k )
        {
        	case 0:
        		key1[i]=(unsigned char)Password[i];
        		break;
        	case 1:
        		key2[i-8]=(unsigned char)Password[i];
        		break;
        	case 2:
        		key3[i-16]=(unsigned char)Password[i];
        		break;
        	default:
        			break;
        }*/
    }
    //set Key
    DES_set_key_unchecked(&key1, &ks1);
    DES_set_key_unchecked(&key2, &ks2);
    DES_set_key_unchecked(&key3, &ks3);

}

/**********************************************************************************
*  in: 加密后的数据 必须是8的倍数
*  out:解密后的数据，程序中默认加上结束符
*  inlen:加密数据的长度（strlen）
*   *outlen:储存传出的out的长度
*   返回值： -1失败。
*   注意：函数本身不判断out区是否太小，导致越界的问题
************************************************************************************/
int yx_3des_decrypt(char *in, char *out, int inlen, int *outlen)
{
    DES_cblock plain_text[40], output[40];
    char 	*string_text = in;
    int 	text_len, text_remainder;
    int 	i, j;

    if(in == NULL || out == NULL || inlen == 0) {
        ERROR("ERROR!!!!\n");
        return -1;
    }
    memset(out, 0, *outlen);
    text_len = inlen / 8  ;
    text_remainder = inlen % 8;

    if(text_remainder != 0) {
        ERROR("the string is not a decrypte by 3des\n");
        return -1;
    }
    PRINTF("text_len=%d,text_remain=%d\n", text_len, text_remainder);

    for(i = 0; i < text_len; i++)
        for(j = 0; j < 8; j++)
            plain_text[i][j] = string_text[ i * 8 + j ];


    for(i = 0; i < text_len; i++) {
        PRINTF("old:%02x:%02x:%02x:%02x\n", plain_text[i][0], plain_text[i][1], plain_text[i][2], plain_text[i][3]);
        PRINTF("old:%02x:%02x:%02x:%02x\n", plain_text[i][4], plain_text[i][5], plain_text[i][6], plain_text[i][7]);
        DES_ecb3_encrypt(&plain_text[i], &output[i], &ks1, &ks2, &ks3, 0);
        PRINTF("decrypt output[1-4]=%02x:%02x:%02x:%02x\n", output[i][0], output[i][1], output[i][2], output[i][3]);
        PRINTF("decrypt output[1-4]=%c:%c:%c:%c\n", output[i][0], output[i][1], output[i][2], output[i][3]);
        PRINTF("decrypt output[5-8]=%02x:%02x:%02x:%02x\n", output[i][4], output[i][5], output[i][6], output[i][7]);
        PRINTF("decrypt output[5-8]=%c:%c:%c:%c\n", output[i][4], output[i][5], output[i][6], output[i][7]);
    }

    int len = 0;
    for(i = 0; i < text_len; i++) {
        for(j = 0; j < 8; j++) {
            len += sprintf(out + len, "%c", output[i][j]);
        }
    }
    int templen = 0;
    if(output[text_len - 1][7] == 0x01) {
        templen = -1;
    } else if((output[text_len - 1][7] == 0x02) && (output[text_len - 1][6] == 0x02)) {
        templen = -2;
    } else if((output[text_len - 1][7] == 0x03) && (output[text_len - 1][6] == 0x03) && (output[text_len - 1][5] == 0x03)) {
        templen = -3;
    } else if((output[text_len - 1][7] == 0x04) && (output[text_len - 1][6] == 0x04) && (output[text_len - 1][5] == 0x04) && (output[text_len - 1][4] == 0x04)) {
        templen = -4;
    } else if((output[text_len - 1][7] == 0x05) && (output[text_len - 1][6] == 0x05) && (output[text_len - 1][5] == 0x05) && (output[text_len - 1][4] == 0x05)
              && (output[text_len - 1][3] == 0x05)) {
        templen = -5;
    } else if((output[text_len - 1][7] == 0x06) && (output[text_len - 1][6] == 0x06) && (output[text_len - 1][5] == 0x06) && (output[text_len - 1][4] == 0x06)
              && (output[text_len - 1][3] == 0x06) && (output[text_len - 1][2] == 0x06)) {
        templen = -6;
    } else if((output[text_len - 1][7] == 0x07) && (output[text_len - 1][6] == 0x07) && (output[text_len - 1][5] == 0x07) && (output[text_len - 1][4] == 0x07)
              && (output[text_len - 1][3] == 0x07) && (output[text_len - 1][2] == 0x07) && (output[text_len - 1][1] == 0x07)) {
        templen = -7;
    } else if((output[text_len - 1][7] == 0x08) && (output[text_len - 1][6] == 0x08) && (output[text_len - 1][5] == 0x08) && (output[text_len - 1][4] == 0x08)
              && (output[text_len - 1][3] == 0x08) && (output[text_len - 1][2] == 0x08) && (output[text_len - 1][1] == 0x08) && (output[text_len - 1][0] == 0x08)) {
        templen = -8;
    }
    *outlen = inlen + templen;
    PRINTF("==%02x==\n", *(out + *outlen));

    *(out + *outlen) = '\0';
    PRINTF("===out =####%s#####\n", out);
    PRINTF("\n");
    return 0;
}

/**********************************************************************************
*  in: 原始未加密数据
*  out:加密后的数据，原始长度，未做任何处理
*  inlen:原始未加密数据的长度（strlen）
*   *outlen:储存传出的out的长度
*   返回值： -1失败。
*   注意：函数本身不判断out区是否太小，导致越界的问题
************************************************************************************/
int yx_3des_encrypt(char *in, char *out, int inlen, int *outlen)
{

    DES_cblock plain_text[40], output[40];
    char 	*string_text = in;
    int 	text_len, text_remainder;
    int 	i, j;

    if(in == NULL || out == NULL || inlen == 0) {
        ERROR("ERROR!!!!\n");
        return -1;
    }
    memset(out, 0, *outlen);
    text_len = inlen / 8 + 1;
    text_remainder = inlen % 8;


    PRINTF("text_len=%d,text_remain=%d\n", text_len, text_remainder);

    for(i = 0; i < text_len; i++) {
        for(j = 0; j < 8; j++) {
            if(i == text_len - 1 && j >= text_remainder) {
                switch(text_remainder) {
                case 0:
                    PRINTF("i=%d,j=%d\n", i, j);
                    plain_text[i][j] = 0x08;
                    break;
                case 1:
                    plain_text[i][j] = 0x07;
                    break;

                case 2:
                    plain_text[i][j] = 0x06;
                    break;

                case 3:
                    plain_text[i][j] = 0x05;
                    break;

                case 4:
                    plain_text[i][j] = 0x04 ;
                    break;

                case 5:
                    plain_text[i][j] = 0x03;
                    break;

                case 6:
                    plain_text[i][j] = 0x02;
                    break;

                case 7:
                    plain_text[i][j] = 0x01;
                    break;
                default:
                    break;
                }
            } else {
                PRINTF("----i=%d,j=%d==%d\n", i, j, i * 8 + j);
                plain_text[i][j] = string_text[ i * 8 + j ];
            }
        }
    }

    for(i = 0; i < text_len; i++) {
        DES_ecb3_encrypt(&plain_text[i], &output[i], &ks1, &ks2, &ks3, 1);
        PRINTF("decrypt output[1-4]=%02x:%02x:%02x:%02x\n", output[i][0], output[i][1], output[i][2], output[i][3]);
        //PRINTF("decrypt output[1-4]=%c:%c:%c:%c\n",output[i][0],output[i][1],output[i][2],output[i][3]);
        PRINTF("decrypt output[5-8]=%02x:%02x:%02x:%02x\n", output[i][4], output[i][5], output[i][6], output[i][7]);
        //	PRINTF("decrypt output[5-8]=%c:%c:%c:%c\n",output[i][4],output[i][5],output[i][6],output[i][7]);
    }

    for(i = 0; i < text_len; i++) {
        PRINTF("iii=%d\n", i);
        memcpy(out, output[i], 8);
        out += 8;
    }
    *outlen = text_len * 8;
    return 0;
}

