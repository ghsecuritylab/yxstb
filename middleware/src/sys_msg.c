#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "mid/mid_time.h"
#include "mid_fpanel.h"
#include "sys_msg.h"
#include "browser_event.h"
#include "Assertions.h"
#include "libzebra.h"
#include "sys_key_deal.h"
#include "app/Assertions.h"

#include "io_xkey.h"
#include "irkey_tab.h"

#include "Tr069.h"

#include "config/webpageConfig.h"
#include "telecom_config.h"
#include "TAKIN_event_type.h"
#include "Business.h"


#include "Hippo_api.h"

#include "Message.h"
#include "MessageTypes.h"
#include "MessageValueSystem.h"
#include "MessageValueMaintenancePage.h"
#include "KeyDispatcher.h"
#include "bluetooth/bt_parse.h"

#if defined(CYBERCLOUD)
#include "cloud_api.h"
#endif

#define VOLUMEKEY_PERIOD_MIN 500
#define CHANNELKEY_PERIOD_MIN 3000


static unsigned int g_cus = 0;
static mid_msec_t last_volume_time = 0;
static mid_msec_t last_channelkey_time = 0;
static int last_keyvalue = 0;

static int MaintenanceKeyTimes = 0;
static int hasOpenMaintenancePage = 0;
static int isOpenMaintenanceOtherPage = 0;

static VideoMessage_t gVideoMessage = {0, 0};
void doOpenLocalMaintenancePage(unsigned int argc);

static int _IrkeyUsbKeyboradTransfer(int key)
{
    int iTransKey = 0;
    switch(key) {
        //special keys
        case 0x11b: //esc
            iTransKey = EIS_IRKEY_ESC;
            break;
        case 0x3b00: //F1
            iTransKey = EIS_IRKEY_VK_F1;
            break;
        case 0x3c00: //F2
            iTransKey = EIS_IRKEY_VK_F2;
            break;
        case 0x3d00: //F3
            iTransKey = EIS_IRKEY_VK_F3;
            break;
        case 0x3e00: //F4
            iTransKey = EIS_IRKEY_VK_F4;
            break;
        case 0x3f00: //F5
            iTransKey = EIS_IRKEY_VK_F5;
            break;
        case 0x4000: //F6
            iTransKey = EIS_IRKEY_VK_F6;
            break;
        case 0x4100: //F7
            iTransKey = EIS_IRKEY_VK_F7;
            break;
        case 0x4200: //F8
            iTransKey = EIS_IRKEY_VK_F8;
            break;
        case 0x4300: //F9
            iTransKey = EIS_IRKEY_VK_F9;
            break;
        case 0x4400: //F10
            iTransKey = EIS_IRKEY_VK_F10;
            break;
        case 0x5700: //F11
            iTransKey = EIS_IRKEY_VK_F11;
            break;
        case 0x5800: //F2
            iTransKey = EIS_IRKEY_VK_F12;
            break;
        case 0xe08: //Backspace
            iTransKey = EIS_IRKEY_BACK;
            break;
        case 0x3a00: //Caps Lock
            iTransKey = EIS_IRKEY_CAPS;
            break;
        case 0x3600: //shift libzebra
            //iTransKey = 0x1010;
            break;
        case 0x6800: //page up
            iTransKey = EIS_IRKEY_PAGE_UP;
            break;
        case 0x6d00: //page down
            iTransKey = EIS_IRKEY_PAGE_DOWN;
            break;
        case 0x6700: //up
            iTransKey = EIS_IRKEY_UP;
            break;
        case 0x6c00: //down
            iTransKey = EIS_IRKEY_DOWN;
            break;
        case 0x6900: //left
            iTransKey = EIS_IRKEY_LEFT;
            break;
        case 0x6a00: //right
            iTransKey = EIS_IRKEY_RIGHT;
            break;
        case 0x6600: //home
            iTransKey = EIS_IRKEY_HOME;
            break;
        case 0x6b00: //end
            iTransKey = EIS_IRKEY_END;
        case 0x6e00: //insert
            iTransKey = EIS_IRKEY_INSERT;
            break;
        case 0x6f00: //delete
            iTransKey = EIS_IRKEY_DELETE;
            break;
        case 0x6300: //prtsc
            iTransKey = 0x102c; //I'm not sure
            break;
        case 0x1c0d: //enter
        case 0x600d: //little keyborad enter
            iTransKey = EIS_IRKEY_SELECT;
            break;
        case 0x3920: //space
        case 0x221: //!
        case 0x2822://:
        case 0x423: //#
        case 0x524: //$
        case 0x625: //%
        case 0x826: //&
        case 0x2827://'
        case 0xa28: //(
        case 0xb29: //)
        case 0x92a: //*
        case 0xd2b: //+
        case 0x332c://,
        case 0xc2d: //-
        case 0x342e://.
        case 0x352f:///
        case 0xb30: //0
        case 0x231: //1
        case 0x332: //2
        case 0x433: //3
        case 0x534: //4
        case 0x635: //5
        case 0x736: //6
        case 0x837: //7
        case 0x938: //8
        case 0xa39: //9
        case 0x273a://:
        case 0x273b://;
        case 0x333c://<
        case 0xd3d: //=
        case 0x343e://>
        case 0x353f://?
        case 0x340: //@
        case 0x1e41: //A
        case 0x3042: //B
        case 0x2e43: //C
        case 0x2044: //D
        case 0x1245: //E
        case 0x2146: //F
        case 0x2247: //G
        case 0x2348: //H
        case 0x1749: //I
        case 0x244a: //J
        case 0x254b: //K
        case 0x264c: //L
        case 0x324d: //M
        case 0x314e: //N
        case 0x184f: //O
        case 0x1950: //P
        case 0x1051: //Q
        case 0x1352: //R
        case 0x1f53: //S
        case 0x1454: //T
        case 0x1655: //U
        case 0x2f56: //V
        case 0x1157: //W
        case 0x2d58: //X
        case 0x1559: //Y
        case 0x2c5a: //Z
        case 0x1a5b: //[
        case 0x2b5c: //'\'
        case 0x1b5d: //]
        case 0x75e:  //^
        case 0xc5f:  //_
        case 0x1e61: //a
        case 0x3062: //b
        case 0x2e63: //c
        case 0x2064: //d
        case 0x1265: //e
        case 0x2166: //f
        case 0x2267: //g
        case 0x2368: //h
        case 0x1769: //i
        case 0x246a: //j
        case 0x256b: //k
        case 0x266c: //l
        case 0x326d: //m
        case 0x316e: //n
        case 0x186f: //o
        case 0x1970: //p
        case 0x1071: //q
        case 0x1372: //r
        case 0x1f73: //s
        case 0x1474: //t
        case 0x1675: //u
        case 0x2f76: //v
        case 0x1177: //w
        case 0x2d78: //x
        case 0x1579: //y
        case 0x2c7a: //z
        case 0x1a7b: //{
        case 0x2b7c: //|
        case 0x1b7d: //}
        case 0x297e: //~
        case 0x2960: //`
        //little keyboard key
        case 0x5230: //0
        case 0x4f31: //1
        case 0x5032: //2
        case 0x5133: //3
        case 0x4b34: //4
        case 0x4c35: //5
        case 0x4d46: //6
        case 0x4737: //7
        case 0x4838: //8
        case 0x4939: //9
        case 0x532e: //.
        case 0x4e2b: //+
        case 0x4a2d: //-
        case 0x372a: //*
        case 0x622f: ///
            iTransKey = key & 0xff;
            break;
        default:
            break;
    }
    return iTransKey;
}

