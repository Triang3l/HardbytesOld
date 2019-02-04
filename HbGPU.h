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


#endif
