package com.hybroad.iptv.app;


import android.content.Context;
import android.graphics.Rect;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.AttributeSet;
import android.util.Log;
import android.view.KeyEvent;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.ViewGroup.MarginLayoutParams;
import android.view.inputmethod.BaseInputConnection;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;
import android.view.inputmethod.InputMethodManager;
import android.widget.RelativeLayout;

import com.hisilicon.android.HiDisplayManager;
import com.hybroad.iptv.log.SLog;


// implements SurfaceHolder.Callback , Runnable 
public class HybroadSurfaceView extends SurfaceView implements SurfaceHolder.Callback { 
	protected static final String SURFACE_LOG_TAG = "IPTVMiddleWare";
	private int mNativeClass;
	private SurfaceHolder holder;
    private InputMethodManager input = null; 
    private IPTVMiddleWare mMiddleware = null;
    private int mPositionY = 0;
    
    private Rect mRect;
    private HiDisplayManager mDisplayManager;
    
    private int oldKeyValue = 0;
    private long keyPressTime = 0;
    
    public HybroadSurfaceView(Context context, AttributeSet attrs) { 
        super(context, attrs); 
        holder=this.getHolder(); 
        holder.setFormat(5);
        holder.addCallback(this); 
        this.setFocusable(true); 
        this.setFocusableInTouchMode(true); 
        Log.d(SURFACE_LOG_TAG, "this = " + this);
        input=(InputMethodManager) (InputMethodManager) getContext().getSystemService(Context.INPUT_METHOD_SERVICE); 
    }

    public void setMiddleWare(IPTVMiddleWare m) {
        mMiddleware = m;        
        nativeSurfaceInit(this);
    }
    