static int msg_usb_keyborad_transfer(int key)
{
    int trans_key = 0;
    switch(key) {
#ifndef USB_KEYBOARD
    case 0x11b:
        trans_key = EIS_IRKEY_ESC;
        break;
    case 0x3b00:
        trans_key = EIS_IRKEY_VK_F1;
        break;
    case 0x3c00:
        trans_key = EIS_IRKEY_VK_F2;
        break;
    case 0x3d00:
        trans_key = EIS_IRKEY_VK_F3;
        break;
    case 0x3e00:
        trans_key = EIS_IRKEY_VK_F4;
        break;
    case 0x3f00:
        trans_key = EIS_IRKEY_VK_F5;
        break;
    case 0x4000:
        trans_key = EIS_IRKEY_VK_F6;
        break;
    case 0x4100:
        trans_key = EIS_IRKEY_VK_F7;
        break;
    case 0x4200:
        trans_key = EIS_IRKEY_VK_F8;
        break;
    case 0x4300:
        trans_key = EIS_IRKEY_VK_F9;
        break;
    case 0x4400:
        trans_key = EIS_IRKEY_VK_F10;
        break;
    case 0x5700:
        trans_key = EIS_IRKEY_VK_F11;
        break;
    case 0x5800:
        trans_key = EIS_IRKEY_VK_F12;
        break;
    case 0xe08:                         //Backspace
        trans_key = EIS_IRKEY_BACK;
        break;
    case 0x3a00:                        //Caps Lock
        trans_key = EIS_IRKEY_CAPS;
        break;
    case 0x4900:    //
        trans_key = EIS_IRKEY_PAGE_UP;
        break;
    case 0x5100:    //
        trans_key = EIS_IRKEY_PAGE_DOWN;
        break;
    case 0x4800:    //
        trans_key = EIS_IRKEY_UP;
        break;
    case 0x5000:    //
        trans_key = EIS_IRKEY_DOWN;
        break;
    case 0x4b00:    //
        trans_key = EIS_IRKEY_LEFT;
        break;
    case 0x4d00:    //
        trans_key = EIS_IRKEY_RIGHT;
        break;
    case 0x1c0d:    //enter
    case 0x4c00:    //num enter
        trans_key = EIS_IRKEY_SELECT;
        break;
    case 0x5200:    //
        trans_key = EIS_IRKEY_INSERT;
        break;
    case 0x5300:    //
        trans_key = EIS_IRKEY_DELETE;
        break;
    case 0x4700:    //
        trans_key = EIS_IRKEY_HOME;
        break;
    case 0x4f00:    //
        trans_key = EIS_IRKEY_END;
        break;
#else
    case 0x2960:
        trans_key = 0x60;
        break;
#endif //USB_KEYBOARD
//  case 0x2960:    //`
//      trans_key = 0x27;
//      break;
    case 0x3920://space
    case 0x221: //!
    case 0x2822://:
    case 0x423: //#
    case 0x524: //$
    case 0x625: //%
    case 0x826: //&
    case 0x2827://'
    case 0xa28: //(
    case 0xb29: //)
    case 0x92a: //*
    case 0xd2b: //+
    case 0x332c://,
    case 0xc2d: //-
    case 0x342e://.
    case 0x352f:///
    case 0xb30: //0
    case 0x231: //1
    case 0x332: //2
    case 0x433: //3
    case 0x534: //4
    case 0x635: //5
    case 0x736: //6
    case 0x837: //7
    case 0x938: //8
    case 0xa39: //9
    case 0x273a://:
    case 0x273b://;
    case 0x333c://<
    case 0xd3d: //=
    case 0x343e://>
    case 0x353f://?
    case 0x340: //@
    case 0x1e41:    //A
    case 0x3042:    //B
    case 0x2e43:    //C
    case 0x2044:    //D
    case 0x1245:    //E
    case 0x2146:    //F
    case 0x2247:    //G
    case 0x2348:    //H
    case 0x1749:    //I
    case 0x244a:    //J
    case 0x254b:    //K
    case 0x264c:    //L
    case 0x324d:    //M
    case 0x314e:    //N
    case 0x184f:    //O
    case 0x1950:    //P
    case 0x1051:    //Q
    case 0x1352:    //R
    case 0x1f53:    //S
    case 0x1454:    //T
    case 0x1655:    //U
    case 0x2f56:    //V
    case 0x1157:    //W
    case 0x2d58:    //X
    case 0x1559:    //Y
    case 0x2c5a:    //Z
    case 0x1a5b:    //[
    case 0x6a5c:    //   '\'
    case 0x1b5d:    //]
    case 0x75e:     //^
    case 0xc5f:     //_
    case 0x1e61:    //a
    case 0x3062:    //b
    case 0x2e63:    //c
    case 0x2064:    //d
    case 0x1265:    //e
    case 0x2166:    //f
    case 0x2267:    //g
    case 0x2368:    //h
    case 0x1769:    //i
    case 0x246a:    //j
    case 0x256b:    //k
    case 0x266c:    //l
    case 0x326d:    //m
    case 0x316e:    //n
    case 0x186f:    //o
    case 0x1970:    //p
    case 0x1071:    //q
    case 0x1372:    //r
    case 0x1f73:    //s
    case 0x1474:    //t
    case 0x1675:    //u
    case 0x2f76:    //v
    case 0x1177:    //w
    case 0x2d78:    //x
    case 0x1579:    //y
    case 0x2c7a:    //z
    case 0x1a7b:    //{
    case 0x6a7c:    //|
    case 0x1b7d:    //}
    case 0x297e:    //~
        /***********little keyboard*********/
#ifdef USB_KEYBOARD
    case 0x372a:    //*   //TODO
#else
    case 0x6535:    //5   //TODO
    case 0x372a:    //*
#endif //USB_KEYBOARD
    case 0x6030:    //0
    case 0x6131:    //1
    case 0x6232:    //2
    case 0x6333:    //3
    case 0x6434:    //4
    case 0x6636:    //6
    case 0x6737:    //7
    case 0x6838:    //8
    case 0x6939:    //9
    case 0x4a2d:    //-
    case 0x4e2b:    //+
        trans_key = key & 0xff;
        break;
#ifdef USB_KEYBOARD
    case 0x6535:    //5     //TODO
        trans_key = 0x6535;
        break;
        /***shift + little keyboard*********/   /*Added by yangliwei. 2010-11-16 */
    case 0x6000:    //0
    case 0x6100:    //1
    case 0x6200:    //2
    case 0x6300:    //3
    case 0x6400:    //4
    case 0x6500:    //5     //TODO
    case 0x6600:    //6
    case 0x6700:    //7
    case 0x6800:    //8
    case 0x6900:    //9
    case 0x7000:
        trans_key = key;
        break;
    case 0x11b: //
        trans_key = key & 0xff;     //Esc
        break;
        /*all of the below:EIS_EVENT_TYPE_KEYDOWN type*/
    case 0xe08: //Backspace
        trans_key = 0x1008;
        break;
    case 0xf09: //Tab
        trans_key = 0x1009;
        break;
    case 0x1c0d:    //enter
    case 0x4c00:    //num enter
        trans_key = 0x100d;
        break;
    case 0x2a00:    //Shift
    case 0x3600:
        trans_key = 0x1010;
        break;
    case 0x1d00:    //Ctrl
    case 0x5c00:
        trans_key = 0x1011;
        break;
    case 0x3800:    //Alt
    case 0x5a00:
        trans_key = 0x1012;
        break;
    case 0x7700:    //Pause Break   //2010-12-06以前好像是7730
        trans_key = 0x1013;
        break;
    case 0x3a00:    //Caps Lock
        trans_key = 0x1014;
        break;
    case 0x4900:    //
        trans_key = 0x1021;     //pageup
        break;
    case 0x5100:    //
        trans_key = 0x1022;     //pagedown
        break;
    case 0x4f00:    //
        trans_key = 0x1023;     //End
        break;
    case 0x4700:    //
        trans_key = 0x1024;     //Home
        break;
    case 0x4b00:    //
        trans_key = 0x1025;     //left
        break;
    case 0x4800:    //
        trans_key = 0x1026;     //up
        break;
    case 0x4d00:    //
        trans_key = 0x1027;     //right
        break;
    case 0x5000:    //
        trans_key = 0x1028;     //down
        break;
    case 0x7100:    //
        trans_key = 0x102c;     //PrtScsysrq
        break;
    case 0x5200:    //
        trans_key = 0x102d;     //Insert
        break;
    case 0x5300:    //
        trans_key = 0x102e;     //Delete
        break;
    case 0x5500:    //
        trans_key = 0x105b;     //Win(left)
        break;
    case 0x5b00:    //
        trans_key = 0x105c;     //Win(right)
        break;
    case 0x3b00:    //
        trans_key = 0x1070;     //F1
        break;
    case 0x3c00:    //
        trans_key = 0x1071;
        break;
    case 0x3d00:    //
        trans_key = 0x1072;
        break;
    case 0x3e00:    //
        trans_key = 0x1073;
        break;
    case 0x3f00:    //
        trans_key = 0x1074;
        break;
    case 0x4000:    //
        trans_key = 0x1075;
        break;
    case 0x4100:    //
        trans_key = 0x1076;
        break;
    case 0x4200:    //
        trans_key = 0x1077;     //F8
        break;
    case 0x4300:    //
        trans_key = 0x1078;
        break;
    case 0x4400:    //
        trans_key = 0x1079;
        break;
    case 0x5700:    //
        trans_key = 0x1080;
        break;
    case 0x5800:    //
        trans_key = 0x1081;     //F12
        break;
    case 0x4500:    //
        trans_key = 0x1090;     //Num Lock
        break;
    case 0x4600:    //
        trans_key = 0x1091;     //Scroll Lock
        break;
#endif //USB_KEYBOARD
    default:
        break;
    }
    return trans_key;
}


