#include "HbFeedback.h"
#include "HbHash.h"
#include "HbInput.h"
#include "HbText.h"

HbMemory_Tag * HbInput_MemoryTag;

char const * const HbInput_Button_BindingNames[HbInput_Button_Count] = {
	[HbInput_Button_Escape] = "Escape",
	[HbInput_Button_1] = "1",
	[HbInput_Button_2] = "2",
	[HbInput_Button_3] = "3",
	[HbInput_Button_4] = "4",
	[HbInput_Button_5] = "5",
	[HbInput_Button_6] = "6",
	[HbInput_Button_7] = "7",
	[HbInput_Button_8] = "8",
	[HbInput_Button_9] = "9",
	[HbInput_Button_0] = "0",
	[HbInput_Button_Minus] = "Minus",
	[HbInput_Button_Equals] = "Equals",
	[HbInput_Button_Backspace] = "Backspace",
	[HbInput_Button_Tab] = "Tab",
	[HbInput_Button_Q] = "Q",
	[HbInput_Button_W] = "W",
	[HbInput_Button_E] = "E",
	[HbInput_Button_R] = "R",
	[HbInput_Button_T] = "T",
	[HbInput_Button_Y] = "Y",
	[HbInput_Button_U] = "U",
	[HbInput_Button_I] = "I",
	[HbInput_Button_O] = "O",
	[HbInput_Button_P] = "P",
	[HbInput_Button_LBracket] = "LBracket",
	[HbInput_Button_RBracket] = "RBracket",
	[HbInput_Button_Return] = "Return",
	[HbInput_Button_LControl] = "LControl",
	[HbInput_Button_A] = "A",
	[HbInput_Button_S] = "S",
	[HbInput_Button_D] = "D",
	[HbInput_Button_F] = "F",
	[HbInput_Button_G] = "G",
	[HbInput_Button_H] = "H",
	[HbInput_Button_J] = "J",
	[HbInput_Button_K] = "K",
	[HbInput_Button_L] = "L",
	[HbInput_Button_Semicolon] = "Semicolon",
	[HbInput_Button_Apostrophe] = "Apostrophe",
	[HbInput_Button_Grave] = "Grave",
	[HbInput_Button_LShift] = "LShift",
	[HbInput_Button_Backslash] = "Backslash",
	[HbInput_Button_Z] = "Z",
	[HbInput_Button_X] = "X",
	[HbInput_Button_C] = "C",
	[HbInput_Button_V] = "V",
	[HbInput_Button_B] = "B",
	[HbInput_Button_N] = "N",
	[HbInput_Button_M] = "M",
	[HbInput_Button_Comma] = "Comma",
	[HbInput_Button_Period] = "Period",
	[HbInput_Button_Slash] = "Slash",
	[HbInput_Button_RShift] = "RShift",
	[HbInput_Button_NumpadStar] = "NumpadStar",
	[HbInput_Button_LAlt] = "LAlt",
	[HbInput_Button_Space] = "Space",
	[HbInput_Button_CapsLock] = "CapsLock",
	[HbInput_Button_F1] = "F1",
	[HbInput_Button_F2] = "F2",
	[HbInput_Button_F3] = "F3",
	[HbInput_Button_F4] = "F4",
	[HbInput_Button_F5] = "F5",
	[HbInput_Button_F6] = "F6",
	[HbInput_Button_F7] = "F7",
	[HbInput_Button_F8] = "F8",
	[HbInput_Button_F9] = "F9",
	[HbInput_Button_F10] = "F10",
	[HbInput_Button_NumLock] = "NumLock",
	[HbInput_Button_Scroll] = "Scroll",
	[HbInput_Button_Numpad7] = "Numpad7",
	[HbInput_Button_Numpad8] = "Numpad8",
	[HbInput_Button_Numpad9] = "Numpad9",
	[HbInput_Button_NumpadMinus] = "NumpadMinus",
	[HbInput_Button_Numpad4] = "Numpad4",
	[HbInput_Button_Numpad5] = "Numpad5",
	[HbInput_Button_Numpad6] = "Numpad6",
	[HbInput_Button_NumpadPlus] = "NumpadPlus",
	[HbInput_Button_Numpad1] = "Numpad1",
	[HbInput_Button_Numpad2] = "Numpad2",
	[HbInput_Button_Numpad3] = "Numpad3",
	[HbInput_Button_Numpad0] = "Numpad0",
	[HbInput_Button_NumpadPeriod] = "NumpadPeriod",
	[HbInput_Button_OEM102] = "OEM102",
	[HbInput_Button_F11] = "F11",
	[HbInput_Button_F12] = "F12",
	[HbInput_Button_F13] = "F13",
	[HbInput_Button_F14] = "F14",
	[HbInput_Button_F15] = "F15",
	[HbInput_Button_Kana] = "Kana",
	[HbInput_Button_ABNT_C1] = "ABNT_C1",
	[HbInput_Button_Convert] = "Convert",
	[HbInput_Button_NoConvert] = "NoConvert",
	[HbInput_Button_Yen] = "Yen",
	[HbInput_Button_ABNT_C2] = "ABNT_C2",
	[HbInput_Button_NumpadEquals] = "NumpadEquals",
	[HbInput_Button_PrevTrack] = "PrevTrack",
	[HbInput_Button_At] = "At",
	[HbInput_Button_Colon] = "Colon",
	[HbInput_Button_Underline] = "Underline",
	[HbInput_Button_Kanji] = "Kanji",
	[HbInput_Button_Stop] = "Stop",
	[HbInput_Button_AX] = "AX",
	[HbInput_Button_Unlabeled] = "Unlabeled",
	[HbInput_Button_NextTrack] = "NextTrack",
	[HbInput_Button_NumpadEnter] = "NumpadEnter",
	[HbInput_Button_RControl] = "RControl",
	[HbInput_Button_Mute] = "Mute",
	[HbInput_Button_Calculator] = "Calculator",
	[HbInput_Button_PlayPause] = "PlayPause",
	[HbInput_Button_MediaStop] = "MediaStop",
	[HbInput_Button_VolumeDown] = "VolumeDown",
	[HbInput_Button_VolumeUp] = "VolumeUp",
	[HbInput_Button_WebHome] = "WebHome",
	[HbInput_Button_NumpadComma] = "NumpadComma",
	[HbInput_Button_NumpadSlash] = "NumpadSlash",
	[HbInput_Button_SysRq] = "SysRq",
	[HbInput_Button_RAlt] = "RAlt",
	[HbInput_Button_Pause] = "Pause",
	[HbInput_Button_Home] = "Home",
	[HbInput_Button_UpArrow] = "UpArrow",
	[HbInput_Button_PgUp] = "PgUp",
	[HbInput_Button_LeftArrow] = "LeftArrow",
	[HbInput_Button_RightArrow] = "RightArrow",
	[HbInput_Button_End] = "End",
	[HbInput_Button_DownArrow] = "DownArrow",
	[HbInput_Button_PgDn] = "PgDn",
	[HbInput_Button_Insert] = "Insert",
	[HbInput_Button_Delete] = "Delete",
	[HbInput_Button_LWin] = "LWin",
	[HbInput_Button_RWin] = "RWin",
	[HbInput_Button_Apps] = "Apps",
	[HbInput_Button_Power] = "Power",
	[HbInput_Button_Sleep] = "Sleep",
	[HbInput_Button_Wake] = "Wake",
	[HbInput_Button_WebSearch] = "WebSearch",
	[HbInput_Button_WebFavorites] = "WebFavorites",
	[HbInput_Button_WebRefresh] = "WebRefresh",
	[HbInput_Button_WebStop] = "WebStop",
	[HbInput_Button_WebForward] = "WebForward",
	[HbInput_Button_WebBack] = "WebBack",
	[HbInput_Button_MyComputer] = "MyComputer",
	[HbInput_Button_Mail] = "Mail",
	[HbInput_Button_MediaSelect] = "MediaSelect",
};

