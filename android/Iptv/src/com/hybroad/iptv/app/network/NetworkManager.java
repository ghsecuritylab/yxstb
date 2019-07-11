package com.hybroad.iptv.app.network;

import java.net.InetAddress;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

import com.hybroad.iptv.log.SLog;
import com.hybroad.iptv.param.NetworkParam;
import com.hybroad.iptv.param.ParamManager;
import com.hybroad.iptv.param.ParameterInterface;
import com.hybroad.iptv.app.IPTVMiddleWare;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.NetworkInfo;
import android.net.ConnectivityManager;
import android.net.LinkAddress;
import android.net.LinkProperties;
import android.net.ethernet.EthernetManager;
import android.net.DhcpInfo;
import android.net.DhcpInfoInternal;
import android.net.NetworkUtils;
import android.net.RouteInfo;
import android.net.NetworkInfo.State;
import android.net.wifi.ScanResult;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.net.wifi.WifiConfiguration.IpAssignment;
import android.os.IBinder;
import android.os.INetworkManagementService;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.provider.Settings;
import android.text.TextUtils;
import android.text.format.Formatter;


public class NetworkManager {
     
    private native void nativeNetworkInit();
    private final static String TAG = "IPTVMiddlewareNET";
	private static NetworkManager mNetworkInstance = null;
	private NetworkStateReceiver mNetworkStateReceiver = null;
	private ConnectivityManager mConnectivityManager = null; 

	public static _IptvParamUtils mNetworkParams; //TODO old.
        
    private NetworkManager() 
    {
        SLog.i(TAG, "Network Manager contruct.");
        mNetworkParams = new _IptvParamUtils(); //TODO old.
        nativeNetworkInit();
        addNetworkListener();        
    }

	public static synchronized NetworkManager getNetworkManager() 
    {
        if (mNetworkInstance == null) 
            mNetworkInstance = new NetworkManager();
        return mNetworkInstance;
    }	
	

    /* 
     * TODO not finish!!
    private Map<String, NetworkParams> mMap = new HashMap<String, NetworkParams>();
    public class NetworkParams {
        public String mConnectType;
        public String mProtocolType;
        public String mAddressType;
        public String mAddress;
        public String mNetmask;
        public String mGateway;
    }
    private void doRefreshEthernet(NetworkParams np, EthernetManager em)
    {       

        np.mConnectType = "ethernet";
        
        String mode = em.getEthernetMode();
        if(mode.equals(EthernetManager.ETHERNET_CONNECT_MODE_PPPOE)) {
            np.mProtocolType = "pppoe";
        }
        if(mode.equals(EthernetManager.ETHERNET_CONNECT_MODE_DHCP)) {
            np.mProtocolType = "dhcp";
        }
        if(mode.equals(EthernetManager.ETHERNET_CONNECT_MODE_MANUAL)) {
            np.mProtocolType = "static";
        }
        if (mode.equals(EthernetManager.ETHERNET_CONNECT_MODE_IPOE)) {
            np.mProtocolType = "ipoe";
        }        

    }
    
    private void doRefreshWireless(NetworkParams np)
    {
        np.mConnectType = "wireless";
    }

    
    private boolean activeNetworkRefresh(Context context) 
    {
        mNetworkParams.setContext(context); //TODO 临时先使用以前的, 太多疑点且受到底层接口的限制.
        mNetworkParams.refreshNetWorkInfo();
        IPTVMiddleWare.getInstance().notify("NETWORK_CHANGED:" + mNetworkParams.getIptvInterface());

        return false;
       
        ConnectivityManager cm = (ConnectivityManager)context.getSystemService(Context.CONNECTIVITY_SERVICE);
        NetworkInfo info = cm.getActiveNetworkInfo(); 
        
        if (info.getState() == NetworkInfo.State.DISCONNECTED) {
            SLog.i(TAG, "Network connected.");
            return false;
        }
       
        String ifname = cm.getLogicNetworkIface(ConnectivityManager.IPTV_IFACE); // TODO 临时使用, 底层目前没有找到提供网络变动的所有接口名. 
        if (!mMap.containsKey(ifname))        
            mMap.put(ifname, new NetworkParams());       
        NetworkParams np = mMap.get(ifname);
        
        switch (info.getType()) {
            case ConnectivityManager.TYPE_ETHERNET:
                doRefreshEthernet(np, (EthernetManager)context.getSystemService(Context.ETHERNET_SERVICE));
                break;
            case ConnectivityManager.TYPE_WIFI:
                doRefreshWireless(np);
                break;
            default:
                return false;
        }
        IPTVMiddleWare.getInstance().notify("NETWORK_CHANGED:" + ifname);
        return true;
        
    }
     */
    
