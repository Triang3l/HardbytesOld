#include "HbGPUi_D3D.h"
#if HbGPU_Implementation_D3D

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
		.SizeInBytes = HbAlign(size, (uint32_t) HbGPU_Buffer_ConstantsAlignment),
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
		.Buffer.StructureByteStride = HbGPU_Image_Format_ElementSize(format),
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
		.Buffer.StructureByteStride = HbGPU_Image_Format_ElementSize(format),
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

#endif
