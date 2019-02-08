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

D3D12_RESOURCE_STATES HbGPUi_D3D_Buffer_Usage_ToStates(HbGPU_Buffer_Usage usage);

// Assumes valid info.
DXGI_FORMAT HbGPUi_D3D_Image_Format_ToTyped(HbGPU_Image_Format format);
DXGI_FORMAT HbGPUi_D3D_Image_Format_ToTypeless(HbGPU_Image_Format format);
DXGI_FORMAT HbGPUi_D3D_Image_Format_ToTexture(HbGPU_Image_Format format, HbBool stencil);
void HbGPUi_D3D_Image_Info_ToResourceDesc(D3D12_RESOURCE_DESC * desc, HbGPU_Image_Info const * info);
D3D12_RESOURCE_STATES HbGPUi_D3D_Image_Usage_ToStates(HbGPU_Image_Usage usage);

#endif
#endif
