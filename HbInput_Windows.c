#include "HbPlatform.h"
#if HbPlatform_OS_Windows
#include "HbBit.h"
#include "HbFeedback.h"
#include "HbInput.h"
#include <roapi.h>
#include <windows.gaming.input.h>
#if HbPlatform_OS_WindowsDesktop
#include <xinput.h>
#endif

static __x_ABI_CWindows_CGaming_CInput_CIGamepadStatics * HbInputi_Windows_Gamepad_Statics;

static uint32_t HbInput_Windows_Gamepad_NextHandle;

typedef struct HbInputi_Windows_Gamepad {
	__x_ABI_CWindows_CGaming_CInput_CIGamepad * gamingInputGamepad; // Null if connected via XInput.
	#if HbPlatform_OS_WindowsDesktop
	uint32_t xInputUser; // UINT32_MAX if connected via Windows.Gaming.Input.
	#endif
	HbInput_Gamepad state;
} HbInputi_Windows_Gamepad;

#define HbInputi_Windows_Gamepad_MaxConnected 4 // XUSER_MAX_COUNT.
static HbInputi_Windows_Gamepad HbInputi_Windows_Gamepad_Connected[HbInputi_Windows_Gamepad_MaxConnected];
static uint32_t HbInputi_Windows_Gamepad_ConnectedCount = 0;

#if HbPlatform_OS_WindowsDesktop
static HbBool HbInputi_Windows_Gamepad_XInput_Enabled;
// XInputGetState may take a long time for unconnected gamepads, so only recheck all gamepads after a WM_DEVICECHANGE.
static uint32_t HbInputi_Windows_Gamepad_XInput_Recheck;
#define HbInputi_Windows_Gamepad_XInput_Recheck_All ((1 << XUSER_MAX_COUNT) - 1)
#endif

void HbInputi_InitPlatform() {
	HbInput_Windows_Gamepad_NextHandle = 1;

	HbInputi_Windows_Gamepad_ConnectedCount = 0;

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

	#if HbPlatform_OS_WindowsDesktop
	HbInputi_Windows_Gamepad_XInput_Enabled = HbFalse;
	HbInputi_Windows_Gamepad_XInput_Recheck = HbInputi_Windows_Gamepad_XInput_Recheck_All;
	#endif
}

static void HbInputi_Windows_Gamepad_ReleaseGamepads() {
	for (uint32_t gamepadIndex = 0; gamepadIndex < HbInputi_Windows_Gamepad_ConnectedCount; ++gamepadIndex) {
		__x_ABI_CWindows_CGaming_CInput_CIGamepad * gamepad = HbInputi_Windows_Gamepad_Connected[gamepadIndex].gamingInputGamepad;
		if (gamepad != HbNull) { // Might be connected via XInput in the previous update.
			gamepad->lpVtbl->Release(gamepad);
		}
	}
	HbInputi_Windows_Gamepad_ConnectedCount = 0;
}

void HbInputi_ShutdownPlatform() {
	HbInputi_Windows_Gamepad_ReleaseGamepads();
	HbInputi_Windows_Gamepad_Statics->lpVtbl->Release(HbInputi_Windows_Gamepad_Statics);
}

#if HbPlatform_OS_WindowsDesktop
HbBool HbInput_Windows_HandleMessage(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_KILLFOCUS:
		memset(HbInput_Button_AreDown, 0, sizeof(HbInput_Button_AreDown));
		break;
	case WM_KEYDOWN:
		if (!(lParam & ((LPARAM) 1 << 30))) {
			// HbInput_Button codes are Windows scancodes.
			uint32_t scanCode = (uint32_t) ((lParam >> 16) & 255);
			if (scanCode != 0 && scanCode < (uint32_t) HbInput_Button_Code_Count) {
				HbInput_Button_AreDown[scanCode >> 5] |= (uint32_t) 1 << (scanCode & 31);
			}
		}
		break;
	case WM_KEYUP:
		{
			// HbInput_Button codes are Windows scancodes.
			uint32_t scanCode = (uint32_t) ((lParam >> 16) & 255);
			if (scanCode != 0 && scanCode < (uint32_t) HbInput_Button_Code_Count) {
				HbInput_Button_AreDown[scanCode >> 5] &= ~((uint32_t) 1 << (scanCode & 31));
			}
		}
		break;
	case WM_DEVICECHANGE:
		HbInputi_Windows_Gamepad_XInput_Recheck = HbInputi_Windows_Gamepad_XInput_Recheck_All;
		break;
	}
	return HbTrue;
}
#endif

static void HbInputi_Windows_Gamepad_Update_GamingInput() {
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
			HbInputi_Windows_Gamepad const * connectedGamepad = &HbInputi_Windows_Gamepad_Connected[connectedGamepadIndex];
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
	HbInputi_Windows_Gamepad_ReleaseGamepads();

	// Write the gamepad states in the same order as returned by get_Gamepads.
	HbInputi_Windows_Gamepad_ConnectedCount = gamepadCount;
	for (uint32_t gamepadIndex = 0; gamepadIndex < gamepadCount; ++gamepadIndex) {
		HbInputi_Windows_Gamepad * connectedGamepad = &HbInputi_Windows_Gamepad_Connected[gamepadIndex];
		connectedGamepad->gamingInputGamepad = gamepads[gamepadIndex]; // Keep the reference.
		#if HbPlatform_OS_WindowsDesktop
		connectedGamepad->xInputUser = UINT32_MAX;
		#endif
		connectedGamepad->state.handle = gamepadHandles[gamepadIndex];
		__x_ABI_CWindows_CGaming_CInput_CGamepadReading const * gamepadReading = &gamepadReadings[gamepadIndex];
		connectedGamepad->state.leftStickRight = (float) gamepadReading->LeftThumbstickX;
		connectedGamepad->state.leftStickUp = (float) gamepadReading->LeftThumbstickY;
		connectedGamepad->state.rightStickRight = (float) gamepadReading->RightThumbstickX;
		connectedGamepad->state.rightStickUp = (float) gamepadReading->RightThumbstickY;
		// TODO: Buttons/axes.
	}
}

