/**
 * @file OSOpenSSL.cpp
 * @brief
 * @author Michael
 * @version 1.0
 * @date 2012-09-14
 */
#include "OSOpenSSL.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include <iostream>
#include <fstream>

#define _FSTREAM_SWITCH_TO_FOPEN //Android <fstream> libstlport.so have one bug; will delete when libstlport.so is ok or remove 'else'

using namespace std;
namespace OpenSSL {
/**
 * @brief RSACrypto
 */
RSACrypto::RSACrypto()
    : m_RsaKey(NULL)
    , m_RsaFlen(0)
    , m_RsaType(ePUBEncrypt)
    , m_Padding(RSA_PKCS1_PADDING)
{

}

/**
 * @brief RSACrypto
 *
 * @param nRsaType
 * @param nPadding
 */
RSACrypto::RSACrypto(const int& nRsaType, const int& nPadding)
    : m_RsaType(nRsaType)
    , m_RsaFlen(0)
    , m_Padding(nPadding)
    , m_RsaKey(NULL)
{

}

/**
 * @brief ~RSACrypto
 */
RSACrypto::~RSACrypto()
{
    if (m_RsaKey)
        RSA_free(m_RsaKey);
}

/**
 * @brief CreateRSA
 *
 * @param nKeyStr
 * @param nKeyPos
 * @param nKeyType
 *
 * @return
 */
RSA*
RSACrypto::CreateRSA(const string& nKeyStr, const int& nKeyPos, const int& nKeyType)
{
    if (nKeyStr.empty())
        return NULL;

    BIO* pBioKey = NULL;
    RSA* lRsaKey = NULL;
    if (nKeyPos == eKeyInFile) {
        /* BIO_new_file:
         * creates a new file BIO with mode, the meaning of mode is the same as the stdio function fopen() */
        pBioKey = BIO_new_file(nKeyStr.c_str(), "r");
    } else {
        /* BIO_new_mem_buf:
         * creates a memory BIO using len bytes of data at buf, if len is -1 then the buf is assumed to be null terminated */
        pBioKey = BIO_new_mem_buf((void*)nKeyStr.c_str(), -1);
    }
    if (!pBioKey) {
        LogToolsError("OpenSSL: can not new bio.\n");
        return NULL;
    }
    if (nKeyType == ePUBEncrypt) {
        /* PEM_read_bio_RSA_PUBKEY:
         * process an RSA public key using an RSA structure.
         * However the public key is encoded using a SubjectPublicKeyInfo structure and
         * an error occurs if the public key is not RSA */
        PEM_read_bio_RSA_PUBKEY(pBioKey, &lRsaKey, NULL, NULL);
    } else {
        /* PEM_read_bio_RSAPrivateKey:
         * process an RSA private key using an RSA structure.
         * It handles the same formats as the PrivateKey functions but
         * an error occurs if the private key is not RSA */
        PEM_read_bio_RSAPrivateKey(pBioKey, &lRsaKey, NULL, NULL);
    }
    BIO_free_all(pBioKey);

    if (lRsaKey) {
        m_RsaFlen = RSA_size(lRsaKey);
        m_RsaKey  = lRsaKey;
    }
    return lRsaKey;
}

/**
 * @brief EncryptoBuff
 *
 * @param nLen
 * @param nFrom
 * @param nTo
 *
 * @return
 */
int
RSACrypto::EncryptoBuff(const int& nLen, const unsigned char* nFrom, unsigned char* nTo)
{
    if (m_RsaType == ePUBEncrypt) {
        /* RSA_public_encrypt:
         * encrypts the lLen bytes at
         * pRawdata(from) using the public key m_RsaKey and stores the ciphertext in pEncData(to) */
        return RSA_public_encrypt(nLen, nFrom, nTo, m_RsaKey, m_Padding);
    } else {
        /* RSA_private_encrypt:
         * signs the lLen bytes at from(pRawData) using the private key rsa(m_RsaKey) and
         * stores the signature in to(pEncData). to must point to RSA_size(rsa) bytes of memory */
        return RSA_private_encrypt(nLen, nFrom, nTo, m_RsaKey, m_Padding);
    }
}

/**
 * @brief DecryptoBuff
 *
 * @param nLen
 * @param nFrom
 * @param nTo
 *
 * @return
 */
int
RSACrypto::DecryptoBuff(const int& nLen, const unsigned char* nFrom, unsigned char* nTo)
{
    if (m_RsaType == ePUBEncrypt) {
        /* RSA_private_decrypt:
         * decrypts the m_RsaFlen bytes at pEncData(from) using the private key m_RsaKey and stores the plaintext in pDecData(to).
         * pDecData(to) must point to a memory section large enough to hold the decrypted data */
        return RSA_private_decrypt(nLen, nFrom, nTo, m_RsaKey, m_Padding);
    } else {
        /* RSA_public_decrypt:
         * recovers the message digest from the m_RsaFlen bytes long signature at pEncData using the signer's public key m_RsaKey,
         * pDecData must point to a memory section large enough to hold the message digest */
        return RSA_public_decrypt(nLen, nFrom, nTo, m_RsaKey, m_Padding);
    }
}

/**
 * @brief Encrypto encrypts plaintext to ciphertext
 *
 * @param nRawDataPath  plaintext
 * @param nEncryptPath  ciphertext
 *
 * @return -1 one error
 */
int
RSACrypto::EncryptoFile(const string& nRawDataPath, const string& nEncryptPath)
{
    if (nRawDataPath.empty() || nEncryptPath.empty() || !m_RsaKey)
        return -1;

    int lRet = 0, lLen = 0;

#if defined(_FSTREAM_SWITCH_TO_FOPEN)
    FILE* lFin = fopen(nRawDataPath.c_str(), "rb");
    if (!lFin)
        return -1;
    FILE* lFout = fopen(nEncryptPath.c_str(), "wb");
    if (!lFout) {
        fclose(lFin);
        return -1;
    }

    int lSize = GetBuffSize(m_RsaKey);
    unsigned char* pRawData = NULL;
    unsigned char* pEncData = NULL;
    if (!(pRawData = (unsigned char*)calloc(1, m_RsaFlen + 1))) {
        fclose(lFin);
        fclose(lFout);
        return -1;
    }
    if (!(pEncData = (unsigned char*)calloc(1, m_RsaFlen + 1))) {
        fclose(lFin);
        fclose(lFout);
        free(pRawData);
        return -1;
    }

    while (!feof(lFin)) {
        memset(pRawData, 0, m_RsaFlen + 1);
        memset(pEncData, 0, m_RsaFlen + 1);
        lLen = fread(pRawData, 1, lSize, lFin);
        if (lLen > 0) {
            /* Problem about RSA_NO_PADDING:
             *   A 1,024-bit RSA key works on 1,024 bits of input and returns 1,024 bits of output.
             *   The actual numerical value of the input can be any positive integer smaller than the RSA modulus.
             *   In order to have 1,024 bits worth of input,
             *   a left-padding with 'zeros'(memset(pRawData, 0, m_RsaFlen + 1)) is assumed by RSA_NO_PADDING */
            if (m_Padding == eRsaNO)
                lRet = EncryptoBuff(m_RsaFlen, pRawData, pEncData);
            else
                lRet = EncryptoBuff(lLen, pRawData, pEncData);
            if (lRet < 0) {
                fclose(lFin);
                fclose(lFout);
                free(pRawData);
                free(pEncData);
                return -1;
            }
            fwrite(pEncData, 1, lRet, lFout);
        }
    }

    /* Release resource */
    fclose(lFin);
    fflush(lFout);
    fclose(lFout);
    free(pRawData);
    free(pEncData);
#else
    ifstream lFin(nRawDataPath.c_str());
    if (lFin.fail())
        return -1;
    ofstream lFout(nEncryptPath.c_str());
    if (lFout.fail())
        return -1;

    int lSize = GetBuffSize(m_RsaKey);
    unsigned char* pRawData = NULL;
    unsigned char* pEncData = NULL;
    if (!(pRawData = (unsigned char*)calloc(1, m_RsaFlen + 1))) {
        lFin.close();
        lFout.close();
        return -1;
    }
    if (!(pEncData = (unsigned char*)calloc(1, m_RsaFlen + 1))) {
        lFin.close();
        lFout.close();
        free(pRawData);
        return -1;
    }

    while (!lFin.eof()) {
        memset(pRawData, 0, m_RsaFlen + 1);
        memset(pEncData, 0, m_RsaFlen + 1);
        lFin.read((char*)pRawData, lSize);
        lLen = lFin.gcount();
        if (lLen > 0) {
            /* Problem about RSA_NO_PADDING:
             *   A 1,024-bit RSA key works on 1,024 bits of input and returns 1,024 bits of output.
             *   The actual numerical value of the input can be any positive integer smaller than the RSA modulus.
             *   In order to have 1,024 bits worth of input,
             *   a left-padding with 'zeros'(memset(pRawData, 0, m_RsaFlen + 1)) is assumed by RSA_NO_PADDING */
            if (m_Padding == eRsaNO)
                lRet = EncryptoBuff(m_RsaFlen, pRawData, pEncData);
            else
                lRet = EncryptoBuff(lLen, pRawData, pEncData);
            if (lRet < 0) {
                lFin.close();
                lFout.close();
                free(pRawData);
                free(pEncData);
                return -1;
            }
            lFout.write((char*)pEncData, lRet);
        }
    }

    /* Release resource */
    lFin.close();
    lFout.close();
    free(pRawData);
    free(pEncData);
#endif
    return 0;
}

/**
 * @brief Decrypto decrypts ciphertext to plaintext
 *
 * @param nEncryptPath ciphertext
 * @param nDecryptPath plaintext
 *
 * @return -1 on error
 */
int
RSACrypto::DecryptoFile(const string& nEncryptPath, const string& nDecryptPath)
{
    if (nDecryptPath.empty() || nEncryptPath.empty() || !m_RsaKey)
        return -1;

    int lRet = 0, lLen = 0;

#if defined(_FSTREAM_SWITCH_TO_FOPEN)
    FILE* lFin = fopen(nEncryptPath.c_str(), "rb");
    if (!lFin)
        return -1;
    FILE* lFout = fopen(nDecryptPath.c_str(), "wb");
    if (!lFout) {
        fclose(lFin);
        return -1;
    }

    unsigned char* pEncData = NULL;
    unsigned char* pDecData = NULL;
    if (!(pEncData = (unsigned char*)calloc(1, m_RsaFlen + 1))) {
        fclose(lFin);
        fclose(lFout);
        return -1;
    }
    if (!(pDecData = (unsigned char*)calloc(1, m_RsaFlen + 1))) {
        fclose(lFin);
        fclose(lFout);
        free(pEncData);
        return -1;
    }

    while (!feof(lFin)) {
        memset(pEncData, 0, m_RsaFlen + 1);
        memset(pDecData, 0, m_RsaFlen + 1);
        lLen = fread(pEncData, 1, m_RsaFlen, lFin);
        if (lLen > 0) {
            lRet = DecryptoBuff(lLen, pEncData, pDecData);
            if (lRet < 0) {
                fclose(lFin);
                fclose(lFout);
                free(pEncData);
                free(pDecData);
                LogToolsError("decrypto, check the buffer size.\n");
                return -1;
            }
            /* Normal: m_RsaFlen == lLen == lRet, if not, something is wrong! */
            if (m_Padding != eRsaNO) {
                fwrite(pDecData, 1, lRet, lFout);
                continue;
            }
            /* That's annoying about RSA_NO_PADDING, padded with more 'zeros' */
            for (int i = 0; i < lRet; ++i) {
                if (pDecData[i])
                    fputc(pDecData[i], lFout);
            }
        }
    }

    /* Release resouce */
    fclose(lFin);
    fflush(lFout);
    fclose(lFout);
    free(pEncData);
    free(pDecData);
#else
    ifstream lFin(nEncryptPath.c_str());
    if (lFin.fail())
        return -1;
    ofstream lFout(nDecryptPath.c_str());
    if (lFout.fail())
        return -1;

    unsigned char* pEncData = NULL;
    unsigned char* pDecData = NULL;
    if (!(pEncData = (unsigned char*)calloc(1, m_RsaFlen + 1))) {
        lFin.close();
        lFout.close();
        return -1;
    }
    if (!(pDecData = (unsigned char*)calloc(1, m_RsaFlen + 1))) {
        lFin.close();
        lFout.close();
        free(pEncData);
        return -1;
    }

    while (!lFin.eof()) {
        memset(pEncData, 0, m_RsaFlen + 1);
        memset(pDecData, 0, m_RsaFlen + 1);
        lFin.read((char*)pEncData, m_RsaFlen);
        lLen = lFin.gcount();
        if (lLen > 0) {
            lRet = DecryptoBuff(lLen, pEncData, pDecData);
            if (lRet < 0) {
                lFin.close();
                lFout.close();
                free(pEncData);
                free(pDecData);
                LogToolsError("decrypto, check the buffer size.\n");
                return -1;
            }
            /* Normal: m_RsaFlen == lLen == lRet, if not, something is wrong! */
            if (m_Padding != eRsaNO) {
                lFout.write((char*)pDecData, lRet);
                continue;
            }
            /* That's annoying about RSA_NO_PADDING, padded with more 'zeros' */
            for (int i = 0; i < lRet; ++i) {
                if (pDecData[i])
                    lFout.put(pDecData[i]);
            }
        }
    }

    /* Release resouce */
    lFin.close();
    lFout.close();
    free(pEncData);
    free(pDecData);
#endif
    return 0;
}

/**
 * @brief GetBuffSize
 *
 * @param nRsa
 *
 * @return
 */
int
RSACrypto::GetBuffSize(const RSA* nRsa)
{
    if (NULL==nRsa)
        return -1;
    /* RSA_size:
     * This function returns the RSA modulus size in bytes.
     * It can be used to determine how much memory must be allocated for an RSA encrypted value */
     int lSize = RSA_size(nRsa);
    /* lSize must be less than RSA_size(rsa) - 11 for the PKCS #1 v1.5 based padding modes,
     * less than RSA_size(rsa) - 41 for RSA_PKCS1_OAEP_PADDING
     * and exactly RSA_size(rsa) for RSA_NO_PADDING. */
    switch (m_Padding) {
        case RSA_PKCS1_PADDING:
            lSize -= 11;
            break;
        case RSA_SSLV23_PADDING:
            lSize -= 11;
            break;
        case RSA_NO_PADDING:
            lSize = lSize;
            break;
        case RSA_PKCS1_OAEP_PADDING:
            lSize = lSize - 2 * SHA_DIGEST_LENGTH - 2;
            break;
        case RSA_X931_PADDING:
            lSize -= 2;
            break;
        default:
            return -1;
    }
    return lSize;
}


/**
 * @brief MD5Crypto construct
 */
MD5Crypto::MD5Crypto() :
    m_ContextA((unsigned long)0x67452301L),
	m_ContextB((unsigned long)0xefcdab89L),
	m_ContextC((unsigned long)0x98badcfeL),
	m_ContextD((unsigned long)0x10325476L)
{
    memset(&m_Context, 0, sizeof(MD5_CTX));
}

/**
 * @brief MD5Crypto
 *
 * @param nA
 * @param nB
 * @param nC
 * @param nD
 */
MD5Crypto::MD5Crypto(long nA, long nB, long nC, long nD) :
    m_ContextA((unsigned long)nA),
    m_ContextB((unsigned long)nB),
    m_ContextC((unsigned long)nC),
    m_ContextD((unsigned long)nD)
{
    memset(&m_Context, 0, sizeof(MD5_CTX));
}

/**
 * @brief ~MD5Crypto destruct
 */
MD5Crypto::~MD5Crypto()
{
}

/**
 * @brief SetContext
 *
 * @param nA
 * @param nB
 * @param nC
 * @param nD
 */
void
MD5Crypto::SetContext(long nA, long nB, long nC, long nD)
{
	m_ContextA = (unsigned long)nA;
	m_ContextB = (unsigned long)nB;
	m_ContextC = (unsigned long)nC;
	m_ContextD = (unsigned long)nD;
}


/**
 * @brief CreateDigest calculate the MD5 digest from buff
 *
 * @param buff
 * @param len
 *
 * @return string digest
 */
string
MD5Crypto::CreateDigest(const char* buff, int len)
{
    if (!buff || len < 0)
        return "";

    unsigned char lDigest[MD5_DIGEST_LENGTH] = { 0 };
    /* MD5_Init */
    {
        memset(&m_Context, 0, sizeof(MD5_CTX));
        m_Context.A = m_ContextA;
        m_Context.B = m_ContextB;
        m_Context.C = m_ContextC;
        m_Context.D = m_ContextD;
    }

    /* MD5_Update */
    MD5_Update(&m_Context, buff, len);

    /* MD5_Final */
    MD5_Final(lDigest, &m_Context);

    /* Hex16 - 32 string */
    string lResult("");
    const unsigned char lHex[] = "0123456789ABCDEF";
    for(int i = 0; i < MD5_DIGEST_LENGTH; ++i) {
        lResult.append(1, lHex[lDigest[i]>>4]); /* hight 4 bit */
        lResult.append(1, lHex[lDigest[i] & 0x0F]); /* low 4 bit */
    }
    return lResult;
}

/**
 * @brief CreateDigest calculate the MD5 digest of one file
 *
 * @param nFile the raw text file path
 *
 * @return string digest
 */
string
MD5Crypto::CreateDigest(const string& nFile)
{
    if (nFile.empty())
        return "";

    char lBuffer[1024] = { 0 };
    unsigned char lDigest[MD5_DIGEST_LENGTH] = { 0 };

#if defined(_FSTREAM_SWITCH_TO_FOPEN)
    FILE* lFin = fopen(nFile.c_str(), "rb");
    if (!lFin)
        return "";

    {
        /* MD5_Init */
        memset(&m_Context, 0, sizeof(MD5_CTX));
        m_Context.A = m_ContextA;
        m_Context.B = m_ContextB;
        m_Context.C = m_ContextC;
        m_Context.D = m_ContextD;
    }

    int lLen = 0;
    while (!feof(lFin)) {
        lLen = fread(lBuffer, 1, 1024, lFin);
        if (lLen > 0)
            MD5_Update(&m_Context, lBuffer, lLen);
    }
    /*  places the message digest in lDigest,
     *  which must have space for MD5_DIGEST_LENGTH == 16 bytes of output */
    MD5_Final(lDigest, &m_Context);
    string lResult("");
    const unsigned char lHex[] = "0123456789ABCDEF";
    for(int i = 0; i < MD5_DIGEST_LENGTH; ++i) {
        lResult.append(1, lHex[lDigest[i]>>4]); /* hight 4 bit */
        lResult.append(1, lHex[lDigest[i] & 0x0F]); /* low 4 bit */
    }

    fclose(lFin);
#else
    ifstream lFin(nFile.c_str());
    if (lFin.fail())
        return "";

    {
        /* MD5_Init */
        memset(&m_Context, 0, sizeof(MD5_CTX));
        m_Context.A = m_ContextA;
        m_Context.B = m_ContextB;
        m_Context.C = m_ContextC;
        m_Context.D = m_ContextD;
    }

    int lLen = 0;
    while (!lFin.eof()) {
        lFin.read(lBuffer, 1024);
        lLen = lFin.gcount();
        if (lLen > 0)
            MD5_Update(&m_Context, lBuffer, lLen);
    }
    /*  places the message digest in lDigest,
     *  which must have space for MD5_DIGEST_LENGTH == 16 bytes of output */
    MD5_Final(lDigest, &m_Context);

    string lResult("");
    const unsigned char lHex[] = "0123456789ABCDEF";
    for(int i = 0; i < MD5_DIGEST_LENGTH; ++i) {
        lResult.append(1, lHex[lDigest[i]>>4]); /* hight 4 bit */
        lResult.append(1, lHex[lDigest[i] & 0x0F]); /* low 4 bit */
    }

    lFin.close();
#endif
    return lResult;
}

/**
 * @brief ObtainDigest get the digest string from given nFile
 *
 * @param nFile md5 file
 * @param nFlag enum
 *          # eHead-digest placed in the file head
 *          # eTail-digest placed in the file tail
 *
 * @return
 */
string
MD5Crypto::ObtainDigest(const string& nFile, const int& nFlag)
{
    if (nFile.empty())
        return "";

    char lBuffer[MD5_STRING_LENGTH+1] = { 0 };
#if defined(_FSTREAM_SWITCH_TO_FOPEN)
    FILE* lFin = fopen(nFile.c_str(), "rb");
    if (!lFin)
        return "";

    fseek(lFin, 0L, SEEK_END);
    long lFileSize = ftell(lFin);
    if (lFileSize < MD5_STRING_LENGTH)
        return "";

    /* seek the lFin according by digest position */
    if (eTail == nFlag)
        fseek(lFin, -MD5_STRING_LENGTH, SEEK_END);
    else
        fseek(lFin, 0L, SEEK_SET);

    int lch;
    for (int i = 0; i < MD5_STRING_LENGTH; ++i) {
        lch = fgetc(lFin);
        if ('a' <= lch && lch <= 'z')
            lch = lch - 32;
        lBuffer[i] = lch;
    }
#else
    ifstream lFin(nFile.c_str());
    if (lFin.fail())
        return "";

    lFin.seekg(0, ios::end);
    long lFileSize = (long)lFin.tellg();
    if (lFileSize < MD5_STRING_LENGTH)
        return "";

    /* seek the lFin according by digest position */
    if (eTail == nFlag)
        lFin.seekg(-MD5_STRING_LENGTH, ios::end);
    else
        lFin.seekg(0, ios::beg);

    char lch;
    for (int i = 0; i < MD5_STRING_LENGTH; ++i) {
        lFin.get(lch);
        if ('a' <= lch && lch <= 'z')
            lch = lch - 32;
        lBuffer[i] = lch;
    }
#endif
    return string(lBuffer);
}

/**
 * @brief RemoveDigest restore md5 file contained of digest to plain text
 *
 * @param nMd5File md5 file
 * @param nRawFile file wihout md5 digest
 * @param nFlag enum
 *          # eHead-digest placed in the file head
 *          # eTail-digest placed in the file tail
 *
 * @return
 */
int
MD5Crypto::RemoveDigest(const string& nMd5File, const string& nRawFile, const int& nFlag)
{
    if (nRawFile.empty() || nMd5File.empty())
        return -1;

#if defined(_FSTREAM_SWITCH_TO_FOPEN)
    FILE* lFin = fopen(nMd5File.c_str(), "rb");
    if (!lFin)
        return -1;
    FILE* lFout = fopen(nRawFile.c_str(), "wb");
    if (!lFout) {
        fclose(lFin);
        return -1;
    }

    fseek(lFin, 0, SEEK_END);
    const long lFileSize = ftell(lFin) - MD5_STRING_LENGTH;
    if (lFileSize <= 0)
        return -1;

    if (eHead == nFlag)
        fseek(lFin, MD5_STRING_LENGTH, SEEK_SET);
    else
        fseek(lFin, 0L, SEEK_SET);

    long lAccLen = 0;
    int  lch = 0;
    while(!feof(lFin) && lAccLen++ < lFileSize) {
        if (EOF != (lch = fgetc(lFin)))
            fputc(lch, lFout);
    }

    fclose(lFin);
    fflush(lFout);
    fclose(lFout);
#else
    ifstream lFin(nMd5File.c_str());
    if (lFin.fail())
        return -1;
    ofstream lFout(nRawFile.c_str());
    if (lFout.fail())
        return -1;

    lFin.seekg(0, ios::end);
    const long lFileSize = (long)lFin.tellg() - MD5_STRING_LENGTH;
    if (lFileSize <= 0)
        return -1;

    if (eHead == nFlag)
        lFin.seekg(MD5_STRING_LENGTH, ios::beg);
    else
        lFin.seekg(0, ios::beg);

    char lch;
    long lAccLen = 0;
    while (lFin.get(lch) && lAccLen++ < lFileSize)
        lFout << lch;

    lFin.close();
    lFout.close();
#endif
    return 0;
}

/**
 * @brief AppendDigest
 *
 * @param nRawFile raw plain text
 * @param nMd5File file addpended digest
 * @param nFlag
 *          # eHead-digest placed in the file head
 *          # eTail-digest placed in the file tail
 *
 * @return
 */
int
MD5Crypto::AppendDigest(const string& nRawFile, const string& nMd5File, const int& nFlag)
{
    if (nRawFile.empty() || nMd5File.empty())
        return -1;

    long lLen = 0;
    char lBuffer[1024] = { 0 };
    string lDigest = CreateDigest(nRawFile);
    if (lDigest.empty())
        return -1;

#if defined(_FSTREAM_SWITCH_TO_FOPEN)
    FILE* lFin = fopen(nRawFile.c_str(), "rb");
    if (!lFin)
        return -1;
    FILE* lFout = fopen(nMd5File.c_str(), "wb");
    if (!lFout) {
        fclose(lFin);
        return -1;
    }

    if (eHead == nFlag)
        fwrite(lDigest.c_str(), 1, lDigest.length(), lFout);
    while (!feof(lFin)) {
        lLen = fread(lBuffer, 1, 1024, lFin);
        if (lLen > 0)
            fwrite(lBuffer, 1, lLen, lFout);
    }
    if (eTail == nFlag)
        fwrite(lDigest.c_str(), 1, lDigest.length(), lFout);

    fclose(lFin);
    fflush(lFout);
    fclose(lFout);
#else
    ifstream lFin(nRawFile.c_str());
    if (lFin.fail())
        return -1;
    ofstream lFout(nMd5File.c_str());
    if (lFout.fail())
        return -1;

    if (eHead == nFlag)
        lFout << lDigest.c_str();
    while (!lFin.eof()) {
        lFin.read(lBuffer, 1024);
        lFout.write(lBuffer, lFin.gcount());
    }
    if (eTail == nFlag)
        lFout << lDigest.c_str();

    lFin.close();
    lFout.close();
#endif
    return 0;
}
#if 0
int
main ( int argc, char *argv[] )
{
    RSACrypto lRsa(OpenSSL::RSACrypto::ePRIEncrypt, OpenSSL::RSACrypto::eRsaNO);
    cout << "Encrypto" << endl;
    lRsa.SetPriKey("./private.pem", OpenSSL::RSACrypto::eKeyInFile);
    cout << lRsa.EncryptoFile("./stbRaw.cfg", "./stbEnc.cfg") << endl;
    cout << "Decrypto" << endl;
    lRsa.SetPubKey("./public.pem", OpenSSL::RSACrypto::eKeyInFile);
    cout << lRsa.DecryptoFile("./stbEnc.cfg", "./stbDec.cfg") << endl;
    MD5Crypto lMd5;
    cout << lMd5.ObtainDigest("./stbDec.cfg", OpenSSL::MD5Crypto::eHead) << endl;
    cout << lMd5.RemoveDigest("./stbDec.cfg", "./stbPlain.cfg", OpenSSL::MD5Crypto::eHead) << endl;
    cout << lMd5.CreateDigest("./stbPlain.cfg") << endl;
    cout << lMd5.AppendDigest("./stbPlain.cfg", "./stbRaw.cfg", OpenSSL::MD5Crypto::eHead) << endl;
    return EXIT_SUCCESS;
}				/* ----------  end of function main  ---------- */
#endif
}

extern "C"
const char *md5_create(char *buf,int len)
{
    OpenSSL::MD5Crypto picMD5;
    std::string digest = picMD5.CreateDigest(buf, len);
    if (digest.empty())
        LogSysOperError("MD5 calculate\n");
    return  digest.c_str();
}

