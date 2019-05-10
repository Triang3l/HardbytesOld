#include "HbFile_DDS.h"

typedef struct HbFile_DDS_PixelFormatImageFormatMapping {
	HbGPU_Image_Format imageFormat;
	uint32_t rgbBitCount;
	uint32_t rBitMask;
	uint32_t gBitMask;
	uint32_t bBitMask;
	uint32_t aBitMask;
} HbFile_DDS_PixelFormatImageFormatMapping;

static HbFile_DDS_PixelFormatImageFormatMapping const HbFile_DDS_PixelFormatImageFormatMappings[] = {
	{ HbGPU_Image_Format_8_8_8_8_RGBA_UNorm, 32, 0xFF, 0xFF00, 0xFF0000, 0xFF000000 },
	{ HbGPU_Image_Format_8_8_8_8_RGBA_UNorm, 32, 0xFF, 0xFF00, 0xFF0000 },
	{ HbGPU_Image_Format_8_8_8_8_BGRA_UNorm, 32, 0xFF0000, 0xFF00, 0xFF, 0xFF000000 },
	{ HbGPU_Image_Format_8_8_8_8_BGRA_UNorm, 32, 0xFF0000, 0xFF00, 0xFF },
	{ HbGPU_Image_Format_16_16_RG_UNorm, 32, 0xFFFF, 0xFFFF0000 },
	{ HbGPU_Image_Format_10_10_10_2_RGBA_UNorm, 32, 0x3FF, 0xFFC00, 0x3FF00000, 0xC0000000 },
	{ HbGPU_Image_Format_10_10_10_2_RGBA_UNorm, 32, 0x3FF, 0xFFC00, 0x3FF00000 },
	{ HbGPU_Image_Format_5_5_5_1_BGRA_UNorm, 16, 0x7C00, 0x3E0, 0x1F, 0x8000 },
	{ HbGPU_Image_Format_5_5_5_1_BGRA_UNorm, 16, 0x7C00, 0x3E0, 0x1F },
	{ HbGPU_Image_Format_5_6_5_BGR_UNorm, 16, 0xF800, 0x7E0, 0x1F },
	{ HbGPU_Image_Format_4_4_4_4_BGRA_UNorm, 16, 0xF00, 0xF0, 0xF, 0xF000 },
	{ HbGPU_Image_Format_4_4_4_4_BGRA_UNorm, 16, 0xF00, 0xF0, 0xF },
	{ HbGPU_Image_Format_16_R_UNorm, 16, 0xFFFF },
	{ HbGPU_Image_Format_8_R_UNorm, 8, 0xFF },
};

typedef struct HbFile_DDS_FourCCImageFormatMapping {
	HbFile_DDS_FourCC fourCC;
	HbGPU_Image_Format imageFormat;
} HbFile_DDS_FourCCImageFormatMapping;

static HbFile_DDS_FourCCImageFormatMapping const HbFile_DDS_FourCCImageFormatMappings[] = {
	{ HbFile_DDS_FourCC_DXT1, HbGPU_Image_Format_S3TC_A1_UNorm },
	{ HbFile_DDS_FourCC_DXT2, HbGPU_Image_Format_S3TC_A4_UNorm },
	{ HbFile_DDS_FourCC_DXT3, HbGPU_Image_Format_S3TC_A4_UNorm },
	{ HbFile_DDS_FourCC_DXT4, HbGPU_Image_Format_S3TC_A8_UNorm },
	{ HbFile_DDS_FourCC_DXT5, HbGPU_Image_Format_S3TC_A8_UNorm },
};

