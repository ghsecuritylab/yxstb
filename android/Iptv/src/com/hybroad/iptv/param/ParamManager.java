package com.hybroad.iptv.param;

import java.util.HashMap;
import java.util.Map;

import android.content.Context;

import com.hybroad.iptv.log.SLog;

public class ParamManager {

	private static Context mContext;
	private static Map<String, ParameterInterface> mMap;
    private BusinessParam mBusinessParam = null;
    private MaintenanceParam mMaintenanceParam = null;
    private ModulesParam mModulesParam = null;
    private NetworkParam mNetworkParam = null;
    private PlayParam mPlayParam = null;
    private SystemParam mSystemParam = null;
    
	private static final String TAG = "ParamManager";

	public ParamManager() {
	}

	public ParamManager(Context paramContext) {
		mContext = paramContext;
		mBusinessParam = new BusinessParam(paramContext);
		mMaintenanceParam = new MaintenanceParam(paramContext);
		mModulesParam = new ModulesParam(paramContext);
		mNetworkParam = new NetworkParam(paramContext);
		mPlayParam = new PlayParam(paramContext);
		mSystemParam = new SystemParam(paramContext);
	
		nativeSettingInit();
	}

	public void regist(String name, ParameterInterface func) {
		if (mMap == null)
			mMap = new HashMap<String, ParameterInterface>();
		
        if (mMap.containsKey(name)) {
        	SLog.e(TAG, "ERROR !!! The param : " + name + " has regist.");
        	return;
        }
        
		mMap.put(name, func);
	}

	public void unregist(String name) {
		if (mMap.get(name) != null) {
			mMap.remove(name);
		} else 
			SLog.e(TAG, "The unregist name " + name + " is not exist");
	}
	
	public static int set(String name, String value) {
		if (mMap.get(name) != null) {
			mMap.get(name).setParameter(name, value);
			SLog.d(TAG, "set name = " + name + " value = " + value);
			return 0;
		} else 
			return -1;
	}

	public static String get(String name) {
		if (mMap.get(name) != null) {
			String value = mMap.get(name).getParameter(name);
			SLog.d(TAG, "GetParameter name = " + name + " value = " + value);
			return value;
		} else 
			return null;
	}

	private native void nativeSettingInit();
}
