
package com.hybroad.iptv.app;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.BroadcastReceiver;
import android.net.ethernet.EthernetManager;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceHolder.Callback;
import android.view.Surface;
import android.widget.RelativeLayout;

import com.hybroad.iptv.utils.SPHelper;
import com.hybroad.iptv.log.SLog;
import com.hybroad.iptv.utils.ToastUtils;
import com.hybroad.iptv.utils.AssetCopyUtils;
import com.hybroad.iptv.app.IPTVMiddleWare;
import com.hybroad.iptv.app.network.NetworkManager;

import com.hybroad.iptv.preset.pack.name.R;

public class HomeActivity extends Activity {

    private static final String TAG = "IPTV HomeActivity";
    private Context mContext;
    private int mAssetVersion = 1;

    private int oldKeyValue = 0;
    private long keyPressTime = 0;

    private RelativeLayout mMainLayout;
    private SurfaceHolder.Callback mEpgCallback;
    private HybroadSurfaceView mEPG;
   
    private IPTVMiddleWare mIptvMiddleWare = null;
    private RemoteServer mRemoteServer = null;


    @Override
    protected void onCreate(Bundle arg0)
    {
        super.onCreate(arg0);
        mContext = this;

        SLog.red("Home Activity onCreate");

        mIptvMiddleWare = IPTVMiddleWare.getInstance(this);
        mIptvMiddleWare.initMiddleware();
        
        // views
        initViews();

        // network state receiver
        registerNetworkListener();

        //LiveRoomServer register
        mRemoteServer = new RemoteServer(this);
        mRemoteServer.registerRemoteServer();
        
        // toast
        ToastUtils.showToast(IPTVConstant.NETWORK_CONNECTTING, this);

        // assets
        CopyAssetsToFileDir();

        // TODO: 定制
        // clearMediaCenter();
    }

    @Override
    protected void onResume()
    {
        SLog.red("onResume");
        super.onResume();
    }

    @Override
    protected void onPause()
    {
        SLog.red("onPause");
        super.onPause();
        mIptvMiddleWare.pause();
    }

    @Override
    protected void onStop()
    {
        SLog.red("onStop");
        super.onStop();
        mIptvMiddleWare.stop();
        mMainLayout.removeAllViewsInLayout();
        mEPG = null;
    }

    @Override
    protected void onRestart()
    {
        SLog.red("onRestart");
        super.onRestart();
        initViews();
        mIptvMiddleWare.restart();
    }

    @Override
    protected void onDestroy()
    {
        SLog.red("onDestroy");
        super.onDestroy();
        mIptvMiddleWare.destroy();
        unregisterNetworkListener();
        mRemoteServer.unRegisterRemoteServer();
    }

    @Override
    protected void onNewIntent(Intent intent)
    {
        SLog.red("onNewIntent");
        // super.onNewIntent(intent);
        setIntent(intent);
    }

    private void initViews()
    {
        setContentView(R.layout.main);
        mMainLayout = (RelativeLayout)findViewById(R.id.mainLayout);
        mEPG = (HybroadSurfaceView)findViewById(R.id.epg);
        mEPG.setMiddleWare(mIptvMiddleWare);
    }

    // monitor network state

    private void registerNetworkListener()
    {
        IntentFilter filter = new IntentFilter();
        filter.addAction(EthernetManager.ETHERNET_STATE_CHANGED_ACTION);
        registerReceiver(mNetworkListener, filter);
    }

    private void unregisterNetworkListener()
    {
        unregisterReceiver(mNetworkListener);
    }

    private void onNetworkConnect()
    {
    	if (IPTVMiddleWare.getInstance(this).getAuthState() != 0) { //No authSuccessed
            ToastUtils.showToast(IPTVConstant.NETWORK_CONNECT, mContext);
        }
        NetworkManager.getNetworkManager().registerBroadcastReceiver(this);
    }

    private void onNetworkDisconnect()
    {
        ToastUtils.showToast(IPTVConstant.NETWORK_DISCONNECT, mContext);
        IPTVMiddleWare.getInstance(this).notify("NETWORK_DISCONNECT");
    }

    private BroadcastReceiver mNetworkListener = new BroadcastReceiver() {
        public void onReceive(Context context, Intent intent)
        {
            String action = intent.getAction();
            if (action.equals(EthernetManager.ETHERNET_STATE_CHANGED_ACTION)) {
                int ret = -1;
                int message = intent.getIntExtra(EthernetManager.EXTRA_ETHERNET_STATE, ret);
                SLog.yellow("message = " + message);
                switch (message) {
                    case IPTVConstant.EVENT_VLAN_DHCP_CONNECT_SUCCESSED:
                        onNetworkConnect();
                        break;
                    case IPTVConstant.EVENT_VLAN_DHCP_CONNECT_FAILED:
                        onNetworkDisconnect();
                        break;
                    default:
                        // 祝捷说后续各种连接方式连接成功后都会发成功消息。。但是现在还没发。
                        onNetworkConnect();
                        break;
                }
            }
        }
    };

    //unzip resource

    private void CopyAssetsToFileDir() {
        // TODO: 版本号不能这样搞！
        try {
            SPHelper.getSP(mContext, "IptvAsset");
            int assetversion = SPHelper.getInt("asset_version", -1);
            if(mAssetVersion == assetversion){
                return;
            }
            long start_time_copy = System.currentTimeMillis();
            AssetCopyUtils.CopyAssets(getAssets(), "hybroad", getFilesDir().getAbsolutePath());
            SPHelper.putInt("asset_version", mAssetVersion);
            long end_time_copy = System.currentTimeMillis();
            SLog.d(TAG, "Copy assets file to files cost time: " + (end_time_copy - start_time_copy));
        } catch (Exception e) {
            SLog.e(TAG, "Copy assets file to files Error", e); 
            e.printStackTrace();
        }
    }
}