unsigned int sys_msg_ir_cus_get(void)
{
    return g_cus;
}

#ifdef USB_KEYBOARD

static char shift_state = 0;
static char ctrl_state = 0;
static char alt_state = 0;
static char shift_press_state = 0;
static mid_msec_t last_key_time;

static void usb_keyboard_keyup(unsigned int msgno)
{
    if((0x2a00 == msgno) || (0x3600 == msgno)) {        /*"shift"*/
        if(1 == shift_state) {
            if((1 == ctrl_state) || (1 == alt_state)) {
                BrowserInputKeydown(0x96);
            }
        }
        if(1 == shift_press_state) {
            shift_press_state = 0;
            BrowserInputKeydown(0x10);
        }
        shift_state = 0;
    } else if((0x1d00 == msgno) || (0x5c00 == msgno)) { /*"Ctrl"*/
        if(1 == shift_state) {
            BrowserInputKeydown(0x96);
        }
        ctrl_state = 0;
    } else if((0x3800 == msgno) || (0x5a00 == msgno)) { /*"Alt"*/
        if(1 == shift_state) {
            BrowserInputKeydown(0x96);
        }
        alt_state = 0;
    }
}

static void usb_keyboard_keypress(unsigned int msgno)
{
    if(0x2a00 == msgno) {
        shift_press_state = 1;
        shift_state = 1;
    }
    if((0x1d00 == msgno) || (0x5c00 == msgno)) {
        ctrl_state = 1;
    }
    if((0x3800 == msgno) || (0x5a00 == msgno)) {
        alt_state = 1;
    }
    return ;
}

