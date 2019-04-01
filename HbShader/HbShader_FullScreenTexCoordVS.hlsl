// Clockwise triangle. Requires 3 vertices.
void main(uint vertexID : SV_VertexID, out float2 texCoord : TEXCOORD0, out float4 position : SV_Position) {
	texCoord = float2(vertexID & uint2(1u, 2u));
	texCoord.x *= 2.0f;
	position = float4(texCoord * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
}
