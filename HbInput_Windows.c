#include "HbPlatform.h"
#if HbPlatform_OS_Windows
#include "HbFeedback.h"
#include "HbInput.h"
#include <roapi.h>
#include <windows.gaming.input.h>

static __x_ABI_CWindows_CGaming_CInput_CIGamepadStatics * HbInputi_Windows_Gamepad_Statics;

static uint32_t HbInput_Windows_Gamepad_NextHandle = 1;

typedef struct HbInputi_Windows_Gamepad {
	__x_ABI_CWindows_CGaming_CInput_CIGamepad * gamingInputGamepad;
	HbInput_Gamepad state;
} HbInputi_Windows_Gamepad;

#define HbInputi_Windows_Gamepad_MaxConnected 4
static HbInputi_Windows_Gamepad HbInputi_Windows_Gamepad_Connected[HbInputi_Windows_Gamepad_MaxConnected];
static uint32_t HbInputi_Windows_Gamepad_ConnectedCount;

void HbInput_Init() {
	HSTRING gamepadClassName;
	if (FAILED(WindowsCreateString(RuntimeClass_Windows_Gaming_Input_Gamepad,
			HbArrayLength(RuntimeClass_Windows_Gaming_Input_Gamepad) - 1, &gamepadClassName))) {
		HbFeedback_Crash("HbInput_Init", "Failed to create a WinRT string reference for Windows.Gaming.Input.Gamepad class name.");
	}
	static IID const gamepadStaticsInterfaceID = { 0x8BBCE529, 0xD49C, 0x39E9, { 0x95, 0x60, 0xE4, 0x7D, 0xDE, 0x96, 0xB7, 0xC8 } };
	if (FAILED(RoGetActivationFactory(gamepadClassName, &gamepadStaticsInterfaceID, &HbInputi_Windows_Gamepad_Statics))) {
		HbFeedback_Crash("HbInput_Init", "Failed to get the IGamepadStatics activation factory for Windows.Gaming.Input.Gamepad.");
	}
	WindowsDeleteString(gamepadClassName);

	HbInputi_Windows_Gamepad_ConnectedCount = 0; // Make sure the state is somehow valid even in case of an error.
	HbInput_Gamepad_Update();
}

void HbInput_Shutdown() {
	for (uint32_t gamepadIndex = 0; gamepadIndex < HbInputi_Windows_Gamepad_ConnectedCount; ++gamepadIndex) {
		__x_ABI_CWindows_CGaming_CInput_CIGamepad * gamepad = HbInputi_Windows_Gamepad_Connected[gamepadIndex].gamingInputGamepad;
		gamepad->lpVtbl->Release(gamepad);
	}
	HbInputi_Windows_Gamepad_ConnectedCount = 0;
	HbInputi_Windows_Gamepad_Statics->lpVtbl->Release(HbInputi_Windows_Gamepad_Statics);
}

