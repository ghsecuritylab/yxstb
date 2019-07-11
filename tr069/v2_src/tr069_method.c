/*******************************************************************************
    公司：
            Yuxing software
    纪录：
            2008-1-26 21:12:26 create by Liu Jianhua
    模块：
            tr069
    简述：
            tr069 远程调用（RPC）
 *******************************************************************************/

#include "tr069_header.h"

#define TR069_FILETYPE_LEN_64        64
#define TR069_METHOD_LEN_MAX_24        24

static char EventCodeArray[EVENTCODE_MAX][TR069_EVENTCODE_LEN_32] = 
{
    "0 BOOTSTRAP",
    "1 BOOT",
    "2 PERIODIC",
    "3 SCHEDULED",
    "4 VALUE CHANGE",
    "5 KICKED",
    "6 CONNECTION REQUEST",
    "7 TRANSFER COMPLETE",
    "8 DIAGNOSTICS COMPLETE",

    "M Reboot",
    "M ScheduleInform",
    "M Download",
    "M Upload",
    "M X_CTC_SHUT_DOWN",

    "X OUI BasicInfo",
    "X OUI DownloadComplete",
    "X OUI UploadComplete",
};



#define METHOD_MAX        14
#define METHOD_LEN        32
static const char MethodNameArray[METHOD_MAX][METHOD_LEN] = 
{
    "GetRPCMethods",

    "GetParameterNames",
    "GetParameterValues",
    "SetParameterValues",
    "GetParameterAttributes",
    "SetParameterAttributes",

    "AddObject",
    "DeleteObject",
    "Reboot",
    "Upload",
    "Download",

    "FactoryReset",
    "ScheduleInform",

    ""
};

static const char FaultStringArray[FAULT_CODE_MAX][64] = 
{
    /*9000*/ "Method not supported Server",
    /*9001*/ "Request denied",
    /*9002*/ "Internal error",
    /*9003*/ "Invalid arguments",
    /*9004*/ "Resources exceeded",
    /*9005*/ "Invalid parameter name",
    /*9006*/ "Invalid parameter type",
    /*9007*/ "Invalid parameter value",
    /*9008*/ "Attempt to set a non-writable parameter",
    /*9009*/ "Notification request rejected",
    /*9010*/ "Download failure",
    /*9011*/ "Upload failure",
    /*9012*/ "File transfer server authentication failure",
    /*9013*/ "Unsupported protocol for file transfer",
    /*9014*/ "Download failure: unable to join multicast group",
    /*9015*/ "Download failure: unable to contact file server",
    /*9016*/ "Download failure: unable to access file",
    /*9017*/ "Download failure: unable to complete download",
    /*9018*/ "Download failure: file corrupted",
    /*9019*/ "Download failure: file authentication failure",
};

static const char ValueType[TR069_VALUETYPE_MAX][16] = {
    "object",

    "int",
    "unsignedInt",
    "boolean",
    "dateTime",
    "string",
    "base64",
};

int g_tr069_paramPedant = 0;//参数设置时，是否严格检测参数类型


static uint32_t g_loadSN = 0;

//------------------------------------------------------------------------------
static void tr069_fault_args_insert(struct TR069 *tr069, const char *name, int code)
{
    struct FaultArg* fault;

    if (tr069->fault_args_num >= TR069_FAULT_NUM) {
        TR069Printf("faultnum = %d\n", tr069->fault_args_num);
        return;
    }
    fault = &(tr069->fault_args_array[tr069->fault_args_num]);
    tr069->fault_args_num ++;
    if (name)
        strcpy(fault->paramname, name);
    else
        fault->paramname[0] = 0;
    fault->faultcode = code;
    tr069->fault_code = FAULT_ARGUMENTS;
}

//------------------------------------------------------------------------------
static char *tr069_soap_value(struct TR069 *tr069)
{
    if (soap_in_element_value(tr069->soap, tr069->buf, TR069_BUF_SIZE_4096 + 1))
        TR069ErrorOut("soap_in_element_value\n");
    return tr069->buf;
Err:
    return NULL;
}

//------------------------------------------------------------------------------
static int tr069_soap_value_int(struct TR069 *tr069)
{
    if (soap_in_element_value(tr069->soap, tr069->buf, TR069_BUF_SIZE_4096 + 1))
        TR069ErrorOut("soap_in_element_value\n");
    return atoi(tr069->buf);
Err:
    return -1;
}

//------------------------------------------------------------------------------
int tr069_c2s_fault(struct TR069 *tr069, char *method)
{
    int i, code, num;
    struct FaultArg *fault;
    struct Soap *soap = tr069->soap;
    char methodfault[TR069_METHOD_LEN_MAX_24 + 4];

    if (!method || strlen(method) >= TR069_METHOD_LEN_MAX_24)
        methodfault[0] = 0;
    else
        sprintf(methodfault, "%sFault", method);
    code = tr069->fault_code;
    num = tr069->fault_args_num;

    if (code != FAULT_ARGUMENTS)
        num = 0;
    if (code < 0 || code >= FAULT_CODE_MAX || num < 0 || num > TR069_FAULT_NUM) {
        TR069Printf("ERROR! code = %d, num = %d\n", code, num);
        code = FAULT_INTERNAL_ERROR;
        num = 0;
    }
    TR069Printf("c > s: Fault\n");
    if (soap_out_envelope_begin(soap, tr069->cwmp_id_req, "Fault") || 
        soap_out_element(soap, "faultcode", "Client") || 
        soap_out_element(soap, "faultstring", "CWMP fault") || 
        soap_out_element_begin(soap, "detail", NULL) || 
        soap_out_element_begin(soap, "cwmp:Fault", NULL) || 
        soap_out_element(soap, "FaultCode", tr069_fmt_int(tr069, "%d", 9000 + code)) || 
        soap_out_element(soap, "FaultString", (char *)FaultStringArray[code]))
        TR069ErrorOut("soap_out_envelope_begin Fault\n");

    for (i = 0; i < num; i ++) {
        fault = &(tr069->fault_args_array[i]);
        code = fault->faultcode;
        if (code < 0 || code >= FAULT_CODE_MAX)
            TR069ErrorOut("code = %d\n", code);
        if (soap_out_element_begin(soap, methodfault, NULL) || 
            soap_out_element(soap, "ParameterName", fault->paramname) || 
            soap_out_element(soap, "FaultCode", tr069_fmt_int(tr069, "%d", 9000 + code)) || 
            soap_out_element(soap, "FaultString", (char *)FaultStringArray[code]) || 
            soap_out_element_end(soap))
            TR069ErrorOut("soap_out_element_begin\n");
    }

    if (soap_out_element_end(soap) || //cwmp:Fault
        soap_out_element_end(soap) || //detail
        soap_out_envelope_end(soap))
        TR069ErrorOut("soap_out_element_end\n");

    return 0;
Err:
    return -1;
}

