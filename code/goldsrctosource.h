/* date = November 12th 2022 8:21 pm */

#ifndef GOLDSRCTOSOURCE_H
#define GOLDSRCTOSOURCE_H

#define MAX_WADS 128
#define FOURCC_TO_INT(a, b, c, d) (((d) << 24) | ((c) << 16) | ((b) << 8) | (a))

#define LOG_ERROR(...) Print(__VA_ARGS__)

#define CMDARG_GET_STRING(cmdarg) ((cmdarg).type == CMDARG_STRING ? (cmdarg).stringValue : NULL)
#define CMDARG_GET_INT(cmdarg) ((cmdarg).type == CMDARG_INTEGER ? (cmdarg).intValue : NULL)

#define SRC_MAP_SIZE 16384
#define GSRC_MAP_SIZE 8192
#define GSRC_MAP_SIZE_HALF 4096
#define SRC_MAX_BRUSH_SIDES 128
#define SRC_MAX_SIDE_VERTS (SRC_MAX_BRUSH_SIDES - 1)

#define CONVERTED_MATERIAL_FOLDER "gsrcconv/"
#define CONVERTED_MATERIAL_PATH "materials/" CONVERTED_MATERIAL_FOLDER

typedef hmm_vec2 v2;
typedef hmm_vec3 v3;
typedef hmm_vec4 v4;
typedef hmm_mat4 mat4;

enum CmdArgType
{
	CMDARG_NONE,
	CMDARG_STRING,
	CMDARG_INTEGER,
	
	CMDARGTYPE_COUNT,
};

struct ModelInfo
{
	s32 model;
	s32 rendermode;
};

union v3i
{
	struct 
	{
		s32 x, y, z;
	};
	s32 e[3];
};

struct TraverseBspTreeNode
{
	s32 parent;
	s32 index;
};

struct CmdArg
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
};

union CmdArgs
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
};
static_assert(MEMBER_SIZE(CmdArgs, args) == sizeof(CmdArgs), "CmdArgs size and args array length are mismatched!");

global char *g_cmdArgTypeStrings[CMDARGTYPE_COUNT] = {
	"None",
	"String",
	"Integer",
};

struct Verts
{
	v3 verts[SRC_MAX_SIDE_VERTS];
	s32 vertCount;
};

struct Polygon2D
{
	v2 verts[SRC_MAX_SIDE_VERTS];
	s32 vertCount;
};

struct aabb
{
	v3 mins;
	v3 maxs;
};

struct aabbi
{
	v3i mins;
	v3i maxs;
};


struct FileWritingBuffer
{
	u8 *memory;
	s64 size;
	s64 usedBytes;
};

struct EntProperty
{
	str key;
	str value;
};

struct TexProjection
{
	v3 vec;
	f32 shift;
	f32 scale;
};

struct Texture
{
	TexProjection vecs[2];
	char name[128];
};

struct SrcPlane
{
	v3 normal;
	f32 distance;
	s32 type;
};

struct BrushSide
{
	SrcPlane plane;
	Verts polygon;
	s32 texture;
};

struct Brush
{
	BrushSide *sides;
	aabb bounds;
	s32 sideCount;
	s32 contents;
	s32 model;
};

struct Face
{
	SrcPlane plane;
	aabb bounds;
	Verts polygon;
	s32 texture;
	aabb size;
};

#define MAX_ENT_PROPERTIES 64
#define MAX_ENTITIES 4096
struct EntProperties
{
	str classname;
	s32 propertyCount;
	s32 model;
	EntProperty properties[MAX_ENT_PROPERTIES];
};

struct EntList
{
	s32 entCount;
	EntProperties *ents;
};

struct EntlumpTokeniser
{
	char *at;
};

enum EntlumpTokenType
{
	ENTLUMPTOKEN_OPEN_BRACE,  // {
	ENTLUMPTOKEN_CLOSE_BRACE, // }
	ENTLUMPTOKEN_IDENTIFIER,  // everything that's inside quotations
	ENTLUMPTOKEN_UNKNOWN,
	ENTLUMPTOKEN_EOS, // end of stream
};

struct EntlumpToken
{
	EntlumpTokenType type;
	str string;
};

enum StringToNumResult
{
	STRINGTONUM_SUCCESS = 0,
	STRINGTONUM_ERR_NUM_TOO_BIG = 1,
	STRINGTONUM_ERR_FAILED = 2,
};

struct SrcHeader;
inline void *BufferPushDataAndSetLumpSize(FileWritingBuffer *buffer, SrcHeader *header, s32 lumpIndex, void *data, s32 bytes);
internal s32 GsrcContentsToSrcContents(s32 gsrcContents);
void BSPMain(s32 argCount, char *arguments[]);

#endif //GOLDSRCTOSOURCE_H
