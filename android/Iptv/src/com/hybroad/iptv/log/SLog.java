package com.hybroad.iptv.log;

import android.content.Context;
import android.util.Log;
import android.os.Build;
import java.io.IOException;
import android.os.RemoteException;

public final class SLog {

	private Context mContext;
    public static final String TERMC_RED = "\033[1;31;40m";
    public static final String TERMC_GREEN = "\033[1;32;40m";
    public static final String TERMC_YELLOW =  "\033[1;33;40m";
    public static final String TERMC_BLUE = "\033[1;34;40m";
    public static final String TERMC_PINK = "\033[1;35;40m";
    public static final String TERMC_NONE = "\033[m";

    private static final String TTAG = "IPTV |iptv_logging";

    public SLog(Context SLogContext){
		mContext = SLogContext;
    }

    public static int v(String tag, String msg) {
        return Log.v(tag, msg);
    }

    public static int v(String msg) {
        return Log.v(TTAG, msg);
    }

    public static int v(String tag, String msg, Throwable tr) {
        return Log.v(tag, msg, tr);
    }

    public static int d(String tag, String msg) {
        return Log.d(tag, msg);
    }

    public static int d(String msg) {
        return Log.d(TTAG, msg);
    }

    public static int d(String tag, String msg, Throwable tr) {
        return Log.d(tag, msg, tr);
    }

    public static int i(String tag, String msg) {
        return Log.i(tag, msg);
    }

    public static int i(String msg) {
        return Log.i(TTAG, msg);
    }

    public static int i(String tag, String msg, Throwable tr) {
        return Log.i(tag, msg, tr);
    }

    public static int e(String tag, String msg) {
        return Log.e(tag, msg);
    }

    public static int e(String msg) {
        return Log.e(TTAG, msg);
    }

    public static int e(String tag, String msg, Throwable tr) {
        return Log.e(tag, msg, tr);
    }

    public static int red(String msg) {
        return Log.v(TTAG, TERMC_RED + msg + TERMC_NONE);
    }

    public static int yellow(String msg) {
        return Log.v(TTAG, TERMC_YELLOW + msg + TERMC_NONE);
    }

    public static int green(String msg) {
        return Log.v(TTAG, TERMC_GREEN + msg + TERMC_NONE);
    }

    public static int blue(String msg) {
        return Log.v(TTAG, TERMC_BLUE + msg + TERMC_NONE);
    }

    public static int pink(String msg) {
        return Log.v(TTAG, TERMC_PINK + msg + TERMC_NONE);
    }

}




