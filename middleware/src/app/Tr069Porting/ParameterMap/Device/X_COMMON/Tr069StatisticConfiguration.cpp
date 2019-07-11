
#include "Tr069StatisticConfiguration.h"
#include "Tr069StatisticConfiguration1.h"
#include "Tr069StatisticConfiguration2.h"
#include "Tr069FunctionCall.h"

#include "Tr069.h"

#include <stdio.h>
#include <stdlib.h>

Tr069Call* g_tr069StatisticConfiguration = new Tr069StatisticConfig();

/*------------------------------------------------------------------------------
   fun1. 
 ------------------------------------------------------------------------------*/
static int getTr069PortLogenable(char* value, unsigned int size)
{
    return tr069_statistic_getConfiguration("Logenable", value, size);
}

static int setTr069PortLogenable(char* value, unsigned int size)
{
    return tr069_statistic_setConfiguration("Logenable", value, size);
}


/*------------------------------------------------------------------------------
   fun2. 
 ------------------------------------------------------------------------------*/
static int getTr069PortAESLogServerUrl(char* value, unsigned int size)
{
    return tr069_statistic_getConfiguration("LogServerUrl", value, size);
}

static int setTr069PortLogServerUrl(char* value, unsigned int size)
{
    return tr069_statistic_setConfiguration("LogServerUrl", value, size);
}

/*------------------------------------------------------------------------------
   fun3. 
  性能监测参数文件上报间隔
	单位：s 默认：3600（即1小时）
	该参数设置为0时，表示关闭性能监测参数文件上报功能
 ------------------------------------------------------------------------------*/
static int getTr069PortLogUploadInterval(char* value, unsigned int size)
{
    return tr069_statistic_getConfiguration("LogUploadInterval", value, size);
}

static int setTr069PortLogUploadInterval(char* value, unsigned int size)
{
    return tr069_statistic_setConfiguration("LogUploadInterval", value, size);
}

/*------------------------------------------------------------------------------
   fun4. 
	性能监测统计参数的记录周期时长
	单位：s 默认：3600（即1小时）
	统计起始为每次开机，到设定的统计周期时长后，启动新的统计周期。
	如不到设定的统计周期时长就关机，则结束这个周期。
	在启动新的统计周期时，应把前个周期的记录数据上传到网管平台
 ------------------------------------------------------------------------------*/
static int getTr069PortLogRecordInterval(char* value, unsigned int size)
{
    return tr069_statistic_getConfiguration("LogRecordInterval", value, size);
}

static int setTr069PortLogRecordInterval(char* value, unsigned int size)
{
    return tr069_statistic_setConfiguration("LogRecordInterval", value, size);
}

/*------------------------------------------------------------------------------
   fun5. 
 ------------------------------------------------------------------------------*/
static int getTr069PortIsFileorRealTime(char* value, unsigned int size)
{
	  //tr069__get_IsFileorRealTime,返回0
    snprintf(value, size, "0"); 
    
    return 0;
}

static int setTr069PortIsFileorRealTime(char* value, unsigned int size)
{
    //app_tr069_port_set_IsFileorRealTime
    
    return 0;
}

/*------------------------------------------------------------------------------
   fun6. 
 ------------------------------------------------------------------------------*/
static int getTr069PortStatInterval(char* value, unsigned int size)
{
    return tr069_statistic_getConfiguration("StatInterval", value, size);
}

static int setTr069PortStatInterval(char* value, unsigned int size)
{
    return tr069_statistic_setConfiguration("StatInterval", value, size);
}

/*------------------------------------------------------------------------------
   fun7. 
 ------------------------------------------------------------------------------*/
static int getTr069PortPacketsLostR1(char* value, unsigned int size)
{
    return tr069_statistic_getConfiguration("PacketsLostR1", value, size);
}

static int setTr069PortPacketsLostR1(char* value, unsigned int size)
{
    return tr069_statistic_setConfiguration("PacketsLostR1", value, size);
}

/*------------------------------------------------------------------------------
   fun8. 
 ------------------------------------------------------------------------------*/
static int getTr069PortPacketsLostR2(char* value, unsigned int size)
{
    return tr069_statistic_getConfiguration("PacketsLostR2", value, size);
}

static int setTr069PortPacketsLostR2(char* value, unsigned int size)
{
    return tr069_statistic_setConfiguration("PacketsLostR2", value, size);
}

/*------------------------------------------------------------------------------
   fun9. 
 ------------------------------------------------------------------------------*/
static int getTr069PortPacketsLostR3(char* value, unsigned int size)
{
    return tr069_statistic_getConfiguration("PacketsLostR3", value, size);
}

