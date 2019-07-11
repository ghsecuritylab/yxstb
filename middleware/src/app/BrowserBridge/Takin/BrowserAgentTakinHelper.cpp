
#include "BrowserAgentTakin.h"
#include "BrowserAssertions.h"

#include "Thread.h"
#include "Message.h"
#include "MessageTypes.h"
#include "BrowserView.h"
#include "SystemManager.h"

#include "mid_stream.h"

#include "browser_event.h"
#include "TAKIN_browser.h"

static int g_frameSizeUpdate = 0;
static int g_frameWidth = 0;
static int g_frameHeight = 0;

extern "C" void
TAKIN_porting_set_frame_size(TAKIN_SETTING_TYPE name, char *buffer, int buffer_len)
{
#ifdef NONE_BROWSER
#else
    int w, h;
    if (name == TAKIN_FRAME_SIZE) {
        sscanf(buffer, "%d*%d", &w, &h);
        if (g_frameWidth != w || g_frameHeight != h) {
            g_frameWidth = w;
            g_frameHeight = h;
            g_frameSizeUpdate = 1;
        }
        BROWSER_LOG("Frame width(%d) height(%d)\n", g_frameWidth, g_frameHeight);
#if !defined(BROWSER_INDEPENDENCE)
#if defined(Jiangsu) && defined(EC1308H)
        char buffer_jiangSU[16] = {"640*530"}; //与浏览器讨论, 将传入浏览器的尺寸固定,防止小于640*530页面平铺
        TAKIN_browser_setSetting(TAKIN_FRAME_SIZE, buffer_jiangSU,  16 > buffer_len ? buffer_len : 16);
#else
        TAKIN_browser_setSetting(TAKIN_FRAME_SIZE, buffer, buffer_len);
#endif
#endif
    }
#endif
}

extern "C" void
TakinSetFrameSizeUpdate(int frameSizeUpdate)
{
    g_frameSizeUpdate = frameSizeUpdate;
    return;
}

extern "C" void
TAKIN_get_frame_size(int *pFrameUpdate, int *pFrameWidth, int *pFrameHeight)
{
    *pFrameUpdate = g_frameSizeUpdate;
    *pFrameWidth = g_frameWidth;
    *pFrameHeight = g_frameHeight;
    return;
}

extern "C" int
TAKIN_porting_invalidateWindow(int* x, int* y, int* w, int* h)
{
    if (g_frameSizeUpdate) {
        Hippo::epgBrowserAgent().mView->setContentSize(g_frameWidth, g_frameHeight);
        Hippo::systemManager().mixer().middleLayer()->setCanvasValidSize(g_frameWidth, g_frameHeight, true);
        Hippo::systemManager().mixer().middleLayer()->calculateCanvasOffset();
        Hippo::systemManager().mixer().middleLayer()->setStandard(Hippo::LayerMixerDevice::Layer::closeStandard(g_frameWidth, g_frameHeight));
        Hippo::epgBrowserAgent().mView->inval(NULL);

        g_frameSizeUpdate = 0;
    }
    else {
        Hippo::Rect r;

        r.set(*x, *y, *x + *w, *y + *h);
        Hippo::epgBrowserAgent().mView->inval(&r);
    }
    return 0;
}

extern "C" int
TAKIN_browser_updateScreen(int handle)
{
#if !(defined(GRAPHIC_SINGLE_LAYER) || defined(BROWSER_INDEPENDENCE))
    if (g_frameSizeUpdate) {
        Hippo::systemManager().mixer().middleLayer()->setCanvasValidSize(g_frameWidth, g_frameHeight, true);
        Hippo::systemManager().mixer().middleLayer()->calculateCanvasOffset();
        Hippo::systemManager().mixer().setStandard(Hippo::LayerMixerDevice::Layer::closeStandard(g_frameWidth, g_frameHeight));
        g_frameSizeUpdate = 0;
    }
    else
        Hippo::epgBrowserAgent().mView->inval(NULL);
#endif
    return 0;
}

extern "C" int
TAKIN_browser_getEvent(YX_INPUTEVENT *event, int time_out)
{
    Hippo::Thread *curThread = Hippo::Thread::currentThread();
    if (!curThread)
        return -1;

    Hippo::MessageQueue *queue = curThread->getMessageQueue();

    while (true) {
        Hippo::Message *msg = queue->next(); // might block
        //if (!me.mRun) {
        //    break;
        //}
        if (msg != NULL) {
            if (msg->target == NULL) {
                // No target is a magic identifier for the quit message.
                return -1;
            }
            if (msg->what == MessageType_Timer) {
                Hippo::epgBrowserAgent().sendEmptyMessageDelayed(MessageType_Timer, 200);
            }
            else if (msg->what == MessageType_KeyDown) {
                event->eventkind = YX_EVENT_KEYDOWN;
                switch (msg->arg1) {
                    case EIS_IRKEY_UP:
                        event->vkey = VK_UP;
                        break;

                    case EIS_IRKEY_DOWN:
                        event->vkey = VK_DOWN;
                        break;

                    case EIS_IRKEY_LEFT:
                        event->vkey = VK_LEFT;
                        break;

                    case EIS_IRKEY_RIGHT:
                        event->vkey = VK_RIGHT;
                        break;

                    case EIS_IRKEY_SELECT:
                        event->vkey = VK_RETURN;
                        break;

                    default:
                    	break;
                }
                msg->recycle();
                return 0;
            }
            else
                msg->target->dispatchMessage(msg);
            msg->recycle();
        }
    }

    return -1;
}


static void statecall(int pIndex, STRM_STATE state, int rate, unsigned int magic, int callarg)
{
}

static void msgcall(int pIndex, STRM_MSG msg, int arg, unsigned int magic, int callarg)
{
}

extern "C" void
TAKIN_browser_Extension_start(const char* url)
{
	APP_TYPE type;

	if(url == NULL) {
		return ;
	}

	char *tmp_url = strdup(url);
	mid_stream_set_call(0, statecall, msgcall, (int)0);

	if(strstr(url, ".mp3") == NULL) {
		type = APP_TYPE_HTTP_PCM;
	} else {
		type = APP_TYPE_HTTP_MP3;
	}

	mid_stream_open(0, tmp_url, type, -1);
	free(tmp_url);
}

extern "C" void
TAKIN_browser_Extension_stop(const char* url)
{
	mid_stream_stop(0);
}

extern "C"
int TAKIN_porting_inputGetEvent(InputEvent* event, int timeout)
{
    return -1;
}

extern "C"
int TAKIN_porting_inputSendEvent(InputEvent event)
{
    return -1;
}