void HbInput_Gamepad_Update() {
	// Retrieve all states.
	__FIVectorView_1_Windows__CGaming__CInput__CGamepad * gamepadVector;
	if (FAILED(HbInputi_Windows_Gamepad_Statics->lpVtbl->get_Gamepads(HbInputi_Windows_Gamepad_Statics, &gamepadVector))) {
		return; // Don't touch the state in case of an error.
	}
	unsigned int gamepadCount;
	if (FAILED(gamepadVector->lpVtbl->get_Size(gamepadVector, &gamepadCount))) {
		gamepadVector->lpVtbl->Release(gamepadVector);
		return;
	}
	gamepadCount = HbMinU32(gamepadCount, HbInputi_Windows_Gamepad_MaxConnected);
	__x_ABI_CWindows_CGaming_CInput_CIGamepad * gamepads[HbInputi_Windows_Gamepad_MaxConnected];
	__x_ABI_CWindows_CGaming_CInput_CGamepadReading gamepadReadings[HbInputi_Windows_Gamepad_MaxConnected];
	for (uint32_t gamepadIndex = 0; gamepadIndex < gamepadCount; ++gamepadIndex) {
		HbBool gamepadRead = HbTrue;
		__x_ABI_CWindows_CGaming_CInput_CIGamepad * gamepad;
		if (SUCCEEDED(gamepadVector->lpVtbl->GetAt(gamepadVector, gamepadIndex, &gamepad))) {
			gamepads[gamepadIndex] = gamepad;
			if (FAILED(gamepad->lpVtbl->GetCurrentReading(gamepad, &gamepadReadings[gamepadIndex]))) {
				gamepad->lpVtbl->Release(gamepad);
				gamepadRead = HbFalse;
			}
		} else {
			gamepadRead = HbFalse;
		}
		if (!gamepadRead) {
			for (uint32_t gamepadPreviousIndex = 0; gamepadPreviousIndex < gamepadIndex; ++gamepadPreviousIndex) {
				__x_ABI_CWindows_CGaming_CInput_CIGamepad * gamepadPrevious = gamepads[gamepadPreviousIndex];
				gamepadPrevious->lpVtbl->Release(gamepadPrevious);
			}
			gamepadVector->lpVtbl->Release(gamepadVector);
			return;
		}
	}
	gamepadVector->lpVtbl->Release(gamepadVector);

	// Must not return prematurely from now on, or will leave things in a corrupted state. Keep persistent handles of already connected gamepads.
	uint32_t gamepadHandles[HbInputi_Windows_Gamepad_MaxConnected];
	for (uint32_t gamepadIndex = 0; gamepadIndex < gamepadCount; ++gamepadIndex) {
		uint32_t gamepadHandle = HbInput_Gamepad_InvalidHandle;
		__x_ABI_CWindows_CGaming_CInput_CIGamepad * gamepad = gamepads[gamepadIndex];
		for (uint32_t connectedGamepadIndex = 0; connectedGamepadIndex < HbInputi_Windows_Gamepad_ConnectedCount; ++connectedGamepadIndex) {
			HbInputi_Windows_Gamepad * connectedGamepad = &HbInputi_Windows_Gamepad_Connected[connectedGamepadIndex];
			if (connectedGamepad->gamingInputGamepad == gamepad) {
				gamepadHandle = connectedGamepad->state.handle;
				break;
			}
		}
		if (gamepadHandle == HbInput_Gamepad_InvalidHandle) {
			// Newly connected.
			gamepadHandle = HbInput_Windows_Gamepad_NextHandle++;
		}
		gamepadHandles[gamepadIndex] = gamepadHandle;
	}

	// Release old references.
	// If a gamepad is still connected, it will have a reference count of 2 (stored + obtained now), and Release will not destroy it.
	// If it's gone, it will have a reference count of 1, and thus will be destroyed.
	for (uint32_t connectedGamepadIndex = 0; connectedGamepadIndex < HbInputi_Windows_Gamepad_ConnectedCount; ++connectedGamepadIndex) {
		__x_ABI_CWindows_CGaming_CInput_CIGamepad * gamepad = HbInputi_Windows_Gamepad_Connected[connectedGamepadIndex].gamingInputGamepad;
		gamepad->lpVtbl->Release(gamepad);
	}

	// Write the gamepad states in the same order as returned by get_Gamepads.
	HbInputi_Windows_Gamepad_ConnectedCount = gamepadCount;
	for (uint32_t gamepadIndex = 0; gamepadIndex < gamepadCount; ++gamepadIndex) {
		HbInputi_Windows_Gamepad * connectedGamepad = &HbInputi_Windows_Gamepad_Connected[gamepadIndex];
		connectedGamepad->gamingInputGamepad = gamepads[gamepadIndex]; // Keep the reference.
		connectedGamepad->state.handle = gamepadHandles[gamepadIndex];
		__x_ABI_CWindows_CGaming_CInput_CGamepadReading const * gamepadReading = &gamepadReadings[gamepadIndex];
		connectedGamepad->state.leftStickRight = (float) gamepadReading->LeftThumbstickX;
		connectedGamepad->state.leftStickUp = (float) gamepadReading->LeftThumbstickY;
		connectedGamepad->state.rightStickRight = (float) gamepadReading->RightThumbstickX;
		connectedGamepad->state.rightStickUp = (float) gamepadReading->RightThumbstickY;
		// TODO: Buttons/axes.
	}
}

uint32_t HbInput_Gamepad_GetCount() {
	return HbInputi_Windows_Gamepad_ConnectedCount;
}

HbInput_Gamepad const * HbInput_Gamepad_GetByIndex(uint32_t index) {
	if (index >= HbInputi_Windows_Gamepad_ConnectedCount) {
		return HbNull;
	}
	return &HbInputi_Windows_Gamepad_Connected[index].state;
}

HbInput_Gamepad const * HbInput_Gamepad_GetByHandle(uint32_t handle) {
	for (uint32_t gamepadIndex = 0; gamepadIndex < HbInputi_Windows_Gamepad_ConnectedCount; ++gamepadIndex) {
		HbInputi_Windows_Gamepad * gamepad = &HbInputi_Windows_Gamepad_Connected[gamepadIndex];
		if (gamepad->state.handle == handle) {
			return &gamepad->state;
		}
	}
	return HbNull;
}

#endif
