#include "HbGPUi_D3D.h"
#if HbGPU_Implementation_D3D
#include "HbBit.h"
#include "HbFeedback.h"

DXGI_FORMAT HbGPUi_D3D_Image_Format_ToTyped(HbGPU_Image_Format format) {
	if (format >= HbGPU_Image_Format_FormatCount) {
		return DXGI_FORMAT_UNKNOWN;
	}
	static const DXGI_FORMAT dxgiFormats[] = {
		[HbGPU_Image_Format_8_R_UNorm] = DXGI_FORMAT_R8_UNORM,
		[HbGPU_Image_Format_8_8_RG_UNorm] = DXGI_FORMAT_R8G8_UNORM,
		[HbGPU_Image_Format_8_8_8_8_RGBA_UNorm] = DXGI_FORMAT_R8G8B8A8_UNORM,
		[HbGPU_Image_Format_8_8_8_8_RGBA_sRGB] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
		[HbGPU_Image_Format_32_UInt] = DXGI_FORMAT_R32_UINT,
		[HbGPU_Image_Format_32_Float] = DXGI_FORMAT_R32_FLOAT,
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
	static const DXGI_FORMAT dxgiFormats[] = {
		[HbGPU_Image_Format_8_R_UNorm] = DXGI_FORMAT_R8_TYPELESS,
		[HbGPU_Image_Format_8_8_RG_UNorm] = DXGI_FORMAT_R8G8_TYPELESS,
		[HbGPU_Image_Format_8_8_8_8_RGBA_UNorm] = DXGI_FORMAT_R8G8B8A8_TYPELESS,
		[HbGPU_Image_Format_8_8_8_8_RGBA_sRGB] = DXGI_FORMAT_R8G8B8A8_TYPELESS,
		[HbGPU_Image_Format_32_UInt] = DXGI_FORMAT_R32_TYPELESS,
		[HbGPU_Image_Format_32_Float] = DXGI_FORMAT_R32_TYPELESS,
		[HbGPU_Image_Format_D32] = DXGI_FORMAT_R32_TYPELESS,
		[HbGPU_Image_Format_D32_S8] = DXGI_FORMAT_R32G8X24_TYPELESS,
	};
	HbFeedback_StaticAssert(HbArrayLength(dxgiFormats) == HbGPU_Image_Format_FormatCount,
			"All known image formats must be mapped in HbGPUi_D3D_Image_Format_ToTypeless.");
	return dxgiFormats[(uint32_t) format];
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
	if (HbGPU_Image_Format_IsDepth(info->format) &&
		!(info->usageOptions & HbGPU_Image_UsageOptions_DepthTestOnly)) {
		desc->Format = HbGPUi_D3D_Image_Format_ToTypeless(info->format);
	} else {
		desc->Format = HbGPUi_D3D_Image_Format_ToTyped(info->format);
	}
	desc->SampleDesc.Count = 1 << info->samplesLog2;
	desc->SampleDesc.Quality = 0;
	desc->Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	desc->Flags = D3D12_RESOURCE_FLAG_NONE;
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

HbBool HbGPU_Image_InitWithInfo(HbGPU_Image * image, HbTextU8 const * name, HbGPU_Device * device,
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

#endif
