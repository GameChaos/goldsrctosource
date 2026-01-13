#if !defined(SOURCE_H)

#define SRC_BSPHEADER FOURCC_TO_INT('V', 'B', 'S', 'P')

#define SRC_MINBSPVERSION 19
#define SRC_BSPVERSION 21

#define SRC_MAX_LIGHTMAP_DIM_WITHOUT_BORDER SRC_MAX_DISP_LIGHTMAP_DIM_WITHOUT_BORDER
#define SRC_MAX_LIGHTMAP_DIM_INCLUDING_BORDER SRC_MAX_DISP_LIGHTMAP_DIM_INCLUDING_BORDER

#define SRC_MAX_LIGHTSTYLES 64

enum
{
	SRC_MAX_BRUSH_LIGHTMAP_DIM_WITHOUT_BORDER = 32,
	SRC_MAX_BRUSH_LIGHTMAP_DIM_INCLUDING_BORDER = 35
};

enum
{
	SRC_MAX_DISP_LIGHTMAP_DIM_WITHOUT_BORDER = 125,
	SRC_MAX_DISP_LIGHTMAP_DIM_INCLUDING_BORDER = 128
};


enum
{
	SRC_MIN_MAP_DISP_POWER = 2,
	SRC_MAX_MAP_DISP_POWER = 4,
	SRC_MAX_DISP_CORNER_NEIGHBOURS = 4,
};

#define SRC_NUM_DISP_POWER_VERTS(power) (((1 << (power)) + 1) * ((1 << (power)) + 1))
#define SRC_NUM_DISP_POWER_TRIS(power) ((1 << (power)) * (1 << (power)) * 2)

enum
{
	SRC_MAX_MAP_MODELS = 1024,
	SRC_MAX_MAP_BRUSHES = 8192,
	SRC_MAX_MAP_ENTITIES = 8192,
	SRC_MAX_MAP_TEXINFO = 12288,
	SRC_MAX_MAP_TEXDATA = 2048,
	SRC_MAX_MAP_DISPINFO = 2048,
	SRC_MAX_MAP_DISP_VERTS = (SRC_MAX_MAP_DISPINFO * ((1 << SRC_MAX_MAP_DISP_POWER) + 1) * ((1 << SRC_MAX_MAP_DISP_POWER) + 1)),
	SRC_MAX_MAP_DISP_TRIS = ((1 << SRC_MAX_MAP_DISP_POWER) * (1 << SRC_MAX_MAP_DISP_POWER) * 2),
	SRC_MAX_DISPVERTS = SRC_NUM_DISP_POWER_VERTS(SRC_MAX_MAP_DISP_POWER),
	SRC_MAX_DISPTRIS = SRC_NUM_DISP_POWER_TRIS(SRC_MAX_MAP_DISP_POWER),
	SRC_MAX_MAP_AREAS = 256,
	SRC_MAX_MAP_AREA_BYTES = (SRC_MAX_MAP_AREAS / 8),
	SRC_MAX_MAP_AREAPORTALS = 1024,
	SRC_MAX_MAP_PLANES = 65536,
	SRC_MAX_MAP_NODES = 65536,
	SRC_MAX_MAP_BRUSHSIDES = 65536,
	SRC_MAX_MAP_LEAFS = 65536,
	SRC_MAX_MAP_VERTS = 65536,
	SRC_MAX_MAP_VERTNORMALS = 256000,
	SRC_MAX_MAP_VERTNORMALINDICES = 256000,
	SRC_MAX_MAP_FACES = 65536,
	SRC_MAX_MAP_LEAFFACES = 65536,
	SRC_MAX_MAP_LEAFBRUSHES = 65536,
	SRC_MAX_MAP_PORTALS = 65536,
	SRC_MAX_MAP_CLUSTERS = 65536,
	SRC_MAX_MAP_LEAFWATERDATA = 32768,
	SRC_MAX_MAP_PORTALVERTS = 128000,
	SRC_MAX_MAP_EDGES = 256000,
	SRC_MAX_MAP_SURFEDGES = 512000,
	SRC_MAX_MAP_LIGHTING = 0x1000000,
	SRC_MAX_MAP_VISIBILITY = 0x1000000,
	SRC_MAX_MAP_TEXTURES = 1024,
	SRC_MAX_MAP_WORLDLIGHTS = 8192,
	SRC_MAX_MAP_CUBEMAPSAMPLES = 1024,
	SRC_MAX_MAP_OVERLAYS = 512,
	SRC_MAX_MAP_WATEROVERLAYS = 16384,
	SRC_MAX_MAP_TEXDATA_STRING_DATA = 256000,
	SRC_MAX_MAP_TEXDATA_STRING_TABLE = 65536,
	SRC_MAX_MAP_PRIMITIVES = 32768,
	SRC_MAX_MAP_PRIMVERTS = 65536,
	SRC_MAX_MAP_PRIMINDICES = 65536,
};

