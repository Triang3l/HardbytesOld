#ifndef HbInclude_HbParallel
#define HbInclude_HbParallel
#include "HbCommon.h"
#if HbPlatform_OS_Windows
#include <intrin.h>
#include <Windows.h>
#endif
#ifdef __cplusplus
extern "C" {
#endif

#define HbParallel_Thread_MaxNameLength 15

#if HbPlatform_OS_Windows
/**************************************************
 * Windows and Visual C parallelization primitives
 **************************************************/

typedef CRITICAL_SECTION HbParallel_Mutex;
HbForceInline HbBool HbParallel_Mutex_Init(HbParallel_Mutex * mutex) {
	InitializeCriticalSection(mutex);
	return HbTrue;
}
#define HbParallel_Mutex_Destroy DeleteCriticalSection
#define HbParallel_Mutex_Lock EnterCriticalSection
#define HbParallel_Mutex_Unlock LeaveCriticalSection

typedef SRWLOCK HbParallel_RWLock;
HbForceInline HbBool HbParallel_RWLock_Init(HbParallel_RWLock * lock) {
	InitializeSRWLock(lock);
	return HbTrue;
}
#define HbParallel_RWLock_Destroy(lock) {}
#define HbParallel_RWLock_LockRead AcquireSRWLockShared
#define HbParallel_RWLock_UnlockRead ReleaseSRWLockShared
#define HbParallel_RWLock_LockWrite AcquireSRWLockExclusive
#define HbParallel_RWLock_UnlockWrite ReleaseSRWLockExclusive

typedef CONDITION_VARIABLE HbParallel_CondEvent;
#define HbParallel_CondEvent_Init InitializeConditionVariable
#define HbParallel_CondEvent_Destroy(condEvent) {}
#define HbParallel_CondEvent_Await(condEvent, mutex) SleepConditionVariableCS(condEvent, mutex, INFINITE)
#define HbParallel_CondEvent_Signal WakeConditionVariable
#define HbParallel_CondEvent_SignalAll WakeAllConditionVariable

typedef HANDLE HbParallel_Thread;
typedef void (* HbParallel_Thread_Entry)(void * data);
HbBool HbParallel_Thread_Start(HbParallel_Thread * thread, char const * name, HbParallel_Thread_Entry entry, void * data);
HbForceInline void HbParallel_Thread_Destroy(HbParallel_Thread * thread) {
	WaitForSingleObject(*thread, INFINITE);
	CloseHandle(*thread);
}

#else
#error No parallelization API implementation for the target OS.
#endif

#ifdef __cplusplus
}
#endif
#endif
