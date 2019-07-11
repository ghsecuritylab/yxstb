package com.hybroad.iptv.param;

import java.net.InetAddress;
import java.util.Iterator;
import java.util.List;

import android.content.Context;
import android.net.ConnectivityManager;
import android.net.DhcpInfo;
import android.net.DhcpInfoInternal;
import android.net.LinkAddress;
import android.net.LinkProperties;
import android.net.NetworkInfo;
import android.net.NetworkUtils;
import android.net.RouteInfo;
import android.net.NetworkInfo.State;
import android.net.ethernet.EthernetManager;
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

import com.hybroad.iptv.log.SLog;
import com.hybroad.iptv.param.ParamManager;
import com.hybroad.iptv.param.ParameterInterface;

public class NetworkParam extends ParamManager {

	private static final String TAG = "NetworkParam";
	private static ConnectivityManager mConnectivityManager;
	private static Context mContext;
	private String mCurrentIp = null;
	private String mCurrentNetMask = null;
	private String mCurrentGateWay = null;
	private String mWifiSSID = null;
	
	public NetworkParam(Context paramContext) {
		mContext = paramContext;
		mConnectivityManager = (ConnectivityManager) mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
		NetworkParamInit();
		DhcpParamInit();
		PppoeParamInit();
		WifiParamInit();
	}

	private void NetworkParamInit() {
		
		regist("LinkProbeTime", func);
		regist("downloadRate", func);
		regist("StbNetName", new ParameterInterface() {
			public String getParameter(String arg0) {
				String iptvInterface = mConnectivityManager.getLogicNetworkIface(ConnectivityManager.IPTV_IFACE);

				SLog.i(TAG, "getIptvInterface = " + iptvInterface);
				return iptvInterface;
			}

			public boolean setParameter(String name, String value) {
				SLog.e(TAG, "NetworkParam, This ParamStr " + name + "is read only .\n");
				return true;
			}
		});

		regist("connecttype", new ParameterInterface() {
			public String getParameter(String arg0) {
				String connectType = getConnectType();

				SLog.i(TAG, "get IptvInterfaceProtocolType = " + connectType);
				return connectType;
			}

			public boolean setParameter(String name, String value) {
				SLog.e(TAG, "NetworkParam, This ParamStr " + name + "is read only .\n");
				return true;
			}
		});

		regist("nettype", new ParameterInterface() {
			public String getParameter(String arg0) {
				String netType = getNetType();

				SLog.i(TAG, "getIptvInterfaceNetworkType = " + netType);
			    return netType;
			}

			public boolean setParameter(String name, String value) {
				SLog.e(TAG, "NetworkParam, This ParamStr " + name + "is read only .\n");
				return true;
			}
		});

		regist("StbIP", new ParameterInterface() {
			public String getParameter(String arg0) {
				refreshNetWorkInfo();
				SLog.i(TAG, "getIptvInterfaceAddress = " + mCurrentIp);
			    return mCurrentIp;
			}

			public boolean setParameter(String name, String value) {
				SLog.e(TAG, "NetworkParam, This ParamStr " + name + "is read only .\n");
				return true;
			}
		});
	}
	
	private void DhcpParamInit() {
		regist("DHCPRetryTime", func);
		regist("DHCPRetryInterval", func);
	}

	private void PppoeParamInit() {
		regist("PADIRetryTime", func);
		regist("PADIRetryInterval", func);
		regist("LCPRetryTime", func);
		regist("LCPRetryInterval", func);
		
		regist("PppoeAccount", new ParameterInterface() {
			public String getParameter(String name) {
				String user = null;
				String mNetType = NetworkParam.getNetType();

				if ("0".equals(mNetType)){
					user = Settings.Secure.getString(mContext.getContentResolver(), Settings.Secure.PPPOE_USER_NAME);
				}else if ("1".equals(mNetType)){
					user = Settings.Hybroad.getString(mContext.getContentResolver(), Settings.Hybroad.PPPOE_WIFI_USER_NAME);
				}
				return user;
			}

			public boolean setParameter(String name, String value) {
				SLog.e(TAG, "This ParamStr " + name + "is read only .\n");
				return true;
			}
		});

		regist("PppoePasswd", new ParameterInterface() {
			public String getParameter(String arg0) {
				String pass = null;
				String mNetType = NetworkParam.getNetType();

				if ("0".equals(mNetType)){
					pass = Settings.Secure.getString(mContext.getContentResolver(), Settings.Secure.PPPOE_USER_PASS);
				}else if ("1".equals(mNetType)){
					pass = Settings.Hybroad.getStringAES(mContext.getContentResolver(), Settings.Hybroad.PPPOE_WIFI_USER_PASS);
				}
				return pass;
			}

			public boolean setParameter(String name, String value) {
				SLog.e(TAG, "This ParamStr " + name + "is read only .\n");
				return true;
			}
		});

	}

	private void WifiParamInit() {
		
		regist("channel", func);
		regist("linkSpeed", func);
		regist("wifiLevel", new ParameterInterface() {
			public String getParameter(String arg0) {
				String wifiLevel = null;

				refreshWifiStatus();
				wifiLevel =  String.valueOf(getWifiLevelFromSSID(mWifiSSID));
				SLog.d(TAG, "getWifiLevel, ssid = " + mWifiSSID + ", Level = " + wifiLevel);
				return wifiLevel;
			}

			public boolean setParameter(String name, String value) {
				SLog.e(TAG, "This ParamStr " + name + "is read only .\n");
				return true;
			}
		});

		regist("ssid", new ParameterInterface() {
			public String getParameter(String arg0) {
				refreshWifiStatus();
				SLog.i(TAG, "getIptvWifiSSID = " + mWifiSSID);
				return mWifiSSID;
			}

			public boolean setParameter(String name, String value) {
				SLog.e(TAG, "This ParamStr " + name + "is read only .\n");
				return true;
			}
		});
	}
	
	private ParameterInterface func = new ParameterInterface() {
		@Override
		public String getParameter(String name) {
			String value = Settings.Hybroad.getString(mContext.getContentResolver(), name);
			SLog.i(TAG, "Get Param: " + name + ", Value is : " + value);
			return value;
		}

		@Override
		public boolean setParameter(String name, String value) {
			Settings.Hybroad.putString(mContext.getContentResolver(), name, value);
			return true;
		}
	};
	
	public WifiConfiguration refreshWifiStatus() {
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
	
	private int getWifiLevelFromSSID(String ssid){

		WifiManager mWifiManager = (WifiManager)mContext.getSystemService(Context.WIFI_SERVICE);
		WifiInfo mWifiInfo = mWifiManager.getConnectionInfo();
        int mlevel = 0;

        List<ScanResult> results = mWifiManager.getScanResults();

        if (results != null){
            for (ScanResult result : results){
                if (mWifiInfo.getBSSID().equalsIgnoreCase(result.BSSID)){
                    mlevel = WifiManager.calculateSignalLevel(result.level, 101);
                    break;
                }
            }
        }

		return mlevel;
	}
		
	
	
	public static String getNetType(){

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

	private void refreshNetWorkInfo() {

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
