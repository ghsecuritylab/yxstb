#include "Tr069BandwidthDiagnostics.h"

#include "TR069Assertions.h"
#include "Tr069FunctionCall.h"

#include "tr069_api.h"
#include "tr069_port1.h"
#include "tr069_port.h"
#include "cryptoFunc.h"
#include "openssl/evp.h"
#include "TR069Assertions.h"
#include "NativeHandler.h"

#include "MessageTypes.h"
#include "ipanel_event.h"

#include "config.h"
#include "ind_string.h"

#include "mid/mid_timer.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct BandwidthDiagnostics {
    int DiagnosticsState;
    char *DownloadURL;
    char *Username;
    char *Password;
    char *ErrorCode;
    int AvgSpeed;
    int MaxSpeed;
    int MinSpeed;
    unsigned int StartByTr069;
};

struct BandwidthDiagnostics *gBandwidthDiagnostics = NULL;

void tr069_diagnostics_set_Speed(int avgSpeed, int maxSpeed, int minSpeed)
{
    if (avgSpeed > 0)
        gBandwidthDiagnostics->AvgSpeed = avgSpeed;

    if (maxSpeed > 0)
        gBandwidthDiagnostics->MaxSpeed = maxSpeed;

    if (minSpeed > 0)
        gBandwidthDiagnostics->MinSpeed = minSpeed;

    LogTr069Debug("avgspeed=%d,maxspeed=%d,minspeed=%d\n", avgSpeed, maxSpeed, minSpeed);
    return;
}

void tr069_diagnostics_set_StateAndErrorCode(int state, char *errorcode)
{
    LogTr069Debug("state = %d, errorcode = %s\n", state, errorcode);

    gBandwidthDiagnostics->DiagnosticsState = state;
    if(errorcode)
        ind_strdup(&gBandwidthDiagnostics->ErrorCode, errorcode);

    if (Complete == state)
        ind_strdup(&gBandwidthDiagnostics->ErrorCode, "");

    if (gBandwidthDiagnostics->StartByTr069 && (Complete == state || ErrorState == state)) {
        int eventID = tr069_api_setValue("Event.Regist", "8 DIAGNOSTICS COMPLETE", 0);
        LogTr069Debug("TR069_EVENT_DIAGNOSTICS_COMPLETE\n");
        tr069_api_setValue("Event.RegistParam", "Device.X_00E0FC.BandwidthDiagnostics.DiagnosticsState", eventID);
        tr069_api_setValue("Event.RegistParam", "Device.X_00E0FC.BandwidthDiagnostics.ErrorCode", eventID);
        tr069_api_setValue("Event.RegistParam", "Device.X_00E0FC.BandwidthDiagnostics.AvgSpeed", eventID);
        tr069_api_setValue("Event.RegistParam", "Device.X_00E0FC.BandwidthDiagnostics.MaxSpeed", eventID);
        tr069_api_setValue("Event.RegistParam", "Device.X_00E0FC.BandwidthDiagnostics.MinSpeed", eventID);
        tr069_api_setValue("Event.Post", "", eventID);

        sendMessageToNativeHandler(MessageType_KeyDown, EIS_IRKEY_MENU, 0, 0);//back from page
        gBandwidthDiagnostics->StartByTr069 = 0;
    }
    return;
}

static void networkSpeedtestStart(int arg)
{
#ifdef NW_DIAGNOSE_OPEN
    char downloadUrl[512] = {0};

    if (!gBandwidthDiagnostics || !gBandwidthDiagnostics->DownloadURL)
        return;
    if (!strncasecmp(gBandwidthDiagnostics->DownloadURL, "ftp://", 6))
        sprintf(downloadUrl, "ftp://%s:%s@%s", gBandwidthDiagnostics->Username, gBandwidthDiagnostics->Password, gBandwidthDiagnostics->DownloadURL + 6);
    else
        strcpy(downloadUrl, gBandwidthDiagnostics->DownloadURL);
    LogTr069Debug("downloadurl = %s\n", downloadUrl);

    sendMessageToNativeHandler(MessageType_Tr069, TR069_NET_SPEED_TEST, 0, 0);
#endif
}

void tr069_port_bandwidthDiagnostics_init(void)
{
    if (gBandwidthDiagnostics)
        return;
    gBandwidthDiagnostics = (struct BandwidthDiagnostics*)calloc(sizeof(struct BandwidthDiagnostics), 1);
}

void tr069_diagnostics_succeed(char *arg)
{
    if (!gBandwidthDiagnostics)
        return;
    gBandwidthDiagnostics->DiagnosticsState = None;
    ind_strdup(&gBandwidthDiagnostics->ErrorCode, "");
    sendMessageToNativeHandler(MessageType_KeyDown, EIS_IRKEY_MENU, 0, 0);//back from page
    return;
}

//以下是参数表的实现

static int getTr069PortDiagnosticsState(char* str, unsigned int val)
{
    str[0] = '\0';
    if (!gBandwidthDiagnostics)
        return -1;
    snprintf(str, val, "%d", gBandwidthDiagnostics->DiagnosticsState);
    return 0;
}

static int setTr069PortDiagnosticsState(char* str, unsigned int val)
{
    str[0] = '\0';
    if (!gBandwidthDiagnostics)
        return -1;
    int state = atoi(str);
    if (state < 1 || state > 4)
        return -1;

    if (Requested == state) {
        gBandwidthDiagnostics->DiagnosticsState = state;
        gBandwidthDiagnostics->StartByTr069 = 1;
        mid_timer_create(2, 1, networkSpeedtestStart, 0);//set DiagnosticsState befor set DownloadURL or set password
    }
    return 0;
}

