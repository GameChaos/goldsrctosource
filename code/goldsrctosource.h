/* date = November 12th 2022 8:21 pm */

#ifndef GOLDSRCTOSOURCE_H
#define GOLDSRCTOSOURCE_H

#define MAX_WADS 128
#define FOURCC_TO_INT(a, b, c, d) (((d) << 24) | ((c) << 16) | ((b) << 8) | (a))

#define CMDARG_GET_STRING(cmdarg) ((cmdarg).type == CMDARG_STRING ? (cmdarg).stringValue : NULL)
#define CMDARG_GET_INT(cmdarg) ((cmdarg).type == CMDARG_INTEGER ? (cmdarg).intValue : NULL)

#define SRC_MAP_SIZE 16384
#define GSRC_MAP_SIZE 8192
#define GSRC_MAP_SIZE_HALF 4096
#define SRC_MAX_BRUSH_SIDES 128
#define SRC_MAX_SIDE_VERTS (SRC_MAX_BRUSH_SIDES - 1)

#define CONVERTED_MATERIAL_FOLDER "gsrcconv/"
#define CONVERTED_MATERIAL_PATH "materials/" CONVERTED_MATERIAL_FOLDER

typedef enum
{
	CMDARG_NONE,
	CMDARG_STRING,
	CMDARG_INTEGER,
	
	CMDARGTYPE_COUNT,
} CmdArgType;

typedef enum
{
	SKY_SIDE_UP = 0,
	SKY_SIDE_DOWN,
	SKY_SIDE_LEFT,
	SKY_SIDE_RIGHT,
	SKY_SIDE_BACK,
	SKY_SIDE_FRONT,
	
	SKY_SIDE_COUNT,
} SkySide;

typedef struct
{
	s32 model;
	s32 rendermode;
} ModelInfo;

typedef struct
{
	s32 parent;
	s32 index;
} TraverseBspTreeNode;

typedef struct
{
	char argName[64];
	char description[1024];
	
	CmdArgType type;
	union
	{
		char stringValue[512];
		s32 intValue;
	};
	
	b32 isInCmdLine; // whether this exists on the command line
} CmdArg;

typedef union
{
	struct
	{
		CmdArg help;
		CmdArg input;
		CmdArg outputbsp;
		CmdArg outputvmf;
		CmdArg enginePath;
		CmdArg mod;
		CmdArg assetPath;
	};
	CmdArg args[7];
} CmdArgs;
static_assert(MEMBER_SIZE(CmdArgs, args) == sizeof(CmdArgs), "CmdArgs size and args array length are mismatched!");

typedef struct
{
	v3 verts[SRC_MAX_SIDE_VERTS];
	s32 vertCount;
} Verts;

typedef struct
{
	v2 verts[SRC_MAX_SIDE_VERTS];
	s32 vertCount;
} Polygon2D;

typedef struct
{
	v3 mins;
	v3 maxs;
} aabb;

typedef struct
{
	v3i mins;
	v3i maxs;
} aabbi;


typedef struct
{
	u8 *memory;
	s64 size;
	s64 usedBytes;
} FileWritingBuffer;

typedef struct
{
	str key;
	str value;
} EntProperty;

typedef struct
{
	v3 vec;
	f32 shift;
	f32 scale;
} TexProjection;

typedef struct
{
	TexProjection vecs[2];
	char name[128];
} Texture;

typedef struct
{
	v3 normal;
	f32 distance;
	s32 type;
} SrcPlane;

typedef struct
{
	SrcPlane plane;
	Verts polygon;
	s32 texture;
} BrushSide;

typedef struct
{
	BrushSide *sides;
	aabb bounds;
	s32 sideCount;
	s32 contents;
	s32 model;
} Brush;

typedef struct
{
	SrcPlane plane;
	aabb bounds;
	Verts polygon;
	s32 texture;
	aabb size;
} Face;

#define MAX_ENT_PROPERTIES 256
#define MAX_ENTITIES 4096
typedef struct
{
	str classname;
	s32 propertyCount;
	s32 model;
	EntProperty properties[MAX_ENT_PROPERTIES];
} EntProperties;

typedef struct
{
	s32 entCount;
	EntProperties *ents;
} EntList;

typedef struct
{
	const char *at;
} EntlumpTokeniser;

typedef enum
{
	ENTLUMPTOKEN_OPEN_BRACE,  // {
	ENTLUMPTOKEN_CLOSE_BRACE, // }
	ENTLUMPTOKEN_IDENTIFIER,  // everything that's inside quotations
	ENTLUMPTOKEN_UNKNOWN,
	ENTLUMPTOKEN_EOS, // end of stream
} EntlumpTokenType;

typedef struct
{
	EntlumpTokenType type;
	str string;
} EntlumpToken;

typedef enum
{
	STRINGTONUM_SUCCESS = 0,
	STRINGTONUM_ERR_NUM_TOO_BIG = 1,
	STRINGTONUM_ERR_FAILED = 2,
} StringToNumResult;

internal void FatalError(const char *error);
internal void Error(const char *format, ...);
internal void Warning(const char *format, ...);
internal s32 GsrcContentsToSrcContents(s32 gsrcContents);
void BSPMain(s32 argCount, char *arguments[]);

#endif //GOLDSRCTOSOURCE_H
