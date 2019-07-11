
#include "bt_parse.h"
#include "rapidxml/rapidxml.hpp"
#include "BluetoothAssertions.h"
#include "../app/BrowserBridge/Huawei/BrowserEventQueue.h"

#include "browser/browser_event.h"
#include "libzebra.h"
#include "SysSetting.h"
#include "config/pathConfig.h"

#include <string.h>
#include <stdio.h>
#include <dlfcn.h>

#include <sstream>
#include <string>
#include <new>
using namespace rapidxml;


const char* kGetXmlFunctionName = "yhw_input_getBTxml";
typedef const char* (*LPFNGETXML)(void);

static void * g_zebra_handle = NULL;
static LPFNGETXML g_lpfnGetXml = NULL;
static int g_Bluetooth_state = 1;

// 键值表
static const struct _TAGKEYTABLE
{
    const char  *   xml_keyname;
    int             mid_keyvalue;
}g_bt_keycode_table[] =
{
    {"KEY_0",                   EIS_IRKEY_NUM0          },
/*{{{*/
    {"KEY_1",                   EIS_IRKEY_NUM1          },
    {"KEY_2",                   EIS_IRKEY_NUM2          },
    {"KEY_3",                   EIS_IRKEY_NUM3          },
    {"KEY_4",                   EIS_IRKEY_NUM4          },
    {"KEY_5",                   EIS_IRKEY_NUM5          },
    {"KEY_6",                   EIS_IRKEY_NUM6          },
    {"KEY_7",                   EIS_IRKEY_NUM7          },
    {"KEY_8",                   EIS_IRKEY_NUM8          },
    {"KEY_9",                   EIS_IRKEY_NUM9          },
    {"KEY_A",                   EIS_IRKEY_A             },
    {"KEY_ALT_LEFT",            EIS_IRKEY_LEFT          },
    {"KEY_ALT_RIGHT",           EIS_IRKEY_RIGHT         },
    {"KEY_APOSTROPHE",          EIS_IRKEY_UNKNOWN       },
    {"KEY_AT",                  EIS_IRKEY_UNKNOWN       },
    {"KEY_B",                   EIS_IRKEY_B             },
    {"KEY_BACK",                EIS_IRKEY_BACK          },
    {"KEY_BACKSLASH",           EIS_IRKEY_UNKNOWN       },
    {"KEY_BLUE",                EIS_IRKEY_BLUE          },
    {"KEY_C",                   EIS_IRKEY_C             },
    {"KEY_CALL",                EIS_IRKEY_UNKNOWN       },
    {"KEY_CAMERA",              EIS_IRKEY_UNKNOWN       },
    {"KEY_CHANNEL_DOWN",        EIS_IRKEY_CHANNEL_DOWN  },
    {"KEY_CHANNEL_UP",          EIS_IRKEY_CHANNEL_UP    },
    {"KEY_CLEAR",               EIS_IRKEY_CLEAR         },
    {"KEY_COMMA",               EIS_IRKEY_UNKNOWN       },
    {"KEY_D",                   EIS_IRKEY_D             },
    {"KEY_DEL",                 EIS_IRKEY_DELETE        },
    {"KEY_DPAD_CENTER",         EIS_IRKEY_UNKNOWN       },
    {"KEY_DPAD_DOWN",           EIS_IRKEY_DOWN          },
    {"KEY_DPAD_LEFT",           EIS_IRKEY_LEFT          },
    {"KEY_DPAD_RIGHT",          EIS_IRKEY_RIGHT         },
    {"KEY_DPAD_UP",             EIS_IRKEY_UP            },
    {"KEY_E",                   EIS_IRKEY_E             },
    {"KEY_ENDCALL",             EIS_IRKEY_UNKNOWN       },
    {"KEY_ENTER",               EIS_IRKEY_SELECT        },
    {"KEY_ENVELOPE",            EIS_IRKEY_UNKNOWN       },
    {"KEY_EQUALS",              EIS_IRKEY_UNKNOWN       },
    {"KEY_EXPLORER",            EIS_IRKEY_UNKNOWN       },
    {"KEY_F",                   EIS_IRKEY_F             },
    {"KEY_F1",                  EIS_IRKEY_VK_F1         },
    {"KEY_F10",                 EIS_IRKEY_VK_F2         },
    {"KEY_F11",                 EIS_IRKEY_VK_F11        },
    {"KEY_F12",                 EIS_IRKEY_VK_F12        },
    {"KEY_F13",                 EIS_IRKEY_UNKNOWN       },
    {"KEY_F14",                 EIS_IRKEY_UNKNOWN       },
    {"KEY_F15",                 EIS_IRKEY_UNKNOWN       },
    {"KEY_F16",                 EIS_IRKEY_UNKNOWN       },
    {"KEY_F2",                  EIS_IRKEY_VK_F2         },
    {"KEY_F3",                  EIS_IRKEY_VK_F3         },
    {"KEY_F4",                  EIS_IRKEY_VK_F4         },
    {"KEY_F5",                  EIS_IRKEY_VK_F5         },
    {"KEY_F6",                  EIS_IRKEY_VK_F6         },
    {"KEY_F7",                  EIS_IRKEY_VK_F7         },
    {"KEY_F8",                  EIS_IRKEY_VK_F8         },
    {"KEY_F9",                  EIS_IRKEY_VK_F9         },
    {"KEY_FOCUS",               EIS_IRKEY_UNKNOWN       },
    {"KEY_G",                   EIS_IRKEY_G             },
    {"KEY_GRAVE",               EIS_IRKEY_UNKNOWN       },
    {"KEY_GREEN",               EIS_IRKEY_GREEN         },
    {"KEY_GREY",                EIS_IRKEY_GREY          },
    {"KEY_H",                   EIS_IRKEY_H             },
    {"KEY_HEADSETHOOK",         EIS_IRKEY_UNKNOWN       },
    {"KEY_HOME",                EIS_IRKEY_HOME          },
    {"KEY_I",                   EIS_IRKEY_I             },
    {"KEY_J",                   EIS_IRKEY_J             },
    {"KEY_K",                   EIS_IRKEY_K             },
    {"KEY_L",                   EIS_IRKEY_L             },
    {"KEY_LEFT_BRACKET",        EIS_IRKEY_UNKNOWN       },
    {"KEY_M",                   EIS_IRKEY_M             },
    {"KEY_MEDIA_FAST_FORWARD",  EIS_IRKEY_FASTFORWARD   },
    {"KEY_MEDIA_NEXT",          EIS_IRKEY_PAGE_DOWN     },
    {"KEY_MEDIA_PLAY_PAUSE",    EIS_IRKEY_PLAY          },
    {"KEY_MEDIA_PREVIOUS",      EIS_IRKEY_PAGE_UP       },
    {"KEY_MEDIA_REWIND",        EIS_IRKEY_REWIND        },
    {"KEY_MEDIA_STOP",          EIS_IRKEY_STOP          },
    {"KEY_MENU",                EIS_IRKEY_MENU          },
    {"KEY_MINUS",               EIS_IRKEY_UNKNOWN       },
    {"KEY_MOUSE_TOUCH",         EIS_IRKEY_UNKNOWN       },
    {"KEY_MUTE",                EIS_IRKEY_VOLUME_MUTE   },
    {"KEY_N",                   EIS_IRKEY_N             },
    {"KEY_NOTIFICATION",        EIS_IRKEY_UNKNOWN       },
    {"KEY_NUM",                 EIS_IRKEY_UNKNOWN       },
    {"KEY_O",                   EIS_IRKEY_O             },
    {"KEY_P",                   EIS_IRKEY_P             },
    {"KEY_PAUSE",               EIS_IRKEY_PAUSE         },
    {"KEY_PERIOD",              EIS_IRKEY_UNKNOWN       },
    {"KEY_PIP",                 EIS_IRKEY_UNKNOWN       },
    {"KEY_PLAY",                EIS_IRKEY_PLAY          },
    {"KEY_PLUS",                EIS_IRKEY_UNKNOWN       },
    {"KEY_POUND",               EIS_IRKEY_POUND         },
    {"KEY_POWER",               EIS_IRKEY_POWER         },
    {"KEY_Q",                   EIS_IRKEY_Q             },
    {"KEY_R",                   EIS_IRKEY_R             },
    {"KEY_RECORD",              EIS_IRKEY_UNKNOWN       },
    {"KEY_RED",                 EIS_IRKEY_RED           },
    {"KEY_RIGHT_BRACKET",       EIS_IRKEY_UNKNOWN       },
    {"KEY_S",                   EIS_IRKEY_S             },
    {"KEY_SEARCH",              EIS_IRKEY_UNKNOWN       },
    {"KEY_SEMICOLON",           EIS_IRKEY_UNKNOWN       },
    {"KEY_SHIFT_LEFT",          EIS_IRKEY_LEFT          },
    {"KEY_SHIFT_RIGHT",         EIS_IRKEY_RIGHT         },
    {"KEY_SLASH",               EIS_IRKEY_UNKNOWN       },
    {"KEY_SOFT_LEFT",           EIS_IRKEY_LEFT          },
    {"KEY_SOFT_RIGHT",          EIS_IRKEY_RIGHT         },
    {"KEY_SPACE",               EIS_IRKEY_UNKNOWN       },
    {"KEY_STAR",                EIS_IRKEY_UNKNOWN       },
    {"KEY_SUBTITLE",            EIS_IRKEY_SUBTITLE      },
    {"KEY_SYM",                 EIS_IRKEY_VK_F10        },
    {"KEY_T",                   EIS_IRKEY_T             },
    {"KEY_TAB",                 EIS_IRKEY_UNKNOWN       },
    {"KEY_TRACK",               EIS_IRKEY_TRACK         },
    {"KEY_U",                   EIS_IRKEY_U             },
    {"KEY_UNKNOWN",             EIS_IRKEY_UNKNOWN       },
    {"KEY_V",                   EIS_IRKEY_V             },
    {"KEY_VOLUME_DOWN",         EIS_IRKEY_VOLUME_DOWN   },
    {"KEY_VOLUME_UP",           EIS_IRKEY_VOLUME_UP     },
    {"KEY_IRKEY_W",             EIS_IRKEY_W             },
    {"KEY_WHEEL_LEFT",          EIS_IRKEY_UNKNOWN       },
    {"KEY_WHEEL_RIGHT",         EIS_IRKEY_UNKNOWN       },
    {"KEY_X",                   EIS_IRKEY_X             },
    {"KEY_Y",                   EIS_IRKEY_Y             },
    {"KEY_YELLOW",              EIS_IRKEY_YELLOW        },
    {"KEY_Z",                   EIS_IRKEY_Z             },
    {"KEY_ENTRY",               EIS_IRKEY_INFO_BLUETOOTH},
/*}}}*/
};