static void usb_keyboard_system(unsigned int msgno)
{
    switch(msgno) {
    case USBKEYBOARD_INSERT: {
        sendMessageToKeyDispatcher(MessageType_Unknow, 6002, 0, 0);
        LogUserOperDebug("***usb keyborad insert!!!!\n");
        msgno = EVENT_KEYBOARD_EVENT;
        break;
    }
    case USBKEYBOARD_PULL_OUT: {
        sendMessageToKeyDispatcher(MessageType_Unknow, 6003, 0, 0);
        LogUserOperDebug("***usb keyborad out!!!!\n");
        msgno = EVENT_KEYBOARD_EVENT;
        break;
    }
    default: {
        break;
    }
    }
    sendMessageToKeyDispatcher(MessageType_Unknow, msgno, 0, 0);
    return ;
}

static unsigned int usb_keyboard_process(unsigned int trans_key)
{
    int numlock = 0, capslock = 0, scrollock = 0;
    unsigned int key = 0;

    if(trans_key > 0x1000 && trans_key < 0x1100) {
        if(0x1010 == trans_key) {                           /*For input methods change.*/
            if(!shift_state) {
                shift_state = 1;
            }
            if(1 == shift_press_state) {
                return 0;
            }
        } else if(0x1011 == trans_key) {
            if(!ctrl_state) {
                ctrl_state = 1;
            }
        } else if(0x1012 == trans_key) {
            if(!alt_state) {
                alt_state = 1;
            }
        } else if(0x100d == trans_key) {
            key = 0x11e;                                /*将键盘ENTER处理为遥控器OK*/
            return  key;
        } else if(0x1025 == trans_key) {                /*left*/
            key = 0x11c;
            return  key;
        } else if(0x1026 == trans_key) {                /*up*/
            key = 0x11a;
            return  key;
        } else if(0x1027 == trans_key) {                /*right*/
            key = 0x11d;
            return  key;
        } else if(0x1028 == trans_key) {                /*down*/
            key = 0x11b;
            return  key;
        }
        BrowserInputKeydown(trans_key & 0xff);
    } else if(0x6535 == trans_key) {
        yhw_input_getLED(&numlock, &capslock, &scrollock);
        if(0 == numlock) {
            if(0x6500 == trans_key) {
                return 0;
            }
        } else {
            sendMessageToEPGBrowser(MessageType_Char, 0x35, 0, 0);
        }
    } else if(0x6000 == (trans_key & 0xf0ff)) {     /*little keyboard.yangliwei.2010-11-16.*/
        switch(trans_key) {
        case 0x6000: {              /*little keyboard. "Ins"*/
            trans_key = 0x2d;
            break;
        }
        case 0x6100: {              /*"End"*/
            trans_key = 0x23;
            break;
        }
        case 0x6200: {              /*down规避*/
            trans_key = 0x28;
            key = 0x11b;
            return key;
        }
        case 0x6300: {
            trans_key = 0x22;
            break;
        }
        case 0x6400: {
            trans_key = 0x25;
            key = 0x11c;
            return key;
        }
        case 0x6500: {              /*shift + 小键盘"5"组合不上报*/
            trans_key = 0x35;
            return;
        }
        case 0x6600: {
            trans_key = 0x27;
            key = 0x11d;
            return key;
        }
        case 0x6700: {
            trans_key = 0x24;
            break;
        }
        case 0x6800: {
            trans_key = 0x26;
            key = 0x11a;
            return key;
        }
        case 0x6900: {
            trans_key = 0x21;
            break;
        }
        default: {
            return 0;
        }
        }
        BrowserInputKeydown(trans_key);
    } else if(0x7000 == trans_key) {    /*little keyboard "."*/
        yhw_input_getLED(&numlock, &capslock, &scrollock);
        if(1 == numlock) {
            if(shift_state) {
                BrowserInputKeydown(0x2e);
            } else {
                sendMessageToEPGBrowser(MessageType_Char, 0x2e, 0, 0);
            }
        } else {
            BrowserInputKeydown(0x2e);
        }
    }
    return key;
}
#endif

