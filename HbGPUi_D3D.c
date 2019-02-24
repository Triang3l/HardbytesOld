#include "HbGPUi_D3D.h"
#if HbGPU_Implementation_D3D
#include "HbBit.h"
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
		HbGPUi_D3D_SetObjectName(object, setter, name);
		return;
	}
	if (name == HbNull || name[0] == '\0') {
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
		HbGPUi_D3D_SetDXGIObjectName(object, setter, name);
		return;
	}
	if (name == HbNull || name[0] == '\0') {
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
		return HbFalse;
	}

	// Query adapter interface version 3 for memory information access.
	if (FAILED(IDXGIAdapter1_QueryInterface(adapter, &IID_IDXGIAdapter3, &device->dxgiAdapter))) {
		IDXGIAdapter1_Release(adapter);
		return HbFalse;
	}
	IDXGIAdapter1_Release(adapter); // Don't need version 1 anymore.
	HbGPUi_D3D_SetDXGISubObjectName(device->dxgiAdapter, device->dxgiAdapter->lpVtbl->SetPrivateData, name, "dxgiAdapter");

	// Create the D3D device.
	if (FAILED(D3D12CreateDevice((IUnknown *) device->dxgiAdapter, D3D_FEATURE_LEVEL_11_0, &IID_ID3D12Device, &device->d3dDevice))) {
		IDXGIAdapter3_Release(device->dxgiAdapter);
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
	}
	D3D12_RESOURCE_STATES states = (D3D12_RESOURCE_STATES) 0;
	if (usage & (HbGPU_Buffer_Usage_Read_Vertices | HbGPU_Buffer_Usage_Read_Constants)) {
		states |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
	}
	if (usage & HbGPU_Buffer_Usage_Read_Indices) {
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
	return HbTrue;
}

void HbGPU_RTStore_Destroy(HbGPU_RTStore * store) {
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
	return HbTrue;
}

HbGPU_RTReference HbGPU_RTStore_GetRT(HbGPU_RTStore * store, uint32_t rtIndex) {
	uint32_t descriptorSize = store->isDepth ? store->device->d3dDSVDescriptorSize : store->device->d3dRTVDescriptorSize;
	HbGPU_RTReference reference = {
		.d3dHandle.ptr = store->d3dHeapStart.ptr + rtIndex * descriptorSize,
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
	if (FAILED(IDXGISwapChain1_QueryInterface(swapChain1, &IID_IDXGISwapChain3, &chain->dxgiSwapChain))) {
		IDXGISwapChain1_Release(swapChain1);
		return HbFalse;
	}
	IDXGISwapChain1_Release(swapChain1);
	HbGPUi_D3D_SetDXGIObjectName(chain->dxgiSwapChain, chain->dxgiSwapChain->lpVtbl->SetPrivateData, name);
	ID3D12Device * d3dDevice = device->d3dDevice;
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {
		.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
		.NumDescriptors = bufferCount,
		.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
		.NodeMask = 0,
	};
	if (FAILED(ID3D12Device_CreateDescriptorHeap(d3dDevice, &rtvHeapDesc, &IID_ID3D12DescriptorHeap, &chain->d3dRTVHeap))) {
		IDXGISwapChain3_Release(chain->dxgiSwapChain);
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
		if (FAILED(IDXGISwapChain3_GetBuffer(chain->dxgiSwapChain, bufferIndex, &IID_ID3D12Resource, &imageResource))) {
			for (uint32_t releaseBufferIndex = 0; releaseBufferIndex < bufferIndex; ++releaseBufferIndex) {
				ID3D12Resource * releaseImage = chain->images[releaseBufferIndex].d3dResource;
				ID3D12Resource_Release(releaseImage);
			}
			ID3D12DescriptorHeap_Release(chain->d3dRTVHeap);
			IDXGISwapChain3_Release(chain->dxgiSwapChain);
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
	return HbTrue;
}

void HbGPU_SwapChain_Destroy(HbGPU_SwapChain * chain) {
	for (uint32_t bufferIndex = 0; bufferIndex < chain->bufferCount; ++bufferIndex) {
		ID3D12Resource_Release(chain->images[bufferIndex].d3dResource);
	}
	ID3D12DescriptorHeap_Release(chain->d3dRTVHeap);
	IDXGISwapChain3_Release(chain->dxgiSwapChain);
}

#endif
