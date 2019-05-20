// Assuming reversed Z (0 is the farthest, 1 is the nearest).

cbuffer getDepthBoundsCbuffer : register(b0) {
	float z0;
	float zUnprojectScale; // (z0 - z1) / z1
	// Unprojection is z0 / (z * zUnprojectScale + 1.0f).
};

#if HbTile_GetDepthBounds_Multisample
Texture2DMS<float> perSampleDepths : register(t0);
#else
Texture2D<float> perPixelDepths : register(t0);
#endif

// For marking tiles as lit - linear buffer (with 256-aligned rows) for copying to the depth buffer of widthTiles x 2*heightTiles size.
// This contains the nearest depth in the cluster. The top half is for the near cluster, the bottom half is for the far one.
// When tiling, viewport can be used to quickly switch between the clusters.
RWBuffer<float> tileBackFaceTestLinearBuffer : register(u0);

// For discarding completely occluded lights - widthTiles x heightTiles x 2 array. Contains the farthest depth in each cluster.
RWTexture2DArray<float> tileFrontFaceTestArray : register(u1);

// X - global nearest depth.
// Y - near cluster farthest depth.
// Z - far cluster nearest depth.
// W - global farthest depth.
groupshared uint4 groupDepthBounds;

[numthreads(16, 16, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID, uint groupThreadIndex : SV_GroupIndex, uint3 groupID : SV_GroupID) {
	uint2 framebufferSize;
	#if HbTile_GetDepthBounds_Multisample
	uint framebufferSamples;
	perSampleDepths.GetDimensions(framebufferSize.x, framebufferSize.y, framebufferSamples);
	#else
	perPixelDepths.GetDimensions(framebufferSize.x, framebufferSize.y);
	#endif

	float pixelDepthNear, pixelDepthFar;
	// Pixels with depth 0 (very far - sky) are removed from far bound calculation, until the entire tile is sky.
	[branch] if (any(dispatchThreadID.xy >= framebufferSize)) {
		pixelDepthNear = 0.0f;
		pixelDepthFar = 1.0f;
	} else {
		#if HbTile_GetDepthBounds_Multisample
		pixelDepthNear = perSampleDepths.Load(int2(dispatchThreadID.xy), 0);
		pixelDepthFar = pixelDepthNear <= 0.0f ? 1.0f : pixelDepthNear;
		for (int sampleIndex = 1; sampleIndex < int(framebufferSamples); ++sampleIndex) {
			float sampleDepth = perSampleDepths.Load(int2(dispatchThreadID.xy), sampleIndex);
			pixelDepthNear = max(pixelDepthNear, sampleDepth);
			[flatten] if (sampleDepth > 0.0f) {
				pixelDepthFar = min(pixelDepthFar, sampleDepth);
			}
		}
		#else
		pixelDepthNear = perPixelDepths.Load(int3(dispatchThreadID)); // dispatchThreadID.z == 0 (LOD 0).
		pixelDepthFar = pixelDepthNear <= 0.0f ? 1.0f : pixelDepthNear;
		#endif
	}

	[branch] if (groupThreadIndex == 0u) {
		groupDepthBounds.xw = uint2(0u, 0x3F800000u);
	}
	GroupMemoryBarrierWithGroupSync();

	// Non-negative numbers - can use integer min/max as float min/max.
	InterlockedMax(groupDepthBounds.x, asuint(pixelDepthNear));
	InterlockedMin(groupDepthBounds.w, asuint(pixelDepthFar));
	GroupMemoryBarrierWithGroupSync();

	[branch] if (groupThreadIndex == 0u) {
		// For sky-only tiles, far depth won't be changed from the initial 1 - prevent 0 minus 1 that would cause negative range.
		groupDepthBounds.w = min(groupDepthBounds.w, groupDepthBounds.x);
		groupDepthBounds.yz = groupDepthBounds.xw;
	}
	GroupMemoryBarrierWithGroupSync();

	// Split into two clusters in linear space.
	float3 linearDepth = z0 / (float3(pixelDepthNear, asfloat(groupDepthBounds.xw)) * zUnprojectScale + 1.0f);
	bool isNear = linearDepth.x < (linearDepth.y + linearDepth.z) * 0.5f; // < rather than <= so the sky will always be "far".
	InterlockedMin(groupDepthBounds.y, isNear ? asuint(pixelDepthFar) : groupDepthBounds.x);
	InterlockedMax(groupDepthBounds.z, isNear ? groupDepthBounds.w : asuint(pixelDepthNear));
	GroupMemoryBarrierWithGroupSync();

	[branch] if (groupThreadIndex == 0u) {
		uint2 tileCount = (framebufferSize + 15u) >> 4u;
		uint backFaceTestBufferStride = (tileCount.x + 63u) & ~63u; // 256-byte-aligned rows.
		uint backFaceTestBufferOffset = groupID.y * backFaceTestBufferStride + groupID.x;
		tileBackFaceTestLinearBuffer[backFaceTestBufferOffset] = asfloat(groupDepthBounds.x);
		tileBackFaceTestLinearBuffer[tileCount.y * backFaceTestBufferStride + backFaceTestBufferOffset] = asfloat(groupDepthBounds.z);
		tileFrontFaceTestArray[groupID] = asfloat(groupDepthBounds.y);
		tileFrontFaceTestArray[uint3(groupID.xy, 1u)] = asfloat(groupDepthBounds.w);
	}
}
