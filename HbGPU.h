#ifndef HbInclude_HbGPU
#define HbInclude_HbGPU
#include "HbMemory.h"
#include "HbText.h"

#if HbPlatform_OS_Windows
#define HbGPU_Implementation_D3D 1
#else
#error No GPU implementation for the target OS.
#endif

#if HbGPU_Implementation_D3D
#ifndef COBJMACROS
#define COBJMACROS
#endif
#include <d3d12.h>
#include <dxgi1_4.h>
#endif

HbBool HbGPU_Init(HbBool debug);
void HbGPU_Shutdown();

typedef enum HbGPU_Comparison {
	HbGPU_Comparison_Never,        // 000
	HbGPU_Comparison_Less,         // 001
	HbGPU_Comparison_Equal,        // 010
	HbGPU_Comparison_LessEqual,    // 011
	HbGPU_Comparison_Greater,      // 100
	HbGPU_Comparison_NotEqual,     // 101
	HbGPU_Comparison_GreaterEqual, // 110
	HbGPU_Comparison_Always,       // 111
} HbGPU_Comparison;

/**************************************
 * Command queue types for submissions
 **************************************/

typedef enum HbGPU_CmdQueue {
	HbGPU_CmdQueue_Graphics,
	HbGPU_CmdQueue_Copy,

	HbGPU_CmdQueue_QueueCount,
} HbGPU_CmdQueue;

/*******************************************************
 * Device executing command queues and owning resources
 *******************************************************/

typedef struct HbGPU_Device {
	#if HbGPU_Implementation_D3D
	HbMemory_Tag * d3dMemoryTag;
	IDXGIAdapter3 * dxgiAdapter;
	ID3D12Device * d3dDevice;
	ID3D12CommandQueue * d3dCommandQueues[HbGPU_CmdQueue_QueueCount];
	uint32_t d3dViewDescriptorSize;
	uint32_t d3dSamplerDescriptorSize;
	uint32_t d3dRTVDescriptorSize;
	uint32_t d3dDSVDescriptorSize;
	#endif
} HbGPU_Device;

// Device index is implementaton-defined index of the device in the system.
HbBool HbGPU_Device_Init(HbGPU_Device * device, HbTextU8 const * name, uint32_t deviceIndex,
		HbBool needGraphicsQueue, HbBool needCopyQueue);
void HbGPU_Device_Shutdown(HbGPU_Device * device);

/****************************************
 * Fence for awaiting the GPU on the CPU
 ****************************************/

typedef struct HbGPU_Fence {
	HbGPU_Device * device;
	HbGPU_CmdQueue queue;
	#if HbGPU_Implementation_D3D
	ID3D12Fence * d3dFence;
	HANDLE d3dCompletionEvent;
	uint64_t d3dAwaitedValue;
	#endif
} HbGPU_Fence;

HbBool HbGPU_Fence_Init(HbGPU_Fence * fence, HbTextU8 const * name, HbGPU_Device * device, HbGPU_CmdQueue queue);
void HbGPU_Fence_Destroy(HbGPU_Fence * fence);
void HbGPU_Fence_Enqueue(HbGPU_Fence * fence);
HbBool HbGPU_Fence_IsCrossed(HbGPU_Fence * fence);
void HbGPU_Fence_Await(HbGPU_Fence * fence);

/*********
 * Buffer
 *********/

typedef enum HbGPU_Buffer_Access {
	HbGPU_Buffer_Access_GPU,
	HbGPU_Buffer_Access_CPUToGPU,
	HbGPU_Buffer_Access_GPUToCPU,
} HbGPU_Buffer_Access;

typedef uint32_t HbGPU_Buffer_Usage;
enum {
	// Read usage modes can be combined, other ones are exclusive.

	HbGPU_Buffer_Usage_Read_Vertices = 1,
	HbGPU_Buffer_Usage_Read_Constants = HbGPU_Buffer_Usage_Read_Vertices << 1,
	HbGPU_Buffer_Usage_Read_Indices = HbGPU_Buffer_Usage_Read_Constants << 1,
	HbGPU_Buffer_Usage_Read_ResourceNonPS = HbGPU_Buffer_Usage_Read_Indices << 1,
	HbGPU_Buffer_Usage_Read_ResourcePS = HbGPU_Buffer_Usage_Read_ResourceNonPS << 1,
	HbGPU_Buffer_Usage_Read_CopySource = HbGPU_Buffer_Usage_Read_ResourcePS << 1,

	HbGPU_Buffer_Usage_ShaderEdit = HbGPU_Buffer_Usage_Read_CopySource << 1,
	HbGPU_Buffer_Usage_CopyTarget = HbGPU_Buffer_Usage_ShaderEdit << 1,
	// Being used on copy queue, for instance.
	HbGPU_Buffer_Usage_CrossQueue = HbGPU_Buffer_Usage_CopyTarget << 1,
	// Special usage for buffers in uploading memory.
	HbGPU_Buffer_Usage_CPUToGPU = HbGPU_Buffer_Usage_CrossQueue << 1,
};