static HbGPU_Image_Format const HbFile_DDS_DXGIFormatsToImageFormats[] = {
	[HbFile_DDS_DXGIFormat_R32G32B32A32_Float] = HbGPU_Image_Format_32_32_32_32_RGBA_SFloat,
	[HbFile_DDS_DXGIFormat_R32G32B32A32_UInt] = HbGPU_Image_Format_32_32_32_32_RGBA_UInt,
	[HbFile_DDS_DXGIFormat_R32G32B32A32_SInt] = HbGPU_Image_Format_32_32_32_32_RGBA_SInt,
	[HbFile_DDS_DXGIFormat_R16G16B16A16_Float] = HbGPU_Image_Format_16_16_16_16_RGBA_Float,
	[HbFile_DDS_DXGIFormat_R16G16B16A16_UNorm] = HbGPU_Image_Format_16_16_16_16_RGBA_UNorm,
	[HbFile_DDS_DXGIFormat_R16G16B16A16_UInt] = HbGPU_Image_Format_16_16_16_16_RGBA_UInt,
	[HbFile_DDS_DXGIFormat_R16G16B16A16_SNorm] = HbGPU_Image_Format_16_16_16_16_RGBA_SNorm,
	[HbFile_DDS_DXGIFormat_R16G16B16A16_SInt] = HbGPU_Image_Format_16_16_16_16_RGBA_SInt,
	[HbFile_DDS_DXGIFormat_R32G32_Float] = HbGPU_Image_Format_32_32_RG_SFloat,
	[HbFile_DDS_DXGIFormat_R32G32_UInt] = HbGPU_Image_Format_32_32_RG_UInt,
	[HbFile_DDS_DXGIFormat_R32G32_SInt] = HbGPU_Image_Format_32_32_RG_SInt,
	[HbFile_DDS_DXGIFormat_R10G10B10A2_UNorm] = HbGPU_Image_Format_10_10_10_2_RGBA_UNorm,
	[HbFile_DDS_DXGIFormat_R10G10B10A2_UInt] = HbGPU_Image_Format_10_10_10_2_RGBA_UInt,
	[HbFile_DDS_DXGIFormat_R11G11B10_Float] = HbGPU_Image_Format_11_11_10_RGB_UFloat,
	[HbFile_DDS_DXGIFormat_R8G8B8A8_UNorm] = HbGPU_Image_Format_8_8_8_8_RGBA_UNorm,
	[HbFile_DDS_DXGIFormat_R8G8B8A8_UNorm_sRGB] = HbGPU_Image_Format_8_8_8_8_RGBA_sRGB,
	[HbFile_DDS_DXGIFormat_R8G8B8A8_UInt] = HbGPU_Image_Format_8_8_8_8_RGBA_UInt,
	[HbFile_DDS_DXGIFormat_R8G8B8A8_SNorm] = HbGPU_Image_Format_8_8_8_8_RGBA_SNorm,
	[HbFile_DDS_DXGIFormat_R8G8B8A8_SInt] = HbGPU_Image_Format_8_8_8_8_RGBA_SInt,
	[HbFile_DDS_DXGIFormat_R16G16_Float] = HbGPU_Image_Format_16_16_RG_SFloat,
	[HbFile_DDS_DXGIFormat_R16G16_UNorm] = HbGPU_Image_Format_16_16_RG_UNorm,
	[HbFile_DDS_DXGIFormat_R16G16_UInt] = HbGPU_Image_Format_16_16_RG_UInt,
	[HbFile_DDS_DXGIFormat_R16G16_SNorm] = HbGPU_Image_Format_16_16_RG_SNorm,
	[HbFile_DDS_DXGIFormat_R16G16_SInt] = HbGPU_Image_Format_16_16_RG_SInt,
	[HbFile_DDS_DXGIFormat_R32_Float] = HbGPU_Image_Format_32_R_SFloat,
	[HbFile_DDS_DXGIFormat_R32_UInt] = HbGPU_Image_Format_32_R_UInt,
	[HbFile_DDS_DXGIFormat_R32_SInt] = HbGPU_Image_Format_32_R_SInt,
	[HbFile_DDS_DXGIFormat_R8G8_UNorm] = HbGPU_Image_Format_8_8_RG_UNorm,
	[HbFile_DDS_DXGIFormat_R8G8_UInt] = HbGPU_Image_Format_8_8_RG_UInt,
	[HbFile_DDS_DXGIFormat_R8G8_SNorm] = HbGPU_Image_Format_8_8_RG_SNorm,
	[HbFile_DDS_DXGIFormat_R8G8_SInt] = HbGPU_Image_Format_8_8_RG_SInt,
	[HbFile_DDS_DXGIFormat_R16_Float] = HbGPU_Image_Format_16_R_SFloat,
	[HbFile_DDS_DXGIFormat_R16_UNorm] = HbGPU_Image_Format_16_R_UNorm,
	[HbFile_DDS_DXGIFormat_R16_UInt] = HbGPU_Image_Format_16_R_UInt,
	[HbFile_DDS_DXGIFormat_R16_SNorm] = HbGPU_Image_Format_16_R_SNorm,
	[HbFile_DDS_DXGIFormat_R16_SInt] = HbGPU_Image_Format_16_R_SInt,
	[HbFile_DDS_DXGIFormat_R8_UNorm] = HbGPU_Image_Format_8_R_UNorm,
	[HbFile_DDS_DXGIFormat_R8_UInt] = HbGPU_Image_Format_8_R_UInt,
	[HbFile_DDS_DXGIFormat_R8_SNorm] = HbGPU_Image_Format_8_R_SNorm,
	[HbFile_DDS_DXGIFormat_R8_SInt] = HbGPU_Image_Format_8_R_SInt,
	[HbFile_DDS_DXGIFormat_BC1_UNorm] = HbGPU_Image_Format_S3TC_A1_UNorm,
	[HbFile_DDS_DXGIFormat_BC1_UNorm_sRGB] = HbGPU_Image_Format_S3TC_A1_sRGB,
	[HbFile_DDS_DXGIFormat_BC2_UNorm] = HbGPU_Image_Format_S3TC_A4_UNorm,
	[HbFile_DDS_DXGIFormat_BC2_UNorm_sRGB] = HbGPU_Image_Format_S3TC_A4_sRGB,
	[HbFile_DDS_DXGIFormat_BC3_UNorm] = HbGPU_Image_Format_S3TC_A8_UNorm,
	[HbFile_DDS_DXGIFormat_BC3_UNorm_sRGB] = HbGPU_Image_Format_S3TC_A8_sRGB,
	[HbFile_DDS_DXGIFormat_BC4_UNorm] = HbGPU_Image_Format_3Dc_R_UNorm,
	[HbFile_DDS_DXGIFormat_BC4_SNorm] = HbGPU_Image_Format_3Dc_R_SNorm,
	[HbFile_DDS_DXGIFormat_BC5_UNorm] = HbGPU_Image_Format_3Dc_RG_UNorm,
	[HbFile_DDS_DXGIFormat_BC5_SNorm] = HbGPU_Image_Format_3Dc_RG_SNorm,
	[HbFile_DDS_DXGIFormat_B5G6R5_UNorm] = HbGPU_Image_Format_5_6_5_BGR_UNorm,
	[HbFile_DDS_DXGIFormat_B5G5R5A1_UNorm] = HbGPU_Image_Format_5_5_5_1_BGRA_UNorm,
	[HbFile_DDS_DXGIFormat_B8G8R8A8_UNorm] = HbGPU_Image_Format_8_8_8_8_BGRA_UNorm,
	[HbFile_DDS_DXGIFormat_B8G8R8A8_UNorm_sRGB] = HbGPU_Image_Format_8_8_8_8_BGRA_sRGB,
	[HbFile_DDS_DXGIFormat_BC6H_UF16] = HbGPU_Image_Format_BPTC_UFloat,
	[HbFile_DDS_DXGIFormat_BC6H_SF16] = HbGPU_Image_Format_BPTC_SFloat,
	[HbFile_DDS_DXGIFormat_BC7_UNorm] = HbGPU_Image_Format_BPTC_UNorm,
	[HbFile_DDS_DXGIFormat_BC7_UNorm_sRGB] = HbGPU_Image_Format_BPTC_sRGB,
	[HbFile_DDS_DXGIFormat_B4G4R4A4_UNorm] = HbGPU_Image_Format_4_4_4_4_BGRA_UNorm,
};

