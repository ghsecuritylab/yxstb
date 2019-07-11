#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

extern "C" void JVMNotifyEvent(int eventType, int eventValue);
extern "C" int JVMCreateLayer(int width, int height, int trueColor, unsigned int* frameBuffer, unsigned int* pitch);
extern "C" int JVMDestroyLayer(unsigned frameBuffer);
extern "C" void JVMLayerSetVisible(int visible);
extern "C" void JVMUpdateScreen(void);

#define JVM_CALL_COMMAND	"/var/g_jvm_command"
#define JVM_CALL_RESPONSE	"/var/g_jvm_response"

static int JVMCallCommand = -1;
static int JVMCallResponse = -1;

static pthread_t JVMCallService;

static void* JVMCallServiceEntry(void* arg)
{
    printf("JVMCallService start ...\n");

    if (mknod(JVM_CALL_COMMAND, S_IFIFO | 0666, 0) != 0)
        ;
    if (mknod(JVM_CALL_RESPONSE, S_IFIFO | 0666, 0) != 0)
        ;

    JVMCallCommand = open(JVM_CALL_COMMAND, 0666);
    if (JVMCallCommand < 0)
        ;
    JVMCallResponse = open(JVM_CALL_RESPONSE, 0666);
    if (JVMCallResponse < 0)
        ;

    printf("JVMCallService fd %d %d ...\n", JVMCallCommand, JVMCallResponse);

    while (1) {
        unsigned int command[12];
        unsigned int response[8];
        int len;

        len = read(JVMCallCommand, command, sizeof(command));
        if (len < 0) {
            break;
        }

        //printf("JVMCallService cmd %d ...\n", command[0]);

        switch (command[0]) {
            case 1: {
                JVMNotifyEvent(command[1], command[2]);
                sleep(1);
                response[0] = 0;
                write(JVMCallResponse, response, 4);
            }
            break;

            case 2: {
                unsigned int fb, pitch;
                int ret = JVMCreateLayer(command[1], command[2], command[3], &fb, &pitch);
                response[0] = ret;
                response[1] = fb;
                response[2] = pitch;
                write(JVMCallResponse, response, 12);
            }
            break;

            case 3: {
                int ret = JVMDestroyLayer(command[1]);
                response[0] = ret;
                write(JVMCallResponse, response, 4);
            }
            break;

            case 4: {
                JVMLayerSetVisible(command[1]);
                response[0] = 0;
                write(JVMCallResponse, response, 4);
            }
            break;

            case 5: {
                JVMUpdateScreen();
                response[0] = 0;
                write(JVMCallResponse, response, 4);
            }
            break;

            default:
                break;
        }
    }

    printf("JVMCallService exit ...\n");

    return 0;
}

int createJVMCallService()
{
    pthread_create(&JVMCallService, 0, JVMCallServiceEntry, 0);
    return 0;
}
