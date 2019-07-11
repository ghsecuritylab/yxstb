package com.hybroad.iptv.app;

import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;

import com.hybroad.iptv.log.SLog;
import com.hybroad.iptv.param.ParamManager;
import android.view.Surface;

import java.lang.ref.WeakReference;


// import com.hybroad.iptv.impl.onPageEventListener;
// import com.hybroad.iptv.impl.onRpcEventListener;
import android.graphics.Rect;

import com.hisilicon.android.HiDisplayManager;

import android.view.ViewGroup.MarginLayoutParams;
import android.widget.RelativeLayout;

public class IPTVMiddleWare {

	private EventHandler mEventHandler;
	private static IPTVMiddleWare mIPTVMiddleware = null;
	private ParamManager mParamManager = null;
	private int mNativeContext;
	private Context mContext;
	private static int mAuthState = -1;

	// private onPageEventListener mOnPageEventListener;
	// private onRpcEventListener mOnRpcEventListener;

	static {
		System.loadLibrary("iptvmw_jni");
		nativeinit();
	}

	private final static String TAG = "IPTVMiddleWare";

	public IPTVMiddleWare(Context paramContext) {
		Looper looper;

		if ((looper = Looper.myLooper()) != null) {
			mEventHandler = new EventHandler(this, looper);
		} else if ((looper = Looper.getMainLooper()) != null) {
			mEventHandler = new EventHandler(this, looper);
		} else {
			mEventHandler = null;
		}
		/*
		 * Native setup requires a weak reference to our object. It's easier to
		 * create it here than in C++.
		 */
		mContext = paramContext;
		mParamManager = new ParamManager(paramContext);
		nativesetup(new WeakReference<IPTVMiddleWare>(this));
	}

	private static native final void nativeinit();

	private native final void nativesetup(Object iptvmiddleware_this);

	public native boolean sendKeyEvent(int keyType, int keyValue);

	public native void pause(); // pause middleware, player resource should be
								// release

	public native void resume(); // resume middleware

	public native void start();

	public native void restart();

	public native void stop();

	public native void destroy(); // release all middleware resource

	public native void initMiddleware(); // init middleware

	public native void onAuthAndOpenMenu(); // On Auth And Open Home Page.

	public native void onDoOpenMenu(); // On Open Home Page, Not Auth.

	public native void openUrl(String urlStr);

	public native void setNativeParameter(int FileNameType, int FileValueType,
			String FieldName, String FieldValue);

	public native void enterStandby();

	public native void killAllProcess();

	public native void notify(String jstr);

	public static IPTVMiddleWare getInstance(Context paramContext) {
		if (mIPTVMiddleware == null) {
			mIPTVMiddleware = new IPTVMiddleWare(paramContext);
		}
		return mIPTVMiddleware;
	}

	public static IPTVMiddleWare getInstance() {
		return mIPTVMiddleware;
	}

	private static void authSucessed() {
		mAuthState = 0;
	}
	
	private static void authFailed() {
		mAuthState = -1;
	}
	
    public int getAuthState(){
        return mAuthState;
    }
    
	private static void postEventFromNative(Object iptvmiddleware_ref,
			int what, int arg1, int arg2, Object obj) {
		IPTVMiddleWare mw = (IPTVMiddleWare) ((WeakReference) iptvmiddleware_ref).get();
		if (mw == null) {
			return;
		}
		SLog.i(TAG, "postEventFromNative:" + what + " ,obj = " + obj);
		if (mw.mEventHandler != null) {
			SLog.i(TAG, "obtainMessage " + what + " ,begin to sendMessage");
			Message m = mw.mEventHandler.obtainMessage(what, arg1, arg2, obj);
			mw.mEventHandler.sendMessage(m);
		}
	}

    /*
	public void setOnPageEventListener(onPageEventListener pageEventListener) {
		mOnPageEventListener = pageEventListener;
	}

	public void setOnRpcEventListener(onRpcEventListener rpcEventListener) {
		mOnRpcEventListener = rpcEventListener;
	}
    */

    /*
	private void handleRpcEvent(Message msg) {

		if (mOnRpcEventListener == null) {
			SLog.e(TAG, " mOnRpcEventListener is null");
			return;
		}
		SLog.i(TAG, "handleRpcEvent, Event Content is :" + msg.arg1);
		switch (msg.arg1) {
		case IPTVConstant.RPC_REBOOT:
			mOnRpcEventListener.onReboot();
			break;
		case IPTVConstant.RPC_RESTORE:
			mOnRpcEventListener.onRestore();
			break;
		case IPTVConstant.RPC_EXIT_APP:
			mOnRpcEventListener.onExitApp();
			break;
		case IPTVConstant.RPC_START_APP_BYNAME:
			mOnRpcEventListener.onStartAppByName((String) msg.obj);
			break;
		case IPTVConstant.RPC_START_APP_BYINTENT:
			mOnRpcEventListener.onStartAppByIntent((String) msg.obj);
			break;
		default:
			break;
		}

	}
	private void handlPageEvent(IPTVMiddleWare mw, Message msg) {
		if (mOnPageEventListener == null) {
			SLog.e(TAG, " mOnPageEventListener is null");
			return;
		}
		SLog.i(TAG, "handlPageEvent, Event Content is :" + msg.arg1);
		switch (msg.arg1) {
		case IPTVConstant.PAGE_AUTH_FILED:
			mOnPageEventListener.onAuthFailed(mw);
			break;
		case IPTVConstant.PAGE_AUTH_FINISHED:
			mOnPageEventListener.onAuthFinished(mw);
			break;
		case IPTVConstant.PAGE_AUTH_STARTED:
			mOnPageEventListener.onAuthStarted(mw);
			break;
		case IPTVConstant.PAGE_LOAD_ERROR:
			String url = (String) msg.obj;
			mOnPageEventListener.onPageLoadError(mw, url, msg.arg2);
			break;
		default:
			break;
		}
	};
   */
   
	class EventHandler extends Handler {
		private IPTVMiddleWare mIptvmw;

		public EventHandler(IPTVMiddleWare mw, Looper looper) {
			super(looper);
			mIptvmw = mw;
		}

		public void handleMessage(Message msg) {
			SLog.i(TAG, "handleMessage, Event Type is :" + msg.what);
			switch (msg.what) {
                /*
			case IPTVConstant.EVENT_RPC:
				handleRpcEvent(msg);
				break;
			case IPTVConstant.EVENT_PAGE:
				handlPageEvent(mIptvmw, msg);
				break;
                */
			default:
				SLog.e(TAG, "No Event Handler.");
				break;
			}

		}
	}

}