    private void addNetworkListener()
    {
        NetworkStateListener listener = new NetworkStateListener()
        {
            public boolean stateChanged(Context context, Intent intent)
            {
                String action = intent.getAction();
                if (!action.equals(ConnectivityManager.CONNECTIVITY_ACTION))
                    return false;
                return activeNetworkRefresh(context);
            }
        };
        NetworkStateReceiver.addListener(listener);            
    }
    
    public void registerBroadcastReceiver(Activity activity)
    {
        SLog.i(TAG, "registerBroadcastReceiver");
        if (mNetworkStateReceiver == null) {
            IntentFilter filter = new IntentFilter();
            filter.addAction(ConnectivityManager.CONNECTIVITY_ACTION);
            mNetworkStateReceiver = new NetworkStateReceiver();
            activity.registerReceiver(mNetworkStateReceiver, filter);
        }
    }
    
    public void unregisterBroadcastReceiver(Activity activity)
    {
        SLog.i(TAG, "unregisterBroadcastReceiver");
        if (mNetworkStateReceiver != null) {
            activity.unregisterReceiver(mNetworkStateReceiver);
            mNetworkStateReceiver = null;
        }
    }
    
    

    
    private boolean activeNetworkRefresh(Context context) 
    {
        mNetworkParams.setContext(context); //TODO 临时先使用以前的, 太多疑点且受到底层接口的限制.
        mNetworkParams.refreshNetWorkInfo();
        IPTVMiddleWare.getInstance().notify("NETWORK_CHANGED:" + mNetworkParams.getIptvInterface());

        return false;
    }
    
   public static String getNetworkInfo(String iface, String name)
   {
       SLog.i(TAG, "Iface:" + iface + " Name:" + name);


       //TODO old.
       if (!iface.equals(mNetworkParams.getIptvInterface()))
           return null;

       if (name.equals("connectType")) {
           String connectType = mNetworkParams.getIptvInterfaceNetworkType();
           if ("1".equals(connectType)){
               return "wired";
           }else{
               return "wireless";
           }
       }
       else if (name.equals("protocolType")) {
           String protocolType = mNetworkParams.getIptvInterfaceProtocolType();
           if ("1".equals(protocolType)){
               return "pppoe";
           }
           else if ("2".equals(protocolType)){
               return "dhcp";
           }
           else if ("3".equals(protocolType))
               return "static";
       }
       else if (name.equals("addressType")) {
           return "v4";
       }
       else if (name.equals("address")) {
           return mNetworkParams.getIptvInterfaceAddress();
       }
       else if (name.equals("netmask")) {
           return mNetworkParams.getIptvInterfaceNewMask();
       }
       else if (name.equals("gateway")) {
           return mNetworkParams.getIptvInterfaceGateWay();
       }
       return null;
   }

   
   // TODO copy from fuyuxiang  com.hybroad.iptv.utils: IptvParamUtils. I think, need modify or rewrite, most importantly need time.
    public class _IptvParamUtils {

        private Context mContext = null;
        private ConnectivityManager mConnectivityManager;
        private String mCurrentIp = null;
        private String mCurrentNetMask = null;
        private String mCurrentGateWay = null;
        private String mWifiSSID = null;

        public _IptvParamUtils(){
            //mContext = context;
            //mConnectivityManager = (ConnectivityManager) mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
        }

        public void setContext(Context context)
        {
            mContext = context;
            mConnectivityManager = (ConnectivityManager) mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
        }
        
