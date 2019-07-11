#include "WirelessNetworkCard.h"
#include "NetlinkStateHandler.h"
#include "NetworkTypes.h"

#include "wirelessnet.h"

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/inotify.h>

#include <linux/ethtool.h>
#include <linux/sockios.h>
#include <linux/wireless.h>
#include <linux/if_arp.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>

#ifndef IW_EV_LCP_PK_LEN
#define IW_EV_LCP_PK_LEN	(4)
#endif

extern "C" int yos_systemcall_runSystemCMD(char*, int*);

WirelessNetworkCard::WirelessNetworkCard(const char* devname) : NetworkCard(devname), mJoinedOk(false)
{
    setType(NT_WIRELESS);
    mStateHandler = new NetlinkStateHandler(this);
}

WirelessNetworkCard::~WirelessNetworkCard()
{
    if (mSockFd > 0)
        close(mSockFd);
    mSockFd = -1;

    if (mStateHandler)
        delete mStateHandler;
    mStateHandler = 0;
}

int
WirelessNetworkCard::_EraseTailSpace(char* str, int len)
{
    for (int i = len - 1; i >= 0; --i) {
        if (' ' == str[i])
            str[i] = 0;
        else
            break;
    }
    return 0;
}

int
WirelessNetworkCard::linkStatus()
{
    struct ifreq ifr;
    struct iwreq iwr;
    struct iw_statistics stat;

    bzero(&ifr, sizeof(ifr));
    strncpy(ifr.ifr_name, devname(), IFNAMSIZ);

    if (-1 == ioctl(mSockFd, SIOCGIFINDEX, &ifr)) {
        NETWORK_LOG_ERR("ioctl: %s\n", strerror(errno));
        return NL_ERR_UNKNOW;
    }

    bzero(&iwr, sizeof(iwr));
    strncpy(iwr.ifr_name, devname(), IFNAMSIZ);
    iwr.u.data.pointer = (caddr_t)&stat;
    iwr.u.data.length = sizeof(stat);
    iwr.u.data.flags = 1;

    if (-1 == ioctl(mSockFd, SIOCGIWSTATS, &iwr)) {
        NETWORK_LOG_ERR("%s is down\n", devname());
        return NL_FLG_DOWN;
    }
    return stat.qual.qual > 0 ? NL_FLG_RUNNING : NL_FLG_DOWN;
}

int
WirelessNetworkCard::flagChange(int flag)
{
    NETWORK_LOG_INFO("%s\n", NetlinkFlagStr(flag));
    if (flag & IFF_UP) {  //flag & IFF_RUNNING not need.
        if (LS_WIRELESS_UP == mLinkState)
           return 0;
        mLinkState = LS_WIRELESS_UP;
    } else {
        if (LS_WIRELESS_DOWN == mLinkState)
          return 0;
        mLinkState = LS_WIRELESS_DOWN;
    }
    return mStateHandler->handleState(mLinkState);
}

int
WirelessNetworkCard::linkChange(int type, char* data, int size)
{
    NETWORK_LOG_INFO("type : %d\n", type);
    struct iw_event* iwe = (struct iw_event*)data;
    if (IFLA_WIRELESS == type) {
        if (SIOCGIWSCAN == iwe->cmd && mJoinedOk)
            return mStateHandler->handleState(LS_WIRELESS_CHECK_SIGNAL); //monitor whether wireless signal quality is changed.
    }
    return 0;
}

int
WirelessNetworkCard::getLinkSpeed()
{
    struct iwreq iwr;
    strncpy(iwr.ifr_name, devname(), IFNAMSIZ);
    if (ioctl(mSockFd, SIOCGIWRATE, &iwr) < 0) {
        NETWORK_LOG_ERR("ioctl.SIOCGIWRATE [%s]\n", strerror(errno));
        return -1;
    }
    return iwr.u.bitrate.value;
}