static void init()
{
    static bool inited = false;
    if (inited)
        return;
    inited = true;

    if (!g_zebra_handle) {
        g_zebra_handle = dlopen("/home/"PLATFORM_HOME"/lib/libzebra.so", RTLD_LAZY);
        if (!g_zebra_handle) {
            BLUETOOTH_LOG("dlopen failed: %s\n", dlerror());
            return;
        }
        g_lpfnGetXml = (LPFNGETXML)dlsym(g_zebra_handle, kGetXmlFunctionName);
        if (!g_lpfnGetXml) {
            BLUETOOTH_LOG("dlsym failed: %s\n", dlerror());
            return;
        }
    }

}


BT_ParsingResult ParseAsBluetooth(unsigned int* msgno, int* type, int* stat)
{
    ASSERT(msgno != NULL);
    ASSERT(type != NULL);
    ASSERT(stat != NULL);
    if (msgno == NULL || type == NULL || stat == NULL)
        return BT_Continue;

    if (((*stat >> 24) & 0xff) != 0x60)
        return BT_Continue;

    init();
    if (!g_lpfnGetXml) {
        BLUETOOTH_LOG("\n\n\t\tYour SDK doesn't support bluetooth.\n\n\n");
        return BT_Processed;
    }

    const char * p = g_lpfnGetXml();
    if (!p) {
        BLUETOOTH_LOG("blue xml message hasn't been peeked.\n");
        return BT_Processed;
    }
    BLUETOOTH_LOG("bluetooth xml message: %s\n", p);

    char * buffer = NULL;
    try {
        buffer = new char[strlen(p) + 1];
        strcpy(buffer, p);

        xml_document<>  doc;
        doc.parse<0>(buffer);
        xml_node<> *    root = doc.first_node();
        if (!root)
            throw ("Not a valid xml string.");

        xml_node<> *    cmd = root->first_node("cmd");
        if (!cmd)
            throw ("request.cmd not found.");
        xml_node<> *    param = root->first_node("param");
        if (!param)
            throw ("request.param not found.");

        if (strncmp(cmd->value(), "KeyEvent", 8) == 0) {
            xml_node<> * keycode = param->first_node("keycode");
            xml_node<> * action = param->first_node("action");
            if (!keycode)
                throw ("Not a valid xml KeyEvent string.");

            int i, len;
            len = sizeof(g_bt_keycode_table) / sizeof(g_bt_keycode_table[0]);
            for (i=0; i<len; i++) {
                if (strcasecmp(g_bt_keycode_table[i].xml_keyname, keycode->value()) == 0) {
                    *msgno = g_bt_keycode_table[i].mid_keyvalue;
                    break;
                }
            }
            if (i >= len)
                throw ("No such key.");

            if (!action || atoi(action->value()) == 0)
                *type = YX_EVENT_KEYDOWN;
            else
                *type = YX_EVENT_KEYUP;
            BLUETOOTH_LOG("bluetooth key: %d/%#x\n", *msgno, *msgno);
            return BT_Continue;
        } else if (strncmp(cmd->value(), "TextEvent", 9) == 0) {
            xml_node<> * text = param->first_node("text");
            xml_node<> * inputMode = param->first_node("inputMode");
            xml_node<> * encode = param->first_node("encode");

            if (!text)
                throw ("not a valid TextEvent xml message.");

            std::stringstream    oss;
            const char qt = '"';
            oss << "{\n"
                << "\t" << qt << "type" << qt << " : "
                << qt << "EVENT_CONTROL_TEXT" << qt << ",\n"
                << "\t" << qt << "event_id" << qt << " : "
                << qt << "203902" << qt << ",\n"
                << "\t" << qt << "text" << qt << " : "
                << qt << text->value() << qt;
            if (inputMode) {
                oss << ",\n"
                    << "\t" << qt << "inputMode" << qt << " : "
                    << qt << inputMode->value() << qt;
            }
            if (encode) {
                oss << ",\n"
                    << "\t" << qt << "encode" << qt << " : "
                    << qt << encode->value() << qt;
            }
            oss << "\n}";
            BLUETOOTH_LOG("TextEvent:\n%s\n", oss.str().c_str());
            browserEventSend(oss.str().c_str(), NULL);
        } else if (strncmp(cmd->value(), "SyncToScreen", 12) == 0) {
            xml_node<> * mediaCode = param->first_node("mediaCode");
            xml_node<> * mediaType = param->first_node("mediaType");
            xml_node<> * entryID = param->first_node("entryID");
            xml_node<> * playByBookmark = param->first_node("playByBookmark");
            xml_node<> * playByTime = param->first_node("playByTime");
            if (!mediaCode || !mediaType)
                throw("Not a valid SyncToScreen xml message.");

            std::stringstream   oss;
            const char qt = '"';
            oss << "{\n"
                << "\t" << qt << "type" << qt << " : "
                << qt << "EVENT_CONTROL_SYNCTOSCREEN" << qt << ",\n"
                << "\t" << qt << "event_id" << qt << " : "
                << qt << "203901" << qt << ",\n"
                << "\t" << qt << "mediaCode" << qt << " : "
                << qt << mediaCode->value() << qt << ",\n"
                << "\t" << qt << "mediaType" << qt << " : "
                << qt << mediaType->value() << qt;
            if (entryID) {
                oss << ",\n"
                    << "\t" << qt << "entryID" << qt << " : "
                    << qt << entryID->value() << qt;
            }
            if (playByBookmark) {
                oss << ",\n"
                    << "\t" << qt << "playByBookmark" << qt << " : "
                    << qt << playByBookmark->value() << qt;
            }
            if (playByTime) {
                oss << ",\n"
                    << "\t" << qt << "playByTime" << qt << " : "
                    << qt << playByTime->value() << qt;
            }
            oss << "\n}";
            BLUETOOTH_LOG("SyncToScreen:\n%s\n", oss.str().c_str());
            browserEventSend(oss.str().c_str(), NULL);
        } else {
        }
    } catch(parse_error e) {
        BLUETOOTH_LOG("parse error. what() = %s, where() = %s\n", e.what(), e.where<char>());
    } catch(std::bad_alloc) {
        BLUETOOTH_LOG("new failed.\n");
    } catch(const char * e) {
        BLUETOOTH_LOG("parse error: %s\n", e);
    }

    if (buffer)
        delete [] buffer;
    return BT_Processed;
}

