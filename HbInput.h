#ifndef HbInclude_HbInput
#define HbInclude_HbInput
#include "HbMemory.h"
#if HbPlatform_OS_WindowsDesktop
#include <Windows.h>
#endif
#ifdef __cplusplus
extern "C" {
#endif

void HbInput_Init();
void HbInputi_InitPlatform();
void HbInput_Shutdown();
void HbInputi_ShutdownPlatform();

#if HbPlatform_OS_WindowsDesktop
HbBool HbInput_Windows_HandleMessage(HWND window, UINT message, WPARAM wParam, LPARAM lParam); // True if safe to call DefWindowProc.
#endif

extern HbMemory_Tag * HbInput_MemoryTag;

/******************************
 * Keyboard/mouse button input
 ******************************/

typedef enum HbInput_Code_Button {
	// Keyboard keys are DirectInput scan codes (DIK_).
	// Copyright (C) Microsoft. All rights reserved.
	HbInput_Button_Code_Invalid, // Zero for hash map empty slots.

	HbInput_Button_Code_Escape,
	HbInput_Button_Code_1,
	HbInput_Button_Code_2,
	HbInput_Button_Code_3,
	HbInput_Button_Code_4,
	HbInput_Button_Code_5,
	HbInput_Button_Code_6,
	HbInput_Button_Code_7,
	HbInput_Button_Code_8,
	HbInput_Button_Code_9,
	HbInput_Button_Code_0,
	HbInput_Button_Code_Minus,
	HbInput_Button_Code_Equals,
	HbInput_Button_Code_Backspace,
	HbInput_Button_Code_Tab,
	HbInput_Button_Code_Q,
	HbInput_Button_Code_W,
	HbInput_Button_Code_E,
	HbInput_Button_Code_R,
	HbInput_Button_Code_T,
	HbInput_Button_Code_Y,
	HbInput_Button_Code_U,
	HbInput_Button_Code_I,
	HbInput_Button_Code_O,
	HbInput_Button_Code_P,
	HbInput_Button_Code_LBracket,
	HbInput_Button_Code_RBracket,
	HbInput_Button_Code_Return,
	HbInput_Button_Code_LControl,
	HbInput_Button_Code_A,
	HbInput_Button_Code_S,
	HbInput_Button_Code_D,
	HbInput_Button_Code_F,
	HbInput_Button_Code_G,
	HbInput_Button_Code_H,
	HbInput_Button_Code_J,
	HbInput_Button_Code_K,
	HbInput_Button_Code_L,
	HbInput_Button_Code_Semicolon,
	HbInput_Button_Code_Apostrophe,
	HbInput_Button_Code_Grave,
	HbInput_Button_Code_LShift,
	HbInput_Button_Code_Backslash,
	HbInput_Button_Code_Z,
	HbInput_Button_Code_X,
	HbInput_Button_Code_C,
	HbInput_Button_Code_V,
	HbInput_Button_Code_B,
	HbInput_Button_Code_N,
	HbInput_Button_Code_M,
	HbInput_Button_Code_Comma,
	HbInput_Button_Code_Period,
	HbInput_Button_Code_Slash,
	HbInput_Button_Code_RShift,
	HbInput_Button_Code_NumpadStar,
	HbInput_Button_Code_LAlt,
	HbInput_Button_Code_Space,
	HbInput_Button_Code_CapsLock,
	HbInput_Button_Code_F1,
	HbInput_Button_Code_F2,
	HbInput_Button_Code_F3,
	HbInput_Button_Code_F4,
	HbInput_Button_Code_F5,
	HbInput_Button_Code_F6,
	HbInput_Button_Code_F7,
	HbInput_Button_Code_F8,
	HbInput_Button_Code_F9,
	HbInput_Button_Code_F10,
	HbInput_Button_Code_NumLock,
	HbInput_Button_Code_Scroll,
	HbInput_Button_Code_Numpad7,
	HbInput_Button_Code_Numpad8,
	HbInput_Button_Code_Numpad9,
	HbInput_Button_Code_NumpadMinus,
	HbInput_Button_Code_Numpad4,
	HbInput_Button_Code_Numpad5,
	HbInput_Button_Code_Numpad6,
	HbInput_Button_Code_NumpadPlus,
	HbInput_Button_Code_Numpad1,
	HbInput_Button_Code_Numpad2,
	HbInput_Button_Code_Numpad3,
	HbInput_Button_Code_Numpad0,
	HbInput_Button_Code_NumpadPeriod,
	HbInput_Button_Code_OEM102 = 0x56,
	HbInput_Button_Code_F11,
	HbInput_Button_Code_F12,
	HbInput_Button_Code_F13 = 0x64,
	HbInput_Button_Code_F14,
	HbInput_Button_Code_F15,
	HbInput_Button_Code_Kana = 0x70,
	HbInput_Button_Code_ABNT_C1 = 0x73,
	HbInput_Button_Code_Convert = 0x79,
	HbInput_Button_Code_NoConvert = 0x7B,
	HbInput_Button_Code_Yen = 0x7D,
	HbInput_Button_Code_ABNT_C2,
	HbInput_Button_Code_NumpadEquals = 0x8D,
	HbInput_Button_Code_PrevTrack = 0x90,
	HbInput_Button_Code_At,
	HbInput_Button_Code_Colon,
	HbInput_Button_Code_Underline,
	HbInput_Button_Code_Kanji,
	HbInput_Button_Code_Stop,
	HbInput_Button_Code_AX,
	HbInput_Button_Code_Unlabeled,
	HbInput_Button_Code_NextTrack = 0x99,
	HbInput_Button_Code_NumpadEnter = 0x9C,
	HbInput_Button_Code_RControl,
	HbInput_Button_Code_Mute = 0xA0,
	HbInput_Button_Code_Calculator,
	HbInput_Button_Code_PlayPause,
	HbInput_Button_Code_MediaStop = 0xA4,
	HbInput_Button_Code_VolumeDown = 0xAE,
	HbInput_Button_Code_VolumeUp = 0xB0,
	HbInput_Button_Code_WebHome = 0xB2,
	HbInput_Button_Code_NumpadComma,
	HbInput_Button_Code_NumpadSlash = 0xB5,
	HbInput_Button_Code_SysRq,
	HbInput_Button_Code_RAlt,
	HbInput_Button_Code_Pause = 0xC5,
	HbInput_Button_Code_Home = 0xC7,
	HbInput_Button_Code_UpArrow = 0xC8,
	HbInput_Button_Code_PgUp,
	HbInput_Button_Code_LeftArrow = 0xCB,
	HbInput_Button_Code_RightArrow = 0xCD,
	HbInput_Button_Code_End = 0xCF,
	HbInput_Button_Code_DownArrow,
	HbInput_Button_Code_PgDn,
	HbInput_Button_Code_Insert,
	HbInput_Button_Code_Delete,
	HbInput_Button_Code_LWin = 0xDB,
	HbInput_Button_Code_RWin,
	HbInput_Button_Code_Apps,
	HbInput_Button_Code_Power,
	HbInput_Button_Code_Sleep,
	HbInput_Button_Code_Wake = 0xE3,
	HbInput_Button_Code_WebSearch = 0xE5,
	HbInput_Button_Code_WebFavorites,
	HbInput_Button_Code_WebRefresh,
	HbInput_Button_Code_WebStop,
	HbInput_Button_Code_WebForward,
	HbInput_Button_Code_WebBack,
	HbInput_Button_Code_MyComputer,
	HbInput_Button_Code_Mail,
	HbInput_Button_Code_MediaSelect,

	HbInput_Button_Code_Count = 0x100,
} HbInput_Button_Code;

extern char const * const HbInput_Button_BindingNames[HbInput_Button_Code_Count];

HbInput_Button_Code HbInput_Button_CodeForBindingName(char const * bindingName);

extern uint32_t HbInput_Button_AreDown[(HbInput_Button_Code_Count + 31) >> 5];
HbForceInline HbBool HbInput_Button_IsDown(HbInput_Button_Code button) {
	return (HbInput_Button_AreDown[(uint32_t) button >> 5] & ((uint32_t) 1 << ((uint32_t) button & 31))) != 0;
}

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

#ifdef __cplusplus
}
#endif
#endif