int
WirelessNetworkCard::_ParserSurveyRT73()
{
    /* Channel+RSSI+SSID+Bssid+EncryType+AuthMode+NetworkType
     * (8+8+36+20+12+12+12) */
    struct _SurveyDataRT73 {
        char nChannel[8];
        char nRSSI[8];
        char nSSID[36];
        char nBSSID[20];
        char nEncrType[12];
        char nAuthMode[12];
        char nNetworkType[12];
    }tRTData;
    unsigned char* buffer = 0;
    int buflen = (sizeof(tRTData) + 1) * 64;
    buffer = (unsigned char*)calloc(buflen, 1);
    if (!buffer)
        return -1;

    int numOfCells = 0;
    struct iwreq iwr;
    bzero(&iwr, sizeof(iwr));
    strncpy(iwr.ifr_name, mDeviceName.c_str(), sizeof(iwr.ifr_name) - 1);
    iwr.u.data.pointer = (caddr_t)buffer;
    iwr.u.data.length = buflen;
    iwr.u.data.flags = 0;
    if (ioctl(mSockFd, RTPRIV_IOCTL_GSITESURVEY, &iwr) < 0) {
        free(buffer);
        return -1;
    }
    numOfCells = iwr.u.data.length / sizeof(tRTData);
    unsigned char* pData = buffer + 1 + sizeof(tRTData) + 1; //skip the first char '\n' + fix line

    WifiAccessPoint* ap = 0;
    for (int i = 0; i < numOfCells; ++i) {
        memset(&tRTData, 0, sizeof(tRTData));
        memcpy(&tRTData, pData, sizeof(tRTData));
        _EraseTailSpace(tRTData.nChannel, sizeof(tRTData.nChannel));
        _EraseTailSpace(tRTData.nRSSI, sizeof(tRTData.nRSSI));
        _EraseTailSpace(tRTData.nSSID, sizeof(tRTData.nSSID));
        _EraseTailSpace(tRTData.nBSSID, sizeof(tRTData.nBSSID));
        _EraseTailSpace(tRTData.nAuthMode, sizeof(tRTData.nAuthMode));
        _EraseTailSpace(tRTData.nEncrType, sizeof(tRTData.nEncrType));
        if (!strlen(tRTData.nSSID))
            continue;
        int quality = 0, RSSI = atoi(tRTData.nRSSI);
        if (RSSI >= -50)
            quality = 100;
        else if (RSSI >= -80)
            quality = (unsigned int)(24 + (RSSI + 80) * 2.6);
        else if (RSSI >= -90)
            quality = (unsigned int)((RSSI + 90) * 2.6);
        else
            quality = 0;
        ap = new WifiAccessPoint();
        if (ap) {
            ap->setChannel(atoi(tRTData.nChannel));
            ap->setQuality(quality);
            ap->setEssid(tRTData.nSSID);
            ap->setBssid(tRTData.nBSSID);
            ap->setAuthMode(tRTData.nAuthMode);
            ap->setEncrType(tRTData.nEncrType);
            mWifiAPs.insert(std::make_pair(tRTData.nSSID, ap));
        }
        pData = pData + sizeof(tRTData) + 1;
    }
    free(buffer);
    return 0;
}

int
WirelessNetworkCard::_ParserSurveyRT307()
{
     /* Channel+SSID+Bssid+Security+Signal+WiressMode+NetworkType
     * (4+33+20+23+9+7+3)	 */
    return -1; //TODO
}