// Nvidia (and Direct3D 12) requirement.
#define HbGPU_Buffer_ConstantsAlignment 256

typedef struct HbGPU_Buffer {
	HbGPU_Buffer_Access access;
	uint32_t size;
	#if HbGPU_Implementation_D3D
	ID3D12Resource * d3dResource;
	D3D12_GPU_VIRTUAL_ADDRESS d3dGPUAddress;
	#endif
} HbGPU_Buffer;

HbBool HbGPU_Buffer_Init(HbGPU_Buffer * buffer, HbTextU8 const * name, HbGPU_Device * device,
		HbGPU_Buffer_Access access, uint32_t size, HbBool shaderEditable, HbGPU_Buffer_Usage initialUsage);
void HbGPU_Buffer_Destroy(HbGPU_Buffer * buffer);
void * HbGPU_Buffer_Map(HbGPU_Buffer * buffer, uint32_t readStart, uint32_t readLength);
void HbGPU_Buffer_Unmap(HbGPU_Buffer * buffer, uint32_t writeStart, uint32_t writeLength);

/********
 * Image
 ********/

typedef enum HbGPU_Image_Format {
	HbGPU_Image_Format_RawStart,
		HbGPU_Image_Format_8_R_UNorm = HbGPU_Image_Format_RawStart,
		HbGPU_Image_Format_8_8_RG_UNorm,
		HbGPU_Image_Format_8_8_8_8_RGBA_UNorm,
		HbGPU_Image_Format_8_8_8_8_RGBA_sRGB,
		HbGPU_Image_Format_32_UInt,
		HbGPU_Image_Format_32_Float,
	HbGPU_Image_Format_RawEnd = HbGPU_Image_Format_32_Float,
	HbGPU_Image_Format_DepthStart,
		HbGPU_Image_Format_D32 = HbGPU_Image_Format_DepthStart,
		HbGPU_Image_Format_DepthAndStencilStart,
			HbGPU_Image_Format_D32_S8 = HbGPU_Image_Format_DepthAndStencilStart,
		HbGPU_Image_Format_DepthAndStencilEnd = HbGPU_Image_Format_D32_S8,
	HbGPU_Image_Format_DepthEnd = HbGPU_Image_Format_DepthAndStencilEnd,
	HbGPU_Image_Format_FormatCount,
} HbGPU_Image_Format;
HbForceInline HbBool HbGPU_Image_Format_IsDepth(HbGPU_Image_Format format) {
	return format >= HbGPU_Image_Format_DepthStart && format <= HbGPU_Image_Format_DepthEnd;
}
HbForceInline HbBool HbGPU_Image_Format_HasStencil(HbGPU_Image_Format format) {
	return format >= HbGPU_Image_Format_DepthAndStencilStart && format <= HbGPU_Image_Format_DepthAndStencilEnd;
}
HbForceInline HbGPU_Image_Format HbGPU_Image_Format_ToLinear(HbGPU_Image_Format format) {
	return format == HbGPU_Image_Format_8_8_8_8_RGBA_sRGB ? HbGPU_Image_Format_8_8_8_8_RGBA_UNorm : format;
}
// Element is either a texel (for uncompressed formats) or a block (for compressed formats).
uint32_t HbGPU_Image_Format_ElementSize(HbGPU_Image_Format format);

typedef enum HbGPU_Image_Dimensions {
	HbGPU_Image_Dimensions_1D,
	HbGPU_Image_Dimensions_1DArray,
	HbGPU_Image_Dimensions_2D,
	HbGPU_Image_Dimensions_2DArray,
	HbGPU_Image_Dimensions_Cube,
	HbGPU_Image_Dimensions_CubeArray,
	HbGPU_Image_Dimensions_3D,
} HbGPU_Image_Dimensions;
HbForceInline HbBool HbGPU_Image_Dimensions_AreArray(HbGPU_Image_Dimensions dimensions) {
	return dimensions == HbGPU_Image_Dimensions_1DArray || dimensions == HbGPU_Image_Dimensions_2DArray ||
			dimensions == HbGPU_Image_Dimensions_CubeArray;
}
HbForceInline HbBool HbGPU_Image_Dimensions_Are1D(HbGPU_Image_Dimensions dimensions) {
	return dimensions == HbGPU_Image_Dimensions_1D || dimensions == HbGPU_Image_Dimensions_1DArray;
}
HbForceInline HbBool HbGPU_Image_Dimensions_Are2D(HbGPU_Image_Dimensions dimensions) {
	return dimensions == HbGPU_Image_Dimensions_2D || dimensions == HbGPU_Image_Dimensions_2DArray;
}
HbForceInline HbBool HbGPU_Image_Dimensions_AreCube(HbGPU_Image_Dimensions dimensions) {
	return dimensions == HbGPU_Image_Dimensions_Cube || dimensions == HbGPU_Image_Dimensions_CubeArray;
}