static int setTr069PortPacketsLostR3(char* value, unsigned int size)
{
    return tr069_statistic_setConfiguration("PacketsLostR3", value, size);
}

/*------------------------------------------------------------------------------
   fun10. 
 ------------------------------------------------------------------------------*/
static int getTr069PortPacketsLostR4(char* value, unsigned int size)
{
    return tr069_statistic_getConfiguration("PacketsLostR4", value, size);
}

static int setTr069PortPacketsLostR4(char* value, unsigned int size)
{
    return tr069_statistic_setConfiguration("PacketsLostR4", value, size);
}

/*------------------------------------------------------------------------------
   fun11. 
 ------------------------------------------------------------------------------*/
static int getTr069PortPacketsLostR5(char* value, unsigned int size)
{
    return tr069_statistic_getConfiguration("PacketsLostR5", value, size);
}

static int setTr069PortPacketsLostR5(char* value, unsigned int size)
{
    return tr069_statistic_setConfiguration("PacketsLostR5", value, size);
}

/*------------------------------------------------------------------------------
   fun12. 
 ------------------------------------------------------------------------------*/
static int getTr069PortHD_PacketsLostR1(char* value, unsigned int size)
{
    return tr069_statistic_getConfiguration("HD_PacketsLostR1", value, size);
}

static int setTr069PortHD_PacketsLostR1(char* value, unsigned int size)
{
    return tr069_statistic_setConfiguration("HD_PacketsLostR1", value, size);
}

/*------------------------------------------------------------------------------
   fun13. 
 ------------------------------------------------------------------------------*/
static int getTr069PortHD_PacketsLostR2(char* value, unsigned int size)
{
    return tr069_statistic_getConfiguration("HD_PacketsLostR2", value, size);
}

static int setTr069PortHD_PacketsLostR2(char* value, unsigned int size)
{
    return tr069_statistic_setConfiguration("HD_PacketsLostR2", value, size);
}

/*------------------------------------------------------------------------------
   fun14. 
 ------------------------------------------------------------------------------*/
static int getTr069PortHD_PacketsLostR3(char* value, unsigned int size)
{
    return tr069_statistic_getConfiguration("HD_PacketsLostR3", value, size);
}

static int setTr069PortHD_PacketsLostR3(char* value, unsigned int size)
{
    return tr069_statistic_setConfiguration("HD_PacketsLostR3", value, size);
}

/*------------------------------------------------------------------------------
   fun15. 
 ------------------------------------------------------------------------------*/
static int getTr069PortHD_PacketsLostR4(char* value, unsigned int size)
{
    return tr069_statistic_getConfiguration("HD_PacketsLostR4", value, size);
}

static int setTr069PortHD_PacketsLostR4(char* value, unsigned int size)
{
    return tr069_statistic_setConfiguration("HD_PacketsLostR4", value, size);
}

/*------------------------------------------------------------------------------
   fun16. 
 ------------------------------------------------------------------------------*/
static int getTr069PortHD_PacketsLostR5(char* value, unsigned int size)
{
    return tr069_statistic_getConfiguration("HD_PacketsLostR5", value, size);
}

static int setTr069PortHD_PacketsLostR5(char* value, unsigned int size)
{
    return tr069_statistic_setConfiguration("HD_PacketsLostR5", value, size);
}

/*------------------------------------------------------------------------------
   fun17. 
 ------------------------------------------------------------------------------*/
static int getTr069PortBitRateRangeR1(char* value, unsigned int size)
{
    return tr069_statistic_getConfiguration("BitRateR1", value, size);
}

static int setTr069PortBitRateRangeR1(char* value, unsigned int size)
{
    return tr069_statistic_setConfiguration("BitRateR1", value, size);
}

/*------------------------------------------------------------------------------
   fun18. 
 ------------------------------------------------------------------------------*/
static int getTr069PortBitRateRangeR2(char* value, unsigned int size)
{
    return tr069_statistic_getConfiguration("BitRateR2", value, size);
}

static int setTr069PortBitRateRangeR2(char* value, unsigned int size)
{
    return tr069_statistic_setConfiguration("BitRateR2", value, size);
}

/*------------------------------------------------------------------------------
   fun19. 
 ------------------------------------------------------------------------------*/
static int getTr069PortBitRateRangeR3(char* value, unsigned int size)
{
    return tr069_statistic_getConfiguration("BitRateR3", value, size);
}

static int setTr069PortBitRateRangeR3(char* value, unsigned int size)
{
    return tr069_statistic_setConfiguration("BitRateR3", value, size);
}

