#include "HbLoad.h"

HbBool HbLoad_GPUCopier_Init(HbLoad_GPUCopier * copier, HbTextU8 * name, HbGPU_Device * device, HbMemory_Tag * tag,
		uint32_t submissionCount, uint32_t bufferSize) {
	if (submissionCount == 0 || bufferSize == 0) {
		return HbFalse;
	}

	size_t nameLength = 0;
	HbTextU8 * submissionFieldName = HbNull;
	size_t submissionFieldNameSize = 0;
	size_t submissionFieldNameSuffixOffset = 0;
	if (name != HbNull) {
		nameLength = HbTextU8_LengthElems(name);
		copier->name = HbMemory_TryAlloc(tag, nameLength + 1, HbFalse);
		if (copier->name != HbNull) {
			HbTextA_Copy(copier->name, nameLength + 1, name);
		}
		char const submissionFieldNamePrefix[] = "";
		// Used fields: fence (length 5), cmdList (7), buffer (6).
		submissionFieldNameSize = nameLength + HbText_Decimal_MaxLengthU32 + sizeof(".submissions[].cmdList");
		submissionFieldName = HbStackAlloc(submissionFieldNameSize);
		submissionFieldNameSuffixOffset = HbTextA_Copy(submissionFieldName, submissionFieldNameSize, name);
		submissionFieldNameSuffixOffset += HbTextA_CopyInto(submissionFieldName, submissionFieldNameSize, submissionFieldNameSuffixOffset, ".submissions[");
	} else {
		copier->name = HbNull;
	}

	copier->device = device;

	if (!HbParallel_Mutex_Init(&copier->queueMutex)) {
		HbMemory_Free(copier->name);
		return HbFalse;
	}
	copier->queueFree = HbNull;
	copier->submissions = HbMemory_TryAlloc(tag, submissionCount * sizeof(HbLoad_GPUCopier_Submission), HbFalse);
	if (copier->submissions == HbNull) {
		HbParallel_Mutex_Destroy(&copier->queueMutex);
		HbMemory_Free(copier->name);
		return HbFalse;
	}
	copier->submissionCount = submissionCount;
	for (uint32_t submissionIndex = 0; submissionIndex < submissionCount; ++submissionIndex) {
		HbLoad_GPUCopier_Submission * submission = &copier->submissions[submissionIndex];
		submission->copier = copier;
		submission->index = submissionIndex;
		submission->largeBufferUsed = HbFalse;
		HbBool submissionInitialized = HbTrue;
		if (submissionFieldName != HbNull) {
			HbTextA_Format(submissionFieldName + submissionFieldNameSuffixOffset, submissionFieldNameSize - submissionFieldNameSuffixOffset,
					"%u].fence", submissionIndex);
		}
		if (!HbGPU_Fence_Init(&submission->fence, submissionFieldName, device, HbGPU_CmdQueue_Copy)) {
			submissionInitialized = HbFalse;
		} else {
			if (submissionFieldName != HbNull) {
				HbTextA_Format(submissionFieldName + submissionFieldNameSuffixOffset, submissionFieldNameSize - submissionFieldNameSuffixOffset,
						"%u].cmdList", submissionIndex);
			}
			if (!HbGPU_CmdList_Init(&submission->cmdList, submissionFieldName, device, HbGPU_CmdQueue_Copy)) {
				HbGPU_Fence_Destroy(&submission->fence);
				submissionInitialized = HbFalse;
			} else {
				HbTextA_Format(submissionFieldName + submissionFieldNameSuffixOffset, submissionFieldNameSize - submissionFieldNameSuffixOffset,
						"%u].buffer", submissionIndex);
				if (!HbGPU_Buffer_Init(&submission->buffer, submissionFieldName, device, HbGPU_Buffer_Access_CPUToGPU, bufferSize, HbFalse, HbGPU_Buffer_Usage_CPUToGPU)) {
					HbGPU_CmdList_Destroy(&submission->cmdList);
					HbGPU_Fence_Destroy(&submission->fence);
					submissionInitialized = HbFalse;
				}
				submission->bufferMapping = HbGPU_Buffer_Map(&submission->buffer, 0, 0);
				if (submission->bufferMapping == HbNull) {
					HbGPU_Buffer_Destroy(&submission->buffer);
					HbGPU_CmdList_Destroy(&submission->cmdList);
					HbGPU_Fence_Destroy(&submission->fence);
					submissionInitialized = HbFalse;
				}
			}
		}
		if (!submissionInitialized) {
			for (uint32_t submissionPreviousIndex = 0; submissionPreviousIndex < submissionIndex; ++submissionPreviousIndex) {
				HbLoad_GPUCopier_Submission * submissionPrevious = &copier->submissions[submissionIndex - 1 - submissionPreviousIndex];
				HbGPU_Buffer_Unmap(&submissionPrevious->buffer, 0, 0);
				HbGPU_Buffer_Destroy(&submissionPrevious->buffer);
				HbGPU_CmdList_Destroy(&submissionPrevious->cmdList);
				HbGPU_Fence_Destroy(&submissionPrevious->fence);
			}
			HbMemory_Free(copier->submissions);
			HbParallel_Mutex_Destroy(&copier->queueMutex);
			HbMemory_Free(copier->name);
			return HbFalse;
		}
		submission->nextInQueue = copier->queueFree;
		copier->queueFree = submission;
	}
	copier->queueSubmittedStart = copier->queueSubmittedEnd = HbNull;

	return HbTrue;
}

