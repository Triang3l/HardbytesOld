#ifndef HbInclude_HbShader
#define HbInclude_HbShader
#include "HbGPU.h"

#define HbShader_GeoGen_Icosphere_VertexCount 240

#if HbGPU_Implementation_D3D
extern BYTE const HbShader_FullScreenTexCoordVS[];
extern size_t const HbShader_FullScreenTexCoordVS_Size;
extern BYTE const HbShader_FullScreenVS[];
extern size_t const HbShader_FullScreenVS_Size;
extern BYTE const HbShader_Tile_GetDepthBoundsCS[];
extern size_t const HbShader_Tile_GetDepthBoundsCS_Size;
extern BYTE const HbShader_Tile_GetDepthBoundsMSCS[];
extern size_t const HbShader_Tile_GetDepthBoundsMSCS_Size;
#endif

#endif