//------------------------------------------------------------------------------
int tr069_c2s_inform(struct TR069 *tr069)
{
    int ret = -1;
    int i, num;
    char currentTime[24];
    struct Soap *soap = tr069->soap;
    struct Param *param;
    GlobalEvent *event;
    EventParam *ep;

    currentTime[0] = 0;
    tr069_param_read(tr069, "Device.Time.CurrentLocalTime", currentTime, 24);
    i = atoi(currentTime);
    tr069->buf[0] = 0;
    tr069_param_read(tr069, "Device.DeviceInfo.FirstUseDate", tr069->buf, TR069_BUF_SIZE_4096);
    num = atoi(tr069->buf);
    if (!tr069->bootstrap || ((num < 2014 || num > 2029) && (i >= 2014 && i <= 2029))) {//非法年份
        tr069_param_write(tr069, "Device.DeviceInfo.FirstUseDate", currentTime, 0);
        tr069->save_flag = 1;
    }

    tr069_paramObject_reset(tr069);

    //告警
    if (tr069->event_global) {
        event = tr069->event_global;
        while (event) {
            ep = event->paramQueue;
            while (ep) {
                param = tr069_param_hash_find(tr069, ep->name);
                if (param)
                    tr069_paramObject_inset(tr069, param, 0);
                ep = ep->next;
            }
            event = event->next;
        }

    } else {
        tr069->event_inform = EVENT_INFORM_POSTING;
        for (i = 0; i < tr069->prm_inform_num; i ++)
            tr069_paramObject_inset_index(tr069, tr069->prm_inform_array[i]);

        if (tr069->event_array[EVENTCODE_BOOTSTRAP].userFlag || tr069->event_array[EVENTCODE_BOOT].userFlag) {
            for (i = 0; i < tr069->prm_boot_num; i ++)
                tr069_paramObject_inset_index(tr069, tr069->prm_boot_array[i]);
        }

        if (!tr069->prm_object.firstChild)
            TR069Printf("regist empty!\n");

        param = tr069->prm_change;
        while (param) {
            TR069Printf("change: %s\n", param->name);
            tr069_paramObject_inset_index(tr069, param->index);
            param = param->changeNext;
        }
    }

    if (!tr069->prm_object.firstChild)
        TR069Printf("param empty!\n");

    TR069Printf("c > s: Inform\n");
    if (soap_out_envelope_begin(soap, tr069_fmt_id(tr069), "Inform") || 
        soap_out_element_begin(soap, "DeviceId", NULL) || 
        soap_out_element(soap, "Manufacturer", tr069_fmt_param(tr069, "Device.DeviceInfo.Manufacturer")) || 
        soap_out_element(soap, "OUI",          tr069_fmt_param(tr069, "Device.DeviceInfo.ManufacturerOUI")) || 
        soap_out_element(soap, "ProductClass", tr069_fmt_param(tr069, "Device.DeviceInfo.ProductClass")) || 
        soap_out_element(soap, "SerialNumber", tr069_fmt_param(tr069, "Device.DeviceInfo.SerialNumber")) || 
        soap_out_element_end(soap))
        TR069ErrorOut("soap_out_envelope_begin Inform\n");

    num = 0;
    if (tr069->event_global) {
        event = tr069->event_global;
        while (event) {
            num++;
            event = event->next;
        }
    } else {
        for (i = 0; i < EVENTCODE_MAX; i ++) {
            if (tr069->event_array[i].userFlag == 1)
                num++;
        }
    }
    if (soap_out_element_begin(soap, "Event", tr069_fmt_int(tr069, "cwmp:EventStruct[%d]", num)))
        TR069ErrorOut("soap_out_element_begin Event\n");

    if (tr069->event_global) {
        event = tr069->event_global;
        while (event) {
            TR069Printf("%s\n", event->eventCode);
            if (soap_out_element_begin(soap, "EventStruct", NULL) || 
                soap_out_element(soap, "EventCode", event->eventCode) || 
                soap_out_element(soap, "CommandKey", "") || 
                soap_out_element_end(soap))
            TR069ErrorOut("soap_out_element_begin EventStruct\n");
            event = event->next;
        }
    } else {
        for (i = 0; i < EVENTCODE_MAX; i ++) {
            struct Event* event = &(tr069->event_array[i]);

            if (!event->userFlag)
                continue;

            TR069Printf("%s\n", EventCodeArray[i]);
            if (soap_out_element_begin(soap, "EventStruct", NULL) || 
                soap_out_element(soap, "EventCode", (char *)EventCodeArray[i]) || 
                soap_out_element(soap, "CommandKey", event->CommandKey) || 
                soap_out_element_end(soap))
            TR069ErrorOut("soap_out_element_begin EventStruct\n");
        }
    }

    if (soap_out_element_end(soap) || //Event
        soap_out_element(soap, "MaxEnvelopes", "1") || 
        soap_out_element(soap, "CurrentTime", currentTime) || 
        soap_out_element(soap, "RetryCount", tr069_fmt_int(tr069, "%d", tr069->retry_count)))
        TR069ErrorOut("soap_out_element MaxEnvelopes\n");

    if (0 == tr069->prm_num)
        TR069Printf("\n");
    if (soap_out_element_begin(soap, "ParameterList", 
            tr069_fmt_int(tr069, "cwmp:ParameterValueStruct[%d]", tr069->prm_num)))
        TR069ErrorOut("soap_out_element_begin ParameterList\n");

    param = tr069->prm_object.firstChild;
    while (param) {
        if (soap_out_element_begin(soap, "ParameterValueStruct", NULL) || 
            soap_out_element(soap, "Name", param->name) || 
            soap_out_element_type(soap, "Value", (char *)ValueType[param->type], tr069_fmt_param(tr069, param->name)) || 
            soap_out_element_end(soap))
            TR069ErrorOut("soap_out_element_begin ParameterValueStruct\n");
        param = param->paramNext;
    }

    if (soap_out_element_end(soap) || 
        soap_out_envelope_end(soap))
        TR069ErrorOut("soap_out_element_end\n");

    ret = 0;
Err:
    return ret;
}

//------------------------------------------------------------------------------
static int tr069_c2s_getrpcmethods(struct TR069 *tr069)
{
    struct Soap *soap = tr069->soap;

    TR069Printf("c > s: GetRPCMethods\n");
    if (soap_out_envelope_begin(soap, tr069_fmt_id(tr069), "GetRPCMethods") || 
        soap_out_envelope_end(soap))
        TR069ErrorOut("soap_out_envelope_begin GetRPCMethods\n");

    return 0;
Err:
    return -1;
}

