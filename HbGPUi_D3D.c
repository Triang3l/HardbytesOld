#include "HbGPUi_D3D.h"
#if HbGPU_Implementation_D3D
#include "HbBit.h"
#include "HbFeedback.h"
#include <dxgidebug.h>

/****************
 * Object naming
 ****************/

void HbGPUi_D3D_SetObjectName(void * object, HbGPUi_D3D_ObjectNameSetter setter, HbTextU8 const * name) {
	if (name == HbNull || name[0] == '\0') {
		return;
	}
	size_t nameU16Size = HbTextU8_LengthU16Elems(name) + 1;
	HbTextU16 * nameU16 = HbStackAlloc(nameU16Size * sizeof(HbTextU16));
	HbTextU16_FromU8(nameU16, nameU16Size, name);
	setter(object, (WCHAR const *) nameU16);
}

void HbGPUi_D3D_SetSubObjectName(void * object, HbGPUi_D3D_ObjectNameSetter setter,
		HbTextU8 const * parentName, HbTextU8 const * name) {
	if (parentName == HbNull || parentName[0] == '\0') {
		return;
	}
	if (name == HbNull || name[0] == '\0') {
		HbGPUi_D3D_SetObjectName(object, setter, parentName);
		return;
	}
	size_t parentNameU16LengthElems = HbTextU8_LengthU16Elems(parentName);
	size_t fullNameU16Size = parentNameU16LengthElems + 1 + HbTextU8_LengthU16Elems(name) + 1;
	HbTextU16 * fullNameU16 = HbStackAlloc(fullNameU16Size * sizeof(HbTextU16));
	HbTextU16_FromU8(fullNameU16, fullNameU16Size, parentName);
	fullNameU16[parentNameU16LengthElems] = '.';
	HbTextU16_FromU8(fullNameU16 + parentNameU16LengthElems + 1,
			fullNameU16Size - parentNameU16LengthElems - 1, name);
	setter(object, (WCHAR const *) fullNameU16);
}

void HbGPUi_D3D_SetDXGIObjectName(void * object, HbGPUi_D3D_DXGIPrivateDataSetter setter, HbTextU8 const * name) {
	if (name == HbNull || name[0] == '\0') {
		return;
	}
	size_t nameU16LengthElems = HbTextU8_LengthU16Elems(name) + 1;
	HbTextU16 * nameU16 = HbStackAlloc((nameU16LengthElems + 1) * sizeof(HbTextU16));
	HbTextU16_FromU8(nameU16, nameU16LengthElems + 1, name);
	setter(object, &WKPDID_D3DDebugObjectNameW, (UINT) (nameU16LengthElems * sizeof(HbTextU16)), nameU16);
}

void HbGPUi_D3D_SetDXGISubObjectName(void * object, HbGPUi_D3D_DXGIPrivateDataSetter setter,
		HbTextU8 const * parentName, HbTextU8 const * name) {
	if (parentName == HbNull || parentName[0] == '\0') {
		return;
	}
	if (name == HbNull || name[0] == '\0') {
		HbGPUi_D3D_SetDXGIObjectName(object, setter, parentName);
		return;
	}
	size_t parentNameU16LengthElems = HbTextU8_LengthU16Elems(parentName);
	size_t fullNameU16LengthElems = parentNameU16LengthElems + 1 + HbTextU8_LengthU16Elems(name) + 1;
	HbTextU16 * fullNameU16 = HbStackAlloc((fullNameU16LengthElems + 1) * sizeof(HbTextU16));
	HbTextU16_FromU8(fullNameU16, fullNameU16LengthElems + 1, parentName);
	fullNameU16[parentNameU16LengthElems] = '.';
	HbTextU16_FromU8(fullNameU16 + parentNameU16LengthElems + 1,
			(fullNameU16LengthElems + 1) - parentNameU16LengthElems - 1, name);
	setter(object, &WKPDID_D3DDebugObjectNameW, (UINT) (fullNameU16LengthElems * sizeof(HbTextU16)), fullNameU16);
}

/********************
 * Subsystem control
 ********************/

static HbBool HbGPUi_D3D_Debug = HbFalse;
IDXGIFactory2 * HbGPUi_D3D_DXGIFactory = HbNull;

HbBool HbGPU_Init(HbBool debug) {
	if (debug) {
		ID3D12Debug * debugInterface;
		if (SUCCEEDED(D3D12GetDebugInterface(&IID_ID3D12Debug, &debugInterface))) {
			ID3D12Debug_EnableDebugLayer(debugInterface);
			ID3D12Debug_Release(debugInterface);
		} else {
			debug = HbFalse;
		}
	}
	HbGPUi_D3D_Debug = debug;

	if (FAILED(CreateDXGIFactory2(debug ? DXGI_CREATE_FACTORY_DEBUG : 0, &IID_IDXGIFactory2, &HbGPUi_D3D_DXGIFactory))) {
		HbGPUi_D3D_Debug = HbFalse;
		return HbFalse;
	}
	HbGPUi_D3D_SetDXGIObjectName(HbGPUi_D3D_DXGIFactory, HbGPUi_D3D_DXGIFactory->lpVtbl->SetPrivateData, "HbGPUi_D3D_DXGIFactory");

	return HbTrue;
}

