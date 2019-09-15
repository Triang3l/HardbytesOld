#include "HbFeedback.h"
#include "HbHash.h"
#include "HbInput.h"
#include "HbText.h"

HbMemory_Tag * HbInput_MemoryTag;

char const * const HbInput_Button_BindingNames[HbInput_Button_Code_Count] = {
	[HbInput_Button_Code_Escape] = "Escape",
	[HbInput_Button_Code_1] = "1",
	[HbInput_Button_Code_2] = "2",
	[HbInput_Button_Code_3] = "3",
	[HbInput_Button_Code_4] = "4",
	[HbInput_Button_Code_5] = "5",
	[HbInput_Button_Code_6] = "6",
	[HbInput_Button_Code_7] = "7",
	[HbInput_Button_Code_8] = "8",
	[HbInput_Button_Code_9] = "9",
	[HbInput_Button_Code_0] = "0",
	[HbInput_Button_Code_Minus] = "Minus",
	[HbInput_Button_Code_Equals] = "Equals",
	[HbInput_Button_Code_Backspace] = "Backspace",
	[HbInput_Button_Code_Tab] = "Tab",
	[HbInput_Button_Code_Q] = "Q",
	[HbInput_Button_Code_W] = "W",
	[HbInput_Button_Code_E] = "E",
	[HbInput_Button_Code_R] = "R",
	[HbInput_Button_Code_T] = "T",
	[HbInput_Button_Code_Y] = "Y",
	[HbInput_Button_Code_U] = "U",
	[HbInput_Button_Code_I] = "I",
	[HbInput_Button_Code_O] = "O",
	[HbInput_Button_Code_P] = "P",
	[HbInput_Button_Code_LBracket] = "LBracket",
	[HbInput_Button_Code_RBracket] = "RBracket",
	[HbInput_Button_Code_Return] = "Return",
	[HbInput_Button_Code_LControl] = "LControl",
	[HbInput_Button_Code_A] = "A",
	[HbInput_Button_Code_S] = "S",
	[HbInput_Button_Code_D] = "D",
	[HbInput_Button_Code_F] = "F",
	[HbInput_Button_Code_G] = "G",
	[HbInput_Button_Code_H] = "H",
	[HbInput_Button_Code_J] = "J",
	[HbInput_Button_Code_K] = "K",
	[HbInput_Button_Code_L] = "L",
	[HbInput_Button_Code_Semicolon] = "Semicolon",
	[HbInput_Button_Code_Apostrophe] = "Apostrophe",
	[HbInput_Button_Code_Grave] = "Grave",
	[HbInput_Button_Code_LShift] = "LShift",
	[HbInput_Button_Code_Backslash] = "Backslash",
	[HbInput_Button_Code_Z] = "Z",
	[HbInput_Button_Code_X] = "X",
	[HbInput_Button_Code_C] = "C",
	[HbInput_Button_Code_V] = "V",
	[HbInput_Button_Code_B] = "B",
	[HbInput_Button_Code_N] = "N",
	[HbInput_Button_Code_M] = "M",
	[HbInput_Button_Code_Comma] = "Comma",
	[HbInput_Button_Code_Period] = "Period",
	[HbInput_Button_Code_Slash] = "Slash",
	[HbInput_Button_Code_RShift] = "RShift",
	[HbInput_Button_Code_NumpadStar] = "NumpadStar",
	[HbInput_Button_Code_LAlt] = "LAlt",
	[HbInput_Button_Code_Space] = "Space",
	[HbInput_Button_Code_CapsLock] = "CapsLock",
	[HbInput_Button_Code_F1] = "F1",
	[HbInput_Button_Code_F2] = "F2",
	[HbInput_Button_Code_F3] = "F3",
	[HbInput_Button_Code_F4] = "F4",
	[HbInput_Button_Code_F5] = "F5",
	[HbInput_Button_Code_F6] = "F6",
	[HbInput_Button_Code_F7] = "F7",
	[HbInput_Button_Code_F8] = "F8",
	[HbInput_Button_Code_F9] = "F9",
	[HbInput_Button_Code_F10] = "F10",
	[HbInput_Button_Code_NumLock] = "NumLock",
	[HbInput_Button_Code_Scroll] = "Scroll",
	[HbInput_Button_Code_Numpad7] = "Numpad7",
	[HbInput_Button_Code_Numpad8] = "Numpad8",
	[HbInput_Button_Code_Numpad9] = "Numpad9",
	[HbInput_Button_Code_NumpadMinus] = "NumpadMinus",
	[HbInput_Button_Code_Numpad4] = "Numpad4",
	[HbInput_Button_Code_Numpad5] = "Numpad5",
	[HbInput_Button_Code_Numpad6] = "Numpad6",
	[HbInput_Button_Code_NumpadPlus] = "NumpadPlus",
	[HbInput_Button_Code_Numpad1] = "Numpad1",
	[HbInput_Button_Code_Numpad2] = "Numpad2",
	[HbInput_Button_Code_Numpad3] = "Numpad3",
	[HbInput_Button_Code_Numpad0] = "Numpad0",
	[HbInput_Button_Code_NumpadPeriod] = "NumpadPeriod",
	[HbInput_Button_Code_OEM102] = "OEM102",
	[HbInput_Button_Code_F11] = "F11",
	[HbInput_Button_Code_F12] = "F12",
	[HbInput_Button_Code_F13] = "F13",
	[HbInput_Button_Code_F14] = "F14",
	[HbInput_Button_Code_F15] = "F15",
	[HbInput_Button_Code_Kana] = "Kana",
	[HbInput_Button_Code_ABNT_C1] = "ABNT_C1",
	[HbInput_Button_Code_Convert] = "Convert",
	[HbInput_Button_Code_NoConvert] = "NoConvert",
	[HbInput_Button_Code_Yen] = "Yen",
	[HbInput_Button_Code_ABNT_C2] = "ABNT_C2",
	[HbInput_Button_Code_NumpadEquals] = "NumpadEquals",
	[HbInput_Button_Code_PrevTrack] = "PrevTrack",
	[HbInput_Button_Code_At] = "At",
	[HbInput_Button_Code_Colon] = "Colon",
	[HbInput_Button_Code_Underline] = "Underline",
	[HbInput_Button_Code_Kanji] = "Kanji",
	[HbInput_Button_Code_Stop] = "Stop",
	[HbInput_Button_Code_AX] = "AX",
	[HbInput_Button_Code_Unlabeled] = "Unlabeled",
	[HbInput_Button_Code_NextTrack] = "NextTrack",
	[HbInput_Button_Code_NumpadEnter] = "NumpadEnter",
	[HbInput_Button_Code_RControl] = "RControl",
	[HbInput_Button_Code_Mute] = "Mute",
	[HbInput_Button_Code_Calculator] = "Calculator",
	[HbInput_Button_Code_PlayPause] = "PlayPause",
	[HbInput_Button_Code_MediaStop] = "MediaStop",
	[HbInput_Button_Code_VolumeDown] = "VolumeDown",
	[HbInput_Button_Code_VolumeUp] = "VolumeUp",
	[HbInput_Button_Code_WebHome] = "WebHome",
	[HbInput_Button_Code_NumpadComma] = "NumpadComma",
	[HbInput_Button_Code_NumpadSlash] = "NumpadSlash",
	[HbInput_Button_Code_SysRq] = "SysRq",
	[HbInput_Button_Code_RAlt] = "RAlt",
	[HbInput_Button_Code_Pause] = "Pause",
	[HbInput_Button_Code_Home] = "Home",
	[HbInput_Button_Code_UpArrow] = "UpArrow",
	[HbInput_Button_Code_PgUp] = "PgUp",
	[HbInput_Button_Code_LeftArrow] = "LeftArrow",
	[HbInput_Button_Code_RightArrow] = "RightArrow",
	[HbInput_Button_Code_End] = "End",
	[HbInput_Button_Code_DownArrow] = "DownArrow",
	[HbInput_Button_Code_PgDn] = "PgDn",
	[HbInput_Button_Code_Insert] = "Insert",
	[HbInput_Button_Code_Delete] = "Delete",
	[HbInput_Button_Code_LWin] = "LWin",
	[HbInput_Button_Code_RWin] = "RWin",
	[HbInput_Button_Code_Apps] = "Apps",
	[HbInput_Button_Code_Power] = "Power",
	[HbInput_Button_Code_Sleep] = "Sleep",
	[HbInput_Button_Code_Wake] = "Wake",
	[HbInput_Button_Code_WebSearch] = "WebSearch",
	[HbInput_Button_Code_WebFavorites] = "WebFavorites",
	[HbInput_Button_Code_WebRefresh] = "WebRefresh",
	[HbInput_Button_Code_WebStop] = "WebStop",
	[HbInput_Button_Code_WebForward] = "WebForward",
	[HbInput_Button_Code_WebBack] = "WebBack",
	[HbInput_Button_Code_MyComputer] = "MyComputer",
	[HbInput_Button_Code_Mail] = "Mail",
	[HbInput_Button_Code_MediaSelect] = "MediaSelect",
};

