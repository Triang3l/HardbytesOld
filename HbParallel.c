#include "HbParallel.h"

#if HbPlatform_OS_Windows
#include "HbText.h"
#include <process.h>
#include <Windows.h>

typedef struct HbParalleli_Windows_Thread_Parameters {
	HbParallel_Thread_Entry entry;
	void * data;
	char name[HbParallel_Thread_MaxNameLength + 1];
} HbParalleli_Windows_Thread_Parameters;

#pragma pack(push, 8)
typedef struct HbParalleli_Windows_Thread_NameInfo {
	DWORD type;
	char const * name;
	DWORD threadID;
	DWORD flags;
} HbParalleli_Windows_Thread_NameInfo;
#pragma pack(pop)

static void HbParalleli_Windows_Thread_Entry(void * parametersPointer) {
	HbParalleli_Windows_Thread_Parameters parameters =
			*((HbParalleli_Windows_Thread_Parameters const *) parametersPointer);
	free(parametersPointer);
	if (parameters.name[0] != '\0') {
		HbParalleli_Windows_Thread_NameInfo nameInfo = {
			.type = 0x1000,
			.name = parameters.name,
			.threadID = (DWORD) -1,
			.flags = 0,
		};
		__try {
			RaiseException(0x406d1388, 0, sizeof(nameInfo) / sizeof(ULONG_PTR), (ULONG_PTR const *) &nameInfo);
		} __except (EXCEPTION_EXECUTE_HANDLER) {}
	}
	parameters.entry(parameters.data);
}

HbBool HbParallel_Thread_Start(HbParallel_Thread * thread, char const * name, HbParallel_Thread_Entry entry, void * data) {
	HbParalleli_Windows_Thread_Parameters * parameters = malloc(sizeof(HbParalleli_Windows_Thread_Parameters));
	if (parameters == HbNull) {
		return HbFalse;
	}
	parameters->entry = entry;
	parameters->data = data;
	if (name != HbNull) {
		HbTextA_Copy(parameters->name, HbArrayLength(parameters->name), name);
	} else {
		parameters->name[0] = '\0';
	}
	uintptr_t handle = _beginthread(HbParalleli_Windows_Thread_Entry, 0, parameters);
	if (handle == -1L) {
		free(parameters);
		return HbFalse;
	}
	*thread = (HANDLE) handle;
	return HbTrue;
}

#endif
