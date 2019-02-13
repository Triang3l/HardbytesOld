#ifndef HbInclude_HbGPUi_D3D
#define HbInclude_HbGPUi_D3D
#include "HbGPU.h"
#if HbGPU_Implementation_D3D
#include "HbText.h"

extern IDXGIFactory2 * HbGPUi_D3D_DXGIFactory;

typedef HRESULT (STDMETHODCALLTYPE * HbGPUi_D3D_ObjectNameSetter)(void * object, WCHAR const * name);
typedef HRESULT (STDMETHODCALLTYPE * HbGPUi_D3D_DXGIPrivateDataSetter)(void * object, REFGUID name, UINT dataSize, void const * data);
 // Setter is SetName from vtbl.
void HbGPUi_D3D_SetObjectName(void * object, HbGPUi_D3D_ObjectNameSetter setter, HbTextU8 const * name);
void HbGPUi_D3D_SetSubObjectName(void * object, HbGPUi_D3D_ObjectNameSetter setter,
		HbTextU8 const * parentName, HbTextU8 const * name);
// Setter is SetPrivateData from vtbl.
void HbGPUi_D3D_SetDXGIObjectName(void * object, HbGPUi_D3D_DXGIPrivateDataSetter setter, HbTextU8 const * name);
void HbGPUi_D3D_SetDXGISubObjectName(void * object, HbGPUi_D3D_DXGIPrivateDataSetter setter,
		HbTextU8 const * parentName, HbTextU8 const * name);

// Fixes for methods returning structures.
typedef void (STDMETHODCALLTYPE * HbGPUi_D3D_ID3D12DescriptorHeap_GetCPUDescriptorHandleForHeapStart)(
		ID3D12DescriptorHeap * heap, D3D12_CPU_DESCRIPTOR_HANDLE * handle);
typedef void (STDMETHODCALLTYPE * HbGPUi_D3D_ID3D12DescriptorHeap_GetGPUDescriptorHandleForHeapStart)(
		ID3D12DescriptorHeap * heap, D3D12_GPU_DESCRIPTOR_HANDLE * handle);
typedef void (STDMETHODCALLTYPE * HbGPUi_D3D_ID3D12Resource_GetDesc)(ID3D12Resource * resource, D3D12_RESOURCE_DESC * desc);

D3D12_RESOURCE_STATES HbGPUi_D3D_Buffer_Usage_ToStates(HbGPU_Buffer_Usage usage);

DXGI_FORMAT HbGPUi_D3D_Image_Format_ToTyped(HbGPU_Image_Format format);
DXGI_FORMAT HbGPUi_D3D_Image_Format_ToTypeless(HbGPU_Image_Format format);
DXGI_FORMAT HbGPUi_D3D_Image_Format_ToTexture(HbGPU_Image_Format format, HbBool stencil);
// Assumes valid info.
void HbGPUi_D3D_Image_Info_ToResourceDesc(D3D12_RESOURCE_DESC * desc, HbGPU_Image_Info const * info);
D3D12_RESOURCE_STATES HbGPUi_D3D_Image_Usage_ToStates(HbGPU_Image_Usage usage);
inline uint32_t HbGPUi_D3D_Image_Slice_ToSubresource(HbGPU_Image_Info const * info, HbGPU_Image_Slice slice) {
	uint32_t cubeSideCount = HbGPU_Image_Dimensions_AreCube(info->dimensions) ? 6 : 1;
	uint32_t arrayLayerCount = info->depthOrLayers * cubeSideCount;
	uint32_t arrayLayer = slice.layer * cubeSideCount + slice.cubeSide;
	return slice.stencil * (arrayLayerCount * info->mips) + arrayLayer * info->mips + slice.mip;
}
// Doesn't increment reference count of the resource. Starts in Present usage.
void HbGPUi_D3D_Image_WrapSwapChainBuffer(HbGPU_Image * image, HbTextU8 const * name,
		ID3D12Resource * resource, HbGPU_Image_Format format);

#endif
#endif