#if HbPlatform_OS_WindowsDesktop
static void HbInputi_Windows_Gamepad_Update_XInput() {
	HbFeedback_StaticAssert(HbInputi_Windows_Gamepad_MaxConnected >= XUSER_MAX_COUNT,
			"HbInputi_Windows_Gamepad_MaxConnected is assumed to be large enough to store all XInput controllers.");

	// Retrieve all states.
	XINPUT_STATE gamepadStates[XUSER_MAX_COUNT];
	uint32_t gamepadsConnected = 0;
	uint32_t gamepadHandles[XUSER_MAX_COUNT];
	// Check only previously connected devices (unless WM_DEVICECHANGE happened).
	uint32_t gamepadBits = HbInputi_Windows_Gamepad_XInput_Recheck;
	while (gamepadBits != 0) {
		uint32_t gamepadIndex = (uint32_t) HbBit_LowestOneU32(gamepadBits);
		gamepadBits &= ~(1 << gamepadIndex);
		if (XInputGetState(gamepadIndex, &gamepadStates[gamepadIndex]) == ERROR_SUCCESS) {
			gamepadsConnected |= 1 << gamepadIndex;
		}
		gamepadHandles[gamepadIndex] = HbInput_Gamepad_InvalidHandle;
	}
	HbInputi_Windows_Gamepad_XInput_Recheck = gamepadsConnected;

	// Keep persistent handles of already connected gamepads.
	for (uint32_t connectedGamepadIndex = 0; connectedGamepadIndex < HbInputi_Windows_Gamepad_ConnectedCount; ++connectedGamepadIndex) {
		HbInputi_Windows_Gamepad const * connectedGamepad = &HbInputi_Windows_Gamepad_Connected[connectedGamepadIndex];
		uint32_t connectedGamepadUser = connectedGamepad->xInputUser;
		if (connectedGamepadUser < XUSER_MAX_COUNT && (gamepadsConnected & (1 << connectedGamepadUser))) {
			gamepadHandles[connectedGamepadUser] = connectedGamepad->state.handle;
		}
	}

	// Clear the list to rewrite it.
	HbInputi_Windows_Gamepad_ReleaseGamepads();

	// Write the gamepad states in the same order as returned by XInputGetState.
	gamepadBits = gamepadsConnected;
	while (HbInputi_Windows_Gamepad_ConnectedCount < HbInputi_Windows_Gamepad_MaxConnected && gamepadBits != 0) {
		uint32_t gamepadIndex = (uint32_t) HbBit_LowestOneU32(gamepadBits);
		gamepadBits &= ~(1 << gamepadIndex);
		HbInputi_Windows_Gamepad * connectedGamepad = &HbInputi_Windows_Gamepad_Connected[HbInputi_Windows_Gamepad_ConnectedCount++];
		connectedGamepad->gamingInputGamepad = HbNull;
		connectedGamepad->xInputUser = gamepadIndex;
		uint32_t gamepadHandle = gamepadHandles[gamepadIndex];
		if (gamepadHandle == HbInput_Gamepad_InvalidHandle) {
			// Newly connected.
			gamepadHandle = HbInput_Windows_Gamepad_NextHandle++;
		}
		connectedGamepad->state.handle = gamepadHandle;
		XINPUT_GAMEPAD const * gamepadState = &gamepadStates[gamepadIndex].Gamepad;
		connectedGamepad->state.leftStickRight = HbMaxI32(gamepadState->sThumbLX, -32767) * (1.0f / 32767.0f);
		connectedGamepad->state.leftStickUp = HbMaxI32(gamepadState->sThumbLY, -32767) * (1.0f / 32767.0f);
		connectedGamepad->state.rightStickRight = HbMaxI32(gamepadState->sThumbRX, -32767) * (1.0f / 32767.0f);
		connectedGamepad->state.rightStickUp = HbMaxI32(gamepadState->sThumbRY, -32767) * (1.0f / 32767.0f);
		// TODO: Buttons/axes.
	}
}
#endif

void HbInput_Gamepad_Update() {
	#if HbPlatform_OS_WindowsDesktop
	if (HbInputi_Windows_Gamepad_XInput_Enabled) {
		HbInputi_Windows_Gamepad_Update_XInput();
	} else {
		HbInputi_Windows_Gamepad_Update_GamingInput();
	}
	#else
	HbInputi_Windows_Gamepad_Update_GamingInput();
	#endif
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

#if HbPlatform_OS_WindowsDesktop
void HbInput_Windows_Gamepad_UseXInput(HbBool useXInput) {
	if (HbInputi_Windows_Gamepad_XInput_Enabled != useXInput) {
		HbInputi_Windows_Gamepad_XInput_Enabled = useXInput;
		if (useXInput) {
			HbInputi_Windows_Gamepad_XInput_Recheck = HbInputi_Windows_Gamepad_XInput_Recheck_All;
		}
	}
}
#endif

#endif
