#ifndef HbInclude_HbGFX
#define HbInclude_HbGFX
#include "HbGPU.h"

/***************************************************
 * Screen tile association with 2 clusters per tile
 ***************************************************/

#define HbGFX_Tile_TileSizeLog2 4
#define HbGFX_Tile_TileSize (1 << HbGFX_Tile_TileSizeLog2)

HbBool HbGFX_Tile_Init(HbGPU_Device * gpuDevice);
void HbGFX_Tile_Shutdown();

// cmdList must not have a pass open (will use a compute pass internally).
//
// perSampleDepthsHandle - index of the handle of the source per-pixel or per-sample depth texture (Read_ResourceNonPS usage).
// perSampleDepthsInfo - perSampleDepths image info, with its full width and height (not sent to the shader - using GetDimensions) and sample count.
// tileDepthBoundsHandles - index of the handles of 2 ShaderEdit resources:
// - widthTiles x 2*heightTiles float32 buffer with 256-aligned rows - nearest depths of two clusters (in two halves) for back face drawing for marking.
// - widthTiles x heightTiles x 2 float32 array image - farthest depths of two clusters for front face drawing for occlusion culling.
// z0AndZUnprojectScale - [0] = linear Z at 0 in the depth buffer (z0), [1] = (z0 - z1) / z1.
//
// Must be followed by edit commit barriers on the buffer and the image behind tileDepthBoundsHandles!
void HbGFX_Tile_GetDepthBounds(HbGPU_CmdList * cmdList, uint32_t perSampleDepthsHandle, HbGPU_Image_Info const * perSampleDepthsInfo,
		uint32_t tileDepthBoundsHandles, float const z0AndZUnprojectScale[2]);

#endif