        public String getIptvInterface(){

            String iptvInterface = mConnectivityManager.getLogicNetworkIface(ConnectivityManager.IPTV_IFACE);

            //SLog.i(TAG, "getIptvInterface = " + iptvInterface);
            return iptvInterface;
        }

        public String getIptvInterfaceNetworkType(){

            String netType = getNetType();

            //SLog.i(TAG, "getIptvInterfaceNetworkType = " + netType);
            return netType;
        }

        public String getIptvInterfaceProtocolType(){
            String connectType = getConnectType();

            SLog.i(TAG, "getIptvInterfaceProtocolType = " + connectType);
            return connectType;
        }

        public String getIptvInterfaceAddress(){

            refreshNetWorkInfo();
            SLog.i(TAG, "getIptvInterfaceAddress = " + mCurrentIp);
            return mCurrentIp;
        }

        public String getIptvInterfaceNewMask(){

            refreshNetWorkInfo();
            SLog.i(TAG, "getIptvInterfaceNewMask = " + mCurrentNetMask);
            return mCurrentNetMask;
        }

        public String getIptvInterfaceGateWay(){

            refreshNetWorkInfo();
            SLog.i(TAG, "getIptvInterfaceGateWay = " + mCurrentGateWay);
            return mCurrentGateWay;
        }

        public String getIptvMacAddress(){

            String iptvMacAddress = getMac();

            SLog.i(TAG, "getIptvMacAddress = " + iptvMacAddress);
            return iptvMacAddress;
        }

        public String getIptvTokenMacAddress() {
            String mac = getEthernetMac();
            SLog.i(TAG, "getIptvTokenMacAddress = " + mac);
            return mac;
        }

        public String getDhcpUser(){
            //dhcp no User...
            return null;
        }

        public String getDhcpPasswd(){
           //dhcp no Passwd...
           return null;
        }

        private String getNetType(){

            boolean isEthConnected = false;
            boolean isWifiConnected = false;

            EthernetManager mEthManager = (EthernetManager) mContext.getSystemService(Context.ETHERNET_SERVICE);
            NetworkInfo ethInfo = mConnectivityManager.getNetworkInfo(ConnectivityManager.TYPE_ETHERNET);
            if (mEthManager.getEthernetState() == EthernetManager.ETHERNET_STATE_ENABLED && ethInfo != null && ethInfo.isAvailable() && ethInfo.isConnected()
                    && ethInfo.getState() == State.CONNECTED) {
                String mode = mEthManager.getEthernetMode();

                if (EthernetManager.ETHERNET_CONNECT_MODE_MANUAL.equals(mode) || EthernetManager.ETHERNET_CONNECT_MODE_DHCP.equals(mode)
                        || EthernetManager.ETHERNET_CONNECT_MODE_PPPOE.equals(mode) || EthernetManager.ETHERNET_CONNECT_MODE_IPOE.equals(mode)) {
                    isEthConnected = true;
                } else {
                    isEthConnected = false;
                }
            }

            WifiManager mWifiManager = (WifiManager) mContext.getSystemService(Context.WIFI_SERVICE);
            NetworkInfo wifiInfo = mConnectivityManager.getNetworkInfo(ConnectivityManager.TYPE_WIFI);
            if (mWifiManager.getWifiState() == WifiManager.WIFI_STATE_ENABLED && wifiInfo != null && wifiInfo.isAvailable() && wifiInfo.isConnected()
                    && wifiInfo.getState() == State.CONNECTED) {
                isWifiConnected = true;
            } else {
                isWifiConnected = false;
            }

            if(true == isEthConnected && false == isWifiConnected){
                return "0";
            }
            if(false == isEthConnected && true == isWifiConnected){
                return "1";
            }
            if(true == isEthConnected && true == isWifiConnected){
                return "2";
            }
            if(false == isEthConnected && false == isWifiConnected){
                return "3";
            }

            return null;
        }

