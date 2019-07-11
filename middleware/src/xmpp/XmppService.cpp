#include "XmppService.h"
#include "XmppAssertions.h"
#include "XmppControlMessageParser.h"
#include "XmppMessageBridge.h"

#include "disco.h"
#include "rostermanager.h"
#include "stanza.h"
#include "nickname.h"
#include "BrowserEventQueue.h"
#include "AppSetting.h"

#include "mid_task.h"
#include "json/json_public.h"
#include "json/json_object.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "NetworkFunctions.h"

namespace gloox {

static XmppService* g_xmppService = 0;

XmppService* xmppService()
{
    return g_xmppService;
}

XmppService::XmppService()
    : m_client(0)
    , m_session(0)
    , m_messageEventFilter(0)
    , m_chatStateFilter(0)
    , m_messageBridge(0)
    , m_port(5222)
    , m_controlMessageParser(0)
    , m_heartbeat(300)
    , m_heartbeatTimes(5)
    , m_pingServerTimes(0)
    , m_xmppPlayMode(XMPP_PLAY_BY_URL)
    , m_isConnecting(false)
    , m_didGetServerUrl(false)
    , m_didGetJID(false)
    , m_isStopConnect(false)
    , m_isResponsePing(true)
    , m_serviceEnable(1)
    , m_xmppAutoBinding(0)
{
    m_controlMessageParser = new XmppControlMessageParser(this);
    m_messageBridge = new XmppMessageBridge(this);
}

XmppService::~XmppService()
{
    delete(m_controlMessageParser);
	delete(m_messageBridge);
}

void
XmppService::prepareConnect(void)
{
    m_messageBridge->translateMessage(XmppMessageBridge::XMPP_MESSAGE_CONNECT, 1000);
}

void
XmppService::startTask()
{
    if (!m_isConnecting)
        mid_task_create("xmpp", (mid_func_t)xmppTaskCreate, 0);
    else
        xmppLogWarning("A xmpp connection is running.\n");
}

void
XmppService::startConnect()
{
    if (!m_serviceEnable || !m_didGetJID || !m_didGetServerUrl) {
        xmppLogError("Have not get JID or serverUrl. JID[%d], serverUrl[%d]\n", (m_didGetJID = true? 1: 0), (m_didGetServerUrl = true? 1: 0));
        return;
    }
    if (m_client)
        delete(m_client);
    char passWord[32] = {0};
    char ifname[512] = { 0 };
    network_default_ifname(ifname, 512);
    network_tokenmac_get(passWord, 32, ':');
    m_client = new Client(m_jid, passWord);
    m_client->setPort(m_port);
    m_client->setServer(m_serverUrl);
    //m_client->disableRoster(); /* 华为平台需要实现roster管理，所以开放好友功能. */
    if (m_client->rosterManager())
        m_client->rosterManager()->registerRosterListener(this);

    Nickname *nicknam = new Nickname("Hybroad");
    m_client->presence().addExtension(nicknam);

    m_client->registerConnectionListener(this);
    m_client->registerMessageSessionHandler(this, Message::Sysctrl); /* Every message type needs a sessionhandler. */
    m_client->disco()->setVersion("xmpp", GLOOX_VERSION, "Linux");
    m_client->disco()->setIdentity("client", "bot");
    m_client->disco()->addFeature(XMLNS_CHAT_STATES);
    StringList ca;
    ca.push_back("/path/to/cacert.crt");
    m_client->setCACerts(ca);

    m_client->logInstance().registerLogHandler(LogLevelDebug, LogAreaAll, this);
    m_isConnecting = true;
    if (m_client->connect(false)) {
        ConnectionError ce = ConnNoError;
        while(ce == ConnNoError) {
            ce = m_client->recv();
            if(m_isStopConnect) {
                m_isStopConnect = false;
                return ;
            }
        }
        xmppLogDebug("ce: %d\n", ce);
    }

    xmppLogDebug("Xmpp service end!\n");
    m_isConnecting = false;
    m_messageBridge->translateMessage(XmppMessageBridge::XMPP_MESSAGE_CONNECT, 2000);

}

void
XmppService::stopConnect()
{
    Tag *t = new Tag("presence", "from", m_jid.full());
    t->addAttribute("type", "unavailable");
    if (m_client)
        m_client->send(t); // "t" will be deleted auomatically by "send".

    m_isConnecting = false;
    if (m_client)
        m_client->disconnect();
}

void
XmppService::onConnect()
{
    xmppLogDebug("Congratulations! Connection successful!\n");
    m_messageBridge->translateMessage(XmppMessageBridge::XMPP_MESSAGE_SERVER_PING, m_heartbeat*1000);
}

void
XmppService::onDisconnect(ConnectionError error)
{
    xmppLogDebug("Bad news: disconnected: %d\n", error);
    if(error == ConnAuthenticationFailed)
        xmppLogDebug("auth failed. reason: %d\n", m_client->authError());
}

bool
XmppService::onTLSConnect(const CertInfo& info)
{
    time_t from(info.date_from);
    time_t to(info.date_to);

    xmppLogDebug("status: %d\nissuer: %s\npeer: %s\nprotocol: %s\nmac: %s\ncipher: %s\ncompression: %s\n"
           "from: %s\nto: %s\n",
            info.status, info.issuer.c_str(), info.server.c_str(),
            info.protocol.c_str(), info.mac.c_str(), info.cipher.c_str(),
            info.compression.c_str(), ctime(&from), ctime(&to));
    return true;
}

void
XmppService::handleMessage(const Message& message, MessageSession* messageSession)
{
    xmppLogDebug("type: %d, subject: %s, message: %s, thread id: %s\n", message.subtype(), message.subject().c_str(), message.body().c_str(), message.thread().c_str());

    m_messageEventFilter->raiseMessageEvent(MessageEventDisplayed);
    m_messageEventFilter->raiseMessageEvent(MessageEventComposing);
    m_chatStateFilter->setChatState(ChatStateComposing);

    m_controlMessageParser->startParseMessage(message);

    if(message.body() == "quit")
        m_client->disconnect();
}

void
XmppService::handleMessageEvent(const JID& jidFrom, MessageEventType event)
{
    xmppLogDebug("Received event: %d from: %s\n", event, jidFrom.full().c_str());
}

void
XmppService::handleChatState(const JID& jidFrom, ChatStateType state)
{
    xmppLogDebug("received state: %d from: %s\n", state, jidFrom.full().c_str());
}

void
XmppService::handleMessageSession(MessageSession* messageSession)
{
    xmppLogDebug("Got new session\n");
    // this example can handle only one session. so we get rid of the old session
    m_client->disposeMessageSession(m_session);
    m_session = messageSession;
    m_session->registerMessageHandler(this);
    m_messageEventFilter = new MessageEventFilter(m_session);
    m_messageEventFilter->registerMessageEventHandler(this);
    m_chatStateFilter = new ChatStateFilter(m_session);
    m_chatStateFilter->registerChatStateHandler(this);
}

void
XmppService::handleLog(gloox::LogLevel level, LogArea area, const std::string& message)
{
    if (strstr(message.c_str(), "success") && strstr(message.c_str(), "heartbeat=")) {
        const char *pointer = NULL;
        m_heartbeat = atoi(strstr(message.c_str(), "heartbeat=") + strlen("heartbeat=") + 1);
        pointer = strstr(message.c_str(), "heartbeatTimes=");
        if (pointer)
            m_heartbeatTimes = atoi(pointer + strlen("heartbeatTimes=") + 1);
        xmppLogDebug("Get heartbeat: %d, heartbeatTime: %d\n", m_heartbeat, m_heartbeatTimes);
    }

    xmppLogDebug("log: level: %d, area: %d, %s\n", level, area, message.c_str());
}

void
XmppService::handleIncomingConnection(ConnectionBase*, ConnectionBase*)
{
}

void
XmppService::setXmppServiceEnableByEPG(int flag)
{
    if (1 == flag || 0 == flag) {
        if (m_serviceEnable != flag) {
            if (m_serviceEnable) {
                stopConnect();
            } else
                startConnect();
            m_serviceEnable = flag;
        }
    }
}

void
XmppService::handleEvent(const Event& event)
{
    m_isResponsePing = true;
    xmppLogDebug("%d\n", event.eventType());
    if (Event::PingError != event.eventType()) {
        m_pingServerTimes = 0;
    } else {
        xmppLogDebug("Ping Error!\n");
        m_pingServerTimes++;
    }
}


void
XmppService::serverPing()
{
    if (!m_serviceEnable) {
        xmppLogWarning("XMPP service is disabled.\n");
        return;
	}
    if(!m_isResponsePing)
        m_pingServerTimes++;
    if (m_pingServerTimes >= m_heartbeatTimes) {
        m_pingServerTimes = 0;
        stopConnect();
        return;
    }
    m_isResponsePing = false;
    JID serverJID(m_serverUrl);
    m_client->xmppPing(serverJID, this);
    m_messageBridge->translateMessage(XmppMessageBridge::XMPP_MESSAGE_SERVER_PING, m_heartbeat*1000);
}


bool
XmppService::handleSubscriptionRequest(const JID& jid, const std::string& msg)
{
    xmppLogDebug("string: %s\n", msg.c_str());
    StringList groups;
    JID id(jid);
    if (m_client->rosterManager())
        m_client->rosterManager()->subscribe(id, "", groups, "");
    else
        return false;
    return true;
}

void
XmppService::handleItemAdded(const gloox::JID&)
{
}

void
XmppService::handleItemSubscribed(const gloox::JID&)
{
}

void
XmppService::handleItemRemoved(const gloox::JID&)
{
}

void
XmppService::handleItemUpdated(const gloox::JID&)
{
}

void
XmppService::handleItemUnsubscribed(const gloox::JID&)
{
}

void
XmppService::handleRoster(const gloox::Roster& roster)
{
    xmppLogDebug("roster arriving\nitems:\n");
    xmppLogDebug("My Phone JID :%s\n", xmppService()->m_myPhone.full().c_str());

    bool isFindMyPhone = false;

    Roster::const_iterator it = roster.begin();
    for(; it != roster.end(); ++it) {
        xmppLogDebug("jid: %s, name: %s, subscription: %d\n",
                    (*it).second->jidJID().full().c_str(), (*it).second->name().c_str(),
                    (*it).second->subscription()); // SubscriptionType
        if ((*it).second->jidJID().username() == m_myPhone.username())
            isFindMyPhone = true;

        StringList g = (*it).second->groups();

        if (S10nNone == (*it).second->subscription() || S10nTo == (*it).second->subscription() || S10nFrom == (*it).second->subscription()) {
            if (m_client->rosterManager())
                m_client->rosterManager()->subscribe((*it).second->jidJID(), "", (*it).second->groups(), "");
        }

        StringList::const_iterator it_g = g.begin();
        for(; it_g != g.end(); ++it_g)
            xmppLogDebug("\tgroup: %s\n", (*it_g).c_str());
        RosterItem::ResourceMap::const_iterator rit = (*it).second->resources().begin();
        for(; rit != (*it).second->resources().end(); ++rit)
            xmppLogDebug("resource: %s\n", (*rit).first.c_str());
    }
    if (!isFindMyPhone && m_xmppAutoBinding) {
        StringList groups;
        if (m_client->rosterManager())
            m_client->rosterManager()->add(m_myPhone, EmptyString, groups);
    }
}

void
XmppService::handleRosterPresence(const gloox::RosterItem&, const std::string&, gloox::Presence::PresenceType, const std::string&)
{
    browserEventSend("{\"type\":\"EVENT_XMPP_ROSTER_STATUS_CHANGED\"}", 0);
}

void
XmppService::handleSelfPresence(const gloox::RosterItem&, const std::string&, gloox::Presence::PresenceType, const std::string&)
{
}

bool
XmppService::handleUnsubscriptionRequest(const gloox::JID&, const std::string&)
{
    return true;
}

void
XmppService::handleNonrosterPresence(const gloox::Presence&)
{
}

void
XmppService::handleRosterError(const gloox::IQ&)
{
}

XmppMessageBridge*
XmppService::getMessageBridge()
{
    return m_messageBridge;
}

void
XmppService::addFriendToRosterByEPG(const JID &jid)
{
    if (!m_serviceEnable) {
        xmppLogWarning("XMPP service is disabled.\n");
        return;
	}
    StringList groups;
    if (m_client && m_client->rosterManager())
        m_client->rosterManager()->add(jid, EmptyString, groups);
    else
        xmppLogError("Roster is disabled, please check it!\n");
}

void
XmppService::removeFriendFromRosterByEPG(const JID &jid)
{
    if (!m_serviceEnable) {
        xmppLogWarning("XMPP service is disabled.\n");
        return;
	}
    StringList groups;
    if (m_client && m_client->rosterManager())
        m_client->rosterManager()->remove(jid);
    else
        xmppLogError("Roster is disabled, please check it!\n");
}

int
XmppService::getRosterItemCountByEPG(int &count)
{
    if (!m_serviceEnable) {
        xmppLogWarning("XMPP service is disabled.\n");
        count = 0;
        return 1;
    }
    if (m_client && m_client->rosterManager()) {
        count = m_client->rosterManager()->roster()->size();
        xmppLogDebug("Get roster acount: %d\n", count);
        return 0;
    } else {
        xmppLogError("Roster is disabled, please check it!\n");
        count = 0;
        return -1;
    }
}

std::string
XmppService::getXmppRosterItemListByEPG(struct json_object *obj)
{
    int i = 0, friendCount = 0, result = 0;
    const char *getPosition = json_get_object_string(json_object_get_object_bykey(obj, "position"));
    const char *getCount = json_get_object_string(json_object_get_object_bykey(obj, "count"));
    char res[10] = {0};

    if (!m_serviceEnable) {
        xmppLogWarning("XMPP service is disabled.\n");
        result = 1;
    }
    if (!getPosition || !getCount) {
        xmppLogError("Get string error.\n");
        json_object_put(obj);
        result = -1;
    }
    int position = atoi(getPosition);
    int count = atoi(getCount);
    obj = json_object_create_object();
    if (m_client && m_client->rosterManager()) {
        friendCount = m_client->rosterManager()->roster()->size();
        result = 0;
    } else {
        xmppLogError("Roster is disabled, please check it!\n");
        result = -1;
        sprintf(res, "%d", result);
        json_object_add_object(obj,"result",json_object_new_string(res));
        return json_object_to_json_string(obj);
    }
    struct json_object *my_array = json_object_create_array();
    struct json_object *new_obj= json_object_create_object();
    gloox::Roster *roster = m_client->rosterManager()->roster();
    Roster::const_iterator it = roster->begin();
    for(i = 0; i < position && it != roster->end(); i++) {
        ++it;
    }
    if(it == roster->end()) {
        result = -1;
    }
    for(i = 0; i < count && it != roster->end(); i++, it++) {
        std::string statusMessage;
        std::string groupName ;
        std::string show;
        std::string jid = (*it).second->jidJID().full();
        std::string name = (*it).second->name();
        std::string subscription = getSubscription((*it).second->subscription());
        StringList g = (*it).second->groups();
        StringList::const_iterator it_g = g.begin();
        for(; it_g != g.end(); ++it_g)
            groupName = (*it_g);
        RosterItem::ResourceMap::const_iterator rit = (*it).second->resources().begin();
        for(; rit != (*it).second->resources().end(); ++rit) {
            statusMessage = (*rit).second->message();
            show = getShow((*rit).second->presence());
        }
        xmppLogDebug("statusMessage = %s, show = %s\n", statusMessage.c_str(), show.c_str());
        json_object_add_object(new_obj,"jid",json_object_new_string(jid.c_str()));
        json_object_add_object(new_obj,"name",json_object_new_string(name.c_str()));
        json_object_add_object(new_obj,"subscription",json_object_new_string(subscription.c_str()));
        json_object_add_object(new_obj,"group",json_object_new_string(groupName.c_str()));
        json_object_add_object(new_obj,"status",json_object_new_string(statusMessage.c_str()));
        json_object_add_object(new_obj,"show",json_object_new_string(show.c_str()));
        json_array_add_object(my_array,new_obj);
        new_obj= json_object_create_object();
    }
    if(-1 == result || 1 == result) {
        sprintf(res, "%d", result);
        json_object_add_object(obj,"result",json_object_new_string(res));
    } else {
        sprintf(res, "%d", result);
        json_object_add_object(obj,"result",json_object_new_string(res));
        json_object_add_object(obj,"friendCount",json_object_new_int(friendCount));
        json_object_add_object(obj,"itemList",my_array);
    }
    xmppLogDebug("obj:%s\n",json_object_to_json_string(obj));
    return json_object_to_json_string(obj);
}

} // namespace gloox


