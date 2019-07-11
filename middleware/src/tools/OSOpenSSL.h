#ifndef _OSOPENSSL_H_
#define _OSOPENSSL_H_

#ifdef __cplusplus
#include "ToolsAssertions.h"
#include "openssl/rsa.h"
#include "openssl/pem.h"
#include "openssl/md5.h"

#include <string>

namespace OpenSSL {

class RSACrypto {
  public:
      /* ====================  LIFECYCLE     ======================================= */
      RSACrypto();
      RSACrypto(const int& nRsaType, const int& nPadding);
      RSACrypto(const RSACrypto& other);
      ~RSACrypto();

      /* ====================  ACCESSORS     ======================================= */
      RSA* CreateRSA(const std::string& nKeyStr, const int& nKeyPos, const int& nKeyType);
      void SetPubKey(const std::string& nKeyStr, const int& nKeyPos);
      void SetPriKey(const std::string& nKeyStr, const int& nKeyPos);
      int EncryptoBuff(const int& nLen, const unsigned char* nFrom, unsigned char* nTo);
      int DecryptoBuff(const int& nLen, const unsigned char* nFrom, unsigned char* nTo);
      int EncryptoFile(const std::string& nRawDataPath, const std::string& nEncryptPath);
      int DecryptoFile(const std::string& nEncryptPath, const std::string& nDecryptPath);

      /* ====================  MUTATORS      ======================================= */

      int GetBuffSize(const RSA* nRsa);

      /* ====================  OPERATORS     ======================================= */
      RSACrypto& operator = (const RSACrypto& other); /* assignment operator */

      /*-----------------------------------------------------------------------------
       *  ref: openssl/rsa.h
       *  #define RSA_PKCS1_PADDING	1 //This currently is the most widely used mode
       *  #define RSA_SSLV23_PADDING	2
       *  #define RSA_NO_PADDING		3
       *  #define RSA_PKCS1_OAEP_PADDING	4
       *  #define RSA_X931_PADDING	5
       *-----------------------------------------------------------------------------*/
      enum _PADDING_ {
          eRsaNO    = RSA_NO_PADDING,
          eRsaPKCS1 = RSA_PKCS1_PADDING,
          eRsaOAEP  = RSA_PKCS1_OAEP_PADDING
      };

      enum _RSA_TYPE_ {
          ePUBEncrypt = 1,
          ePRIEncrypt = 2,
      };

      enum _KEY_POS_ {
          eKeyInFile = 1,
          eKeyInMem  = 2,
      };

      enum _KEY_TYPE_ {
          ePUBKey = 1,
          ePRIKey = 2,
      };

  private:
      /* ====================  DATA MEMBERS  ======================================= */
      RSA* m_RsaKey;
      int m_RsaFlen;
      int m_RsaType; /* encrypt using public key or private key */
      int m_Padding; /* RSA_NO_PADDING, RSA_PKCS1_PADDING */
}; /* -----  end of class RSACrypto  ----- */

inline void
RSACrypto::SetPubKey(const std::string& nKeyStr, const int& nKeyPos)
{
    if (!CreateRSA(nKeyStr, nKeyPos, ePUBKey))
        LogToolsError("OpenSSL: CreateRSA\n");
}

inline void
RSACrypto::SetPriKey(const std::string& nKeyStr, const int& nKeyPos)
{
    if (!CreateRSA(nKeyStr, nKeyPos, ePRIKey))
        LogToolsError("OpenSSL: CreateRSA\n");
}

#define MD5_STRING_LENGTH (2*MD5_DIGEST_LENGTH)

class MD5Crypto {
  public:
      /* ====================  LIFECYCLE     ======================================= */
      MD5Crypto();
      MD5Crypto(long nS1, long nS2, long nS3, long nS4);
      MD5Crypto(const MD5Crypto& sother);
      ~MD5Crypto();

      /* ====================  ACCESSORS     ======================================= */
      void SetContext(long nS1, long nS2, long nS3, long nS4);
      std::string CreateDigest(const char* buff, int len);
      std::string CreateDigest(const std::string& nFile);
      std::string ObtainDigest(const std::string& nFile, const int& nFlag);
      int RemoveDigest(const std::string& nMd5File, const std::string& nRawFile, const int& nFlag);
      int AppendDigest(const std::string& nRawFile, const std::string& nMd5File, const int& nFlag);

      /* ====================  OPERATORS     ======================================= */
      MD5Crypto& operator = (const MD5Crypto& other); /* assignment operator */

      enum _DigestPos_ {
          eHead = 1,
          eTail = 2,
      };

  private:
      /* ====================  DATA MEMBERS  ======================================= */
      MD5_CTX m_Context;
      unsigned long m_ContextA;
      unsigned long m_ContextB;
      unsigned long m_ContextC;
      unsigned long m_ContextD;

}; /* -----  end of class MD5Crypto  ----- */

}
#endif
#endif
