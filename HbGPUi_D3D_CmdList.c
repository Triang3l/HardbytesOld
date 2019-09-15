#include "HbGPUi_D3D.h"
#if HbGPU_Implementation_D3D
#include "HbFeedback.h"

HbBool HbGPU_CmdList_Init(HbGPU_CmdList * cmdList, HbTextU8 const * name, HbGPU_Device * device, HbGPU_CmdQueue queue) {
	cmdList->device = device;
	cmdList->queue = queue;
	D3D12_COMMAND_LIST_TYPE type;
	switch (queue) {
	case HbGPU_CmdQueue_Graphics:
		type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		break;
	case HbGPU_CmdQueue_Copy:
		type = D3D12_COMMAND_LIST_TYPE_COPY;
		break;
	default:
		return HbFalse;
	}
	if (FAILED(ID3D12Device_CreateCommandAllocator(device->d3dDevice, type,
			&IID_ID3D12CommandAllocator, &cmdList->d3dCommandAllocator))) {
		return HbFalse;
	}
	HbGPUi_D3D_SetSubObjectName(cmdList->d3dCommandAllocator,
			cmdList->d3dCommandAllocator->lpVtbl->SetName, name, "d3dCommandAllocator");
	if (FAILED(ID3D12Device_CreateCommandList(device->d3dDevice, 0, type, cmdList->d3dCommandAllocator,
			NULL, &IID_ID3D12CommandList, &cmdList->d3dSubmissionCommandList))) {
		ID3D12CommandAllocator_Release(cmdList->d3dCommandAllocator);
		return HbFalse;
	}
	if (FAILED(ID3D12CommandList_QueryInterface(cmdList->d3dSubmissionCommandList,
			&IID_ID3D12GraphicsCommandList, &cmdList->d3dGraphicsCommandList))) {
		ID3D12CommandList_Release(cmdList->d3dSubmissionCommandList);
		ID3D12CommandAllocator_Release(cmdList->d3dCommandAllocator);
		return HbFalse;
	}
	HbGPUi_D3D_SetObjectName(cmdList->d3dGraphicsCommandList, cmdList->d3dGraphicsCommandList->lpVtbl->SetName, name);
	// Command lists are created by D3D in an open state, but it will be opened later.
	ID3D12GraphicsCommandList_Close(cmdList->d3dGraphicsCommandList);
	return HbTrue;
}

void HbGPU_CmdList_Destroy(HbGPU_CmdList * cmdList) {
	ID3D12GraphicsCommandList_Release(cmdList->d3dGraphicsCommandList);
	ID3D12CommandList_Release(cmdList->d3dSubmissionCommandList);
	ID3D12CommandAllocator_Release(cmdList->d3dCommandAllocator);
}

void HbGPU_CmdList_Begin(HbGPU_CmdList * cmdList, HbGPU_HandleStore * handleStore, HbGPU_SamplerStore * samplerStore) {
	ID3D12CommandAllocator_Reset(cmdList->d3dCommandAllocator);
	ID3D12GraphicsCommandList_Reset(cmdList->d3dGraphicsCommandList, cmdList->d3dCommandAllocator, NULL);
	cmdList->d3dCurrentHandleStore = handleStore;
	cmdList->d3dCurrentSamplerStore = samplerStore;
	ID3D12DescriptorHeap * descriptorHeaps[2];
	uint32_t descriptorHeapCount = 0;
	if (handleStore != NULL) {
		descriptorHeaps[descriptorHeapCount++] = handleStore->d3dHeap;
	}
	if (samplerStore != NULL) {
		descriptorHeaps[descriptorHeapCount++] = samplerStore->d3dHeap;
	}
	if (descriptorHeapCount != 0) {
		ID3D12GraphicsCommandList_SetDescriptorHeaps(cmdList->d3dGraphicsCommandList, descriptorHeapCount, descriptorHeaps);
	}
	cmdList->d3dIsDrawing = HbFalse;
}

void HbGPU_CmdList_Abort(HbGPU_CmdList * cmdList) {
	ID3D12GraphicsCommandList_Close(cmdList->d3dGraphicsCommandList);
}