static void HbLoad_GPUCopier_DestroyLargeBuffer(HbLoad_GPUCopier_Submission * submission) {
	if (submission->largeBufferUsed) {
		HbGPU_Buffer_Destroy(&submission->largeBuffer);
		submission->largeBufferUsed = HbFalse;
	}
}

void HbLoad_GPUCopier_Destroy(HbLoad_GPUCopier * copier) {
	while (HbLoad_GPUCopier_HandleCompletion(copier, HbNull, HbTrue)) {}
	for (uint32_t submissionIndex = 0; submissionIndex < copier->submissionCount; ++submissionIndex) {
		HbLoad_GPUCopier_Submission * submission = &copier->submissions[copier->submissionCount - 1 - submissionIndex];
		HbLoad_GPUCopier_DestroyLargeBuffer(submission);
		HbGPU_Buffer_Unmap(&submission->buffer, 0, submission->buffer.size);
		HbGPU_Buffer_Destroy(&submission->buffer);
		HbGPU_CmdList_Destroy(&submission->cmdList);
		HbGPU_Fence_Destroy(&submission->fence);
	}
	HbMemory_Free(copier->submissions);
	HbParallel_Mutex_Destroy(&copier->queueMutex);
	HbMemory_Free(copier->name);
}

HbBool HbLoad_GPUCopier_HandleCompletion(HbLoad_GPUCopier * copier, void * * requestDataOut, HbBool blockUntilComplete) {
	HbParallel_Mutex_Lock(&copier->queueMutex);

	HbLoad_GPUCopier_Submission * submission = copier->queueSubmittedStart;
	if (submission == HbNull) {
		HbParallel_Mutex_Unlock(&copier->queueMutex);
		return HbFalse;
	}

	if (!HbGPU_Fence_IsCrossed(&submission->fence)) {
		if (blockUntilComplete) {
			// Still remove it from the queue right now to release the mutex so waiting won't block other threads.
			copier->queueSubmittedStart = submission->nextInQueue;
			if (copier->queueSubmittedStart == HbNull) {
				copier->queueSubmittedEnd = HbNull;
			}
			HbParallel_Mutex_Unlock(&copier->queueMutex);
			HbGPU_Fence_Await(&submission->fence);
			HbParallel_Mutex_Lock(&copier->queueMutex);
		} else {
			HbParallel_Mutex_Unlock(&copier->queueMutex);
			return HbFalse;
		}
	}
	copier->queueSubmittedStart = submission->nextInQueue;
	if (copier->queueSubmittedStart == HbNull) {
		copier->queueSubmittedEnd = HbNull;
	}

	// Do post-load actions that depend on the values in the submission structure before moving the submission to the free queue.
	HbLoad_GPUCopier_DestroyLargeBuffer(submission);
	if (requestDataOut != HbNull) {
		*requestDataOut = submission->requestData;
	}

	submission->nextInQueue = copier->queueFree;
	copier->queueFree = submission;

	HbParallel_Mutex_Unlock(&copier->queueMutex);

	return HbTrue;
}

