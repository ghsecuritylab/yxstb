/***************************************************************************
 *     Copyright (c) 2005-2009, Yuxing Software Corporation
 *     All Rights Reserved
 *     Confidential Property of Yuxing Softwate Corporation
 *
 * $ys_Workfile: YX_gfx_porting.c $
 * $ys_Revision: 01 $
 * $ys_Date: 11/26/08 14:30 $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $ys_Log: YX_input_porting.c $
 *
 * 1. create by mabaoquan  11/26/2008
 *
 * 2. Yxapi. H does not contain the event statement.
 ***************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <linux/input.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <linux/types.h>
#include <errno.h>
#include <stdint.h>
#include <stdarg.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "libzebra.h"
#include "sdk_utility.h"
#include "io_xkey.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


#define USB_IR_DEVICE_0 "/dev/input/event0"
#define USB_IR_DEVICE_1 "/dev/input/event1"
#define USB_IR_DEVICE_2 "/dev/input/event2"
static int gUsbIrFd = -1;
static int gUsbIrExist = 0; // 1 means exist, 0 is not
static unsigned int sg_reverseDword[32] = {
	0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,
	0x100, 0x200, 0x400, 0x800, 0x1000, 0x2000, 0x4000, 0x8000,
	0x10000, 0x20000, 0x40000, 0x80000, 0x100000, 0x200000, 0x400000, 0x800000,
	0x1000000, 0x2000000, 0x4000000, 0x8000000, 0x10000000, 0x20000000, 0x40000000, 0x80000000
};


#ifdef USB_KEYBOARD
#define BITS_PER_LONG        (sizeof(long) * 8)
#define NBITS(x)             ((((x)-1)/BITS_PER_LONG)+1)
#define OFF(x)               ((x)%BITS_PER_LONG)
#define LONG(x)              ((x)/BITS_PER_LONG)
#define TEST_BIT(bit, array) ((array[LONG(bit)] >> OFF(bit)) & 1)

static int usb_keyboard = -1;

static int open_usb_event_abc(void);
#endif

static xkey_call g_key_call = NULL;

int UsbIrCheckAndOpen(void);


#ifdef USB_KEYBOARD
static int open_usb_event_abc(void)
{
    if (usb_keyboard < 0) { /* get device name */
		char buf[32];
		int fd; /* get event type bits */
		unsigned int num_keys = 0;
		unsigned int num_ext_keys = 0;
		unsigned int num_buttons = 0;
		unsigned long evbit[NBITS(EV_MAX)];
		unsigned long keybit[NBITS(KEY_MAX)];

		fd = open("/dev/input/event0", O_RDWR);
		if (fd < 0) {
			printe("Open /dev/input/event0: %s\n", strerror(errno));
			return -1;
		}
		ioctl(fd, EVIOCGNAME(31), buf);
		ioctl(fd, EVIOCGBIT(0, EV_MAX), evbit);
		if (TEST_BIT(EV_KEY, evbit)){
			int i;

			ioctl(fd, EVIOCGBIT(EV_KEY, KEY_MAX), keybit);	/* get keyboard bits */
			for(i = 16; i < 50; i++) /**  count typical keyboard keys only */
				if(TEST_BIT(i, keybit))
					num_keys++;
			for(i = KEY_OK; i < KEY_MAX; i++)
				if(TEST_BIT(i, keybit))
					num_ext_keys++;
			for(i = BTN_MOUSE; i < BTN_JOYSTICK; i++)
				if(TEST_BIT(i, keybit))
					num_buttons++;
			printm("Open [%s]  as %d with %d+%d keys\n", buf, fd, num_keys, num_ext_keys);
			if (num_keys > 20){ /* It's keyboard */
				InputEvent yevent;

				usleep(100000); // Must add delay, or the keyboard is not ready to receive commands to light up the LED.
				usb_keyboard = fd;
				memset(&yevent, 0, sizeof(InputEvent));
				yhw_input_setLED(1, 0, 1); /* NumLock & ScrollLock Plug in lights */
				yhw_board_systemEvent( USBKEYBOARD_INSERT ); // send system information
				close(fd);
			}
		}
		END(0);
	}
	END(0);
}
#endif

