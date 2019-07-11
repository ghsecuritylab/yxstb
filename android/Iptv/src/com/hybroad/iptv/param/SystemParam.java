package com.hybroad.iptv.param;

import android.content.Context;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.NetworkInfo.State;
import android.net.ethernet.EthernetManager;
import android.net.wifi.WifiManager;
import android.os.Build;
import android.os.IBinder;
import android.os.INetworkManagementService;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.provider.Settings;

import com.hybroad.iptv.log.SLog;
import com.hybroad.iptv.param.ParamManager;
import com.hybroad.iptv.param.ParameterInterface;

public class SystemParam extends ParamManager {

	private static final String TAG = "SystemParam";
    private Context mContext;
    
	public SystemParam(Context paramContext) {
		mContext = paramContext;
		systemParamInit();
		TimeParamInit();
		VersionParamInit();
	}

	private void systemParamInit() {
		
		regist("STBID", new ParameterInterface() {
			public String getParameter(String name) {
				String stbID = null;
			    stbID = SystemProperties.get("ro.product.stb.stbid");
				SLog.i(TAG, "getSTBID = " + stbID);
				return stbID;
			}

			public boolean setParameter(String name, String value) {
				SLog.e(TAG, "SystemParam, This ParamStr " + name + "is read only .\n");
				return true;
			}
		});
		
		regist("SerialNumber", new ParameterInterface() {
			public String getParameter(String name) {
				String stbID = null;
			    stbID = SystemProperties.get("ro.serialno");
				SLog.i(TAG, "getSTBID = " + stbID);
				return stbID;
			}

			public boolean setParameter(String name, String value) {
				SLog.e(TAG, "SystemParam, This ParamStr " + name + "is read only .\n");
				return true;
			}
		});

		regist("HardWareVersion", new ParameterInterface() {
			public String getParameter(String name) {
				String hardWareVersion = Build.HWVERSION;

				SLog.i(TAG, "getHardWareVersion = " + hardWareVersion);
				return hardWareVersion;
			}

			public boolean setParameter(String name, String value) {
				SLog.e(TAG, "SystemParam, This ParamStr " + name + "is read only .\n");
				return true;
			}
		});

		regist("MACAddress", new ParameterInterface() {
			public String getParameter(String arg0) {
				String iptvMacAddress;
				ConnectivityManager connMgr = (ConnectivityManager)mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
				NetworkInfo ethInfo = connMgr.getNetworkInfo(ConnectivityManager.TYPE_ETHERNET);
				NetworkInfo wifiInfo = connMgr.getNetworkInfo(ConnectivityManager.TYPE_WIFI);
				WifiManager wifiMgr = (WifiManager)mContext.getSystemService(Context.WIFI_SERVICE);

				if (ethInfo != null && ethInfo.isAvailable() && ethInfo.isConnected() && ethInfo.getState() == State.CONNECTED) {
					iptvMacAddress = getEthernetMac();
				} else if (wifiMgr != null && wifiMgr.getWifiState() == WifiManager.WIFI_STATE_ENABLED && wifiInfo != null && wifiInfo.isAvailable()
						&& wifiInfo.getState() == State.CONNECTED) {
					iptvMacAddress = getWirelessMac();
				} else {
					iptvMacAddress = getEthernetMac();
				}

				SLog.i(TAG, "getIptvMacAddress = " + iptvMacAddress);
				return iptvMacAddress;
			}

			public boolean setParameter(String name, String value) {
				SLog.e(TAG, "SystemParam, This ParamStr " + name + "is read only .\n");
				return true;
			}
		});

		regist("TokenMACAddress", new ParameterInterface() {
			public String getParameter(String arg0) {
				String mac = getEthernetMac();
				SLog.d(TAG, "getIptvTokenMacAddress = " + mac);
				return mac;
			}

			public boolean setParameter(String name, String value) {
				SLog.e(TAG, "SystemParam, This ParamStr " + name + "is read only .\n");
				return true;
			}
		});
	}

	private void TimeParamInit() {
		regist("ntp", new ParameterInterface() {
			
			@Override
			public boolean setParameter(String name, String value) {
				Settings.Hybroad.putString(mContext.getContentResolver(), Settings.Hybroad.NTP_DOMAIN, value);
				return true;
			}
			
			@Override
			public String getParameter(String name) {
				String value = Settings.Hybroad.getString(mContext.getContentResolver(), Settings.Hybroad.NTP_DOMAIN);
				SLog.d(TAG, "Get Param: " + name + ", Value is : " + value);
				return value;
			}
		});
		
		regist("ntp1", new ParameterInterface() {
			
			@Override
			public boolean setParameter(String name, String value) {
				Settings.Hybroad.putString(mContext.getContentResolver(), Settings.Hybroad.NTP_BACKUP, value);
				return true;
			}
			
			@Override
			public String getParameter(String name) {
				String value = Settings.Hybroad.getString(mContext.getContentResolver(), Settings.Hybroad.NTP_BACKUP);
				SLog.d(TAG, "Get Param: " + name + ", Value is : " + value);
				return value;
			}
		});
	};


	private void VersionParamInit() {
		regist("SoftwareHWversion", new ParameterInterface() {
			public String getParameter(String name) {
				String softVersion = Build.SOFTWARE;
				SLog.d(TAG, "getSoftwareHWversion = " + softVersion);
				return softVersion;
			}

			public boolean setParameter(String name, String value) {
				SLog.e(TAG, "This ParamStr " + name + "is read only .\n");
				return true;
			}
		});

		regist("SoftwareVersion", new ParameterInterface() {
			public String getParameter(String arg0) {
				String softwareVersion = Build.APPVERSION;
				SLog.d(TAG, "getSoftwareVersion = " + softwareVersion);
				return softwareVersion;
			}

			public boolean setParameter(String name, String value) {
				SLog.e(TAG, "This ParamStr " + name + "is read only .\n");
				return true;
			}
		});
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

}
