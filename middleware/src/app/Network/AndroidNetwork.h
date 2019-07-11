#ifndef __ANDROIDNETWORK__H_
#define __ANDROIDNETWORK__H_

void AndroidNetworkStateChanaged(const char* ifname);

//see AndroidNetworkJNI.cpp
int getAndroidNetworkInfo(const char* iface, const char* name, char* value, int len);

#endif