//------------------------------------------------------------------------------
int tr069_c2s_unsupported_response(struct TR069 *tr069)
{
    tr069->fault_code = FAULT_SUPPORTED;
    if (tr069_c2s_fault(tr069, NULL))
        TR069ErrorOut("tr069_c2s_fault\n");

    return 0;
Err:
    return -1;
}

//------------------------------------------------------------------------------
int tr069_c2s_getrpcmethods_response(struct TR069 *tr069)
{
    int i;
    struct Soap *soap = tr069->soap;

    TR069Printf("c > s: GetRPCMethodsResponse\n");
    if (soap_out_envelope_begin(soap, tr069->cwmp_id_req, "GetRPCMethodsResponse") || 
        soap_out_element_begin(soap, "MethodList", tr069_fmt_int(tr069, "xsd:string[%d]", METHOD_MAX)))
        TR069ErrorOut("soap_out_envelope_begin GetRPCMethodsResponse\n");

    for (i = 0; i < METHOD_MAX; i ++) {
        if (soap_out_element(soap, "string", (char *)MethodNameArray[i]))
            TR069ErrorOut("soap_out_element string\n");
    }

    if (soap_out_element_end(soap) || 
        soap_out_envelope_end(soap))
        TR069ErrorOut("soap_out_element_end\n");

    return 0;
Err:
    return -1;
}

//------------------------------------------------------------------------------
int tr069_c2s_getparameternames_response(struct TR069 *tr069)
{
    char writeable[2];
    struct Param *param;
    struct Soap *soap = tr069->soap;

    TR069Printf("c > s: GetParameterNamesResponse\n");
    if (soap_out_envelope_begin(soap, tr069->cwmp_id_req, "GetParameterNamesResponse") || 
        soap_out_element_begin(soap, "ParameterList", tr069_fmt_int(tr069, 
                        "cwmp:ParameterInfoStruct[%d]", tr069->prm_num)))
        TR069ErrorOut("soap_out_envelope_begin GetParameterNamesResponse\n");

    param = tr069->prm_object.firstChild;
    while (param) {
        if (param->attr & TR069_ENABLE_WRITE)
            writeable[0] = '1';
        else
            writeable[0] = '0';
        writeable[1] = 0;
        if (soap_out_element_begin(soap, "ParameterInfoStruct", NULL) || 
            soap_out_element(soap, "Name", param->name) || 
            soap_out_element(soap, "Writable", writeable) || 
            soap_out_element_end(soap))
            TR069ErrorOut("soap_out_element_begin ParameterInfoStruct\n");
        param = param->paramNext;
    }
    
    if (soap_out_element_end(soap) || 
        soap_out_envelope_end(soap))
        TR069ErrorOut("soap_out_element_end\n");

    return 0;
Err:
    return -1;
}

//------------------------------------------------------------------------------
int tr069_c2s_setparametervalues_response(struct TR069 *tr069)
{
    struct Soap *soap = tr069->soap;

    if (tr069->chg_delay != 1)
        tr069->chg_delay = 0;
    TR069Printf("c > s: SetParameterValuesResponse\n");
    if (soap_out_envelope_begin(soap, tr069->cwmp_id_req, "SetParameterValuesResponse") || 
        soap_out_element(soap, "Status", tr069_fmt_int(tr069, "%d", tr069->chg_delay)) || 
        soap_out_envelope_end(soap))
        TR069ErrorOut("soap_out_envelope_begin SetParameterValuesResponse\n");

    return 0;
Err:
    return -1;
}

//------------------------------------------------------------------------------
int tr069_c2s_getparametervalues_response(struct TR069 *tr069)
{
    int ret = -1;
    struct Param *param;
    struct Soap *soap = tr069->soap;

    TR069Printf("c > s: GetParameterValuesResponse\n");
    if (soap_out_envelope_begin(soap, tr069->cwmp_id_req, "GetParameterValuesResponse") || 
        soap_out_element_begin(soap, "ParameterList", tr069_fmt_int(tr069, 
                "cwmp:ParameterValueStruct[%d]", tr069->prm_num)))
        TR069ErrorOut("soap_out_envelope_begin GetParameterValuesResponse\n");

    param = tr069->prm_object.firstChild;
    while (param) {
        //TR069Printf("param->type = %d, param->name = %s\n", param->type, param->name);
        if (soap_out_element_begin(soap, "ParameterValueStruct", NULL) || 
            soap_out_element(soap, "Name", param->name) || 
            soap_out_element_type(soap, "Value", (char *)ValueType[param->type], tr069_fmt_param(tr069, param->name)) || 
            soap_out_element_end(soap))
            TR069ErrorOut("soap_out_element_begin ParameterValueStruct\n");
        param = param->paramNext;
    }

    if (soap_out_element_end(soap) || 
        soap_out_envelope_end(soap))
        TR069ErrorOut("soap_out_element_end\n");

    ret = 0;
Err:
    return ret;
}

//------------------------------------------------------------------------------
int tr069_c2s_setparameterattributes_response(struct TR069 *tr069)
{
    struct Soap *soap = tr069->soap;

    TR069Printf("c > s: SetParameterAttributesResponse\n");
    if (soap_out_envelope_begin(soap, tr069->cwmp_id_req, "SetParameterAttributesResponse") || 
        soap_out_envelope_end(soap))
        TR069ErrorOut("soap_out_envelope_begin SetParameterAttributesResponse\n");

    return 0;
Err:
    return -1;
}

//------------------------------------------------------------------------------
int tr069_c2s_getparameterattributes_response(struct TR069 *tr069)
{
    struct Param *param;
    struct Soap *soap = tr069->soap;

    TR069Printf("c > s: GetParameterAttributesResponse\n");
    if (soap_out_envelope_begin(soap, tr069->cwmp_id_req, "GetParameterAttributesResponse") || 
        soap_out_element_begin(soap, "ParameterList", tr069_fmt_int(tr069, 
                "cwmp:ParameterAttributeStruct[%d]", tr069->prm_num)))
        TR069ErrorOut("soap_out_envelope_begin GetParameterAttributesResponse\n");

    param = tr069->prm_object.firstChild;
    while (param) {
        if (soap_out_element_begin(soap, "ParameterAttributeStruct", NULL) || 
            soap_out_element(soap, "Name", param->name) || 
            soap_out_element(soap, "Notification", tr069_fmt_int(tr069, "%d", param->currentNotification)) || 
            soap_out_element(soap, "AccessList", "Subscriber") || 
            soap_out_element_end(soap))
            TR069ErrorOut("soap_out_element_begin ParameterAttributeStruct\n");
        param = param->paramNext;
    }

    if (soap_out_element_end(soap) || 
        soap_out_envelope_end(soap))
        TR069ErrorOut("soap_out_element_end\n");

    return 0;
Err:
    return -1;
}

