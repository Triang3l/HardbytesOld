#include "HbFile.h"
#include "HbFile_IQM.h"

HbBool HbFile_Mapping_InitRead(HbFile_Mapping * mapping, HbTextU8 const * path, HbBool max4GB) {
	#if HbPlatform_OS_Windows
	size_t pathU16Size = HbTextU8_LengthU16Elems(path) + 1;
	HbTextU16 * pathU16 = HbStackAlloc(pathU16Size * sizeof(HbTextU16));
	HbTextU16_FromU8(pathU16, pathU16Size, path);
	mapping->windowsFileHandle = CreateFileW(pathU16, GENERIC_READ, FILE_SHARE_READ, HbNull, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, HbNull);
	if (mapping->windowsFileHandle == INVALID_HANDLE_VALUE) {
		return HbFalse;
	}
	LARGE_INTEGER fileSize;
	if (!GetFileSizeEx(mapping->windowsFileHandle, &fileSize) || fileSize.QuadPart == 0) {
		CloseHandle(mapping->windowsFileHandle);
		return HbFalse;
	}
	if (max4GB && fileSize.HighPart != 0) {
		CloseHandle(mapping->windowsFileHandle);
		return HbFalse;
	}
	mapping->size = (size_t) fileSize.QuadPart;
	mapping->windowsHandle = CreateFileMappingFromApp(mapping->windowsFileHandle, HbNull, PAGE_READONLY, fileSize.QuadPart, HbNull);
	if (mapping->windowsHandle == HbNull) {
		CloseHandle(mapping->windowsFileHandle);
		return HbFalse;
	}
	mapping->data = MapViewOfFileFromApp(mapping->windowsHandle, FILE_MAP_READ, 0, mapping->size);
	if (mapping->data == HbNull) {
		CloseHandle(mapping->windowsHandle);
		CloseHandle(mapping->windowsFileHandle);
		return HbFalse;
	}
	return HbTrue;
	#else
	#error No file mapping implementation for the target OS.
	#endif
}

void HbFile_Mapping_Destroy(HbFile_Mapping * mapping) {
	#if HbPlatform_OS_Windows
	UnmapViewOfFile(mapping->data);
	CloseHandle(mapping->windowsHandle);
	CloseHandle(mapping->windowsFileHandle);
	#endif
}

char const HbFile_IQM_Magic[16] = "INTERQUAKEMODEL";
