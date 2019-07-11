#ifndef __telecom_config_h_2011_1_11
#define __telecom_config_h_2011_1_11

#ifdef __cplusplus
extern "C" {
#endif

//关于网页路径的配置已经移到webpageConfig.h文件中了

#if defined(Jiangsu) /* TR069 parameter */
#define TR069_LIANCHUANG
#endif

#define TAKIN_JVM_MSG

#ifdef HAERBIN_CUC_HD
#define IPOE_PPPOE_AUTOSWITCH
#endif

#ifdef __cplusplus
}
#endif

#endif

