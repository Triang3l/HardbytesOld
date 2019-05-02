#ifndef HbInclude_HbInput
#define HbInclude_HbInput
#include "HbCommon.h"
#if HbPlatform_OS_WindowsDesktop
#include <Windows.h>
#endif

void HbInput_Init();
void HbInput_Shutdown();

#if HbPlatform_OS_WindowsDesktop
HbBool HbInput_Windows_HandleMessage(HWND window, UINT message, WPARAM wParam, LPARAM lParam); // True if safe to call DefWindowProc.
#endif

/***********
 * Gamepads
 ***********/

// Handles are identifiers of each gamepad connection.
// Gamepad connections are rechecked every HbInput_Gamepad_Update call, and state is updated by it.
// When a gamepad is connected (or reconnected), it's given a handle that has never been used previously (monotonically increasing value).
// This way player input can be linked directly to a specific gamepad, disregarding behind-the-scenes enumeration details.

#define HbInput_Gamepad_InvalidHandle 0 // Must be zero for auto-initialization of static variables.

typedef struct HbInput_Gamepad {
	uint32_t handle;
	float leftStickRight;
	float leftStickUp;
	float rightStickRight;
	float rightStickUp;
} HbInput_Gamepad;

#define HbInput_Gamepad_Deadzone_LeftStick (7849.0f / 32767.0f)
#define HbInput_Gamepad_Deadzone_RightStick (8689.0f / 32767.0f)

void HbInput_Gamepad_Update();
// All these reflect the state after an Update.
uint32_t HbInput_Gamepad_GetCount();
HbInput_Gamepad const * HbInput_Gamepad_GetByIndex(uint32_t index); // Keep the ->handle for a persistent weak reference.
// Returns nullptr if not connected.
// Handles are never reused, this can also be used to check if a gamepad has been disconnected in the past.
// The returned state is valid until the next update.
// Implemented in common, not per-OS, code.
HbInput_Gamepad const * HbInput_Gamepad_GetByHandle(uint32_t handle);
// Inner dead zone is applied to each axis separately, so moving fully in one direction is possible.
// Outer dead zone is applied after the inner one (but is in pre-inner-deadzone coordinates), the maximum magnitude, will be treated as 1.
void HbInput_Gamepad_ApplyDeadZones(float * right, float * up, float innerDeadZoneHalfWidth, float outerDeadZoneRadius);

#if HbPlatform_OS_WindowsDesktop
// Windows.Gaming.Input is used by default (after HbInput_Init), but XInput 1.4 can be enabled to support things like DLL overrides.
void HbInput_Windows_Gamepad_UseXInput(HbBool useXInput);
#endif

#endif
