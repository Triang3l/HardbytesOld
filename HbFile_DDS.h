#ifndef HbInclude_HbFile_DDS
#define HbInclude_HbFile_DDS
#include "HbGPU.h"

#define HbFile_DDS_Magic 0x20534444

typedef uint32_t HbFile_DDS_Flags;
enum {
	HbFile_DDS_Flags_Caps = 1 << 0,
	HbFile_DDS_Flags_Height = 1 << 1,
	HbFile_DDS_Flags_Width = 1 << 2,
	HbFile_DDS_Flags_Pitch = 1 << 3,
	HbFile_DDS_Flags_PixelFormat = 1 << 12,
	HbFile_DDS_Flags_MipMapCount = 1 << 17,
	HbFile_DDS_Flags_LinearSize = 1 << 19,
	HbFile_DDS_Flags_Volume = 1 << 23,

	HbFile_DDS_Flags_Required = HbFile_DDS_Flags_Caps | HbFile_DDS_Flags_Height | HbFile_DDS_Flags_Width | HbFile_DDS_Flags_PixelFormat,
};

typedef uint32_t HbFile_DDS_PixelFormat_Flags;
enum {
	HbFile_DDS_PixelFormat_Flags_AlphaPixels = 1 << 0,
	HbFile_DDS_PixelFormat_Flags_Alpha = 1 << 1,
	HbFile_DDS_PixelFormat_Flags_FourCC = 1 << 2,
	HbFile_DDS_PixelFormat_Flags_RGB = 1 << 6,
	HbFile_DDS_PixelFormat_Flags_YUV = 1 << 9,
	HbFile_DDS_PixelFormat_Flags_Luminance = 1 << 17,
};

typedef uint32_t HbFile_DDS_Caps;
enum {
	HbFile_DDS_Caps_Complex = 1 << 3,
	HbFile_DDS_Caps_Texture = 1 << 12,
	HbFile_DDS_Caps_Mipmap = 1 << 22,
};

typedef uint32_t HbFile_DDS_Caps2;
enum {
	HbFile_DDS_Caps2_Cubemap = 1 << 9,
	HbFile_DDS_Caps2_CubemapXP = 1 << 10,
	HbFile_DDS_Caps2_CubemapXN = 1 << 11,
	HbFile_DDS_Caps2_CubemapYP = 1 << 12,
	HbFile_DDS_Caps2_CubemapYN = 1 << 13,
	HbFile_DDS_Caps2_CubemapZP = 1 << 14,
	HbFile_DDS_Caps2_CubemapZN = 1 << 15,
	// DDSTextureLoader uses HbFile_DDS_Flags_Volume.
	// HbFile_DDS_Caps2_Volume = 1 << 21,

	HbFile_DDS_Caps2_CubemapAllFaces = HbFile_DDS_Caps2_CubemapXP | HbFile_DDS_Caps2_CubemapXN |
			HbFile_DDS_Caps2_CubemapYP | HbFile_DDS_Caps2_CubemapYN | HbFile_DDS_Caps2_CubemapZP | HbFile_DDS_Caps2_CubemapZN,
};

typedef uint32_t HbFile_DDS_FourCC;
enum {
	HbFile_DDS_FourCC_DX10 = 0x30315844,
	HbFile_DDS_FourCC_DXT1 = 0x31545844,
	HbFile_DDS_FourCC_DXT2 = 0x32545844,
	HbFile_DDS_FourCC_DXT3 = 0x33545844,
	HbFile_DDS_FourCC_DXT4 = 0x34545844,
	HbFile_DDS_FourCC_DXT5 = 0x35545844,
};

typedef struct HbFile_DDS_PixelFormat {
	uint32_t structSize;
	HbFile_DDS_PixelFormat_Flags flags;
	uint32_t fourCC;
	uint32_t rgbBitCount;
	uint32_t rBitMask;
	uint32_t gBitMask;
	uint32_t bBitMask;
	uint32_t aBitMask;
} HbFile_DDS_PixelFormat;

typedef struct HbFile_DDS_Header {
	uint32_t structSize;
	HbFile_DDS_Flags flags;
	uint32_t height;
	uint32_t width;
	uint32_t pitchOrLinearSize; // Ignore - not written reliably by D3DX, ignored by DDSTextureLoader.
	uint32_t depth;
	uint32_t mipMapCount;
	uint32_t reserved1[11];
	HbFile_DDS_PixelFormat pixelFormat;
	HbFile_DDS_Caps caps;
	HbFile_DDS_Caps2 caps2;
	uint32_t caps3;
	uint32_t caps4;
	uint32_t reserved2;
} HbFile_DDS_Header;