// Limits, based on the targeted D3D feature level 11_0, to prevent overflows (in file loading, for instance),
// and also for use in bitfields.
enum {
	HbGPU_Image_MaxSize1D2DLog2 = 14,
	HbGPU_Image_MaxSize1D2D = 1 << HbGPU_Image_MaxSize1D2DLog2,
	HbGPU_Image_MaxSize3DLog2 = 11,
	HbGPU_Image_MaxSize3D = 1 << HbGPU_Image_MaxSize3DLog2,
	HbGPU_Image_MipCountBits = 4,
	HbGPU_Image_MaxLayersLog2 = 11,
	HbGPU_Image_MaxLayers = 1 << HbGPU_Image_MaxLayersLog2,
	HbGPU_Image_MaxLayersCube = HbGPU_Image_MaxLayers / 6,
	HbGPU_Image_MaxSamplesLog2 = 4,
};

typedef uint32_t HbGPU_Image_UsageOptions;
enum {
	// Certain color formats only - can be written to in shaders.
	HbGPU_Image_UsageOptions_ShaderEditable = 1,
	// Certain color formats only - can be bound as a color render target.
	HbGPU_Image_UsageOptions_ColorRenderable = HbGPU_Image_UsageOptions_ShaderEditable << 1,
	// For depth buffers - optimization, can't use as a texture.
	HbGPU_Image_UsageOptions_DepthTestOnly = HbGPU_Image_UsageOptions_ColorRenderable << 1,
};

typedef struct HbGPU_Image_Info {
	HbGPU_Image_Format format;
	HbGPU_Image_Dimensions dimensions;
	uint32_t width;
	uint32_t height; // Must be 1 for 1D.
	uint32_t depthOrLayers; // Must be 1 for non-arrays and non-3D.
	uint32_t mips; // Must be at least 1, mips are not allowed for 1D (Metal restriction) and multisampled images.
	uint32_t samplesLog2; // For 2D non-arrays only - in other cases, only 0 is allowed.
	HbGPU_Image_UsageOptions usageOptions;
} HbGPU_Image_Info;

HbBool HbGPU_Image_Info_CleanupAndValidate(HbGPU_Image_Info * info);

typedef struct HbGPU_Image {
	HbGPU_Image_Info info;
	#if HbGPU_Implementation_D3D
	ID3D12Resource * d3dResource;
	#endif
} HbGPU_Image;

typedef uint32_t HbGPU_Image_Usage;
enum {
	// Read usage modes can be combined, other ones are exclusive.

	HbGPU_Image_Usage_Read_TexturePS = 1,
	HbGPU_Image_Usage_Read_TextureNonPS = HbGPU_Image_Usage_Read_TexturePS << 1,
	HbGPU_Image_Usage_Read_DepthReject = HbGPU_Image_Usage_Read_TextureNonPS << 1,
	HbGPU_Image_Usage_Read_CopySource = HbGPU_Image_Usage_Read_DepthReject << 1,
	HbGPU_Image_Usage_Read_ResolveSource = HbGPU_Image_Usage_Read_CopySource << 1,

	HbGPU_Image_Usage_ColorRT = HbGPU_Image_Usage_Read_ResolveSource << 1,
	HbGPU_Image_Usage_DepthTest = HbGPU_Image_Usage_ColorRT << 1,
	HbGPU_Image_Usage_ShaderEdit = HbGPU_Image_Usage_DepthTest << 1,
	HbGPU_Image_Usage_CopyTarget = HbGPU_Image_Usage_ShaderEdit << 1,
	HbGPU_Image_Usage_ResolveTarget = HbGPU_Image_Usage_CopyTarget << 1,
	HbGPU_Image_Usage_Present = HbGPU_Image_Usage_ResolveTarget << 1,
	// Being used on copy queue, for instance.
	HbGPU_Image_Usage_CrossQueue = HbGPU_Image_Usage_Present << 1,
};

typedef union HbGPU_Image_ClearValue {
	float color[4];
	struct {
		float depth;
		uint8_t stencil;
	} depthStencil;
} HbGPU_Image_ClearValue;

typedef struct HbGPU_Image_Slice {
	uint32_t mip : HbGPU_Image_MipCountBits;
	uint32_t cubeSide : 3;
	uint32_t layer : HbGPU_Image_MaxLayersLog2;
	uint32_t stencil : 1;
} HbGPU_Image_Slice;

// Assumes VALID image->info!
HbBool HbGPU_Image_InitWithInfo(HbGPU_Image * image, HbTextU8 const * name, HbGPU_Device * device,
		HbGPU_Image_Usage initialUsage, HbGPU_Image_ClearValue const * optimalClearValue);
