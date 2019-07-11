
#ifndef XmppControlMessageParser_h
#define XmppControlMessageParser_h

#include "json/json_object.h"

#ifdef __cplusplus

namespace gloox {

class XmppService;
class Message;

typedef enum {
    XMPP_PLAY_TYPE_CHANNEL = 1,
    XMPP_PLAY_TYPE_VOD,
    XMPP_PLAY_TYPE_TVOD,
    XMPP_PLAY_TYPE_MUSIC,
    XMPP_PLAY_TYPE_LOCAL
}XmppPlayType;

typedef enum {
    XMPP_TRICK_MODE_PLAY = 0,
    XMPP_TRICK_MODE_PAUSE,
    XMPP_TRICK_MODE_FAST,
    XMPP_TRICK_MODE_SEEK,
    XMPP_TRICK_MODE_GO_BEGINNING,
    XMPP_TRICK_MODE_GO_END,
    XMPP_TRICK_MODE_QUIT = 10
}XmppTrickPlayMode;

typedef struct XmppVolumeSetContent {
    char eventType[16];
    int newVolume;
    int oldVolume;
}XmppVolumeSetContent_t;

typedef struct XmppPlayerStateContent {
    int playerInstance;
    int trickPlayMode; //0: normal; 1: pause; 2: fast
    int fastSpeed;
    int playPostion;
    char mediaType[16]; //XmppPlayType
    int mediaCode;
    int chanKey;
    int duration;
}XmppPlayerStateContent_t;

class XmppControlMessageParser {

public:
    XmppControlMessageParser(XmppService*);
    ~XmppControlMessageParser();

    void startParseMessage(const Message&);

protected:
    void startPlayEncapsulate(struct json_object*);
    void trickPlayControlEncapsulate(struct json_object*);
    void channelListEncapsulate();
    void pictureShowEncapsulate(struct json_object*);
    void remoteControlHandler(struct json_object*);
    void setVolumeHandler(struct json_object*);
    void playerStateEncapsulate();
    void playerVolumeEncapsulate();
    void parseActionOfFunctionCall(struct json_object*);
    void parseActionOfEventInfo(struct json_object*);
    void parseActionOfGetAppState(struct json_object*);
#if 0
    void parseActionOftest(struct json_object *obj); //just for test.
#endif
private:
    XmppService *m_xmppService;
};

}


#endif //__cplusplus
#endif //XmppControlMessageParser_h

