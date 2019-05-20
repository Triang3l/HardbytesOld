#ifndef HbInclude_Math
#define HbInclude_Math

#define HbMath_Pi 3.14159265358979323846f
#define HbMath_InversePi 0.318309886183790671538f

// Quaternions are in WXYZ order (so use .x swizzle for W and .yzw for XYZ).

float3 HbMath_Quat_Apply(float4 q, float3 v) {
	// By Fabian @rygorous Giesen.
	// https://fgiesen.wordpress.com/2019/02/09/rotating-a-single-vector-using-a-quaternion/
	float3 t = 2.0f * cross(q.yzw, v);
	return v + q.x * t + cross(q.yzw, t);
}

float3 HbMath_Quat_ApplyConjugate(float4 q, float3 v) {
	// By Fabian @rygorous Giesen.
	// https://fgiesen.wordpress.com/2019/02/09/rotating-a-single-vector-using-a-quaternion/
	float3 t = 2.0f * cross(-q.yzw, v);
	return v + q.x * t + cross(-q.yzw, t);
}

// perspective = [cot(vFOV * 0.5) * (height / width) | cot(vFOV * 0.5) | z1 / (z1 - z0) | -z0 * z1 / (z1 - z0)]
float4 HbMath_Project_Perspective(float4 perspective, float3 position) {
	float4 projected = float4(perspective.xyz * position, position.z);
	projected.z += perspective.w;
	return projected;
}

#endif