int BluetoothParameterGet(char *BluetoothName, char *BluetoothPIN)
{

    FILE *fp = NULL;
    char *p_hcif = NULL, *p_temp = NULL;
    int len = 0;
    fp = fopen(DEFAULT_MODULE_BT_DATAPATH"/hcid.modify.conf", "rw+");
    if (NULL == fp) {
        BLUETOOTH_LOG("%s , %d fopen Bluetooth config file error........",__FILE__, __LINE__);
        return -1;
    }
    p_hcif = (char *)malloc(32*1024);
    len = fread(p_hcif, 1, 32*1024, fp);
    if (len == 0) {
        BLUETOOTH_LOG("%s , %d fread Bluetooth config file error.........",__FILE__, __LINE__);
        fclose(fp);
        if (p_hcif) {
            free(p_hcif);
            p_hcif = NULL;
        }
        return -1;
    }
    p_hcif[len] = '\0';
    fclose(fp);

    if ((p_temp = strstr(p_hcif, "\""))) {
        sscanf(p_temp + 1, "%[^\";]", BluetoothName);
        BLUETOOTH_LOG("Have got the Bluetooth Name is : %s .....\n", BluetoothName);
    } else {
        BLUETOOTH_LOG("The Bluetooth Name is illicit ....\n");
    }

    if ((p_temp = strstr(p_hcif, "passkey "))) {
        sscanf(p_temp + 8, "%[^;]", BluetoothPIN);
        BLUETOOTH_LOG("Have got the Bluetooth PIN is : %s .....\n", BluetoothPIN);
        if (p_hcif) {
            free(p_hcif);
            p_hcif = NULL;
        }
    } else {
        BLUETOOTH_LOG("The Bluetooth PIN is illicit ....\n");
        if (p_hcif) {
            free(p_hcif);
            p_hcif = NULL;
        }
        return -1;
    }
    return 0;
}