/*------------------------------------------------------------------------------
   fun20. 
 ------------------------------------------------------------------------------*/
static int getTr069PortBitRateRangeR4(char* value, unsigned int size)
{
    return tr069_statistic_getConfiguration("BitRateR4", value, size);
}

static int setTr069PortBitRateRangeR4(char* value, unsigned int size)
{
    return tr069_statistic_setConfiguration("BitRateR4", value, size);
}

/*------------------------------------------------------------------------------
   fun21. 
 ------------------------------------------------------------------------------*/
static int getTr069PortBitRateRangeR5(char* value, unsigned int size)
{
    return tr069_statistic_getConfiguration("BitRateR5", value, size);
}

static int setTr069PortBitRateRangeR5(char* value, unsigned int size)
{
    return tr069_statistic_setConfiguration("BitRateR5", value, size);
}

/*------------------------------------------------------------------------------
   fun22. 
 ------------------------------------------------------------------------------*/
static int getTr069PortHD_BitRateR1(char* value, unsigned int size)
{
    return tr069_statistic_getConfiguration("HD_BitRateR1", value, size);
}

static int setTr069PortHD_BitRateR1(char* value, unsigned int size)
{
    return tr069_statistic_setConfiguration("HD_BitRateR1", value, size);
}

/*------------------------------------------------------------------------------
   fun23. 
 ------------------------------------------------------------------------------*/
static int getTr069PortHD_BitRateR2(char* value, unsigned int size)
{
    return tr069_statistic_getConfiguration("HD_BitRateR2", value, size);
}

static int setTr069PortHD_BitRateR2(char* value, unsigned int size)
{
    return tr069_statistic_setConfiguration("HD_BitRateR2", value, size);
}

/*------------------------------------------------------------------------------
   fun24. 
 ------------------------------------------------------------------------------*/
static int getTr069PortHD_BitRateR3(char* value, unsigned int size)
{
    return tr069_statistic_getConfiguration("HD_BitRateR3", value, size);
}

static int setTr069PortHD_BitRateR3(char* value, unsigned int size)
{
    return tr069_statistic_setConfiguration("HD_BitRateR3", value, size);
}

/*------------------------------------------------------------------------------
   fun25. 
 ------------------------------------------------------------------------------*/
static int getTr069PortHD_BitRateR4(char* value, unsigned int size)
{
    return tr069_statistic_getConfiguration("HD_BitRateR4", value, size);
}

static int setTr069PortHD_BitRateR4(char* value, unsigned int size)
{
    return tr069_statistic_setConfiguration("HD_BitRateR4", value, size);
}

/*------------------------------------------------------------------------------
   fun26. 
 ------------------------------------------------------------------------------*/
static int getTr069PortHD_BitRateR5(char* value, unsigned int size)
{
    return tr069_statistic_getConfiguration("HD_BitRateR5", value, size);
}

static int setTr069PortHD_BitRateR5(char* value, unsigned int size)
{
    return tr069_statistic_setConfiguration("HD_BitRateR5", value, size);
}

/*------------------------------------------------------------------------------
   fun27. 
	丢包率范围1起始
	以0.01%为单位
	例如： 1 表示0.01%
	5 表示0.05%
	默认值：0
 ------------------------------------------------------------------------------*/
static int getTr069PortPacketsLostR1From(char* value, unsigned int size)
{
    return tr069_statistic_getConfiguration("PacketsLostR1From", value, size);
}

static int setTr069PortPacketsLostR1From(char* value, unsigned int size)
{
    return tr069_statistic_setConfiguration("PacketsLostR1From", value, size);
}

/*------------------------------------------------------------------------------
   fun28. 
	丢包率范围1结束
	以0.01%为单位
	例如： 1 表示0.01%
	5 表示0.05%
	9999 表示最大
	默认值：0
 ------------------------------------------------------------------------------*/
static int getTr069PortPacketsLostR1Till(char* value, unsigned int size)
{
    return tr069_statistic_getConfiguration("PacketsLostR1Till", value, size);
}

static int setTr069PortPacketsLostR1Till(char* value, unsigned int size)
{
    return tr069_statistic_setConfiguration("PacketsLostR1Till", value, size);
}

/*------------------------------------------------------------------------------
   fun29. 
	丢包率范围2－起始
	以0.01%为单位
	默认值：0
 ------------------------------------------------------------------------------*/
static int getTr069PortPacketsLostR2From(char* value, unsigned int size)
{
    return tr069_statistic_getConfiguration("PacketsLostR2From", value, size);
}