void HbGPU_CmdList_Submit(uint32_t cmdListCount, HbGPU_CmdList * const * cmdLists) {
	if (cmdListCount == 0) {
		return;
	}
	HbGPU_Device * device = cmdLists[0]->device;
	if (cmdListCount == 1) {
		ID3D12GraphicsCommandList_Close(cmdLists[0]->d3dGraphicsCommandList);
		ID3D12CommandQueue * d3dQueue = device->d3dCommandQueues[cmdLists[0]->queue];
		ID3D12CommandQueue_ExecuteCommandLists(d3dQueue, 1, &cmdLists[0]->d3dSubmissionCommandList);
		return;
	}
	// In case there are command lists for different queues, generate arrays of lists to execute on each.
	ID3D12CommandList * * d3dCommandLists = HbStackAlloc(
			ID3D12CommandList *, HbGPU_CmdQueue_QueueCount * cmdListCount);
	uint32_t queueCmdListCounts[HbGPU_CmdQueue_QueueCount] = { 0 };
	for (uint32_t cmdListIndex = 0; cmdListIndex < cmdListCount; ++cmdListIndex) {
		HbGPU_CmdList * cmdList = cmdLists[cmdListIndex];
		ID3D12GraphicsCommandList_Close(cmdList->d3dGraphicsCommandList);
		d3dCommandLists[cmdList->queue * cmdListCount + queueCmdListCounts[cmdList->queue]++] =
				cmdList->d3dSubmissionCommandList;
	}
	for (uint32_t queueIndex = 0; queueIndex < HbGPU_CmdQueue_QueueCount; ++queueIndex) {
		if (queueCmdListCounts[queueIndex] != 0) {
			ID3D12CommandQueue * d3dQueue = device->d3dCommandQueues[queueIndex];
			ID3D12CommandQueue_ExecuteCommandLists(d3dQueue, queueCmdListCounts[queueIndex],
					&d3dCommandLists[queueIndex * cmdListCount]);
		}
	}
}

void HbGPU_CmdList_SetBindingStores(HbGPU_CmdList * cmdList, HbGPU_HandleStore * handleStore, HbGPU_SamplerStore * samplerStore) {
	cmdList->d3dCurrentHandleStore = handleStore;
	cmdList->d3dCurrentSamplerStore = samplerStore;
	ID3D12DescriptorHeap * descriptorHeaps[2];
	uint32_t descriptorHeapCount = 0;
	if (handleStore != NULL) {
		descriptorHeaps[descriptorHeapCount++] = handleStore->d3dHeap;
	}
	if (samplerStore != NULL) {
		descriptorHeaps[descriptorHeapCount++] = samplerStore->d3dHeap;
	}
	ID3D12GraphicsCommandList_SetDescriptorHeaps(cmdList->d3dGraphicsCommandList, descriptorHeapCount, descriptorHeaps);
}

