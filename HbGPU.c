#include "HbBit.h"
#include "HbFeedback.h"
#include "HbGPU.h"

uint32_t HbGPU_Image_Format_ElementSize(HbGPU_Image_Format format) {
	if (format >= HbGPU_Image_Format_FormatCount) {
		return 0;
	}
	static const uint32_t sizes[] = {
		[HbGPU_Image_Format_8_R_UNorm] = 1,
		[HbGPU_Image_Format_8_8_RG_UNorm] = 2,
		[HbGPU_Image_Format_8_8_8_8_RGBA_UNorm] = 4,
		[HbGPU_Image_Format_8_8_8_8_RGBA_sRGB] = 4,
		[HbGPU_Image_Format_32_UInt] = 4,
		[HbGPU_Image_Format_32_Float] = 4,
		[HbGPU_Image_Format_D32] = 4,
		[HbGPU_Image_Format_D32_S8] = 8,
	};
	HbFeedback_StaticAssert(HbArrayLength(sizes) == HbGPU_Image_Format_FormatCount,
			"All known image formats must have sizes defined in HbGPU_Image_Format_ElementSize.");
	return sizes[(uint32_t) format];
}

HbBool HbGPU_Image_Info_CleanupAndValidate(HbGPU_Image_Info * info) {
	info->width = HbMaxU32(info->width, 1);
	info->height = HbMaxU32(info->height, 1);
	info->depthOrLayers = HbMaxU32(info->depthOrLayers, 1);
	info->mips = HbMaxU32(info->mips, 1);
	if (HbGPU_Image_Format_IsDepth(info->format)) {
		info->usageOptions &= HbGPU_Image_UsageOptions_DepthTestOnly;
	} else {
		info->usageOptions &= HbGPU_Image_UsageOptions_ShaderEditable | HbGPU_Image_UsageOptions_ColorRenderable;
	}
	uint32_t maxWidth = HbGPU_Image_MaxSize1D2D;
	uint32_t maxHeight = HbGPU_Image_MaxSize1D2D;
	uint32_t maxDepthOrLayers = 1;
	uint32_t maxSamplesLog2 = 0;
	switch (info->dimensions) {
	case HbGPU_Image_Dimensions_1D:
		maxHeight = 1;
		break;
	case HbGPU_Image_Dimensions_1DArray:
		maxHeight = 1;
		maxDepthOrLayers = HbGPU_Image_MaxLayers;
		break;
	case HbGPU_Image_Dimensions_2D:
		maxSamplesLog2 = HbGPU_Image_MaxSamplesLog2;
		break;
	case HbGPU_Image_Dimensions_2DArray:
		maxDepthOrLayers = HbGPU_Image_MaxLayers;
		break;
	case HbGPU_Image_Dimensions_Cube:
		break;
	case HbGPU_Image_Dimensions_CubeArray:
		maxDepthOrLayers = HbGPU_Image_Dimensions_CubeArray;
		break;
	case HbGPU_Image_Dimensions_3D:
		maxWidth = maxHeight = maxDepthOrLayers = HbGPU_Image_MaxSize3D;
		break;
	default:
		return HbFalse;
	}
	uint32_t maxSide = HbMaxU32(info->width, info->height);
	if (info->dimensions == HbGPU_Image_Dimensions_3D) {
		maxSide = HbMaxU32(maxSide, info->depthOrLayers);
	}
	uint32_t maxMips;
	if (HbGPU_Image_Dimensions_Are1D(info->dimensions) || info->samplesLog2 > 0) {
		// For 1D it's Metal restriction, for multisampled it's Direct3D restriction.
		maxMips = 1;
	} else {
		maxMips = (uint32_t) HbBit_HighestOneU32(maxSide) + 1;
	}
	if (info->width > maxWidth || info->height > maxHeight || info->depthOrLayers > maxDepthOrLayers ||
			info->mips > maxMips || info->samplesLog2 > maxSamplesLog2) {
		return HbFalse;
	}
	return HbTrue;
}