static int setTr069PortPacketsLostR2From(char* value, unsigned int size)
{
    return tr069_statistic_setConfiguration("PacketsLostR2From", value, size);
}

/*------------------------------------------------------------------------------
   fun30. 
	丢包率范围2－结束
	以0.01%为单位
	默认值：10
 ------------------------------------------------------------------------------*/
static int getTr069PortPacketsLostR2Till(char* value, unsigned int size)
{
    return tr069_statistic_getConfiguration("PacketsLostR2Till", value, size);
}

static int setTr069PortPacketsLostR2Till(char* value, unsigned int size)
{
    return tr069_statistic_setConfiguration("PacketsLostR2Till", value, size);
}

/*------------------------------------------------------------------------------
   fun31. 
	丢包率范围3－起始
	以0.01%为单位
	默认值：10
 ------------------------------------------------------------------------------*/
static int getTr069PortPacketsLostR3From(char* value, unsigned int size)
{
    return tr069_statistic_getConfiguration("PacketsLostR3From", value, size);
}

static int setTr069PortPacketsLostR3From(char* value, unsigned int size)
{
    return tr069_statistic_setConfiguration("PacketsLostR3From", value, size);
}

/*------------------------------------------------------------------------------
   fun32. 
	丢包率范围3－结束
	以0.01%为单位
	默认值：20
 ------------------------------------------------------------------------------*/
static int getTr069PortPacketsLostR3Till(char* value, unsigned int size)
{
    return tr069_statistic_getConfiguration("PacketsLostR3Till", value, size);
}

static int setTr069PortPacketsLostR3Till(char* value, unsigned int size)
{
    return tr069_statistic_setConfiguration("PacketsLostR3Till", value, size);
}

/*------------------------------------------------------------------------------
   fun33. 
	丢包率范围4－起始
	以0.01%为单位
	默认值：20
 ------------------------------------------------------------------------------*/
static int getTr069PortPacketsLostR4From(char* value, unsigned int size)
{
    return tr069_statistic_getConfiguration("PacketsLostR4From", value, size);
}

static int setTr069PortPacketsLostR4From(char* value, unsigned int size)
{
    return tr069_statistic_setConfiguration("PacketsLostR4From", value, size);
}

/*------------------------------------------------------------------------------
   fun34. 
	丢包率范围4－结束
	以0.01%为单位
	默认值：50
 ------------------------------------------------------------------------------*/
static int getTr069PortPacketsLostR4Till(char* value, unsigned int size)
{
    return tr069_statistic_getConfiguration("PacketsLostR4Till", value, size);
}

static int setTr069PortPacketsLostR4Till(char* value, unsigned int size)
{
    return tr069_statistic_setConfiguration("PacketsLostR4Till", value, size);
}

/*------------------------------------------------------------------------------
   fun35. 
	丢包率范围5－起始
	以0.01%为单位
	默认值：50
 ------------------------------------------------------------------------------*/
static int getTr069PortPacketsLostR5From(char* value, unsigned int size)
{
    return tr069_statistic_getConfiguration("PacketsLostR5From", value, size);
}

static int setTr069PortPacketsLostR5From(char* value, unsigned int size)
{
    return tr069_statistic_setConfiguration("PacketsLostR5From", value, size);
}

/*------------------------------------------------------------------------------
   fun36. 
	丢包率范围5－结束
	以0.01%为单位
	默认值：9999
 ------------------------------------------------------------------------------*/
static int getTr069PortPacketsLostR5Till(char* value, unsigned int size)
{
    return tr069_statistic_getConfiguration("PacketsLostR5Till", value, size);
}

static int setTr069PortPacketsLostR5Till(char* value, unsigned int size)
{
    return tr069_statistic_setConfiguration("PacketsLostR5Till", value, size);
}

/*------------------------------------------------------------------------------
   fun37. 
	实时媒体速率范围1－起始
	以100kbps为单位
	例如：10 表示1Mbps
	13 表示1.3Mbps
	默认值：16
 ------------------------------------------------------------------------------*/
static int getTr069PortBitRateR1From(char* value, unsigned int size)
{
    return tr069_statistic_getConfiguration("BitRateR1From", value, size);
}

static int setTr069PortBitRateR1From(char* value, unsigned int size)
{
    return tr069_statistic_getConfiguration("BitRateR1From", value, size);
}

/*------------------------------------------------------------------------------
   fun38. 
	实时媒体速率范围1－结束
	以100kbps为单位
	例如： 10 表示1Mbps
	13 表示1.3Mbps
	9999 表示不限速
	默认值：9999
 ------------------------------------------------------------------------------*/
