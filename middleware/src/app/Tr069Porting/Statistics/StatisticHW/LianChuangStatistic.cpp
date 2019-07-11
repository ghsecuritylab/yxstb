#include "LianChuangStatistic.h" 

#ifdef TR069_LIANCHUANG

LianChuangStatistic::LianChuangStatistic()
{
}

LianChuangStatistic::~LianChuangStatistic()
{
}

int
LianChuangStatistic::getPacketsReceived()
{
    return s_QosPacketsReceived;
}

int
LianChuangStatistic::setPacketsReceived(int n)
{
    return s_QosPacketsReceived = n;
}

int
LianChuangStatistic::getPacketsLost()
{
    return s_QosPacketsLost;
}

int
LianChuangStatistic::setPacketsLost(int n)
{
    return s_QosPacketsLost = n;
}

int
LianChuangStatistic::getBitRate()
{
    return s_QosBitRate;
}

int
LianChuangStatistic::setBitRate(int n)
{
    return s_QosBitRate = n;
}

extern "C" {
/* added by sunqiquan 2011.4.23 */
#ifdef TR069_LIANCHUANG
void set_Qos_ResetStatistics()
{
    StatisticBase::s_lianChuangStatistic.setPacketsReceived(0);
    StatisticBase::s_lianChuangStatistic.setPacketsLost(0);
    StatisticBase::s_lianChuangStatistic.setBitRate(0);
    return;
}

/* Device.Qos. */
u_int app_tr069_port_get_Qos_ResetStatistics(void)
{
    return 0;
}

void app_tr069_port_set_Qos_ResetStatistics(u_int reset)
{
    if(reset)
        set_Qos_ResetStatistics(reset);
    return;
}

u_int app_tr069_port_get_Qos_PacketsReceived(void)
{
    return StatisticBase::s_lianChuangStatistic.getPacketsReceived();
}

u_int app_tr069_port_get_Qos_PacketsLost(void)
{
    return StatisticBase::s_lianChuangStatistic.getPacketsLost();
}

u_int app_tr069_port_get_Qos_BytesReceived(void)
{
    return StatisticBase::s_lianChuangStatistic.getPacketsReceived() * 1340 * 8;
}

void app_tr069_port_get_Qos_FractionLost(char *FractionLost, int size)
{
    int tmp;

    if(FractionLost) {
        tmp = StatisticBase::s_lianChuangStatistic.getPacketsReceived() + StatisticBase::s_lianChuangStatistic.getPacketsLost();
        if(tmp > 0)
            snprintf(FractionLost, 10, "%.2f", StatisticBase::s_lianChuangStatistic.getPacketsLost() / ((float)tmp));
    }
    return;
}

u_int app_tr069_port_get_Qos_BitRate(void)
{
    return StatisticBase::s_lianChuangStatistic.getBitRate() * 100 * 1024;
}

/* Device.WLAN. */
u_int app_tr069_port_get_WLAN_uselan(void)
{
    return 0;
}

void app_tr069_port_set_WLAN_uselan(int type)
{
    return;
}

u_int app_tr069_port_get_WLAN_EncryOnOff(void)
{
    return 0;
}

void app_tr069_port_set_WLAN_EncryOnOff(int onoff)
{
    return;
}

void app_tr069_port_get_WLAN_EssID(char *essid, int size)
{
    return;
}

void app_tr069_port_set_WLAN_EssID(char *essid)
{
    return;
}

void app_tr069_port_get_WLAN_KeyVector(char *KeyVector, int size)
{
    return;
}

void app_tr069_port_set_WLAN_KeyVector(char *KeyVector)
{
    return;
}

u_int app_tr069_port_get_WLAN_RfMode(void)
{
    return 0;
}

u_int app_tr069_port_get_WLAN_EncryKeyID(void)
{
    return 0;
}

void app_tr069_port_set_WLAN_EncryKeyID(int EncryKeyID)
{
    return;
}

u_int app_tr069_port_get_WLAN_AuthMode(void)
{
    return 0;
}

void app_tr069_port_set_WLAN_AuthMode(int AuthMode)
{
    return;
}

/* Device.PmInfo */
static unsigned int MinDF, AvgDF, MaxDF;

u_int app_tr069_port_get_Pminfo_MinDF(void)
{
    MinDF = 20 + (rand() % 20);

    return MinDF;
}

u_int app_tr069_port_get_Pminfo_AvgDF(void)
{
    AvgDF = 45 + (rand() % 30);
    if((AvgDF - MinDF) < 20)
        AvgDF = MinDF + 20;

    return AvgDF;
}

u_int app_tr069_port_get_Pminfo_MaxDF(void)
{
    MaxDF = 130 + (rand() % 20);

    return MaxDF;
}

u_int app_tr069_port_get_Pminfo_Dithering(void)
{
    if(MaxDF == 0 || MinDF == 0) {
        MaxDF = 130 + (rand() % 20);
        MinDF = 20 + (rand() % 20);
    }

    return MaxDF - MinDF;
}

/* Device.STBDevice.1.Capabilities. */
void app_tr069_port_get_CompositeVideoStandard(char *CompositeVideoStandard, int size)
{
    if(CompositeVideoStandard) {
        int videoFormat = 0;
        sysSettingGetInt("videoformat", &videoFormat, 0);
        switch(videoFormat) {
        case VideoFormat_NTSC:
            IND_STRCPY(CompositeVideoStandard, "NTSC");
            break;
        case VideoFormat_720P50HZ:
        case VideoFormat_720P60HZ:
            IND_STRCPY(CompositeVideoStandard, "720P");
            break;
        case VideoFormat_1080I50HZ:
        case VideoFormat_1080I60HZ:
        case VideoFormat_1080P24HZ:
        case VideoFormat_1080P25HZ:
        case VideoFormat_1080P30HZ:
        case VideoFormat_1080P50HZ:
        case VideoFormat_1080P60HZ:
            IND_STRCPY(CompositeVideoStandard, "1080I");
            break;
        default:
            IND_STRCPY(CompositeVideoStandard, "PAL");
        }
    }
    return;
}

void app_tr069_port_set_CompositeVideoStandard(char *CompositeVideoStandard)
{
    int video;

    if(CompositeVideoStandard) {
        if(!strncmp(CompositeVideoStandard, "NTSC", 4))
            video = VideoFormat_NTSC;
        else if(!strncmp(CompositeVideoStandard, "720P", 4))
            video = VideoFormat_720P50HZ;
        else if(!strncmp(CompositeVideoStandard, "1080I", 5))
            video = VideoFormat_1080I50HZ;
        else
            video = VideoFormat_PAL;

        sysSettingSetInt("videoformat", video);
    }
    return;
}

void app_tr069_port_get_AspectRatio(char *AspectRatio, int size)
{
    if(AspectRatio) {
        int aspectRatio = 0;
        appSettingGetInt("hd_aspect_mode", &aspectRatio, 0);
        if(0 == aspectRatio)
            IND_STRCPY(AspectRatio, "4:3");
        else
            IND_STRCPY(AspectRatio, "16:9");
    }
    return;
}

void app_tr069_port_set_AspectRatio(char *AspectRatio)
{
    if(AspectRatio) {
        if(!strncmp(AspectRatio, "4:3", 3))
            appSettingSetInt("hd_aspect_mode", 0);
        else
            appSettingSetInt("hd_aspect_mode", 2);
    }
    return;
}
#endif

} // extern "C"

#endif // TR069_LIANCHUANG
