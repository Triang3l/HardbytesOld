#include "HbGPUi_D3D.h"
#if HbGPU_Implementation_D3D
#include <dxgidebug.h>

static HbBool HbGPUi_D3D_Debug = HbFalse;
IDXGIFactory2 * HbGPUi_D3D_DXGIFactory = HbNull;

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

#endif
