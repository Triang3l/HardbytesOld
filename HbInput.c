#include "HbInput.h"

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
