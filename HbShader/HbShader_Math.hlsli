#ifndef HbInclude_ColorSpace
#define HbInclude_ColorSpace

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

#endif
