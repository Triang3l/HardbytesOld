#include "HbGPU.h"
#if HbGPU_Implementation_D3D
#include "HbFeedback.h"
#if HbFeedback_DebugBuild
#define USE_PIX
#include <pix3.h>
#endif

void HbGPU_CmdList_Feedback_BeginNameScope(HbGPU_CmdList * cmdList, char const * name, float r, float g, float b) {
	#ifdef PIX_EVENTS_ARE_TURNED_ON
	PIXBeginEvent(cmdList->d3dGraphicsCommandList, PIX_COLOR((BYTE) (r * 255.0f), (BYTE) (g * 255.0f), (BYTE) (b * 255.0f)), "%s", name);
	#endif
}

void HbGPU_CmdList_Feedback_EndNameScope(HbGPU_CmdList * cmdList) {
	#ifdef PIX_EVENTS_ARE_TURNED_ON
	PIXEndEvent(cmdList->d3dGraphicsCommandList);
	#endif
}

void HbGPU_CmdList_Feedback_InsertText(HbGPU_CmdList * cmdList, char const * text, float r, float g, float b) {
	#ifdef PIX_EVENTS_ARE_TURNED_ON
	PIXSetMarker(cmdList->d3dGraphicsCommandList, PIX_COLOR((BYTE) (r * 255.0f), (BYTE) (g * 255.0f), (BYTE) (b * 255.0f)), "%s", text);
	#endif
}

#endif