void HbGPU_Image_Destroy(HbGPU_Image * image);

/*******************************************************
 * Storage of binding descriptors of buffers and images
 *******************************************************/

typedef struct HbGPU_HandleStore {
	HbGPU_Device * device;
	#if HbGPU_Implementation_D3D
	ID3D12DescriptorHeap * d3dHeap;
	D3D12_CPU_DESCRIPTOR_HANDLE d3dHeapCPUStart;
	D3D12_GPU_DESCRIPTOR_HANDLE d3dHeapGPUStart;
	#endif
} HbGPU_HandleStore;

HbBool HbGPU_HandleStore_Init(HbGPU_HandleStore * store, HbTextU8 const * name, HbGPU_Device * device, uint32_t handleCount);
void HbGPU_HandleStore_Destroy(HbGPU_HandleStore * store);
void HbGPU_HandleStore_SetConstantBuffer(HbGPU_HandleStore * store, uint32_t index,
		HbGPU_Buffer * buffer, uint32_t offset, uint32_t size);
void HbGPU_HandleStore_SetTexelResourceBuffer(HbGPU_HandleStore * store, uint32_t index,
		HbGPU_Buffer * buffer, HbGPU_Image_Format format, uint32_t offsetInTexels, uint32_t texelCount);
void HbGPU_HandleStore_SetStructResourceBuffer(HbGPU_HandleStore * store, uint32_t index,
		HbGPU_Buffer * buffer, uint32_t structSize, uint32_t offsetInStructs, uint32_t structCount);
void HbGPU_HandleStore_SetEditBuffer(HbGPU_HandleStore * store, uint32_t index,
		HbGPU_Buffer * buffer, HbGPU_Image_Format format, uint32_t offsetInTexels, uint32_t texelCount);
void HbGPU_HandleStore_SetTexture(HbGPU_HandleStore * store, uint32_t index, HbGPU_Image * image, HbBool stencil);
void HbGPU_HandleStore_SetNullTexture(HbGPU_HandleStore * store, uint32_t index, HbGPU_Image_Dimensions dimensions, HbBool multisample);
// The implementation may ignore the specified mip level if shaders can access all mips.
void HbGPU_HandleStore_SetEditImage(HbGPU_HandleStore * store, uint32_t index, HbGPU_Image * image, uint32_t mip);

/**********
 * Sampler
 **********/

typedef enum HbGPU_Sampler_Filter {
	HbGPU_Sampler_Filter_Point,
	HbGPU_Sampler_Filter_Bilinear,
	HbGPU_Sampler_Filter_Trilinear,
	HbGPU_Sampler_Filter_Aniso2x,
	HbGPU_Sampler_Filter_Aniso4x,
	HbGPU_Sampler_Filter_Aniso8x,
	HbGPU_Sampler_Filter_Aniso16,

	HbGPU_Sampler_Filter_BitCount = 3,
} HbGPU_Sampler_Filter;

typedef enum HbGPU_Sampler_Wrap {
	HbGPU_Sampler_Wrap_Repeat,
	HbGPU_Sampler_Wrap_Clamp,
	HbGPU_Sampler_Wrap_MirrorRepeat,
	HbGPU_Sampler_Wrap_MirrorClamp,
	HbGPU_Sampler_Wrap_Border,

	HbGPU_Sampler_Wrap_BitCount = 3,
} HbGPU_Sampler_Wrap;

typedef enum HbGPU_Sampler_Border {
	HbGPU_Sampler_Border_RGB0_A0,
	HbGPU_Sampler_Border_RGB0_A1,
	HbGPU_Sampler_Border_RGB1_A1,

	HbGPU_Sampler_Border_BitCount = 2,
} HbGPU_Sampler_Border;

typedef struct HbGPU_Sampler_Info {
	uint32_t filter : HbGPU_Sampler_Filter_BitCount;
	uint32_t wrapS : HbGPU_Sampler_Wrap_BitCount;
	uint32_t wrapT : HbGPU_Sampler_Wrap_BitCount;
	uint32_t wrapR : HbGPU_Sampler_Wrap_BitCount;
	uint32_t border : HbGPU_Sampler_Border_BitCount;
	uint32_t mipMostDetailed : HbGPU_Image_MipCountBits;
	uint32_t mipLeastDetailed : HbGPU_Image_MipCountBits;
	uint32_t mipBias : HbGPU_Image_MipCountBits;
	HbBool isComparison : 1;
	uint32_t comparison : 3;
} HbGPU_Sampler_Info;

enum { HbGPU_Sampler_MipLeastDetailed_FullPyramid = (1 << HbGPU_Image_MipCountBits) - 1 };