int
WirelessNetworkCard::_ParserSurveyDefault()
{
    /* Channel+SSID+Bssid+Security+Signal+WiressMode+ExtCh+NetworkType
     * (4+33+20+23+9+7+7+3)
     * "Ch", "SSID", "BSSID", "Security", "Siganl(%)", "W-Mode", " ExtCH"," NT" */
    struct _SurveyDataRTDefault {
        char nChannel[4];
        char nSSID[33];
        char nBSSID[20];
        char nSecurity[23];
        char nSignal[9];
        char nWifiMode[7];
        char nExtCh[7];
        char nNetworkType[3];
    }tRTData;
    unsigned char* buffer = 0;
    int buflen = (sizeof(tRTData) + 1) * 64;
    buffer = (unsigned char*)calloc(buflen, 1);
    if (!buffer)
        return -1;

    int numOfCells = 0;
    struct iwreq iwr;
    bzero(&iwr, sizeof(iwr));
    strncpy(iwr.ifr_name, devname(), IFNAMSIZ);
    iwr.u.data.pointer = (caddr_t)buffer;
    iwr.u.data.length = buflen;
    iwr.u.data.flags = 0;
    if (ioctl(mSockFd, RTPRIV_IOCTL_GSITESURVEY, &iwr) < 0) {
        free(buffer);
        return -1;
    }
    numOfCells = iwr.u.data.length / sizeof(tRTData);
    unsigned char* pData = buffer + 1 + sizeof(tRTData) + 1; //skip the first char '\n' + fix line
    WifiAccessPoint* ap = 0;
    char* pEncrType = 0;
    char* pAuthMode = 0;
    NETWORK_LOG_INFO("\n\t%s\n", buffer);
    for (int i = 0; i < numOfCells; ++i) {
        memset(&tRTData, 0, sizeof(tRTData));
        memcpy(&tRTData, pData, sizeof(tRTData));
        _EraseTailSpace(tRTData.nChannel, sizeof(tRTData.nChannel));
        _EraseTailSpace(tRTData.nSSID, sizeof(tRTData.nSSID));
        _EraseTailSpace(tRTData.nBSSID, sizeof(tRTData.nBSSID));
        _EraseTailSpace(tRTData.nSecurity, sizeof(tRTData.nSecurity));
        _EraseTailSpace(tRTData.nSignal, sizeof(tRTData.nSignal));
        _EraseTailSpace(tRTData.nNetworkType, sizeof(tRTData.nNetworkType));
        pAuthMode = tRTData.nSecurity;
        pEncrType = strchr(pAuthMode, '/');
        if (pEncrType && strlen(tRTData.nSSID)) {
            *pEncrType++ = 0;
            ap = new WifiAccessPoint();
            if (ap) {
                ap->setChannel(atoi(tRTData.nChannel));
                ap->setEssid(tRTData.nSSID);
                ap->setBssid(tRTData.nBSSID);
                ap->setQuality(atoi(tRTData.nSignal));
                ap->setAuthMode(pAuthMode);
                ap->setEncrType(pEncrType);
                mWifiAPs.insert(std::make_pair(tRTData.nSSID, ap));
            }
        }
        pData = pData + sizeof(tRTData) + 1; //must run here
    }
    free(buffer);
    return 0;
}

int
WirelessNetworkCard::findAccessPoints()
{
    std::map<std::string, WifiAccessPoint*>::iterator iter;
    for (iter = mWifiAPs.begin(); iter != mWifiAPs.end(); ++iter)
        delete (iter->second);
    mWifiAPs.clear();

    struct iwreq iwr;
    char drivname[32] = { 0 };
    bzero(&iwr, sizeof(iwr));
    strncpy(iwr.ifr_name, devname(), IFNAMSIZ);
    iwr.u.data.pointer = (caddr_t)drivname;
    iwr.u.data.flags = RT_OID_DEVICE_NAME;

    if (ioctl(mSockFd, RT_PRIV_IOCTL, &iwr) < 0) {
        NETWORK_LOG_ERR("ioctl.RT_PRIV_IOCTL %s\n", strerror(errno));
        return -1;
    }

    NETWORK_LOG_INFO("wireless driver name: %s\n", drivname);
    if (!strncmp(drivname, "RT73", 4))
        _ParserSurveyRT73();
    else if (!strncmp(drivname, "RT307", 5))
        _ParserSurveyRT307();
    else
        _ParserSurveyDefault();
    return 0;
}