void HbGPU_CmdList_Barrier(HbGPU_CmdList * cmdList, uint32_t count, HbGPU_CmdList_Barrier_Info const * infos) {
	if (count == 0) {
		return;
	}
	D3D12_RESOURCE_BARRIER * d3dBarriers = HbStackAlloc(D3D12_RESOURCE_BARRIER, count);
	uint32_t d3dBarrierCount = 0; // Can be smaller, for instance, in case old state == new state.
	for (uint32_t barrierIndex = 0; barrierIndex < count; ++barrierIndex) {
		HbGPU_CmdList_Barrier_Info const * info = &infos[barrierIndex];
		D3D12_RESOURCE_BARRIER * d3dBarrier = &d3dBarriers[d3dBarrierCount];
		switch (info->type) {
		case HbGPU_CmdList_Barrier_Type_BufferUsageSwitch:
			d3dBarrier->Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			d3dBarrier->Transition.pResource = info->barrier.bufferUsageSwitch.buffer->d3dResource;
			d3dBarrier->Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			d3dBarrier->Transition.StateBefore = HbGPUi_D3D_Buffer_Usage_ToStates(info->barrier.bufferUsageSwitch.usageOld);
			d3dBarrier->Transition.StateAfter = HbGPUi_D3D_Buffer_Usage_ToStates(info->barrier.bufferUsageSwitch.usageNew);
			if (d3dBarrier->Transition.StateBefore == d3dBarrier->Transition.StateAfter) {
				continue;
			}
			break;
		case HbGPU_CmdList_Barrier_Type_ImageUsageSwitch:
			d3dBarrier->Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			d3dBarrier->Transition.pResource = info->barrier.imageUsageSwitch.image->d3dResource;
			if (info->barrier.imageUsageSwitch.isSingleSlice) {
				d3dBarrier->Transition.Subresource = HbGPUi_D3D_Image_Slice_ToSubresource(
						&info->barrier.imageUsageSwitch.image->info, info->barrier.imageUsageSwitch.slice);
			} else {
				d3dBarrier->Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			}
			d3dBarrier->Transition.StateBefore = HbGPUi_D3D_Image_Usage_ToStates(info->barrier.imageUsageSwitch.usageOld);
			d3dBarrier->Transition.StateAfter = HbGPUi_D3D_Image_Usage_ToStates(info->barrier.imageUsageSwitch.usageNew);
			if (d3dBarrier->Transition.StateBefore == d3dBarrier->Transition.StateAfter) {
				continue;
			}
			break;
		case HbGPU_CmdList_Barrier_Type_BufferEditCommit:
			d3dBarrier->Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
			d3dBarrier->UAV.pResource = info->barrier.bufferUsageSwitch.buffer->d3dResource;
			break;
		case HbGPU_CmdList_Barrier_Type_ImageEditCommit:
			d3dBarrier->Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
			d3dBarrier->UAV.pResource = info->barrier.imageUsageSwitch.image->d3dResource;
			break;
		default:
			HbFeedback_Assert(HbFalse, "HbGPU_CmdList_Barrier", "Unknown barrier type %u.", (uint32_t) info->type);
			continue;
		}
		switch (info->time) {
		case HbGPU_CmdList_Barrier_Time_Start:
			d3dBarrier->Flags = D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY;
			break;
		case HbGPU_CmdList_Barrier_Time_Finish:
			d3dBarrier->Flags = D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;
			break;
		default:
			d3dBarrier->Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			break;
		}
		++d3dBarrierCount;
	}
	if (d3dBarrierCount == 0) {
		return;
	}
	ID3D12GraphicsCommandList_ResourceBarrier(cmdList->d3dGraphicsCommandList, d3dBarrierCount, d3dBarriers);
}

void HbGPU_CmdList_Bind_SetLayout(HbGPU_CmdList * cmdList, HbGPU_BindingLayout * layout) {
	cmdList->d3dCurrentBindingLayout = layout;
	ID3D12GraphicsCommandList * d3dCommandList = cmdList->d3dGraphicsCommandList;
	void (STDMETHODCALLTYPE * d3dSetRootSignature)(ID3D12GraphicsCommandList *, ID3D12RootSignature *) = cmdList->d3dIsDrawing ?
			d3dCommandList->lpVtbl->SetGraphicsRootSignature : d3dCommandList->lpVtbl->SetComputeRootSignature;
	d3dSetRootSignature(d3dCommandList, layout->d3dRootSignature);
}

void HbGPU_CmdList_Bind_Handles(HbGPU_CmdList * cmdList, uint32_t bindingIndex, uint32_t handleOffsetInStore) {
	ID3D12GraphicsCommandList * d3dCommandList = cmdList->d3dGraphicsCommandList;
	void (STDMETHODCALLTYPE * d3dSetRootDescriptorTable)(ID3D12GraphicsCommandList *, UINT, D3D12_GPU_DESCRIPTOR_HANDLE) = cmdList->d3dIsDrawing ?
			d3dCommandList->lpVtbl->SetGraphicsRootDescriptorTable : d3dCommandList->lpVtbl->SetComputeRootDescriptorTable;
	d3dSetRootDescriptorTable(d3dCommandList, cmdList->d3dCurrentBindingLayout->d3dRootParameterIndexes[bindingIndex],
			HbGPUi_D3D_HandleStore_GetGPUHandle(cmdList->d3dCurrentHandleStore, handleOffsetInStore));
}

void HbGPU_CmdList_Bind_Samplers(HbGPU_CmdList * cmdList, uint32_t bindingIndex, uint32_t samplerOffsetInStore) {
	uint32_t rootParameterIndex = cmdList->d3dCurrentBindingLayout->d3dRootParameterIndexes[bindingIndex];
	if (rootParameterIndex == UINT32_MAX) {
		// Static samplers here.
		return;
	}
	ID3D12GraphicsCommandList * d3dCommandList = cmdList->d3dGraphicsCommandList;
	void (STDMETHODCALLTYPE * d3dSetRootDescriptorTable)(ID3D12GraphicsCommandList *, UINT, D3D12_GPU_DESCRIPTOR_HANDLE) = cmdList->d3dIsDrawing ?
			d3dCommandList->lpVtbl->SetGraphicsRootDescriptorTable : d3dCommandList->lpVtbl->SetComputeRootDescriptorTable;
	d3dSetRootDescriptorTable(d3dCommandList, rootParameterIndex,
			HbGPUi_D3D_SamplerStore_GetGPUHandle(cmdList->d3dCurrentSamplerStore, samplerOffsetInStore));
}

