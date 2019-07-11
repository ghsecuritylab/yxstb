package com.hybroad.iptv.param;

import android.content.Context;

import com.hybroad.iptv.log.SLog;
import com.hybroad.iptv.param.ParamManager;
import com.hybroad.iptv.param.ParameterInterface;

import android.media.AudioManager;
import android.provider.Settings;

public class PlayParam extends ParamManager {

	private AudioManager mAm = null;
    private Context mContext = null;
	private static final String TAG = "PlayParam";

	public PlayParam(Context paramContext) {
		mContext = paramContext;
		mAm = (AudioManager) paramContext.getSystemService(Context.AUDIO_SERVICE);
		playParamInit();
	}

	private void playParamInit() {
		
		regist("TransportProtocol", func);
		regist("iptv_last_multicast_address", func);
		regist("changevideomode", new ParameterInterface() {		
			public boolean setParameter(String param, String value) {
		        Settings.Hybroad.putString(mContext.getContentResolver(), Settings.Hybroad.VIDEO_CHANNEL_CHANGE_MODE, value);
				return true;
			}
			
			public String getParameter(String name) {
				String value = Settings.Hybroad.getString(mContext.getContentResolver(), Settings.Hybroad.VIDEO_CHANNEL_CHANGE_MODE);
				SLog.d(TAG, "Get Param: " + name + ", Value is : " + value);
				return value;
			}
		});
		
		regist("volume", new ParameterInterface() {
			public String getParameter(String name) {
				int v = 0;
				if (mAm.isStreamMute(AudioManager.STREAM_MUSIC)) {
					v = mAm.getLastAudibleStreamVolume(AudioManager.STREAM_MUSIC);
				} else {
					v = mAm.getStreamVolume(AudioManager.STREAM_MUSIC);
				}
				v *= 5;
				SLog.d(TAG, "CurrentVolume = " + v);
				return String.valueOf(v);
			}

			public boolean setParameter(String name, String value) {
				SLog.d(TAG, "setVolume: " + Integer.parseInt(value));
				mAm.setStreamVolume(AudioManager.STREAM_MUSIC, Integer.parseInt(value) / 5, 0);
				return true;
			}
		});

		regist("mute", new ParameterInterface() {
			public String getParameter(String name) {
				boolean flag = mAm.isStreamMute(AudioManager.STREAM_MUSIC);
				return (flag ? "1" : "0");
			}

			public boolean setParameter(String name, String value) {
				SLog.d(TAG, "setMute: " + Integer.parseInt(value));
				if (value.equals("0") == mAm.isStreamMute(AudioManager.STREAM_MUSIC))
					mAm.setStreamMute(AudioManager.STREAM_MUSIC, !value.equals("0"));
				return true;
			}
		});
	}

	private ParameterInterface func = new ParameterInterface() {
		public String getParameter(String name) {
			String value = Settings.Hybroad.getString(mContext.getContentResolver(), name);
			SLog.i(TAG, "Get Param: " + name + ", Value is : " + value);
			return value;
		}

		public boolean setParameter(String name, String value) {
		    Settings.Hybroad.putString(mContext.getContentResolver(), name, value);
			return true;
		}
	};

}
