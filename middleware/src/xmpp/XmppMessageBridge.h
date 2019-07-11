#ifndef XmppMessageBridge_h
#define XmppMessageBridge_h
#include "MessageHandler.h"

#include <iostream>

#ifdef __cplusplus

namespace gloox {

class XmppService;

class XmppMessageBridge: public Hippo::MessageHandler {
public:
    XmppMessageBridge(XmppService*);
    ~XmppMessageBridge();

    enum XmppTranslateMessage {
        XMPP_MESSAGE_CONNECT,
        XMPP_MESSAGE_SERVER_PING,
        XMPP_MESSAGE_REDIRECT_URL,
    };
    void handleMessage(Hippo::Message*);
    void translateMessage(int, int);
    void onStartConnect();
    void onServerPing();
    void onRedirectionURLRequest();
    std::string m_redirectionURL;
private:
    XmppService *m_xmppSevice;

}; 
} //namespace gloox

#endif //__cplusplus

#endif //XmppMessageBridge_h