void HbGPU_CmdList_Bind_ConstantBuffer(HbGPU_CmdList * cmdList, uint32_t bindingIndex, HbGPU_Buffer * buffer, uint32_t offset, uint32_t size) {
	ID3D12GraphicsCommandList * d3dCommandList = cmdList->d3dGraphicsCommandList;
	void (STDMETHODCALLTYPE * d3dSetRootConstantBufferView)(ID3D12GraphicsCommandList *, UINT, D3D12_GPU_VIRTUAL_ADDRESS) = cmdList->d3dIsDrawing ?
			d3dCommandList->lpVtbl->SetGraphicsRootConstantBufferView : d3dCommandList->lpVtbl->SetComputeRootConstantBufferView;
	d3dSetRootConstantBufferView(d3dCommandList, cmdList->d3dCurrentBindingLayout->d3dRootParameterIndexes[bindingIndex], buffer->d3dGPUAddress + offset);
}

void HbGPU_CmdList_Bind_SmallConstants(HbGPU_CmdList * cmdList, uint32_t bindingIndex, void const * data, uint32_t sizeInDwords) {
	ID3D12GraphicsCommandList * d3dCommandList = cmdList->d3dGraphicsCommandList;
	void (STDMETHODCALLTYPE * d3dSetRoot32BitConstants)(ID3D12GraphicsCommandList *, UINT, UINT, void const *, UINT) = cmdList->d3dIsDrawing ?
			d3dCommandList->lpVtbl->SetGraphicsRoot32BitConstants : d3dCommandList->lpVtbl->SetComputeRoot32BitConstants;
	d3dSetRoot32BitConstants(d3dCommandList, cmdList->d3dCurrentBindingLayout->d3dRootParameterIndexes[bindingIndex], sizeInDwords, data, 0);
}