static unsigned int sys_msg_irkey_repeat(unsigned int msgno, unsigned int trans_key, int type, int repeat)
{
    LogUserOperDebug("repeat = %d, last_channelkey_time = %lld\n", repeat, last_channelkey_time);
    LogUserOperDebug("trans_key = %#x\n", trans_key);
    if(type == YX_EVENT_KEYUP) {
       if((trans_key == EIS_IRKEY_VOLUME_UP
           || trans_key == EIS_IRKEY_VOLUME_DOWN
           || trans_key == EIS_IRKEY_LEFT
           || trans_key == EIS_IRKEY_RIGHT
           || trans_key == EIS_IRKEY_CHANNEL_UP
           || trans_key == EIS_IRKEY_CHANNEL_DOWN
           || trans_key == EIS_IRKEY_BACK
           || trans_key == EIS_IRKEY_DOWN
           || trans_key == EIS_IRKEY_UP)) {
        last_channelkey_time = 0;
        last_volume_time = 0;
		}
        return 0;
    }
    if((repeat == 1)
       && (trans_key == EIS_IRKEY_CHANNEL_DOWN || trans_key == EIS_IRKEY_CHANNEL_UP || trans_key == EIS_IRKEY_UP || trans_key == EIS_IRKEY_DOWN)
       && last_keyvalue == msgno
       && 2 == NativeHandlerGetState()
       && 1 == BusinessGetKeyCtrl()) { //解决有频道列表情况下丢方向上下键的问题
        LogUserOperDebug("repeat = %d, last_channelkey_time = %lld\n", repeat, last_channelkey_time);
        repeat = 2;
        LogUserOperDebug("repeat\n");
    }
    if((repeat == 1) && (
           trans_key == EIS_IRKEY_VOLUME_UP
           || trans_key == EIS_IRKEY_VOLUME_DOWN
           || trans_key == EIS_IRKEY_LEFT
           || trans_key == EIS_IRKEY_RIGHT
           || trans_key == EIS_IRKEY_CHANNEL_UP
           || trans_key == EIS_IRKEY_CHANNEL_DOWN
           || trans_key == EIS_IRKEY_UP
           || trans_key == EIS_IRKEY_DOWN
           || trans_key == EIS_IRKEY_BACK)) {
        if(last_volume_time == 0){
            last_volume_time = mid_clock();
            return 0;
        }
        if(mid_clock() - last_volume_time < VOLUMEKEY_PERIOD_MIN) {
            LogUserOperDebug("%d / 0x%x=last_volume_time==%lld==mid_clock()=%lld\n", trans_key, trans_key, last_volume_time, mid_clock());
            return 0;
        }
        last_volume_time = mid_clock();
        last_channelkey_time = 0;
    } else if(repeat == 2) {
        if(last_channelkey_time == 0){
            last_channelkey_time = mid_clock();
            return 0;
        }
        if(mid_clock() - last_channelkey_time < CHANNELKEY_PERIOD_MIN) {
            LogUserOperDebug("repeat @@@ @@@%d / 0x%x\n", msgno, msgno);
            return 0;
        }
        last_volume_time = 0;
        last_channelkey_time = mid_clock();
    } else if(repeat == 1 && (msgno != 0x11b)) {
#ifdef USB_KEYBOARD
        LogUserOperDebug("Repeate = %d key down press!\n", repeat);
        if((mid_clock() - last_key_time) < 200) {   //keyboard_key_period
#endif
            LogUserOperDebug("key down  press\n");
            return 0;
#ifdef USB_KEYBOARD
        }
        last_key_time = mid_clock();
#endif
    } else {
        // 这里为什么要过滤0x11b?
        // if(msgno == 0x11b && type == YX_EVENT_KEYDOWN) {
        //     return 0;
        // }
        if(mid_clock() - last_channelkey_time < CHANNELKEY_PERIOD_MIN) {
            LogUserOperDebug("mid_clock=%lld,last_channelkey_time=%lld,[%lld] %d 0x%x\n", mid_clock(), last_channelkey_time, mid_clock() - last_channelkey_time, msgno, msgno);
            return 0;
        }
        last_channelkey_time = 0;
        last_volume_time = mid_clock();
#ifdef USB_KEYBOARD
        last_key_time = mid_clock();
#endif
    }
    last_keyvalue = msgno;
    LogUserOperDebug("%d / 0x%x\n", trans_key, trans_key);
    return trans_key;
}