void const * HbFile_DDS_ValidateAndGetInfo(void const * dds, size_t ddsSize, HbGPU_Image_Info * info) {
	if (ddsSize < sizeof(uint32_t) + sizeof(HbFile_DDS_Header)) {
		return HbNull;
	}
	if (*((uint32_t const *) dds) != HbFile_DDS_Magic) {
		return HbNull;
	}
	HbFile_DDS_Header const * header = (HbFile_DDS_Header const *) ((uint8_t const *) dds + sizeof(uint32_t));
	if (header->structSize != sizeof(HbFile_DDS_Header) || (header->flags & HbFile_DDS_Flags_Required) != HbFile_DDS_Flags_Required ||
			header->height == 0 || header->width == 0) {
		return HbNull;
	}
	uint32_t mipMapCount = 1;
	if (header->flags & HbFile_DDS_Flags_MipMapCount) {
		mipMapCount = header->mipMapCount;
		if (mipMapCount == 0) {
			return HbNull;
		}
	}
	HbGPU_Image_Dimensions dimensions;
	uint32_t depth = 1, arraySize = 1;
	size_t imageDataOffset = sizeof(uint32_t) + sizeof(HbFile_DDS_Header);
	HbGPU_Image_Format format = HbGPU_Image_Format_Invalid;
	uint32_t fourCC = (header->pixelFormat.flags & HbFile_DDS_PixelFormat_Flags_FourCC) ? header->pixelFormat.fourCC : 0;
	if (fourCC == HbFile_DDS_FourCC_DX10) {
		if (ddsSize < (imageDataOffset + sizeof(HbFile_DDS_HeaderDXT10))) {
			return HbNull;
		}
		HbFile_DDS_HeaderDXT10 const * headerDXT10 = (HbFile_DDS_HeaderDXT10 const *) ((uint8_t const *) dds + imageDataOffset);
		imageDataOffset += sizeof(HbFile_DDS_HeaderDXT10);
		arraySize = headerDXT10->arraySize;
		if (arraySize == 0) {
			return HbNull;
		}
		switch (headerDXT10->dimension) {
		case HbFile_DDS_Dimension_Texture1D:
			if (header->height != 1) {
				return HbNull;
			}
			dimensions = arraySize > 1 ? HbGPU_Image_Dimensions_1DArray : HbGPU_Image_Dimensions_1D;
			break;
		case HbFile_DDS_Dimension_Texture2D:
			if (headerDXT10->miscFlags & HbFile_DDS_MiscFlags_TextureCube) {
				dimensions = arraySize > 1 ? HbGPU_Image_Dimensions_CubeArray : HbGPU_Image_Dimensions_Cube;
			} else {
				dimensions = arraySize > 1 ? HbGPU_Image_Dimensions_2DArray : HbGPU_Image_Dimensions_2D;
			}
			break;
		case HbFile_DDS_Dimension_Texture3D:
			if (arraySize != 1 || !(header->flags & HbFile_DDS_Flags_Volume)) {
				return HbNull;
			}
			dimensions = HbGPU_Image_Dimensions_3D;
			depth = header->depth;
			if (depth == 0) {
				return HbNull;
			}
			break;
		default:
			return HbNull;
		}
		if (headerDXT10->dxgiFormat >= HbArrayLength(HbFile_DDS_DXGIFormatsToImageFormats)) {
			return HbNull;
		}
		format = HbFile_DDS_DXGIFormatsToImageFormats[headerDXT10->dxgiFormat];
	} else {
		if (header->flags & HbFile_DDS_Flags_Volume) {
			dimensions = HbGPU_Image_Dimensions_3D;
			depth = header->depth;
			if (depth == 0) {
				return HbNull;
			}
		} else if (header->caps2 & HbFile_DDS_Caps2_Cubemap) {
			if ((header->caps2 & HbFile_DDS_Caps2_CubemapAllFaces) != HbFile_DDS_Caps2_CubemapAllFaces) {
				return HbNull;
			}
			dimensions = HbGPU_Image_Dimensions_Cube;
		} else {
			dimensions = HbGPU_Image_Dimensions_2D;
		}
		if (fourCC != 0) {
			for (size_t mappingIndex = 0; mappingIndex < HbArrayLength(HbFile_DDS_FourCCImageFormatMappings); ++mappingIndex) {
				HbFile_DDS_FourCCImageFormatMapping const * mapping = &HbFile_DDS_FourCCImageFormatMappings[mappingIndex];
				if (fourCC == mapping->fourCC) {
					format = mapping->imageFormat;
					break;
				}
			}
		} else {
			uint32_t rBitMask = 0, gBitMask = 0, bBitMask = 0, aBitMask = 0;
			if (header->pixelFormat.flags & HbFile_DDS_PixelFormat_Flags_Alpha) {
				aBitMask = header->pixelFormat.aBitMask;
			} else if (header->pixelFormat.flags & HbFile_DDS_PixelFormat_Flags_Luminance) {
				rBitMask = header->pixelFormat.rBitMask;
				if (header->pixelFormat.flags & HbFile_DDS_PixelFormat_Flags_AlphaPixels) {
					aBitMask = header->pixelFormat.aBitMask;
				}
			} else {
				if (header->pixelFormat.flags & (HbFile_DDS_PixelFormat_Flags_RGB | HbFile_DDS_PixelFormat_Flags_YUV)) {
					rBitMask = header->pixelFormat.rBitMask;
					gBitMask = header->pixelFormat.gBitMask;
					bBitMask = header->pixelFormat.bBitMask;
				}
				if (header->pixelFormat.flags & HbFile_DDS_PixelFormat_Flags_AlphaPixels) {
					aBitMask = header->pixelFormat.aBitMask;
				}
			}
			for (size_t mappingIndex = 0; mappingIndex < HbArrayLength(HbFile_DDS_PixelFormatImageFormatMappings); ++mappingIndex) {
				HbFile_DDS_PixelFormatImageFormatMapping const * mapping = &HbFile_DDS_PixelFormatImageFormatMappings[mappingIndex];
				if (header->pixelFormat.rgbBitCount == mapping->rgbBitCount &&
						rBitMask == mapping->rBitMask && gBitMask == mapping->gBitMask && bBitMask == mapping->bBitMask && aBitMask == mapping->aBitMask) {
					format = mapping->imageFormat;
					break;
				}
			}
		}
	}
	info->format = format;
	info->dimensions = dimensions;
	info->width = header->width;
	info->height = header->height;
	info->depthOrLayers = dimensions == HbGPU_Image_Dimensions_3D ? depth : arraySize;
	info->mips = mipMapCount;
	info->samplesLog2 = 0;
	info->usageOptions = 0;
	if (!HbGPU_Image_Info_CleanupAndValidate(info)) {
		return HbNull;
	}
	uint32_t formatElementSize = HbGPU_Image_Copy_ElementSize(format, HbFalse);
	HbBool formatIs4x4 = HbGPU_Image_Format_Is4x4(format);
	uint64_t requiredImageDataSize = 0;
	for (uint32_t mip = 0; mip < mipMapCount; ++mip) {
		uint32_t mipWidth = header->width, mipHeight = header->height, mipDepth = depth;
		HbGPU_Image_MipSize(mip, dimensions, &mipWidth, &mipHeight, &mipDepth);
		if (formatIs4x4) {
			mipWidth = (mipWidth + 3) >> 2;
			mipHeight = (mipHeight + 3) >> 2;
		}
		requiredImageDataSize += ((uint64_t) (formatElementSize * mipWidth)) * mipHeight * mipDepth;
	}
	if (HbGPU_Image_Dimensions_AreCube(dimensions)) {
		requiredImageDataSize *= 6;
	}
	if (HbGPU_Image_Dimensions_AreArray(dimensions)) {
		requiredImageDataSize *= arraySize;
	}
	if (ddsSize < imageDataOffset + requiredImageDataSize) {
		return HbNull;
	}
	return (uint8_t const *) dds + imageDataOffset;
}