/******************************************************************************************
Description:  get input irTpye   for  sham standby of  jiangsu SD
Date:
Author:
Input: none
Output: none
Return: if Usb  Ir  Exist then return 1,otherwise return 0;
Remarks: none
*******************************************************************************************/
int io_input_IRtype(void)
{
    return gUsbIrExist;
}

/***********************************************************************************************
Description: copied from old codes, from guangdong.
Date:
Author:
Input: none
Output: none
Return: none
Remarks: none
***********************************************************************************************/
static unsigned int reverse_dword( unsigned int value )
{
    int l_result = 0;
    int i,counter;
    
    for (counter = 0, i = 31; i >= 0; i--, counter++){
        if (i >= 16)
            l_result |= ((value & sg_reverseDword[i]) >> (i - counter));
        l_result |= ((value & sg_reverseDword[i]) << (counter - i));
    }
    
    return l_result;
}

/***********************************************************************************************
Description: copied from old codes, from guangdong.
Date:
Author:
Input: none
Output: none
Return: none
Remarks: none
***********************************************************************************************/
static unsigned int handle_usb_ir( struct input_event pInputEvent[], int size )
{
    int i;
    unsigned int l_result = 0;
    struct input_event l_event[5];
    
    if (size < 0 || NULL == pInputEvent) {
        fprintf( stderr, "[%s,%d]:parameter error!\n", __func__, __LINE__ );
        return -1;
    }
	
    //代码补丁形式，告诉调用者目前的遥控器编码可能与标准的不一样(可以在此处打补丁)
    if (5 != size) {
        fprintf( stderr, "[%s,%d]:please check the remote nec code!\n", __func__, __LINE__ );
        return -1;
    }
	
    for (i = size -1; i >= 0; i--) {
        l_event[i] = pInputEvent[i];
        
        switch (l_event[i].code) {
        case 40:
            l_result |= ((l_event[i].value & 0xff)<<24);
            break;
        case 41:
            l_result |= ((l_event[i].value & 0xff)<<16);
            break;
        case 42:
            l_result |= ((l_event[i].value & 0xff)<<8);
            break;
        case 43:
            l_result |= (l_event[i].value & 0xff);
            break;
        case 0:
        default:
            break;
        }
    }
	
    l_result = reverse_dword(l_result);
	
    return l_result;	
}

/*********************************************************************
 * io_xkey_loop
 *
 * Descript:
 *   Get news button loop.
 *
 * Input Parameters:
 *
 * Return Value:
 ****************************************************************/
void io_xkey_loop(void)
{
    InputEvent event;
    unsigned long keystatus = 0, usbIrStatus = 0;
    struct input_event rcv_key_info[5];
    int readBytes;
    unsigned int handled = 0;
    int i;
    char usbIrCheckCount = 0;

#ifdef USB_KEYBOARD
	open_usb_event_abc();
#endif

    while (1) {

        if (3 == usbIrCheckCount) { // prevent open and close device in frequence
            usbIrCheckCount = 0;
            gUsbIrExist = UsbIrCheckAndOpen();
        }
        usbIrCheckCount++;

        if (1 == gUsbIrExist) {
            //yhw_input_IRtype_set(gUsbIrExist);
            memset(&rcv_key_info, 0, 5*sizeof(struct input_event));
            readBytes = read(gUsbIrFd, rcv_key_info, 5*sizeof(struct input_event));
            if ((-1 != readBytes) && (0 != readBytes)) {
                // printf("Here the read readBytes = [%d], size InputEvent = [%d], size input_event = [%d].\n", readBytes, sizeof(InputEvent), 5*sizeof(struct input_event));
                // for (i = 0; i < 5; i++)
                    // printf("Here is type= [%x], code= [%x], value= [%x].\n",rcv_key_info[i].type, rcv_key_info[i].code, rcv_key_info[i].value);
                handled = handle_usb_ir( rcv_key_info, 5 );
                // printf("the handled key is [%x].\n", handled);
                if (0 != handled) { // when on simplex click , we can receive the data twice, the second handled key value is 0.
                    usbIrStatus = 0xffffff & handled;
                    g_key_call(handled, YX_EVENT_KEYDOWN, (rcv_key_info[0].type << 24) + usbIrStatus);
                }
            }
        }
        
        if (yhw_input_getEvent(&event, 50) == 0 && g_key_call) {
            // printf("the status is [%x].\n", (rcv_key_info[0].type << 24) + usbIrStatus);
            if ((event.device == YX_DEVICE_NEC_IR) && (1 == gUsbIrExist)) // prevent the frontpanel infrared send the same key again
                continue;
            keystatus = (event.device << 24) + (event.status << 16) + event.scancode; // device 0x10 nec_ir
            g_key_call(event.keyvalue, event.eventkind, keystatus);
            // printf("InputEvent keyvalue = [%x], eventkind = [%x], keystatus = [%x].\n", event.keyvalue, event.eventkind, keystatus);
        }
    }
    return;
}