        private String getConnectType(){

            String netType = getNetType();

            if ("0".equals(netType)){
                EthernetManager mEthManager = (EthernetManager) mContext.getSystemService(Context.ETHERNET_SERVICE);
                NetworkInfo ethInfo = mConnectivityManager.getNetworkInfo(ConnectivityManager.TYPE_ETHERNET);

                if (mEthManager.getEthernetState() == EthernetManager.ETHERNET_STATE_ENABLED && ethInfo != null && ethInfo.isAvailable() && ethInfo.isConnected()
                        && ethInfo.getState() == State.CONNECTED) {

                    String mode = mEthManager.getEthernetMode();

                    SLog.d(TAG, "ConnectType = " + mode);

                    if(EthernetManager.ETHERNET_CONNECT_MODE_PPPOE.equals(mode)){
                        return "1";
                    }
                    if(EthernetManager.ETHERNET_CONNECT_MODE_DHCP.equals(mode)){
                        return "2";
                    }
                    if(EthernetManager.ETHERNET_CONNECT_MODE_MANUAL.equals(mode)){
                        return "3";
                    }
                    if (EthernetManager.ETHERNET_CONNECT_MODE_IPOE.equals(mode)) {
                        return "4";
                    }
                }
            }else if ("1".equals(netType)){

                WifiConfiguration mConfiguration = refreshWifiStatus();

                if (mConfiguration != null) {
                    if (mConfiguration.ipAssignment == IpAssignment.DHCP || mConfiguration.ipAssignment == IpAssignment.UNASSIGNED) {
                        return "2";
                    } else if (mConfiguration.ipAssignment == IpAssignment.STATIC) {
                        return "3";
                    } else if (mConfiguration.ipAssignment == IpAssignment.PPPOE) {
                        return "1";
                    } else if (mConfiguration.ipAssignment == IpAssignment.IPOE) {
                        return "4";
                    }
                }
            }
            return null;
        }

        private WifiConfiguration refreshWifiStatus() {
            WifiManager mWifiManager = (WifiManager) mContext.getSystemService(Context.WIFI_SERVICE);
            WifiInfo wifiInfo = mWifiManager.getConnectionInfo();
            if (wifiInfo == null) {
                return null;
            }

            List<WifiConfiguration> configList = mWifiManager.getConfiguredNetworks();
            if (configList == null) {
                return null;
            }

            String ssid = wifiInfo.getSSID();
            if (TextUtils.isEmpty(ssid)) {
                return null;
            }

            mWifiSSID = ssid;
            WifiConfiguration config = null;
            int len = configList.size();
            for (int i = 0; i < len; i++) {
                if (ssid.equals(configList.get(i).SSID)) {
                    config = configList.get(i);
                    break;
                }
            }

            return config;
        }

        private String getMac(){

            ConnectivityManager connMgr = (ConnectivityManager)mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
            NetworkInfo ethInfo = connMgr.getNetworkInfo(ConnectivityManager.TYPE_ETHERNET);
            NetworkInfo wifiInfo = connMgr.getNetworkInfo(ConnectivityManager.TYPE_WIFI);
            WifiManager wifiMgr = (WifiManager)mContext.getSystemService(Context.WIFI_SERVICE);

            if (ethInfo != null && ethInfo.isAvailable() && ethInfo.isConnected() && ethInfo.getState() == State.CONNECTED) {
                return getEthernetMac();
            } else if (wifiMgr != null && wifiMgr.getWifiState() == WifiManager.WIFI_STATE_ENABLED && wifiInfo != null && wifiInfo.isAvailable()
                    && wifiInfo.getState() == State.CONNECTED) {
                return getWirelessMac();
            } else {
                return getEthernetMac();
            }
        }

        private String getWirelessMac(){

            String MAC = null;
            ConnectivityManager connMgr = (ConnectivityManager)mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
            NetworkInfo mWifiNetworkInfo = connMgr.getNetworkInfo(ConnectivityManager.TYPE_WIFI);
            WifiManager mWifiManager = (WifiManager)mContext.getSystemService(Context.WIFI_SERVICE);

            if (mWifiNetworkInfo.isAvailable() && mWifiManager.isWifiEnabled()) {
                MAC = mWifiManager.getConnectionInfo().getMacAddress();
            }

            return MAC;
        }