static HbInput_Button * HbInput_Button_BindingNameHashMap;
static uint32_t HbInput_Button_BindingNameHashMapMask;

HbInput_Button HbInput_Button_ForBindingName(char const * bindingName) {
	uint32_t hash = HbHash_FVN1a_HashTextACaseless(bindingName);
	uint32_t index = hash & HbInput_Button_BindingNameHashMapMask;
	HbInput_Button button;
	while ((button = HbInput_Button_BindingNameHashMap[index]) != HbInput_Button_Invalid) {
		if (HbTextA_CompareCaseless(HbInput_Button_BindingNames[button], bindingName) == 0) {
			break;
		}
		HbHash_Map_PerturbateIndex(&hash, &index, HbInput_Button_BindingNameHashMapMask);
	}
	return button;
}

HbInput_Gamepad const * HbInput_Gamepad_GetByHandle(uint32_t handle) {
	uint32_t gamepadCount = HbInput_Gamepad_GetCount();
	for (uint32_t gamepadIndex = 0; gamepadIndex < gamepadCount; ++gamepadIndex) {
		HbInput_Gamepad const * gamepad = HbInput_Gamepad_GetByIndex(gamepadIndex);
		if (gamepad->handle == handle) {
			return gamepad;
		}
	}
	return HbNull;
}

