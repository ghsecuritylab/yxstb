package com.hybroad.iptv.app.network;

import android.content.Context;
import android.content.Intent;

public interface NetworkStateListener {
    public boolean stateChanged(Context context, Intent intent);
}