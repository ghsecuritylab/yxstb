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
#include <sys/socket.h> // add for android----before <sys/un.h>----
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

#include "io_xkey.h"
#include "libzebra.h"


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
				yhw_board_systemEvent( USBKEYBOARD_INSERTt ); // send system information
				close(fd);
			}
		}
		END(0);
	}
	END(0);
}
#endif

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
    unsigned long keystatus = 0;

#ifdef USB_KEYBOARD
	open_usb_event_abc();
#endif

    while(1) {
        if (mid_real_standbyFlag_get() == 1){
            break;
        }
        if (g_key_call && yhw_input_getEvent(&event, 50) == 0 ){
        	keystatus = (event.device << 24) + (event.status << 16) + event.scancode;
            g_key_call(event.keyvalue, event.eventkind, keystatus);
        }
    }
}

/*********************************************************************
 * io_xkey_reg
 *
 * Descript:
 *   Registration button response callback function.
 *
 * Input Parameters:
 * Return Value:
 ****************************************************************/
void io_xkey_reg(xkey_call call)
{
    g_key_call = call;
}


