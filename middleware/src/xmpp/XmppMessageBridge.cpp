#include "XmppMessageBridge.h"
#include "XmppService.h"

#include "BrowserAgent.h"

namespace gloox {

XmppMessageBridge::XmppMessageBridge(XmppService *xmppService)
    : m_xmppSevice(xmppService)
{
}

XmppMessageBridge::~XmppMessageBridge()
{
}

void 
XmppMessageBridge::handleMessage(Hippo::Message *msg)
{
    switch (msg->what) {
    case XMPP_MESSAGE_CONNECT:
        onStartConnect();
        break;
    case XMPP_MESSAGE_SERVER_PING:
        onServerPing();
        break;
    case XMPP_MESSAGE_REDIRECT_URL:
        onRedirectionURLRequest();
        break;
    default:
        break;
    }
}

void
XmppMessageBridge::onStartConnect()
{
    m_xmppSevice->startTask();
}

void 
XmppMessageBridge::onServerPing()
{
    m_xmppSevice->serverPing();
}

void 
XmppMessageBridge::translateMessage(int what, int delay)
{
    sendEmptyMessageDelayed(what, delay);    
}

void
XmppMessageBridge::onRedirectionURLRequest()
{
    epgBrowserAgentOpenUrl(m_redirectionURL.c_str());
}

} //namespace gloox