//------------------------------------------------------------------------------
int tr069_c2s_addobject_response(struct TR069 *tr069)
{
    struct Soap *soap = tr069->soap;

    TR069Printf("c > s: AddObjectResponse\n");
    if (soap_out_envelope_begin(soap, tr069->cwmp_id_req, "AddObjectResponse") || 
        soap_out_element(soap, "InstanceNumber", tr069_fmt_int(tr069, "%d", tr069->inst_num)) || 
        soap_out_element(soap, "Status", "0") ||
        soap_out_envelope_end(soap))
        TR069ErrorOut("soap_out_envelope_begin AddObjectResponse\n");

    return 0;
Err:
    return -1;
}

//------------------------------------------------------------------------------
int tr069_c2s_deleteobject_response(struct TR069 *tr069)
{
    struct Soap *soap = tr069->soap;

    TR069Printf("c > s: DeleteObjectResponse\n");
    if (soap_out_envelope_begin(soap, tr069->cwmp_id_req, "DeleteObjectResponse") || 
        soap_out_element(soap, "Status", "0") ||
        soap_out_envelope_end(soap))
        TR069ErrorOut("soap_out_envelope_begin DeleteObjectResponse\n");

    return 0;
Err:
    return -1;
}

//------------------------------------------------------------------------------
int tr069_c2s_reboot_response(struct TR069 *tr069)
{
    struct Soap *soap = tr069->soap;

    TR069Printf("c > s: RebootResponse\n");
    if (soap_out_envelope_begin(soap, tr069->cwmp_id_req, "RebootResponse") || 
        soap_out_envelope_end(soap))
        TR069ErrorOut("soap_out_envelope_begin RebootResponse\n");

    return 0;
Err:
    return -1;
}

//------------------------------------------------------------------------------
int tr069_c2s_download_response(struct TR069 *tr069)
{
    struct Soap *soap = tr069->soap;

    /*
        <Status></Status>
        <StartTime></StartTime>
        <CompleteTime></CompleteTime>
    */
    TR069Printf("c > s: DownloadResponse\n");
    if (soap_out_envelope_begin(soap, tr069->cwmp_id_req, "DownloadResponse") || 
        soap_out_element(soap, "Status", "1") || 
        soap_out_element(soap, "StartTime", "") || 
        soap_out_element(soap, "CompleteTime", "") || 
        soap_out_envelope_end(soap))
        TR069ErrorOut("soap_out_envelope_begin DownloadResponse\n");

    return 0;
Err:
    return -1;
}

//------------------------------------------------------------------------------
int tr069_c2s_upload_response(struct TR069 *tr069)
{
    struct Soap *soap = tr069->soap;

    /*
        <Status></Status>
        <StartTime></StartTime>
        <CompleteTime></CompleteTime>
    */
    TR069Printf("c > s: UploadResponse\n");
    if (soap_out_envelope_begin(soap, tr069->cwmp_id_req, "UploadResponse") || 
        soap_out_element(soap, "Status", "1") || 
        soap_out_element(soap, "StartTime", "") || 
        soap_out_element(soap, "CompleteTime", "") || 
        soap_out_envelope_end(soap))
        TR069ErrorOut("soap_out_envelope_begin UploadResponse\n");

    return 0;
Err:
    return -1;
}

//------------------------------------------------------------------------------
int tr069_c2s_factoryreset_response(struct TR069 *tr069)
{
    struct Soap *soap = tr069->soap;

    TR069Printf("c > s: FactoryResetResponse\n");
    if (soap_out_envelope_begin(soap, tr069->cwmp_id_req, "FactoryResetResponse") || 
        soap_out_envelope_end(soap))
        TR069ErrorOut("soap_out_envelope_begin FactoryResetResponse\n");

    return 0;
Err:
    return -1;
}

//------------------------------------------------------------------------------
int tr069_c2s_scheduleinform_response(struct TR069 *tr069)
{
    struct Soap *soap = tr069->soap;

    TR069Printf("c > s: ScheduleInformResponse\n");
    if (soap_out_envelope_begin(soap, tr069->cwmp_id_req, "ScheduleInformResponse") || 
        soap_out_envelope_end(soap))
        TR069ErrorOut("soap_out_envelope_begin ScheduleInformResponse\n");

    return 0;
Err:
    return -1;
}

//------------------------------------------------------------------------------
int tr069_c2s_transfercomplete(struct TR069 *tr069)
{
/*
    CommandKey string
    FaultStruct FaultStruct
    StartTime dateTime
    CompleteTime dateTime
*/
    int faultcode;
    struct Soap *soap = tr069->soap;
    struct Load *load;

    load = tr069->loadQueue;
    if (!load)
        TR069ErrorOut("load is NULL\n");

    faultcode = load->faultcode;
    TR069Printf("c > s: TransferComplete faultcode = %d\n", faultcode);
    if (soap_out_envelope_begin(soap, tr069->cwmp_id_req, "TransferComplete") || 
        soap_out_element(soap, "CommandKey", load->CommandKey) || 
        soap_out_element_begin(soap, "cwmp:FaultStruct", NULL))
        TR069ErrorOut("FaultCode TransferComplete\n");
    if (faultcode) {
        if (faultcode >= 9000 && faultcode < FAULT_CODE_MAX) {
            TR069Printf("FaultCode: %d\n", faultcode);
            TR069Printf("FaultString: %s\n", (char *)FaultStringArray[faultcode - 9000]);
            if (soap_out_element(soap, "FaultCode", tr069_fmt_int(tr069, "%d",  faultcode)) || 
                soap_out_element(soap, "FaultString", (char *)FaultStringArray[faultcode - 9000]))
                TR069ErrorOut("soap_out_element FaultCode1\n");
        } else {
            char faulstring[256];
            TR069Printf("Customer FaultCode: %d\n", faultcode);
            if (tr069_port_fault2string(faultcode, faulstring, 255))
                TR069ErrorOut("tr069_port_faultstring\n");
            if (soap_out_element(soap, "FaultCode", tr069_fmt_int(tr069, "%d", faultcode)) || 
                soap_out_element(soap, "FaultString", faulstring))
                TR069ErrorOut("soap_out_element FaultCode\n");
        }
    } else {
        TR069Printf("FaultCode: 0\n");
        if (soap_out_element(soap, "FaultCode", "0") || 
            soap_out_element(soap, "FaultString", ""))
            TR069ErrorOut("soap_out_element FaultCode2\n");
    }
    if (soap_out_element_end(soap) || 
        soap_out_element(soap, "StartTime", tr069_fmt_time(tr069, load->starttime)) || 
        soap_out_element(soap, "CompleteTime", tr069_fmt_time(tr069, load->completetime)) || 
        soap_out_envelope_end(soap))
        TR069ErrorOut("soap_out_element StartTime\n");

    return 0;
Err:
    return -1;
}