void HbGPU_CmdList_Draw_Begin(HbGPU_CmdList * cmdList, HbGPU_DrawPass_Info const * passInfo) {
	cmdList->d3dIsDrawing = HbTrue;
	cmdList->d3dCurrentDrawPass = *passInfo;
	ID3D12GraphicsCommandList * d3dCommandList = cmdList->d3dGraphicsCommandList;
	D3D12_CPU_DESCRIPTOR_HANDLE colorRTDescriptors[HbGPU_RT_MaxBound];
	for (uint32_t rtIndex = 0; rtIndex < passInfo->colorRTCount; ++rtIndex) {
		colorRTDescriptors[rtIndex] = passInfo->colorRTs[rtIndex].d3dHandle;
	}
	ID3D12GraphicsCommandList_OMSetRenderTargets(d3dCommandList, passInfo->colorRTCount, colorRTDescriptors, FALSE,
			passInfo->hasDepthStencilRT ? &passInfo->depthStencilRT.d3dHandle : NULL);
	D3D12_DISCARD_REGION discardRegion = { .NumSubresources = 1 };
	for (uint32_t rtIndex = 0; rtIndex < passInfo->colorRTCount; ++rtIndex) {
		HbGPU_RTReference const * colorRT = &passInfo->colorRTs[rtIndex];
		HbGPU_DrawPass_Actions const * colorActions = &passInfo->colorActions[rtIndex];
		switch (colorActions->beginAction) {
		case HbGPU_DrawPass_BeginAction_Discard:
			discardRegion.FirstSubresource = HbGPUi_D3D_Image_Slice_ToSubresource(&colorRT->d3dImageRef.image->info, colorRT->d3dImageRef.slice);
			ID3D12GraphicsCommandList_DiscardResource(d3dCommandList, colorRT->d3dImageRef.image->d3dResource, &discardRegion);
			break;
		case HbGPU_DrawPass_BeginAction_Clear:
			ID3D12GraphicsCommandList_ClearRenderTargetView(d3dCommandList, colorRT->d3dHandle, colorActions->clearValue.color, 0, NULL);
			break;
		default:
			break;
		}
	}
	D3D12_CLEAR_FLAGS depthStencilClearFlags = (D3D12_CLEAR_FLAGS) 0;
	if (passInfo->hasDepthStencilRT) {
		HbGPU_Image_SliceReference const * depthStencilRef = &passInfo->depthStencilRT.d3dImageRef;
		switch (passInfo->depthActions.beginAction) {
		case HbGPU_DrawPass_BeginAction_Discard:
			discardRegion.FirstSubresource = HbGPUi_D3D_Image_Slice_ToSubresource(&depthStencilRef->image->info, depthStencilRef->slice);
			ID3D12GraphicsCommandList_DiscardResource(d3dCommandList, depthStencilRef->image->d3dResource, &discardRegion);
			break;
		case HbGPU_DrawPass_BeginAction_Clear:
			depthStencilClearFlags |= D3D12_CLEAR_FLAG_DEPTH;
			break;
		default:
			break;
		}
		if (HbGPU_Image_Format_HasStencil(depthStencilRef->image->info.format)) {
			switch (passInfo->stencilActions.beginAction) {
			case HbGPU_DrawPass_BeginAction_Discard:
				{
					HbGPU_Image_Slice stencilSlice = depthStencilRef->slice;
					stencilSlice.stencil = 1;
					discardRegion.FirstSubresource = HbGPUi_D3D_Image_Slice_ToSubresource(&depthStencilRef->image->info, stencilSlice);
					ID3D12GraphicsCommandList_DiscardResource(d3dCommandList, depthStencilRef->image->d3dResource, &discardRegion);
				}
				break;
			case HbGPU_DrawPass_BeginAction_Clear:
				depthStencilClearFlags |= D3D12_CLEAR_FLAG_DEPTH;
				break;
			default:
				break;
			}
		}
		if (depthStencilClearFlags != 0) {
			ID3D12GraphicsCommandList_ClearDepthStencilView(d3dCommandList, passInfo->depthStencilRT.d3dHandle, depthStencilClearFlags,
					passInfo->depthActions.clearValue.depthStencil.depth, passInfo->stencilActions.clearValue.depthStencil.stencil, 0, NULL);
		}
	}
}

void HbGPU_CmdList_Draw_End(HbGPU_CmdList * cmdList) {
	ID3D12GraphicsCommandList * d3dCommandList = cmdList->d3dGraphicsCommandList;
	D3D12_DISCARD_REGION discardRegion = { .NumSubresources = 1 };
	for (uint32_t rtIndex = 0; rtIndex < cmdList->d3dCurrentDrawPass.colorRTCount; ++rtIndex) {
		HbGPU_RTReference const * colorRT = &cmdList->d3dCurrentDrawPass.colorRTs[rtIndex];
		HbGPU_DrawPass_Actions const * colorActions = &cmdList->d3dCurrentDrawPass.colorActions[rtIndex];
		HbGPU_DrawPass_EndAction colorEndAction = colorActions->endAction;
		switch (colorActions->endAction) {
		case HbGPU_DrawPass_EndAction_Discard:
			discardRegion.FirstSubresource = HbGPUi_D3D_Image_Slice_ToSubresource(&colorRT->d3dImageRef.image->info, colorRT->d3dImageRef.slice);
			ID3D12GraphicsCommandList_DiscardResource(d3dCommandList, colorRT->d3dImageRef.image->d3dResource, &discardRegion);
			break;
		case HbGPU_DrawPass_EndAction_ResolveStore:
		case HbGPU_DrawPass_EndAction_ResolveDiscard:
			ID3D12GraphicsCommandList_ResolveSubresource(d3dCommandList, colorActions->resolveImage->d3dResource,
					HbGPUi_D3D_Image_Slice_ToSubresource(&colorActions->resolveImage->info, colorActions->resolveSlice),
					colorRT->d3dImageRef.image->d3dResource,
					HbGPUi_D3D_Image_Slice_ToSubresource(&colorRT->d3dImageRef.image->info, colorRT->d3dImageRef.slice),
					HbGPUi_D3D_Image_Format_ToTyped(colorActions->resolveImage->info.format));
			// Don't discard here, discarding is only allowed in RENDER_TARGET state, not RESOLVE_SOURCE.
			break;
		default:
			break;
		}
	}
	if (cmdList->d3dCurrentDrawPass.hasDepthStencilRT) {
		HbGPU_Image_SliceReference const * depthStencilRef = &cmdList->d3dCurrentDrawPass.depthStencilRT.d3dImageRef;
		if (cmdList->d3dCurrentDrawPass.depthActions.endAction == HbGPU_DrawPass_EndAction_Discard) {
			discardRegion.FirstSubresource = HbGPUi_D3D_Image_Slice_ToSubresource(&depthStencilRef->image->info, depthStencilRef->slice);
			ID3D12GraphicsCommandList_DiscardResource(d3dCommandList, depthStencilRef->image->d3dResource, &discardRegion);
		}
		if (HbGPU_Image_Format_HasStencil(depthStencilRef->image->info.format)) {
			if (cmdList->d3dCurrentDrawPass.stencilActions.endAction == HbGPU_DrawPass_EndAction_Discard) {
				HbGPU_Image_Slice stencilSlice = depthStencilRef->slice;
				stencilSlice.stencil = 1;
				discardRegion.FirstSubresource = HbGPUi_D3D_Image_Slice_ToSubresource(&depthStencilRef->image->info, stencilSlice);
				ID3D12GraphicsCommandList_DiscardResource(d3dCommandList, depthStencilRef->image->d3dResource, &discardRegion);
			}
		}
	}
	cmdList->d3dIsDrawing = HbFalse;
}

