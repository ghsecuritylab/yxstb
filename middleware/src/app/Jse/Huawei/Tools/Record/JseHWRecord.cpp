
#include "JseHWRecord.h"
#include "JseFunctionCall.h"

#include "JseAssertions.h"

#include "json/json_public.h"
#include "config/pathConfig.h"


#include <map>
#include <fstream>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define TOOLSRECORDFILE CONFIG_FILE_DIR"/yx_tools_record.ini"
#define _FSTREAM_SWITCH_TO_FOPEN //Android <fstream> libstlport.so have one bug; will delete when libstlport.so is ok or remove 'else'

class Tools_Record
{
public:
    Tools_Record() {
        m_Type = -1;
    }
    Tools_Record(int& aType, std::string& aVal)
    {
        m_Value = aVal, m_Type = aType;
    }
    Tools_Record(const Tools_Record& me);
    Tools_Record& operator=(const Tools_Record& p);

    static int addRecord(std::string key, Tools_Record aRecord);
    static int delRecord(std::string key);

public:
    std::string m_Value;
    int m_Type;
    static std::map<std::string, Tools_Record> s_RecordMap;
};

std::map<std::string, Tools_Record> Tools_Record::s_RecordMap;

Tools_Record::Tools_Record(const Tools_Record& me)
    : m_Value(me.m_Value)
    , m_Type(me.m_Type)
{

}
Tools_Record& Tools_Record::operator=(const Tools_Record& p)
{
    m_Value = p.m_Value;
    m_Type = p.m_Type;
    return *this;
}

int Tools_Record::addRecord(std::string key, Tools_Record aRecord)
{
    s_RecordMap[key] = aRecord;

    return 0;
}

int Tools_Record::delRecord(std::string key)
{
    std::map<std::string, Tools_Record>::iterator it;
    it = s_RecordMap.find(key);
    if(it != s_RecordMap.end()) {
        s_RecordMap.erase(it);
    }

    return 0;
}

static int JseSaveRecord(const char* param, char* value, int ret)
{

    Tools_Record record;
    std::string key;

    LogJseDebug("------------------jse_ioctl_RecordSave-----param[%s]-\n", param);

#if defined(_FSTREAM_SWITCH_TO_FOPEN)
    char lBuff[4097] = "";
    mode_t oldMask = umask(0077);
    FILE* outfile = fopen(TOOLSRECORDFILE, "wb");
    umask(oldMask);
    if (!outfile)
        return -1;
    std::map<std::string, Tools_Record>::iterator it;
    for (it = Tools_Record::s_RecordMap.begin(); it != Tools_Record::s_RecordMap.end(); it++) {
        record = it->second;
        key = it->first;
        if (record.m_Type == 2 || record.m_Type == 3) {
            snprintf(lBuff, 4096, "key=%s,value=%s,type=%d\n", key.c_str(), record.m_Value.c_str(), record.m_Type);
            fputs(lBuff, outfile);
        }
    }
    fflush(outfile);
    fclose(outfile);
#else
    mode_t oldMask = umask(0077);
    fstream outfile(TOOLSRECORDFILE, ios::out);
    umask(oldMask);

    if (!outfile) {
        LogJseDebug("open tools record file error!\n");
        return -1;
    }

    for (std::map<std::string, Tools_Record>::iterator it = Tools_Record::s_RecordMap.begin(); it != Tools_Record::s_RecordMap.end(); it++) {
        record = it->second;
        key = it->first;
        if (record.m_Type == 2 || record.m_Type == 3) {
            outfile << "key=" << key.c_str() << ",value=" << record.m_Value.c_str() << ",type=" << record.m_Type << endl;
        }
    }
    outfile.close();
    sync();
#endif
    return 0;

}

static int JseAddRecord(const char* param, char* value, int ret)
{
    struct json_object *object = NULL;
    struct json_object *obj = NULL;
    int type = 3;
    LogJseDebug("------------------jse_ioctlRead_AddRecord----param[%s]-value[%s]\n", param, value);

    object = json_tokener_parse_string(value);
    if (!object) {
        return -1;
    }

    obj = json_object_get_object_bykey(object, "key");
    if(!obj) {
        json_object_delete(object);
        return -1;
    }

    std::string key = json_object_get_string(obj);
    LogJseDebug("*******************key:%s\n", key.c_str());
    obj = json_object_get_object_bykey(object, "value");
    if (!obj) {
        json_object_delete(object);
        return -1;
    }

    std::string val = json_object_get_string(obj);
    LogJseDebug("*******************val:%s\n", val.c_str());
    obj = json_object_get_object_bykey(object, "type");
    if (!obj) {
        json_object_delete(object);
        return -1;
    }

    type = json_object_get_int(obj);
    if (val.empty()) {
        Tools_Record::delRecord(key);
        JseSaveRecord(NULL, NULL, 0);
        return 0;
     }
    Tools_Record record = Tools_Record(type, val) ;
    Tools_Record::addRecord(key, record);
    JseSaveRecord(NULL, NULL, 0);
    json_object_delete(object);
    return 0;
}