static int getTr069PortBitRateR1Till(char* value, unsigned int size)
{
    return tr069_statistic_getConfiguration("BitRateR1Till", value, size);
}

static int setTr069PortBitRateR1Till(char* value, unsigned int size)
{
    return tr069_statistic_setConfiguration("BitRateR1Till", value, size);
}

/*------------------------------------------------------------------------------
   fun39. 
	实时媒体速率范围2－起始
	以100kbps为单位
	默认值：14
 ------------------------------------------------------------------------------*/
static int getTr069PortBitRateR2From(char* value, unsigned int size)
{
    return tr069_statistic_getConfiguration("BitRateR2From", value, size);
}

static int setTr069PortBitRateR2From(char* value, unsigned int size)
{
    return tr069_statistic_setConfiguration("BitRateR2From", value, size);
}

/*------------------------------------------------------------------------------
   fun40. 
	实时媒体速率范围2－结束
	以100kbps为单位
	默认值：16
 ------------------------------------------------------------------------------*/
static int getTr069PortBitRateR2Till(char* value, unsigned int size)
{
    return tr069_statistic_getConfiguration("BitRateR2Till", value, size);
}

static int setTr069PortBitRateR2Till(char* value, unsigned int size)
{
    return tr069_statistic_setConfiguration("BitRateR2Till", value, size);
}

/*------------------------------------------------------------------------------
   fun41. 
	实时媒体速率范围3－起始
	以100kbps为单位
	默认值：12
 ------------------------------------------------------------------------------*/
static int getTr069PortBitRateR3From(char* value, unsigned int size)
{
    return tr069_statistic_getConfiguration("BitRateR3From", value, size);
}

static int setTr069PortBitRateR3From(char* value, unsigned int size)
{
    return tr069_statistic_setConfiguration("BitRateR3From", value, size);
}

/*------------------------------------------------------------------------------
   fun42. 
	实时媒体速率范围3－结束
	以100kbps为单位
	默认值：14
 ------------------------------------------------------------------------------*/
static int getTr069PortBitRateR3Till(char* value, unsigned int size)
{
    return tr069_statistic_getConfiguration("BitRateR3Till", value, size);
}

static int setTr069PortBitRateR3Till(char* value, unsigned int size)
{
    return tr069_statistic_setConfiguration("BitRateR3Till", value, size);
}

/*------------------------------------------------------------------------------
   fun43. 
	实时媒体速率范围4－起始
	以100kbps为单位
	默认值：8
 ------------------------------------------------------------------------------*/
static int getTr069PortBitRateR4From(char* value, unsigned int size)
{
    return tr069_statistic_getConfiguration("BitRateR4From", value, size);
}

static int setTr069PortBitRateR4From(char* value, unsigned int size)
{
    return tr069_statistic_setConfiguration("BitRateR4From", value, size);
}

/*------------------------------------------------------------------------------
   fun44. 
	实时媒体速率范围4－结束
	以100kbps为单位
	默认值：12
 ------------------------------------------------------------------------------*/
static int getTr069PortBitRateR4Till(char* value, unsigned int size)
{
    return tr069_statistic_getConfiguration("BitRateR4Till", value, size);
}

static int setTr069PortBitRateR4Till(char* value, unsigned int size)
{
    return tr069_statistic_setConfiguration("BitRateR4Till", value, size);
}

/*------------------------------------------------------------------------------
   fun45. 
	实时媒体速率范围5－起始
	以100kbps为单位
	默认值：0
 ------------------------------------------------------------------------------*/
static int getTr069PortBitRateR5From(char* value, unsigned int size)
{
    return tr069_statistic_getConfiguration("BitRateR5From", value, size);
}

static int setTr069PortBitRateR5From(char* value, unsigned int size)
{
    return tr069_statistic_setConfiguration("BitRateR5From", value, size);
}

/*------------------------------------------------------------------------------
   fun46. 
	实时媒体速率范围5－结束
	以100kbps为单位
	默认值：8
 ------------------------------------------------------------------------------*/
static int getTr069PortBitRateR5Till(char* value, unsigned int size)
{
    return tr069_statistic_getConfiguration("BitRateR5Till", value, size);
}

static int setTr069PortBitRateR5Till(char* value, unsigned int size)
{
    return tr069_statistic_setConfiguration("BitRateR5Till", value, size);
}





/*------------------------------------------------------------------------------
   CU FramesLostR1
 ------------------------------------------------------------------------------*/
static int getTr069PortFramesLostR1(char* value, unsigned int size)
{
    return tr069_statistic_getConfiguration("FramesLostR1", value, size);
}