//------------------------------------------------------------------------------
int tr069_c2s_default(struct TR069 *tr069)
{
    struct Load *load;

    load = tr069->loadQueue;
    if (load) {
        if (TRANSFER_STATE_COMPLETE == load->transferstate)
            return tr069_c2s_transfercomplete(tr069);
    }
    if (tr069->flag_getrpc)
        return tr069_c2s_getrpcmethods(tr069);
    TR069Printf("c > s: Empty\n");
    return 0;
}

//------------------------------------------------------------------------------
int tr069_s2c_inform_response(struct TR069 *tr069)
{
    int i;
    struct Param *param;
    struct Soap *soap = tr069->soap;

    do {
        if (soap_in_element_either(soap))
            TR069ErrorOut("err Header\n");
    } while (soap->level != SOAP_METHOD_LEVEL);

    if (tr069->prm_change) {
        TR069Printf("#############################\n");
        while (tr069->prm_change) {
            param = tr069->prm_change;
            tr069->prm_change = param->changeNext;

            param->changeNext = NULL;
            param->isChange = 0;

            tr069_param_read_cksum(tr069, param, &param->cksum);
            param->cksum_flag = 1;
        }
        tr069->save_flag = 1;
    }

    if (!tr069->bootstrap) {
        tr069->bootstrap = 1;
        tr069->save_flag = 1;
    }
    if (tr069->reboot.rebootstate != REBOOT_STATE_NULL) {
        tr069->reboot.rebootstate = REBOOT_STATE_NULL;
        tr069->save_flag = 1;
    }

    if (tr069->event_global) {
        GlobalEvent *event;
        EventParam *param;

        TR069Printf("free global!\n");

        while (tr069->event_global) {
            event = tr069->event_global;
            tr069->event_global = event->next;

            tr069_global_eventFinished(event->eventID, event->eventNum);
            while (event->paramQueue) {
                param = event->paramQueue;
                event->paramQueue = param->next;
                IND_FREE(param->name);
                IND_FREE(param);
            }
            if (event->eventCode)
                IND_FREE(event->eventCode);
            IND_FREE(event);
        }
    } else {
        for (i = 0; i < EVENTCODE_MAX; i ++)
            tr069->event_array[i].userFlag = 0;
        tr069->event_inform = EVENT_INFORM_NONE;
    }

    {
        unsigned int urlModifyFlag = tr069_tr106_getUnsigned("Device.ManagementServer.URLModifyFlag");
        TR069Printf("URLModifyFlag = %d\n", urlModifyFlag);
        if (15 == urlModifyFlag) {
            char buf[4];
            strcpy(buf, "14");
            tr069_tr106_SetParamValue("Device.ManagementServer.URLModifyFlag", "", 14);
            tr069_port_setValue("Device.ManagementServer.URLModifyFlag", buf, 2);
        }
    }

    return 0;
Err:
    return -1;
}

//------------------------------------------------------------------------------
int tr069_s2c_getrpcmethods(struct TR069 *tr069)
{
    return 0;
}

//------------------------------------------------------------------------------
int tr069_s2c_getrpcmethods_response(struct TR069 *tr069)
{
    struct Soap *soap = tr069->soap;
    /*
    <MethodList>
      <string></string>
    </MethodList>
    */
    if (soap_in_element_begin(soap, "MethodList"))
        TR069ErrorOut("soap_in_element_begin MethodList\n");

    tr069->flag_getrpc = 0;
    while (soap->level > SOAP_METHOD_LEVEL) {
        if (soap_in_element_either(soap))
            TR069ErrorOut("soap_in_element_either\n");
    }
    return 0;
Err:
    return -1;
}

//------------------------------------------------------------------------------
int tr069_s2c_getparameternames(struct TR069 *tr069)
{
    char *value;
    char path[TR069_NAME_FULL_SIZE_128 + 4];
    int level;
    struct Param *param;
    struct Soap *soap = tr069->soap;

    tr069_paramObject_reset(tr069);
    /*
        <ParameterPath></ParameterPath>
        <NextLevel></NextLevel>
    */
    if (soap_in_element(soap, "ParameterPath") || 
        soap_in_element_value(soap, path, TR069_NAME_FULL_SIZE_128 + 1) || 
        soap_in_element(soap, "NextLevel") || 
        !(value = tr069_soap_value(tr069)))
            TR069ErrorOut("soap_in_element ParameterPath\n");

    if (path[0] == 0)
        param = tr069->table_array[0];
    else
        param = tr069_param_hash_find(tr069, path);
    if (!param) {
        TR069Printf("%s param not fond!\n", path);
        tr069->fault_code = FAULT_PARAMETER_NAME;
        return 0;
    }
    if (stricmp(value, "true") == 0)
        level = 1;
    else if (stricmp(value, "false") == 0)
        level = 0;
    else
        level = atoi(value);
    if (level == 1) {
        if (param->type != TR069_OBJECT) {
            TR069Printf("%s nextlevel disable!\n", path);
                tr069->fault_code = FAULT_ARGUMENTS;
                return 0;
            }
        if (tr069_paramObject_inset_leaf(tr069, param))
            TR069ErrorOut("tr069_paramObject_inset_level\n");
    } else {
        if (tr069_paramObject_inset(tr069, param, 1))
            TR069ErrorOut("tr069_paramObject_inset\n");
    }

    return 0;
Err:
    return -1;
}

//------------------------------------------------------------------------------
int tr069_s2c_getparametervalues(struct TR069 *tr069)
{
    struct Taget *tag;
    char *name;
    struct Soap *soap = tr069->soap;
    struct Param *param;

    tr069_paramObject_reset(tr069);
    if (soap_in_element_begin(soap, "ParameterNames"))
        TR069ErrorOut("soap_in_element_begin ParameterNames\n");

    /*
     <string></string>
    */
    while (soap->level > SOAP_METHOD_LEVEL) {

        if (soap_in_element_either(soap))
            TR069ErrorOut("soap_in_element_either\n");
        if (soap->level == SOAP_METHOD_LEVEL)
            break; 
        tag = soap_tag(soap);
        if (stricmp(tag->name, "string") || 
            soap_in_element_end(soap) || 
            !(name = tr069_soap_value(tr069)))
            TR069ErrorOut("soap_in_element_end string\n");

        param = tr069_param_hash_find(tr069, name);
        if (!param) {
            TR069Printf("%s param not fond!\n", name);
            tr069->fault_code = FAULT_PARAMETER_NAME;
            continue;
        }
        if (tr069->fault_code != -1)
            continue;

        if (tr069_paramObject_inset(tr069, param, 0))
            TR069ErrorOut("tr069_paramObject_inset\n");
    }

    return 0;
Err:
    return -1;
}