/*********************************************************************
 * io_xkey_reg
 *
 * Descript:
 *   Registration button response back off function.
 *
 * Input Parameters:
 * Return Value:
 ****************************************************************/

void io_xkey_reg(xkey_call call)
{
    g_key_call = call;
}

/***********************************************************************************************
Description: Check the usb infrared is exist or not, if it is then open it.
Date:
Author:
Input: none
Output: none
Return: 1 on exist, 0 on not.
Remarks: 具体逻辑参考“E:\source\Experiment\Trunk\Doc\usb红外文档.txt”文件
***********************************************************************************************/
int UsbIrCheckAndOpen(void)
{
    int ret = -1;
    char devname[64];
    
    if (-1 != gUsbIrFd) {
        close(gUsbIrFd);
        gUsbIrFd = -1;
    }

    ret = access(USB_IR_DEVICE_0, F_OK);   
    if (ret) {  //检查设备0不存在
        ret = access(USB_IR_DEVICE_1, F_OK);
        if (ret) { //检查设备1不存在
            ret = access(USB_IR_DEVICE_2, F_OK);
            if (ret) { //检查设备2不存在
                return 0;
            } else { //设备2存在
                gUsbIrFd = open(USB_IR_DEVICE_2, O_RDONLY | O_NONBLOCK);
                ioctl(gUsbIrFd, EVIOCGNAME(63), devname);
                if (strstr(devname, "Infrared")) { // 设备2存在且是红外：返回1
                    return 1;
                } else {  // 设备2存在且非红外：返回0
                    close(gUsbIrFd);
                    gUsbIrFd = -1;
                    return 0;
                }
            }
        } else { //设备1存在
            gUsbIrFd = open(USB_IR_DEVICE_1, O_RDONLY | O_NONBLOCK);
            ioctl(gUsbIrFd, EVIOCGNAME(63), devname);
            if (strstr(devname, "Infrared")) { //设备1存在且是红外：返回1
                return 1;
            } else { //设备1不存在且非红外
                close(gUsbIrFd);
                gUsbIrFd = -1;
                return 0;
            }
        }
    } else { //检查设备0存在
        gUsbIrFd = open(USB_IR_DEVICE_0, O_RDONLY | O_NONBLOCK);
        ioctl(gUsbIrFd, EVIOCGNAME(63), devname);
        if (strstr(devname, "Infrared")) { //设备0存在且是红外：返回1
            return 1;
        } else { //设备0存在且非红外：检查设备1是否存在
            close(gUsbIrFd);
            ret = access(USB_IR_DEVICE_1, F_OK);
            if (ret) { //设备1不存在
                return 0;
            } else { //设备1存在
                gUsbIrFd = open(USB_IR_DEVICE_1, O_RDONLY | O_NONBLOCK);
                ioctl(gUsbIrFd, EVIOCGNAME(63), devname);
                if (strstr(devname, "Infrared")) { //设备1存在且是红外：返回1
                    return 1;
                } else { //设备1存在且非红外：检查设备2是否存在
                    close(gUsbIrFd);
                    ret = access(USB_IR_DEVICE_2, F_OK);
                    if (ret) { //设备2不存在
                        return 0;
                    } else { //设备2存在
                        gUsbIrFd = open(USB_IR_DEVICE_2, O_RDONLY | O_NONBLOCK);
                        ioctl(gUsbIrFd, EVIOCGNAME(63), devname);
                        if (strstr(devname, "Infrared")) { //设备2存在且是红外：返回1
                            return 1;
                        } else { //设备2存在且非红外
                            close(gUsbIrFd);
                            gUsbIrFd = -1;
                            return 0;
                        }
                    }
                }
            }
        }
    }
}