enum
{
	SRC_MAX_KEY = 32,
	SRC_MAX_VALUE = 1024
};

enum
{
	SRC_LUMP_ENTITIES                       = 0,
	SRC_LUMP_PLANES                         = 1,
	SRC_LUMP_TEXDATA                        = 2,
	SRC_LUMP_VERTEXES                       = 3,
	SRC_LUMP_VISIBILITY                     = 4,
	SRC_LUMP_NODES                          = 5,
	SRC_LUMP_TEXINFO                        = 6,
	SRC_LUMP_FACES                          = 7,
	SRC_LUMP_LIGHTING                       = 8,
	SRC_LUMP_OCCLUSION                      = 9,
	SRC_LUMP_LEAFS                          = 10,
	SRC_LUMP_FACEIDS                        = 11,
	SRC_LUMP_EDGES                          = 12,
	SRC_LUMP_SURFEDGES                      = 13,
	SRC_LUMP_MODELS                         = 14,
	SRC_LUMP_WORLDLIGHTS                    = 15,
	SRC_LUMP_LEAFFACES                      = 16,
	SRC_LUMP_LEAFBRUSHES                    = 17,
	SRC_LUMP_BRUSHES                        = 18,
	SRC_LUMP_BRUSHSIDES                     = 19,
	SRC_LUMP_AREAS                          = 20,
	SRC_LUMP_AREAPORTALS                    = 21,
	SRC_LUMP_FACEBRUSHES                    = 22,
	SRC_LUMP_FACEBRUSHLIST                  = 23, // BSP v21 only? indices of faces and which brush they came from.
	SRC_LUMP_UNUSED2                        = 24,
	SRC_LUMP_UNUSED3                        = 25,
	SRC_LUMP_DISPINFO                       = 26,
	SRC_LUMP_ORIGINALFACES                  = 27,
	SRC_LUMP_PHYSDISP                       = 28,
	SRC_LUMP_PHYSCOLLIDE                    = 29,
	SRC_LUMP_VERTNORMALS                    = 30,
	SRC_LUMP_VERTNORMALINDICES              = 31,
	SRC_LUMP_DISP_LIGHTMAP_ALPHAS           = 32,
	SRC_LUMP_DISP_VERTS                     = 33,
	SRC_LUMP_DISP_LIGHTMAP_SAMPLE_POSITIONS = 34,
	SRC_LUMP_GAME_LUMP                      = 35,
	SRC_LUMP_LEAFWATERDATA                  = 36,
	SRC_LUMP_PRIMITIVES                     = 37,
	SRC_LUMP_PRIMVERTS                      = 38,
	SRC_LUMP_PRIMINDICES                    = 39,
	SRC_LUMP_PAKFILE                        = 40,
	SRC_LUMP_CLIPPORTALVERTS                = 41,
	SRC_LUMP_CUBEMAPS                       = 42,
	SRC_LUMP_TEXDATA_STRING_DATA            = 43,
	SRC_LUMP_TEXDATA_STRING_TABLE           = 44,
	SRC_LUMP_OVERLAYS                       = 45,
	SRC_LUMP_LEAFMINDISTTOWATER             = 46,
	SRC_LUMP_FACE_MACRO_TEXTURE_INFO        = 47,
	SRC_LUMP_DISP_TRIS                      = 48,
	SRC_LUMP_PROP_BLOB                      = 49,
	SRC_LUMP_WATEROVERLAYS                  = 50,
	SRC_LUMP_LEAF_AMBIENT_INDEX_HDR         = 51,
	SRC_LUMP_LEAF_AMBIENT_INDEX             = 52,
	SRC_LUMP_LIGHTING_HDR                   = 53,
	SRC_LUMP_WORLDLIGHTS_HDR                = 54,
	SRC_LUMP_LEAF_AMBIENT_LIGHTING_HDR      = 55,
	SRC_LUMP_LEAF_AMBIENT_LIGHTING          = 56,
	SRC_LUMP_XZIPPAKFILE                    = 57,
	SRC_LUMP_FACES_HDR                      = 58,
	SRC_LUMP_MAP_FLAGS                      = 59,
	SRC_LUMP_OVERLAY_FADES                  = 60,
	SRC_LUMP_OVERLAY_SYSTEM_LEVELS          = 61,
	SRC_LUMP_PHYSLEVEL                      = 62,
	SRC_LUMP_DISP_MULTIBLEND                = 63,
	SRC_HEADER_LUMPS                        = 64,
};