static int setTr069PortFramesLostR1(char* value, unsigned int size)
{
    return tr069_statistic_setConfiguration("FramesLostR1", value, size);
}


/*------------------------------------------------------------------------------
   CU FramesLostR2
 ------------------------------------------------------------------------------*/
static int getTr069PortFramesLostR2(char* value, unsigned int size)
{
    return tr069_statistic_getConfiguration("FramesLostR2", value, size);
}

static int setTr069PortFramesLostR2(char* value, unsigned int size)
{
    return tr069_statistic_setConfiguration("FramesLostR2", value, size);
}


/*------------------------------------------------------------------------------
   CU FramesLostR3
 ------------------------------------------------------------------------------*/
static int getTr069PortFramesLostR3(char* value, unsigned int size)
{
    return tr069_statistic_getConfiguration("FramesLostR3", value, size);
}

static int setTr069PortFramesLostR3(char* value, unsigned int size)
{
    return tr069_statistic_setConfiguration("FramesLostR3", value, size);
}


/*------------------------------------------------------------------------------
   CU FramesLostR4
 ------------------------------------------------------------------------------*/
static int getTr069PortFramesLostR4(char* value, unsigned int size)
{
    return tr069_statistic_getConfiguration("FramesLostR4", value, size);
}

static int setTr069PortFramesLostR4(char* value, unsigned int size)
{
    return tr069_statistic_setConfiguration("FramesLostR4", value, size);
}


/*------------------------------------------------------------------------------
   CU FramesLostR5
 ------------------------------------------------------------------------------*/
static int getTr069PortFramesLostR5(char* value, unsigned int size)
{
    return tr069_statistic_getConfiguration("FramesLostR5", value, size);
}

static int setTr069PortFramesLostR5(char* value, unsigned int size)
{
    return tr069_statistic_setConfiguration("FramesLostR5", value, size);
}





/*------------------------------------------------------------------------------
 *  以下对象的注册到表root.XXX.StatisticConfiguration
 *  XXX可以是X_CTC_IPTV，X_00E0FC
 ------------------------------------------------------------------------------*/
