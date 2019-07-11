
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>

extern "C" void JVMNotifyEventBrowserThread(int eventType, int eventValue);

#define JVM_CALL_COMMAND	"/var/g_jvm_command"
#define JVM_CALL_RESPONSE	"/var/g_jvm_response"

static int JVMCallCommand = -1;
static int JVMCallResponse = -1;

extern "C" void 
JVMNotifyEvent(int eventType, int eventValue)
{
    /* 危险代码，找不到头文件暂时这样写。进入消息（末位1）在浏览器线程。退出在jvm线程。*/
    if (eventType & 0x01) {
        JVMNotifyEventBrowserThread(eventType, eventValue);
        return;
    }

    if (JVMCallCommand < 0) {
        int nonBlock;
        char buffer[32];

        JVMCallCommand = open(JVM_CALL_COMMAND, 0666);
        JVMCallResponse = open(JVM_CALL_RESPONSE, 0666);

        nonBlock = 1;
        ioctl(JVMCallResponse, FIONBIO, &nonBlock);
        while (read(JVMCallResponse, buffer, sizeof(buffer)) > 0);
        nonBlock = 0;
        ioctl(JVMCallResponse, FIONBIO, &nonBlock);
    }

    if (JVMCallCommand > 0) {
        unsigned int cmd[3];
        cmd[0] = 1; /* function number */
        cmd[1] = eventType; /* arg 1 */
        cmd[2] = eventValue; /* arg 2 */
        write(JVMCallCommand, cmd, sizeof(cmd));

        unsigned int response[8];
        if (read(JVMCallResponse, response, sizeof(response)) == (sizeof(unsigned int) * 1)) {
            int ret = response[0];
        }
    }
}

extern "C" int 
JVMCreateLayer(int width, int height, int trueColor, unsigned int* frameBuffer, unsigned int* pitch)
{
    if (JVMCallCommand < 0) {
        int nonBlock;
        char buffer[32];

        JVMCallCommand = open(JVM_CALL_COMMAND, 0666);
        JVMCallResponse = open(JVM_CALL_RESPONSE, 0666);

        nonBlock = 1;
        ioctl(JVMCallResponse, FIONBIO, &nonBlock);
        while (read(JVMCallResponse, buffer, sizeof(buffer)) > 0);
        nonBlock = 0;
        ioctl(JVMCallResponse, FIONBIO, &nonBlock);
    }

    if (JVMCallCommand > 0) {
        unsigned int cmd[4];
        cmd[0] = 2; /* function number */
        cmd[1] = width; /* arg 1 */
        cmd[2] = height; /* arg 2 */
        cmd[3] = trueColor; /* arg 3 */
        write(JVMCallCommand, cmd, sizeof(cmd));

        unsigned int response[8];
        if (read(JVMCallResponse, response, sizeof(response)) == (sizeof(unsigned int) * 3)) {
            int ret = response[0];
            *frameBuffer = response[1];
            *pitch = response[2];
            return 0;
        }
    }

    return -1;
}

extern "C" int 
JVMDestroyLayer(unsigned framebuffer)
{
    if (JVMCallCommand > 0) {
        unsigned int cmd[2];
        cmd[0] = 3; /* function number */
        cmd[1] = framebuffer; /* arg 1 */
        write(JVMCallCommand, cmd, sizeof(cmd));

        unsigned int response[8];
        if (read(JVMCallResponse, response, sizeof(response)) == (sizeof(unsigned int) * 1)) {
            int ret = response[0];
            return ret;
        }
    }

    return -1;
}

extern "C" void 
JVMLayerSetVisible(int visible)
{
    if (JVMCallCommand > 0) {
        unsigned int cmd[2];
        cmd[0] = 4; /* function number */
        cmd[1] = visible; /* arg 1 */
        write(JVMCallCommand, cmd, sizeof(cmd));

        unsigned int response[8];
        if (read(JVMCallResponse, response, sizeof(response)) == (sizeof(unsigned int) * 1)) {
            int ret = response[0];
        }
    }
}

extern "C" void 
JVMUpdateScreen(void)
{
    if (JVMCallCommand > 0) {
        unsigned int cmd[1];
        cmd[0] = 5; /* function number */
        write(JVMCallCommand, cmd, sizeof(cmd));

        unsigned int response[8];
        if (read(JVMCallResponse, response, sizeof(response)) == (sizeof(unsigned int) * 1)) {
            int ret = response[0];
        }
    }
}
