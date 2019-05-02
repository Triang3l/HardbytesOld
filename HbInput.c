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
