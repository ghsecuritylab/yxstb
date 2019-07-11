#ifndef _LianChuangStatistic_H_
#define _LianChuangStatistic_H_

#ifdef TR069_LIANCHUANG
 
#ifdef __cplusplus

class LianChuangStatistic{

public:
    LianChuangStatistic();
    ~LianChuangStatistic();

    int getPacketsReceived();
    int setPacketsReceived(int n);
    int getPacketsLost();
    int setPacketsLost(int n);
    int getBitRate();
    int setBitRate(int n);

protected:
    static int s_QosPacketsReceived;
    static int s_QosPacketsLost;
    static int s_QosBitRate;
};

extern "C"{
void set_Qos_ResetStatistics();
unsigned int app_tr069_port_get_Qos_ResetStatistics(void);
void app_tr069_port_set_Qos_ResetStatistics(unsigned int reset);
unsigned int app_tr069_port_get_Qos_PacketsReceived(void);
unsigned int app_tr069_port_get_Qos_PacketsLost(void);
unsigned int app_tr069_port_get_Qos_BytesReceived(void);
void app_tr069_port_get_Qos_FractionLost(char *FractionLost, int size);
unsigned int app_tr069_port_get_Qos_BitRate(void);
unsigned int app_tr069_port_get_WLAN_uselan(void);
void app_tr069_port_set_WLAN_uselan(int type);
unsigned int app_tr069_port_get_WLAN_EncryOnOff(void);
void app_tr069_port_set_WLAN_EncryOnOff(int onoff);
void app_tr069_port_get_WLAN_EssID(char *essid, int size);
void app_tr069_port_set_WLAN_EssID(char *essid);
void app_tr069_port_get_WLAN_KeyVector(char *KeyVector, int size);
void app_tr069_port_set_WLAN_KeyVector(char *KeyVector);
unsigned int app_tr069_port_get_WLAN_RfMode(void);
unsigned int app_tr069_port_get_WLAN_EncryKeyID(void);
void app_tr069_port_set_WLAN_EncryKeyID(int EncryKeyID);
unsigned int app_tr069_port_get_WLAN_AuthMode(void);
void app_tr069_port_set_WLAN_AuthMode(int AuthMode);
unsigned int app_tr069_port_get_Pminfo_MinDF(void);
unsigned int app_tr069_port_get_Pminfo_AvgDF(void);
unsigned int app_tr069_port_get_Pminfo_MaxDF(void);
unsigned int app_tr069_port_get_Pminfo_Dithering(void);
void app_tr069_port_get_CompositeVideoStandard(char *CompositeVideoStandard, int size);
void app_tr069_port_set_CompositeVideoStandard(char *CompositeVideoStandard);
void app_tr069_port_get_AspectRatio(char *AspectRatio, int size);
void app_tr069_port_set_AspectRatio(char *AspectRatio);

} // extern "C"
#endif // __cplusplus

#endif // TR069_LIANCHUANG

#endif // _LianChuangStatistic_H_
