#ifndef _NativeHandlerPublic_H_
#define _NativeHandlerPublic_H_

#include "NativeHandler.h"
#include "NetworkFunctions.h"

#include <Hippo_HString.h>

#ifdef __cplusplus

namespace Hippo {

class Dialog;

class NativeHandlerPublic : public NativeHandler::Callback {
public:
	typedef enum {
        NICStatus_on,
        NICStatus_off
    }NIC_Status;

    enum {
	/* network error */
        ERROR_NETWORK_DISCONNECTED = 102001,
        ERROR_NETWORK_IPCONFLICT   = 102003,
        ERROR_NETWORK_UNREACHABLE_DHCP_NORESPOND  = 102004,
        ERROR_NETWORK_UNREACHABLE_PPPOE_TIMEOUT   = 102005,
        ERROR_NETWORK_UNREACHABLE_PPPOE_DISCONNECTED  = 102006,
        ERROR_NETWORK_PPPOE_ACCOUNT_OR_PASSWORD_ERROR = 102007,
        ERROR_NETWORK_NTP_SYNCHRONIZE_FAILED = 102011,
        ERROR_NETWORK_NTP_SERVER_DNS_CONNECET_FAILED = 102013,
        ERROR_NETWORK_NTP_SERVER_DNS_RESOLVED_FAILED = 102014,
        ERROR_NETWORK_UNREACHABLE_DHCP_ADDRESS_GET_FAILED = 102015
    };

	NativeHandlerPublic();
	~NativeHandlerPublic();

	void InitActiveObject() {};

	virtual bool handleMessage(Message *msg);

    virtual NativeHandler::State state() = 0;
    virtual void onActive() = 0;
    virtual void onUnactive() = 0;

	virtual bool doPowerOff();
    static Dialog *mProgressBarDialog;

protected:
	virtual bool onPowerOff();
    virtual bool doPowerOn() { return false; }
    virtual bool onDLNADialog(){ return false; }
    virtual bool onUpgrade();
    virtual bool onWatchLoggedUsers(int arg2);
    virtual bool onDebug(int);

    virtual bool doMenu() = 0;
    virtual bool doShortCut(Message *msg);
    // open local webpage message
	virtual bool doLocalPage(Message *msg);
	virtual bool doOpenMenuPage();
	virtual bool doOpenStandbyPage() = 0;
	virtual bool doOpenTransparentPage();
	virtual bool doOpenBootPage();
	virtual bool doOpenConfigPage();
	virtual bool doOpenLoginPage(Message *msg);
	virtual bool doOpenTimeoutPage() = 0;
	virtual bool doOpenErrorPage();
	virtual bool doOpenUnzipRunningPage();
	virtual bool doOpenUnzipErrorPage();
#if defined(Gansu)
	virtual bool doUdiskUnzipErrReboot();
#endif
	virtual bool doOpenDVBPage();
    virtual bool doModifyPPPoEAccount();
    virtual bool doModifyPPPoEPwd();
    virtual bool doOpenCheckPPPoEAccountPage();
    // heart bit message
	virtual bool doHeartBit();
    // ppv message
    virtual bool doPPVChannelSync();
	virtual bool doPPVChannelError();
	virtual bool doPPVEnd() { return true; }
    // media message
	virtual bool doChannelListError();
	virtual bool doReminderListSync();
	virtual bool doLocalTS(Message *msg);
	// specific IRkey message
	virtual bool doUpdateGolbalKeyTable();
	virtual bool doConfig(Message *msg) = 0;
	virtual bool doIME(Message *msg);
	virtual bool doClear(Message *msg);
	virtual	bool doSwitch();
	virtual bool doDeskTop();
    // upgrade message
	virtual bool doOpenUpgradePage();
    // Android message
    virtual bool doAndroidOpenUrl(Message* msg);
    virtual bool doAndroidSetSurface(int surface);
    virtual bool doAndroidStop();
    virtual bool doAndroidRestart();
    // tr069/TMS message
#if defined(INCLUDE_TR069) || defined(INCLUDE_HMWMGMT)
	virtual bool doTr069Start();
	virtual bool doTr069Stop();
#endif
	virtual bool doUpgradeTms();
	virtual bool doUpgradeAuto();
	virtual bool doRequestReboot();
#if defined(INCLUDE_TR069) || defined(INCLUDE_HMWMGMT)
	virtual bool doUpgradeTr069BootRequest();
	virtual bool doUpgradeTr069Request();
#endif
	virtual bool doTr069NetConnectOk();
	virtual bool doTr069NetConnectError();
    // EPG auth
	virtual bool doEpgAuth();
    // usb plugin message
	virtual bool doUSBInsert(Message *msg);
	virtual bool doUSBUninsert(Message *msg);
	virtual bool doUSBConfig(Message *msg);
	virtual bool doUSBUpgrade(Message *msg);
#if defined(Huawei_v5)
    //HDMI message
    virtual bool doHdmiInsert(int);
    virtual bool doHdmiUnInsert(int);
    virtual bool doHdmiConnect(int);
    virtual bool doHdmiDisconnect(int);
#endif
    // NTP sync message
	virtual bool doNtpSyncOk() = 0;
	virtual bool doNtpSyncError() = 0;
	virtual bool doDNSServerNotfound() = 0;
	virtual bool doDNSResolveError() = 0;

