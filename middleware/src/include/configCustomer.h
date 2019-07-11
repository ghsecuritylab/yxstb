#ifndef _configCustomer_H_
#define _configCustomer_H_

#include "mid_sys.h"
#include "SettingEnum.h"

#if 0
#if (defined(Sichuan))
#include "configCustomer/configSichuanHD.h"
#elif defined(Jiangsu)
#  if defined(EC1308H)
#    include "configCustomer/configJiangsuCTCSD.h"
#  else
#    include "configCustomer/configJiangsuCTCHD.h"
#  endif
#elif (defined(Guangdong))
#include "configCustomer/configGuangdongHD.h"
#elif (defined(Hubei))
#include "configCustomer/configHubeiHD.h"
#elif (defined(VIETTEL_HD))
#include "configCustomer/configViettelHD.h"
#elif (defined(Anhui))
#include "configCustomer/configAnhuiCTCSD.h"
#elif (defined(Gansu))
#include "configCustomer/configGansuHD.h"
#elif (defined(Chongqing_EC2108V3H))
#include "configCustomer/configChongqingHD.h"
#elif (defined(Chongqing_EC1308H))
#include "configCustomer/configChongqingSD.h"
#endif
#endif

#include "configCustomer/configDefault.h"

#endif // _configCustomer_H_
