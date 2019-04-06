#include "HbCore.h"
#include "HbInput.h"
#include "HbMemory.h"
#include "HbPlatform.h"

void HbCore_InitEngine() {
	HbMemory_Init(); // Must be the first in case something creates a tag.
	HbPlatform_Init();
	HbInput_Init();
}

void HbCore_ShutdownEngine() {
	HbInput_Shutdown();
	HbPlatform_Shutdown();
	HbMemory_Shutdown();
}
