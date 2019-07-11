package com.hybroad.iptv.param;

import android.content.Context;
import android.provider.Settings;

import com.hybroad.iptv.log.SLog;
import com.hybroad.iptv.param.ParamManager;
import com.hybroad.iptv.param.ParameterInterface;

public class BusinessParam extends ParamManager {

	private static final String TAG = "BusinessParam";
    private Context mContext;
    
	public BusinessParam(Context paramContext) {
		mContext = paramContext;
		BusinessParamInit();
	}

	private void BusinessParamInit() {
		
		regist("eds", func);
		regist("eds1", func);
		regist("epg", func);
		regist("lastChannelPlay", func);
		regist("CurrentEPGUrl", new ParameterInterface() {
			public String getParameter(String name) {
				SLog.e(TAG, "setParametereter paramStr " + name + "cann't read.\n");
				return null;
			}

			public boolean setParameter(String name, String value) {
				android.os.SystemProperties.set("sys.iptv.CurrentEPGUrl", value);
				return true;
			}
		});
		
		AccountParamInit();
		LogoParamInit();
		SessionParamInit();
	}
    
	private void AccountParamInit() {
		regist("ntvuser", func);
		regist("ntvAESpasswd", func);
	}
	
	private void LogoParamInit() {
		regist("PADBootLogPicURL", func);
		regist("PADAuthenBackgroundPicURL", func);
		regist("bootlogo_md5", func);
		regist("authbg_md5", func);
	}

	private void SessionParamInit() {	
		regist("user_token", func);
	}
	
    private ParameterInterface func = new ParameterInterface() {
		@Override
		public String getParameter(String name) {
			String newName = convertMwToSettings(name);
			String value;
			
			if (newName.equals(Settings.Hybroad.IPTV_PASSWORD))
				value = Settings.Hybroad.getStringAES(mContext.getContentResolver(), name);
		    else
				value = Settings.Hybroad.getString(mContext.getContentResolver(), name);
			
			SLog.i(TAG, "Get Param: " + name + ", Value is : " + value);
			return value;
		}

		@Override
		public boolean setParameter(String name, String value) {
			String newName = convertMwToSettings(name);
			if (newName.equals(Settings.Hybroad.IPTV_PASSWORD))
				Settings.Hybroad.putStringAES(mContext.getContentResolver(), name, value);
			else
				Settings.Hybroad.putString(mContext.getContentResolver(), name, value);
			
			return true;
		}
	};
	
	private String convertMwToSettings(String paramName) {
		SLog.d(TAG, "convertMwToSettings");

		if ("eds".equals(paramName)) 
			return Settings.Hybroad.IPTV_EDS;
	    else if ("eds1".equals(paramName))
			return Settings.Hybroad.IPTV_EDS_BACKUP;
		else if ("epg".equals(paramName))
			return Settings.Hybroad.IPTV_EPG;
	    else if ("lastChannelPlay".equals(paramName)) 
			return Settings.Hybroad.IPTV_START_MODE;
	    else if ("ntvuser".equals(paramName))
            return Settings.Hybroad.IPTV_USERNAME;
	    else if ("ntvAESpasswd".equals(paramName))
	    	return Settings.Hybroad.IPTV_PASSWORD;
		return paramName;
	}
	
}