static int JseDelRecord(const char* param, char* value, int ret)
{
    struct json_object* object = NULL;
    struct json_object* obj = NULL;

    object = json_tokener_parse_string(param);
    if (!object) {
        return -1;
    }

    obj = json_object_get_object_bykey(object, "key");
    if (!obj) {
        json_object_delete(object);
        return -1;
    }

    std::string key = json_object_get_string(obj); //get key from value that's a JSON format string.
    Tools_Record::delRecord(key);

    //TODO: added your code here format a json string.
    json_object_delete(object);
    LogJseDebug("get record string :%s\n", value);
    return 0;
}

static int JseGetRecord(const char* param, char* value, int ret)
{
    struct json_object* object = NULL;
    struct json_object* obj = NULL;

    LogJseDebug("\n--jse_ioctlGetRecord----param[%s]--value[%s]--ret[%d]----\n", param, value, ret);

    object = json_tokener_parse_string(param);
    if(!object) {
        return -1;
    }

    obj = json_object_get_object_bykey(object, "key");
    if (!obj) {
        json_object_delete(object);
        return -1;
    }

    std::string key = json_object_get_string(obj); //get key from value that's a JSON format string.
    std::map<std::string, Tools_Record>::iterator it;
    LogJseDebug("get value by key %s\n", key.c_str());
    it = Tools_Record::s_RecordMap.find(key);
    if (it != Tools_Record::s_RecordMap.end()){
        char* src = (char*)Tools_Record::s_RecordMap[key].m_Value.c_str();
        strncpy(value, src, strlen(src));
  	    value[strlen(src)] = '\0';
    } else
        value[0] = '\0';

    //TODO: added your code here format a json string.
    json_object_delete(object);
    LogJseDebug("### get record string :%s\n", value);
    return 0;
}

static std::string GetValueByName(std::string param, std::string token)
{
    if (param.empty())
        return "fail";

    std::string::size_type position = param.find(token);
    if (position == std::string::npos)
        return "fail";

    return param.substr(position + token.length());
}

void ToolsRecordFileLoad()
{ /*重新加载yx_tools_record.ini内容到map中*/
    FILE* fp = NULL;
    std::string key, value, gline;
    std::string::size_type position;
    char ch;
    int type = 0;
    if (( fp = fopen(TOOLSRECORDFILE, "rb")) != NULL) {
        //gline eg. key=defineSTBName,value=55,type=3
        while ((ch = fgetc(fp)) != (char)EOF) {
            if(ch != '\r' && ch != '\n' && ch != (char)EOF) {
                gline += ch;
                continue;
            }
            if ((position = gline.find(",value=")) != std::string::npos) {
                std::string tmpline = gline.substr(0, position);
                gline.erase(0, position + 1);
                if (tmpline.find("key=") != std::string::npos) {
                    key = GetValueByName(tmpline, std::string("key="));
                    if (key == "fail") {
                        key.clear();
                        continue;
                    }
                    LogJseDebug("key=[%s]\n", key.c_str());
                }
            }
            if ((position = gline.find(",type=")) != std::string::npos) {
                std::string tmpline = gline.substr(0, position);
                gline.erase(0, position + 1);
                if (tmpline.find("value=") != std::string::npos) {
                    value = GetValueByName(tmpline, std::string("value="));
                    if (value == "fail") {
                        value.clear();
                        continue;
                    }
                    LogJseDebug("value=[%s]\n", value.c_str());
                }
            }
            if (gline.find("type=") != std::string::npos)
                type = atoi(GetValueByName(gline, std::string("type=")).c_str());
            LogJseDebug("type=[%d]\n", type);
            if (key.empty() || type == 0)
                continue;
            Tools_Record record = Tools_Record(type, value);
            Tools_Record::addRecord(key, record);
        }
        fclose(fp);
    }
}

/*************************************************
Description: 初始化并注册华为定义的接口 <Tools.Record.***> 
接口相关说明见 《IPTV 海外版本STB与EPG接口文档 STB公共能力(Webkit) V1.1》
Input: 无
Return: 无
 *************************************************/
JseHWRecord::JseHWRecord()
	: JseGroupCall("Record")
{
    ToolsRecordFileLoad();
    JseCall* call;
    
    call  = new JseFunctionCall("AddRecord", JseAddRecord, JseAddRecord);
    regist(call->name(), call);
    
    call  = new JseFunctionCall("Save", JseSaveRecord, JseSaveRecord);
    regist(call->name(), call);
    
    call  = new JseFunctionCall("DelRecord", JseDelRecord, JseDelRecord);
    regist(call->name(), call);
    
    call  = new JseFunctionCall("GetRecord", JseGetRecord, JseGetRecord);
    regist(call->name(), call);
}

JseHWRecord::~JseHWRecord()
{
}

