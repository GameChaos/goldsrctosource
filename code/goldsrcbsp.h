
#ifndef GOLDSRCBSP_H
#define GOLDSRCBSP_H

enum
{
	GSRC_LUMP_ENTITIES      = 0,
	GSRC_LUMP_PLANES        = 1,
	GSRC_LUMP_TEXTURES      = 2,
	GSRC_LUMP_VERTICES      = 3,
	GSRC_LUMP_VISIBILITY    = 4,
	GSRC_LUMP_NODES         = 5,
	GSRC_LUMP_TEXINFO       = 6,
	GSRC_LUMP_FACES         = 7,
	GSRC_LUMP_LIGHTING      = 8,
	GSRC_LUMP_CLIPNODES     = 9,
	GSRC_LUMP_LEAVES        = 10,
	GSRC_LUMP_MARKSURFACES  = 11,
	GSRC_LUMP_EDGES         = 12,
	GSRC_LUMP_SURFEDGES     = 13,
	GSRC_LUMP_MODELS        = 14,
	GSRC_HEADER_LUMPS       = 15,
};

enum
{
	GSRC_MAX_MAP_HULLS        = 4,
	
	GSRC_MAX_MAP_MODELS       = 400,
	GSRC_MAX_MAP_BRUSHES      = 4096,
	GSRC_MAX_MAP_ENTITIES     = 1024,
	GSRC_MAX_MAP_ENTSTRING    = 128 * 1024,
	
	GSRC_MAX_MAP_PLANES       = 32767,
	GSRC_MAX_MAP_NODES        = 32767,
	GSRC_MAX_MAP_CLIPNODES    = 32767,
	GSRC_MAX_MAP_LEAFS        = 8192,
	GSRC_MAX_MAP_VERTS        = 65535,
	GSRC_MAX_MAP_FACES        = 65535,
	GSRC_MAX_MAP_MARKSURFACES = 65535,
	GSRC_MAX_MAP_TEXINFO      = 8192,
	GSRC_MAX_MAP_EDGES        = 256000,
	GSRC_MAX_MAP_SURFEDGES    = 512000,
	GSRC_MAX_MAP_TEXTURES     = 512,
	GSRC_MAX_MAP_MIPTEX       = 0x200000,
	GSRC_MAX_MAP_LIGHTING     = 0x200000,
	GSRC_MAX_MAP_VISIBILITY   = 0x200000,
	
	GSRC_MAX_MAP_PORTALS      = 65536,
};

// for entity keyvalues
enum
{
	GSRC_MAX_KEY     = 32,
	GSRC_MAX_VALUE   = 1024,
};

enum
{
	GSRC_PLANE_X    = 0, // Plane is perpendicular to given axis
	GSRC_PLANE_Y    = 1,
	GSRC_PLANE_Z    = 2,
	GSRC_PLANE_ANYX = 3, // Non-axial plane is snapped to the nearest
	GSRC_PLANE_ANYY = 4,
	GSRC_PLANE_ANYZ = 5,
};

enum
{
	GSRC_CONTENTS_EMPTY        = -1,
	GSRC_CONTENTS_SOLID        = -2,
	GSRC_CONTENTS_WATER        = -3,
	GSRC_CONTENTS_SLIME        = -4,
	GSRC_CONTENTS_LAVA         = -5,
	GSRC_CONTENTS_SKY          = -6,
	GSRC_CONTENTS_ORIGIN       = -7,
	GSRC_CONTENTS_CLIP         = -8,
	GSRC_CONTENTS_CURRENT_0    = -9,
	GSRC_CONTENTS_CURRENT_90   = -10,
	GSRC_CONTENTS_CURRENT_180  = -11,
	GSRC_CONTENTS_CURRENT_270  = -12,
	GSRC_CONTENTS_CURRENT_UP   = -13,
	GSRC_CONTENTS_CURRENT_DOWN = -14,
	GSRC_CONTENTS_TRANSLUCENT  = -15,
};

typedef struct
{
	v3 normal;
	f32 distance;
	i32 type;
} GsrcPlane;