void HbGPU_CmdList_Draw_SetViewport(HbGPU_CmdList * cmdList, float left, float top, float width, float height, float depthMin, float depthMax) {
	D3D12_VIEWPORT viewport = { .TopLeftX = left, .TopLeftY = top, .Width = width, .Height = height, .MinDepth = depthMin, .MaxDepth = depthMax };
	ID3D12GraphicsCommandList_RSSetViewports(cmdList->d3dGraphicsCommandList, 1, &viewport);
}

void HbGPU_CmdList_Draw_SetScissor(HbGPU_CmdList * cmdList, int32_t left, int32_t top, uint32_t width, uint32_t height) {
	D3D12_RECT scissor = { .left = left, .top = top, .right = left + width, .bottom = top + height };
	ID3D12GraphicsCommandList_RSSetScissorRects(cmdList->d3dGraphicsCommandList, 1, &scissor);
}

void HbGPU_CmdList_Draw_SetStencilReference(HbGPU_CmdList * cmdList, uint8_t reference) {
	ID3D12GraphicsCommandList_OMSetStencilRef(cmdList->d3dGraphicsCommandList, reference);
}

void HbGPU_CmdList_Draw_SetBlendConstantFactor(HbGPU_CmdList * cmdList, float const factor[4]) {
	ID3D12GraphicsCommandList_OMSetBlendFactor(cmdList->d3dGraphicsCommandList, factor);
}

void HbGPU_CmdList_Draw_SetConfig(HbGPU_CmdList * cmdList, HbGPU_DrawConfig * config) {
	cmdList->d3dCurrentDrawConfig = config;
	ID3D12GraphicsCommandList_SetPipelineState(cmdList->d3dGraphicsCommandList, config->d3dPipelineState);
}

void HbGPU_CmdList_Draw_SetPrimitive(HbGPU_CmdList * cmdList, HbGPU_CmdList_Primitive primitive) {
	D3D_PRIMITIVE_TOPOLOGY d3dTopology;
	switch (primitive) {
	case HbGPU_CmdList_Primitive_TriangleList: d3dTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; break;
	case HbGPU_CmdList_Primitive_TriangleStrip: d3dTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP; break;
	case HbGPU_CmdList_Primitive_LineList: d3dTopology = D3D_PRIMITIVE_TOPOLOGY_LINELIST; break;
	case HbGPU_CmdList_Primitive_LineStrip: d3dTopology = D3D10_PRIMITIVE_TOPOLOGY_LINESTRIP; break;
	case HbGPU_CmdList_Primitive_PointList: d3dTopology = D3D_PRIMITIVE_TOPOLOGY_POINTLIST; break;
	default:
		HbFeedback_Assert(HbFalse, "HbGPU_CmdList_Draw_SetPrimitive", "Unknwown primitive type %u.", (uint32_t) primitive);
		return;
	}
	ID3D12GraphicsCommandList_IASetPrimitiveTopology(cmdList->d3dGraphicsCommandList, d3dTopology);
}

