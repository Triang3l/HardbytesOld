#ifndef HbInclude_HbLoad
#define HbInclude_HbLoad
#include "HbGPU.h"
#include "HbMemory.h"
#include "HbParallel.h"
#include "HbText.h"

/*********************************
 * GPU copy queue multi-buffering
 *********************************/

typedef struct HbLoad_GPUCopier_Submission {
	struct HbLoad_GPUCopier * copier;
	uint32_t index; // For large buffer naming.
	HbGPU_Fence fence;
	HbGPU_CmdList cmdList;
	HbGPU_Buffer buffer;
	void * bufferMapping;
	HbGPU_Buffer largeBuffer; // Temporary buffer created if requested more than the size of the buffer.
	HbBool largeBufferUsed;
	struct HbLoad_GPUCopier_Submission * nextInQueue; // In queueFree or queueSubmitted.
	void * requestData;
} HbLoad_GPUCopier_Submission;

typedef struct HbLoad_GPUCopier {
	HbTextU8 * name; // Allocated from the tag - for large buffer creation.
	HbGPU_Device * device;
	uint32_t submissionCount;
	HbLoad_GPUCopier_Submission * submissions; // Allocated from the tag.
	// Submissions are moved between the queues depending on their state. Submissions currently being written (or force-awaited) are in neither.
	// The mutex protects both queueFree (for aborting from file loading threads) and queueSubmitted (for submitting from file loading threads).
	HbParallel_Mutex queueMutex;
	HbLoad_GPUCopier_Submission * queueFree; // Order doesn't matter (free stack).
	HbLoad_GPUCopier_Submission * queueSubmittedStart, * queueSubmittedEnd; // Order matters.
} HbLoad_GPUCopier;

HbBool HbLoad_GPUCopier_Init(HbLoad_GPUCopier * copier, HbTextU8 * name, HbGPU_Device * device, HbMemory_Tag * tag,
		uint32_t submissionCount, uint32_t bufferSize);
void HbLoad_GPUCopier_Destroy(HbLoad_GPUCopier * copier);
// Call occasionally to move slots of the completed submissions back to the free queue, and most importantly to mark the asset as loaded
// (by doing something for its requestData). blockUntilComplete is for rare cases, actually awaits completion, mostly for destruction.
HbBool HbLoad_GPUCopier_HandleCompletion(HbLoad_GPUCopier * copier, void * * requestDataOut, HbBool blockUntilComplete);

// To upload something, the following things need to be done:
// 1) Request a submission. This will check if there are free submissions, and will return one (and make it not free) if there is
// 2) GetBuffer when the size is known - will return the mapping of the buffer.
//    The buffer returned will be either pre-allocated or, if the data is too big, created as the "large buffer".
//    The latter may fail, so this case should be checked.
// 3) Submit when data and commands have been written.
// If an error happens after a request, Abort MUST be called eventually, otherwise the submission will not be returned to the free queue!
// requestData is a user data pointer that will be returned after the completion, to connect the submission with the actual asset.
HbLoad_GPUCopier_Submission * HbLoad_GPUCopier_Request(HbLoad_GPUCopier * copier, void * requestData);
// Returns the mapping if successful.
void * HbLoad_GPUCopier_GetBuffer(HbLoad_GPUCopier_Submission * submission, uint32_t size, HbGPU_Buffer * * bufferOut, HbGPU_CmdList * * cmdListOut);
void HbLoad_GPUCopier_Submit(HbLoad_GPUCopier_Submission * submission);
void HbLoad_GPUCopier_Abort(HbLoad_GPUCopier_Submission * submission); // Don't do this after submission - only to cancel an failed request.

#endif
