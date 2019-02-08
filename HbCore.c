#include "HbCore.h"
#include "HbMemory.h"
#include "HbPlatform.h"

void HbCore_InitEngine() {
	HbMemory_Init(); // Must be the first in case something creates a tag.
	HbPlatform_Init();
}

void HbCore_ShutdownEngine() {
	HbMemory_Shutdown();
}
