/* date = November 14th 2022 11:43 am */

#ifndef DEBUG_RENDER_H
#define DEBUG_RENDER_H

#define DEBUG_GFX_MAX_TEXTURES 256
#define DEBUG_GFX_MAX_MESHES 512

#define DEBUG_GFX_FOV 90
#define DEBUG_GFX_NEARZ 4
#define DEBUG_GFX_FARZ 32768

struct GfxRayHit
{
	b32 hit;
	f32 distance;
	v3 endPosition;
};

struct GfxVertData
{
	v3 pos;
	v3 normal;
	v2 uv;
};

struct GfxMesh
{
	sg_buffer vertexBuffer;
	sg_buffer indexBuffer; // if invalid, then fallback is GfxState.indexBuffer
	sg_buffer wireIndexBuffer; // if invalid, then fallback is GfxState.wireIndexBuffer
	s32 vertCount;
	s32 indexCount; // if 0, then this will be calculated from vertCount
	s32 wireIndexCount; // if 0, then this will be calculated from vertCount
	s32 texture;
	v3 wireColour;
};

struct GfxPoly
{
	s32 texture;
	s32 vertCount;
	GfxVertData *verts;
};

struct GfxTexture
{
	sg_image img;
};

struct GfxButton
{
	b32 isDown;
	s32 actionCount; // amount of up & down events in this frame.
};

struct GfxInput
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
};

struct GfxBrush
{
	s32 firstSide;
	s32 sideCount;
};

struct GfxState
{
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
	s32 specificFaceIndex; // draw only this mesh index
	GfxMesh worldMeshes[DEBUG_GFX_MAX_TEXTURES]; // every face that shares a texture is in 1 mesh.
	s32 worldMeshCount;
	GfxPoly faces[SRC_MAX_MAP_FACES];
	s32 faceCount;
	
	GfxMesh specificBrushMeshes[SRC_MAX_BRUSH_SIDES]; // one mesh for every brushside
	GfxMesh specificBrushSideMesh;
	s32 specificBrushIndex; // draw only this brush index
	s32 specificBrushSideIndex; // draw only this brushSide[] index
	GfxMesh brushMeshes[DEBUG_GFX_MAX_TEXTURES]; // every brush side that shares a texture is in 1 mesh.
	s32 brushMeshCount;
	GfxPoly brushSides[SRC_MAX_MAP_BRUSHSIDES];
	s32 brushSideCount;
	GfxBrush brushes[SRC_MAX_MAP_BRUSHES]; // contains which brush sides belong to a brush
	s32 brushCount;
	
	s32 solidDrawType; // 0 = textured, 1 = solid, 2 = don't draw
	b32 wireframe;
	bool drawWorld;
	bool drawBrushes;
	bool pickFace;
	bool pickBrush;
	
	GfxTexture textures[DEBUG_GFX_MAX_TEXTURES];
	s32 textureCount;
	
	GfxMesh meshes[DEBUG_GFX_MAX_MESHES];
	s32 meshCount;
	
	// buffer for rgb888 > rgba8888 conversion
	u32 rgba8888[8192 * 8192];
};


#endif //DEBUG_RENDER_H