HbLoad_GPUCopier_Submission * HbLoad_GPUCopier_Request(HbLoad_GPUCopier * copier, void * requestData) {
	HbParallel_Mutex_Lock(&copier->queueMutex);
	HbLoad_GPUCopier_Submission * submission = copier->queueFree;
	if (submission != HbNull) {
		copier->queueFree = submission->nextInQueue;
		submission->requestData = requestData;
	}
	HbParallel_Mutex_Unlock(&copier->queueMutex);
	if (submission != HbNull) {
		HbGPU_CmdList_Begin(&submission->cmdList, HbNull, HbNull);
	}
	return submission;
}

void * HbLoad_GPUCopier_GetBuffer(HbLoad_GPUCopier_Submission * submission, uint32_t size, HbGPU_Buffer * * bufferOut, HbGPU_CmdList * * cmdListOut) {
	*cmdListOut = &submission->cmdList;
	HbLoad_GPUCopier_DestroyLargeBuffer(submission); // In case this is called twice for some reason.
	if (size < submission->buffer.size) {
		*bufferOut = &submission->buffer;
		return submission->bufferMapping;
	}
	HbTextU8 * largeBufferName = HbNull;
	HbTextU8 const * copierName = submission->copier->name;
	if (copierName != HbNull) {
		size_t largeBufferNameSize = HbTextU8_LengthElems(copierName) + HbText_Decimal_MaxLengthU32 + sizeof(".submissions[].largeBuffer");
		largeBufferName = HbStackAlloc(largeBufferNameSize);
		HbTextA_Format(largeBufferName, largeBufferNameSize, "%s.submissions[%u].largeBuffer", copierName, submission->index);
	}
	if (!HbGPU_Buffer_Init(&submission->largeBuffer, largeBufferName, submission->copier->device,
			HbGPU_Buffer_Access_CPUToGPU, size, HbFalse, HbGPU_Buffer_Usage_CPUToGPU)) {
		return HbNull;
	}
	void * mapping = HbGPU_Buffer_Map(&submission->largeBuffer, 0, 0);
	if (mapping != HbNull) {
		submission->largeBufferUsed = HbTrue;
		*bufferOut = &submission->largeBuffer;
	} else {
		HbGPU_Buffer_Destroy(&submission->largeBuffer);
	}
	return mapping;
}

void HbLoad_GPUCopier_Submit(HbLoad_GPUCopier_Submission * submission) {
	// Locking before to maintain the order in the queue the same as the actual GPU submission order.
	HbLoad_GPUCopier * copier = submission->copier;
	HbParallel_Mutex_Lock(&copier->queueMutex);
	HbGPU_CmdList * cmdList = &submission->cmdList;
	HbGPU_CmdList_Submit(1, &cmdList);
	HbGPU_Fence_Enqueue(&submission->fence);
	submission->nextInQueue = HbNull;
	if (copier->queueSubmittedEnd != HbNull) {
		copier->queueSubmittedEnd->nextInQueue = submission;
	} else {
		copier->queueSubmittedStart = submission;
	}
	copier->queueSubmittedEnd = submission;
	HbParallel_Mutex_Unlock(&copier->queueMutex);
}

void HbLoad_GPUCopier_Abort(HbLoad_GPUCopier_Submission * submission) {
	HbLoad_GPUCopier_DestroyLargeBuffer(submission);
	HbLoad_GPUCopier * copier = submission->copier;
	HbParallel_Mutex_Lock(&copier->queueMutex);
	submission->nextInQueue = copier->queueFree;
	copier->queueFree = submission;
	HbParallel_Mutex_Unlock(&copier->queueMutex);
}