static int getTr069PortDownloadURL(char* str, unsigned int val)
{
    str[0] = '\0';
    if (!gBandwidthDiagnostics)
        return -1;
    snprintf(str, val, "%s", gBandwidthDiagnostics->DownloadURL);
    return 0;
}

static int setTr069PortDownloadURL(char* str, unsigned int val)
{
    str[0] = '\0';
    if (!gBandwidthDiagnostics)
        return -1;
    ind_strdup(&gBandwidthDiagnostics->DownloadURL, str);
    return 0;
}

static int getTr069PortUsername(char* str, unsigned int val)
{
    str[0] = '\0';
    if (!gBandwidthDiagnostics)
        return -1;
    snprintf(str, val, "%s", gBandwidthDiagnostics->Username);
    return 0;
}

static int setTr069PortUsername(char* str, unsigned int val)
{
    str[0] = '\0';
    if (!gBandwidthDiagnostics)
        return -1;
    ind_strdup(&gBandwidthDiagnostics->Username, str);
    return 0;
}

static int getTr069PortBandwidthDiagnosticsPassword(char* str, unsigned int val)
{
    str[0] = '\0';
    if (!gBandwidthDiagnostics)
        return -1;
    #if  _HW_BASE_VER_ >= 58
        char output[256] = {'\0'};
        unsigned char key[256] = {0};
        char temp[512] = {0};
        int ret;

        app_TMS_aes_keys_get(key);
        ret = aesEcbEncrypt(gBandwidthDiagnostics->Password, strlen(gBandwidthDiagnostics->Password), (char*)key, temp, sizeof(temp));
        EVP_EncodeBlock((unsigned char*)output, (unsigned char*)temp, ret);//base64
        snprintf(str, val, "%s", output);
    #else
        snprintf(str, val, "%s", gBandwidthDiagnostics->Password);
    #endif
    return 0;
}

static int setTr069PortBandwidthDiagnosticsPassword(char* str, unsigned int val)
{
    str[0] = '\0';
    if (!gBandwidthDiagnostics)
        return -1;
#if _HW_BASE_VER_ >=58
        int len;
        char output[128] = {'\0'};

        len = strlen(str);
        if(len >= 16 || 0 == len % 4) {
            unsigned char key[256] = {0};
            char temp[256] = {0};
            app_TMS_aes_keys_get(key);
            EVP_DecodeBlock((unsigned char*)temp, (unsigned char*)str, strlen(str));
            aesEcbDecrypt(temp, strlen(temp), (char*)key, output, sizeof(output));
            output[127] = '\0';
            str = output;
        }
#endif
        ind_strdup(&gBandwidthDiagnostics->Password, str);
    return 0;
}

static int getTr069PortErrorCode(char* str, unsigned int val)
{
    str[0] = '\0';
    if (!gBandwidthDiagnostics)
        return -1;
    snprintf(str, val, "%s", gBandwidthDiagnostics->ErrorCode);
    return 0;
}

static int getTr069PortAvgSpeed(char* str, unsigned int val)
{
    str[0] = '\0';
    if (!gBandwidthDiagnostics)
        return -1;
    snprintf(str, val, "%d", gBandwidthDiagnostics->AvgSpeed);
    return 0;
}

static int getTr069PortMaxSpeed(char* str, unsigned int val)
{
    str[0] = '\0';
    if (!gBandwidthDiagnostics)
        return -1;
    snprintf(str, val, "%d", gBandwidthDiagnostics->MaxSpeed);
    return 0;
}

static int getTr069PortMinSpeed(char* str, unsigned int val)
{
    str[0] = '\0';
    if (!gBandwidthDiagnostics)
        return -1;
    snprintf(str, val, "%d", gBandwidthDiagnostics->MinSpeed);
    return 0;
}


Tr069BandwidthDiagnostics::Tr069BandwidthDiagnostics()
	: Tr069GroupCall("BandwidthDiagnostics")
{

    Tr069Call* DiagnosticsState = new Tr069FunctionCall("DiagnosticsState", getTr069PortDiagnosticsState, setTr069PortDiagnosticsState);
    Tr069Call* DownloadURL      = new Tr069FunctionCall("DownloadURL", getTr069PortDownloadURL, setTr069PortDownloadURL);
    Tr069Call* Username         = new Tr069FunctionCall("Username", getTr069PortUsername, setTr069PortUsername);
    Tr069Call* Password         = new Tr069FunctionCall("PasswordL", getTr069PortBandwidthDiagnosticsPassword, setTr069PortBandwidthDiagnosticsPassword);
    Tr069Call* ErrorCode        = new Tr069FunctionCall("ErrorCode", getTr069PortErrorCode, NULL);
    Tr069Call* AvgSpeed         = new Tr069FunctionCall("AvgSpeed", getTr069PortAvgSpeed, NULL);
    Tr069Call* MaxSpeed         = new Tr069FunctionCall("MaxSpeed", getTr069PortMaxSpeed, NULL);
    Tr069Call* MinSpeed         = new Tr069FunctionCall("MinSpeed", getTr069PortMinSpeed, NULL);

    regist(DiagnosticsState->name(), DiagnosticsState);
    regist(DownloadURL->name(), DownloadURL);
    regist(Username->name(), Username);
    regist(Password->name(), Password);
    regist(ErrorCode->name(), ErrorCode);
    regist(AvgSpeed->name(), AvgSpeed);
    regist(MaxSpeed->name(), MaxSpeed);
    regist(MinSpeed->name(), MinSpeed);

}

Tr069BandwidthDiagnostics::~Tr069BandwidthDiagnostics()
{
}