#ifndef __WirelessNetworkCard_H_
#define __WirelessNetworkCard_H_

#include "NetworkCard.h"

#ifdef __cplusplus
#include <map>
#include <list>

#define NET_WIRELESS_FILE "/proc/net/wireless"

class WifiAccessPoint {
public:
    WifiAccessPoint() : mQuality(0), mEssid("") { }
    ~WifiAccessPoint() { }

    bool operator < (const WifiAccessPoint& right) const {
        return right.mQuality < mQuality;
    }

    void setChannel(int channel) { mChannel = channel; }
    int getChannel() { return mChannel; }

    void setQuality(int quality) { mQuality = quality; }
    int getQuality() { return mQuality; }

    void setEssid(const char* essid) { mEssid = essid; }
    const char* getEssid() { return mEssid.c_str(); }

    void setBssid(const char* bssid) { mBssid = bssid; }
    const char* getBssid() { return mBssid.c_str(); }

    void setAuthMode(const char* authmode) { mAuthMode = authmode; }
    const char* getAuthMode() { return mAuthMode.c_str(); }

    void setEncrType(const char* encrtype) { mEncrType = encrtype; }
    const char* getEncrType() { return mEncrType.c_str(); }

private:
    int mChannel;
    int mQuality;
    std::string mEssid;
    std::string mBssid; //mac address
    std::string mAuthMode;
    std::string mEncrType;
};

class WirelessNetworkCard : public NetworkCard {
public:
    WirelessNetworkCard(const char* devname = "rausb0");
    ~WirelessNetworkCard();

    virtual int linkStatus();
    virtual int linkChange(int type, char* data, int size);
    virtual int flagChange(int flag);
    virtual int getLinkSpeed();

    int findAccessPoints();
    int joinAccessPoint(const char* essid, const char* password);
    int joinConnStatus();
    int getAccessPoints(std::list<WifiAccessPoint>& aps);
    int getSignalQuality(int* quality, int* level, int* noise);

    bool reJoinAP();
private:
    int _EraseTailSpace(char* str, int len);
    int _ParserSurveyRT73();
    int _ParserSurveyRT307();
    int _ParserSurveyDefault();

    int _CommandSetAuthMode(const char* authmode);
    int _CommandSetEncrType(const char* encrtype);

    bool mJoinedOk;
    std::string mAPEssid;
    std::string mAPPsswd;
    std::map<std::string, WifiAccessPoint*> mWifiAPs;
};

#endif

#endif