void HbGPU_Shutdown() {
	if (HbGPUi_D3D_DXGIFactory != HbNull) {
		IDXGIFactory2_Release(HbGPUi_D3D_DXGIFactory);
		HbGPUi_D3D_DXGIFactory = HbNull;
	}
	if (HbGPUi_D3D_Debug) {
		// Report DXGI and D3D object leaks.
		HMODULE debugLibrary = LoadLibraryA("DXGIDebug.dll");
		if (debugLibrary != HbNull) {
			HRESULT (WINAPI * getDebugInterface)(REFIID riid, void * * debug) =
					(void *) GetProcAddress(debugLibrary, "DXGIGetDebugInterface");
			if (getDebugInterface != HbNull) {
				IDXGIDebug * debugInterface;
				if (SUCCEEDED(getDebugInterface(&IID_IDXGIDebug, &debugInterface))) {
					IDXGIDebug_ReportLiveObjects(debugInterface, DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
					IDXGIDebug_Release(debugInterface);
				}
			}
			FreeLibrary(debugLibrary);
		}
		HbGPUi_D3D_Debug = HbFalse;
	}
}

/*********
 * Device
 *********/

HbBool HbGPU_Device_Init(HbGPU_Device * device, HbTextU8 const * name, uint32_t deviceIndex,
		HbBool needGraphicsQueue, HbBool needCopyQueue) {
	// Create a tag for additional allocations.
	device->d3dMemoryTag = HbMemory_Tag_Create("HbGPU_Device (D3D)");

	// Make the index reference only supported adapters.
	IDXGIAdapter1 * adapter = HbNull;
	uint32_t adapterIndex = 0, supportedAdapterIndex = 0;
	while (IDXGIFactory2_EnumAdapters1(HbGPUi_D3D_DXGIFactory, adapterIndex, &adapter) == S_OK) {
		if (SUCCEEDED(D3D12CreateDevice((IUnknown *) adapter, D3D_FEATURE_LEVEL_11_0, &IID_ID3D12Device, HbNull))) {
			if (supportedAdapterIndex == deviceIndex) {
				break;
			}
			++supportedAdapterIndex;
		}
		++adapterIndex;
		IDXGIAdapter1_Release(adapter);
		adapter = HbNull;
	}
	if (adapter == HbNull) {
		HbMemory_Tag_Destroy(device->d3dMemoryTag, HbFalse);
		return HbFalse;
	}

	// Query adapter interface version 3 for memory information access.
	if (FAILED(IDXGIAdapter1_QueryInterface(adapter, &IID_IDXGIAdapter3, &device->dxgiAdapter))) {
		IDXGIAdapter1_Release(adapter);
		HbMemory_Tag_Destroy(device->d3dMemoryTag, HbFalse);
		return HbFalse;
	}
	IDXGIAdapter1_Release(adapter); // Don't need version 1 anymore.
	HbGPUi_D3D_SetDXGISubObjectName(device->dxgiAdapter, device->dxgiAdapter->lpVtbl->SetPrivateData, name, "dxgiAdapter");

	// Create the D3D device.
	if (FAILED(D3D12CreateDevice((IUnknown *) device->dxgiAdapter, D3D_FEATURE_LEVEL_11_0, &IID_ID3D12Device, &device->d3dDevice))) {
		IDXGIAdapter3_Release(device->dxgiAdapter);
		HbMemory_Tag_Destroy(device->d3dMemoryTag, HbFalse);
		return HbFalse;
	}
	HbGPUi_D3D_SetObjectName(device->d3dDevice, device->d3dDevice->lpVtbl->SetName, name);

	// Create the command queues.
	memset(device->d3dCommandQueues, 0, sizeof(device->d3dCommandQueues));
	D3D12_COMMAND_QUEUE_DESC queueDesc = {
		.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
		.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
		.NodeMask = 0,
	};
	if (needGraphicsQueue) {
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		ID3D12CommandQueue * graphicsQueue;
		if (FAILED(ID3D12Device_CreateCommandQueue(device->d3dDevice, &queueDesc, &IID_ID3D12CommandQueue, &graphicsQueue))) {
			ID3D12Device_Release(device->d3dDevice);
			IDXGIAdapter3_Release(device->dxgiAdapter);
			HbMemory_Tag_Destroy(device->d3dMemoryTag, HbFalse);
			return HbFalse;
		}
		device->d3dCommandQueues[HbGPU_CmdQueue_Graphics] = graphicsQueue;
		HbGPUi_D3D_SetSubObjectName(graphicsQueue, graphicsQueue->lpVtbl->SetName, name, "d3dCommandQueues[HbGPU_CmdQueue_Graphics]");
	}
	if (needCopyQueue) {
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
		ID3D12CommandQueue * copyQueue;
		if (FAILED(ID3D12Device_CreateCommandQueue(device->d3dDevice, &queueDesc, &IID_ID3D12CommandQueue, &copyQueue))) {
			if (needGraphicsQueue) {
				ID3D12CommandQueue_Release(device->d3dCommandQueues[HbGPU_CmdQueue_Graphics]);
			}
			ID3D12Device_Release(device->d3dDevice);
			IDXGIAdapter3_Release(device->dxgiAdapter);
			HbMemory_Tag_Destroy(device->d3dMemoryTag, HbFalse);
			return HbFalse;
		}
		device->d3dCommandQueues[HbGPU_CmdQueue_Copy] = copyQueue;
		HbGPUi_D3D_SetSubObjectName(copyQueue, copyQueue->lpVtbl->SetName, name, "d3dCommandQueues[HbGPU_CmdQueue_Copy]");
	}

	// Get descriptor sizes.
	device->d3dViewDescriptorSize = ID3D12Device_GetDescriptorHandleIncrementSize(device->d3dDevice, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	device->d3dSamplerDescriptorSize = ID3D12Device_GetDescriptorHandleIncrementSize(device->d3dDevice, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
	device->d3dRTVDescriptorSize = ID3D12Device_GetDescriptorHandleIncrementSize(device->d3dDevice, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	device->d3dDSVDescriptorSize = ID3D12Device_GetDescriptorHandleIncrementSize(device->d3dDevice, D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	return HbTrue;
}

void HbGPU_Device_Shutdown(HbGPU_Device * device) {
	for (uint32_t queueIndex = 0; queueIndex < HbGPU_CmdQueue_QueueCount; ++queueIndex) {
		ID3D12CommandQueue * queue = device->d3dCommandQueues[queueIndex];
		if (queue != HbNull) {
			ID3D12CommandQueue_Release(queue);
		}
	}
	ID3D12Device_Release(device->d3dDevice);
	IDXGIAdapter3_Release(device->dxgiAdapter);
	HbMemory_Tag_Destroy(device->d3dMemoryTag, HbFalse);
}

/********
 * Fence
 ********/

HbBool HbGPU_Fence_Init(HbGPU_Fence * fence, HbTextU8 const * name, HbGPU_Device * device, HbGPU_CmdQueue queue) {
	fence->device = device;
	fence->queue = queue;
	fence->d3dAwaitedValue = 0;
	fence->d3dCompletionEvent = CreateEvent(HbNull, FALSE, FALSE, HbNull);
	if (fence->d3dCompletionEvent == HbNull) {
		return HbFalse;
	}
	if (FAILED(ID3D12Device_CreateFence(device->d3dDevice, fence->d3dAwaitedValue, D3D12_FENCE_FLAG_NONE,
			&IID_ID3D12Fence, &fence->d3dFence))) {
		CloseHandle(fence->d3dCompletionEvent);
		return HbFalse;
	}
	HbGPUi_D3D_SetObjectName(fence->d3dFence, fence->d3dFence->lpVtbl->SetName, name);
	return HbTrue;
}

void HbGPU_Fence_Destroy(HbGPU_Fence * fence) {
	ID3D12Fence_Release(fence->d3dFence);
	CloseHandle(fence->d3dCompletionEvent);
}

void HbGPU_Fence_Enqueue(HbGPU_Fence * fence) {
	ID3D12CommandQueue * queue = fence->device->d3dCommandQueues[fence->queue];
	ID3D12CommandQueue_Signal(queue, fence->d3dFence, ++fence->d3dAwaitedValue);
}

HbBool HbGPU_Fence_IsCrossed(HbGPU_Fence * fence) {
	return ID3D12Fence_GetCompletedValue(fence->d3dFence) >= fence->d3dAwaitedValue;
}

void HbGPU_Fence_Await(HbGPU_Fence * fence) {
	if (ID3D12Fence_GetCompletedValue(fence->d3dFence) >= fence->d3dAwaitedValue) {
		return;
	}
	ID3D12Fence_SetEventOnCompletion(fence->d3dFence, fence->d3dAwaitedValue, fence->d3dCompletionEvent);
	WaitForSingleObject(fence->d3dCompletionEvent, INFINITE);
}

/*********
 * Buffer
 *********/

D3D12_RESOURCE_STATES HbGPUi_D3D_Buffer_Usage_ToStates(HbGPU_Buffer_Usage usage) {
	switch (usage) {
	case HbGPU_Buffer_Usage_ShaderEdit:
		return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	case HbGPU_Buffer_Usage_CopyTarget:
		return D3D12_RESOURCE_STATE_COPY_DEST;
	case HbGPU_Buffer_Usage_CrossQueue:
		return D3D12_RESOURCE_STATE_COMMON;
	case HbGPU_Buffer_Usage_CPUToGPU:
		return D3D12_RESOURCE_STATE_GENERIC_READ;
	default:
		break;
	}
	D3D12_RESOURCE_STATES states = (D3D12_RESOURCE_STATES) 0;
	if (usage & (HbGPU_Buffer_Usage_Read_Vertexes | HbGPU_Buffer_Usage_Read_Constants)) {
		states |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
	}
	if (usage & HbGPU_Buffer_Usage_Read_Indexes) {
		states |= D3D12_RESOURCE_STATE_INDEX_BUFFER;
	}
	if (usage & HbGPU_Buffer_Usage_Read_ResourceNonPS) {
		states |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
	}
	if (usage & HbGPU_Buffer_Usage_Read_ResourcePS) {
		states |= D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	}
	if (usage & HbGPU_Buffer_Usage_Read_CopySource) {
		states |= D3D12_RESOURCE_STATE_COPY_SOURCE;
	}
	return states;
}

HbBool HbGPU_Buffer_Init(HbGPU_Buffer * buffer, HbTextU8 const * name, HbGPU_Device * device,
		HbGPU_Buffer_Access access, uint32_t size, HbBool shaderEditable, HbGPU_Buffer_Usage initialUsage) {
	if (size == 0) {
		return HbFalse;
	}
	buffer->access = access;
	buffer->size = size;
	D3D12_HEAP_PROPERTIES heapProperties = { 0 };
	switch (access) {
	case HbGPU_Buffer_Access_GPU:
		heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
		break;
	case HbGPU_Buffer_Access_CPUToGPU:
		heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
		initialUsage = HbGPU_Buffer_Usage_CPUToGPU;
		break;
	case HbGPU_Buffer_Access_GPUToCPU:
		heapProperties.Type = D3D12_HEAP_TYPE_READBACK;
		break;
	}
	D3D12_RESOURCE_DESC resourceDesc = {
		.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
		.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
		.Width = size,
		.Height = 1,
		.DepthOrArraySize = 1,
		.MipLevels = 1,
		.Format = DXGI_FORMAT_UNKNOWN,
		.SampleDesc.Count = 1,
		.SampleDesc.Quality = 0,
		.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
		.Flags = shaderEditable ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE,
	};
	if (FAILED(ID3D12Device_CreateCommittedResource(device->d3dDevice, &heapProperties, D3D12_HEAP_FLAG_NONE,
			&resourceDesc, HbGPUi_D3D_Buffer_Usage_ToStates(initialUsage), HbNull, &IID_ID3D12Resource, &buffer->d3dResource))) {
		return HbFalse;
	}
	HbGPUi_D3D_SetObjectName(buffer->d3dResource, buffer->d3dResource->lpVtbl->SetName, name);
	buffer->d3dGPUAddress = ID3D12Resource_GetGPUVirtualAddress(buffer->d3dResource);
	return HbTrue;
}

void HbGPU_Buffer_Destroy(HbGPU_Buffer * buffer) {
	ID3D12Resource_Release(buffer->d3dResource);
}

void * HbGPU_Buffer_Map(HbGPU_Buffer * buffer, uint32_t readStart, uint32_t readLength) {
	if (buffer->access != HbGPU_Buffer_Access_CPUToGPU) {
		readStart = readLength = 0;
	}
	D3D12_RANGE readRange = { .Begin = readStart, .End = readStart + readLength };
	void * mapping;
	if (FAILED(ID3D12Resource_Map(buffer->d3dResource, 0, &readRange, &mapping))) {
		return HbNull;
	}
	return mapping;
}

void HbGPU_Buffer_Unmap(HbGPU_Buffer * buffer, uint32_t writeStart, uint32_t writeLength) {
	if (buffer->access != HbGPU_Buffer_Access_GPUToCPU) {
		writeStart = writeLength = 0;
	}
	D3D12_RANGE writeRange = { .Begin = writeStart, .End = writeStart + writeLength };
	ID3D12Resource_Unmap(buffer->d3dResource, 0, &writeRange);
}

/********
 * Image
 ********/

DXGI_FORMAT HbGPUi_D3D_Image_Format_ToTyped(HbGPU_Image_Format format) {
	if (format >= HbGPU_Image_Format_FormatCount) {
		return DXGI_FORMAT_UNKNOWN;
	}
	static DXGI_FORMAT const dxgiFormats[] = {
		[HbGPU_Image_Format_4_4_4_4_BGRA_UNorm] = DXGI_FORMAT_B4G4R4A4_UNORM,
		[HbGPU_Image_Format_5_5_5_1_BGRA_UNorm] = DXGI_FORMAT_B5G5R5A1_UNORM,
		[HbGPU_Image_Format_5_6_5_BGR_UNorm] = DXGI_FORMAT_B5G6R5_UNORM,
		[HbGPU_Image_Format_8_R_UNorm] = DXGI_FORMAT_R8_UNORM,
		[HbGPU_Image_Format_8_R_UInt] = DXGI_FORMAT_R8_UINT,
		[HbGPU_Image_Format_8_R_SNorm] = DXGI_FORMAT_R8_SNORM,
		[HbGPU_Image_Format_8_R_SInt] = DXGI_FORMAT_R8_SINT,
		[HbGPU_Image_Format_8_8_RG_UNorm] = DXGI_FORMAT_R8G8_UNORM,
		[HbGPU_Image_Format_8_8_RG_UInt] = DXGI_FORMAT_R8G8_UINT,
		[HbGPU_Image_Format_8_8_RG_SNorm] = DXGI_FORMAT_R8G8_SNORM,
		[HbGPU_Image_Format_8_8_RG_SInt] = DXGI_FORMAT_R8G8_SINT,
		[HbGPU_Image_Format_8_8_8_8_RGBA_UNorm] = DXGI_FORMAT_R8G8B8A8_UNORM,
		[HbGPU_Image_Format_8_8_8_8_RGBA_sRGB] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
		[HbGPU_Image_Format_8_8_8_8_RGBA_UInt] = DXGI_FORMAT_R8G8B8A8_UINT,
		[HbGPU_Image_Format_8_8_8_8_RGBA_SNorm] = DXGI_FORMAT_R8G8B8A8_SNORM,
		[HbGPU_Image_Format_8_8_8_8_RGBA_SInt] = DXGI_FORMAT_R8G8B8A8_UINT,
		[HbGPU_Image_Format_8_8_8_8_BGRA_UNorm] = DXGI_FORMAT_B8G8R8A8_UNORM,
		[HbGPU_Image_Format_8_8_8_8_BGRA_sRGB] = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,
		[HbGPU_Image_Format_10_10_10_2_RGBA_UNorm] = DXGI_FORMAT_R10G10B10A2_UNORM,
		[HbGPU_Image_Format_10_10_10_2_RGBA_UInt] = DXGI_FORMAT_R10G10B10A2_UINT,
		[HbGPU_Image_Format_11_11_10_RGB_Float] = DXGI_FORMAT_R11G11B10_FLOAT,
		[HbGPU_Image_Format_16_R_UNorm] = DXGI_FORMAT_R16_UNORM,
		[HbGPU_Image_Format_16_R_UInt] = DXGI_FORMAT_R16_UINT,
		[HbGPU_Image_Format_16_R_SNorm] = DXGI_FORMAT_R16_SNORM,
		[HbGPU_Image_Format_16_R_SInt] = DXGI_FORMAT_R16_SINT,
		[HbGPU_Image_Format_16_R_Float] = DXGI_FORMAT_R16_FLOAT,
		[HbGPU_Image_Format_16_16_RG_UNorm] = DXGI_FORMAT_R16G16_UNORM,
		[HbGPU_Image_Format_16_16_RG_UInt] = DXGI_FORMAT_R16G16_UINT,
		[HbGPU_Image_Format_16_16_RG_SNorm] = DXGI_FORMAT_R16G16_SNORM,
		[HbGPU_Image_Format_16_16_RG_SInt] = DXGI_FORMAT_R16G16_SINT,
		[HbGPU_Image_Format_16_16_RG_Float] = DXGI_FORMAT_R16G16_FLOAT,
		[HbGPU_Image_Format_16_16_16_16_RGBA_UNorm] = DXGI_FORMAT_R16G16B16A16_UNORM,
		[HbGPU_Image_Format_16_16_16_16_RGBA_UInt] = DXGI_FORMAT_R16G16B16A16_UINT,
		[HbGPU_Image_Format_16_16_16_16_RGBA_SNorm] = DXGI_FORMAT_R16G16B16A16_SNORM,
		[HbGPU_Image_Format_16_16_16_16_RGBA_SInt] = DXGI_FORMAT_R16G16B16A16_SINT,
		[HbGPU_Image_Format_16_16_16_16_RGBA_Float] = DXGI_FORMAT_R16G16B16A16_FLOAT,
		[HbGPU_Image_Format_32_R_UInt] = DXGI_FORMAT_R32_UINT,
		[HbGPU_Image_Format_32_R_SInt] = DXGI_FORMAT_R32_SINT,
		[HbGPU_Image_Format_32_R_Float] = DXGI_FORMAT_R32_FLOAT,
		[HbGPU_Image_Format_32_32_RG_UInt] = DXGI_FORMAT_R32G32_UINT,
		[HbGPU_Image_Format_32_32_RG_SInt] = DXGI_FORMAT_R32G32_SINT,
		[HbGPU_Image_Format_32_32_RG_Float] = DXGI_FORMAT_R32G32_FLOAT,
		[HbGPU_Image_Format_32_32_32_32_RGBA_UInt] = DXGI_FORMAT_R32G32B32A32_UINT,
		[HbGPU_Image_Format_32_32_32_32_RGBA_SInt] = DXGI_FORMAT_R32G32B32A32_SINT,
		[HbGPU_Image_Format_32_32_32_32_RGBA_Float] = DXGI_FORMAT_R32G32B32A32_FLOAT,
		[HbGPU_Image_Format_S3TC_A1_UNorm] = DXGI_FORMAT_BC1_UNORM,
		[HbGPU_Image_Format_S3TC_A1_sRGB] = DXGI_FORMAT_BC1_UNORM_SRGB,
		[HbGPU_Image_Format_S3TC_A4_UNorm] = DXGI_FORMAT_BC2_UNORM,
		[HbGPU_Image_Format_S3TC_A4_sRGB] = DXGI_FORMAT_BC2_UNORM_SRGB,
		[HbGPU_Image_Format_S3TC_A8_UNorm] = DXGI_FORMAT_BC3_UNORM,
		[HbGPU_Image_Format_S3TC_A8_sRGB] = DXGI_FORMAT_BC3_UNORM_SRGB,
		[HbGPU_Image_Format_3Dc_R_UNorm] = DXGI_FORMAT_BC4_UNORM,
		[HbGPU_Image_Format_3Dc_R_SNorm] = DXGI_FORMAT_BC4_SNORM,
		[HbGPU_Image_Format_3Dc_RG_UNorm] = DXGI_FORMAT_BC5_UNORM,
		[HbGPU_Image_Format_3Dc_RG_SNorm] = DXGI_FORMAT_BC5_SNORM,
		[HbGPU_Image_Format_D32] = DXGI_FORMAT_D32_FLOAT,
		[HbGPU_Image_Format_D32_S8] = DXGI_FORMAT_D32_FLOAT_S8X24_UINT,
	};
	HbFeedback_StaticAssert(HbArrayLength(dxgiFormats) == HbGPU_Image_Format_FormatCount,
			"All known image formats must be mapped in HbGPUi_D3D_Image_Format_ToTyped.");
	return dxgiFormats[(uint32_t) format];
}

DXGI_FORMAT HbGPUi_D3D_Image_Format_ToTypeless(HbGPU_Image_Format format) {
	if (format >= HbGPU_Image_Format_FormatCount) {
		return DXGI_FORMAT_UNKNOWN;
	}
	static DXGI_FORMAT const dxgiFormats[] = {
		[HbGPU_Image_Format_8_R_UNorm] = DXGI_FORMAT_R8_TYPELESS,
		[HbGPU_Image_Format_8_R_UInt] = DXGI_FORMAT_R8_TYPELESS,
		[HbGPU_Image_Format_8_R_SNorm] = DXGI_FORMAT_R8_TYPELESS,
		[HbGPU_Image_Format_8_R_SInt] = DXGI_FORMAT_R8_TYPELESS,
		[HbGPU_Image_Format_8_8_RG_UNorm] = DXGI_FORMAT_R8G8_TYPELESS,
		[HbGPU_Image_Format_8_8_RG_UInt] = DXGI_FORMAT_R8G8_TYPELESS,
		[HbGPU_Image_Format_8_8_RG_SNorm] = DXGI_FORMAT_R8G8_TYPELESS,
		[HbGPU_Image_Format_8_8_RG_SInt] = DXGI_FORMAT_R8G8_TYPELESS,
		[HbGPU_Image_Format_8_8_8_8_RGBA_UNorm] = DXGI_FORMAT_R8G8B8A8_TYPELESS,
		[HbGPU_Image_Format_8_8_8_8_RGBA_sRGB] = DXGI_FORMAT_R8G8B8A8_TYPELESS,
		[HbGPU_Image_Format_8_8_8_8_RGBA_UInt] = DXGI_FORMAT_R8G8B8A8_TYPELESS,
		[HbGPU_Image_Format_8_8_8_8_RGBA_SNorm] = DXGI_FORMAT_R8G8B8A8_TYPELESS,
		[HbGPU_Image_Format_8_8_8_8_RGBA_SInt] = DXGI_FORMAT_R8G8B8A8_TYPELESS,
		[HbGPU_Image_Format_8_8_8_8_BGRA_UNorm] = DXGI_FORMAT_B8G8R8A8_TYPELESS,
		[HbGPU_Image_Format_8_8_8_8_BGRA_sRGB] = DXGI_FORMAT_B8G8R8A8_TYPELESS,
		[HbGPU_Image_Format_10_10_10_2_RGBA_UNorm] = DXGI_FORMAT_R10G10B10A2_TYPELESS,
		[HbGPU_Image_Format_10_10_10_2_RGBA_UInt] = DXGI_FORMAT_R10G10B10A2_TYPELESS,
		[HbGPU_Image_Format_16_R_UNorm] = DXGI_FORMAT_R16_TYPELESS,
		[HbGPU_Image_Format_16_R_UInt] = DXGI_FORMAT_R16_TYPELESS,
		[HbGPU_Image_Format_16_R_SNorm] = DXGI_FORMAT_R16_TYPELESS,
		[HbGPU_Image_Format_16_R_SInt] = DXGI_FORMAT_R16_TYPELESS,
		[HbGPU_Image_Format_16_R_Float] = DXGI_FORMAT_R16_TYPELESS,
		[HbGPU_Image_Format_16_16_RG_UNorm] = DXGI_FORMAT_R16G16_TYPELESS,
		[HbGPU_Image_Format_16_16_RG_UInt] = DXGI_FORMAT_R16G16_TYPELESS,
		[HbGPU_Image_Format_16_16_RG_SNorm] = DXGI_FORMAT_R16G16_TYPELESS,
		[HbGPU_Image_Format_16_16_RG_SInt] = DXGI_FORMAT_R16G16_TYPELESS,
		[HbGPU_Image_Format_16_16_RG_Float] = DXGI_FORMAT_R16G16_TYPELESS,
		[HbGPU_Image_Format_16_16_16_16_RGBA_UNorm] = DXGI_FORMAT_R16G16B16A16_TYPELESS,
		[HbGPU_Image_Format_16_16_16_16_RGBA_UInt] = DXGI_FORMAT_R16G16B16A16_TYPELESS,
		[HbGPU_Image_Format_16_16_16_16_RGBA_SNorm] = DXGI_FORMAT_R16G16B16A16_TYPELESS,
		[HbGPU_Image_Format_16_16_16_16_RGBA_SInt] = DXGI_FORMAT_R16G16B16A16_TYPELESS,
		[HbGPU_Image_Format_16_16_16_16_RGBA_Float] = DXGI_FORMAT_R16G16B16A16_TYPELESS,
		[HbGPU_Image_Format_32_R_UInt] = DXGI_FORMAT_R32_TYPELESS,
		[HbGPU_Image_Format_32_R_SInt] = DXGI_FORMAT_R32_TYPELESS,
		[HbGPU_Image_Format_32_R_Float] = DXGI_FORMAT_R32_TYPELESS,
		[HbGPU_Image_Format_32_32_RG_UInt] = DXGI_FORMAT_R32G32_TYPELESS,
		[HbGPU_Image_Format_32_32_RG_SInt] = DXGI_FORMAT_R32G32_TYPELESS,
		[HbGPU_Image_Format_32_32_RG_Float] = DXGI_FORMAT_R32G32_TYPELESS,
		[HbGPU_Image_Format_32_32_32_32_RGBA_UInt] = DXGI_FORMAT_R32G32B32A32_TYPELESS,
		[HbGPU_Image_Format_32_32_32_32_RGBA_SInt] = DXGI_FORMAT_R32G32B32A32_TYPELESS,
		[HbGPU_Image_Format_32_32_32_32_RGBA_Float] = DXGI_FORMAT_R32G32B32A32_TYPELESS,
		[HbGPU_Image_Format_S3TC_A1_UNorm] = DXGI_FORMAT_BC1_TYPELESS,
		[HbGPU_Image_Format_S3TC_A1_sRGB] = DXGI_FORMAT_BC1_TYPELESS,
		[HbGPU_Image_Format_S3TC_A4_UNorm] = DXGI_FORMAT_BC2_TYPELESS,
		[HbGPU_Image_Format_S3TC_A4_sRGB] = DXGI_FORMAT_BC2_TYPELESS,
		[HbGPU_Image_Format_S3TC_A8_UNorm] = DXGI_FORMAT_BC3_TYPELESS,
		[HbGPU_Image_Format_S3TC_A8_sRGB] = DXGI_FORMAT_BC3_TYPELESS,
		[HbGPU_Image_Format_3Dc_R_UNorm] = DXGI_FORMAT_BC4_TYPELESS,
		[HbGPU_Image_Format_3Dc_R_SNorm] = DXGI_FORMAT_BC4_TYPELESS,
		[HbGPU_Image_Format_3Dc_RG_UNorm] = DXGI_FORMAT_BC5_TYPELESS,
		[HbGPU_Image_Format_3Dc_RG_SNorm] = DXGI_FORMAT_BC5_TYPELESS,
		[HbGPU_Image_Format_D32] = DXGI_FORMAT_R32_TYPELESS,
		[HbGPU_Image_Format_D32_S8] = DXGI_FORMAT_R32G8X24_TYPELESS,
	};
	DXGI_FORMAT dxgiFormat = dxgiFormats[(uint32_t) format];
	if (dxgiFormat == DXGI_FORMAT_UNKNOWN) {
		// Only a single version of the format.
		return HbGPUi_D3D_Image_Format_ToTyped(format);
	}
	return dxgiFormat;
}

DXGI_FORMAT HbGPUi_D3D_Image_Format_ToCopy(HbGPU_Image_Format format, HbBool stencil) {
	if (stencil) {
		return HbGPU_Image_Format_HasStencil(format) ? DXGI_FORMAT_R8_TYPELESS : DXGI_FORMAT_UNKNOWN;
	}
	if (format == HbGPU_Image_Format_D32_S8) {
		return DXGI_FORMAT_R32_TYPELESS;
	}
	return HbGPUi_D3D_Image_Format_ToTypeless(format);
}

DXGI_FORMAT HbGPUi_D3D_Image_Format_ToTexture(HbGPU_Image_Format format, HbBool stencil) {
	switch (format) {
	case HbGPU_Image_Format_D32:
		return DXGI_FORMAT_R32_FLOAT;
	case HbGPU_Image_Format_D32_S8:
		return stencil ? DXGI_FORMAT_X32_TYPELESS_G8X24_UINT : DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
	}
	return HbGPUi_D3D_Image_Format_ToTyped(format);
}

void HbGPUi_D3D_Image_Info_ToResourceDesc(D3D12_RESOURCE_DESC * desc, HbGPU_Image_Info const * info) {
	switch (info->dimensions) {
	case HbGPU_Image_Dimensions_1D:
	case HbGPU_Image_Dimensions_1DArray:
		desc->Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
		break;
	case HbGPU_Image_Dimensions_3D:
		desc->Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
		break;
	default:
		desc->Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		break;
	}
	desc->Alignment = (info->samplesLog2 > 0) ? D3D12_DEFAULT_MSAA_RESOURCE_PLACEMENT_ALIGNMENT :
			D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
	desc->Width = info->width;
	desc->Height = info->height;
	desc->DepthOrArraySize = info->depthOrLayers;
	if (HbGPU_Image_Dimensions_AreCube(info->dimensions)) {
		desc->DepthOrArraySize *= 6;
	}
	desc->MipLevels = info->mips;
	HbBool formatIsDepth = HbGPU_Image_Format_IsDepth(info->format);
	if (formatIsDepth && !(info->usageOptions & HbGPU_Image_UsageOptions_DepthTestOnly)) {
		desc->Format = HbGPUi_D3D_Image_Format_ToTypeless(info->format);
	} else {
		desc->Format = HbGPUi_D3D_Image_Format_ToTyped(info->format);
	}
	desc->SampleDesc.Count = 1 << info->samplesLog2;
	desc->SampleDesc.Quality = 0;
	desc->Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	desc->Flags = formatIsDepth ? D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL : D3D12_RESOURCE_FLAG_NONE;
	if (info->usageOptions & HbGPU_Image_UsageOptions_ShaderEditable) {
		desc->Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	}
	if (info->usageOptions & HbGPU_Image_UsageOptions_ColorRenderable) {
		desc->Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	}
	if (info->usageOptions & HbGPU_Image_UsageOptions_DepthTestOnly) {
		desc->Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
	}
}

D3D12_RESOURCE_STATES HbGPUi_D3D_Image_Usage_ToStates(HbGPU_Image_Usage usage) {
	switch (usage) {
	case HbGPU_Image_Usage_ColorRT:
		return D3D12_RESOURCE_STATE_RENDER_TARGET;
	case HbGPU_Image_Usage_DepthTest:
		return D3D12_RESOURCE_STATE_DEPTH_WRITE;
	case HbGPU_Image_Usage_ShaderEdit:
		return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	case HbGPU_Image_Usage_CopyTarget:
		return D3D12_RESOURCE_STATE_COPY_DEST;
	case HbGPU_Image_Usage_ResolveTarget:
		return D3D12_RESOURCE_STATE_RESOLVE_DEST;
	case HbGPU_Image_Usage_Present:
		return D3D12_RESOURCE_STATE_PRESENT;
	case HbGPU_Image_Usage_CrossQueue:
		return D3D12_RESOURCE_STATE_COMMON;
	}
	D3D12_RESOURCE_STATES states = (D3D12_RESOURCE_STATES) 0;
	if (usage & HbGPU_Image_Usage_Read_TextureNonPS) {
		states |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
	}
	if (usage & HbGPU_Image_Usage_Read_TexturePS) {
		states |= D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	}
	if (usage & HbGPU_Image_Usage_Read_DepthReject) {
		states |= D3D12_RESOURCE_STATE_DEPTH_READ;
	}
	if (usage & HbGPU_Image_Usage_Read_CopySource) {
		states |= D3D12_RESOURCE_STATE_COPY_SOURCE;
	}
	if (usage & HbGPU_Image_Usage_Read_ResolveSource) {
		states |= D3D12_RESOURCE_STATE_RESOLVE_SOURCE;
	}
	return states;
}

HbBool HbGPU_Image_InitWithValidInfo(HbGPU_Image * image, HbTextU8 const * name, HbGPU_Device * device,
		HbGPU_Image_Usage initialUsage, HbGPU_Image_ClearValue const * optimalClearValue) {
	D3D12_HEAP_PROPERTIES heapProperties = { 0 };
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	D3D12_RESOURCE_DESC resourceDesc;
	HbGPUi_D3D_Image_Info_ToResourceDesc(&resourceDesc, &image->info);
	D3D12_CLEAR_VALUE d3dOptimizedClearValue;
	if (optimalClearValue != HbNull) {
		// In case the resource description has a typeless format.
		d3dOptimizedClearValue.Format = HbGPUi_D3D_Image_Format_ToTyped(image->info.format);
		if (HbGPU_Image_Format_IsDepth(image->info.format)) {
			d3dOptimizedClearValue.DepthStencil.Depth = optimalClearValue->depthStencil.depth;
			d3dOptimizedClearValue.DepthStencil.Stencil = optimalClearValue->depthStencil.stencil;
		} else {
			d3dOptimizedClearValue.Color[0] = optimalClearValue->color[0];
			d3dOptimizedClearValue.Color[1] = optimalClearValue->color[1];
			d3dOptimizedClearValue.Color[2] = optimalClearValue->color[2];
			d3dOptimizedClearValue.Color[3] = optimalClearValue->color[3];
		}
	}
	if (FAILED(ID3D12Device_CreateCommittedResource(device->d3dDevice, &heapProperties, D3D12_HEAP_FLAG_NONE,
			&resourceDesc, HbGPUi_D3D_Image_Usage_ToStates(initialUsage),
			optimalClearValue != HbNull ? &d3dOptimizedClearValue : HbNull, &IID_ID3D12Resource, &image->d3dResource))) {
		return HbFalse;
	}
	HbGPUi_D3D_SetObjectName(image->d3dResource, image->d3dResource->lpVtbl->SetName, name);
	return HbTrue;
}

void HbGPUi_D3D_Image_WrapSwapChainBuffer(HbGPU_Image * image, HbTextU8 const * name,
		ID3D12Resource * resource, HbGPU_Image_Format format) {
	D3D12_RESOURCE_DESC resourceDesc;
	((HbGPUi_D3D_ID3D12Resource_GetDesc) (resource->lpVtbl->GetDesc))(resource, &resourceDesc);
	image->info.format = format;
	image->info.dimensions = HbGPU_Image_Dimensions_2D;
	image->info.width = (uint32_t) resourceDesc.Width;
	image->info.height = resourceDesc.Height;
	image->info.depthOrLayers = 1;
	image->info.mips = 1;
	image->info.samplesLog2 = (uint32_t) HbBit_HighestOneU32(resourceDesc.SampleDesc.Count);
	image->info.usageOptions = HbGPU_Image_UsageOptions_ColorRenderable;
	image->d3dResource = resource;
}

void HbGPU_Image_Destroy(HbGPU_Image * image) {
	ID3D12Resource_Release(image->d3dResource);
}

/*******************************************************
 * Storage of binding descriptors of buffers and images
 *******************************************************/

HbBool HbGPU_HandleStore_Init(HbGPU_HandleStore * store, HbTextU8 const * name, HbGPU_Device * device, uint32_t handleCount) {
	store->device = device;
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {
		.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		.NumDescriptors = handleCount,
		.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
		.NodeMask = 0,
	};
	if (FAILED(ID3D12Device_CreateDescriptorHeap(device->d3dDevice, &heapDesc, &IID_ID3D12DescriptorHeap, &store->d3dHeap))) {
		return HbFalse;
	}
	HbGPUi_D3D_SetObjectName(store->d3dHeap, store->d3dHeap->lpVtbl->SetName, name);
	((HbGPUi_D3D_ID3D12DescriptorHeap_GetCPUDescriptorHandleForHeapStart)
			store->d3dHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart)(
					store->d3dHeap, &store->d3dHeapCPUStart);
	((HbGPUi_D3D_ID3D12DescriptorHeap_GetGPUDescriptorHandleForHeapStart)
			store->d3dHeap->lpVtbl->GetGPUDescriptorHandleForHeapStart)(
					store->d3dHeap, &store->d3dHeapGPUStart);
	return HbTrue;
}

void HbGPU_HandleStore_Destroy(HbGPU_HandleStore * store) {
	ID3D12DescriptorHeap_Release(store->d3dHeap);
}

void HbGPU_HandleStore_SetConstantBuffer(HbGPU_HandleStore * store, uint32_t index,
		HbGPU_Buffer * buffer, uint32_t offset, uint32_t size) {
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {
		.BufferLocation = buffer->d3dGPUAddress + offset,
		.SizeInBytes = HbAlignU32(size, HbGPU_Buffer_ConstantsAlignment),
	};
	ID3D12Device_CreateConstantBufferView(store->device->d3dDevice, &cbvDesc,
			HbGPUi_D3D_HandleStore_GetCPUHandle(store, index));
}

void HbGPU_HandleStore_SetTexelResourceBuffer(HbGPU_HandleStore * store, uint32_t index,
		HbGPU_Buffer * buffer, HbGPU_Image_Format format, uint32_t offsetInTexels, uint32_t texelCount) {
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {
		.Format = HbGPUi_D3D_Image_Format_ToTexture(format, HbFalse),
		.ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
		.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
		.Buffer.FirstElement = offsetInTexels,
		.Buffer.NumElements = texelCount,
		.Buffer.StructureByteStride = HbGPU_Image_Copy_ElementSize(format, HbFalse),
		.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE,
	};
	ID3D12Device_CreateShaderResourceView(store->device->d3dDevice, buffer->d3dResource, &srvDesc,
			HbGPUi_D3D_HandleStore_GetCPUHandle(store, index));
}

void HbGPU_HandleStore_SetStructResourceBuffer(HbGPU_HandleStore * store, uint32_t index,
		HbGPU_Buffer * buffer, uint32_t structSize, uint32_t offsetInStructs, uint32_t structCount) {
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {
		.Format = DXGI_FORMAT_UNKNOWN,
		.ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
		.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
		.Buffer.FirstElement = offsetInStructs,
		.Buffer.NumElements = structCount,
		.Buffer.StructureByteStride = structSize,
		.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE,
	};
	ID3D12Device_CreateShaderResourceView(store->device->d3dDevice, buffer->d3dResource, &srvDesc,
			HbGPUi_D3D_HandleStore_GetCPUHandle(store, index));
}

void HbGPU_HandleStore_SetEditBuffer(HbGPU_HandleStore * store, uint32_t index,
		HbGPU_Buffer * buffer, HbGPU_Image_Format format, uint32_t offsetInTexels, uint32_t texelCount) {
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {
		.Format = HbGPUi_D3D_Image_Format_ToTexture(format, HbFalse),
		.ViewDimension = D3D12_UAV_DIMENSION_BUFFER,
		.Buffer.FirstElement = offsetInTexels,
		.Buffer.NumElements = texelCount,
		.Buffer.StructureByteStride = HbGPU_Image_Copy_ElementSize(format, HbFalse),
		.Buffer.CounterOffsetInBytes = 0,
		.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE,
	};
	ID3D12Device_CreateUnorderedAccessView(store->device->d3dDevice, buffer->d3dResource, HbNull, &uavDesc,
			HbGPUi_D3D_HandleStore_GetCPUHandle(store, index));
}

void HbGPU_HandleStore_SetTexture(HbGPU_HandleStore * store, uint32_t index, HbGPU_Image * image, HbBool stencil) {
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = HbGPUi_D3D_Image_Format_ToTexture(image->info.format, stencil);
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	switch (image->info.dimensions) {
	case HbGPU_Image_Dimensions_1D:
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
		srvDesc.Texture1D.MostDetailedMip = 0;
		srvDesc.Texture1D.MipLevels = image->info.mips;
		srvDesc.Texture1D.ResourceMinLODClamp = 0.0f;
		break;
	case HbGPU_Image_Dimensions_1DArray:
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
		srvDesc.Texture1DArray.MostDetailedMip = 0;
		srvDesc.Texture1DArray.MipLevels = image->info.mips;
		srvDesc.Texture1DArray.FirstArraySlice = 0;
		srvDesc.Texture1DArray.ArraySize = image->info.depthOrLayers;
		srvDesc.Texture1DArray.ResourceMinLODClamp = 0.0f;
		break;
	case HbGPU_Image_Dimensions_2D:
		if (image->info.samplesLog2 > 0) {
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
		} else {
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.MipLevels = image->info.mips;
			srvDesc.Texture2D.PlaneSlice = stencil ? 1 : 0;
			srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		}
		break;
	case HbGPU_Image_Dimensions_2DArray:
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
		srvDesc.Texture2DArray.MostDetailedMip = 0;
		srvDesc.Texture2DArray.MipLevels = image->info.mips;
		srvDesc.Texture2DArray.FirstArraySlice = 0;
		srvDesc.Texture2DArray.ArraySize = image->info.depthOrLayers;
		srvDesc.Texture2DArray.PlaneSlice = stencil ? 1 : 0;
		srvDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
		break;
	case HbGPU_Image_Dimensions_Cube:
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.TextureCubeArray.MostDetailedMip = 0;
		srvDesc.TextureCubeArray.MipLevels = image->info.mips;
		srvDesc.TextureCubeArray.First2DArrayFace = 0;
		srvDesc.TextureCubeArray.NumCubes = image->info.depthOrLayers;
		srvDesc.TextureCubeArray.ResourceMinLODClamp = 0.0f;
		break;
	case HbGPU_Image_Dimensions_CubeArray:
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
		srvDesc.TextureCube.MostDetailedMip = 0;
		srvDesc.TextureCube.MipLevels = image->info.mips;
		srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
		break;
	case HbGPU_Image_Dimensions_3D:
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
		srvDesc.Texture3D.MostDetailedMip = 0;
		srvDesc.Texture3D.MipLevels = image->info.mips;
		srvDesc.Texture3D.ResourceMinLODClamp = 0.0f;
		break;
	default:
		return;
	}
	ID3D12Device_CreateShaderResourceView(store->device->d3dDevice, image->d3dResource, &srvDesc,
			HbGPUi_D3D_HandleStore_GetCPUHandle(store, index));
}

void HbGPU_HandleStore_SetNullTexture(HbGPU_HandleStore * store, uint32_t index, HbGPU_Image_Dimensions dimensions, HbBool multisample) {
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	switch (dimensions) {
	case HbGPU_Image_Dimensions_1D:
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
		srvDesc.Texture1D.MostDetailedMip = 0;
		srvDesc.Texture1D.MipLevels = 1;
		srvDesc.Texture1D.ResourceMinLODClamp = 0.0f;
		break;
	case HbGPU_Image_Dimensions_1DArray:
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
		srvDesc.Texture1DArray.MostDetailedMip = 0;
		srvDesc.Texture1DArray.MipLevels = 1;
		srvDesc.Texture1DArray.FirstArraySlice = 0;
		srvDesc.Texture1DArray.ArraySize = 1;
		srvDesc.Texture1DArray.ResourceMinLODClamp = 0.0f;
		break;
	case HbGPU_Image_Dimensions_2D:
		if (multisample) {
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
		} else {
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.MipLevels = 1;
			srvDesc.Texture2D.PlaneSlice = 0;
			srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		}
		break;
	case HbGPU_Image_Dimensions_2DArray:
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
		srvDesc.Texture2DArray.MostDetailedMip = 0;
		srvDesc.Texture2DArray.MipLevels = 1;
		srvDesc.Texture2DArray.FirstArraySlice = 0;
		srvDesc.Texture2DArray.ArraySize = 1;
		srvDesc.Texture2DArray.PlaneSlice = 0;
		srvDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
		break;
	case HbGPU_Image_Dimensions_Cube:
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.TextureCubeArray.MostDetailedMip = 0;
		srvDesc.TextureCubeArray.MipLevels = 1;
		srvDesc.TextureCubeArray.First2DArrayFace = 0;
		srvDesc.TextureCubeArray.NumCubes = 1;
		srvDesc.TextureCubeArray.ResourceMinLODClamp = 0.0f;
		break;
	case HbGPU_Image_Dimensions_CubeArray:
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
		srvDesc.TextureCube.MostDetailedMip = 0;
		srvDesc.TextureCube.MipLevels = 1;
		srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
		break;
	case HbGPU_Image_Dimensions_3D:
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
		srvDesc.Texture3D.MostDetailedMip = 0;
		srvDesc.Texture3D.MipLevels = 1;
		srvDesc.Texture3D.ResourceMinLODClamp = 0.0f;
		break;
	default:
		return;
	}
	ID3D12Device_CreateShaderResourceView(store->device->d3dDevice, HbNull, &srvDesc,
			HbGPUi_D3D_HandleStore_GetCPUHandle(store, index));
}

void HbGPU_HandleStore_SetEditImage(HbGPU_HandleStore * store, uint32_t index, HbGPU_Image * image, uint32_t mip) {
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	uavDesc.Format = HbGPUi_D3D_Image_Format_ToTexture(image->info.format, HbFalse);
	switch (image->info.dimensions) {
	case HbGPU_Image_Dimensions_1D:
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
		uavDesc.Texture1D.MipSlice = mip;
		break;
	case HbGPU_Image_Dimensions_1DArray:
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
		uavDesc.Texture1DArray.MipSlice = mip;
		uavDesc.Texture1DArray.FirstArraySlice = 0;
		uavDesc.Texture1DArray.ArraySize = image->info.depthOrLayers;
		break;
	case HbGPU_Image_Dimensions_2D:
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.MipSlice = mip;
		uavDesc.Texture2D.PlaneSlice = 0;
		break;
	case HbGPU_Image_Dimensions_2DArray:
	case HbGPU_Image_Dimensions_Cube:
	case HbGPU_Image_Dimensions_CubeArray:
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
		uavDesc.Texture2DArray.MipSlice = mip;
		uavDesc.Texture2DArray.PlaneSlice = 0;
		uavDesc.Texture2DArray.FirstArraySlice = 0;
		uavDesc.Texture2DArray.ArraySize = image->info.depthOrLayers;
		if (HbGPU_Image_Dimensions_AreCube(image->info.dimensions)) {
			uavDesc.Texture2DArray.ArraySize *= 6;
		}
		break;
	case HbGPU_Image_Dimensions_3D:
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
		uavDesc.Texture3D.MipSlice = mip;
		uavDesc.Texture3D.FirstWSlice = 0;
		uavDesc.Texture3D.WSize = image->info.depthOrLayers;
		break;
	}
	ID3D12Device_CreateUnorderedAccessView(store->device->d3dDevice, image->d3dResource, HbNull, &uavDesc,
			HbGPUi_D3D_HandleStore_GetCPUHandle(store, index));
}

/**********
 * Sampler
 **********/

HbBool HbGPU_SamplerStore_Init(HbGPU_SamplerStore * store, HbTextU8 const * name, HbGPU_Device * device, uint32_t samplerCount) {
	store->device = device;
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {
		.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
		.NumDescriptors = samplerCount,
		.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
		.NodeMask = 0,
	};
	if (FAILED(ID3D12Device_CreateDescriptorHeap(device->d3dDevice, &heapDesc, &IID_ID3D12DescriptorHeap, &store->d3dHeap))) {
		return HbFalse;
	}
	HbGPUi_D3D_SetObjectName(store->d3dHeap, store->d3dHeap->lpVtbl->SetName, name);
	((HbGPUi_D3D_ID3D12DescriptorHeap_GetCPUDescriptorHandleForHeapStart)
			store->d3dHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart)(
					store->d3dHeap, &store->d3dHeapCPUStart);
	((HbGPUi_D3D_ID3D12DescriptorHeap_GetGPUDescriptorHandleForHeapStart)
			store->d3dHeap->lpVtbl->GetGPUDescriptorHandleForHeapStart)(
					store->d3dHeap, &store->d3dHeapGPUStart);
	return HbTrue;
}

void HbGPU_SamplerStore_Destroy(HbGPU_SamplerStore * store) {
	ID3D12DescriptorHeap_Release(store->d3dHeap);
}

static D3D12_FILTER HbGPUi_D3D_Sampler_Filter_ToD3D(HbGPU_Sampler_Filter filter, HbBool isComparison, UINT * maxAnistoropyOut) {
	// Anisotropic implies linear min/mag/mip.
	D3D12_FILTER_TYPE minMagType = filter == HbGPU_Sampler_Filter_Point ? D3D12_FILTER_TYPE_POINT : D3D12_FILTER_TYPE_LINEAR;
	D3D12_FILTER d3dFilter = D3D12_ENCODE_BASIC_FILTER(minMagType, minMagType,
			(filter == HbGPU_Sampler_Filter_Point || filter == HbGPU_Sampler_Filter_Bilinear) ? D3D12_FILTER_TYPE_POINT : D3D12_FILTER_TYPE_LINEAR,
			isComparison ? D3D12_FILTER_REDUCTION_TYPE_COMPARISON : D3D12_FILTER_REDUCTION_TYPE_STANDARD);
	if (filter >= HbGPU_Sampler_Filter_Aniso2x) {
		d3dFilter |= D3D12_ANISOTROPIC_FILTERING_BIT;
		*maxAnistoropyOut = 2 << (filter - HbGPU_Sampler_Filter_Aniso2x);
	} else {
		*maxAnistoropyOut = 1;
	}
	return d3dFilter;
}

static inline D3D12_TEXTURE_ADDRESS_MODE HbGPUi_D3D_Sampler_Wrap_ToD3D(HbGPU_Sampler_Wrap wrap) {
	switch (wrap) {
	case HbGPU_Sampler_Wrap_Repeat:
		return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	case HbGPU_Sampler_Wrap_Clamp:
		return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	case HbGPU_Sampler_Wrap_MirrorRepeat:
		return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
	case HbGPU_Sampler_Wrap_MirrorClamp:
		return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
	case HbGPU_Sampler_Wrap_Border:
		return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	default:
		break;
	}
	return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
}

HbBool HbGPU_SamplerStore_CreateSampler(HbGPU_SamplerStore * store, uint32_t index, HbGPU_Sampler_Info info) {
	D3D12_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = HbGPUi_D3D_Sampler_Filter_ToD3D(info.filter, info.isComparison, &samplerDesc.MaxAnisotropy);
	samplerDesc.AddressU = HbGPUi_D3D_Sampler_Wrap_ToD3D(info.wrapS);
	samplerDesc.AddressV = HbGPUi_D3D_Sampler_Wrap_ToD3D(info.wrapT);
	samplerDesc.AddressW = HbGPUi_D3D_Sampler_Wrap_ToD3D(info.wrapR);
	samplerDesc.MipLODBias = (float) info.mipBias;
	samplerDesc.ComparisonFunc = (D3D12_COMPARISON_FUNC) (D3D12_COMPARISON_FUNC_NEVER + info.comparison);
	samplerDesc.BorderColor[0] = samplerDesc.BorderColor[1] = samplerDesc.BorderColor[2] =
			info.border == HbGPU_Sampler_Border_RGB1_A1 ? 1.0f : 0.0f;
	samplerDesc.BorderColor[3] = info.border != HbGPU_Sampler_Border_RGB0_A0 ? 1.0f : 0.0f;
	samplerDesc.MinLOD = (float) info.mipMostDetailed;
	samplerDesc.MaxLOD =
			info.mipLeastDetailed == HbGPU_Sampler_MipLeastDetailed_FullPyramid ? FLT_MAX : (float) info.mipLeastDetailed;
	ID3D12Device_CreateSampler(store->device->d3dDevice, &samplerDesc, HbGPUi_D3D_SamplerStore_GetCPUHandle(store, index));
	return HbTrue;
}

void HbGPU_SamplerStore_DestroySampler(HbGPU_SamplerStore * store, uint32_t index) {
	// Not needed - samplers are purely descriptors.
}

void HbGPUi_D3D_Sampler_ToStatic(HbGPU_Sampler_Info info, D3D12_STATIC_SAMPLER_DESC * samplerDesc) {
	samplerDesc->Filter = HbGPUi_D3D_Sampler_Filter_ToD3D(info.filter, info.isComparison, &samplerDesc->MaxAnisotropy);
	samplerDesc->AddressU = HbGPUi_D3D_Sampler_Wrap_ToD3D(info.wrapS);
	samplerDesc->AddressV = HbGPUi_D3D_Sampler_Wrap_ToD3D(info.wrapT);
	samplerDesc->AddressW = HbGPUi_D3D_Sampler_Wrap_ToD3D(info.wrapR);
	samplerDesc->MipLODBias = (float) info.mipBias;
	samplerDesc->ComparisonFunc = (D3D12_COMPARISON_FUNC) (D3D12_COMPARISON_FUNC_NEVER + info.comparison);
	switch (info.border) {
	case HbGPU_Sampler_Border_RGB0_A1:
		samplerDesc->BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
		break;
	case HbGPU_Sampler_Border_RGB1_A1:
		samplerDesc->BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
		break;
	default:
		samplerDesc->BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		break;
	}
	samplerDesc->MinLOD = (float) info.mipMostDetailed;
	samplerDesc->MaxLOD =
			info.mipLeastDetailed == HbGPU_Sampler_MipLeastDetailed_FullPyramid ? FLT_MAX : (float) info.mipLeastDetailed;
}

/************************
 * Render target storage
 ************************/

HbBool HbGPU_RTStore_Init(HbGPU_RTStore * store, HbTextU8 const * name, HbGPU_Device * device, HbBool isDepth, uint32_t rtCount) {
	store->device = device;
	store->isDepth = isDepth;
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {
		.Type = isDepth ? D3D12_DESCRIPTOR_HEAP_TYPE_DSV : D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
		.NumDescriptors = rtCount,
		.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
		.NodeMask = 0,
	};
	if (FAILED(ID3D12Device_CreateDescriptorHeap(device->d3dDevice, &heapDesc, &IID_ID3D12DescriptorHeap, &store->d3dHeap))) {
		return HbFalse;
	}
	HbGPUi_D3D_SetObjectName(store->d3dHeap, store->d3dHeap->lpVtbl->SetName, name);
	((HbGPUi_D3D_ID3D12DescriptorHeap_GetCPUDescriptorHandleForHeapStart)
			store->d3dHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart)(
					store->d3dHeap, &store->d3dHeapStart);
	store->d3dImageRefs = HbMemory_TryAlloc(device->d3dMemoryTag, rtCount * sizeof(store->d3dImageRefs[0]), HbFalse);
	if (store->d3dImageRefs == HbNull) {
		ID3D12DescriptorHeap_Release(store->d3dHeap);
		return HbFalse;
	}
	return HbTrue;
}

void HbGPU_RTStore_Destroy(HbGPU_RTStore * store) {
	HbMemory_Free(store->d3dImageRefs);
	ID3D12DescriptorHeap_Release(store->d3dHeap);
}

HbBool HbGPU_RTStore_SetColor(HbGPU_RTStore * store, uint32_t rtIndex,
		HbGPU_Image * image, HbGPU_Image_Slice slice, uint32_t zOf3D) {
	if (HbGPU_Image_Format_IsDepth(image->info.format)) {
		return HbFalse;
	}
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = HbGPUi_D3D_Image_Format_ToTyped(image->info.format);
	switch (image->info.dimensions) {
	case HbGPU_Image_Dimensions_1D:
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1D;
		rtvDesc.Texture1D.MipSlice = slice.mip;
		break;
	case HbGPU_Image_Dimensions_1DArray:
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
		rtvDesc.Texture1DArray.MipSlice = slice.mip;
		rtvDesc.Texture1DArray.FirstArraySlice = slice.layer;
		rtvDesc.Texture1DArray.ArraySize = 1;
		break;
	case HbGPU_Image_Dimensions_2D:
		if (image->info.samplesLog2 > 0) {
			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
		} else {
			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			rtvDesc.Texture2D.MipSlice = slice.mip;
			rtvDesc.Texture2D.PlaneSlice = 0;
		}
		break;
	case HbGPU_Image_Dimensions_2DArray:
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
		rtvDesc.Texture2DArray.MipSlice = slice.mip;
		rtvDesc.Texture2DArray.FirstArraySlice = slice.layer;
		rtvDesc.Texture2DArray.ArraySize = 1;
		rtvDesc.Texture2DArray.PlaneSlice = 0;
		break;
	case HbGPU_Image_Dimensions_Cube:
	case HbGPU_Image_Dimensions_CubeArray:
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
		rtvDesc.Texture2DArray.MipSlice = slice.mip;
		rtvDesc.Texture2DArray.FirstArraySlice = slice.layer * 6 + slice.cubeSide;
		rtvDesc.Texture2DArray.ArraySize = 1;
		rtvDesc.Texture2DArray.PlaneSlice = 0;
		break;
	case HbGPU_Image_Dimensions_3D:
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
		rtvDesc.Texture3D.MipSlice = slice.mip;
		rtvDesc.Texture3D.FirstWSlice = zOf3D;
		rtvDesc.Texture3D.WSize = 1;
		break;
	default:
		return HbFalse;
	}
	ID3D12Device * d3dDevice = store->device->d3dDevice;
	D3D12_CPU_DESCRIPTOR_HANDLE handle = {
		.ptr = store->d3dHeapStart.ptr + rtIndex * store->device->d3dRTVDescriptorSize,
	};
	ID3D12Device_CreateRenderTargetView(d3dDevice, image->d3dResource, &rtvDesc, handle);
	HbGPU_Image_SliceReference * imageRef = &store->d3dImageRefs[rtIndex];
	imageRef->image = image;
	imageRef->slice = slice;
	return HbTrue;
}

HbBool HbGPU_RTStore_SetDepth(HbGPU_RTStore * store, uint32_t rtIndex,
		HbGPU_Image * image, HbGPU_Image_Slice slice, HbBool readOnlyDepth, HbBool readOnlyStencil) {
	if (!HbGPU_Image_Format_IsDepth(image->info.format)) {
		return HbFalse;
	}
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Format = HbGPUi_D3D_Image_Format_ToTyped(image->info.format);
	switch (image->info.dimensions) {
	case HbGPU_Image_Dimensions_1D:
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1D;
		dsvDesc.Texture1D.MipSlice = slice.mip;
		break;
	case HbGPU_Image_Dimensions_1DArray:
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1DARRAY;
		dsvDesc.Texture1DArray.MipSlice = slice.mip;
		dsvDesc.Texture1DArray.FirstArraySlice = slice.layer;
		dsvDesc.Texture1DArray.ArraySize = 1;
		break;
	case HbGPU_Image_Dimensions_2D:
		if (image->info.samplesLog2 > 0) {
			dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
		} else {
			dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			dsvDesc.Texture2D.MipSlice = slice.mip;
		}
		break;
	case HbGPU_Image_Dimensions_2DArray:
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
		dsvDesc.Texture2DArray.MipSlice = slice.mip;
		dsvDesc.Texture2DArray.FirstArraySlice = slice.layer;
		dsvDesc.Texture2DArray.ArraySize = 1;
		break;
	case HbGPU_Image_Dimensions_Cube:
	case HbGPU_Image_Dimensions_CubeArray:
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
		dsvDesc.Texture2DArray.MipSlice = slice.mip;
		dsvDesc.Texture2DArray.FirstArraySlice = slice.layer * 6 + slice.cubeSide;
		dsvDesc.Texture2DArray.ArraySize = 1;
		break;
	default:
		return HbFalse;
	}
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	if (readOnlyDepth) {
		dsvDesc.Flags |= D3D12_DSV_FLAG_READ_ONLY_DEPTH;
	}
	if (readOnlyStencil) {
		dsvDesc.Flags |= D3D12_DSV_FLAG_READ_ONLY_STENCIL;
	}
	ID3D12Device * d3dDevice = store->device->d3dDevice;
	D3D12_CPU_DESCRIPTOR_HANDLE handle = {
		.ptr = store->d3dHeapStart.ptr + rtIndex * store->device->d3dDSVDescriptorSize,
	};
	ID3D12Device_CreateDepthStencilView(d3dDevice, image->d3dResource, &dsvDesc, handle);
	HbGPU_Image_SliceReference * imageRef = &store->d3dImageRefs[rtIndex];
	imageRef->image = image;
	imageRef->slice = slice;
	return HbTrue;
}

HbGPU_RTReference HbGPU_RTStore_GetRT(HbGPU_RTStore * store, uint32_t rtIndex) {
	uint32_t descriptorSize = store->isDepth ? store->device->d3dDSVDescriptorSize : store->device->d3dRTVDescriptorSize;
	HbGPU_RTReference reference = {
		.d3dHandle.ptr = store->d3dHeapStart.ptr + rtIndex * descriptorSize,
		.d3dImageRef = store->d3dImageRefs[rtIndex],
	};
	return reference;
}

/*************
 * Swap chain
 *************/

HbBool HbGPU_SwapChain_Init(HbGPU_SwapChain * chain, HbTextU8 const * name, HbGPU_Device * device,
		HbGPU_SwapChain_Target target, HbGPU_Image_Format format, uint32_t width, uint32_t height, HbBool tripleBuffered) {
	chain->device = device;
	uint32_t bufferCount = tripleBuffered ? 3 : 2;
	bufferCount = tripleBuffered ? 3 : 2;
	width = HbClampU32(width, 1, HbGPU_Image_MaxSize1D2D);
	height = HbClampU32(height, 1, HbGPU_Image_MaxSize1D2D);
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {
		.Width = width,
		.Height = height,
		.Format = HbGPUi_D3D_Image_Format_ToTyped(HbGPU_Image_Format_ToLinear(format)),
		.Stereo = FALSE,
		.SampleDesc.Count = 1,
		.SampleDesc.Quality = 0,
		.BufferUsage = DXGI_USAGE_BACK_BUFFER,
		.BufferCount = bufferCount,
		.Scaling = DXGI_SCALING_STRETCH,
		.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
		.AlphaMode = DXGI_ALPHA_MODE_IGNORE,
		.Flags = 0,
	};
	IUnknown * commandQueue = (IUnknown *) device->d3dCommandQueues[HbGPU_CmdQueue_Graphics];
	IDXGISwapChain1 * swapChain1;
	#if HbPlatform_OS_WindowsDesktop
	if (FAILED(IDXGIFactory2_CreateSwapChainForHwnd(HbGPUi_D3D_DXGIFactory,
			commandQueue, target.windowsHWnd, &swapChainDesc, HbNull, HbNull, &swapChain1))) {
		return HbFalse;
	}
	#else
	#error No HbGPU_SwapChain_Init for the target application model.
	#endif
	if (FAILED(IDXGISwapChain1_QueryInterface(swapChain1, &IID_IDXGISwapChain3, &chain->d3dSwapChain))) {
		IDXGISwapChain1_Release(swapChain1);
		return HbFalse;
	}
	IDXGISwapChain1_Release(swapChain1);
	HbGPUi_D3D_SetDXGIObjectName(chain->d3dSwapChain, chain->d3dSwapChain->lpVtbl->SetPrivateData, name);
	ID3D12Device * d3dDevice = device->d3dDevice;
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {
		.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
		.NumDescriptors = bufferCount,
		.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
		.NodeMask = 0,
	};
	if (FAILED(ID3D12Device_CreateDescriptorHeap(d3dDevice, &rtvHeapDesc, &IID_ID3D12DescriptorHeap, &chain->d3dRTVHeap))) {
		IDXGISwapChain3_Release(chain->d3dSwapChain);
		return HbFalse;
	}
	HbGPUi_D3D_SetSubObjectName(chain->d3dRTVHeap, chain->d3dRTVHeap->lpVtbl->SetName, name, "d3dRTVHeap");
	((HbGPUi_D3D_ID3D12DescriptorHeap_GetCPUDescriptorHandleForHeapStart)
			chain->d3dRTVHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart)(
					chain->d3dRTVHeap, &chain->d3dRTVHeapStart);
	HbTextU8 imageName[] = "images[0]";
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {
		.Format = HbGPUi_D3D_Image_Format_ToTyped(format),
		.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D,
		.Texture2D.MipSlice = 0,
		.Texture2D.PlaneSlice = 0,
	};
	for (uint32_t bufferIndex = 0; bufferIndex < bufferCount; ++bufferIndex) {
		HbGPU_Image * image = &chain->images[bufferIndex];
		ID3D12Resource * imageResource;
		if (FAILED(IDXGISwapChain3_GetBuffer(chain->d3dSwapChain, bufferIndex, &IID_ID3D12Resource, &imageResource))) {
			for (uint32_t releaseBufferIndex = 0; releaseBufferIndex < bufferIndex; ++releaseBufferIndex) {
				ID3D12Resource * releaseImage = chain->images[releaseBufferIndex].d3dResource;
				ID3D12Resource_Release(releaseImage);
			}
			ID3D12DescriptorHeap_Release(chain->d3dRTVHeap);
			IDXGISwapChain3_Release(chain->d3dSwapChain);
			return HbFalse;
		}
		imageName[sizeof(imageName) - 3] = '0' + bufferIndex;
		HbGPUi_D3D_SetSubObjectName(imageResource, imageResource->lpVtbl->SetName, name, imageName);
		image->d3dResource = imageResource;
		image->info.format = format;
		image->info.dimensions = HbGPU_Image_Dimensions_2D;
		image->info.width = width;
		image->info.height = height;
		image->info.depthOrLayers = 1;
		image->info.mips = 1;
		image->info.samplesLog2 = (uint32_t) HbBit_HighestOneU32(swapChainDesc.SampleDesc.Count);
		image->info.usageOptions = HbGPU_Image_UsageOptions_ColorRenderable;
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = {
			.ptr = chain->d3dRTVHeapStart.ptr + bufferIndex * device->d3dRTVDescriptorSize,
		};
		ID3D12Device_CreateRenderTargetView(d3dDevice, imageResource, &rtvDesc, rtvHandle);
	}
	chain->d3dCurrentBackBufferIndex = IDXGISwapChain3_GetCurrentBackBufferIndex(chain->d3dSwapChain);
	return HbTrue;
}

void HbGPU_SwapChain_Destroy(HbGPU_SwapChain * chain) {
	for (uint32_t bufferIndex = 0; bufferIndex < chain->bufferCount; ++bufferIndex) {
		ID3D12Resource_Release(chain->images[bufferIndex].d3dResource);
	}
	ID3D12DescriptorHeap_Release(chain->d3dRTVHeap);
	IDXGISwapChain3_Release(chain->d3dSwapChain);
}

void HbGPU_SwapChain_StartComposition(HbGPU_SwapChain * chain) {
	// Not needed - FinishComposition updates the current buffer index.
}

HbGPU_RTReference HbGPU_SwapChain_GetCurrentRT(HbGPU_SwapChain * chain) {
	HbGPU_RTReference reference = {
		.d3dHandle.ptr = chain->d3dRTVHeapStart.ptr + chain->d3dCurrentBackBufferIndex * chain->device->d3dRTVDescriptorSize,
		.d3dImageRef.image = &chain->images[chain->d3dCurrentBackBufferIndex],
	};
	return reference;
}

HbGPU_Image * HbGPU_SwapChain_GetCurrentImage(HbGPU_SwapChain * chain) {
	return &chain->images[chain->d3dCurrentBackBufferIndex];
}

void HbGPU_SwapChain_FinishComposition(HbGPU_SwapChain * chain, uint32_t vsyncDivisor) {
	IDXGISwapChain3_Present(chain->d3dSwapChain, vsyncDivisor, 0);
	chain->d3dCurrentBackBufferIndex = IDXGISwapChain3_GetCurrentBackBufferIndex(chain->d3dSwapChain);
}

/*****************
 * Binding layout
 *****************/

static inline HbGPUi_D3D_Binding_GetShaderVisibility(HbGPU_ShaderStageBits stages) {
	switch (stages) {
	case HbGPU_ShaderStageBits_Vertex:
		return D3D12_SHADER_VISIBILITY_VERTEX;
	case HbGPU_ShaderStageBits_Pixel:
		return D3D12_SHADER_VISIBILITY_PIXEL;
	default:
		break;
	}
	// Multiple stages or compute.
	return D3D12_SHADER_VISIBILITY_ALL;
}

HbBool HbGPU_BindingLayout_Init(HbGPU_BindingLayout * layout, HbTextU8 const * name, HbGPU_Device * device,
		HbGPU_Binding const * bindings, uint32_t bindingCount, HbBool useVertexAttributes) {
	if (bindingCount > HbGPU_BindingLayout_MaxBindings) {
		return HbFalse;
	}
	memset(layout->d3dRootParameterIndexes, UINT8_MAX, sizeof(layout->d3dRootParameterIndexes));
	D3D12_ROOT_PARAMETER parameters[HbGPU_BindingLayout_MaxBindings];
	uint32_t parameterCount = 0;
	uint32_t staticSamplerCount = 0;
	for (uint32_t bindingIndex = 0; bindingIndex < bindingCount; ++bindingIndex) {
		HbGPU_Binding const * binding = &bindings[bindingIndex];
		D3D12_ROOT_PARAMETER * parameter = &parameters[parameterCount];
		parameter->ShaderVisibility = HbGPUi_D3D_Binding_GetShaderVisibility(binding->stages);
		switch (binding->type) {
		case HbGPU_Binding_Type_HandleRangeSet:
			parameter->ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			uint32_t rangeCount = binding->binding.handleRangeSet.rangeCount;
			if (rangeCount == 0) {
				continue; // HbStackAlloc safety.
			}
			parameter->DescriptorTable.NumDescriptorRanges = rangeCount;
			HbGPU_Binding_HandleRange const * ranges = binding->binding.handleRangeSet.ranges;
			D3D12_DESCRIPTOR_RANGE * d3dRanges = HbStackAlloc(rangeCount * sizeof(D3D12_DESCRIPTOR_RANGE));
			parameter->DescriptorTable.pDescriptorRanges = d3dRanges;
			for (uint32_t rangeIndex = 0; rangeIndex < rangeCount; ++rangeIndex) {
				HbGPU_Binding_HandleRange const * range = &ranges[rangeIndex];
				D3D12_DESCRIPTOR_RANGE * d3dRange = &d3dRanges[rangeIndex];
				switch (range->type) {
				case HbGPU_Binding_HandleRange_Type_ConstantBuffer:
					d3dRange->RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
					break;
				case HbGPU_Binding_HandleRange_Type_ResourceBuffer:
				case HbGPU_Binding_HandleRange_Type_Texture:
					d3dRange->RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
					break;
				case HbGPU_Binding_HandleRange_Type_EditBuffer:
				case HbGPU_Binding_HandleRange_Type_EditImage:
					d3dRange->RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
					break;
				default:
					return HbFalse;
				}
				d3dRange->NumDescriptors = range->handleCount;
				d3dRange->BaseShaderRegister = range->firstRegister.cbufferResourceEdit;
				d3dRange->RegisterSpace = 0;
				d3dRange->OffsetInDescriptorsFromTableStart = range->handleOffset;
			}
			layout->d3dRootParameterIndexes[bindingIndex] = parameterCount++;
			break;
		case HbGPU_Binding_Type_SamplerRangeSet:
			{
				uint32_t rangeCount = binding->binding.samplerRangeSet.rangeCount;
				if (rangeCount == 0) {
					continue; // HbStackAlloc safety.
				}
				HbGPU_Binding_SamplerRange const * ranges = binding->binding.samplerRangeSet.ranges;
				if (binding->binding.samplerRangeSet.staticSamplers != HbNull) {
					for (uint32_t rangeIndex = 0; rangeIndex < rangeCount; ++rangeIndex) {
						staticSamplerCount += ranges[rangeIndex].samplerCount;
					}
					continue;
				}
				parameter->ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
				parameter->DescriptorTable.NumDescriptorRanges = rangeCount;
				D3D12_DESCRIPTOR_RANGE * d3dRanges = HbStackAlloc(rangeCount * sizeof(D3D12_DESCRIPTOR_RANGE));
				parameter->DescriptorTable.pDescriptorRanges = d3dRanges;
				for (uint32_t rangeIndex = 0; rangeIndex < rangeCount; ++rangeIndex) {
					HbGPU_Binding_SamplerRange const * range = &ranges[rangeIndex];
					D3D12_DESCRIPTOR_RANGE * d3dRange = &d3dRanges[rangeIndex];
					d3dRange->RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
					d3dRange->NumDescriptors = range->samplerCount;
					d3dRange->BaseShaderRegister = range->firstRegister;
					d3dRange->RegisterSpace = 0;
					d3dRange->OffsetInDescriptorsFromTableStart = range->samplerOffset;
				}
				layout->d3dRootParameterIndexes[bindingIndex] = parameterCount++;
			}
			break;
		case HbGPU_Binding_Type_ConstantBuffer:
			parameter->ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
			parameter->Descriptor.ShaderRegister = binding->binding.constantBuffer.bindRegister.cbufferResourceEdit;
			parameter->Descriptor.RegisterSpace = 0;
			layout->d3dRootParameterIndexes[bindingIndex] = parameterCount++;
			break;
		case HbGPU_Binding_Type_SmallConstants:
			parameter->ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
			parameter->Constants.ShaderRegister = binding->binding.smallConstants.bindRegister.cbufferResourceEdit;
			parameter->Constants.RegisterSpace = 0;
			parameter->Constants.Num32BitValues = binding->binding.smallConstants.sizeInDwords;
			layout->d3dRootParameterIndexes[bindingIndex] = parameterCount++;
			break;
		default:
			return HbFalse;
		}
	}
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {
		.NumParameters = parameterCount,
		.pParameters = parameters,
		.NumStaticSamplers = staticSamplerCount,
		.Flags = useVertexAttributes ? D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT : D3D12_ROOT_SIGNATURE_FLAG_NONE,
	};
	if (staticSamplerCount != 0) {
		D3D12_STATIC_SAMPLER_DESC * d3dStaticSamplers = HbStackAlloc(staticSamplerCount * sizeof(D3D12_STATIC_SAMPLER_DESC));
		rootSignatureDesc.pStaticSamplers = d3dStaticSamplers;
		uint32_t d3dStaticSamplerIndex = 0;
		for (uint32_t bindingIndex = 0; bindingIndex < bindingCount; ++bindingIndex) {
			HbGPU_Binding const * binding = &bindings[bindingIndex];
			if (binding->type != HbGPU_Binding_Type_SamplerRangeSet) {
				continue;
			}
			HbGPU_Sampler_Info const * bindingSamplers = binding->binding.samplerRangeSet.staticSamplers;
			uint32_t rangeCount = binding->binding.samplerRangeSet.rangeCount;
			if (bindingSamplers == HbNull || rangeCount == 0) {
				continue;
			}
			D3D12_SHADER_VISIBILITY bindingVisiblity = HbGPUi_D3D_Binding_GetShaderVisibility(binding->stages);
			uint32_t bindingSamplerIndex = 0;
			HbGPU_Binding_SamplerRange const * ranges = binding->binding.samplerRangeSet.ranges;
			for (uint32_t rangeIndex = 0; rangeIndex < rangeCount; ++rangeIndex) {
				HbGPU_Binding_SamplerRange const * range = &ranges[rangeIndex];
				for (uint32_t rangeSamplerIndex = 0; rangeSamplerIndex < range->samplerCount; ++rangeSamplerIndex) {
					D3D12_STATIC_SAMPLER_DESC * d3dStaticSampler = &d3dStaticSamplers[d3dStaticSamplerIndex++];
					HbGPUi_D3D_Sampler_ToStatic(bindingSamplers[bindingSamplerIndex++], d3dStaticSampler);
					d3dStaticSampler->ShaderRegister = range->firstRegister + rangeSamplerIndex;
					d3dStaticSampler->RegisterSpace = 0;
					d3dStaticSampler->ShaderVisibility = bindingVisiblity;
				}
			}
		}
	}
	ID3D10Blob * blob, * errorBlob = HbNull;
	if (FAILED(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &errorBlob))) {
		if (errorBlob != HbNull) {
			ID3D10Blob_Release(errorBlob);
		}
		return HbFalse;
	}
	if (FAILED(ID3D12Device_CreateRootSignature(device->d3dDevice, 0, ID3D10Blob_GetBufferPointer(blob), ID3D10Blob_GetBufferSize(blob),
			&IID_ID3D12RootSignature, &layout->d3dRootSignature))) {
		ID3D10Blob_Release(blob);
		return HbFalse;
	}
	ID3D10Blob_Release(blob);
	HbGPUi_D3D_SetObjectName(layout->d3dRootSignature, layout->d3dRootSignature->lpVtbl->SetName, name);
	return HbTrue;
}

void HbGPU_BindingLayout_Destroy(HbGPU_BindingLayout * layout) {
	ID3D12RootSignature_Release(layout->d3dRootSignature);
}

/************************
 * Drawing configuration
 ************************/

static D3D12_BLEND HbGPUi_D3D_DrawConfig_RT_BlendFactor_ToD3D(HbGPU_DrawConfig_RT_BlendFactor factor, HbBool alpha) {
	switch (factor) {
	case HbGPU_DrawConfig_RT_BlendFactor_Zero:
		return D3D12_BLEND_ZERO;
	case HbGPU_DrawConfig_RT_BlendFactor_One:
		return D3D12_BLEND_ONE;
	case HbGPU_DrawConfig_RT_BlendFactor_SourceColor:
		return alpha ? D3D12_BLEND_SRC_ALPHA : D3D12_BLEND_SRC_COLOR;
	case HbGPU_DrawConfig_RT_BlendFactor_OneMinusSourceColor:
		return alpha ? D3D12_BLEND_INV_SRC_ALPHA : D3D12_BLEND_INV_SRC_COLOR;
	case HbGPU_DrawConfig_RT_BlendFactor_SourceAlpha:
		return D3D12_BLEND_SRC_ALPHA;
	case HbGPU_DrawConfig_RT_BlendFactor_OneMinusSourceAlpha:
		return D3D12_BLEND_INV_SRC_ALPHA;
	case HbGPU_DrawConfig_RT_BlendFactor_TargetColor:
		return alpha ? D3D12_BLEND_DEST_ALPHA : D3D12_BLEND_DEST_COLOR;
	case HbGPU_DrawConfig_RT_BlendFactor_OneMinusTargetColor:
		return alpha ? D3D12_BLEND_INV_DEST_ALPHA : D3D12_BLEND_INV_DEST_COLOR;
	case HbGPU_DrawConfig_RT_BlendFactor_TargetAlpha:
		return D3D12_BLEND_DEST_ALPHA;
	case HbGPU_DrawConfig_RT_BlendFactor_OneMinusTargetAlpha:
		return D3D12_BLEND_INV_DEST_ALPHA;
	case HbGPU_DrawConfig_RT_BlendFactor_SourceAlphaSaturated:
		return D3D12_BLEND_SRC_ALPHA_SAT;
	case HbGPU_DrawConfig_RT_BlendFactor_Constant:
		return D3D12_BLEND_BLEND_FACTOR;
	case HbGPU_DrawConfig_RT_BlendFactor_OneMinusConstant:
		return D3D12_BLEND_INV_BLEND_FACTOR;
	case HbGPU_DrawConfig_RT_BlendFactor_Source1Color:
		return alpha ? D3D12_BLEND_SRC1_ALPHA : D3D12_BLEND_SRC1_COLOR;
	case HbGPU_DrawConfig_RT_BlendFactor_OneMinusSource1Color:
		return alpha ? D3D12_BLEND_INV_SRC1_ALPHA : D3D12_BLEND_INV_SRC1_COLOR;
	case HbGPU_DrawConfig_RT_BlendFactor_Source1Alpha:
		return D3D12_BLEND_SRC1_ALPHA;
	case HbGPU_DrawConfig_RT_BlendFactor_OneMinusSource1Alpha:
		return D3D12_BLEND_INV_SRC1_ALPHA;
	default:
		break;
	}
	return D3D12_BLEND_ZERO;
}

static D3D12_BLEND_OP HbGPUi_D3D_DrawConfig_RT_BlendOp_ToD3D(HbGPU_DrawConfig_RT_BlendOp op) {
	switch (op) {
	case HbGPU_DrawConfig_RT_BlendOp_Add:
		return D3D12_BLEND_OP_ADD;
	case HbGPU_DrawConfig_RT_BlendOp_Subtract:
		return D3D12_BLEND_OP_SUBTRACT;
	case HbGPU_DrawConfig_RT_BlendOp_ReverseSubtract:
		return D3D12_BLEND_OP_REV_SUBTRACT;
	case HbGPU_DrawConfig_RT_BlendOp_Min:
		return D3D12_BLEND_OP_MIN;
	case HbGPU_DrawConfig_RT_BlendOp_Max:
		return D3D12_BLEND_OP_MAX;
	default:
		break;
	}
	return D3D12_BLEND_OP_ADD;
}

static D3D12_STENCIL_OP HbGPUi_D3D_DrawConfig_Stencil_Op_ToD3D(HbGPU_DrawConfig_Stencil_Op op) {
	switch (op) {
	case HbGPU_DrawConfig_Stencil_Op_Keep:
		return D3D12_STENCIL_OP_KEEP;
	case HbGPU_DrawConfig_Stencil_Op_Zero:
		return D3D12_STENCIL_OP_ZERO;
	case HbGPU_DrawConfig_Stencil_Op_Replace:
		return D3D12_STENCIL_OP_REPLACE;
	case HbGPU_DrawConfig_Stencil_Op_IncrementSaturate:
		return D3D12_STENCIL_OP_INCR_SAT;
	case HbGPU_DrawConfig_Stencil_Op_DecrementSaturate:
		return D3D12_STENCIL_OP_DECR_SAT;
	case HbGPU_DrawConfig_Stencil_Op_Invert:
		return D3D12_STENCIL_OP_INVERT;
	case HbGPU_DrawConfig_Stencil_Op_Increment:
		return D3D12_STENCIL_OP_INCR;
	case HbGPU_DrawConfig_Stencil_Op_Decrement:
		return D3D12_STENCIL_OP_DECR;
	}
	return D3D12_STENCIL_OP_KEEP;
}

HbBool HbGPU_DrawConfig_Init(HbGPU_DrawConfig * config, HbTextU8 const * name, HbGPU_Device * device, HbGPU_DrawConfig_Info const * info) {
	if (info->vertexStreamCount > D3D12_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT ||
		info->vertexAttributeCount > D3D12_IA_VERTEX_INPUT_STRUCTURE_ELEMENT_COUNT ||
		info->rtCount > D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT) {
		return HbFalse;
	}
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc = { 0 };
	pipelineStateDesc.pRootSignature = info->bindingLayout->d3dRootSignature;
	pipelineStateDesc.VS.pShaderBytecode = info->shaderVertex.dxbc;
	pipelineStateDesc.VS.BytecodeLength = info->shaderVertex.dxbcSize;
	pipelineStateDesc.PS.pShaderBytecode = info->shaderPixel.dxbc;
	pipelineStateDesc.PS.BytecodeLength = info->shaderPixel.dxbcSize;
	pipelineStateDesc.BlendState.AlphaToCoverageEnable = info->alphaToCoverage;
	pipelineStateDesc.BlendState.IndependentBlendEnable = !info->rtsSameBlendAndWriteMasks;
	pipelineStateDesc.NumRenderTargets = info->rtCount;
	for (uint32_t rtIndex = 0; rtIndex < info->rtCount; ++rtIndex) {
		HbGPU_DrawConfig_RT const * rt = &info->rts[rtIndex];
		if (rtIndex == 0 || !info->rtsSameBlendAndWriteMasks) {
			D3D12_RENDER_TARGET_BLEND_DESC * blendDesc = &pipelineStateDesc.BlendState.RenderTarget[rtIndex];
			blendDesc->RenderTargetWriteMask = rt->unmodifiedComponentsMask ^ 0xF;
			if (rt->blend) {
				blendDesc->BlendEnable = TRUE;
				blendDesc->SrcBlend = HbGPUi_D3D_DrawConfig_RT_BlendFactor_ToD3D(rt->blendFactorSourceRGB, HbFalse);
				blendDesc->DestBlend = HbGPUi_D3D_DrawConfig_RT_BlendFactor_ToD3D(rt->blendFactorTargetRGB, HbFalse);
				blendDesc->BlendOp = HbGPUi_D3D_DrawConfig_RT_BlendOp_ToD3D(rt->blendOpRGB);
				blendDesc->SrcBlendAlpha = HbGPUi_D3D_DrawConfig_RT_BlendFactor_ToD3D(rt->blendFactorSourceAlpha, HbTrue);
				blendDesc->DestBlendAlpha = HbGPUi_D3D_DrawConfig_RT_BlendFactor_ToD3D(rt->blendFactorTargetAlpha, HbTrue);
				blendDesc->BlendOpAlpha = HbGPUi_D3D_DrawConfig_RT_BlendOp_ToD3D(rt->blendOpAlpha);
			}
		}
		pipelineStateDesc.RTVFormats[rtIndex] = HbGPUi_D3D_Image_Format_ToTyped(rt->format);
	}
	pipelineStateDesc.SampleMask = UINT_MAX;
	pipelineStateDesc.RasterizerState.FillMode = info->wireframe ? D3D12_FILL_MODE_WIREFRAME : D3D12_FILL_MODE_SOLID;
	if (info->cullSide < 0) {
		pipelineStateDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	} else if (info->cullSide > 0) {
		pipelineStateDesc.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT;
	} else {
		pipelineStateDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	}
	pipelineStateDesc.RasterizerState.FrontCounterClockwise = info->frontCounterClockwise;
	pipelineStateDesc.RasterizerState.DepthBias = info->depthBias;
	pipelineStateDesc.RasterizerState.DepthBiasClamp = info->depthBiasClamp;
	pipelineStateDesc.RasterizerState.SlopeScaledDepthBias = info->depthBiasSlope;
	pipelineStateDesc.RasterizerState.DepthClipEnable = !info->depthClamp;
	HbGPU_DrawConfig_DepthStencilInfo const * depthStencil = info->depthStencil;
	if (depthStencil != HbNull) {
		if (depthStencil->depthTest) {
			pipelineStateDesc.DepthStencilState.DepthEnable = TRUE;
			pipelineStateDesc.DepthStencilState.DepthWriteMask =
					depthStencil->depthWrite ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
			pipelineStateDesc.DepthStencilState.DepthFunc =
					(D3D12_COMPARISON_FUNC) (D3D12_COMPARISON_FUNC_NEVER + depthStencil->depthComparison);
		}
		if (depthStencil->stencil) {
			pipelineStateDesc.DepthStencilState.StencilEnable = TRUE;
			pipelineStateDesc.DepthStencilState.StencilReadMask = depthStencil->stencilReadMask;
			pipelineStateDesc.DepthStencilState.StencilWriteMask = depthStencil->stencilWriteMask;
			pipelineStateDesc.DepthStencilState.FrontFace.StencilFailOp =
					HbGPUi_D3D_DrawConfig_Stencil_Op_ToD3D(depthStencil->stencilFront.fail);
			pipelineStateDesc.DepthStencilState.FrontFace.StencilDepthFailOp =
					HbGPUi_D3D_DrawConfig_Stencil_Op_ToD3D(depthStencil->stencilFront.depthFail);
			pipelineStateDesc.DepthStencilState.FrontFace.StencilPassOp =
					HbGPUi_D3D_DrawConfig_Stencil_Op_ToD3D(depthStencil->stencilFront.pass);
			pipelineStateDesc.DepthStencilState.FrontFace.StencilFunc =
					(D3D12_COMPARISON_FUNC) (D3D12_COMPARISON_FUNC_NEVER + depthStencil->stencilFront.comparison);
			pipelineStateDesc.DepthStencilState.BackFace.StencilFailOp =
					HbGPUi_D3D_DrawConfig_Stencil_Op_ToD3D(depthStencil->stencilBack.fail);
			pipelineStateDesc.DepthStencilState.BackFace.StencilDepthFailOp =
					HbGPUi_D3D_DrawConfig_Stencil_Op_ToD3D(depthStencil->stencilBack.depthFail);
			pipelineStateDesc.DepthStencilState.BackFace.StencilPassOp =
					HbGPUi_D3D_DrawConfig_Stencil_Op_ToD3D(depthStencil->stencilBack.pass);
			pipelineStateDesc.DepthStencilState.BackFace.StencilFunc =
					(D3D12_COMPARISON_FUNC) (D3D12_COMPARISON_FUNC_NEVER + depthStencil->stencilBack.comparison);
		}
		pipelineStateDesc.DSVFormat = HbGPUi_D3D_Image_Format_ToTyped(depthStencil->format);
	}
	memset(config->d3dVertexStreamStridesInDwords, 0, sizeof(config->d3dVertexStreamStridesInDwords));
	for (uint32_t vertexStreamIndex = 0; vertexStreamIndex < info->vertexStreamCount; ++vertexStreamIndex) {
		config->d3dVertexStreamStridesInDwords[vertexStreamIndex] = info->vertexStreams[vertexStreamIndex].strideInDwords;
	}
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[D3D12_IA_VERTEX_INPUT_STRUCTURE_ELEMENT_COUNT];
	for (uint32_t attributeIndex = 0; attributeIndex < info->vertexAttributeCount; ++attributeIndex) {
		HbGPU_Vertex_Attribute const * attribute = &info->vertexAttributes[attributeIndex];
		D3D12_INPUT_ELEMENT_DESC * inputElementDesc = &inputElementDescs[attributeIndex];
		switch (attribute->semantic) {
		case HbGPU_Vertex_Semantic_Position: inputElementDesc->SemanticName = "POSITION"; break;
		case HbGPU_Vertex_Semantic_Normal: inputElementDesc->SemanticName = "NORMAL"; break;
		case HbGPU_Vertex_Semantic_Tangent: inputElementDesc->SemanticName = "TANGENT"; break;
		case HbGPU_Vertex_Semantic_TexCoord: inputElementDesc->SemanticName = "TEXCOORD"; break;
		case HbGPU_Vertex_Semantic_Color: inputElementDesc->SemanticName = "COLOR"; break;
		case HbGPU_Vertex_Semantic_BlendIndexes: inputElementDesc->SemanticName = "BLENDINDICES"; break;
		case HbGPU_Vertex_Semantic_BlendWeights: inputElementDesc->SemanticName = "BLENDWEIGHTS"; break;
		case HbGPU_Vertex_Semantic_InstancePosition: inputElementDesc->SemanticName = "INSTANCEPOSITION"; break;
		case HbGPU_Vertex_Semantic_InstanceRotation: inputElementDesc->SemanticName = "INSTANCEROTATION"; break;
		default: return HbFalse;
		}
		inputElementDesc->SemanticIndex = attribute->semanticIndex;
		static DXGI_FORMAT const vertexFormats[] = {
			[HbGPU_Vertex_Format_Float_32x1] = DXGI_FORMAT_R32_FLOAT,
			[HbGPU_Vertex_Format_Float_32x2] = DXGI_FORMAT_R32G32_FLOAT,
			[HbGPU_Vertex_Format_Float_32x3] = DXGI_FORMAT_R32G32B32_FLOAT,
			[HbGPU_Vertex_Format_Float_32x4] = DXGI_FORMAT_R32G32B32A32_FLOAT,
			[HbGPU_Vertex_Format_Float_16x2] = DXGI_FORMAT_R16G16_FLOAT,
			[HbGPU_Vertex_Format_Float_16x4] = DXGI_FORMAT_R16G16B16A16_FLOAT,
			[HbGPU_Vertex_Format_Float_11_11_10] = DXGI_FORMAT_R11G11B10_FLOAT,
			[HbGPU_Vertex_Format_UNorm_16x2] = DXGI_FORMAT_R16G16_UNORM,
			[HbGPU_Vertex_Format_UNorm_16x4] = DXGI_FORMAT_R16G16B16A16_UNORM,
			[HbGPU_Vertex_Format_UNorm_8x4] = DXGI_FORMAT_R8G8B8A8_UNORM,
			[HbGPU_Vertex_Format_SNorm_16x2] = DXGI_FORMAT_R16G16_SNORM,
			[HbGPU_Vertex_Format_SNorm_16x4] = DXGI_FORMAT_R16G16B16A16_SNORM,
			[HbGPU_Vertex_Format_SNorm_8x4] = DXGI_FORMAT_R8G8B8A8_SNORM,
			[HbGPU_Vertex_Format_UInt_32x1] = DXGI_FORMAT_R32_UINT,
			[HbGPU_Vertex_Format_UInt_32x2] = DXGI_FORMAT_R32G32_UINT,
			[HbGPU_Vertex_Format_UInt_32x3] = DXGI_FORMAT_R32G32B32_UINT,
			[HbGPU_Vertex_Format_UInt_32x4] = DXGI_FORMAT_R32G32B32A32_UINT,
			[HbGPU_Vertex_Format_UInt_16x2] = DXGI_FORMAT_R16G16_UINT,
			[HbGPU_Vertex_Format_UInt_16x4] = DXGI_FORMAT_R16G16B16A16_UINT,
			[HbGPU_Vertex_Format_UInt_8x4] = DXGI_FORMAT_R8G8B8A8_UINT,
			[HbGPU_Vertex_Format_SInt_32x1] = DXGI_FORMAT_R32_SINT,
			[HbGPU_Vertex_Format_SInt_32x2] = DXGI_FORMAT_R32G32_SINT,
			[HbGPU_Vertex_Format_SInt_32x3] = DXGI_FORMAT_R32G32B32_SINT,
			[HbGPU_Vertex_Format_SInt_32x4] = DXGI_FORMAT_R32G32B32A32_SINT,
			[HbGPU_Vertex_Format_SInt_16x2] = DXGI_FORMAT_R16G16_SINT,
			[HbGPU_Vertex_Format_SInt_16x4] = DXGI_FORMAT_R16G16B16A16_SINT,
			[HbGPU_Vertex_Format_SInt_8x4] = DXGI_FORMAT_R8G8B8A8_SINT,
		};
		if ((uint32_t) attribute->format >= HbArrayLength(vertexFormats)) {
			return HbFalse;
		}
		inputElementDesc->Format = vertexFormats[attribute->format];
		inputElementDesc->InputSlot = attribute->streamIndex;
		inputElementDesc->AlignedByteOffset = attribute->offsetInDwords << 2;
		inputElementDesc->InstanceDataStepRate = info->vertexStreams[attribute->streamIndex].instanceStepRate;
		inputElementDesc->InputSlotClass = inputElementDesc->InstanceDataStepRate != 0 ?
				D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA : D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
	}
	pipelineStateDesc.InputLayout.pInputElementDescs = inputElementDescs;
	pipelineStateDesc.InputLayout.NumElements = info->vertexAttributeCount;
	pipelineStateDesc.IBStripCutValue = sizeof(HbGPU_Vertex_Index) >= sizeof(uint32_t) ?
			D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFFFFFF : D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFF;
	switch (info->inputPrimitive) {
	case HbGPU_DrawConfig_InputPrimitive_Triangle:
		pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		break;
	case HbGPU_DrawConfig_InputPrimitive_Line:
		pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
		break;
	case HbGPU_DrawConfig_InputPrimitive_Point:
		pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
		break;
	default:
		return HbFalse;
	}
	pipelineStateDesc.SampleDesc.Count = 1 << info->samplesLog2;
	if (FAILED(ID3D12Device_CreateGraphicsPipelineState(
			device->d3dDevice, &pipelineStateDesc, &IID_ID3D12PipelineState, &config->d3dPipelineState))) {
		return HbFalse;
	}
	HbGPUi_D3D_SetObjectName(config->d3dPipelineState, config->d3dPipelineState->lpVtbl->SetName, name);
	return HbTrue;
}

void HbGPU_DrawConfig_Destroy(HbGPU_DrawConfig * config) {
	ID3D12PipelineState_Release(config->d3dPipelineState);
}

/****************************
 * Computation configuration
 ****************************/

HbBool HbGPU_ComputeConfig_Init(HbGPU_ComputeConfig * config, HbTextU8 const * name, HbGPU_Device * device,
		HbGPU_ShaderReference shader, uint32_t const groupSize[3], HbGPU_BindingLayout * bindingLayout) {
	D3D12_COMPUTE_PIPELINE_STATE_DESC pipelineStateDesc = {
		.pRootSignature = bindingLayout->d3dRootSignature,
		.CS.pShaderBytecode = shader.dxbc,
		.CS.BytecodeLength = shader.dxbcSize,
	};
	if (FAILED(ID3D12Device_CreateComputePipelineState(
			device->d3dDevice, &pipelineStateDesc, &IID_ID3D12PipelineState, &config->d3dPipelineState))) {
		return HbFalse;
	}
	HbGPUi_D3D_SetObjectName(config->d3dPipelineState, config->d3dPipelineState->lpVtbl->SetName, name);
	return HbTrue;
}

void HbGPU_ComputeConfig_Destroy(HbGPU_ComputeConfig * config) {
	ID3D12PipelineState_Release(config->d3dPipelineState);
}

#endif