int BluetoothParameterSet(int para, char *para_data)
{
    FILE *fp = NULL;
    char *hcid_buf = NULL, *temp = NULL;
    char *p_head = NULL, *p_last = NULL;
    char *p_parameter = NULL;
    int len = 0;
    char Bluetooth_Name[18] = {0};
    char Bluetooth_PIN[130] = {0};

    fp = fopen(DEFAULT_MODULE_BT_DATAPATH"/hcid.modify.conf", "rw+");
    if (NULL == fp) {
        BLUETOOTH_LOG(" fopen error \n");
        return -1;
    }

    sysSettingGetString("BluetoothDeviceName", Bluetooth_Name, 18, 0);
    sysSettingGetString("BluetoothPIN", Bluetooth_PIN, 130, 0);

    if (1 == para){
        p_parameter = Bluetooth_Name;
    } else if (0 == para) {
        p_parameter = Bluetooth_PIN;
    }
    hcid_buf = (char *)malloc(32*1024);
    if (NULL == hcid_buf) {
        BLUETOOTH_LOG(" malloc error !! hcid_buf\n");
        fclose(fp);
        return -1;
    }

    temp = (char *)malloc(32*1024);
    if (NULL == temp) {
        BLUETOOTH_LOG(" malloc error !! temp\n");
        if (hcid_buf) {
            free(hcid_buf);
            hcid_buf = NULL;
        }
        fclose(fp);
        return -1;
    }

    len = fread(hcid_buf, 1, 32*1024, fp);
    if(0 == len) {
        BLUETOOTH_LOG("fread error \n");
        if (hcid_buf) {
            free(hcid_buf);
            hcid_buf = NULL;
        }
        if (temp) {
            free(temp);
            temp = NULL;
        }
        fclose(fp);
        return -1;
    }

    hcid_buf[len] = '\0';
    p_head = hcid_buf;
    p_last = hcid_buf;
    p_head = strstr(hcid_buf, p_parameter);
    if (!p_head) {
        BLUETOOTH_LOG("The Bluetooth p_parameter is illicit ....\n");
        if (hcid_buf) {
            free(hcid_buf);
            hcid_buf = NULL;
        }
        if (temp) {
            free(temp);
            temp = NULL;
        }
        fclose(fp);
        return -1;
    }
    p_last = p_head + strlen(p_parameter);
    strcpy(temp, p_last);
    *p_head = '\0';

    strcat(hcid_buf, para_data);
    strcat(hcid_buf, temp);
    fseek(fp,0,SEEK_SET);
    len = fwrite(hcid_buf, 32*1024, 1, fp);
    if(0 == len) {
        BLUETOOTH_LOG(" fwrite error \n");
        if (hcid_buf) {
            free(hcid_buf);
            hcid_buf = NULL;
        }
        if (temp) {
            free(temp);
            temp = NULL;
        }
        fclose(fp);
        return -1;
    }

    if (hcid_buf) {
        free(hcid_buf);
        hcid_buf = NULL;
    }
    if (temp) {
        free(temp);
        temp = NULL;
    }
    fclose(fp);
    sleep(1);
    return 0;
}

