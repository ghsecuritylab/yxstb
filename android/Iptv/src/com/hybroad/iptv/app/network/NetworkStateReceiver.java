package com.hybroad.iptv.app.network;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

import java.util.HashSet;
import java.util.Set;

public class NetworkStateReceiver extends BroadcastReceiver {

	private static Set<NetworkStateListener> listeners = new HashSet<NetworkStateListener>();

	public static void addListener(NetworkStateListener listener) 
    {
		listeners.add(listener);
	}

    @Override
    public void onReceive(Context context, Intent intent)
    {
        for (NetworkStateListener listener : listeners) {
            if (listener.stateChanged(context, intent))     
                break;
        }
    }
}