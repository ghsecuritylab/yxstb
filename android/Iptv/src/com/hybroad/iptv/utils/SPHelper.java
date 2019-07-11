package com.hybroad.iptv.utils;

import android.content.Context;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;

public class SPHelper {

    private boolean DEBUG = true;
    private static SharedPreferences.Editor mEditor;
    private static SharedPreferences mSharedPref;
    public static final String pkgIndex = "index";
    public static final String pkgName = "pkg";

    public static String getPkg() {

        return mSharedPref.getString("pkg", null);

    }

    public static void putPkg(String paramString) {
        mEditor.putString("pkg", paramString);
        mEditor.commit();
    }

    public static boolean getBoolean(String paramString, boolean paramBoolean) {

        return mSharedPref.getBoolean(paramString, paramBoolean);
    }

    public static void putInt(String paramString, int paramInt) {
        mEditor.putInt(paramString, paramInt);
        mEditor.commit();
    }

    public static void getSP(Context paramContext, String paramString) {
        mSharedPref = paramContext.getSharedPreferences(paramString, 0);
        mEditor = mSharedPref.edit();

    }

    public static int getIndex() {
        return mSharedPref.getInt("index", 0);
    }

    public static int getInt(String paramString) {
        return mSharedPref.getInt(paramString, 0);
    }

    public static int getInt(String paramString, int paramInt) {
        return mSharedPref.getInt(paramString, paramInt);
    }

    public static void putBoolean(String paramString, boolean paramBoolean) {

        mEditor.putBoolean(paramString, paramBoolean);
        mEditor.commit();
    }

    public static void putIndex(int paramInt) {
        mEditor.putInt("index", paramInt);
        mEditor.commit();
    }

    public static void putString(String paramString1, String paramString2) {
        mEditor.putString(paramString1, paramString2);
        mEditor.commit();
    }
}




