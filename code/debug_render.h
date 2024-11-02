/* date = November 14th 2022 11:43 am */

#ifndef DEBUG_RENDER_H
#define DEBUG_RENDER_H

#define DEBUG_GFX_MAX_TEXTURES 4096
#define DEBUG_GFX_MAX_MESHES 512

#define DEBUG_GFX_FOV 90
#define DEBUG_GFX_NEARZ 4
#define DEBUG_GFX_FARZ 32768

typedef struct
{
	v3 endPosition;
	f32 distance;
	bool hit;
} GfxRayHit;

typedef struct
{
	v3 pos;
	v3 normal;
	v2 uv;
} GfxVertData;

typedef struct
{
	sg_buffer vertexBuffer;
	sg_buffer indexBuffer; // if invalid, then fallback is GfxState.indexBuffer
	sg_buffer wireIndexBuffer; // if invalid, then fallback is GfxState.wireIndexBuffer
	i32 vertCount;
	i32 indexCount; // if 0, then this will be calculated from vertCount
	i32 wireIndexCount; // if 0, then this will be calculated from vertCount
	i32 texture;
	v3 wireColour;
} GfxMesh;

typedef struct
{
	i32 texture;
	i32 vertCount;
	v4 s;
	v4 t;
	GfxVertData *verts;
} GfxPoly;

typedef struct
{
	sg_image img;
} GfxTexture;

typedef struct
{
	bool isDown;
	i32 actionCount; // amount of up & down events in this frame.
} GfxButton;

typedef struct
{
	v2 mousePos;
	v2 mouseDelta;
	v2 mouseScroll;
	union
	{
		struct
		{
			GfxButton mouse1;
			GfxButton mouse2;
			GfxButton moveForwards;
			GfxButton moveBackwards;
			GfxButton moveLeft;
			GfxButton moveRight;
			GfxButton leftShift;
			GfxButton leftCtrl;
		};
		GfxButton buttons[8];
	};
} GfxInput;

typedef struct
{
	i32 firstSide;
	i32 sideCount;
} GfxBrush;

typedef struct
{
	// arguments passed to main()
	i32 argCount;
	char **arguments;
	
	Arena arena;
	GfxInput input;
	
	v3 angles;
	v3 origin;
	
	sg_pipeline pipeline;
	sg_pipeline wirePipeline;
	sg_pass_action passAction;
	sg_buffer indexBuffer;
	sg_buffer wireIndexBuffer;
	
	GfxMesh specificFaceMesh;
	i32 specificFaceIndex; // draw only this mesh index
	GfxMesh worldMeshes[DEBUG_GFX_MAX_TEXTURES]; // every face that shares a texture is in 1 mesh.
	i32 worldMeshCount;
	GfxPoly faces[SRC_MAX_MAP_FACES];
	i32 faceCount;
	
	GfxMesh specificBrushMeshes[SRC_MAX_BRUSH_SIDES]; // one mesh for every brushside
	GfxMesh specificBrushSideMesh;
	i32 specificBrushIndex; // draw only this brush index
	i32 specificBrushSideIndex; // draw only this brushSide[] index
	GfxMesh brushMeshes[DEBUG_GFX_MAX_TEXTURES]; // every brush side that shares a texture is in 1 mesh.
	i32 brushMeshCount;
	GfxPoly brushSides[SRC_MAX_MAP_BRUSHSIDES];
	i32 brushSideCount;
	GfxBrush brushes[SRC_MAX_MAP_BRUSHES * 2]; // contains which brush sides belong to a brush
	i32 brushCount;
	
	i32 solidDrawType; // 0 = textured, 1 = solid, 2 = don't draw
	bool wireframe;
	bool drawWorld;
	bool drawBrushes;
	bool pickFace;
	bool pickBrush;
	
	GfxTexture textures[DEBUG_GFX_MAX_TEXTURES];
	i32 textureCount;
	
	GfxMesh meshes[DEBUG_GFX_MAX_MESHES];
	i32 meshCount;
	
	// buffer for rgb888 > rgba8888 conversion
	u32 rgba8888[8192 * 8192];
} GfxState;


#endif //DEBUG_RENDER_H