//------------------------------------------------------------------------------
int tr069_s2c_setparametervalues(struct TR069 *tr069)
{
    char paramkey[TR069_COMMANDKEY_LEN_32 + 4];
    char name[TR069_NAME_FULL_SIZE_128 + 4], type[SOAP_TYPE_LEN_16 + 4], *value;
    struct Param *param;
    struct Soap *soap = tr069->soap;

    tr069_paramObject_reset(tr069);
    tr069->chg_delay = 0;
    tr069->addrtype_config[0] = 0;

    if (soap_in_element_begin(soap, "ParameterList"))
        TR069ErrorOut("soap_in_element_begin ParameterList\n");

    /*
    <ParameterValueStruct>
        <Name></Name>
        <Value></Value>
    </ParameterValueStruct>
    <ParameterKey></ParameterKey>
    */
    while (soap->level > SOAP_METHOD_LEVEL) {
        if (soap_in_element_either(soap))
            TR069ErrorOut("soap_in_element_either\n");
        if (soap->level == SOAP_METHOD_LEVEL)
            break;

        if (soap_in_element(soap, "Name") || 
            soap_in_element_value(soap, name, TR069_NAME_FULL_SIZE_128 + 1) || 
            soap_in_element(soap, "Value"))
            TR069ErrorOut("soap_in_element Name\n");
        value = tr069_soap_value(tr069);
        strcpy(type, soap->tagtype);
        if (!value || 
            soap_in_element_end(soap)) //end ParameterValueStruct
            TR069ErrorOut("ParameterKey\n");

        param = tr069_param_hash_find(tr069, name);
        if (!param) {
            TR069Printf("%s param not fond!\n", name);
            tr069_fault_args_insert(tr069, name, FAULT_PARAMETER_NAME);
            continue;
        }
        if ((param->attr & TR069_ENABLE_WRITE) == 0) {
            TR069Printf("%s not writable!\n", name);
            tr069_fault_args_insert(tr069, name, FAULT_NON_WRITABLE);
            continue;
        }
        if (g_tr069_paramPedant && strcmp((char *)ValueType[param->type], type)) {
            TR069Printf("%s type not match!\n", name);
            tr069_fault_args_insert(tr069, name, FAULT_PARAMETER_TYPE);
            continue;
        }
        //'<' 是非法值
        if (value[0] == '<' || tr069_param_write_try(tr069, param->index, value)) {
            TR069Printf("%s value invalid!\n", name);
            tr069_fault_args_insert(tr069, name, FAULT_PARAMETER_VALUE);
            continue;
        }
        if (tr069->fault_code != -1)
            continue;
        if ((param->attr & TR069_ENABLE_APPLY) == 0)
            tr069->chg_delay = 1;

        if (tr069_paramObject_inset(tr069, param, 0)) {
            TR069Printf("Resources exceeded\n");
            continue;
        }
    }
    if (soap_in_element(soap, "ParameterKey") || 
        soap_in_element_value(soap, paramkey, TR069_COMMANDKEY_LEN_32 + 1))
        TR069ErrorOut("soap_in_element ParameterKey\n");

    tr069_paramObject_apply_values(tr069, 0);
    param = tr069_param_hash_find(tr069, "Device.ManagementServer.ParameterKey");
    if (param->prm_setval(param->name, paramkey, 0))
        TR069Error("setval\n");

    return 0;
Err:
    return -1;
}

static void setParameterAttributes(struct TR069 *tr069, struct Param *param, int notification)
{
    if (!param)
        return;

    param->changeNotification = (u_char)notification;
    tr069_paramObject_inset(tr069, param, 0);

    if (TR069_OBJECT == param->type) {
        struct Param *child = param->prm_objValue.firstChild;
        while (child) {
            setParameterAttributes(tr069, child, notification);
            child = child->treeNext;
        }
    }
}

//------------------------------------------------------------------------------
int tr069_s2c_setparameterattributes(struct TR069 *tr069)
{
    int level, notificationChange, notification;
    char name[TR069_NAME_FULL_SIZE_128 + 4];
    char *value;
    struct Param *param;
    struct Soap *soap = tr069->soap;

    tr069_paramObject_reset(tr069);
    if (soap_in_element_begin(soap, "ParameterList"))
        TR069ErrorOut("soap_in_element_begin ParameterList\n");

    /*
    <ParameterList>
      <SetParameterAttributesStruct>
        <Name></Name>
        <NotificationChange></NotificationChange>
        <Notification></Notification>
        <AccessListChange></AccessListChange>
        <AccessList>
          <string></string>
        </AccessList>
      </SetParameterAttributesStruct>
      ...
    </ParameterList>
    */
    while (soap->level > SOAP_METHOD_LEVEL) {
        if (soap_in_element_either(soap))
            TR069ErrorOut("soap_in_element_either\n");
        if (soap->level == SOAP_METHOD_LEVEL)
            break;

        if (soap_in_element(soap, "Name") || 
            soap_in_element_value(soap, name, TR069_NAME_FULL_SIZE_128 + 1) || 
            soap_in_element(soap, "NotificationChange") || 
            !(value = tr069_soap_value(tr069)))
            TR069ErrorOut("soap_in_element Name\n");

        if (stricmp(value, "true") == 0)
            notificationChange = 1;
        else if (stricmp(value, "false") == 0)
            notificationChange = 0;
        else
            notificationChange = atoi(value);

        if (soap_in_element(soap, "Notification") || 
            (notification = tr069_soap_value_int(tr069)) == -1 || 
            soap_in_element(soap, "AccessListChange"))
            TR069ErrorOut("soap_in_element Notification\n");

        level = soap->level;
        if (soap_in_element_begin(soap, "AccessList"))
            TR069ErrorOut("soap_in_element_begin AccessList\n");
        while (soap->level > level) {
            if (soap_in_element_either(soap))
                TR069ErrorOut("soap_in_element_either AccessList\n");
        }

        if (soap_in_element_end(soap)) //SetParameterAttributesStruct
            TR069ErrorOut("soap_in_element_end\n");

        param = tr069_param_hash_find(tr069, name);
        if (!param) {
            TR069Printf("%s param not fond!\n", name);
            tr069->fault_code = FAULT_PARAMETER_NAME;
            return 0;
        }
        if (notificationChange != 1 || (notification < 0 || notification > 2) || (param->attr & TR069_ENABLE_ATTR) == 0) {
            TR069Printf("%s notification rejected!\n", name);
            TR069Printf("notificationChange = %d, notification = %d, TR069_ENABLE_ATTR = %d!\n", notificationChange, notification, (param->attr & TR069_ENABLE_ATTR));
            tr069->fault_code = FAULT_NOTIFICATION_REJECTED;
            continue;
        }
        if (tr069->fault_code != -1)
            continue;

        setParameterAttributes(tr069, param, notification);
    }
    if (tr069->fault_code == -1) {
        param = tr069->prm_object.firstChild;
        while (param) {
            if (param->currentNotification != param->changeNotification) {
                param->currentNotification = param->changeNotification;
                tr069->save_flag = 1;
            }
            param = param->paramNext;
        }
    }
    tr069_paramObject_reset(tr069);

    return 0;
Err:
    return -1;
}