extern "C" {
using namespace gloox;

void xmppServiceCreate()
{
    g_xmppService = new XmppService();
    if (!g_xmppService)
        xmppLogError("New XmppService failed!\n");
}

void xmppTaskCreate(void)
{
    if (xmppService())
        xmppService()->startConnect();
}

void xmppTaskStop(void)
{
    if (xmppService()) {
        if(!xmppService()->m_isConnecting)
            return ;
        xmppService()->m_isStopConnect = true;
        xmppService()->stopConnect();
    }
}

std::string xmppAccountEscape(std::string userName)
{
    std::string resultName;
    std::string::iterator it;
    for (it = userName.begin(); it != userName.end(); it++) {
        if ('-' == *it)
            resultName.append("%2D");
        else if ('*' == *it)
            resultName.append("%2A");
        else if (' ' == *it)
            resultName.append("%20");
        else
            resultName.push_back(*it);
    }
    return resultName;
}

std::string getSubscription(int subscription)
{
    if (7 == subscription || 6 == subscription)
        return "from";
    else if (8 == subscription)
        return "both";
    else
        return "to";
}
std::string getShow(int show)
{
    if (1 == show)
        return "chat";
    else if (2 == show)
        return "away";
    else if (3 == show)
        return "dnd";
    else if (4 == show)
        return "xa";
    else
        return "";
}

int xmppInfoHeartbitRecv(int type, char* buf, int len, int arg)
{
    const char *jid = NULL;
    const char *port = NULL;
    const char *serverUrl = NULL;
    struct json_object *obj = NULL;

    PRINTF("type = %d\n", type);
    if(buf == NULL) {
        xmppLogError("data is null!\n");
        return -1;
    }
    xmppLogDebug("%s\n", buf);
    if (!strncmp(buf, "{", 1)) {
        obj = json_tokener_parse_string(buf);
        if (obj) {
            jid = json_object_get_string(json_object_get_object_bykey(obj, "JID"));
            serverUrl = json_object_get_string(json_object_get_object_bykey(obj, "xmppUrl"));
            if (jid && serverUrl) {
                if (xmppService()) {
                    port = strchr(serverUrl, ':');
                    if (port) {
                        xmppService()->setServerUrl(serverUrl, port - serverUrl);
                        xmppService()->setPort(atoi(port + 1));
                    } else
                        xmppService()->setServerUrl(serverUrl, strlen(serverUrl));

                    xmppService()->setXmppJID(std::string(jid));
                    xmppService()->m_didGetJID = true;
                    xmppService()->m_didGetServerUrl = true;

                    char netUser[34] = {0};
                    appSettingGetString("ntvuser", netUser, 32, 0);
                    std::string phoneName = xmppAccountEscape(std::string(netUser));
                    xmppService()->m_myPhone = xmppService()->getXmppJID();
                    xmppService()->m_myPhone.setUsername(phoneName);

                    xmppService()->prepareConnect();
                    return 0;
                }
            } else {
                xmppLogError("JSON is invalid.\n");
            }
        } else
            xmppLogError("JSON string parse error.\n");
    } else
        xmppLogError("Invalid xmpp info.\n");
    return -1;
}

int isXmppInfoGet()
{
    if (xmppService() && xmppService()->m_didGetJID && xmppService()->m_didGetServerUrl)
        return 0;
    return -1;
}
}


