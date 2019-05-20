#include "HbGFX.h"
#include "HbShader.h"

enum {
	// 2 root constants z0 and zUnprojectScale.
	HbGFX_Tile_GetDepthBounds_Binding_ZUnprojectConstants,
	HbGFX_Tile_GetDepthBounds_Binding_PerSampleDepths,
	HbGFX_Tile_GetDepthBounds_Binding_TileBounds,
};
static HbGPU_BindingLayout HbGFX_Tile_GetDepthBounds_BindingLayout;
static HbGPU_ComputeConfig HbGFX_Tile_GetDepthBounds_ConfigSS, HbGFX_Tile_GetDepthBounds_ConfigMS;

HbBool HbGFX_Tile_Init(HbGPU_Device * gpuDevice) {
	static HbGPU_Binding_HandleRange const perSampleDepthsHandleRange = {
		.type = HbGPU_Binding_HandleRange_Type_Texture,
		.handleOffset = 0,
		.handleCount = 1,
		.firstRegister = { .cbufferResourceEdit = 0, .bufferImage = 0 },
	};
	static HbGPU_Binding_HandleRange const tileBoundsHandleRanges[] = {
		{
			.type = HbGPU_Binding_HandleRange_Type_EditBuffer,
			.handleOffset = 0,
			.handleCount = 1,
			.firstRegister = { .cbufferResourceEdit = 0, .bufferImage = 1 },
		},
		{
			.type = HbGPU_Binding_HandleRange_Type_EditImage,
			.handleOffset = 1,
			.handleCount = 1,
			.firstRegister = { .cbufferResourceEdit = 1, .bufferImage = 1 },
		},
	};
	static HbGPU_Binding const bindings[] = {
		[HbGFX_Tile_GetDepthBounds_Binding_ZUnprojectConstants] = {
			.stages = HbGPU_ShaderStageBits_Compute,
			.type = HbGPU_Binding_Type_SmallConstants,
			.binding.smallConstants = {
				.bindRegister = { .cbufferResourceEdit = 0, .bufferImage = 0 },
				.sizeInDwords = 2,
			},
		},
		[HbGFX_Tile_GetDepthBounds_Binding_PerSampleDepths] = {
			.stages = HbGPU_ShaderStageBits_Compute,
			.type = HbGPU_Binding_Type_HandleRangeSet,
			.binding.handleRangeSet = {
				.ranges = &perSampleDepthsHandleRange,
				.rangeCount = 1,
			},
		},
		[HbGFX_Tile_GetDepthBounds_Binding_TileBounds] = {
			.stages = HbGPU_ShaderStageBits_Compute,
			.type = HbGPU_Binding_Type_HandleRangeSet,
			.binding.handleRangeSet = {
				.ranges = tileBoundsHandleRanges,
				.rangeCount = HbArrayLength(tileBoundsHandleRanges),
			},
		},
	};
	if (!HbGPU_BindingLayout_Init(&HbGFX_Tile_GetDepthBounds_BindingLayout, "HbGFX_Tile_GetDepthBounds_BindingLayout",
			gpuDevice, bindings, HbArrayLength(bindings), HbFalse)) {
		return HbFalse;
	}

	#if HbGPU_Implementation_D3D
	HbGPU_ShaderReference const shaderSS = {
		.dxbc = HbShader_Tile_GetDepthBoundsCS,
		.dxbcSize = HbShader_Tile_GetDepthBoundsCS_Size,
	};
	HbGPU_ShaderReference const shaderMS = {
		.dxbc = HbShader_Tile_GetDepthBoundsMSCS,
		.dxbcSize = HbShader_Tile_GetDepthBoundsMSCS_Size,
	};
	#else
	#error No tile depth bounds shaders for the target GPU implementation.
	#endif
	uint32_t const groupSize[3] = { HbGFX_Tile_TileSize, HbGFX_Tile_TileSize, 1 };
	if (!HbGPU_ComputeConfig_Init(&HbGFX_Tile_GetDepthBounds_ConfigSS, "HbGFX_Tile_GetDepthBounds_ConfigSS",
			gpuDevice, shaderSS, groupSize, &HbGFX_Tile_GetDepthBounds_BindingLayout)) {
		HbGPU_BindingLayout_Destroy(&HbGFX_Tile_GetDepthBounds_BindingLayout);
		return HbFalse;
	}
	if (!HbGPU_ComputeConfig_Init(&HbGFX_Tile_GetDepthBounds_ConfigMS, "HbGFX_Tile_GetDepthBounds_ConfigMS",
			gpuDevice, shaderMS, groupSize, &HbGFX_Tile_GetDepthBounds_BindingLayout)) {
		HbGPU_ComputeConfig_Destroy(&HbGFX_Tile_GetDepthBounds_ConfigSS);
		HbGPU_BindingLayout_Destroy(&HbGFX_Tile_GetDepthBounds_BindingLayout);
		return HbFalse;
	}
	return HbTrue;
}

void HbGFX_Tile_Shutdown() {
	HbGPU_ComputeConfig_Destroy(&HbGFX_Tile_GetDepthBounds_ConfigMS);
	HbGPU_ComputeConfig_Destroy(&HbGFX_Tile_GetDepthBounds_ConfigSS);
	HbGPU_BindingLayout_Destroy(&HbGFX_Tile_GetDepthBounds_BindingLayout);
}

void HbGFX_Tile_GetDepthBounds(HbGPU_CmdList * cmdList, uint32_t perSampleDepthsHandle, HbGPU_Image_Info const * perSampleDepthsInfo,
		uint32_t tileDepthBoundsHandles, float const z0AndZUnprojectScale[2]) {
	HbGPU_CmdList_Compute_Begin(cmdList);
	HbGPU_CmdList_Bind_SetLayout(cmdList, &HbGFX_Tile_GetDepthBounds_BindingLayout);
	HbGPU_CmdList_Bind_SmallConstants(cmdList, HbGFX_Tile_GetDepthBounds_Binding_ZUnprojectConstants, z0AndZUnprojectScale, 2);
	HbGPU_CmdList_Bind_Handles(cmdList, HbGFX_Tile_GetDepthBounds_Binding_PerSampleDepths, perSampleDepthsHandle);
	HbGPU_CmdList_Bind_Handles(cmdList, HbGFX_Tile_GetDepthBounds_Binding_TileBounds, tileDepthBoundsHandles);
	HbGPU_CmdList_Compute_SetConfig(cmdList,
			perSampleDepthsInfo->samplesLog2 > 0 ? &HbGFX_Tile_GetDepthBounds_ConfigMS : &HbGFX_Tile_GetDepthBounds_ConfigSS);
	HbGPU_CmdList_Compute_Dispatch(cmdList, (perSampleDepthsInfo->width + (HbGFX_Tile_TileSize - 1)) >> HbGFX_Tile_TileSizeLog2,
			(perSampleDepthsInfo->height + (HbGFX_Tile_TileSize - 1)) >> HbGFX_Tile_TileSizeLog2, 1);
	HbGPU_CmdList_Compute_End(cmdList);
}
