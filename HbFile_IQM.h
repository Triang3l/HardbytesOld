#ifndef HbInclude_HbFile_IQM
#define HbInclude_HbFile_IQM
#include "HbCommon.h"
#ifdef __cplusplus
extern "C" {
#endif

extern char const HbFile_IQM_Magic[16];
#define HbFile_IQM_Version 2

typedef struct HbFile_IQM_Header {
	char magic[16];
	uint32_t version;
	uint32_t fileSize;
	uint32_t flags;
	uint32_t textSize, textOffset;
	uint32_t meshCount, meshOffset;
	uint32_t vertexArrayCount, vertexCount, vertexArrayOffset;
	uint32_t triangleCount, triangleOffset, adjacencyOffset;
	uint32_t jointCount, jointOffset;
	uint32_t poseCount, poseOffset;
	uint32_t animationCount, animationOffset;
	uint32_t frameCount, frameChannelCount, frameOffset, boundsOffset;
	uint32_t commentSize, commentOffset;
	uint32_t extensionCount, extensionOffset;
} HbFile_IQM_Header;

typedef struct HbFile_IQM_Mesh {
	uint32_t name;
	uint32_t materialName;
	uint32_t vertexFirst, vertexCount;
	uint32_t triangleFirst, triangleCount;
} HbFile_IQM_Mesh;

typedef enum HbFile_IQM_VertexArray_Type {
	HbFile_IQM_VertexArray_Type_Position, // Float32x3 by default.
	HbFile_IQM_VertexArray_Type_TexCoord, // Float32x2.
	HbFile_IQM_VertexArray_Type_Normal, // Float32x3.
	HbFile_IQM_VertexArray_Type_Tangent, // Float32x4.
	HbFile_IQM_VertexArray_Type_BlendIndexes, // UInt8x4.
	HbFile_IQM_VertexArray_Type_BlendWeights, // UNorm8x4.
	HbFile_IQM_VertexArray_Type_Color, // UNorm8x4.
	// All values up to Custom are reserved for future use. Any value >= Custom is interpreted as a custom type.
	// The value then defines an offset into the string table, where offset = value - Custom. This must be a valid string naming the type.
	HbFile_IQM_VertexArray_Type_Custom = 0x10,
} HbFile_IQM_VertexArray_Type;

typedef enum HbFile_IQM_VertexArray_Format {
	HbFile_IQM_VertexArray_Format_Byte,
	HbFile_IQM_VertexArray_Format_UByte,
	HbFile_IQM_VertexArray_Format_Short,
	HbFile_IQM_VertexArray_Format_UShort,
	HbFile_IQM_VertexArray_Format_Int,
	HbFile_IQM_VertexArray_Format_UInt,
	HbFile_IQM_VertexArray_Format_Half,
	HbFile_IQM_VertexArray_Format_Float,
	HbFile_IQM_VertexArray_Format_Double,
} HbFile_IQM_VertexArray_Format;

typedef struct HbFile_IQM_VertexArray {
	uint32_t type; // Type or custom name.
	uint32_t flags;
	uint32_t format;
	uint32_t componentCount;
	uint32_t offset;
} HbFile_IQM_VertexArray;

typedef uint32_t HbFile_IQM_VertexIndex;

typedef uint32_t HbFile_IQM_TriangleIndex;

typedef struct HbFile_IQM_Joint {
	uint32_t name;
	int32_t parent; // Parent < 0 means this is a root bone.
	// Output = (input * scale) * rotation + translation.
	float translate[3], rotate[4], scale[3];
} HbFile_IQM_Joint;

typedef struct HbFile_IQM_Pose {
	int32_t parent; // Parent < 0 means this is a root bone.
	uint32_t channelMask; // Mask of which of the 10 channels are present for this joint pose.
    float channeloffset[10], channelscale[10]; 
    // Channels 0..2 are translation <Tx, Ty, Tz> and channels 3..6 are quaternion rotation <Qx, Qy, Qz, Qw>.
    // Rotation is in relative/parent local space.
    // Channels 7..9 are scale <Sx, Sy, Sz>.
    // Output = (input * scale) * rotation + translation.
} HbFile_IQM_Pose;

uint16_t HbFile_IQM_Frame;

typedef uint32_t HbFile_IQM_Animation_Flags;
enum {
	HbFile_IQM_Animation_Flags_Loop = 1 << 0,
};

typedef struct HbFile_IQM_Animation {
	uint32_t name;
	uint32_t frameFirst, frameCount;
	float framerate;
	HbFile_IQM_Animation_Flags flags;
} HbFile_IQM_Animation;

typedef struct HbFile_IQM_Bounds {
	float mins[3], maxs[3];
	float xyRadius, radius;
} HbFile_IQM_Bounds;

typedef struct HbFile_IQM_Extension {
	uint32_t name;
	uint32_t dataSize, dataOffset;
	uint32_t nextOffset;
} HbFile_IQM_Extension;

#ifdef __cplusplus
}
#endif
#endif
