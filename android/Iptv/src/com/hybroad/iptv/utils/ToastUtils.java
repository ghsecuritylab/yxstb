package com.hybroad.iptv.utils;

import android.content.Context;
import android.widget.Toast;
import com.hybroad.iptv.log.SLog;
import com.hybroad.iptv.app.IPTVConstant;
// import com.hybroad.iptv.app.R;
import com.hybroad.iptv.preset.pack.name.R;

public class ToastUtils {
    private static String TAG = "IPTVToastUtils";
    private static String oldMsg = null;
    protected static Toast mtoast = null;
    private static long oneTime = 0;
    private static long twoTime = 0;
    private static Context toastContext = null;

    public ToastUtils(Context mContext)
    {
        toastContext = mContext;
    }

    public static void showToast(int paramString, Context mContext)
    {
        String currentStr = null;

        if (toastContext == null)
            toastContext = mContext;

        switch (paramString) {
        case IPTVConstant.NETWORK_CONNECTTING:
            currentStr = toastContext.getString(R.string.network_connectting);
            break;
        case IPTVConstant.NETWORK_DISCONNECT:
            currentStr = toastContext.getString(R.string.network_disconnect);
            break;
        case IPTVConstant.NETWORK_CONNECT:
            currentStr = toastContext.getString(R.string.network_connect);
            break;
        default:
            break;
        }

        if ((currentStr == null) || "".equals(currentStr)) {
            SLog.d(TAG, "No Toast Data To Show.");
            return;
        } else {
            SLog.i(TAG, "Begin to Show Toast ..." + currentStr);
            if (mtoast == null) {
                mtoast = Toast.makeText(mContext, currentStr, Toast.LENGTH_SHORT);
                mtoast.show();
                oneTime = System.currentTimeMillis();
            } else {
                twoTime = System.currentTimeMillis();
                if (currentStr.equals(oldMsg)) {
                    if ((twoTime - oneTime) > Toast.LENGTH_SHORT) {
                        mtoast.show();
                    }
                } else {
                    oldMsg = currentStr;
                    mtoast.setText(currentStr);
                    mtoast.show();
                }
            }
            oneTime = twoTime;
        }
    }

}
