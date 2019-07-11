#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tr069_hmac_sha1.h"

// Define the SHA1 circular left shift macro 
#define SHA1CircularShift(bits, word) (((word) << (bits)) | ((word) >> (32-(bits))))

// Local Function Prototyptes */
void tr069_SHA1PadMessage(SHA1Context *);
void tr069_SHA1ProcessMessageBlock(SHA1Context *);
int tr069_SHA1Reset(SHA1Context *);
int tr069_SHA1Input(SHA1Context *, const unsigned char *, unsigned int);
int tr069_SHA1Result(SHA1Context *, unsigned char Message_Digest[SHA1HashSize]);

int tr069_SHA1Reset(SHA1Context *c)
{
    if (!c)
        return shaNull;

    c->Length_Low             = 0;
    c->Length_High            = 0;
    c->Message_Block_Index    = 0;

    c->Intermediate_Hash[0]   = 0x67452301;
    c->Intermediate_Hash[1]   = 0xEFCDAB89;
    c->Intermediate_Hash[2]   = 0x98BADCFE;
    c->Intermediate_Hash[3]   = 0x10325476;
    c->Intermediate_Hash[4]   = 0xC3D2E1F0;

    c->Computed   = 0;
    c->Corrupted  = 0;

    return shaSuccess;
}

int tr069_SHA1Result( SHA1Context *c, unsigned char Message_Digest[SHA1HashSize])
{
    int i;

    if (!c || !Message_Digest)
        return shaNull;

    if (c->Corrupted)
        return c->Corrupted;

    if (!c->Computed)
    {
        tr069_SHA1PadMessage(c);
        for(i = 0; i<64; c->Message_Block[++i] = 0)
            ;
        c->Length_Low = 0;    /* and clear length */
        c->Length_High = 0;
        c->Computed = 1;

    }

    for(i = 0; i < SHA1HashSize; ++i)
        Message_Digest[i] = c->Intermediate_Hash[i>>2] >> 8 * (3 - (i & 0x03));

    return shaSuccess;
}

int tr069_SHA1Input(SHA1Context *context, const unsigned char *message_array, unsigned length)
{
    if (!length)
        return shaSuccess;

    if (!context || !message_array)
        return shaNull;

    if (context->Computed)
    {
        context->Corrupted = shaStateError;
        return shaStateError;
    }

    if (context->Corrupted)
        return context->Corrupted;

    while(length-- && !context->Corrupted)
    {
        context->Message_Block[context->Message_Block_Index++] = (*message_array & 0xFF);
        context->Length_Low += 8;
        if (context->Length_Low == 0)
        {
            context->Length_High++;
            if (context->Length_High == 0)
            {
                /* Message is too long */
                context->Corrupted = 1;
            }
        }

        if (context->Message_Block_Index == 64)
            tr069_SHA1ProcessMessageBlock(context);

        message_array++;
    }

    return shaSuccess;
}