typedef uint32_t HbFile_DDS_DXGIFormat;
enum {
	// Only known HbGPU_Image_Formats.
	HbFile_DDS_DXGIFormat_R32G32B32A32_Float = 2,
	HbFile_DDS_DXGIFormat_R32G32B32A32_UInt,
	HbFile_DDS_DXGIFormat_R32G32B32A32_SInt,
	HbFile_DDS_DXGIFormat_R16G16B16A16_Float = 10,
	HbFile_DDS_DXGIFormat_R16G16B16A16_UNorm,
	HbFile_DDS_DXGIFormat_R16G16B16A16_UInt,
	HbFile_DDS_DXGIFormat_R16G16B16A16_SNorm,
	HbFile_DDS_DXGIFormat_R16G16B16A16_SInt,
	HbFile_DDS_DXGIFormat_R32G32_Float = 16,
	HbFile_DDS_DXGIFormat_R32G32_UInt,
	HbFile_DDS_DXGIFormat_R32G32_SInt,
	HbFile_DDS_DXGIFormat_R10G10B10A2_UNorm = 24,
	HbFile_DDS_DXGIFormat_R10G10B10A2_UInt,
	HbFile_DDS_DXGIFormat_R11G11B10_Float = 26,
	HbFile_DDS_DXGIFormat_R8G8B8A8_UNorm = 28,
	HbFile_DDS_DXGIFormat_R8G8B8A8_UNorm_sRGB,
	HbFile_DDS_DXGIFormat_R8G8B8A8_UInt,
	HbFile_DDS_DXGIFormat_R8G8B8A8_SNorm,
	HbFile_DDS_DXGIFormat_R8G8B8A8_SInt,
	HbFile_DDS_DXGIFormat_R16G16_Float = 34,
	HbFile_DDS_DXGIFormat_R16G16_UNorm,
	HbFile_DDS_DXGIFormat_R16G16_UInt,
	HbFile_DDS_DXGIFormat_R16G16_SNorm,
	HbFile_DDS_DXGIFormat_R16G16_SInt,
	HbFile_DDS_DXGIFormat_R32_Float = 41,
	HbFile_DDS_DXGIFormat_R32_UInt,
	HbFile_DDS_DXGIFormat_R32_SInt,
	HbFile_DDS_DXGIFormat_R8G8_UNorm = 49,
	HbFile_DDS_DXGIFormat_R8G8_UInt,
	HbFile_DDS_DXGIFormat_R8G8_SNorm,
	HbFile_DDS_DXGIFormat_R8G8_SInt,
	HbFile_DDS_DXGIFormat_R16_Float = 54,
	HbFile_DDS_DXGIFormat_R16_UNorm = 56,
	HbFile_DDS_DXGIFormat_R16_UInt,
	HbFile_DDS_DXGIFormat_R16_SNorm,
	HbFile_DDS_DXGIFormat_R16_SInt,
	HbFile_DDS_DXGIFormat_R8_UNorm = 61,
	HbFile_DDS_DXGIFormat_R8_UInt,
	HbFile_DDS_DXGIFormat_R8_SNorm,
	HbFile_DDS_DXGIFormat_R8_SInt,
	HbFile_DDS_DXGIFormat_BC1_UNorm = 71,
	HbFile_DDS_DXGIFormat_BC1_UNorm_sRGB,
	HbFile_DDS_DXGIFormat_BC2_UNorm = 74,
	HbFile_DDS_DXGIFormat_BC2_UNorm_sRGB,
	HbFile_DDS_DXGIFormat_BC3_UNorm = 77,
	HbFile_DDS_DXGIFormat_BC3_UNorm_sRGB,
	HbFile_DDS_DXGIFormat_BC4_UNorm = 80,
	HbFile_DDS_DXGIFormat_BC4_SNorm,
	HbFile_DDS_DXGIFormat_BC5_UNorm = 83,
	HbFile_DDS_DXGIFormat_BC5_SNorm,
	HbFile_DDS_DXGIFormat_B5G6R5_UNorm = 85,
	HbFile_DDS_DXGIFormat_B5G5R5A1_UNorm = 86,
	HbFile_DDS_DXGIFormat_B8G8R8A8_UNorm = 87,
	HbFile_DDS_DXGIFormat_B8G8R8A8_UNorm_sRGB = 91,
	HbFile_DDS_DXGIFormat_BC6H_UF16 = 95,
	HbFile_DDS_DXGIFormat_BC6H_SF16,
	HbFile_DDS_DXGIFormat_BC7_UNorm = 98,
	HbFile_DDS_DXGIFormat_BC7_UNorm_sRGB,
	HbFile_DDS_DXGIFormat_B4G4R4A4_UNorm = 115,
};

typedef uint32_t HbFile_DDS_Dimension;
enum {
	HbFile_DDS_Dimension_Texture1D = 2,
	HbFile_DDS_Dimension_Texture2D,
	HbFile_DDS_Dimension_Texture3D,
};

typedef uint32_t HbFile_DDS_MiscFlags;
enum {
	HbFile_DDS_MiscFlags_TextureCube = 1 << 2,
};

typedef uint32_t HbFile_DDS_AlphaMode;
enum {
	HbFile_DDS_AlphaMode_Unknown,
	HbFile_DDS_AlphaMode_Straight,
	HbFile_DDS_AlphaMode_Premultiplied,
	HbFile_DDS_AlphaMode_Opaque,
	HbFile_DDS_AlphaMode_Custom,
};

typedef struct HbFile_DDS_HeaderDXT10 {
	HbFile_DDS_DXGIFormat dxgiFormat;
	HbFile_DDS_Dimension dimension;
	HbFile_DDS_MiscFlags miscFlags;
	uint32_t arraySize;
	HbFile_DDS_AlphaMode alphaMode : 3;
	uint32_t miscFlags2Reserved : 29;
} HbFile_DDS_HeaderDXT10;

// dds must be 4-aligned. Returns pointer to the texture data if successful, with byte-aligned rows and slices.
void const * HbFile_DDS_ValidateAndGetInfo(void const * dds, size_t ddsSize, HbGPU_Image_Info * info);

#endif
