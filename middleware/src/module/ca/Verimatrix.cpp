
#include "Verimatrix.h"

#include "Assertions.h"
#include "config/pathConfig.h"

#include "json/json_object.h"
#include "BrowserEventQueue.h"

#define VMCA_CFG_FILE DEFAULT_MODULE_VMCA_DATAPATH"/VERIMATRIX.INI"
extern int yos_systemcall_runSystemCMD(char* buf, int* ret);

#ifdef __cplusplus
extern "C" {
#endif

#ifdef INCLUDE_VMCA
static void check_VERIMATRIX_INI(void)
{
    FILE *fp = NULL;
    char *pStr = NULL;
    char *pTemp = NULL;

    fp = fopen(VMCA_CFG_FILE, "r");
    if (fp == NULL) {
        LogUserOperError("Open VMCA_CFG_FILE fail!");
        yos_systemcall_runSystemCMD("rm -rf "DEFAULT_MODULE_VMCA_DATAPATH, NULL); // remove /root/module/vmca/
        yos_systemcall_runSystemCMD("cp /home/hybroad/share/vmdrm/ "DEFAULT_MODULE_DATAPATH" -rf", NULL); // copy /home/hybroad/share/vmdrm/ to /root/module/
        return ;
    }

    pTemp = (char *)malloc(256 + 1);
    fread(pTemp, 240, 1, fp);

    pStr = strstr(pTemp, "YuXing");
    if (pStr == NULL) {
        LogUserOperError("VERIMATRIX.INI Version error,please remove it and copy it from share/!");
        yos_systemcall_runSystemCMD("rm -rf "DEFAULT_MODULE_VMCA_DATAPATH, NULL); // remove /root/module/vmca/
        yos_systemcall_runSystemCMD("cp /home/hybroad/share/vmdrm/ "DEFAULT_MODULE_DATAPATH" -rf", NULL); // copy /home/hybroad/share/vmdrm/ to /root/module/
    } else {
        LogUserOperDebug("VERIMATRIX.INI Version !");
    }

    fclose(fp);

    if (pTemp != NULL)
        free(pTemp);
}

static int vmCA_ErrCallback(int wAcsId, int wChannel, int wError)
{
    json_object *eventInfo = json_object_new_object();
    json_object_object_add(eventInfo, "type", json_object_new_string("EVENT_STB_ERROR"));
    char errBuf[64] = {0};
    snprintf(errBuf, 64, "%d", wError);
    json_object_object_add(eventInfo, "error_code", json_object_new_string(errBuf));
    json_object_object_add(eventInfo, "instance_id", json_object_new_int(wAcsId));
    char* m_event = (char *)json_object_to_json_string(eventInfo);

    browserEventSend(m_event, NULL);

    json_object_put(eventInfo);
    return 0;
}

void ca_vm_api_init()
{
    check_VERIMATRIX_INI();
#if defined(hi3716m)
    static int vmca_ret0 = 0;		//如果CA初始化成功，则以后不再调用初始始化语句
    if (!vmca_ret0) {
        vmca_ret0 = vmCAS_app_api_init( 3 );
       // vmCAS_api_initErr_set_callback(vmCA_ErrCallback);
    }
#else
    static int vmca_ret1 = 0;
    if (!vmca_ret1) {
        vmca_ret1 = vmip_api_init( 3 );
        vmip_api_initErr_set_callback(vmCA_ErrCallback);
    }
#endif
}

void ca_vm_get_configfile(VMCAS_CONFIG_TYPE enType, char* pConfigValue)
{
#if defined(hi3716m)
    vmCAS_app_api_get_configfile(enType, pConfigValue);
#else
    vmip_api_get_configfile(enType, pConfigValue);
#endif
}

void ca_vm_set_configfile(VMCAS_CONFIG_TYPE enType, const char* pConfigValue)
{
#if defined(hi3716m)
    vmCAS_app_api_set_configfile(enType, pConfigValue);
#else
    vmip_api_set_configfile(enType, pConfigValue);
#endif
}

int ca_vm_decrypt_stream(int dwAcsId, char* pTsBuf, int wTsBufSize, int wEcmPid, unsigned int wPhyAddr)
{
#if defined(hi3716m)
    return vmCAS_app_api_decrypt_stream(dwAcsId, pTsBuf, wTsBufSize, wEcmPid, wPhyAddr);
#else
    return 0;
#endif
}

void ca_vm_decrypt_mosic(int dwAcsId, char* pStream, int wStreamLength)
{
#if defined(hi3716m)
    vmCAS_app_api_MosicDecryptSoftDecryption(dwAcsId, pStream, wStreamLength);
#endif
}

void ca_vm_check_KeyIsReady(int dwAcsId, int* pwkeyIsReady)
{
#if defined(hi3716m)
    vmCAS_app_api_check_KeyIsReady(dwAcsId, pwkeyIsReady);
#endif
}

void ca_vm_reset_masicstream(int dwAcsId)
{
#if defined(hi3716m)
    vmCAS_app_api_reset_masicstream(dwAcsId);
#endif
}

void ca_vm_reset_stream(int dwAcsId)
{
#if defined(hi3716m)
    vmCAS_app_api_reset_stream(dwAcsId);
#endif
}

#endif

void ca_vm_get_type(char* buf)
{
#ifdef INCLUDE_VMCA
    strcpy(buf, "Verimatrix");
#else
    strcpy(buf, "CANONE");
#endif
}

#ifdef __cplusplus
} // extern "C"
#endif