enum
{
	SRC_SURF_LIGHT     = 0x1,
	SRC_SURF_SKY2D     = 0x2,
	SRC_SURF_SKY       = 0x4,
	SRC_SURF_WARP      = 0x8, // apparently indicates water
	SRC_SURF_TRANS     = 0x10,
	SRC_SURF_NOPORTAL  = 0x20,
	SRC_SURF_TRIGGER   = 0x40,
	SRC_SURF_NODRAW    = 0x80,
	SRC_SURF_HINT      = 0x100,
	SRC_SURF_SKIP      = 0x200,
	SRC_SURF_NOLIGHT   = 0x400,
	SRC_SURF_BUMPLIGHT = 0x800,
	SRC_SURF_NOSHADOWS = 0x1000,
	SRC_SURF_NODECALS  = 0x2000,
	SRC_SURF_NOCHOP    = 0x4000,
	SRC_SURF_HITBOX    = 0x8000,
};

enum
{
	SRC_CONTENTS_EMPTY                 = 0x0,
	SRC_CONTENTS_SOLID                 = 0x1,
	SRC_CONTENTS_WINDOW                = 0x2,
	SRC_CONTENTS_AUX                   = 0x4,
	SRC_CONTENTS_GRATE                 = 0x8,
	SRC_CONTENTS_SLIME                 = 0x10,
	SRC_CONTENTS_WATER                 = 0x20,
	SRC_CONTENTS_BLOCKLOS              = 0x40,
	SRC_CONTENTS_OPAQUE                = 0x80,
	SRC_LAST_VISIBLE_CONTENTS          = 0x80,
	SRC_ALL_VISIBLE_CONTENTS           = (SRC_LAST_VISIBLE_CONTENTS | (SRC_LAST_VISIBLE_CONTENTS - 1)),
	SRC_CONTENTS_TESTFOGVOLUME         = 0x100,
	SRC_CONTENTS_UNUSED                = 0x200,
	SRC_CONTENTS_UNUSED6               = 0x400,
	SRC_CONTENTS_TEAM1                 = 0x800,
	SRC_CONTENTS_TEAM2                 = 0x1000,
	SRC_CONTENTS_IGNORE_NODRAW_OPAQUE  = 0x2000,
	SRC_CONTENTS_MOVEABLE              = 0x4000,
	SRC_CONTENTS_AREAPORTAL            = 0x8000,
	SRC_CONTENTS_PLAYERCLIP            = 0x10000,
	SRC_CONTENTS_MONSTERCLIP           = 0x20000,
	SRC_CONTENTS_CURRENT_0             = 0x40000,
	SRC_CONTENTS_CURRENT_90            = 0x80000,
	SRC_CONTENTS_CURRENT_180           = 0x100000,
	SRC_CONTENTS_CURRENT_270           = 0x200000,
	SRC_CONTENTS_CURRENT_UP            = 0x400000,
	SRC_CONTENTS_CURRENT_DOWN          = 0x800000,
	SRC_CONTENTS_ORIGIN                = 0x1000000,
	SRC_CONTENTS_MONSTER               = 0x2000000,
	SRC_CONTENTS_DEBRIS                = 0x4000000,
	SRC_CONTENTS_DETAIL                = 0x8000000,
	SRC_CONTENTS_TRANSLUCENT           = 0x10000000,
	SRC_CONTENTS_LADDER                = 0x20000000,
	SRC_CONTENTS_HITBOX                = 0x40000000,
};

