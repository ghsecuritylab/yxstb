
#ifndef Verimatrix_h
#define Verimatrix_h

#include "YX_codec_porting.h"

#if defined(hi3716m)
#include "vmCAS_global3716m.h"
#define CA_CONFIG_TYPE VMCAS_CONFIG_TYPE
#else
#include "vmip_global7405.h"
#define CA_CONFIG_TYPE VMIP_CONFIG_TYPE
#endif

#if defined(INCLUDE_VMCA)
#define USE_VERIMATRIX_OTT

#define CA_VM_DECRYPT_STREAM(dwAcsId, pTsBuf, wTsBufSize, wEcmPid, wPhyAddr) ca_vm_decrypt_stream(dwAcsId, pTsBuf, wTsBufSize, wEcmPid, wPhyAddr)
#define CA_VM_DECRYPT_MOSIC(dwAcsId, pStream, wStreamLength)  ca_vm_decrypt_mosic(dwAcsId, pStream, wStreamLength)
#define CA_VM_CHECK_KEYISREADY(dwAcsId, pwkeyIsReady)         ca_vm_check_KeyIsReady(dwAcsId, pwkeyIsReady)
#define CA_VM_RESET_MASICSTREAM(dwAcsId)                      ca_vm_reset_masicstream(dwAcsId)
#define CA_VM_RESET_STREAM(dwAcsId)                           ca_vm_reset_stream(dwAcsId)
#define CA_VM_API_INIT()                                      ca_vm_api_init()

#else

#define CA_VM_DECRYPT_STREAM(dwAcsId, pTsBuf, wTsBufSize, wEcmPid, wPhyAddr) 0
#define CA_VM_DECRYPT_MOSIC(dwAcsId, pStream, wStreamLength)
#define CA_VM_CHECK_KEYISREADY(dwAcsId, pwkeyIsReady)
#define CA_VM_RESET_MASICSTREAM(dwAcsId)
#define CA_VM_RESET_STREAM(dwAcsId)
#define CA_VM_API_INIT()

#endif

#ifdef __cplusplus
extern "C" {
#endif

void ca_vm_get_configfile(CA_CONFIG_TYPE enType, char* pConfigValue);
void ca_vm_set_configfile(CA_CONFIG_TYPE enType, const char* pConfigValue);
void ca_vm_api_init();
int ca_vm_decrypt_stream(int dwAcsId, char* pTsBuf, int wTsBufSize, int wEcmPid, unsigned int wPhyAddr);
void ca_vm_decrypt_mosic(int dwAcsId, char* pStream, int wStreamLength);
void ca_vm_check_KeyIsReady(int dwAcsId, int* pwkeyIsReady);
void ca_vm_reset_masicstream(int dwAcsId);
void ca_vm_reset_stream(int dwAcsId);
void ca_vm_get_type(char* buf);

#ifdef __cplusplus
}
#endif

#endif // Verimatrix_h