typedef struct Wad3TextureHeader Wad3TextureHeader;

typedef struct
{
	// header
	u32 mipTextureCount;   // Number of MipTexture structures and mipTextureOffsets
	i32 *mipTextureOffsets; // array of MipTexture offsets
	
	// data
	Wad3TextureHeader **mipTextures;  // array of pointers to pointers of MipTextures
} GsrcLumpTextures;

typedef struct
{
	u32 plane;      // Index into Planes lump
	i16 children[2]; // If > 0, then indices into Nodes // otherwise bitwise inverse indices into Leafs
	i16 mins[3];     // Defines bounding box
	i16 maxs[3];
	u16 firstFace;  // Index and
	u16 faceCount;  // count into Faces
} GsrcNode;

typedef struct
{
	// NOTE(GameChaos): NOT a v4 because that screws up packing!
	f32 vecs[2][4];
	u32 miptex; // Index into textures array
	u32 flags;  // Texture flags, seem to always be 0
} GsrcTexinfo;

typedef struct
{
	u16 plane;          // Plane the face is parallel to
	u16 planeSide;      // Set if different normals orientation
	u32 firstEdge;      // Index of the first surfedge
	u16 edges;          // Number of consecutive surfedges
	u16 texInfoIndex;   // Index of the texture info structure
	u8 styles[4];       // Specify lighting styles
	u32 lightmapOffset; // Byte offset into the raw lightmap data
} GsrcFace;

typedef struct
{
	i32 plane;
	i16 children[2];
} GsrcClipnode;

typedef struct
{
	i32 contents;  // Contents enumeration
	i32 visOffset; // Offset into the visibility lump
	i16 mins[3];
	i16 maxs[3];
	i16 firstMarkSurface; // Index into marksurfaces array
	i16 markSurfaces;      // Count into marksurfaces array
	u8 ambientLevels[4]; // Ambient sound levels
} GsrcLeaf;

typedef struct
{
	i16 vertex[2]; // indices into the vertex array
} GsrcEdge;

typedef struct
{
	i32 offset; // File offset to data
	i32 length; // Length of data
} GsrcLump;

typedef struct
{
	i32 version;           // Must be 30 for a valid HL BSP file
	GsrcLump lump[GSRC_HEADER_LUMPS]; // Stores the directory of lumps
} GsrcHeader;

typedef struct
{
	v3 mins;
	v3 maxs;
	v3 origin;
	i32 headnodes[GSRC_MAX_MAP_HULLS]; // Index into nodes array
	i32 visLeafs;
	i32 firstFace;
	i32 faces; // Index and count into faces
} GsrcModel;

typedef struct
{
	size_t fileDataSize;
	void *fileData;
	
	GsrcHeader *header;
	
	// the order of these has to correspond to the LUMP_* enum.
	
	// this has to be the first lump.
	str lumpEntities;
	
	i32 planeCount;
	GsrcPlane *lumpPlanes;
	
	// NOTE: lumpTextures is just a lump of data for now
	i32 textureLumpSize;
	u8 *lumpTextureMemory;
	GsrcLumpTextures lumpTextures;
	
	i32 vertexCount;
	v3 *lumpVertices;
	
	i32 visLength;
	u8 *lumpVIS;
	
	i32 nodeCount;
	GsrcNode *lumpNodes;
	
	i32 texinfoCount;
	GsrcTexinfo *lumpTexinfo;
	
	i32 faceCount;
	GsrcFace *lumpFaces;
	
	i32 lightingLength;
	u8 *lumpLighting;
	
	i32 clipnodeCount;
	GsrcClipnode *lumpClipnodes;
	
	i32 leafCount;
	GsrcLeaf *lumpLeaves;
	
	i32 marksurfaceCount;
	u16 *lumpMarksurfaces;
	
	i32 edgeCount;
	GsrcEdge *lumpEdges;
	
	i32 surfEdgeCount;
	i32 *lumpSurfEdges;
	
	i32 modelCount;
	GsrcModel *lumpModels;
} GsrcMapData;

#endif // GOLDSRCBSP_H
