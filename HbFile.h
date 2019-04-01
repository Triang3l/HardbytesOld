#ifndef HbInclude_HbFile
#define HbInclude_HbFile
#include "HbText.h"
#if HbPlatform_OS_Windows
#include <Windows.h>
#endif

typedef struct HbFile_Mapping {
	void * data;
	size_t size;
#if HbPlatform_OS_Windows
	HANDLE windowsFileHandle;
	HANDLE windowsHandle;
#endif
} HbFile_Mapping;
// Fails for empty files.
HbBool HbFile_Mapping_InitRead(HbFile_Mapping * mapping, HbTextU8 const * path, HbBool max4GB);
void HbFile_Mapping_Destroy(HbFile_Mapping * mapping);

#endif