void HbGPU_CmdList_Draw_SetVertexStreams(HbGPU_CmdList * cmdList,
		uint32_t firstStream, uint32_t streamCount, HbGPU_CmdList_VertexStream const * streams) {
	D3D12_VERTEX_BUFFER_VIEW d3dVertexBuffers[D3D12_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
	for (uint32_t streamIndex = 0; streamIndex < streamCount; ++streamIndex) {
		HbGPU_CmdList_VertexStream const * stream = &streams[streamIndex];
		D3D12_VERTEX_BUFFER_VIEW * d3dVertexBuffer = &d3dVertexBuffers[streamIndex];
		d3dVertexBuffer->BufferLocation = stream->buffer->d3dGPUAddress + stream->offset;
		d3dVertexBuffer->SizeInBytes = stream->size;
		d3dVertexBuffer->StrideInBytes = cmdList->d3dCurrentDrawConfig->d3dVertexStreamStridesInDwords[firstStream + streamIndex] * sizeof(uint32_t);
	}
	ID3D12GraphicsCommandList_IASetVertexBuffers(cmdList->d3dGraphicsCommandList, firstStream, streamCount, d3dVertexBuffers);
}

void HbGPU_CmdList_Draw_SetIndexes(HbGPU_CmdList * cmdList, HbGPU_Buffer * buffer, uint32_t offset, uint32_t sizeInBytes) {
	D3D12_INDEX_BUFFER_VIEW d3dIndexBuffer = {
		.BufferLocation = buffer->d3dGPUAddress + offset,
		.SizeInBytes = sizeInBytes,
		.Format = sizeof(HbGPU_Vertex_Index) >= sizeof(uint32_t) ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT,
	};
	ID3D12GraphicsCommandList_IASetIndexBuffer(cmdList->d3dGraphicsCommandList, &d3dIndexBuffer);
}

void HbGPU_CmdList_Draw_NonIndexed(HbGPU_CmdList * cmdList, uint32_t vertexCount, int32_t vertexIDBase,
		uint32_t instanceCount, uint32_t instanceBase) {
	ID3D12GraphicsCommandList_DrawInstanced(cmdList->d3dGraphicsCommandList, vertexCount, instanceCount, vertexIDBase, instanceBase);
}

void HbGPU_CmdList_Draw_Indexed(HbGPU_CmdList * cmdList, uint32_t indexCount, uint32_t indexFirst, int32_t vertexIDBase,
		uint32_t instanceCount, uint32_t instanceBase) {
	ID3D12GraphicsCommandList_DrawIndexedInstanced(cmdList->d3dGraphicsCommandList,
			indexCount, instanceCount, indexFirst, vertexIDBase, instanceBase);
}

void HbGPU_CmdList_Compute_Begin(HbGPU_CmdList * cmdList) {
	HbFeedback_Assert(!cmdList->d3dIsDrawing, "HbGPU_CmdList_Compute_Begin", "Drawing pass must be ended before computing.");
	cmdList->d3dIsDrawing = HbFalse;
}

void HbGPU_CmdList_Compute_End(HbGPU_CmdList * cmdList) {}

void HbGPU_CmdList_Compute_SetConfig(HbGPU_CmdList * cmdList, HbGPU_ComputeConfig * config) {
	ID3D12GraphicsCommandList_SetPipelineState(cmdList->d3dGraphicsCommandList, config->d3dPipelineState);
}

void HbGPU_CmdList_Compute_Dispatch(HbGPU_CmdList * cmdList, uint32_t groupsX, uint32_t groupsY, uint32_t groupsZ) {
	ID3D12GraphicsCommandList_Dispatch(cmdList->d3dGraphicsCommandList, groupsX, groupsY, groupsZ);
}

void HbGPU_CmdList_Copy_Begin(HbGPU_CmdList * cmdList) {}

void HbGPU_CmdList_Copy_End(HbGPU_CmdList * cmdList) {}

void HbGPU_CmdList_Copy_BufferXBuffer(HbGPU_CmdList * cmdList, HbGPU_Buffer * target, uint32_t targetOffset,
		HbGPU_Buffer * source, uint32_t sourceOffset, uint32_t size) {
	ID3D12GraphicsCommandList_CopyBufferRegion(cmdList->d3dGraphicsCommandList, target->d3dResource, targetOffset, source->d3dResource, sourceOffset, size);
}

void HbGPU_CmdList_Copy_ImageXBuffer(HbGPU_CmdList * cmdList, HbBool toBuffer,
		HbGPU_Image * image, HbGPU_Image_Slice imageSlice, uint32_t imageX, uint32_t imageY, uint32_t imageZ,
		HbGPU_Buffer * buffer, uint32_t bufferOffset, uint32_t bufferRowPitchBytes, uint32_t buffer3DLayerPitchRows,
		uint32_t width, uint32_t height, uint32_t depth) {
	D3D12_TEXTURE_COPY_LOCATION imageLocation = {
		.pResource = image->d3dResource,
		.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
		.SubresourceIndex = HbGPUi_D3D_Image_Slice_ToSubresource(&image->info, imageSlice),
	};
	D3D12_TEXTURE_COPY_LOCATION bufferLocation = {
		.pResource = buffer->d3dResource,
		.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT,
		.PlacedFootprint.Offset = bufferOffset,
		.PlacedFootprint.Footprint.Format = HbGPUi_D3D_Image_Format_ToCopy(image->info.format, imageSlice.stencil),
		.PlacedFootprint.Footprint.Width = width,
		.PlacedFootprint.Footprint.Height = buffer3DLayerPitchRows,
		.PlacedFootprint.Footprint.Depth = depth,
		.PlacedFootprint.Footprint.RowPitch = bufferRowPitchBytes,
	};
	if (HbGPU_Image_Format_Is4x4(image->info.format)) {
		bufferLocation.PlacedFootprint.Footprint.Width = HbAlignU32(bufferLocation.PlacedFootprint.Footprint.Width, 4);
		bufferLocation.PlacedFootprint.Footprint.Height <<= 2;
	}
	D3D12_BOX sourceBox;
	if (toBuffer) {
		sourceBox.left = imageX;
		sourceBox.top = imageY;
		sourceBox.front = imageZ;
	} else {
		sourceBox.left = sourceBox.top = sourceBox.front = 0;
	}
	sourceBox.right = sourceBox.left + width;
	sourceBox.bottom = sourceBox.top + height;
	sourceBox.back = sourceBox.front + depth;
	if (toBuffer) {
		ID3D12GraphicsCommandList_CopyTextureRegion(cmdList->d3dGraphicsCommandList, &bufferLocation, 0, 0, 0, &imageLocation, &sourceBox);
	} else {
		ID3D12GraphicsCommandList_CopyTextureRegion(cmdList->d3dGraphicsCommandList, &imageLocation, imageX, imageY, imageZ, &bufferLocation, &sourceBox);
	}
}

void HbGPU_CmdList_Copy_ImageXImage(HbGPU_CmdList * cmdList,
		HbGPU_Image * target, HbGPU_Image_Slice targetSlice, uint32_t targetX, uint32_t targetY, uint32_t targetZ,
		HbGPU_Image * source, HbGPU_Image_Slice sourceSlice, uint32_t sourceX, uint32_t sourceY, uint32_t sourceZ,
		uint32_t width, uint32_t height, uint32_t depth) {
	D3D12_TEXTURE_COPY_LOCATION targetLocation = {
		.pResource = target->d3dResource,
		.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
		.SubresourceIndex = HbGPUi_D3D_Image_Slice_ToSubresource(&target->info, targetSlice),
	};
	D3D12_TEXTURE_COPY_LOCATION sourceLocation = {
		.pResource = source->d3dResource,
		.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
		.SubresourceIndex = HbGPUi_D3D_Image_Slice_ToSubresource(&source->info, sourceSlice),
	};
	if (target->info.samplesLog2 > 0) {
		// Only full-image copies with MSAA.
		ID3D12GraphicsCommandList_CopyTextureRegion(cmdList->d3dGraphicsCommandList, &targetLocation, 0, 0, 0, &sourceLocation, NULL);
	} else {
		D3D12_BOX sourceBox;
		sourceBox.left = sourceX;
		sourceBox.top = sourceY;
		sourceBox.front = sourceZ;
		sourceBox.right = sourceBox.left + width;
		sourceBox.bottom = sourceBox.top + height;
		sourceBox.back = sourceBox.front + depth;
		ID3D12GraphicsCommandList_CopyTextureRegion(cmdList->d3dGraphicsCommandList, &targetLocation, targetX, targetY, targetZ, &sourceLocation, &sourceBox);
	}
}

#endif