        private String getEthernetMac(){

            String MAC = null;
            IBinder b = ServiceManager.getService(Context.NETWORKMANAGEMENT_SERVICE);
            INetworkManagementService mNwService = INetworkManagementService.Stub.asInterface(b);
            EthernetManager mEthManager = (EthernetManager) mContext.getSystemService(Context.ETHERNET_SERVICE);
            String ethInterface;ethInterface = mEthManager.getDatabaseInterfaceName().toString();

            try {
                try {
                    MAC = mNwService.getInterfaceConfig(ethInterface).getHardwareAddress().toString();
                    return MAC;
                } catch (NullPointerException e) {
                    SLog.e(TAG, "NullPointerException  " + e);
                }
            } catch (RemoteException remote) {
                SLog.e(TAG, "RemoteException  " + remote);
            }
            return MAC;
        }


        private void getEtherNetStaticAddress() {
            DhcpInfoInternal mDhcpInfoInternal = new DhcpInfoInternal();
            EthernetManager mEthManager = (EthernetManager) mContext.getSystemService(Context.ETHERNET_SERVICE);
            mDhcpInfoInternal.getFromDhcpInfo(mEthManager.getSavedEthernetIpInfo());

            String IP = mDhcpInfoInternal.ipAddress;
            String mGW = mDhcpInfoInternal.getRoutes().toString();
            String[] arrGW = mGW.split("\\[|\\]| ");
            int prefixLength = mDhcpInfoInternal.prefixLength;
            int NetmaskInt = NetworkUtils.prefixLengthToNetmaskInt(prefixLength);
            InetAddress Netmask = NetworkUtils.intToInetAddress(NetmaskInt);
            String mNM = Netmask.toString();
            String[] arrNM = mNM.split("\\/");

            mCurrentIp = IP;
            mCurrentNetMask = arrNM[1];
            mCurrentGateWay = arrGW[3];
        }

        private void getEtherNetDhcpAddress() {
            try {
                IBinder b = ServiceManager.getService(Context.NETWORKMANAGEMENT_SERVICE);
                INetworkManagementService mNwService = INetworkManagementService.Stub.asInterface(b);
                int prefixLength = mNwService.getInterfaceConfig("eth0").getLinkAddress().getNetworkPrefixLength();
                int NetmaskInt = NetworkUtils.prefixLengthToNetmaskInt(prefixLength);
                InetAddress Netmask = NetworkUtils.intToInetAddress(NetmaskInt);
                String NM = Netmask.toString();
                String[] arrNM = NM.split("\\/");
                mCurrentNetMask = arrNM[1];
            } catch (RemoteException remote) {
                SLog.e(TAG, "RemoteException  " + remote);
            }

            try {
                String tmpIp = mConnectivityManager.getLinkProperties(ConnectivityManager.TYPE_ETHERNET).getAddresses()
                        .toString();
                String[] arrIP = tmpIp.split("/|\\[|\\]| ");
                if (arrIP.length != 0) {
                    String ip = arrIP[2].toString();
                    mCurrentIp = ip;
                }
                String tmpGetway = mConnectivityManager.getLinkProperties(ConnectivityManager.TYPE_ETHERNET).getRoutes()
                        .toString();
                String[] arrGW = tmpGetway.split("\\[|\\]| ");
                if (arrGW.length >= 3) {
                    String getway = arrGW[3].toString();
                    mCurrentGateWay = getway;
                }

            } catch (NullPointerException e) {
                SLog.e(TAG, "can not get IP" + e);
            }
        }