void tr069_SHA1ProcessMessageBlock(SHA1Context *context)
{
    const DWORD K[] =    { 0x5A827999, 0x6ED9EBA1, 0x8F1BBCDC, 0xCA62C1D6 };
    int   t;
    DWORD temp;
    DWORD W[80];
    DWORD A, B, C, D, E;

    /*
    *  Initialize the first 16 words in the array W
    */
    for(t = 0; t < 16; t++)
    {
        W[t]  = context->Message_Block[t * 4] << 24;
        W[t] |= context->Message_Block[t * 4 + 1] << 16;
        W[t] |= context->Message_Block[t * 4 + 2] << 8;
        W[t] |= context->Message_Block[t * 4 + 3];
    }

    for(t = 16; t < 80; t++)
        W[t] = SHA1CircularShift(1,W[t-3] ^ W[t-8] ^ W[t-14] ^ W[t-16]);

    A = context->Intermediate_Hash[0];
    B = context->Intermediate_Hash[1];
    C = context->Intermediate_Hash[2];
    D = context->Intermediate_Hash[3];
    E = context->Intermediate_Hash[4];

    for(t = 0; t < 20; t++)
    {
        temp =  SHA1CircularShift(5,A) +
            ((B & C) | ((~B) & D)) + E + W[t] + K[0];
        E = D;
        D = C;
        C = SHA1CircularShift(30,B);

        B = A;
        A = temp;
    }

    for(t = 20; t < 40; t++)
    {
        temp = SHA1CircularShift(5,A) + (B ^ C ^ D) + E + W[t] + K[1];
        E = D;
        D = C;
        C = SHA1CircularShift(30,B);
        B = A;
        A = temp;
    }

    for(t = 40; t < 60; t++)
    {
        temp = SHA1CircularShift(5,A) +
            ((B & C) | (B & D) | (C & D)) + E + W[t] + K[2];
        E = D;
        D = C;
        C = SHA1CircularShift(30,B);
        B = A;
        A = temp;
    }

    for(t = 60; t < 80; t++)
    {
        temp = SHA1CircularShift(5,A) + (B ^ C ^ D) + E + W[t] + K[3];
        E = D;
        D = C;
        C = SHA1CircularShift(30,B);
        B = A;
        A = temp;
    }

    context->Intermediate_Hash[0] += A;
    context->Intermediate_Hash[1] += B;
    context->Intermediate_Hash[2] += C;
    context->Intermediate_Hash[3] += D;
    context->Intermediate_Hash[4] += E;

    context->Message_Block_Index = 0;
}


void tr069_SHA1PadMessage(SHA1Context *context)
{
    if (context->Message_Block_Index > 55)
    {
        context->Message_Block[context->Message_Block_Index++] = 0x80;
        while(context->Message_Block_Index < 64)
            context->Message_Block[context->Message_Block_Index++] = 0;

        tr069_SHA1ProcessMessageBlock(context);

        while(context->Message_Block_Index < 56)
            context->Message_Block[context->Message_Block_Index++] = 0;
    }
    else
    {
        context->Message_Block[context->Message_Block_Index++] = 0x80;
        while(context->Message_Block_Index < 56)
            context->Message_Block[context->Message_Block_Index++] = 0;
    }

    /*
    *  Store the message length as the last 8 octets
    */
    context->Message_Block[56] = context->Length_High >> 24;
    context->Message_Block[57] = context->Length_High >> 16;
    context->Message_Block[58] = context->Length_High >> 8;
    context->Message_Block[59] = context->Length_High;
    context->Message_Block[60] = context->Length_Low >> 24;
    context->Message_Block[61] = context->Length_Low >> 16;
    context->Message_Block[62] = context->Length_Low >> 8;
    context->Message_Block[63] = context->Length_Low;

    tr069_SHA1ProcessMessageBlock(context);
}


int tr069_GetSHA1String(char* sSource, unsigned int length, BYTE *digest)
{
    SHA1Context context;
    int i;

    tr069_SHA1Reset(&context);
    tr069_SHA1Input(&context, (const BYTE*)sSource, length);
    tr069_SHA1Result(&context,digest);

    for (i = 0; i < 20; i++) {
        printf("%02X ",digest[i]);
    }

    printf("\n");

    return 0;
}