typedef struct HbGPU_SamplerStore {
	HbGPU_Device * device;
	#if HbGPU_Implementation_D3D
	ID3D12DescriptorHeap * d3dHeap;
	D3D12_CPU_DESCRIPTOR_HANDLE d3dHeapCPUStart;
	D3D12_GPU_DESCRIPTOR_HANDLE d3dHeapGPUStart;
	#endif
} HbGPU_SamplerStore;

HbBool HbGPU_SamplerStore_Init(HbGPU_SamplerStore * store, HbTextU8 const * name, HbGPU_Device * device, uint32_t samplerCount);
void HbGPU_SamplerStore_Destroy(HbGPU_SamplerStore * store);
HbBool HbGPU_SamplerStore_CreateSampler(HbGPU_SamplerStore * store, uint32_t index, HbGPU_Sampler_Info info);
#if HbGPU_Implementation_D3D
// Not needed - samplers are purely descriptors.
#define HbGPU_SamplerStore_DestroySampler(store, index) {}
#else
void HbGPU_SamplerStore_DestroySampler(HbGPU_SamplerStore * store, uint32_t index);
#endif

/************************
 * Render target storage
 ************************/

// A bindable reference to a render target actually located in a store or some other place like a swap chain.
typedef struct HbGPU_RTReference {
	#if HbGPU_Implementation_D3D
	D3D12_CPU_DESCRIPTOR_HANDLE d3dHandle;
	ID3D12Resource * d3dResolveSource; // Null for non-MSAA RTs.
	#endif
} HbGPU_RTReference;

typedef struct HbGPU_RTStore {
	HbGPU_Device * device;
	HbBool isDepth;
	#if HbGPU_Implementation_D3D
	ID3D12DescriptorHeap * d3dHeap;
	D3D12_CPU_DESCRIPTOR_HANDLE d3dHeapStart;
	ID3D12Resource * * d3dResolveSources; // Null for non-MSAA RTs.
	#endif
} HbGPU_RTStore;

HbBool HbGPU_RTStore_Init(HbGPU_RTStore * store, HbTextU8 const * name, HbGPU_Device * device, HbBool isDepth, uint32_t rtCount);
void HbGPU_RTStore_Destroy(HbGPU_RTStore * store);
HbBool HbGPU_RTStore_SetColor(HbGPU_RTStore * store, uint32_t rtIndex,
		HbGPU_Image * image, HbGPU_Image_Slice slice, uint32_t zOf3D);
HbBool HbGPU_RTStore_SetDepth(HbGPU_RTStore * store, uint32_t rtIndex,
		HbGPU_Image * image, HbGPU_Image_Slice slice, HbBool readOnlyDepth, HbBool readOnlyStencil);
HbGPU_RTReference HbGPU_RTStore_GetRT(HbGPU_RTStore * store, uint32_t rtIndex);

#define HbGPU_RT_MaxCount 8

/**************************
 * Presentation swap chain
 **************************/

typedef struct HbGPU_SwapChain_Target {
	#if HbPlatform_OS_WindowsDesktop
	HWND windowsHWnd;
	#endif
} HbGPU_SwapChain_Target;

typedef struct HbGPU_SwapChain {
	HbGPU_Device * device;
	uint32_t bufferCount;
	HbGPU_Image images[3]; // May be modified during presentation (implementation-dependent).
	#if HbGPU_Implementation_D3D
	IDXGISwapChain3 * dxgiSwapChain;
	ID3D12DescriptorHeap * d3dRTVHeap;
	D3D12_CPU_DESCRIPTOR_HANDLE d3dRTVHeapStart;
	#endif
} HbGPU_SwapChain;

HbBool HbGPU_SwapChain_Init(HbGPU_SwapChain * chain, HbTextU8 const * name, HbGPU_Device * device,
		HbGPU_SwapChain_Target target, HbGPU_Image_Format format, uint32_t width, uint32_t height, HbBool tripleBuffered);
void HbGPU_SwapChain_Destroy(HbGPU_SwapChain * chain);

/******************
 * Shader programs
 ******************/

typedef enum HbGPU_ShaderStage {
	HbGPU_ShaderStage_Vertex,
	HbGPU_ShaderStage_Pixel,
	HbGPU_ShaderStage_Compute,
} HbGPU_ShaderStage;
typedef uint32_t HbGPU_ShaderStageBits;
enum {
	HbGPU_ShaderStageBits_Vertex = 1,
	HbGPU_ShaderStageBits_Pixel = HbGPU_ShaderStageBits_Vertex << 1,
	HbGPU_ShaderStageBits_Compute = HbGPU_ShaderStageBits_Pixel << 1,
};

typedef struct HbGPU_ShaderReference {
	#if HbGPU_Implementation_D3D
	void const * dxbc; // Null if the stage is not active.
	size_t dxbcSize;
	#endif
} HbGPU_ShaderReference;

/*****************
 * Binding layout
 *****************/

typedef struct HbGPU_Binding_RegisterIndex {
	uint32_t cbufferResourceEdit; // In Direct3D namespaces.
	uint32_t bufferImage; // In Metal namespaces.
} HbGPU_Binding_RegisterIndex;

