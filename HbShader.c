#include "HbShader.h"

#if HbGPU_Implementation_D3D
#include "HbShader/HbShader_FullScreenTexCoordVS.cso.inc"
size_t const HbShader_FullScreenTexCoordVS_Size = sizeof(HbShader_FullScreenTexCoordVS);
#include "HbShader/HbShader_FullScreenVS.cso.inc"
size_t const HbShader_FullScreenVS_Size = sizeof(HbShader_FullScreenVS);
#include "HbShader/HbShader_Tile_GetDepthBoundsCS.cso.inc"
size_t const HbShader_Tile_GetDepthBoundsCS_Size = sizeof(HbShader_Tile_GetDepthBoundsCS);
#include "HbShader/HbShader_Tile_GetDepthBoundsMSCS.cso.inc"
size_t const HbShader_Tile_GetDepthBoundsMSCS_Size = sizeof(HbShader_Tile_GetDepthBoundsMSCS);
#endif
