#include "HbGPUi_D3D.h"
#if HbGPU_Implementation_D3D
#include "HbFeedback.h"

HbBool HbGPU_CmdList_Init(HbGPU_CmdList * cmdList, HbTextU8 const * name, HbGPU_Device * device, HbGPU_CmdQueue queue) {
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
			HbNull, &IID_ID3D12CommandList, &cmdList->d3dSubmissionCommandList))) {
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
	ID3D12GraphicsCommandList_Reset(cmdList->d3dGraphicsCommandList, cmdList->d3dCommandAllocator, HbNull);
	cmdList->d3dCurrentHandleStore = handleStore;
	cmdList->d3dCurrentSamplerStore = samplerStore;
	ID3D12DescriptorHeap * descriptorHeaps[2];
	uint32_t descriptorHeapCount = 0;
	if (handleStore != HbNull) {
		descriptorHeaps[descriptorHeapCount++] = handleStore->d3dHeap;
	}
	if (samplerStore != HbNull) {
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

void HbGPU_CmdList_Submit(HbGPU_Device * device, HbGPU_CmdList * const * cmdLists, uint32_t cmdListCount) {
	if (cmdListCount == 0) {
		return;
	}
	if (cmdListCount == 1) {
		ID3D12GraphicsCommandList_Close(cmdLists[0]->d3dGraphicsCommandList);
		ID3D12CommandQueue * d3dQueue = device->d3dCommandQueues[cmdLists[0]->queue];
		ID3D12CommandQueue_ExecuteCommandLists(d3dQueue, 1, &cmdLists[0]->d3dSubmissionCommandList);
		return;
	}
	// In case there are command lists for different queues, generate arrays of lists to execute on each.
	ID3D12CommandList * * d3dCommandLists = HbStackAlloc(
			HbGPU_CmdQueue_QueueCount * cmdListCount * sizeof(ID3D12CommandList *));
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
	if (handleStore != HbNull) {
		descriptorHeaps[descriptorHeapCount++] = handleStore->d3dHeap;
	}
	if (samplerStore != HbNull) {
		descriptorHeaps[descriptorHeapCount++] = samplerStore->d3dHeap;
	}
	ID3D12GraphicsCommandList_SetDescriptorHeaps(cmdList->d3dGraphicsCommandList, descriptorHeapCount, descriptorHeaps);
}

void HbGPU_CmdList_Barrier(HbGPU_CmdList * cmdList, uint32_t count, HbGPU_CmdList_Barrier_Info const * infos) {
	if (count == 0) {
		return;
	}
	D3D12_RESOURCE_BARRIER * d3dBarriers = HbStackAlloc(count * sizeof(D3D12_RESOURCE_BARRIER));
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

void HbGPU_CmdList_BindSetLayout(HbGPU_CmdList * cmdList, HbGPU_BindingLayout * layout) {
	cmdList->d3dCurrentBindingLayout = layout;
	ID3D12GraphicsCommandList * d3dCommandList = cmdList->d3dGraphicsCommandList;
	void (STDMETHODCALLTYPE * d3dSetRootSignature)(ID3D12GraphicsCommandList *, ID3D12RootSignature *) = cmdList->d3dIsDrawing ?
			d3dCommandList->lpVtbl->SetGraphicsRootSignature : d3dCommandList->lpVtbl->SetComputeRootSignature;
	d3dSetRootSignature(d3dCommandList, layout->d3dRootSignature);
}

void HbGPU_CmdList_BindHandles(HbGPU_CmdList * cmdList, uint32_t bindingIndex, uint32_t handleOffsetInStore) {
	ID3D12GraphicsCommandList * d3dCommandList = cmdList->d3dGraphicsCommandList;
	void (STDMETHODCALLTYPE * d3dSetRootDescriptorTable)(ID3D12GraphicsCommandList *, UINT, D3D12_GPU_DESCRIPTOR_HANDLE) = cmdList->d3dIsDrawing ?
			d3dCommandList->lpVtbl->SetGraphicsRootDescriptorTable : d3dCommandList->lpVtbl->SetComputeRootDescriptorTable;
	d3dSetRootDescriptorTable(d3dCommandList, cmdList->d3dCurrentBindingLayout->d3dRootParameterIndices[bindingIndex],
			HbGPUi_D3D_HandleStore_GetGPUHandle(cmdList->d3dCurrentHandleStore, handleOffsetInStore));
}

void HbGPU_CmdList_BindSamplers(HbGPU_CmdList * cmdList, uint32_t bindingIndex, uint32_t samplerOffsetInStore) {
	uint32_t rootParameterIndex = cmdList->d3dCurrentBindingLayout->d3dRootParameterIndices[bindingIndex];
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

void HbGPU_CmdList_BindConstantBuffer(HbGPU_CmdList * cmdList, uint32_t bindingIndex, HbGPU_Buffer * buffer, uint32_t offset, uint32_t size) {
	ID3D12GraphicsCommandList * d3dCommandList = cmdList->d3dGraphicsCommandList;
	void (STDMETHODCALLTYPE * d3dSetRootConstantBufferView)(ID3D12GraphicsCommandList *, UINT, D3D12_GPU_VIRTUAL_ADDRESS) = cmdList->d3dIsDrawing ?
			d3dCommandList->lpVtbl->SetGraphicsRootConstantBufferView : d3dCommandList->lpVtbl->SetComputeRootConstantBufferView;
	d3dSetRootConstantBufferView(d3dCommandList, cmdList->d3dCurrentBindingLayout->d3dRootParameterIndices[bindingIndex], buffer->d3dGPUAddress + offset);
}

void HbGPU_CmdList_BindSmallConstants(HbGPU_CmdList * cmdList, uint32_t bindingIndex, void const * data, uint32_t sizeInDwords) {
	ID3D12GraphicsCommandList * d3dCommandList = cmdList->d3dGraphicsCommandList;
	void (STDMETHODCALLTYPE * d3dSetRoot32BitConstants)(ID3D12GraphicsCommandList *, UINT, UINT, void const *, UINT) = cmdList->d3dIsDrawing ?
			d3dCommandList->lpVtbl->SetGraphicsRoot32BitConstants : d3dCommandList->lpVtbl->SetComputeRoot32BitConstants;
	d3dSetRoot32BitConstants(d3dCommandList, cmdList->d3dCurrentBindingLayout->d3dRootParameterIndices[bindingIndex], sizeInDwords, data, 0);
}

void HbGPU_CmdList_DrawBegin(HbGPU_CmdList * cmdList, HbGPU_DrawPass_Info const * passInfo) {
	cmdList->d3dIsDrawing = HbTrue;
	cmdList->d3dCurrentDrawPass = *passInfo;
	ID3D12GraphicsCommandList * d3dCommandList = cmdList->d3dGraphicsCommandList;
	D3D12_CPU_DESCRIPTOR_HANDLE colorRTDescriptors[HbGPU_RT_MaxBound];
	for (uint32_t rtIndex = 0; rtIndex < passInfo->colorRTCount; ++rtIndex) {
		colorRTDescriptors[rtIndex] = passInfo->colorRTs[rtIndex].d3dHandle;
	}
	ID3D12GraphicsCommandList_OMSetRenderTargets(d3dCommandList, passInfo->colorRTCount, colorRTDescriptors, FALSE,
			passInfo->hasDepthStencilRT ? &passInfo->depthStencilRT.d3dHandle : HbNull);
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
			ID3D12GraphicsCommandList_ClearRenderTargetView(d3dCommandList, colorRT->d3dHandle, colorActions->clearValue.color, 0, HbNull);
			break;
		default:
			break;
		}
	}
	D3D12_CLEAR_FLAGS depthStencilClearFlags = (D3D12_CLEAR_FLAGS) 0;
	if (passInfo->hasDepthStencilRT) {
		switch (passInfo->depthActions.beginAction) {
		case HbGPU_DrawPass_BeginAction_Discard:
			discardRegion.FirstSubresource = HbGPUi_D3D_Image_Slice_ToSubresource(
					&passInfo->depthStencilRT.d3dImageRef.image->info, passInfo->depthStencilRT.d3dImageRef.slice);
			ID3D12GraphicsCommandList_DiscardResource(d3dCommandList, passInfo->depthStencilRT.d3dImageRef.image->d3dResource, &discardRegion);
			break;
		case HbGPU_DrawPass_BeginAction_Clear:
			depthStencilClearFlags |= D3D12_CLEAR_FLAG_DEPTH;
			break;
		default:
			break;
		}
		switch (passInfo->stencilActions.beginAction) {
		case HbGPU_DrawPass_BeginAction_Discard: {
			HbGPU_Image_Slice stencilSlice = passInfo->depthStencilRT.d3dImageRef.slice;
			stencilSlice.stencil = 1;
			discardRegion.FirstSubresource = HbGPUi_D3D_Image_Slice_ToSubresource(&passInfo->depthStencilRT.d3dImageRef.image->info, stencilSlice);
			ID3D12GraphicsCommandList_DiscardResource(d3dCommandList, passInfo->depthStencilRT.d3dImageRef.image->d3dResource, &discardRegion);
			break;
		} case HbGPU_DrawPass_BeginAction_Clear:
			depthStencilClearFlags |= D3D12_CLEAR_FLAG_DEPTH;
			break;
		default:
			break;
		}
		if (depthStencilClearFlags != 0) {
			ID3D12GraphicsCommandList_ClearDepthStencilView(d3dCommandList, passInfo->depthStencilRT.d3dHandle, depthStencilClearFlags,
					passInfo->depthActions.clearValue.depthStencil.depth, passInfo->stencilActions.clearValue.depthStencil.stencil, 0, HbNull);
		}
	}
}

void HbGPU_CmdList_DrawEnd(HbGPU_CmdList * cmdList) {
	ID3D12GraphicsCommandList * d3dCommandList = cmdList->d3dGraphicsCommandList;
	D3D12_DISCARD_REGION discardRegion = { .NumSubresources = 1 };
	for (uint32_t rtIndex = 0; cmdList->d3dCurrentDrawPass.colorRTCount; ++rtIndex) {
		HbGPU_RTReference const * colorRT = &cmdList->d3dCurrentDrawPass.colorRTs[rtIndex];
		HbGPU_DrawPass_Actions const * colorActions = &cmdList->d3dCurrentDrawPass.colorActions[rtIndex];
		if (colorActions->endAction == HbGPU_DrawPass_EndAction_ResolveStore || HbGPU_DrawPass_EndAction_ResolveDiscard) {
			ID3D12GraphicsCommandList_ResolveSubresource(d3dCommandList, colorActions->resolveImage->d3dResource,
					HbGPUi_D3D_Image_Slice_ToSubresource(&colorActions->resolveImage->info, colorActions->resolveSlice),
					colorRT->d3dImageRef.image->d3dResource,
					HbGPUi_D3D_Image_Slice_ToSubresource(&colorRT->d3dImageRef.image->info, colorRT->d3dImageRef.slice),
					HbGPUi_D3D_Image_Format_ToTyped(colorActions->resolveImage->info.format));
		}
		if (colorActions->endAction == HbGPU_DrawPass_EndAction_Discard || HbGPU_DrawPass_EndAction_ResolveDiscard) {
			discardRegion.FirstSubresource = HbGPUi_D3D_Image_Slice_ToSubresource(&colorRT->d3dImageRef.image->info, colorRT->d3dImageRef.slice);
			ID3D12GraphicsCommandList_DiscardResource(d3dCommandList, colorRT->d3dImageRef.image->d3dResource, &discardRegion);
		}
	}
	if (cmdList->d3dCurrentDrawPass.hasDepthStencilRT) {
		if (cmdList->d3dCurrentDrawPass.depthActions.endAction == HbGPU_DrawPass_EndAction_Discard) {
			discardRegion.FirstSubresource = HbGPUi_D3D_Image_Slice_ToSubresource(
					&cmdList->d3dCurrentDrawPass.depthStencilRT.d3dImageRef.image->info, cmdList->d3dCurrentDrawPass.depthStencilRT.d3dImageRef.slice);
			ID3D12GraphicsCommandList_DiscardResource(d3dCommandList,
					cmdList->d3dCurrentDrawPass.depthStencilRT.d3dImageRef.image->d3dResource, &discardRegion);
		}
		if (cmdList->d3dCurrentDrawPass.stencilActions.endAction == HbGPU_DrawPass_EndAction_Discard) {
			HbGPU_Image_Slice stencilSlice = cmdList->d3dCurrentDrawPass.depthStencilRT.d3dImageRef.slice;
			stencilSlice.stencil = 1;
			discardRegion.FirstSubresource = HbGPUi_D3D_Image_Slice_ToSubresource(
					&cmdList->d3dCurrentDrawPass.depthStencilRT.d3dImageRef.image->info, stencilSlice);
			ID3D12GraphicsCommandList_DiscardResource(d3dCommandList,
					cmdList->d3dCurrentDrawPass.depthStencilRT.d3dImageRef.image->d3dResource, &discardRegion);
		}
	}
	cmdList->d3dIsDrawing = HbFalse;
}

void HbGPU_CmdList_DrawSetViewport(HbGPU_CmdList * cmdList, float left, float top, float width, float height, float depthMin, float depthMax) {
	D3D12_VIEWPORT viewport = { .TopLeftX = left, .TopLeftY = top, .Width = width, .Height = height, .MinDepth = depthMin, .MaxDepth = depthMax };
	ID3D12GraphicsCommandList_RSSetViewports(cmdList->d3dGraphicsCommandList, 1, &viewport);
}

void HbGPU_CmdList_DrawSetScissor(HbGPU_CmdList * cmdList, int32_t left, int32_t top, uint32_t width, uint32_t height) {
	D3D12_RECT scissor = { .left = left, .top = top, .right = left + width, .bottom = top + height };
	ID3D12GraphicsCommandList_RSSetScissorRects(cmdList->d3dGraphicsCommandList, 1, &scissor);
}

void HbGPU_CmdList_DrawSetConfig(HbGPU_CmdList * cmdList, HbGPU_DrawConfig * config) {
	ID3D12GraphicsCommandList_SetPipelineState(cmdList->d3dGraphicsCommandList, config->d3dPipelineState);
}

#endif