void HbInput_Gamepad_ApplyDeadZones(float * right, float * up, float innerDeadZoneHalfWidth, float outerDeadZoneRadius) {
	float rightValue = *right, upValue = *up;
	float innerDeadZoneScale = 1.0f / (1.0f - innerDeadZoneHalfWidth);
	// Remap the magnitude limit to post-inner-deadzone coordinates.
	float outerDeadZoneScale = 1.0f / ((outerDeadZoneRadius - innerDeadZoneHalfWidth) * innerDeadZoneScale);
	float rightAbs = fmaxf((fabsf(rightValue) - innerDeadZoneHalfWidth) * innerDeadZoneScale, 0.0f) * outerDeadZoneScale;
	float upAbs = fmaxf((fabsf(upValue) - innerDeadZoneHalfWidth) * innerDeadZoneScale, 0.0f) * outerDeadZoneScale;
	float magnitudeSqr = rightAbs * rightAbs + upAbs * upAbs;
	if (magnitudeSqr > 1.0f) {
		float invMagnitude = 1.0f / sqrtf(magnitudeSqr);
		rightAbs *= invMagnitude;
		upAbs *= invMagnitude;
	}
	*right = copysignf(rightAbs, rightValue);
	*up = copysignf(upAbs, upValue);
}

void HbInput_Init() {
	HbInput_MemoryTag = HbMemory_Tag_Create("HbInput");

	// Generate the hash table for binding name -> button code mapping.
	HbInput_Button_BindingNameHashMapMask = HbHash_Map_GetIndexMask32(HbInput_Button_Count);
	size_t bindingNameHashMapSize = ((size_t) HbInput_Button_BindingNameHashMapMask + 1) * sizeof(HbInput_Button);
	HbInput_Button_BindingNameHashMap = HbMemory_Alloc(HbInput_MemoryTag, bindingNameHashMapSize, HbFalse);
	HbFeedback_StaticAssert(HbInput_Button_Invalid == 0, "HbInput_Button_Invalid must be 0 for hash map memsetting.");
	memset(HbInput_Button_BindingNameHashMap, 0, bindingNameHashMapSize);
	for (uint32_t buttonIndex = 1; buttonIndex < HbInput_Button_Count; ++buttonIndex) {
		char const * bindingName = HbInput_Button_BindingNames[buttonIndex];
		if (bindingName == HbNull) {
			continue;
		}
		uint32_t bindingNameHash = HbHash_FVN1a_HashTextACaseless(bindingName);
		uint32_t bindingNameHashMapIndex = bindingNameHash & HbInput_Button_BindingNameHashMapMask;
		while (HbInput_Button_BindingNameHashMap[bindingNameHashMapIndex] != HbInput_Button_Invalid) {
			HbHash_Map_PerturbateIndex(&bindingNameHash, &bindingNameHashMapIndex, HbInput_Button_BindingNameHashMapMask);
		}
		HbInput_Button_BindingNameHashMap[bindingNameHashMapIndex] = (HbInput_Button) buttonIndex;
	}

	HbInputi_InitPlatform();
}

void HbInput_Shutdown() {
	HbInputi_ShutdownPlatform();

	HbMemory_Free(HbInput_Button_BindingNameHashMap);

	HbMemory_Tag_Destroy(HbInput_MemoryTag, HbTrue);
}