typedef enum HbGPU_Binding_HandleRange_Type {
	HbGPU_Binding_HandleRange_Type_ConstantBuffer, // b# in both Direct3D and Metal.
	HbGPU_Binding_HandleRange_Type_ResourceBuffer, // t# in Direct3D, b# in Metal.
	HbGPU_Binding_HandleRange_Type_EditBuffer, // u# in Direct3D, b# in Metal.
	HbGPU_Binding_HandleRange_Type_Texture, // t# in Direct3D, t# in Metal.
	HbGPU_Binding_HandleRange_Type_EditImage, // u# in Direct3D, t# in Metal.
} HbGPU_Binding_HandleRange_Type;

typedef struct HbGPU_Binding_HandleRange {
	HbGPU_Binding_HandleRange_Type type;
	uint32_t handleOffset;
	uint32_t handleCount;
	HbGPU_Binding_RegisterIndex firstRegister;
} HbGPU_Binding_HandleRange;

typedef struct HbGPU_Binding_SamplerRange {
	uint32_t samplerOffset;
	uint32_t samplerCount;
	uint32_t firstRegister;
} HbGPU_Binding_SamplerRange;

typedef enum HbGPU_Binding_Type {
	HbGPU_Binding_Type_HandleRangeSet,
	HbGPU_Binding_Type_SamplerRangeSet,
	HbGPU_Binding_Type_ConstantBuffer,
	HbGPU_Binding_Type_SmallConstants,
} HbGPU_Binding_Type;

typedef struct HbGPU_Binding {
	HbGPU_Binding_Type type;
	HbGPU_ShaderStageBits stages; // On Direct3D, filtering works the "one stage or all stages" way.
	union {
		struct {
			HbGPU_Binding_HandleRange const * ranges;
			uint32_t rangeCount;
		} handleRangeSet;
		struct {
			HbGPU_Binding_SamplerRange const * ranges;
			uint32_t rangeCount;
			// If not null, the implementation may try to pre-compile the ranges using these samplers.
			// Some implementations do not support this - binding when drawing still must be done.
			HbGPU_Sampler_Info const * staticSamplers;
		} samplerRangeSet;
		struct {
			HbGPU_Binding_RegisterIndex bindRegister;
		} constantBuffer;
		struct {
			HbGPU_Binding_RegisterIndex bindRegister;
			uint32_t sizeInDwords;
		} smallConstants;
	} binding;
} HbGPU_Binding;

// Don't need too many. On AMD, more than 13 D3D root signature dwords is not recommended.
#define HbGPU_BindingLayout_MaxBindings 16

typedef struct HbGPU_BindingLayout {
	#if HbGPU_Implementation_D3D
	ID3D12RootSignature * d3dRootSignature;
	// Mappings of HbGPU_Binding indices to root signature indices (skipping static samplers).
	// UINT32_MAX for unmapped (including static samplers).
	uint32_t d3dRootSlots[HbGPU_BindingLayout_MaxBindings];
	#endif
} HbGPU_BindingLayout;

// The binding array is not stored, can be built locally and dynamically.
HbBool HbGPU_BindingLayout_Init(HbGPU_BindingLayout * layout, HbTextU8 const * name, HbGPU_Device * device,
		HbGPU_Binding const * bindings, uint32_t bindingCount, HbBool useVertexAttributes);
void HbGPU_BindingLayout_Destroy(HbGPU_BindingLayout * layout);

/****************
 * Vertex layout
 ****************/

typedef struct HbGPU_Vertex_Stream {
	uint32_t stride;
	uint32_t instanceStepRate; // 0 for per-vertex.
} HbGPU_Vertex_Stream;

typedef enum HbGPU_Vertex_Semantic {
	HbGPU_Vertex_Semantic_Position,
	HbGPU_Vertex_Semantic_Normal,
	HbGPU_Vertex_Semantic_Tangent,
	HbGPU_Vertex_Semantic_TexCoord,
	HbGPU_Vertex_Semantic_Color,
	HbGPU_Vertex_Semantic_BlendIndices,
	HbGPU_Vertex_Semantic_BlendWeights,
} HbGPU_Vertex_Semantic;