enum
{
	SRC_LEAF_FLAGS_SKY                    = 0x1,
	SRC_LEAF_FLAGS_RADIAL                 = 0x2,
	SRC_LEAF_FLAGS_SKY2D                  = 0x4,
	SRC_LEAF_FLAGS_CONTAINS_DETAILOBJECTS = 0x8,
};

enum
{
	LVLFLAGS_BAKED_STATIC_PROP_LIGHTING_NONHDR = 0x1,
	LVLFLAGS_BAKED_STATIC_PROP_LIGHTING_HDR    = 0x2,
	LVLFLAGS_UNKNOWN1 = 0x4,
	LVLFLAGS_UNKNOWN2 = 0x8,
	LVLFLAGS_UNKNOWN3 = 0x10,
	LVLFLAGS_UNKNOWN4 = 0x20,
	LVLFLAGS_UNKNOWN5 = 0x40,
};

enum
{
	SRC_PLANE_X,
	SRC_PLANE_Y,
	SRC_PLANE_Z,
	SRC_PLANE_ANYX,
	SRC_PLANE_ANYY,
	SRC_PLANE_ANYZ,
};

typedef enum
{
	SRC_EMIT_SURFACE,
	SRC_EMIT_POINT,
	SRC_EMIT_SPOTLIGHT,
	SRC_EMIT_SKYLIGHT,
	SRC_EMIT_QUAKELIGHT,
	SRC_EMIT_SKYAMBIENT,
} SrcEmitType;

typedef union
{
	struct
	{
		u8 r, g, b;
		i8 exponent;
	};
	u8 elements[3];
} Rgbe8888;

typedef struct
{
	i32 offset;
	i32 length;
	i32 version;
	char fourCC[4];
} SrcLump;

typedef struct
{
	int ident;
	int version;
	SrcLump lump[SRC_HEADER_LUMPS];
	int mapRevision;
} SrcHeader;


typedef struct
{
	i32 lumpCount;
} SrcGamelumpHeader;

typedef struct
{
	union
	{
		i32 id;
		char fourcc[4];
	};
	u16 flags;
	u16 version;
	i32 fileOffset;
	i32 length;
} SrcGamelump;

typedef struct
{
	u16 unknown;
	u16 brush;
} SrcFaceBrush;

typedef struct
{
	i16 vertex[2]; // indices into the vertex array
} SrcEdge;

typedef struct
{
	v3 origin;
	v3 intensity;
	v3 normal;
	i32 cluster;
	SrcEmitType type;
	i32 style;
	f32 stopdot;
	f32 stopdot2;
	f32 exponent;
	f32 radius;
	f32 constantAttenuation;	
	f32 linearAttenuation;
	f32 quadraticAttenuation;
	i32 flags;
	i32 texinfo;
	i32 owner;
} SrcWorldLight_v0;

// v1
typedef struct
{
	v3 origin;
	v3 intensity;
	v3 normal;
	v3 unknown;
	i32 cluster;
	SrcEmitType type;
	i32 style;
	f32 stopdot;
	f32 stopdot2;
	f32 exponent;
	f32 radius;
	f32 constantAttenuation;	
	f32 linearAttenuation;
	f32 quadraticAttenuation;
	i32 flags;
	i32 texinfo;
	i32 owner;
} SrcWorldLight;

typedef struct
{
	Rgbe8888 colour[6];
} SrcCompressedLightCube;

typedef struct
{
	SrcCompressedLightCube cube;
	u8 x;
	u8 y;
	u8 z;
	u8 padding_;
} SrcLeafAmbientLighting;

typedef struct
{
	u16 ambientSampleCount;
	u16 firstAmbientSample;
} SrcLeafAmbientIndex;

typedef struct
{
	// NOTE(GameChaos): NOT a v4 because that screws up packing!
	f32 textureVecs[2][4];
	f32 lightmapVecs[2][4];
	i32 flags; // SRC_SURF_ flags
	i32 texdata;
} SrcTexinfo;

typedef struct
{
	v3 reflectivity;
	i32 nameStringTableID;
	i32 width;
	i32 height;
	i32 viewWidth;
	i32 viewHeight;
} SrcTexdata;

typedef struct
{
	v3 mins;
	v3 maxs;
	v3 origin;
	i32 headnode;
	i32 firstFace;
	i32 numFaces;
} SrcModel;

