#ifndef HbInclude_HbPlatform
#define HbInclude_HbPlatform
#include "HbCommon.h"
#ifdef __cplusplus
extern "C" {
#endif

void HbPlatform_Init();
void HbPlatform_Shutdown();

int64_t HbPlatform_Time_RealUsec();

#ifdef __cplusplus
}
#endif
#endif
