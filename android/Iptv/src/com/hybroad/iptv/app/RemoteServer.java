package com.hybroad.iptv.app;

import android.content.Context;

import com.huawei.iptv.remote.framework.LiveRoomServer;
import com.huawei.iptv.remote.framework.LiveRoomServerAppListener;
import com.huawei.iptv.remote.framework.LiveRoomServerImpl;
import com.hybroad.iptv.log.SLog;


public class RemoteServer implements LiveRoomServerAppListener { 
	protected static final String TAG = "RemoteServer";
    private IPTVMiddleWare mMiddleWare = null;
    private LiveRoomServerImpl mLiveRoomServer = null;
    private Context mContext = null;
    
    public RemoteServer(Context paramContext) {
    	mContext = paramContext;
    }
    
    public void registerRemoteServer(){
		SLog.d(TAG, "registerRemoteServer");
		mLiveRoomServer = LiveRoomServer.getInstance(mContext, null);
		mLiveRoomServer.setDebug(true);
		mLiveRoomServer.setAppType("huawei.vihome.iptv");
		mLiveRoomServer.registerAppListner(this);
	}

    public void unRegisterRemoteServer(){
		SLog.d(TAG, "unRegisterRemoteServer");
		if(mLiveRoomServer!=null){
			mLiveRoomServer.release();
		}
	}
    
	@Override
	public String getInfoFromApp(String app, String type) {
		String out;
		SLog.i(TAG,"getInfoFromApp type="+type);
		out = RemoteServerGet(type);
		SLog.i(TAG,"out = "+out);
	    return out;
	}

	@Override
	public void sendInfoToApp(String app, String type, String info) {
		String mInputText = "";
        mInputText = "type=["+type+"] info=["+info+"]";
        SLog.i(TAG,"sendInfoToApp \n"+mInputText);
        RemoteServerSend(type);
	}
    
	public native void RemoteServerSend(String jstr);

	public native String RemoteServerGet(String jstr);
}



