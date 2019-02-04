#ifndef HbInclude_HbGPUi_D3D
#define HbInclude_HbGPUi_D3D
#include "HbGPU.h"
#include "HbText.h"
#if HbGPU_Implementation_D3D

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

#endif
#endif