//------------------------------------------------------------------------------
int tr069_s2c_getparameterattributes(struct TR069 *tr069)
{
    struct Taget *tag;
    char *name;
    struct Param *param;
    struct Soap *soap = tr069->soap;

    tr069_paramObject_reset(tr069);
    if (soap_in_element_begin(soap, "ParameterNames"))
        TR069ErrorOut("soap_in_element_begin\n");

    /*
       <string></string>
    */
    while (soap->level > SOAP_METHOD_LEVEL) {
        if (soap_in_element_either(soap))
            TR069ErrorOut("soap_in_element_either\n");
        if (soap->level == SOAP_METHOD_LEVEL)
            break;
        tag = soap_tag(soap);
        if (stricmp(tag->name, "string") || 
            soap_in_element_end(soap) || 
            !(name = tr069_soap_value(tr069)))
            TR069ErrorOut("tr069_soap_value\n");

        param = tr069_param_hash_find(tr069, name);
        if (!param) {
            TR069Printf("%s param not fond!\n", name);
            tr069->fault_code = FAULT_PARAMETER_NAME;
            continue;
        }
        if (tr069->fault_code != -1)
            continue;
        if (tr069_paramObject_inset(tr069, param, 0))
            TR069ErrorOut("tr069_paramObject_inset\n");
    }

    return 0;
Err:
    return -1;
}

//------------------------------------------------------------------------------
int tr069_s2c_addobject(struct TR069 *tr069)
{
    int instnum;
    char paramkey[TR069_COMMANDKEY_LEN_32 + 4];
    char name[TR069_NAME_FULL_SIZE_128 + 4];
    struct Param *param;
    struct Soap *soap = tr069->soap;

/*
    <ObjectName></ObjectName>
    <ParameterKey></ParameterKey>
*/
    if (soap_in_element(soap, "ObjectName") || 
        soap_in_element_value(soap, name, TR069_NAME_FULL_SIZE_128 + 1) || 
        soap_in_element(soap, "ParameterKey") || 
        soap_in_element_value(soap, paramkey, TR069_COMMANDKEY_LEN_32 + 1))
        TR069ErrorOut("soap_in_element ObjectName\n");

    param = tr069_param_hash_find(tr069, name);
    if (!param) {
        TR069Printf("%s param not fond!\n", name);
        tr069->fault_code = FAULT_PARAMETER_NAME;
        return 0;
    }

    if (param->type != TR069_OBJECT || (param->attr & TR069_ENABLE_ADD) == 0) {
        TR069Printf("%s objadd disable!\n", name);
        tr069->fault_code = FAULT_ARGUMENTS;
        return 0;
    }

    instnum = tr069_object_add(tr069, name, 0);
    if (instnum < 0) {
        TR069Printf("%s objadd failed!\n", name);
        tr069->fault_code = FAULT_RESOURCES_EXCEEDED;
        return 0;
    }
    tr069->inst_num = instnum;
    param = tr069_param_hash_find(tr069, "Device.ManagementServer.ParameterKey");
    if (param->prm_setval(param->name, paramkey, 0))
        TR069Error("setval\n");

    return 0;
Err:
    return -1;
}

//------------------------------------------------------------------------------
int tr069_s2c_deleteobject(struct TR069 *tr069)
{
    char paramkey[TR069_COMMANDKEY_LEN_32 + 4];
    char name[TR069_NAME_FULL_SIZE_128 + 4];
    struct Param *param;
    struct Soap *soap = tr069->soap;
/*
    <ObjectName></ObjectName>
    <ParameterKey></ParameterKey>
*/
    if (soap_in_element(soap, "ObjectName") || 
        soap_in_element_value(soap, name, TR069_NAME_FULL_SIZE_128 + 1) || 
        soap_in_element(soap, "ParameterKey") || 
        soap_in_element_value(soap, paramkey, TR069_COMMANDKEY_LEN_32 + 1))
        TR069ErrorOut("soap_in_element ObjectName\n");


    param = tr069_param_hash_find(tr069, name);
    if (!param) {
        TR069Printf("%s param not fond!\n", name);
        tr069->fault_code = FAULT_PARAMETER_NAME;
        return 0;
    }

    if (param->type != TR069_OBJECT || (param->attr & TR069_ENABLE_DELETE) == 0) {
        TR069Printf("%s param invalid!\n", name);
        tr069->fault_code = FAULT_ARGUMENTS;
        return 0;
    }

    tr069_object_delete(tr069, param);
    param = tr069_param_hash_find(tr069, "Device.ManagementServer.ParameterKey");
    if (param->prm_setval(param->name, paramkey, 0))
        TR069Error("setval\n");

    return 0;
Err:
    return -1;
}

//------------------------------------------------------------------------------
int tr069_s2c_reboot(struct TR069 *tr069)
{
    struct Soap *soap = tr069->soap;
    struct Reboot *reboot = &tr069->reboot;
    /*
        <CommandKey></CommandKey>
    */
    if (soap_in_element(soap, "CommandKey") || 
        soap_in_element_value(soap, reboot->commandkey, TR069_COMMANDKEY_LEN_32 + 1))
        TR069ErrorOut("soap_in_element CommandKey\n");
    reboot->rebootstate = REBOOT_STATE_START;
    tr069->sys_reboot = 1;
    tr069->save_flag = 1;

    return 0;
Err:
    return -1;
}

