#ifndef _UPGRADEASSERTIONS_H_
#define _UPGRADEASSERTIONS_H_

#include "Assertions.h"

#ifdef __cplusplus

extern int gUpgradeModuleLevel;

namespace Hippo {

#define UpgradeLogError(args...)   LOG_ERROR(HLG_OPERATION, gUpgradeModuleLevel, args)
#define UpgradeLogWarning(args...) LOG_WARNING(HLG_OPERATION, gUpgradeModuleLevel, args)
#define UpgradeLogDebug(args...)   LOG(HLG_OPERATION, gUpgradeModuleLevel, args)

} // namespace Hippo

#endif //__cplusplus

#endif //_UpgradeLogAssertions_H_