int BluetoothStateflagGet()
{
    return g_Bluetooth_state;
}

void BluetoothStateflagSet(int state)
{
    g_Bluetooth_state = state;
}

void BluetoothDeviceOpen()
{
    if (!g_Bluetooth_state) {
        system("hciconfig hci0 up");
        g_Bluetooth_state = 1;
    }
}

void BluetoothDeviceClose()
{
    if (g_Bluetooth_state) {
        system("hciconfig hci0 down");
        g_Bluetooth_state = 0;
    }
}

int BluetoothDeviceNameSet(char *str)
{
    if (strlen(str)) {
        if (!BluetoothParameterSet(1, str)) {
            sysSettingSetString("BluetoothDeviceName", str);
            settingManagerSave();
            return 0;
        }
    }
    return -1;
}

int BluetoothPINSet(char *str)
{
    if (strlen(str)) {
        if (!BluetoothParameterSet(0, str)) {
            sysSettingSetString("BluetoothPIN", str);
            settingManagerSave();
            return 0;
        }
    }
    return -1;
}

int BluetoothParamCheck()
{
    char DeviceName[18] = {0};
    char PIN[130] = {0};

    sysSettingGetString("BluetoothDeviceName", DeviceName, 18, 0);
    if (!strlen(DeviceName)) {
        BluetoothParameterGet(DeviceName, PIN);
        sysSettingSetString("BluetoothDeviceName", DeviceName);
        sysSettingSetString("BluetoothPIN", PIN);
    }
    return 0;
}