Tr069StatisticConfig::Tr069StatisticConfig()
	: Tr069GroupCall("StatisticConfiguration")
{
    //Tr069Call* objPrimaryParam = new StatisticConfiguration1();
    //regist(objPrimaryParam->name(), objPrimaryParam);

    //Tr069Call* objSecondaryParam = new StatisticConfiguration2();
    //regist(objSecondaryParam->name(), objSecondaryParam);

    // Log  HW,CTC,CU 共有 fun2 - fun4
    Tr069Call* fun2  = new Tr069FunctionCall("LogServerUrl", getTr069PortAESLogServerUrl, setTr069PortLogServerUrl);
    regist(fun2->name(), fun2);    
    
    Tr069Call* fun3  = new Tr069FunctionCall("LogUploadInterval", getTr069PortLogUploadInterval, setTr069PortLogUploadInterval);
    regist(fun3->name(), fun3);    
    
    Tr069Call* fun4  = new Tr069FunctionCall("LogRecordInterval", getTr069PortLogRecordInterval, setTr069PortLogRecordInterval);
    regist(fun4->name(), fun4);    
    
    
    // Configuration HW,CTC,CU 共有 fun6 -21
    Tr069Call* fun6  = new Tr069FunctionCall("StatInterval", getTr069PortStatInterval, setTr069PortStatInterval);
    regist(fun6->name(), fun6);    
    
    Tr069Call* fun7  = new Tr069FunctionCall("PacketsLostR1", getTr069PortPacketsLostR1, setTr069PortPacketsLostR1);
    regist(fun7->name(), fun7);    
    
    Tr069Call* fun8  = new Tr069FunctionCall("PacketsLostR2", getTr069PortPacketsLostR2, setTr069PortPacketsLostR2);
    regist(fun8->name(), fun8);       
    
    Tr069Call* fun9  = new Tr069FunctionCall("PacketsLostR3", getTr069PortPacketsLostR3, setTr069PortPacketsLostR3);
    regist(fun9->name(), fun9);    
    
    Tr069Call* fun10  = new Tr069FunctionCall("PacketsLostR4", getTr069PortPacketsLostR4, setTr069PortPacketsLostR4);
    regist(fun10->name(), fun10);    
    
    Tr069Call* fun11  = new Tr069FunctionCall("PacketsLostR5", getTr069PortPacketsLostR5, setTr069PortPacketsLostR5);
    regist(fun11->name(), fun11);    
    
    

    Tr069Call* fun17  = new Tr069FunctionCall("BitRateR1", getTr069PortBitRateRangeR1, setTr069PortBitRateRangeR1);
    regist(fun17->name(), fun17);    
    
    Tr069Call* fun18  = new Tr069FunctionCall("BitRateR2", getTr069PortBitRateRangeR2, setTr069PortBitRateRangeR2);
    regist(fun18->name(), fun18);    
    
    Tr069Call* fun19  = new Tr069FunctionCall("BitRateR3", getTr069PortBitRateRangeR3, setTr069PortBitRateRangeR3);
    regist(fun19->name(), fun19);    
    
    Tr069Call* fun20  = new Tr069FunctionCall("BitRateR4", getTr069PortBitRateRangeR4, setTr069PortBitRateRangeR4);
    regist(fun20->name(), fun20);    
    
    Tr069Call* fun21  = new Tr069FunctionCall("BitRateR5", getTr069PortBitRateRangeR5, setTr069PortBitRateRangeR5);
    regist(fun21->name(), fun21);         

        


    // PacketsLost HW,CTC 共有fun27 -fun46
    Tr069Call* fun27  = new Tr069FunctionCall("PacketsLostR1From", getTr069PortPacketsLostR1From, setTr069PortPacketsLostR1From);
    regist(fun27->name(), fun27);

    Tr069Call* fun28  = new Tr069FunctionCall("PacketsLostR1Till", getTr069PortPacketsLostR1Till, setTr069PortPacketsLostR1Till);
    regist(fun28->name(), fun28);             

    Tr069Call* fun29  = new Tr069FunctionCall("PacketsLostR2From", getTr069PortPacketsLostR2From, setTr069PortPacketsLostR2From);
    regist(fun29->name(), fun29);

    Tr069Call* fun30  = new Tr069FunctionCall("PacketsLostR2Till", getTr069PortPacketsLostR2Till, setTr069PortPacketsLostR2Till);
    regist(fun30->name(), fun30);             

    Tr069Call* fun31  = new Tr069FunctionCall("PacketsLostR3From", getTr069PortPacketsLostR3From, setTr069PortPacketsLostR3From);
    regist(fun31->name(), fun31);

    Tr069Call* fun32  = new Tr069FunctionCall("PacketsLostR3Till", getTr069PortPacketsLostR3Till, setTr069PortPacketsLostR3Till);
    regist(fun32->name(), fun32);             

    Tr069Call* fun33  = new Tr069FunctionCall("PacketsLostR4From", getTr069PortPacketsLostR4From, setTr069PortPacketsLostR4From);
    regist(fun33->name(), fun33);

    Tr069Call* fun34  = new Tr069FunctionCall("PacketsLostR4Till", getTr069PortPacketsLostR4Till, setTr069PortPacketsLostR4Till);
    regist(fun34->name(), fun34);

    Tr069Call* fun35  = new Tr069FunctionCall("PacketsLostR5From", getTr069PortPacketsLostR5From, setTr069PortPacketsLostR5From);
    regist(fun35->name(), fun35);             

    Tr069Call* fun36  = new Tr069FunctionCall("PacketsLostR5Till", getTr069PortPacketsLostR5Till, setTr069PortPacketsLostR5Till);
    regist(fun36->name(), fun36);

    // BitRate  HW,CTC 共有fun27 -fun46
    Tr069Call* fun37  = new Tr069FunctionCall("BitRateR1From", getTr069PortBitRateR1From, setTr069PortBitRateR1From);
    regist(fun37->name(), fun37);

    Tr069Call* fun38  = new Tr069FunctionCall("BitRateR1Till", getTr069PortBitRateR1Till, setTr069PortBitRateR1Till);
    regist(fun38->name(), fun38);             

    Tr069Call* fun39  = new Tr069FunctionCall("BitRateR2From", getTr069PortBitRateR2From, setTr069PortBitRateR2From);
    regist(fun39->name(), fun39);

    Tr069Call* fun40  = new Tr069FunctionCall("BitRateR2Till", getTr069PortBitRateR2Till, setTr069PortBitRateR2Till);
    regist(fun40->name(), fun40);             

    Tr069Call* fun41  = new Tr069FunctionCall("BitRateR3From", getTr069PortBitRateR3From, setTr069PortBitRateR3From);
    regist(fun41->name(), fun41);

    Tr069Call* fun42  = new Tr069FunctionCall("BitRateR3Till", getTr069PortBitRateR3Till, setTr069PortBitRateR3Till);
    regist(fun42->name(), fun42);             

    Tr069Call* fun43  = new Tr069FunctionCall("BitRateR4From", getTr069PortBitRateR4From, setTr069PortBitRateR4From);
    regist(fun43->name(), fun43);

    Tr069Call* fun44  = new Tr069FunctionCall("BitRateR4Till", getTr069PortBitRateR4Till, setTr069PortBitRateR4Till);
    regist(fun44->name(), fun44);

    Tr069Call* fun45  = new Tr069FunctionCall("BitRateR5From", getTr069PortBitRateR5From, setTr069PortBitRateR5From);
    regist(fun45->name(), fun45);             

    Tr069Call* fun46  = new Tr069FunctionCall("BitRateR5Till", getTr069PortBitRateR5Till, setTr069PortBitRateR5Till);
    regist(fun46->name(), fun46);

    // CTC 独有 fun60 - fun71
    Tr069Call* fun60  = new Tr069FunctionCall("Enable", getTr069PortLogenable, setTr069PortLogenable);
    regist(fun60->name(), fun60);   

    Tr069Call* fun61  = new Tr069FunctionCall("IsFileorRealTime", getTr069PortIsFileorRealTime, setTr069PortIsFileorRealTime);
    regist(fun61->name(), fun61);  

    Tr069Call* fun62  = new Tr069FunctionCall("HD_PacketsLostR1", getTr069PortHD_PacketsLostR1, setTr069PortHD_PacketsLostR1);
    regist(fun62->name(), fun62);    
    
    Tr069Call* fun63  = new Tr069FunctionCall("HD_PacketsLostR2", getTr069PortHD_PacketsLostR2, setTr069PortHD_PacketsLostR2);
    regist(fun63->name(), fun63);    
    
    Tr069Call* fun64  = new Tr069FunctionCall("HD_PacketsLostR3", getTr069PortHD_PacketsLostR3, setTr069PortHD_PacketsLostR3);
    regist(fun64->name(), fun64);    
    
    Tr069Call* fun65  = new Tr069FunctionCall("HD_PacketsLostR4", getTr069PortHD_PacketsLostR4, setTr069PortHD_PacketsLostR4);
    regist(fun65->name(), fun65);    
    
    Tr069Call* fun66  = new Tr069FunctionCall("HD_PacketsLostR5", getTr069PortHD_PacketsLostR5, setTr069PortHD_PacketsLostR5);
    regist(fun66->name(), fun66);  

    Tr069Call* fun67  = new Tr069FunctionCall("HD_BitRateR1", getTr069PortHD_BitRateR1, setTr069PortHD_BitRateR1);
    regist(fun67->name(), fun67);    
    
    Tr069Call* fun68  = new Tr069FunctionCall("HD_BitRateR2", getTr069PortHD_BitRateR2, setTr069PortHD_BitRateR2);
    regist(fun68->name(), fun68);    
    
    Tr069Call* fun69  = new Tr069FunctionCall("HD_BitRateR3", getTr069PortHD_BitRateR3, setTr069PortHD_BitRateR3);
    regist(fun69->name(), fun69);    
    
    Tr069Call* fun70  = new Tr069FunctionCall("HD_BitRateR4", getTr069PortHD_BitRateR4, setTr069PortHD_BitRateR4);
    regist(fun70->name(), fun70);    
    
    Tr069Call* fun71  = new Tr069FunctionCall("HD_BitRateR5", getTr069PortHD_BitRateR5, setTr069PortHD_BitRateR5);
    regist(fun71->name(), fun71);       
    
    // CU 独有 fun48 - fun52
    Tr069Call* fun48  = new Tr069FunctionCall("FramesLostR1", getTr069PortFramesLostR1, setTr069PortFramesLostR1);
    regist(fun48->name(), fun48);    
    
    Tr069Call* fun49  = new Tr069FunctionCall("FramesLostR2", getTr069PortFramesLostR2, setTr069PortFramesLostR2);
    regist(fun49->name(), fun49);    
    
    Tr069Call* fun50  = new Tr069FunctionCall("FramesLostR3", getTr069PortFramesLostR3, setTr069PortFramesLostR3);
    regist(fun50->name(), fun50);    
    
    Tr069Call* fun51  = new Tr069FunctionCall("FramesLostR4", getTr069PortFramesLostR4, setTr069PortFramesLostR4);
    regist(fun51->name(), fun51);    
    
    Tr069Call* fun52  = new Tr069FunctionCall("FramesLostR5", getTr069PortFramesLostR5, setTr069PortFramesLostR5);
    regist(fun52->name(), fun52);     

}

Tr069StatisticConfig::~Tr069StatisticConfig()
{
}


