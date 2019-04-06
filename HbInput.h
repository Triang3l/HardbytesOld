#include "HbCommon.h"

void HbInput_Init();
void HbInput_Shutdown();

/***********
 * Gamepads
 ***********/

// Handles are identifiers of each gamepad connection.
// Gamepad connections are rechecked every HbInput_Gamepad_Update call, and state is updated by it.
// When a gamepad is connected (or reconnected), it's given a handle that has never been used previously (monotonically increasing value).
// This way player input can be linked directly to a specific gamepad, disregarding behind-the-scenes enumeration details.

#define HbInput_Gamepad_InvalidHandle 0

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
HbInput_Gamepad const * HbInput_Gamepad_GetByHandle(uint32_t handle);
