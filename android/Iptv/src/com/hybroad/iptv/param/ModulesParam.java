package com.hybroad.iptv.param;

import android.content.Context;
import android.provider.Settings;

import com.hybroad.iptv.log.SLog;
import com.hybroad.iptv.param.ParamManager;

public class ModulesParam extends ParamManager {

	private static final String TAG = "ModulesParam";
    private Context mContext;
    
	public ModulesParam(Context paramContext) {
		mContext = paramContext;
		TvmsParamInit();
	}
	

	private void TvmsParamInit() {
		regist("tvms_heartbeat", func);
		regist("tvms_delay", func);
		regist("tvms_url", func);
		regist("tvms_vod_url", func);
	}
	
    private ParameterInterface func = new ParameterInterface() {
		@Override
		public String getParameter(String name) {
			String value = Settings.Hybroad.getString(mContext.getContentResolver(), name);
			SLog.d(TAG, "Get Param: " + name + ", Value is : " + value);
			return value;
		}

		@Override
		public boolean setParameter(String name, String value) {
			Settings.Hybroad.putString(mContext.getContentResolver(), name, value);
			return true;
		}
	};
	
}
