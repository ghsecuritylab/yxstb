#ifndef XmppService_h
#define XmppService_h

#include "XmppMessageBridge.h"

#include "messagesessionhandler.h"
#include "loghandler.h"
#include "messageeventhandler.h"
#include "messagehandler.h"
#include "chatstatehandler.h"
#include "client.h"
#include "messageeventfilter.h"
#include "chatstatefilter.h"
#include "connectionlistener.h"
#include "eventhandler.h"
#include "rosterlistener.h"
#include "json/json_object.h"


#ifdef __cplusplus

namespace gloox {

class XmppControlMessageParser;

class XmppService: public MessageSessionHandler, ConnectionListener, LogHandler, MessageEventHandler, MessageHandler, ChatStateHandler, EventHandler, RosterListener {
public:
    XmppService();
    ~XmppService();

    enum XmppPlayMode {
        XMPP_PLAY_BY_EVENT,
        XMPP_PLAY_BY_URL
    };

    void startTask();
    void startConnect();
    MessageSession* getSession(void){ return m_session; }
    void setXmppJID(const std::string jid){ m_jid.setJID(jid); }
    JID getXmppJID(void){ return m_jid; };
    std::string getServerUrl(void){ return m_serverUrl; }
    void setServerUrl(const char *url, int n){ m_serverUrl.assign(url, n); }
    void setXmppServiceEnableByEPG(int flag);
    int getPort(void){ return m_port; }
    void setPort(int port){ m_port = port; }
    void setScreenPlayStartMode(int mode) { m_xmppPlayMode = mode; }
    void stopConnect();
    void prepareConnect();
    void serverPing();
    XmppMessageBridge* getMessageBridge();
    void removeFriendFromRosterByEPG(const JID&);
    void addFriendToRosterByEPG(const JID&);
    int getRosterItemCountByEPG(int &);
    std::string getXmppRosterItemListByEPG(struct json_object*);

protected:
    virtual void onConnect();
    virtual void onDisconnect(ConnectionError);
    virtual bool onTLSConnect(const CertInfo&);
    virtual void handleMessage(const Message&, MessageSession*);
    virtual void handleMessageEvent(const JID&, MessageEventType);
    virtual void handleChatState(const JID&, ChatStateType);
    virtual void handleMessageSession(MessageSession*);
    virtual void handleLog(gloox::LogLevel, LogArea, const std::string&);
    virtual void handleIncomingConnection(ConnectionBase*, ConnectionBase*);
    virtual void handleEvent(const Event&);
    virtual bool handleSubscriptionRequest(const JID&, const std::string&);
    virtual void handleItemAdded(const gloox::JID&);
    virtual void handleItemSubscribed(const gloox::JID&);
    virtual void handleItemRemoved(const gloox::JID&);
    virtual void handleItemUpdated(const gloox::JID&);
    virtual void handleItemUnsubscribed(const gloox::JID&);
    virtual void handleRoster(const gloox::Roster&);
    virtual void handleRosterPresence(const gloox::RosterItem&, const std::string&, gloox::Presence::PresenceType, const std::string&);
    virtual void handleSelfPresence(const gloox::RosterItem&, const std::string&, gloox::Presence::PresenceType, const std::string&);
    virtual bool handleUnsubscriptionRequest(const gloox::JID&, const std::string&);
    virtual void handleNonrosterPresence(const gloox::Presence&);
    virtual void handleRosterError(const gloox::IQ&);

private:
    JID m_jid;
    Client* m_client;
    MessageSession* m_session;
    MessageEventFilter* m_messageEventFilter;
    ChatStateFilter* m_chatStateFilter;
    std::string m_serverUrl;
    XmppMessageBridge *m_messageBridge;
    int m_port;
    XmppControlMessageParser* m_controlMessageParser;
    int m_heartbeat;
    int m_heartbeatTimes;
    int m_pingServerTimes;
public:
    int m_xmppPlayMode;
    JID m_myPhone;
    bool m_isConnecting;
    bool m_didGetServerUrl;
    bool m_didGetJID;
    bool m_isStopConnect;
    bool m_isResponsePing;
    int m_serviceEnable;
    int m_xmppAutoBinding;
};

XmppService* xmppService();

}// namespace gloox


#endif //__cplusplus

#ifdef __cplusplus
extern "C" {
#endif

void xmppServiceCreate();
void xmppTaskCreate();
void xmppTaskStop();
std::string xmppAccountEscape(std::string userName);
std::string getSubscription(int subscription);
std::string getShow(int show);
int xmppInfoHeartbitRecv(int type, char* buf, int len, int arg);
int isXmppInfoGet();

#ifdef __cplusplus
}
#endif

#endif //XmppService_h

