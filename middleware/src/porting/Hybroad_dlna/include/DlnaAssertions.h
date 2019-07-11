#ifndef _DLNAAssertions_H_
#define _DLNAAssertions_H_

#include "Assertions.h"

extern int g_DlnaModuleLevel;


#define DLNA_LOG_ERROR(args...) LOG_ERROR(HLG_OPERATION, g_DlnaModuleLevel, args)
#define DLNA_LOG_WARNING(args...) LOG_WARNING(HLG_OPERATION, g_DlnaModuleLevel, args)
#define DLNA_LOG(args...) LOG(HLG_OPERATION, g_DlnaModuleLevel, args)
#define DLNA_LOG_VERBOSE(args...) LOG_VERBOSE(HLG_OPERATION, g_DlnaModuleLevel, args)



#endif //_DLNAAssertions_H_