typedef enum HbGPU_Vertex_Format {
	HbGPU_Vertex_Format_Float_32x1,
	HbGPU_Vertex_Format_Float_32x2,
	HbGPU_Vertex_Format_Float_32x3,
	HbGPU_Vertex_Format_Float_32x4,
	HbGPU_Vertex_Format_Float_16x2,
	HbGPU_Vertex_Format_Float_16x4,
	HbGPU_Vertex_Format_Float_11_11_10,
	HbGPU_Vertex_Format_UNorm_16x2,
	HbGPU_Vertex_Format_UNorm_16x4,
	HbGPU_Vertex_Format_UNorm_10_10_10_2,
	HbGPU_Vertex_Format_UNorm_8x4,
	HbGPU_Vertex_Format_SNorm_16x2,
	HbGPU_Vertex_Format_SNorm_16x4,
	HbGPU_Vertex_Format_SNorm_8x4,
	HbGPU_Vertex_Format_UInt_32x1,
	HbGPU_Vertex_Format_UInt_32x2,
	HbGPU_Vertex_Format_UInt_32x3,
	HbGPU_Vertex_Format_UInt_32x4,
	HbGPU_Vertex_Format_UInt_16x2,
	HbGPU_Vertex_Format_UInt_16x4,
	HbGPU_Vertex_Format_UInt_10_10_10_2,
	HbGPU_Vertex_Format_UInt_8x4,
	HbGPU_Vertex_Format_SInt_32x1,
	HbGPU_Vertex_Format_SInt_32x2,
	HbGPU_Vertex_Format_SInt_32x3,
	HbGPU_Vertex_Format_SInt_32x4,
	HbGPU_Vertex_Format_SInt_16x2,
	HbGPU_Vertex_Format_SInt_16x4,
	HbGPU_Vertex_Format_SInt_8x4,
} HbGPU_Vertex_Format;

typedef struct HbGPU_Vertex_Attribute {
	uint32_t streamIndex;
	HbGPU_Vertex_Semantic semantic;
	uint32_t semanticIndex;
	HbGPU_Vertex_Format format;
	uint32_t offsetInDwords;
} HbGPU_Vertex_Attribute;

typedef uint16_t HbGPU_Vertex_Index;

/************************
 * Drawing configuration
 ************************/

typedef enum HbGPU_DrawConfig_InputPrimitive {
	HbGPU_DrawConfig_InputPrimitive_Triangle, // Zero (default).
	HbGPU_DrawConfig_InputPrimitive_Line,
	HbGPU_DrawConfig_InputPrimitive_Point,
} HbGPU_DrawConfig_InputPrimitive;

typedef enum HbGPU_DrawConfig_Stencil_Op {
	HbGPU_DrawConfig_Stencil_Op_Keep,
	HbGPU_DrawConfig_Stencil_Op_Zero,
	HbGPU_DrawConfig_Stencil_Op_Replace,
	HbGPU_DrawConfig_Stencil_Op_IncrementSaturate,
	HbGPU_DrawConfig_Stencil_Op_DecrementSaturate,
	HbGPU_DrawConfig_Stencil_Op_Invert,
	HbGPU_DrawConfig_Stencil_Op_Increment,
	HbGPU_DrawConfig_Stencil_Op_Decrement,
} HbGPU_DrawConfig_Stencil_Op;

typedef struct HbGPU_DrawConfig_Stencil_Side {
	HbGPU_DrawConfig_Stencil_Op fail;
	HbGPU_DrawConfig_Stencil_Op depthFail;
	HbGPU_DrawConfig_Stencil_Op pass;
	HbGPU_Comparison comparison;
} HbGPU_DrawConfig_Stencil_Side;

typedef struct HbGPU_DrawConfig_DepthStencilInfo {
	HbGPU_Image_Format format;
	HbBool depthTest;
	HbGPU_Comparison depthComparison;
	HbBool depthWrite;
	HbBool stencil;
	uint8_t stencilReadMask;
	uint8_t stencilWriteMask;
	HbGPU_DrawConfig_Stencil_Side stencilFront;
	HbGPU_DrawConfig_Stencil_Side stencilBack;
} HbGPU_DrawConfig_DepthStencilInfo;

typedef enum HbGPU_DrawConfig_RT_BlendFactor {
	HbGPU_DrawConfig_RT_BlendFactor_Zero,
	HbGPU_DrawConfig_RT_BlendFactor_One,
	HbGPU_DrawConfig_RT_BlendFactor_SourceColor,
	HbGPU_DrawConfig_RT_BlendFactor_OneMinusSourceColor,
	HbGPU_DrawConfig_RT_BlendFactor_SourceAlpha,
	HbGPU_DrawConfig_RT_BlendFactor_OneMinusSourceAlpha,
	HbGPU_DrawConfig_RT_BlendFactor_TargetColor,
	HbGPU_DrawConfig_RT_BlendFactor_OneMinusTargetColor,
	HbGPU_DrawConfig_RT_BlendFactor_TargetAlpha,
	HbGPU_DrawConfig_RT_BlendFactor_OneMinusTargetAlpha,
	HbGPU_DrawConfig_RT_BlendFactor_SourceAlphaSaturated,
	HbGPU_DrawConfig_RT_BlendFactor_Constant,
	HbGPU_DrawConfig_RT_BlendFactor_OneMinusConstant,
	HbGPU_DrawConfig_RT_BlendFactor_Source1Color,
	HbGPU_DrawConfig_RT_BlendFactor_OneMinusSource1Color,
	HbGPU_DrawConfig_RT_BlendFactor_Source1Alpha,
	HbGPU_DrawConfig_RT_BlendFactor_OneMinusSource1Alpha,
} HbGPU_DrawConfig_RT_BlendFactor;