static HbInput_Button_Code * HbInput_Button_BindingNameHashMap;
static uint32_t HbInput_Button_BindingNameHashMapMask;

HbInput_Button_Code HbInput_Button_CodeForBindingName(char const * bindingName) {
	uint32_t hash = HbHash_FNV1a_HashTextACaseless(bindingName);
	uint32_t index = hash & HbInput_Button_BindingNameHashMapMask;
	HbInput_Button_Code button;
	while ((button = HbInput_Button_BindingNameHashMap[index]) != HbInput_Button_Code_Invalid) {
		if (HbTextA_CompareCaseless(HbInput_Button_BindingNames[button], bindingName) == 0) {
			break;
		}
		HbHash_MapUtil_PerturbateIndex(&hash, &index, HbInput_Button_BindingNameHashMapMask);
	}
	return button;
}

uint32_t HbInput_Button_AreDown[(HbInput_Button_Code_Count + 31) >> 5];

HbInput_Gamepad const * HbInput_Gamepad_GetByHandle(uint32_t handle) {
	uint32_t gamepadCount = HbInput_Gamepad_GetCount();
	for (uint32_t gamepadIndex = 0; gamepadIndex < gamepadCount; ++gamepadIndex) {
		HbInput_Gamepad const * gamepad = HbInput_Gamepad_GetByIndex(gamepadIndex);
		if (gamepad->handle == handle) {
			return gamepad;
		}
	}
	return NULL;
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
	HbInput_Button_BindingNameHashMapMask = ((uint32_t) 1 << HbHash_MapUtil_GetNeededEntriesLog2(HbInput_Button_Code_Count)) - 1;
	size_t bindingNameHashMapSize = ((size_t) HbInput_Button_BindingNameHashMapMask + 1) * sizeof(HbInput_Button_Code);
	HbInput_Button_BindingNameHashMap = HbMemory_Alloc(HbInput_MemoryTag, bindingNameHashMapSize, HbFalse);
	HbFeedback_StaticAssert(HbInput_Button_Code_Invalid == 0, "HbInput_Button_Invalid must be 0 for hash map memsetting.");
	memset(HbInput_Button_BindingNameHashMap, 0, bindingNameHashMapSize);
	for (uint32_t buttonCode = 1; buttonCode < HbInput_Button_Code_Count; ++buttonCode) {
		char const * bindingName = HbInput_Button_BindingNames[buttonCode];
		if (bindingName == NULL) {
			continue;
		}
		uint32_t bindingNameHash = HbHash_FNV1a_HashTextACaseless(bindingName);
		uint32_t bindingNameHashMapIndex = bindingNameHash & HbInput_Button_BindingNameHashMapMask;
		while (HbInput_Button_BindingNameHashMap[bindingNameHashMapIndex] != HbInput_Button_Code_Invalid) {
			HbHash_MapUtil_PerturbateIndex(&bindingNameHash, &bindingNameHashMapIndex, HbInput_Button_BindingNameHashMapMask);
		}
		HbInput_Button_BindingNameHashMap[bindingNameHashMapIndex] = (HbInput_Button_Code) buttonCode;
	}

	HbInputi_InitPlatform();
}

void HbInput_Shutdown() {
	HbInputi_ShutdownPlatform();

	HbMemory_Free(HbInput_Button_BindingNameHashMap);

	HbMemory_Tag_Destroy(HbInput_MemoryTag, HbTrue);
}
