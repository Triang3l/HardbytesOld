#include "HbBit.h"
#include "HbGPU.h"

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
	if (info->width > maxWidth || info->height > maxHeight || info->depthOrLayers > maxDepthOrLayers ||
			info->mips > ((uint32_t) HbBit_HighestOneU32(maxSide) + 1) || info->samplesLog2 > maxSamplesLog2 ||
			(info->samplesLog2 > 0 && info->mips > 1)) {
		return HbFalse;
	}
	return HbTrue;
}