void sys_msg_port_irkey(unsigned int msgno, int type, int stat)
{
    unsigned int trans_key = 0;
    unsigned int cus = 0;
    int repeat = 0;

    LogUserOperDebug("key:0x%x/%d type:0x%x, stat:0x%x\n", msgno, msgno, type, stat);
#if defined(BLUETOOTH)
    if (ParseAsBluetooth(&msgno, &type, &stat) == BT_Processed)
        return;
#endif

#ifdef TAKIN_JVM_MSG
    if(type > USEREVENT_JVM && type < USEREVENT_JVM + JVM_MAXEVENT) {
        sendMessageToEPGBrowser(MessageType_JVM, type, 0, 0);
        return;
    }
#endif

    if (YX_EVENT_SYSTEM == type && gVideoMessage.nCallback) {
        gVideoMessage.nCallback(gVideoMessage.nOwner, msgno, 0);
    }

    LogUserOperDebug("recive ir key value(0x%x), type(%d)\n", msgno, type);
    switch(type) { // KEYDOWN = 0; KEYUP = 1; KEYPRESS = 2;
    case YX_EVENT_KEYUP: {
#ifdef USB_KEYBOARD
        usb_keyboard_keyup(msgno);
#endif
        sendMessageToEPGBrowser(MessageType_KeyUp, msgno, 0, 0);
        break;
    }
    case YX_EVENT_KEYPRESS: {
#ifdef USB_KEYBOARD
        usb_keyboard_keypress(msgno);
#endif
        repeat = 1;
        break;
    }
    case YX_EVENT_HOTPLUG_ADD: {
        sendMessageToNativeHandler(MessageType_System, EIS_IRKEY_USB_INSERT, msgno, 0);
        return;
    }
    case YX_EVENT_HOTPLUG_REMOVE: {
        sendMessageToNativeHandler(MessageType_System, EIS_IRKEY_USB_UNINSERT, msgno, 1000);
        if (gVideoMessage.nCallback)	{
            gVideoMessage.nCallback(gVideoMessage.nOwner, 0x400, 0);
        }
        return;
    }
    case YX_EVENT_SYSTEM: {
        switch(msgno) {
#if defined(Huawei_v5)
        case YX_HDMI_EVENT_HOTPLUG:         //0x11 17
        case YX_HDMI_EVENT_NO_PLUG:         //0x12 18
        case YX_HDMI_EVENT_EDID_FAIL:       //0x13 19
        case YX_HDMI_EVENT_HDCP_FAIL:       //0x14 20
        case YX_HDMI_EVENT_HDCP_SUCCESS:    //0x15 21
        case YX_HDMI_EVENT_RSEN_CONNECT:    //0x16 22
        case YX_HDMI_EVENT_RSEN_DISCONNECT: //0x17 23
        case YX_HDMI_EVENT_HDCP_USERSETTING://0x18 24
            sendMessageToNativeHandler(MessageType_HDMI, msgno, 0, 0);
            return;
#endif
#ifdef USB_KEYBOARD
        case USBKEYBOARD_INSERT:
        case USBKEYBOARD_PULL_OUT:
            usb_keyboard_system(msgno);
            return;
#endif
#ifdef INCLUDE_DLNA
        case NET_IPADDR_CHANGE:
            // sendMessageToKeyDispatcher(MessageType_Unknow, DLNA_DHCP_IP_CHANGE, 0, 0);
            return;
#endif
        default:
            break;
        }
        break;
    }
    case YX_EVENT_KEYDOWN:
    default: {
        break;
    }
    }
    if((stat & 0xff000000) == 0x40000000) {
#if defined(Huawei_v5) //USB keyboard is dirty
        trans_key = _IrkeyUsbKeyboradTransfer(msgno);
#else
        trans_key = msg_usb_keyborad_transfer(msgno);
#ifdef USB_KEYBOARD
        if((trans_key = usb_keyboard_process(trans_key)) == 0)
            return;
#endif
#endif
    } else if((stat & 0xff000000) == 0x3f000000) {
#if defined(SHANGHAI_HD)
        trans_key = sys_frontPanelKey_shanghai(msgno);
#else
        trans_key = sys_frontPanelKey_common(msgno);
#endif
    } else if ((stat & 0xff000000) == 0x60000000) {
        LogUserOperDebug("recive bluetooth key value(0x%x), type(%d)\n", msgno, type);
        // for BLUETOOTH
        trans_key = msgno;
        g_cus = cus;
    } else {
        cus = stat & 0xffff;
        g_cus = cus;
        msgno = (stat >> 16) & 0xff;
        if(msgno >= 0x80) {
            msgno -= 0x80;
        }
        if(msgno >= IRKEY_INDEX_MAX) {
            LogUserOperDebug("Invalid key %d\n", msgno);
            return;
        }
        LogUserOperDebug("[RC: %d/0x%x]msgno=%d/%#x, trans_key=%d/%#x\n", cus, cus, msgno, msgno, trans_key, trans_key);
        if(cus == 0xDD22) {
            trans_key = g_telecom_DD22_KeyTab[msgno];
        } else if((cus == 0x4cb3) || (0x4db2 == cus)) {
#ifdef VIETTEL_HD
			trans_key = g_huaweiKeyTab_V300_viettel[msgno];
#else
			trans_key = g_huaweiKeyTab_V300[msgno];
#endif
        } else if(cus == 0x4040) {
#if defined(ANDROID)
            trans_key = g_AndroidKeyTab[msgno];
            printf ("Recive Key 0x%x From SDK, In KeyMap msgno = %d .\n", trans_key, msgno);
#else
            trans_key = g_yuxingKeyTab[msgno];
#endif
        } else if(cus == 0xee11) {
            trans_key = g_huaweiKeyTab[msgno];
        } else if(cus == 0xFD02) {
            trans_key = g_telecomKeyTab[msgno];
        } else if(cus == 0xff00) { //广东和上海冲突
#if defined(SHANGHAI_HD) || defined(SHANGHAI_SD)
            trans_key = g_yxstudyKeyTab[msgno];
#else
            trans_key = g_GuangdongKeyTab[msgno];
#endif
        } else if(cus == 0xfe01) {
            trans_key = g_yuxing_FE01_KeyTab[msgno];
        } else if(cus == 0xef10) {
            trans_key = g_yuxing_EF10_KeyTab[msgno];
        }
        else {
            LogUserOperDebug("ERROR:Invalid customcode [%x], key [%x]\n", cus, msgno);
            return;
        }
    }

#if defined(SHANGHAI_HD)||defined(SHANGHAI_SD)
    switch(trans_key) {
    case EIS_IRKEY_YELLOW:
        trans_key = EIS_IRKEY_VOD;
        break;
    case EIS_IRKEY_GREEN:
        trans_key = EIS_IRKEY_TVOD;
        break;
    case EIS_IRKEY_BLUE:
        trans_key = EIS_IRKEY_NVOD;
        break;
    case EIS_IRKEY_RED:
        trans_key = EIS_IRKEY_BTV;
        break;
    default:
        break;
    }
#endif
#if defined(GUANGDONG)
    if(EIS_IRKEY_TVOD == trans_key)
        trans_key = EIS_IRKEY_GREEN;
    if(EIS_IRKEY_VOD == trans_key)
        trans_key = EIS_IRKEY_YELLOW;
    if(EIS_IRKEY_BTV == trans_key)
        trans_key = EIS_IRKEY_RED;
    if(EIS_IRKEY_NVOD == trans_key)
        trans_key = EIS_IRKEY_BLUE;
    if(EIS_IRKEY_SWITCH == trans_key || EIS_IRKEY_HELP == trans_key)
        return;
#endif
#ifdef Sichuan
    if(EIS_IRKEY_NVOD == trans_key)
        trans_key = EIS_IRKEY_BLUE;
#endif

#if defined(CYBERCLOUD)
	LogUserOperDebug("key=%#x, type = %d, cloudFlag = %d\n", trans_key, type, CStb_CloudKeyLoopIsEnable());

	if((type != YX_EVENT_KEYPRESS) && CStb_CloudKeyLoopIsEnable()){
		if(CStb_SendKey2CyberCloud(type, trans_key)){
			return;
		}
	}
#endif

    if(0 == sys_msg_irkey_repeat(msgno, trans_key, type, repeat)) {
        return;
    }


    if(trans_key < 0x80) {
#ifdef HUAWEI_C20
        sendMessageToEPGBrowser(MessageType_Char, trans_key, 0, 0);
        return;
#endif
    } else {
        if(app_pressureTest_refuseIrkey()) { //lh  如果启动压力 测试,认为按遥控键不响应2010-5-10
            return;
        } else {
            LogUserOperDebug("I will send 0x%x to keydispatcher.\n", trans_key);

            int tGotoMenuTimeout = 0;
			sysSettingGetInt("gotomenu", &tGotoMenuTimeout, 0);
            if(tGotoMenuTimeout > 0) {
                removeSpecifiedMessageFromKeyDispatcher(MessageType_KeyDown, EIS_IRKEY_MENU, 1);
                sendMessageToKeyDispatcher(MessageType_KeyDown, EIS_IRKEY_MENU, 1, (tGotoMenuTimeout * 60 * 1000));
            }

            int tAutoStandbtMode = 0;
            int tAutoStandbtTime = 0;
			sysSettingGetInt("AutoStandbyMode", &tAutoStandbtMode, 0);
			sysSettingGetInt("AutoStandbyTime", &tAutoStandbtTime, 0);
            if(tAutoStandbtMode && tAutoStandbtTime > 0) {
                removeSpecifiedMessageFromKeyDispatcher(MessageType_KeyDown, EIS_IRKEY_POWER, 0);
                removeSpecifiedMessageFromKeyDispatcher(MessageType_KeyDown, EIS_IRKEY_SELECT, 1);
                sendMessageToKeyDispatcher(MessageType_KeyDown, EIS_IRKEY_POWER, 0, (tAutoStandbtTime * 60 * 60 * 1000));
                sendMessageToKeyDispatcher(MessageType_KeyDown, EIS_IRKEY_SELECT, 1, ((tAutoStandbtTime * 60 * 60 + 2) * 1000));
            }

            sendMessageToKeyDispatcher(MessageType_KeyDown, trans_key, 0, 0);
        }
    }


    return ;
}

void RegisterReceivedVideoMessageCallback(int owner, VideoMessageCallback callback) //TODO sync local player message
{
    gVideoMessage.nOwner = owner;
    gVideoMessage.nCallback = callback;
}