int
WirelessNetworkCard::_CommandSetAuthMode(const char* authmode)
{
    int ret = 0;
    char command[128] = { 0 };

    if (strstr(authmode, "WPAPSK") || strstr(authmode, "WPA-PSK"))
        snprintf(command, 127, "iwpriv %s set AuthMode=WPAPSK", devname());
    else if (strstr(authmode, "WPA1PSKWPA2PSK") || strstr(authmode, "WPAPSKWPA2PSK"))
        snprintf(command, 127, "iwpriv %s set AuthMode=WPA2PSK", devname());
    else if (strstr(authmode, "OPEN"))
        snprintf(command, 127, "iwpriv %s set AuthMode=WEPAUTO", devname());
    else
        snprintf(command, 127, "iwpriv %s set AuthMode=WPA2PSK", devname()); //default

    yos_systemcall_runSystemCMD(command, &ret);
    if (0 != ret) {
        NETWORK_LOG_ERR("run cmd: %s\n", command);
        return -1;
    }
    return 0;
}

int
WirelessNetworkCard::_CommandSetEncrType(const char* encrtype)
{
    int ret = 0;
    char command[128] = { 0 };

    if (strstr(encrtype, "AES"))
        snprintf(command, 127, "iwpriv %s set EncrypType=AES", devname());
    else if (strstr(encrtype, "TKIP"))
        snprintf(command, 127, "iwpriv %s set EncrypType=TKIP", devname());
    else if (strstr(encrtype, "NONE"))
        snprintf(command, 127, "iwpriv %s set EncrypType=NOEN", devname());
    else if (strstr(encrtype, "WEP"))
        snprintf(command, 127, "iwpriv %s set EncrypType=WEP", devname());
    else
        snprintf(command, 127, "iwpriv %s set EncrypType=AES", devname());

    yos_systemcall_runSystemCMD(command, &ret);
    if (0 != ret) {
        NETWORK_LOG_ERR("run cmd: %s\n", command);
        return -1;
    }
    return 0;
}

bool
WirelessNetworkCard::reJoinAP()
{
    NETWORK_LOG_INFO("mAPEssid.c_str: %s\n", mAPEssid.c_str());
    int ret = -1;
    if (mJoinedOk)
        ret = joinAccessPoint(mAPEssid.c_str(), mAPPsswd.c_str());
    return ret < 0 ? false : true;
}

int
WirelessNetworkCard::joinAccessPoint(const char* essid, const char* password)
{
    NETWORK_LOG_INFO("essid: %s password: %s\n", essid, password);

    int ret = 0;
    int status = 0;
    const int trycnt = 10;
    char* buffer = 0;
    char tCmdBuffer[256] = { 0 };

    mJoinedOk = false;
    WifiAccessPoint* ap = mWifiAPs[essid];
    if (!ap || !essid)
        goto End;

    struct iwreq iwr;
    bzero(&iwr, sizeof(iwr));
    strncpy(iwr.ifr_name, devname(), IFNAMSIZ);
    iwr.u.essid.flags = 0;
    iwr.u.essid.pointer = (caddr_t)tCmdBuffer;
    iwr.u.essid.length = 0;
    ioctl(mSockFd, SIOCSIWESSID, &iwr); //iwconfig iface essid off

    //wait for wifi drive up, can use usleep(800000) instead, but must 800ms at least.
    buffer = (char*)calloc(8192, 1);
    if (!buffer)
        goto End;
    for (int i = 0; i < trycnt; ++i) {
        iwr.u.data.pointer = (caddr_t)buffer;
        iwr.u.data.length = 8192;
        iwr.u.data.flags = 0;
        if (ioctl(mSockFd, RTPRIV_IOCTL_GSITESURVEY, &iwr) < 0)
            continue;
        if (strstr(buffer, essid))
            break;
        usleep(200000);
    }
    free(buffer);

    _CommandSetAuthMode(ap->getAuthMode());
    _CommandSetEncrType(ap->getEncrType());

    if (strstr(ap->getEncrType(), "WEP")) { //I hate it really.
        for (int i = 1; !status && i <= 4; ++i) {
            snprintf(tCmdBuffer, 255, "iwconfig %s key [%d]; iwconfig %s key s:%s; iwpriv %s set SSID=\"%s\"", devname(), i, devname(), password, devname(), essid);
            yos_systemcall_runSystemCMD(tCmdBuffer, &ret);
            if (0 != ret) {
                NETWORK_LOG_ERR("run cmd: %s\n", tCmdBuffer);
                goto End;
            }
            for (int j = 0; j < trycnt; ++j) {
                status = joinConnStatus();
                if (status) {
                    mJoinedOk = true;
                    goto End;
                }
                usleep(200000);
            }
        }
    } else {
        snprintf(tCmdBuffer, 255, "iwpriv %s set WPAPSK=%s; iwpriv %s set SSID=\"%s\"", devname(), password, devname(), essid);
        yos_systemcall_runSystemCMD(tCmdBuffer, &ret);
        if (0 != ret) {
            NETWORK_LOG_ERR("run cmd: %s\n", tCmdBuffer);
            goto End;
        }
        for (int j = 0; j < trycnt; ++j) {
            status = joinConnStatus();
            if (status) {
                mJoinedOk = true;
                goto End;
            }
            usleep(200000);
        }
    }

End:
    if (!mJoinedOk) {
        mStateHandler->handleState(LS_WIRELESS_JOIN_FAIL);
        return -1;
    }
    mAPEssid = essid;
    mAPPsswd = password;
    mStateHandler->handleState(LS_WIRELESS_JOIN_SECCESS);
    return 0;
}

