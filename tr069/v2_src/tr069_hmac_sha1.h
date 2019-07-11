/* HMAC-SHA1 implement. 
 * tr069_hmacSha1.h */

#ifndef _SHA1_H_
#define _SHA1_H_

#ifndef _SHA_enum_
#define _SHA_enum_
enum
{
    shaSuccess = 0,
    shaNull,            /* Null pointer parameter */
    shaInputTooLong,    /* input data too long */
    shaStateError       /* called Input after Result */
};
#endif

#define SHA1HashSize 20
typedef unsigned int DWORD;
typedef unsigned char BYTE;

typedef struct SHA1Context
{
    DWORD Intermediate_Hash[SHA1HashSize/4]; // Message Digest

    DWORD Length_Low;            // Message length in bits
    DWORD Length_High;            // Message length in bits

    int Message_Block_Index;    // Index into message block array
    unsigned char Message_Block[64];    // 512-bit message blocks

    int Computed;                // Is the digest computed?
    int Corrupted;                // Is the message digest corrupted?
} SHA1Context;

// Function Prototypes 
int GetSHA1String(char* sSource, unsigned length, unsigned char* sResult);

/* HMAC_SHA1 digest generate function, impletement according rfc2104. */
int HMAC_SHA1DigestGenerate(unsigned char *key, int key_len, 
                            unsigned char *text, int text_len, BYTE *digest);
void HMAC_SHA1HedaxOutput(unsigned char *digest, unsigned char *hedax);

#endif