/* HMAC_SHA1 digest generate function, impletement according rfc2104. */
int HMAC_SHA1DigestGenerate(unsigned char *key, int key_len, 
                            unsigned char *text, int text_len, BYTE *digest)
{
	SHA1Context context;
	unsigned char k_ipad[64] = {0}; /* inner padding - key XORd with ipad. */
	unsigned char k_opad[64] = {0}; /* outer padding - key XORd with opad. */
    unsigned char tk[20] = {0};     /* tempt key - if key lenger than 64, H(k)*/
	unsigned char *inner_buf = NULL;
	unsigned char outer_buf[84] = {0}; /* inner digest(20 bytes) + message block(64 bytes) */
    int i;

    /* If key is lenger than 64 bytes, rest it to key = SHA1(key). */
    if(key_len > 64){
        SHA1Context tctx;
        tr069_SHA1Reset(&tctx);
        tr069_SHA1Input(&tctx, key, key_len);
        tr069_SHA1Result(&tctx,tk);

        key = tk;
        key_len = SHA1HashSize;
    }
	
    /* The HMAC_SHA1 transform looks like:
     * SHA1(key XOR opad, SHA1(key XOR ipad, text))
     * Where key is an n byte key, ipad is the byte 0x36 repeated 64 times,
     * opad is the byte 0x5C repeated 64 times, and text is the data being protected. */

    /* Start out by storing key in pads. */
	for(i = 0; i < key_len; i++){
		k_ipad[i] = k_opad[i] = key[i];
	}

    /* XOR key with ipad and opad values. */
    for(i = 0; i < 64; i++){
        k_ipad[i] ^= 0x36;
        k_opad[i] ^= 0x5C;
    }

    /* Perform inner SHA1 */
	inner_buf = (unsigned char*) malloc(text_len + 64);
	if(NULL == inner_buf){
		printf(" Allocate buffer failed\n");
		return -1;
	}
	memcpy(inner_buf, k_ipad, 64);
	memcpy(inner_buf+64, text, text_len);
    tr069_SHA1Reset(&context);
    tr069_SHA1Input(&context, inner_buf, text_len + 64);
    tr069_SHA1Result(&context,digest);
	if(NULL != inner_buf){
		free(inner_buf);
	}

    /* Perform outer SHA1 */
	memcpy(outer_buf, k_opad, 64);
	memcpy(outer_buf+64, digest, SHA1HashSize);
    tr069_SHA1Reset(&context);
    tr069_SHA1Input(&context, outer_buf, 84);
    tr069_SHA1Result(&context,digest);

    return 0;
}

void HMAC_SHA1HedaxOutput(unsigned char *digest, unsigned char *hedax)
{
	int i,j;
	char *dst = (char *)hedax;
	
	if(digest == NULL || hedax == NULL){
		return ;
	}
	
	for(i = j = 0; i < SHA1HashSize; i++, j += 2){
		sprintf(dst + j, "%02X", digest[i]);
	}
}

/* Test case. */
#if 0
int SHA1TestFunc(void){
	unsigned char *s1 = "abc";
    unsigned char *s2 = "The quick brown fox jumps over the lazy dog";
    unsigned char *s3 = "The quick brown fox jumps over the lazy cog";
    unsigned char digest[20] = {0};
    int i;

    printf("\nsrc = %s,\n",s1);
    GetSHA1String(s1, strlen(s1), digest);
    for(i = 0; i < 20; i++){
        printf("%02X ",digest[i]);
    }

    printf("\n\nsrc = %s,\n",s2);
    GetSHA1String(s2, strlen(s2), digest);
    for(i = 0; i < 20; i++){
        printf("%02X ",digest[i]);
    }

    printf("\n\nsrc = %s,\n",s3);
    GetSHA1String(s3, strlen(s3), digest);
    for(i = 0; i < 20; i++){
        printf("%02X ",digest[i]);
    }
}

int HMAC_SHA1TestFunc(void)
{
	unsigned char *key1 = "123";
	unsigned char *data1 = "456";
	unsigned char *key2 = "1111111111111111111111111111111111111111111111111111111111111111111111"; /* 70 bytes. */
	unsigned char *data2 = "456";
	unsigned char digest[20] = {0};
	unsigned char digestHedax[41] = {0};
	int i;

	HMAC_SHA1DigestGenerate(key1, strlen(key1), data1, strlen(data1), digest);

	HMAC_SHA1HedaxOutput(digest, digestHedax);
	printf("hedax: %s\n", digestHedax);

	HMAC_SHA1DigestGenerate(key2, strlen(key2), data2, strlen(data2), digest);
	
	HMAC_SHA1HedaxOutput(digest, digestHedax);
	printf("hedax: %s\n", digestHedax);
}

int main(void)
{
	SHA1TestFunc();
	printf("\n*************************\n");
	HMAC_SHA1TestFunc();
    return 0;
}
#endif