int
WirelessNetworkCard::joinConnStatus()
{
    int ret = 0, filesize = 0, num = 0;
    FILE* fp = 0;
    const char* filepath = "/var/_connstatus.txt";
    char command[128] = { 0 };
    char* ptext = 0;

    snprintf(command, 127, "iwpriv %s connStatus > %s", devname(), filepath);
    yos_systemcall_runSystemCMD(command, &ret);
    if (0 != ret) {
        NETWORK_LOG_WARN("iwpriv %s connStatus > %s error\n", devname(), filepath);
        return 0;
    }

    if (0 == (fp = fopen(filepath, "rb"))) {
        NETWORK_LOG_WARN("fopen %s: %s\n", filepath, strerror(errno));
        return 0;
    }
    fseek(fp, 0, SEEK_END);
    filesize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (0 == (ptext = (char*)calloc(filesize + 1, 1))) {
        NETWORK_LOG_ERR("calloc buffer error !\n");
        fclose(fp);
        return 0;
    }

    ret = 0;
    while (ret < filesize) {
        num = fread(ptext + ret, 1, filesize - ret, fp);
        if (num <= 0)
            break;
        ret += num;
    }
    fclose(fp);
    unlink(filepath);

    NETWORK_LOG_INFO("%s", ptext);
    ret = 0;
    if (!strcasestr(ptext, "Disconnected"))
        ret = 1;
    free(ptext);
    return ret;
}

int
WirelessNetworkCard::getAccessPoints(std::list<WifiAccessPoint>& aps)
{
    std::map<std::string, WifiAccessPoint*>::iterator iter;
    for (iter = mWifiAPs.begin(); iter != mWifiAPs.end(); ++iter)
        aps.push_back(*(iter->second));
    return 0;
}

int
WirelessNetworkCard::getSignalQuality(int* quality, int* level, int* noise)
{
    struct iwreq iwr;
    struct iw_statistics stat;

    bzero(&stat, sizeof(iw_statistics));
    bzero(&iwr, sizeof(iwr));
    strncpy(iwr.ifr_name, devname(), IFNAMSIZ);
    iwr.u.data.pointer = (caddr_t)&stat;
    iwr.u.data.length = sizeof(stat);
    iwr.u.data.flags = 1;

    if (-1 == ioctl(mSockFd, SIOCGIWSTATS, &iwr)) {
        NETWORK_LOG_ERR("%s is down\n", devname());
        return -1;
    }

    if (quality)
        *quality = stat.qual.qual;
    if (level)
        *level = stat.qual.level;
    if (noise)
        *noise = stat.qual.noise;
    return stat.qual.qual;
}
