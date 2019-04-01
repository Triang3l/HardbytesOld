// Clockwise triangle. Requires 3 vertices.
float4 main(uint vertexID : SV_VertexID) : SV_Position {
	return float4(float2(vertexID & uint2(1u, 2u)) * float2(4.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
}
