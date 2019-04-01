#include "HbBit.h"
#include "HbFeedback.h"
#include "HbGPU.h"

/********
 * Image
 ********/

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
	// Direct3D restriction.
	if (HbGPU_Image_Format_Is4x4(info->format) && ((info->width & 3) != 0 || (info->height & 3) != 0)) {
		return HbFalse;
	}
	return HbTrue;
}

uint32_t HbGPU_Image_Copy_ElementSize(HbGPU_Image_Format format, HbBool stencil) {
	if (format >= HbGPU_Image_Format_FormatCount) {
		return 0;
	}
	if (stencil) {
		return HbGPU_Image_Format_HasStencil(format) ? 1 : 0;
	}
	static uint32_t const sizes[] = {
		[HbGPU_Image_Format_8_R_UNorm] = 1,
		[HbGPU_Image_Format_8_8_RG_UNorm] = 2,
		[HbGPU_Image_Format_8_8_8_8_RGBA_UNorm] = 4,
		[HbGPU_Image_Format_8_8_8_8_RGBA_sRGB] = 4,
		[HbGPU_Image_Format_32_UInt] = 4,
		[HbGPU_Image_Format_32_Float] = 4,
		[HbGPU_Image_Format_S3TC_A1_UNorm] = 8,
		[HbGPU_Image_Format_S3TC_A1_sRGB] = 8,
		[HbGPU_Image_Format_S3TC_A4_UNorm] = 16,
		[HbGPU_Image_Format_S3TC_A4_sRGB] = 16,
		[HbGPU_Image_Format_S3TC_A8_UNorm] = 16,
		[HbGPU_Image_Format_S3TC_A8_sRGB] = 16,
		[HbGPU_Image_Format_3Dc_R_UNorm] = 8,
		[HbGPU_Image_Format_3Dc_R_SNorm] = 8,
		[HbGPU_Image_Format_3Dc_RG_UNorm] = 16,
		[HbGPU_Image_Format_3Dc_RG_SNorm] = 16,
		[HbGPU_Image_Format_D32] = 4,
		[HbGPU_Image_Format_D32_S8] = 4,
	};
	HbFeedback_StaticAssert(HbArrayLength(sizes) == HbGPU_Image_Format_FormatCount,
			"All known image formats must have sizes defined in HbGPU_Image_Format_ElementCopySize.");
	return sizes[(uint32_t) format];
}

uint32_t HbGPU_Image_Copy_MipLayout(HbGPU_Image_Info const * info, HbBool stencil, uint32_t mip,
		uint32_t * outRowPitchBytes, uint32_t * out3DLayerPitchRows, uint32_t * outDepth) {
	// Can't exchange data between multisampled images and buffers in Direct3D.
	if (info->samplesLog2 > 0 || mip >= info->mips) {
		return 0;
	}
	uint32_t mipWidth = info->width, mipHeight = info->height, mipDepth = HbGPU_Image_Info_Get3DDepth(info);
	HbGPU_Image_MipSize(mip, info->dimensions, &mipWidth, &mipHeight, &mipDepth);
	if (HbGPU_Image_Format_Is4x4(info->format)) {
		mipWidth = (mipWidth + 3) >> 2;
		mipHeight = (mipHeight + 3) >> 2;
	}
	uint32_t rowPitch = HbAlignU32(mipWidth * HbGPU_Image_Copy_ElementSize(info->format, stencil), HbGPU_Image_Copy_RowAlignment);
	if (outRowPitchBytes != HbNull) {
		*outRowPitchBytes = rowPitch;
	}
	if (out3DLayerPitchRows != HbNull) {
		*out3DLayerPitchRows = mipHeight;
	}
	if (outDepth != HbNull) {
		*outDepth = mipDepth;
	}
	return HbAlignU32(rowPitch * mipHeight * mipDepth, HbGPU_Image_Copy_SliceAlignment);
}

HbBool HbGPU_Image_InitWithInfo(HbGPU_Image * image, HbTextU8 const * name, HbGPU_Device * device,
		HbGPU_Image_Usage initialUsage, HbGPU_Image_ClearValue const * optimalClearValue) {
	if (!HbGPU_Image_Info_CleanupAndValidate(&image->info)) {
		return HbFalse;
	}
	return HbGPU_Image_InitWithValidInfo(image, name, device, initialUsage, optimalClearValue);
}
