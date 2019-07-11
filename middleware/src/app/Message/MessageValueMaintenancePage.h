#ifndef _MessageValueMaintenancePage_H_
#define _MessageValueMaintenancePage_H_


//#define MV_Maintenance_openMaintenancePage         	      0x10 //打开本地运维界面
#define MV_Maintenance_refreshVisualizationInfo           0x11 //刷新码流信息 2秒刷新一次
#define MV_Maintenance_openMaintenancePage_clear          0x12
#define MV_Maintenance_stopCollectDebugInfo               0x13 // dtop collect debug info


#ifdef __cplusplus

namespace Hippo {


} // namespace Hippo

#endif // __cplusplus

#endif // _MessageValueMaintenancePage_H_
