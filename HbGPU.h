#ifndef HbInclude_HbGPU
#define HbInclude_HbGPU
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
	uint32_t mip : 5;
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

/************************
 * Render target storage
 ************************/

typedef struct HbGPU_RTStore {
	HbGPU_Device * device;
	HbBool isDepth;
	#if HbGPU_Implementation_D3D
	ID3D12DescriptorHeap * d3dHeap;
	D3D12_CPU_DESCRIPTOR_HANDLE d3dHeapStart;
	#endif
} HbGPU_RTStore;

HbBool HbGPU_RTStore_Init(HbGPU_RTStore * store, char const * name, HbGPU_Device * device, HbBool isDepth, uint32_t rtCount);
void HbGPU_RTStore_Destroy(HbGPU_RTStore * store);
void HbGPU_RTStore_SetColor(HbGPU_RTStore * store, uint32_t rtIndex, HbGPU_Image * image, HbGPU_Image_Slice slice, uint32_t zOf3D);
void HbGPU_RTStore_SetDepth(HbGPU_RTStore * store, uint32_t rtIndex, HbGPU_Image * image, HbGPU_Image_Slice slice,
		HbBool readOnlyDepth, HbBool readOnlyStencil);

#endif
