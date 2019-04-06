#include "HbPlatform.h"
#if HbPlatform_OS_Windows
#include "HbFeedback.h"
#include <Windows.h>
#include <roapi.h>

static LARGE_INTEGER HbPlatformi_Time_Windows_QPFrequency, HbPlatformi_Time_Windows_QPOrigin;

void HbPlatform_Init() {
	if (FAILED(RoInitialize(RO_INIT_MULTITHREADED))) {
		HbFeedback_Crash("HbPlatform_Init", "Failed to initalize the Windows Runtime API.");
	}
	if (!QueryPerformanceFrequency(&HbPlatformi_Time_Windows_QPFrequency)) {
		HbFeedback_Crash("HbPlatform_Init", "Failed to obtain the high-resolution timer frequency.");
	}
	QueryPerformanceCounter(&HbPlatformi_Time_Windows_QPOrigin);
}

void HbPlatform_Shutdown() {
	RoUninitialize();
}

int64_t HbPlatform_Time_RealUsec() {
	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);
	return ((counter.QuadPart - HbPlatformi_Time_Windows_QPOrigin.QuadPart) * 1000000) /
			HbPlatformi_Time_Windows_QPFrequency.QuadPart;
}

#endif
