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

static unsigned int __stdcall HbParalleli_Windows_Thread_Entry(void * parametersPointer) {
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
	return 0;
}

HbBool HbParallel_Thread_Start(HbParallel_Thread * thread, char const * name, HbParallel_Thread_Entry entry, void * data) {
	HbParalleli_Windows_Thread_Parameters * parameters = malloc(sizeof(HbParalleli_Windows_Thread_Parameters));
	if (parameters == NULL) {
		return HbFalse;
	}
	parameters->entry = entry;
	parameters->data = data;
	if (name != NULL) {
		HbTextA_Copy(parameters->name, HbArrayLength(parameters->name), name);
	} else {
		parameters->name[0] = '\0';
	}
	// _beginthread is not safe because the handle is closed in the end, and it causes a race condition between the thread being shut down and
	// the thread doing WaitForSingleObject on it. _beginthreadex doesn't result in an automatically closed handle.
	uintptr_t handle = _beginthreadex(NULL, 0, HbParalleli_Windows_Thread_Entry, parameters, 0, NULL);
	if (handle == 0) {
		free(parameters);
		return HbFalse;
	}
	*thread = (HANDLE) handle;
	return HbTrue;
}

#endif