    virtual bool doNetworkReconnect(int force, const char* devname);
    virtual bool doNetworkErrorDisplay(int errcode, const char* errmsg);
	virtual bool doNetworkProtocolUp(int errcode, const char* devname);
	virtual bool doNetworkProtocolDown(int errcode, const char* devname);
    virtual bool doNetworkUnlinkMonitor(int errcode, const char* devname);
    virtual bool doNetworkWifiJoinFail(int errcode, const char* devname);
    virtual bool doNetworkWifiJoinSuccess(int arg, const char* devname);
    virtual bool doNetworkWifiCheckSignal(int arg, const char* devname);
	virtual bool doNetworkProtocolConflict(int errcode, const char* ifname);
    virtual bool doNetworkConnectOk(int errcode, const char* ifname);
	virtual bool doNetworkDhcpError(int errcode, const char* ifname);
	virtual bool doNetworkPppoeError(int errcode, const char* ifname);
    
    virtual bool doNetworkRefresh();
	virtual bool doNetworkDNSTimeout(int);
	virtual bool doNetworkDNSError(int);
#ifdef Liaoning
       virtual bool doBack();
#endif

#if defined(PAY_SHELL)
    virtual bool doPayShellError(int);
#endif
	int buildEvent( std::string json_type,
	                int instance_id,
	                int newstate,
                    int newspeed,
                    int oldstate,
                    int oldspeed,
                    int errcode,
                    std::string errmsg,
                    std::string mediaCode,
                    std::string entryID);

	friend class Dialog;
	static Dialog *mDialog;
	bool m_isConnect;
	int mErrorCodeOfConnect;

    //Network Mask operator
    enum _NETWORK_MASK_BITS {
        NMB_SHOW_BOTTOM_ICON  = 0x1,
        NMB_SHOW_ERROR_CODE   = 0x2,
        NMB_SEND_WEBPAGE_KEY  = 0x4,
        NMB_REJOIN_WIRELESS   = 0x8,
        NMB_CONNECTOK_DOMENU  = 0x16,
    };
    void DONT_SET(int bit);
    void DONT_CLR(int bit);
    void DONT_ZERO();
    bool DONT_ISSET(int bit);
    int mDoNetworkMask;
private:
	NIC_Status mNICStatus;
	bool mWaitUpgradeUrl;
    bool mPPPoEAccountIsModify;
    bool mPPPoEPwdIsModify;
};

} // namespace Hippo

#endif // __cplusplus

#endif // _NativeHandlerPublic_H_;