//------------------------------------------------------------------------------
int tr069_s2c_download(struct TR069 *tr069)
{
    struct Soap *soap = tr069->soap;
    struct Load *load, *next, *prev;
    int delayseconds;
    /*
        <CommandKey></CommandKey>
        <FileType></FileType>
        <URL></URL>
        <Username></Username>
        <Password></Password>
        <FileSize></FileSize>
        <TargetFileName></TargetFileName>
        <DelaySeconds></DelaySeconds>
        <SuccessURL></SuccessURL>
        <FailureURL></FailureURL>
    */
    load = (struct Load *)IND_CALLOC(sizeof(struct Load), 1);
    if (!load)
        TR069ErrorOut("malloc Load\n");

    if (soap_in_element(soap, "CommandKey") || 
        soap_in_element_value(soap, load->CommandKey, TR069_COMMANDKEY_LEN_32 + 1) || 
        soap_in_element(soap, "FileType") || 
        soap_in_element_value(soap, load->FileType, TR069_FILETYPE_LEN_64 + 1) || 
        soap_in_element(soap, "URL") || 
        soap_in_element_value(soap, load->URL, TR069_URL_LEN_1024 + 1) || 
        soap_in_element(soap, "Username") || 
        soap_in_element_value(soap, load->Username, TR069_USERNAME_LEN_256 + 1) || 
        soap_in_element(soap, "Password") || 
        soap_in_element_value(soap, load->Password, TR069_PASSWORD_LEN_256 + 1) || 
        soap_in_element(soap, "FileSize") || 
        soap_in_element(soap, "TargetFileName") || 
        soap_in_element_value(soap, load->TargetFileName, TR069_TARGETFILENAME_LEN_256 + 1) || 
        soap_in_element(soap, "DelaySeconds"))
        TR069ErrorOut("soap_in_element CommandKey\n");
    delayseconds = tr069_soap_value_int(tr069);

    if (soap_in_element(soap, "SuccessURL") || 
        soap_in_element(soap, "FailureURL"))
        TR069ErrorOut("soap_in_element SuccessURL\n");

    if (delayseconds > MAX_DELAY_SECONDS)
        TR069ErrorOut("delaySeconds = %d\n", delayseconds);

    g_loadSN ++;
    load->SN = g_loadSN;

    load->loadtime = (int)tr069_sec( );
    if (delayseconds > 0)
        load->loadtime += delayseconds;
    load->transferstate = TRANSFER_STATE_INIT;
    load->isDownload = 1;

    prev = NULL;
    next = tr069->loadQueue;
    while (next) {
        if (TRANSFER_STATE_INIT == load->transferstate && next->loadtime > load->loadtime)
            break;
        prev = next;
        next = next->next;
    }
    if (prev)
        prev->next = load;
    else
        tr069->loadQueue = load;
    load->next = next;

    tr069->save_flag = 1;
    tr069_load_timer(tr069);

    return 0;
Err:
    if (load)
        IND_FREE(load);
    return -1;
}

//------------------------------------------------------------------------------
int tr069_s2c_upload(struct TR069 *tr069)
{
    struct Soap *soap = tr069->soap;
    struct Load *load, *next, *prev;
    int delayseconds;
    /*
        <CommandKey></CommandKey>
        <FileType></FileType>
        <URL></URL>
        <Username></Username>
        <Password></Password>
        <DelaySeconds></DelaySeconds>
    */
    load = (struct Load *)IND_CALLOC(sizeof(struct Load), 1);
    if (!load)
        TR069ErrorOut("malloc Load\n");

    if (soap_in_element(soap, "CommandKey") || 
        soap_in_element_value(soap, load->CommandKey, TR069_COMMANDKEY_LEN_32 + 1) || 
        soap_in_element(soap, "FileType") || 
        soap_in_element_value(soap, load->FileType, TR069_FILETYPE_LEN_64 + 1) || 
        soap_in_element(soap, "URL") || 
        soap_in_element_value(soap, load->URL, TR069_URL_LEN_1024 + 1) || 
        soap_in_element(soap, "Username") || 
        soap_in_element_value(soap, load->Username, TR069_USERNAME_LEN_256 + 1) || 
        soap_in_element(soap, "Password") || 
        soap_in_element_value(soap, load->Password, TR069_PASSWORD_LEN_256 + 1) || 
        soap_in_element(soap, "DelaySeconds"))
        TR069ErrorOut("soap_in_element CommandKey\n");
    delayseconds = tr069_soap_value_int(tr069);

    if (delayseconds > MAX_DELAY_SECONDS)
        TR069ErrorOut("delaySeconds = %d\n", delayseconds);

    g_loadSN ++;
    load->SN = g_loadSN;

    load->loadtime = (int)tr069_sec( );
    if (delayseconds > 0)
        load->loadtime += delayseconds;
    load->transferstate = TRANSFER_STATE_INIT;
    load->isDownload = 0;

    prev = NULL;
    next = tr069->loadQueue;
    while (next) {
        if (TRANSFER_STATE_INIT == load->transferstate && next->loadtime > load->loadtime)
            break;
        prev = next;
        next = next->next;
    }
    if (prev)
        prev->next = load;
    else
        tr069->loadQueue = load;
    load->next = next;

    tr069->save_flag = 1;
    tr069_load_timer(tr069);

    return 0;
Err:
    return -1;
}

//------------------------------------------------------------------------------
int tr069_s2c_transfercomplete_response(struct TR069 *tr069)
{
    struct Load *load;

    load = tr069->loadQueue;
    if (load) {
        tr069->loadQueue = load->next;
        IND_FREE(load);
    }
    tr069_load_timer(tr069);

    //This method has no arguments
    return 0;
}

//------------------------------------------------------------------------------
int tr069_s2c_factoryreset(struct TR069 *tr069)
{
    //This method has no arguments
    tr069->sys_reset = 1;

    return 0;
}

//------------------------------------------------------------------------------
int tr069_s2c_fault(const struct TR069 *tr069)
{
    struct Soap *soap = tr069->soap;

    if (soap_in_element(soap, "faultcode") || 
        soap_in_element(soap, "faultstring") || 
        soap_in_element_begin(soap, "detail") || 
        soap_in_element_begin(soap, "Fault") || 
        soap_in_element(soap, "FaultCode") || 
        soap_in_element(soap, "FaultString") || 
        soap_in_element_end(soap) || //Fault
        soap_in_element_end(soap)) //detail
        TR069ErrorOut("soap_in_element faultcode\n");

    return 0;
Err:
    return -1;
}

//------------------------------------------------------------------------------
int tr069_s2c_scheduleinform(struct TR069 *tr069)
{
    int sec;
    struct Soap *soap = tr069->soap;
    struct Schedule *schedule = &tr069->schedule;

    if (soap_in_element(soap, "CommandKey") || 
        soap_in_element_value(soap, schedule->commandkey, TR069_COMMANDKEY_LEN_32 + 1))
        TR069ErrorOut("soap_in_element_value CommandKey\n");
    if (soap_in_element(soap, "DelaySeconds"))
        TR069ErrorOut("soap_in_element DelaySeconds\n");
    sec = tr069_soap_value_int(tr069);

    if (sec < MAX_DELAY_SECONDS) {
        schedule->scheduletime = (int)tr069_sec( ) + sec;
        tr069_schedule_timer(tr069);
    }

    return 0;
Err:
    return -1;
}