        private void getEtherNetPppoeAddress() {
            try {
                IBinder b = ServiceManager.getService(Context.NETWORKMANAGEMENT_SERVICE);
                INetworkManagementService mNwService = INetworkManagementService.Stub.asInterface(b);
                int prefixLength = mNwService.getInterfaceConfig("ppp0").getLinkAddress().getNetworkPrefixLength();
                int NetmaskInt = NetworkUtils.prefixLengthToNetmaskInt(prefixLength);
                InetAddress netmask = NetworkUtils.intToInetAddress(NetmaskInt);
                if (netmask != null) {
                    String[] nmArray = netmask.toString().split("\\/");
                    if (nmArray != null && nmArray.length >= 2) {
                        mCurrentNetMask = nmArray[1];
                    } else {
                        mCurrentNetMask = null;
                    }
                } else {
                    mCurrentNetMask = null;
                }
            } catch (RemoteException remote) {
                SLog.e(TAG, "RemoteException  " + remote);
            }

            try {
                String ipTemp = mConnectivityManager.getLinkProperties(ConnectivityManager.TYPE_ETHERNET).getAddresses()
                        .toString();
                String[] ipArray = ipTemp.split("/|\\[|\\]| ");
                if (ipArray != null && ipArray.length >= 3) {
                    mCurrentIp = ipArray[2];
                }
                EthernetManager mEthManager = (EthernetManager) mContext.getSystemService(Context.ETHERNET_SERVICE);
                String getway = mEthManager.getGateway("ppp0");
                if (!TextUtils.isEmpty(getway)) {
                    mCurrentGateWay = getway;
                }

            } catch (NullPointerException e) {
                SLog.e(TAG, "can not get ip" + e);
            }

        }

        private void getWifiStaticAddress(){

            int prefixLength = -1;
            String currentIp = "";
            String currentNm = "";
            String currentGw = "";
            WifiConfiguration mInitConfig = refreshWifiStatus();
            if (mInitConfig == null){
                return ;
            }
            LinkProperties linkProperties = mInitConfig.linkProperties;
            Iterator<LinkAddress> iterator = linkProperties.getLinkAddresses().iterator();

            if (iterator.hasNext()) {
                LinkAddress linkAddress = iterator.next();
                currentIp = linkAddress.getAddress().getHostAddress();
                prefixLength = linkAddress.getNetworkPrefixLength();
                int NetmaskInt = NetworkUtils.prefixLengthToNetmaskInt(prefixLength);
                InetAddress Netmask = NetworkUtils.intToInetAddress(NetmaskInt);
                String netmask = Netmask.toString();
                if (!TextUtils.isEmpty(netmask)) {
                    String[] netmaskArray = netmask.split("\\/");
                    if (netmaskArray != null && netmaskArray.length >= 2) {
                        currentNm = netmaskArray[1];
                    }
                }
            }

            for (RouteInfo route : linkProperties.getRoutes()) {
                if (route.isDefaultRoute()) {
                    currentGw = route.getGateway().getHostAddress();
                    break;
                }
            }

            mCurrentIp = currentIp;
            mCurrentNetMask = currentNm;
            mCurrentGateWay = currentGw;
        }

        private void getWifiDhcpAddress() {

            WifiManager mWifiManager = ((WifiManager) mContext.getSystemService(Context.WIFI_SERVICE));
            DhcpInfo localDhcpInfo = mWifiManager.getDhcpInfo();
            String ip = Formatter.formatIpAddress(localDhcpInfo.ipAddress);
            String netmask = Formatter.formatIpAddress(localDhcpInfo.netmask);
            String gateway = Formatter.formatIpAddress(localDhcpInfo.gateway);

            mCurrentIp = ip;
            mCurrentNetMask = netmask;
            mCurrentGateWay = gateway;
        }

        private void refreshNetWorkInfo(){

            String netType = getNetType();
            String connedtType = getConnectType();

            if ("0".equals(netType)){
                if ("1".equals(connedtType)){
                    getEtherNetPppoeAddress();
                }else if ("2".equals(connedtType) || "4".equals(connedtType)){
                    getEtherNetDhcpAddress();
                }else if ("3".equals(connedtType)){
                    getEtherNetStaticAddress();
                }
            }else if ("1".equals(netType)){
                if ("1".equals(connedtType)){
                    getWifiDhcpAddress();
                }else if ("2".equals(connedtType) || "4".equals(connedtType)){
                    getWifiDhcpAddress();
                }else if ("3".equals(connedtType)){
                    getWifiStaticAddress();
                }

            }

        }
    }
}