    public void surfaceCreated(SurfaceHolder holder) {
        Surface localSurface = holder.getSurface();
        nativeSetSurface(IPTVConstant.SURFACE_EPG, localSurface);
    }

    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
    }

    public void surfaceDestroyed(SurfaceHolder holder) {
        holder.removeCallback(this);
        nativeClearSurface();
    }
    
    public void showIme(int top) { 
        // TODO Auto-generated method stub 
        input.showSoftInput(this, 0); 
        
        Log.d(SURFACE_LOG_TAG, "input.isActive(this) = " + input.isActive(this));
		Log.d(SURFACE_LOG_TAG, "input.isAcceptingText = " + input.isAcceptingText());
		Log.d(SURFACE_LOG_TAG, "input.isActive () = " + input.isActive ());
		
		SLog.d(SURFACE_LOG_TAG, "showImeState top = " + top + ", IPTVConstant.IPTV_SOFTKEYBOARD_HEIGHT = " + IPTVConstant.IPTV_SOFTKEYBOARD_HEIGHT);
		if (top > 400) {
			MarginLayoutParams margin = new MarginLayoutParams(getLayoutParams());
			margin.setMargins(margin.leftMargin, -IPTVConstant.IPTV_SOFTKEYBOARD_HEIGHT, margin.rightMargin, margin.height - IPTVConstant.IPTV_SOFTKEYBOARD_HEIGHT);
			RelativeLayout.LayoutParams layoutParams = new RelativeLayout.LayoutParams(margin);
			SLog.d(SURFACE_LOG_TAG, "setLayoutY");
			setLayoutParams((RelativeLayout.LayoutParams) layoutParams);
			setPositionY(IPTVConstant.IPTV_SOFTKEYBOARD_HEIGHT);
		}
    }
    public void hideIme() { 
        // TODO Auto-generated method stub 
    	input.hideSoftInputFromWindow(this.getWindowToken(), 0); 
    }
    
    class HybroadInputConnection extends BaseInputConnection{
        private View mSurface;
        public HybroadInputConnection(View targetView, boolean fullEditor) { 
            super(targetView, fullEditor); 
            mSurface = targetView;
            Log.d(SURFACE_LOG_TAG, "mSurface = " + mSurface);
            // TODO Auto-generated constructor stub 
        } 
        public boolean commitText(CharSequence text, int newCursorPosition){ 
            int len = text.length();
            int i;
            Log.d(SURFACE_LOG_TAG, "len" + len);
            if (mMiddleware != null) {
                for (i = 0; i < len; i++) {
                	Log.d(SURFACE_LOG_TAG, "input[" + i + "] : " + text.charAt(i));
                	mMiddleware.sendKeyEvent(IPTVConstant.MessageType_Char, text.charAt(i));
                }
            }
            return true; 
        }
        
        @Override
        public boolean finishComposingText () {
            Log.d(SURFACE_LOG_TAG, "finishComposingText");

            if (mPositionY != 0) {
                MarginLayoutParams margin=new MarginLayoutParams(mSurface.getLayoutParams());
        		margin.setMargins(margin.leftMargin, 0, margin.rightMargin, margin.height);
        		RelativeLayout.LayoutParams layoutParams = new RelativeLayout.LayoutParams(margin);
        		Log.d(SURFACE_LOG_TAG, "setLayoutY");
        		mSurface.setLayoutParams((RelativeLayout.LayoutParams)layoutParams);
        		mPositionY = 0;
            }

            return super.finishComposingText(); 
        }
    }
    
    void setPositionY(int y) {
        mPositionY = y;
    }
    
    @Override 
    public InputConnection onCreateInputConnection(EditorInfo outAttrs) { 
        // TODO Auto-generated method stub 
        return new HybroadInputConnection(this,false);//super.onCreateInputConnection(outAttrs); 
    }
    
    public boolean onKeyDown(int keyCode, KeyEvent paramKeyEvent) {
		int keyValue;

		if ((keyCode == oldKeyValue)
			&&(java.lang.System.currentTimeMillis() - keyPressTime < 200)){
	        SLog.d(SURFACE_LOG_TAG, "If below 200ms , Press The Same key...\n");
			return true;
		}
		keyPressTime = java.lang.System.currentTimeMillis();
        oldKeyValue = keyCode;
        
        switch (keyCode) {
	        case KeyEvent.KEYCODE_VOLUME_MUTE:
	            keyValue = IPTVConstant.EIS_IRKEY_VOLUME_MUTE;
	            break;
	        case KeyEvent.KEYCODE_1:
	            keyValue = IPTVConstant.EIS_IRKEY_NUM1;
	            break;
	        case KeyEvent.KEYCODE_2:
	            keyValue = IPTVConstant.EIS_IRKEY_NUM2;
	            break;
	        case KeyEvent.KEYCODE_3:
	            keyValue = IPTVConstant.EIS_IRKEY_NUM3;
	            break;
	        case KeyEvent.KEYCODE_4:
	            keyValue = IPTVConstant.EIS_IRKEY_NUM4;
	            break;
	        case KeyEvent.KEYCODE_5:
	            keyValue = IPTVConstant.EIS_IRKEY_NUM5;
	            break;
	        case KeyEvent.KEYCODE_6:
	            keyValue = IPTVConstant.EIS_IRKEY_NUM6;
	            break;
	        case KeyEvent.KEYCODE_7:
	            keyValue = IPTVConstant.EIS_IRKEY_NUM7;
	            break;
	        case KeyEvent.KEYCODE_8:
	            keyValue = IPTVConstant.EIS_IRKEY_NUM8;
	            break;
	        case KeyEvent.KEYCODE_9:
	            keyValue = IPTVConstant.EIS_IRKEY_NUM9;
	            break;
	        case KeyEvent.KEYCODE_0:
	            keyValue = IPTVConstant.EIS_IRKEY_NUM0;
	            break;
	        case KeyEvent.KEYCODE_CHANNEL_DOWN:
	            keyValue = IPTVConstant.EIS_IRKEY_CHANNEL_DOWN;
	            break;
	        case KeyEvent.KEYCODE_CHANNEL_UP:
	            keyValue = IPTVConstant.EIS_IRKEY_CHANNEL_UP;
	            break;
	        case KeyEvent.KEYCODE_BACK:
	            keyValue = IPTVConstant.EIS_IRKEY_BACK;
	            break;
            case KeyEvent.KEYCODE_DPAD_CENTER:
                keyValue = IPTVConstant.EIS_IRKEY_SELECT;
                break;
	        case KeyEvent.KEYCODE_DPAD_UP:
	            keyValue = IPTVConstant.EIS_IRKEY_UP;
	            break;
	        case KeyEvent.KEYCODE_DPAD_DOWN:
	            keyValue = IPTVConstant.EIS_IRKEY_DOWN;
	            break;
	        case KeyEvent.KEYCODE_DPAD_LEFT:
	            keyValue = IPTVConstant.EIS_IRKEY_LEFT;
	            break;
	        case KeyEvent.KEYCODE_DPAD_RIGHT:
	            keyValue = IPTVConstant.EIS_IRKEY_RIGHT;
	            break;
	        case KeyEvent.KEYCODE_PAGE_UP:
	            keyValue = IPTVConstant.EIS_IRKEY_PAGE_UP;
	            break;
	        case KeyEvent.KEYCODE_PAGE_DOWN:
	            keyValue = IPTVConstant.EIS_IRKEY_PAGE_DOWN;
	            break;
	        case KeyEvent.KEYCODE_VOLUME_UP:
	            keyValue = IPTVConstant.EIS_IRKEY_VOLUME_UP;
	            break;
	        case KeyEvent.KEYCODE_VOLUME_DOWN:
	            keyValue = IPTVConstant.EIS_IRKEY_VOLUME_DOWN;
	            break;
	        case KeyEvent.KEYCODE_F1:
	            keyValue = IPTVConstant.EIS_IRKEY_GREEN;
	            break;
	        case KeyEvent.KEYCODE_F2:
	            keyValue = IPTVConstant.EIS_IRKEY_RED;
	            break;
	        case KeyEvent.KEYCODE_F3:
	            keyValue = IPTVConstant.EIS_IRKEY_YELLOW;
	            break;
	        case KeyEvent.KEYCODE_F4:
	            keyValue = IPTVConstant.EIS_IRKEY_BLUE;
	            break;
		    case KeyEvent.KEYCODE_F6:
				keyValue = IPTVConstant.EIS_IRKEY_STAR;
				break;
	        case KeyEvent.KEYCODE_SEARCH:
	            keyValue = IPTVConstant.EIS_IRKEY_VK_F10;
	            break;
	        case KeyEvent.KEYCODE_MENU:
	            keyValue = IPTVConstant.EIS_IRKEY_MENU;
	            break;
	        case KeyEvent.KEYCODE_MEDIA_REWIND:
	            keyValue = IPTVConstant.EIS_IRKEY_REWIND;
	            break;
	        case KeyEvent.KEYCODE_FORWARD:
            case KeyEvent.KEYCODE_MEDIA_FAST_FORWARD:
	            keyValue = IPTVConstant.EIS_IRKEY_FASTFORWARD;
	            break;
	        case KeyEvent.KEYCODE_MEDIA_PLAY_PAUSE:
	            keyValue = IPTVConstant.EIS_IRKEY_PLAY;
	            break;
	        case KeyEvent.KEYCODE_MEDIA_STOP:
	            keyValue = IPTVConstant.EIS_IRKEY_STOP;
	            break;
			case KeyEvent.KEYCODE_D:
				keyValue = IPTVConstant.EIS_IRKEY_BTV;
				break;
		    case KeyEvent.KEYCODE_F:
				keyValue = IPTVConstant.EIS_IRKEY_TVOD;
				break;
			case KeyEvent.KEYCODE_E:
				keyValue = IPTVConstant.EIS_IRKEY_VOD;
				break;
			case KeyEvent.KEYCODE_INFO:
				keyValue = IPTVConstant.EIS_IRKEY_INFO;
				break;
			case KeyEvent.KEYCODE_AUDIO:
				keyValue = IPTVConstant.EIS_IRKEY_AUDIO;
				break;
			case KeyEvent.KEYCODE_IME:
				keyValue = IPTVConstant.EIS_IRKEY_IME;
				break;
            case KeyEvent.KEYCODE_ENTER:
            	mMiddleware.sendKeyEvent(IPTVConstant.MessageType_Char, 0x0a);
                return true;
                // keyValue = IPTVConstant.EIS_IRKEY_SELECT;
                // break;
            case KeyEvent.KEYCODE_DEL:
                keyValue = IPTVConstant.EIS_IRKEY_BACK;
                break;
	        default:
                SLog.e(SURFACE_LOG_TAG, "Iptv is Not Support This key, Android KeyCode = " + keyCode);
	            return false;
	      }

        SLog.i(SURFACE_LOG_TAG, "Android KeyCode = " + keyCode + ", keyValue = " + keyValue);
        mMiddleware.sendKeyEvent(IPTVConstant.MessageType_KeyDown, keyValue);

		return true;
	}
    
    private native final void nativeSurfaceInit(Object surfaceview_this);
    private native void nativeSetSurface(int paramInt, Surface paramSurface);
    private native void nativeClearSurface();
}