typedef enum HbGPU_DrawConfig_RT_BlendOp {
	HbGPU_DrawConfig_RT_BlendOp_Add,
	HbGPU_DrawConfig_RT_BlendOp_Subtract,
	HbGPU_DrawConfig_RT_BlendOp_ReverseSubtract,
	HbGPU_DrawConfig_RT_BlendOp_Min,
	HbGPU_DrawConfig_RT_BlendOp_Max,
} HbGPU_DrawConfig_RT_BlendOp;

typedef struct HbGPU_DrawConfig_RT {
	HbGPU_Image_Format format;
	HbBool blend;
	uint8_t writeMask;
	HbGPU_DrawConfig_RT_BlendFactor blendFactorSourceRGB;
	HbGPU_DrawConfig_RT_BlendFactor blendFactorSourceAlpha;
	HbGPU_DrawConfig_RT_BlendFactor blendFactorTargetRGB;
	HbGPU_DrawConfig_RT_BlendFactor blendFactorTargetAlpha;
	HbGPU_DrawConfig_RT_BlendOp blendOpRGB;
	HbGPU_DrawConfig_RT_BlendOp blendOpAlpha;
} HbGPU_DrawConfig_RT;

typedef struct HbGPU_DrawConfig_Info {
	// Pointers preferred for nested structures for easier reuse of common state parameters.

	HbGPU_ShaderReference shaderVertex;
	HbGPU_ShaderReference shaderPixel;
	HbGPU_BindingLayout * bindingLayout;

	HbGPU_DrawConfig_InputPrimitive inputPrimitive;

	uint32_t vertexStreamCount;
	uint32_t vertexAttributeCount;
	HbGPU_Vertex_Stream const * vertexStreams;
	HbGPU_Vertex_Attribute const * vertexAttributes;

	HbBool frontCounterClockwise;
	int32_t cullSide; // -1 - cull back faces, 0 - don't cull, 1 - cull front faces.
	uint32_t samplesLog2;
	HbBool wireframe;

	int32_t depthBias;
	float depthBiasClamp;
	float depthBiasSlope;
	HbBool depthClamp;

	HbGPU_DrawConfig_DepthStencilInfo const * depthStencil; // Null if not using a depth/stencil buffer.

	uint32_t rtCount;
	HbGPU_DrawConfig_RT const * rts;
	HbBool rtsSameBlendAndWriteMasks;
	HbBool alphaToCoverage;
} HbGPU_DrawConfig_Info;

typedef struct HbGPU_DrawConfig {
	#if HbGPU_Implementation_D3D
	ID3D12PipelineState * d3dPipelineState;
	#endif
} HbGPU_DrawConfig;

HbBool HbGPU_DrawConfig_Init(HbGPU_DrawConfig * config, HbTextU8 const * name, HbGPU_Device * device, HbGPU_DrawConfig_Info const * info);
void HbGPU_DrawConfig_Destroy(HbGPU_DrawConfig * config);

/****************************
 * Computation configuration
 ****************************/

typedef struct HbGPU_ComputeConfig {
	#if HbGPU_Implementation_D3D
	ID3D12PipelineState * d3dPipelineState;
	#endif
} HbGPU_ComputeConfig;

HbBool HbGPU_ComputeConfig_Init(HbGPU_ComputeConfig * config, HbTextU8 const * name, HbGPU_Device * device,
		HbGPU_ShaderReference shader, HbGPU_BindingLayout * bindingLayout);
void HbGPU_ComputeConfig_Destroy(HbGPU_ComputeConfig * config);

/***************
 * Command list
 ***************/

typedef struct HbGPU_CmdList {
	HbGPU_CmdQueue queue;
	#if HbGPU_Implementation_D3D
	ID3D12CommandAllocator * d3dCommandAllocator;
	ID3D12CommandList * d3dSubmissionCommandList;
	ID3D12GraphicsCommandList * d3dGraphicsCommandList;
	#endif
} HbGPU_CmdList;

HbBool HbGPU_CmdList_Init(HbGPU_CmdList * cmdList, HbTextU8 const * name, HbGPU_Device * device, HbGPU_CmdQueue queue);
void HbGPU_CmdList_Destroy(HbGPU_CmdList * cmdList);
void HbGPU_CmdList_BeginRecording(HbGPU_CmdList * cmdList);
void HbGPU_CmdList_Abort(HbGPU_CmdList * cmdList);
void HbGPU_CmdList_Submit(HbGPU_Device * device, HbGPU_CmdList * const * cmdLists, uint32_t cmdListCount);

#endif
