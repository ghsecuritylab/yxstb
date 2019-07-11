#if defined(BROWSER_INDEPENDENCE) || defined(ANDROID)

#include "SystemManager.h"
#include "Message.h"
#include "MessageTypes.h"
#include "MessageValueSystem.h"
#include "KeyDispatcher.h"
#include "BrowserAgent.h"

#include "BrowserAssertions.h"
#include "TAKIN_mid_porting.h"
#include "TAKIN_browser.h"

#include "libzebra.h"

#include "Assertions.h"
#if defined(ANDROID)
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/ashmem.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#define CONTROLLEN (sizeof(struct cmsghdr) + sizeof(int))
#define JVM_ASHMEM_SERVER_SOCK "/var/jvm_ashmem_server_sock"
#define JVM_ASHMEM_CLIENT_SOCK "/var/jvm_ashmem_client_sock"
#endif

extern "C" {

static int JVMLayer = 0;

//����JVMͼ�㣬ͼ�㲻�ɼ�
//������
//  width: ͼ����
//  height: ͼ��߶�
//  trueColor: 0ΪRGB565ģʽ����0ΪARGB8888ģʽ
//  framebuffer: ����ͼ��framebuffer�������ַ
//  pitch: ͼ��ÿ������ʵ��ռ�õ�������
//����ֵ: 0Ϊ�ɹ�����0Ϊʧ��
#if defined(ANDROID)
static int gCreateInit = 0;
static int gShmSize = 0;
static int gFrameBuffer = 0;
static char* gMapAddr = 0;
int JVMSendAshmemFd(int fd, int fdToSend)
{   
    struct cmsghdr* cmptr = NULL;
    struct iovec iov[1];  
    struct msghdr msg;  
    char buf[2];  
  
    iov[0].iov_base = buf;  
    iov[0].iov_len = 2;  
    msg.msg_iov = iov;  
    msg.msg_iovlen = 1;  
    msg.msg_name = NULL;  
    msg.msg_namelen = 0;  
  
    if (fdToSend <= 0) {
        BROWSER_LOG_ERROR("fdToSend <= 0\n");  
        return -1;  
    }  

    cmptr = (struct cmsghdr*)malloc(CONTROLLEN);
    if (cmptr == NULL)  {
        BROWSER_LOG_ERROR("malloc fault\n");
        return -1;  
    }

    cmptr->cmsg_level = SOL_SOCKET;  
    cmptr->cmsg_type = SCM_RIGHTS;  
    cmptr->cmsg_len = CONTROLLEN;  
    msg.msg_control = cmptr;  
    msg.msg_controllen = CONTROLLEN;  
    *(int*)CMSG_DATA(cmptr) = fdToSend;  
    buf[1] = 0;  
    buf[0] = 0;  
    if (sendmsg(fd, &msg, 0) != 2) {
        BROWSER_LOG_ERROR("sendmsg fault\n");
        return -1;  
    }
    return 0;  
}  

int JVMConnectToAshmemServer(const char *name)
{
    int fd, len, err, rval;
    struct sockaddr_un  un;

    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
        return -1;

    /* fill socket address structure with out address */
    memset(&un, 0, sizeof(un));
    un.sun_family = AF_UNIX;
    strcpy(un.sun_path, "/var/jvm_ashmem_client");
    len = offsetof(struct sockaddr_un, sun_path) + strlen(un.sun_path);

    unlink(un.sun_path);

    if (bind(fd, (struct sockaddr *)&un, len) < 0) {
        rval = -2;
        goto errout;
    }

    /* fill socet address structure with server's address */
    memset(&un, 0, sizeof(un));
    un.sun_family = AF_UNIX;
    strcpy(un.sun_path, name);
    len = offsetof(struct sockaddr_un, sun_path) + strlen(un.sun_path);
   
    if (connect(fd, (struct sockaddr *)&un, len) < 0) {
        rval = -4;
        goto errout;
    }
    return fd;

errout:
    BROWSER_LOG_ERROR("error==================\n");
    err = errno;
    close(fd);
    errno = err;
    return rval;
}
#endif

int JVMCreateLayer(int width, int height, int trueColor, unsigned int* framebuffer, unsigned int* pitch)
{
    int colorType;

    if (trueColor)
        colorType = YX_COLOR_ARGB8888;
    else
        colorType = YX_COLOR_RGB565;
    if (ygp_layer_createGraphics(width, height, colorType, &JVMLayer))
        return -1;

    int x, y, w, h;
    ygp_layer_getDisplayPosition(Hippo::systemManager().mixer().middleLayer()->mPlatformLayer, &x, &y ,&w, &h);

    ygp_layer_setDisplayPosition(JVMLayer, x, y, w, h);
    ygp_layer_setZorder(JVMLayer, 108);
    //ygp_layer_setShow(JVMLayer, 1);

#if !defined(ANDROID)
    ygp_layer_getDeviceMemory(JVMLayer, (int *)framebuffer, (int *)pitch);
#else
    ygp_layer_getMemory(JVMLayer, (int*)&framebuffer, (int*)pitch);
    if (gCreateInit)
        return 0;
    gCreateInit = 1;

    gFrameBuffer = (int)framebuffer;
    int fd, fdToSend;

    fd = JVMConnectToAshmemServer(JVM_ASHMEM_SERVER_SOCK);
    if (fd < 0)
        BROWSER_LOG_ERROR("client error:\n");

    fdToSend = open("/dev/ashmem", O_RDWR);
    
    gShmSize = height*(*pitch);
    ioctl(fdToSend, ASHMEM_SET_SIZE, gShmSize);
    gMapAddr = (char*)mmap(NULL, gShmSize, PROT_READ | PROT_WRITE , MAP_SHARED, fdToSend, 0); 
    *framebuffer = 0;
    JVMSendAshmemFd(fd, fdToSend); 
#endif
    BROWSER_LOG("Create Layer Ok\n");
    return 0;
}

//����JVMͼ��
//������
//  framebuffer: ����ͼ��framebuffer�������ַ
//����ֵ: 0Ϊ�ɹ�����0Ϊʧ��
int JVMDestroyLayer(unsigned framebuffer)
{
    if (JVMLayer) {
        ygp_layer_destroyGraphics(JVMLayer);
        ygp_layer_updateScreen();
#if defined(ANDROID)
        if (gCreateInit) 
            munmap((void*)gMapAddr, gShmSize);
        gCreateInit = 0;
#endif
    }
    JVMLayer = 0;
    return 0;
}

//ˢ��
void JVMUpdateScreen(void)
{
#if defined(ANDROID)
    memcpy((void*)gFrameBuffer, gMapAddr, gShmSize);
#endif
    ygp_layer_updateScreen();
    return ;
}

//����JVMͼ���Ƿ�ɼ�
//����
//visible: 0Ϊ���ɼ�����0Ϊ�ɼ�
void JVMLayerSetVisible(int visible)
{
    ygp_layer_setShow(JVMLayer, visible);
    //ygp_layer_redraw(1);
    ygp_layer_updateScreen();
}

//���м�����¼�
//˵����JVM�������˳�ʱ��Ҫ����yhw_input_sendEvent���м�������ⰴ���¼�
//������
//  eventkind����ӦYX_INPUTEVENT.eventkind
//  keyValue����ӦYX_INPUTEVENT.keyvalue
void JVMNotifyEvent(int eventType, int keyValue)
{
#if 0
    YX_INPUTEVENT jvm_event;
    memset(&jvm_event, 0, sizeof(YX_INPUTEVENT));
    jvm_event.eventkind = (YX_EventType)eventType;
    jvm_event.keyvalue = keyValue;
    yhw_input_sendEvent(jvm_event);
#else
    sendMessageToEPGBrowser(MessageType_JVM, eventType, 0, 0);
#endif
}

void JVMInit()
{
}

}

#endif
