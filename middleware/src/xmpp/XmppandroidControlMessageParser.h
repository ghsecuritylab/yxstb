
#ifndef XmppandroidControlMessageParser_h
#define XmppandroidControlMessageParser_h

#include <string>

#ifdef __cplusplus

namespace gloox {

class Message;

class XmppAndroidControlMessageParser {

public:
    XmppAndroidControlMessageParser();
    ~XmppAndroidControlMessageParser();

    void startParseMessageAndroid(std::string&  str,char *result,int len);
    static XmppAndroidControlMessageParser* GetInstance(void);

protected:

    void startPlayEncapsulateAndroid(struct json_object*);
    void trickPlayControlEncapsulateAndroid(struct json_object*);
    void channelListEncapsulateAndroid();
    void pictureShowEncapsulateAndroid(struct json_object*);
    void remoteControlHandlerAndroid(struct json_object*);
    void setVolumeHandlerAndroid(struct json_object*);
    void playerStateEncapsulateAndroid(char *result,int len);
    void playerVolumeEncapsulateAndroid(char *result,int len);
    void parseActionOfFunctionCallAndroid(struct json_object*);
    void parseActionOfEventInfoAndroid(struct json_object*);
    void parseActionOfGetAppStateAndroid(struct json_object*,char *result,int len);
#if 0
    void parseActionOftest(struct json_object *obj); //just for test.
#endif
//private:
//    XmppService *m_xmppService;
};

}


#endif //__cplusplus
#endif //XmppControlMessageParser_h

