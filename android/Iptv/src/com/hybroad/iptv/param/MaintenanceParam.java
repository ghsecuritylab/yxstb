package com.hybroad.iptv.param;

import android.content.Context;
import android.provider.Settings;

import com.hybroad.iptv.log.SLog;
import com.hybroad.iptv.param.ParamManager;

public class MaintenanceParam extends ParamManager {

	private static final String TAG = "MaintenanceParam";
    private Context mContext;
    
	public MaintenanceParam(Context paramContext) {
		mContext = paramContext;
		NetDiagnoseParamInit();
		Tr069ParamInit();
		UpgradeParamInit();
	}
	
	private void NetDiagnoseParamInit() {
		regist("check_type", func);
		regist("mul_type", func);
		regist("downloadtest_url", func);
	}
	
	private void Tr069ParamInit() {
		regist("tr069_reg_username", func);
		regist("tr069_reg_password", func);
		regist("tr069_enable", func);
		regist("tr069_url", func);
		regist("tr069_heartbeat_enable", func);
		regist("tr069_heartbeat_cycle", func);
	}
	
	private void UpgradeParamInit() {
		regist("upgradeUrl", new ParameterInterface() {	
			@Override
			public boolean setParameter(String name, String value) {
				Settings.Hybroad.putString(mContext.getContentResolver(), Settings.Hybroad.UPGRADE_URL, value);
				return true;
			}
	
			@Override
			public String getParameter(String name) {
				String value = Settings.Hybroad.getString(mContext.getContentResolver(), Settings.Hybroad.UPGRADE_URL);
				SLog.d(TAG, "Get Param: " + name + ", Value is : " + value);
				return value;
			}
		});
		
		regist("upgradeBackupUrl", new ParameterInterface() {	
			@Override
			public boolean setParameter(String name, String value) {
				Settings.Hybroad.putString(mContext.getContentResolver(), Settings.Hybroad.UPGRADE_BACKUP_URL, value);
				return true;
			}
			
			@Override
			public String getParameter(String name) {
				String value = Settings.Hybroad.getString(mContext.getContentResolver(), Settings.Hybroad.UPGRADE_BACKUP_URL);
				SLog.d(TAG, "Get Param: " + name + ", Value is : " + value);
				return value;
			}
		});
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
