#ifndef HbInclude_HbInput
#define HbInclude_HbInput
#include "HbMemory.h"
#if HbPlatform_OS_WindowsDesktop
#include <Windows.h>
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

typedef enum HbInput_Button {
	// Keyboard keys are DirectInput scan codes (DIK_).
	// Copyright (C) Microsoft. All rights reserved.
	HbInput_Button_Invalid, // Zero for hash map empty slots.

	HbInput_Button_Escape,
	HbInput_Button_1,
	HbInput_Button_2,
	HbInput_Button_3,
	HbInput_Button_4,
	HbInput_Button_5,
	HbInput_Button_6,
	HbInput_Button_7,
	HbInput_Button_8,
	HbInput_Button_9,
	HbInput_Button_0,
	HbInput_Button_Minus,
	HbInput_Button_Equals,
	HbInput_Button_Backspace,
	HbInput_Button_Tab,
	HbInput_Button_Q,
	HbInput_Button_W,
	HbInput_Button_E,
	HbInput_Button_R,
	HbInput_Button_T,
	HbInput_Button_Y,
	HbInput_Button_U,
	HbInput_Button_I,
	HbInput_Button_O,
	HbInput_Button_P,
	HbInput_Button_LBracket,
	HbInput_Button_RBracket,
	HbInput_Button_Return,
	HbInput_Button_LControl,
	HbInput_Button_A,
	HbInput_Button_S,
	HbInput_Button_D,
	HbInput_Button_F,
	HbInput_Button_G,
	HbInput_Button_H,
	HbInput_Button_J,
	HbInput_Button_K,
	HbInput_Button_L,
	HbInput_Button_Semicolon,
	HbInput_Button_Apostrophe,
	HbInput_Button_Grave,
	HbInput_Button_LShift,
	HbInput_Button_Backslash,
	HbInput_Button_Z,
	HbInput_Button_X,
	HbInput_Button_C,
	HbInput_Button_V,
	HbInput_Button_B,
	HbInput_Button_N,
	HbInput_Button_M,
	HbInput_Button_Comma,
	HbInput_Button_Period,
	HbInput_Button_Slash,
	HbInput_Button_RShift,
	HbInput_Button_NumpadStar,
	HbInput_Button_LAlt,
	HbInput_Button_Space,
	HbInput_Button_CapsLock,
	HbInput_Button_F1,
	HbInput_Button_F2,
	HbInput_Button_F3,
	HbInput_Button_F4,
	HbInput_Button_F5,
	HbInput_Button_F6,
	HbInput_Button_F7,
	HbInput_Button_F8,
	HbInput_Button_F9,
	HbInput_Button_F10,
	HbInput_Button_NumLock,
	HbInput_Button_Scroll,
	HbInput_Button_Numpad7,
	HbInput_Button_Numpad8,
	HbInput_Button_Numpad9,
	HbInput_Button_NumpadMinus,
	HbInput_Button_Numpad4,
	HbInput_Button_Numpad5,
	HbInput_Button_Numpad6,
	HbInput_Button_NumpadPlus,
	HbInput_Button_Numpad1,
	HbInput_Button_Numpad2,
	HbInput_Button_Numpad3,
	HbInput_Button_Numpad0,
	HbInput_Button_NumpadPeriod,
	HbInput_Button_OEM102 = 0x56,
	HbInput_Button_F11,
	HbInput_Button_F12,
	HbInput_Button_F13 = 0x64,
	HbInput_Button_F14,
	HbInput_Button_F15,
	HbInput_Button_Kana = 0x70,
	HbInput_Button_ABNT_C1 = 0x73,
	HbInput_Button_Convert = 0x79,
	HbInput_Button_NoConvert = 0x7B,
	HbInput_Button_Yen = 0x7D,
	HbInput_Button_ABNT_C2,
	HbInput_Button_NumpadEquals = 0x8D,
	HbInput_Button_PrevTrack = 0x90,
	HbInput_Button_At,
	HbInput_Button_Colon,
	HbInput_Button_Underline,
	HbInput_Button_Kanji,
	HbInput_Button_Stop,
	HbInput_Button_AX,
	HbInput_Button_Unlabeled,
	HbInput_Button_NextTrack = 0x99,
	HbInput_Button_NumpadEnter = 0x9C,
	HbInput_Button_RControl,
	HbInput_Button_Mute = 0xA0,
	HbInput_Button_Calculator,
	HbInput_Button_PlayPause,
	HbInput_Button_MediaStop = 0xA4,
	HbInput_Button_VolumeDown = 0xAE,
	HbInput_Button_VolumeUp = 0xB0,
	HbInput_Button_WebHome = 0xB2,
	HbInput_Button_NumpadComma,
	HbInput_Button_NumpadSlash = 0xB5,
	HbInput_Button_SysRq,
	HbInput_Button_RAlt,
	HbInput_Button_Pause = 0xC5,
	HbInput_Button_Home = 0xC7,
	HbInput_Button_UpArrow = 0xC8,
	HbInput_Button_PgUp,
	HbInput_Button_LeftArrow = 0xCB,
	HbInput_Button_RightArrow = 0xCD,
	HbInput_Button_End = 0xCF,
	HbInput_Button_DownArrow,
	HbInput_Button_PgDn,
	HbInput_Button_Insert,
	HbInput_Button_Delete,
	HbInput_Button_LWin = 0xDB,
	HbInput_Button_RWin,
	HbInput_Button_Apps,
	HbInput_Button_Power,
	HbInput_Button_Sleep,
	HbInput_Button_Wake = 0xE3,
	HbInput_Button_WebSearch = 0xE5,
	HbInput_Button_WebFavorites,
	HbInput_Button_WebRefresh,
	HbInput_Button_WebStop,
	HbInput_Button_WebForward,
	HbInput_Button_WebBack,
	HbInput_Button_MyComputer,
	HbInput_Button_Mail,
	HbInput_Button_MediaSelect,

	HbInput_Button_Count = 0x100,
} HbInput_Button;

extern char const * const HbInput_Button_BindingNames[HbInput_Button_Count];

HbInput_Button HbInput_Button_ForBindingName(char const * bindingName);

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