typedef struct
{
	u16 planeIndex;
	u8 side;
	u8 onNode;
	i32 firstEdge;          // index into surfedges
	i16 edges;              // number of surfedges
	i16 texinfo;            // texture info index
	i16 dispinfo;           // displacement info index
	i16 surfaceFogVolumeID; // ?
	u8 styles[4];           // switchable lighting info
	i32 lightOffset;        // byte offset into lightmap lump
	f32 area;               // face area in units^2
	i32 lightmapTextureMinsInLuxels[2];
	i32 lightmapTextureSizeInLuxels[2];
	i32 origFace;
	u16 numPrims;
	u16 firstPrimID;
	u32 smoothingGroups; // lightmap smoothing group
} SrcFace;
static_assert(sizeof(SrcFace) == 56);

typedef struct
{
	i32 firstSide; // first brushside
	i32 sides;     // number of brushsides
	i32 contents;  // contents flags
} SrcBrush;

typedef struct
{
	u16 plane;
	i16 texinfo;  // texture info
	i16 dispinfo; // displacement info index
	i16 bevel;    // bevel planes are used for AABB tracing.
} SrcBrushSide;

typedef struct
{
	i32 plane;       // index into plane array
	i32 children[2];
	i16 mins[3];
	i16 maxs[3];
	u16 firstFace;   // index into face array
	u16 faceCount;
	i16 area;
	i16 padding_;     // pad to 32 bytes length
} SrcNode;

typedef struct
{
	f32 surfaceZ;
	f32 minZ;
	i16 surfaceTexinfo;
} SrcLeafWaterData;

typedef struct
{
	i32 contents;
	i16 cluster;  // cluster this leaf is in
	i16 area:9;   // area this leaf is in
	i16 flags:7;  // leaf flags
	i16 mins[3];
	i16 maxs[3];
	u16 firstLeafFace;   // index into leaffaces
	u16 leafFaces;
	u16 firstLeafBrush;  // index into leafbrushes
	u16 leafBrushes;
	i16 leafWaterDataIndex;
	i16 padding_; // padding to 4-byte boundary
} SrcLeaf;

typedef struct
{
	i32 areaportalCount;
	i32 areaportalIndex;
} SrcArea;

typedef struct
{
	u16 portalKey;
	u16 otherarea;
	u16 firstClipPortalVert;
	u16 clipPortalVerts;
	i32 planenum;
} SrcAreaportal;

// TODO: order the structs in good nice order
typedef struct
{
	void *fileData;
	size_t fileDataSize;
	
	SrcHeader *header;
	
	//i32 entitiesLength;
	//char *lumpEntities;
	
	i32 planeCount;
	SrcPlane *lumpPlanes;
	
	//i32 texdataCount;
	//SrcTexdata *lumpTexdata;
	
	//i32 vertexCount;
	//v3 *lumpVertices;
	
	i32 edgeCount;
	SrcEdge *lumpEdges;
	
	i32 surfEdgeCount;
	i32 *lumpSurfEdges;
	
	//i32 faceCount;
	//SrcFace *lumpFaces;
	
	i32 brushCount;
	SrcBrush *lumpBrushes;
	
	i32 brushSideCount;
	SrcBrushSide *lumpBrushSides;
	
	/*
	i32 visLength;
	u8 *lumpVIS;
	*/
	
	//i32 nodeCount;
	//SrcNode *lumpNodes;
	
	//i32 leafCount;
	//SrcLeaf *lumpLeaves;
	
	i32 leafFaceCount;
	u16 *lumpLeafFaces;
	
	i32 leafBrushCount;
	u16 *lumpLeafBrushes;
	
	i32 areaCount;
	SrcArea *lumpAreas;
	
	//i32 texinfoCount;
	//SrcTexinfo *lumpTexinfo;
	
	//i32 texdataStringDataSize;
	//char *lumpTexdataStringData;
	
	//i32 texdataStringTableCount;
	//i32 *lumpTexdataStringTable;
	
	/*
			i32 lightingLength;
			u8 *lumpLighting;
			
			i32 clipnodeCount;
			Clipnode *lumpClipnodes;
			*/
	
	/*
	i32 marksurfaceCount;
	u16 *lumpMarksurfaces;
	*/
	
	i32 modelCount;
	SrcModel *lumpModels;
} SrcMapData;

#define SOURCE_H
#endif
