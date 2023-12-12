
#ifdef GC_DEBUG
#define SOKOL_DEBUG
#endif
#define SOKOL_IMPL
#define SOKOL_D3D11
#define SOKOL_NO_ENTRY
#include "sokol/sokol_app.h"
#include "sokol/sokol_gfx.h"
#include "sokol/sokol_glue.h"
#include "imgui/imgui.h"
#include "sokol/sokol_imgui.h"

#include "shaders_compiled/world.h"
#include "shaders_compiled/wire.h"

#include "debug_render.h"

global GfxState *g_gfxState;

// NOTE: from https://www.mesa3d.org/
internal mat4 InvertMat4(mat4 in)
{
	f32 inv[16];
	f64 det;
	
	f32 *m = (f32 *)&in;
	
	inv[0] = m[5]  * m[10] * m[15] - 
		m[5]  * m[11] * m[14] - 
		m[9]  * m[6]  * m[15] + 
		m[9]  * m[7]  * m[14] +
		m[13] * m[6]  * m[11] - 
		m[13] * m[7]  * m[10];
	
    inv[4] = -m[4]  * m[10] * m[15] + 
		m[4]  * m[11] * m[14] + 
		m[8]  * m[6]  * m[15] - 
		m[8]  * m[7]  * m[14] - 
		m[12] * m[6]  * m[11] + 
		m[12] * m[7]  * m[10];
	
    inv[8] = m[4]  * m[9] * m[15] - 
		m[4]  * m[11] * m[13] - 
		m[8]  * m[5] * m[15] + 
		m[8]  * m[7] * m[13] + 
		m[12] * m[5] * m[11] - 
		m[12] * m[7] * m[9];
	
    inv[12] = -m[4]  * m[9] * m[14] + 
		m[4]  * m[10] * m[13] +
		m[8]  * m[5] * m[14] - 
		m[8]  * m[6] * m[13] - 
		m[12] * m[5] * m[10] + 
		m[12] * m[6] * m[9];
	
    inv[1] = -m[1]  * m[10] * m[15] + 
		m[1]  * m[11] * m[14] + 
		m[9]  * m[2] * m[15] - 
		m[9]  * m[3] * m[14] - 
		m[13] * m[2] * m[11] + 
		m[13] * m[3] * m[10];
	
    inv[5] = m[0]  * m[10] * m[15] - 
		m[0]  * m[11] * m[14] - 
		m[8]  * m[2] * m[15] + 
		m[8]  * m[3] * m[14] + 
		m[12] * m[2] * m[11] - 
		m[12] * m[3] * m[10];
	
    inv[9] = -m[0]  * m[9] * m[15] + 
		m[0]  * m[11] * m[13] + 
		m[8]  * m[1] * m[15] - 
		m[8]  * m[3] * m[13] - 
		m[12] * m[1] * m[11] + 
		m[12] * m[3] * m[9];
	
    inv[13] = m[0]  * m[9] * m[14] - 
		m[0]  * m[10] * m[13] - 
		m[8]  * m[1] * m[14] + 
		m[8]  * m[2] * m[13] + 
		m[12] * m[1] * m[10] - 
		m[12] * m[2] * m[9];
	
    inv[2] = m[1]  * m[6] * m[15] - 
		m[1]  * m[7] * m[14] - 
		m[5]  * m[2] * m[15] + 
		m[5]  * m[3] * m[14] + 
		m[13] * m[2] * m[7] - 
		m[13] * m[3] * m[6];
	
    inv[6] = -m[0]  * m[6] * m[15] + 
		m[0]  * m[7] * m[14] + 
		m[4]  * m[2] * m[15] - 
		m[4]  * m[3] * m[14] - 
		m[12] * m[2] * m[7] + 
		m[12] * m[3] * m[6];
	
    inv[10] = m[0]  * m[5] * m[15] - 
		m[0]  * m[7] * m[13] - 
		m[4]  * m[1] * m[15] + 
		m[4]  * m[3] * m[13] + 
		m[12] * m[1] * m[7] - 
		m[12] * m[3] * m[5];
	
    inv[14] = -m[0]  * m[5] * m[14] + 
		m[0]  * m[6] * m[13] + 
		m[4]  * m[1] * m[14] - 
		m[4]  * m[2] * m[13] - 
		m[12] * m[1] * m[6] + 
		m[12] * m[2] * m[5];
	
    inv[3] = -m[1] * m[6] * m[11] + 
		m[1] * m[7] * m[10] + 
		m[5] * m[2] * m[11] - 
		m[5] * m[3] * m[10] - 
		m[9] * m[2] * m[7] + 
		m[9] * m[3] * m[6];
	
    inv[7] = m[0] * m[6] * m[11] - 
		m[0] * m[7] * m[10] - 
		m[4] * m[2] * m[11] + 
		m[4] * m[3] * m[10] + 
		m[8] * m[2] * m[7] - 
		m[8] * m[3] * m[6];
	
    inv[11] = -m[0] * m[5] * m[11] + 
		m[0] * m[7] * m[9] + 
		m[4] * m[1] * m[11] - 
		m[4] * m[3] * m[9] - 
		m[8] * m[1] * m[7] + 
		m[8] * m[3] * m[5];
	
    inv[15] = m[0] * m[5] * m[10] - 
		m[0] * m[6] * m[9] - 
		m[4] * m[1] * m[10] + 
		m[4] * m[2] * m[9] + 
		m[8] * m[1] * m[6] - 
		m[8] * m[2] * m[5];
	
    det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];
	
	mat4 result = {};
    if (det == 0)
	{
		// TODO: return identity?
        return result;
	}
	
    det = 1.0 / det;
	
	for (s32 i = 0; i < 16; i++)
	{
		((f32 *)&result)[i] = inv[i] * (f32)det;
	}
	
	return result;
}

internal void AnglesToVectors(v3 angles, v3 *forwards, v3 *right, v3 *up)
{
	v3 radAngles = Vec3(ToRadians(angles.x),
						ToRadians(angles.y),
						ToRadians(angles.z));
	
	v3 cos;
	v3 sin;
	
	cos.x = CosF(radAngles.x);
	sin.x = SinF(radAngles.x);
	
	cos.y = CosF(radAngles.y);
	sin.y = SinF(radAngles.y);
	
	cos.z = CosF(radAngles.z);
	sin.z = SinF(radAngles.z);
	
	if (forwards)
	{
		forwards->x = cos.x * cos.y;
		forwards->y = cos.x * sin.y;
		forwards->z = -sin.x;
	}
	
	if (right)
	{
		right->x = (-1 * sin.z * sin.x * cos.y) + (-1 * cos.z * -sin.y);
		right->y = (-1 * sin.z * sin.x * sin.y) + (-1 * cos.z * cos.y);
		right->z = -1 * sin.z * cos.x;
	}
	
	if (up)
	{
		up->x = (cos.z * sin.x * cos.y + -sin.z * -sin.y);
		up->y = (cos.z * sin.x * sin.y + -sin.z * cos.y);
		up->z = cos.z * cos.x;
	}
}

// NOTE(GameChaos): if rgb888 is true, then it converts to rgba8888, if rgb888 is false it expects rgba8888 data
internal void DebugGfxAddTexture(u8 *data, s32 width, s32 height, b32 rgb888 = false)
{
	GfxState *state = g_gfxState;
	if (state->textureCount < DEBUG_GFX_MAX_TEXTURES)
	{
		s32 pixels = width * height;
		sg_image_desc desc = {};
		if (rgb888)
		{
			for (s32 i = 0; i < pixels; i++)
			{
				state->rgba8888[i] = (  (u32)data[i * 3 + 2] << 0
									  | (u32)data[i * 3 + 1] << 8
									  | (u32)data[i * 3 + 0] << 16
									  | (u32)255 << 24);
			}
			// state->rgba8888 is only used for rgb888 > rgba8888 conversion
			desc.data.subimage[0][0].ptr = state->rgba8888;
			desc.data.subimage[0][0].size = pixels * 4;
		}
		else
		{
			desc.data.subimage[0][0].ptr = data;
			desc.data.subimage[0][0].size = pixels * 4;
		}
		desc.width = width;
		desc.height = height;
		desc.min_filter = SG_FILTER_LINEAR_MIPMAP_LINEAR;
		desc.mag_filter = SG_FILTER_LINEAR;
		sg_image img = sg_make_image(&desc);
		if (img.id != SG_INVALID_ID)
		{
			GfxTexture tex = {};
			tex.img = img;
			state->textures[state->textureCount++] = tex;
		}
		else
		{
			ASSERT(0);
		}
	}
	else
	{
		ASSERT(0);
	}
}

internal void DebugGfxAddMiptexture(Arena *tempArena, GsrcMipTexture mipTexture, u8 *textureData)
{
	ArenaTemp arenaTmp = ArenaBeginTemp(tempArena);
	u8 *tempImgDataRgb888 = (u8 *)ArenaAlloc(tempArena, mipTexture.width * mipTexture.height * 3);
	u32 pixels = mipTexture.width * mipTexture.height;
	u8 *palette = textureData + 2 + mipTexture.offsets[0] + pixels + (pixels >> 2) + (pixels >> 4) + (pixels >> 6);
	for (u32 pix = 0; pix < pixels; pix++)
	{
		u32 indexOffset = mipTexture.offsets[0] + pix;
		tempImgDataRgb888[pix * 3 + 0] = palette[textureData[indexOffset] * 3 + 2];
		tempImgDataRgb888[pix * 3 + 1] = palette[textureData[indexOffset] * 3 + 1];
		tempImgDataRgb888[pix * 3 + 2] = palette[textureData[indexOffset] * 3 + 0];
	}
	DebugGfxAddTexture(tempImgDataRgb888, mipTexture.width, mipTexture.height, true);
	ArenaEndTemp(arenaTmp);
}

internal GfxVertData *VertsToGfxVertData(Arena *arena, Verts *poly, v3 normal, v4 s, v4 t)
{
	GfxVertData *result = (GfxVertData *)ArenaAlloc(arena, poly->vertCount * sizeof(*result));
	if (result)
	{
		for (s32 v = 0; v < poly->vertCount; v++)
		{
			GfxVertData vertData;
			vertData.pos = poly->verts[v];
			vertData.normal = normal;
			vertData.uv.x = Dot(poly->verts[v], s.xyz) + s.w;
			vertData.uv.y = Dot(poly->verts[v], t.xyz) + t.w;
			result[v] = vertData;
		}
	}
	
	return result;
}

internal GfxMesh DebugGfxCreateMesh(GfxVertData *verts, s32 vertCount, s32 textureIndex, v3 wireColour = {0, 0, 0})
{
	GfxMesh result = {};
	sg_buffer_desc vertexDesc = {};
	vertexDesc.type = SG_BUFFERTYPE_VERTEXBUFFER;
	vertexDesc.data.ptr = verts;
	vertexDesc.data.size = vertCount * sizeof(*verts);
	
	result.vertexBuffer = sg_make_buffer(&vertexDesc);
	ASSERT(result.vertexBuffer.id != SG_INVALID_ID);
	result.vertCount = vertCount;
	result.wireColour = wireColour;
	if (textureIndex >= 0 && textureIndex < g_gfxState->textureCount)
	{
		// NOTE(GameChaos): 0 is reserved for missing texture texture
		result.texture = textureIndex;
	}
	ASSERT(textureIndex < g_gfxState->textureCount);
	
	return result;
}

internal void DebugGfxTextureVecsFromNormal(v3 normal, v4 *sOut, v4 *tOut)
{
	v3 nonParallel = GetNonParallelVector(normal);
	v3 i = Cross(normal, nonParallel);
	v3 j = Cross(normal, i);
	sOut->xyz = i * 0.25f;
	tOut->xyz = j * 0.25f;
	sOut->w = 0;
	tOut->w = 0;
}
internal void DebugGfxAddFace(Verts *poly, v3 normal, s32 textureIndex = -1, v4 s = {}, v4 t = {})
{
	GfxState *state = g_gfxState;
	if (state->faceCount < SRC_MAX_MAP_FACES
		&& poly->vertCount > 0
		&& poly->vertCount < SRC_MAX_SIDE_VERTS)
	{
		GfxPoly face = {};
		face.s = s;
		face.t = t;
		if (textureIndex < 0)
		{
			DebugGfxTextureVecsFromNormal(normal, &s, &t);
		}
		face.verts = VertsToGfxVertData(&state->arena, poly, normal, s, t);
		face.vertCount = poly->vertCount;
		if (textureIndex >= 0 && textureIndex < DEBUG_GFX_MAX_TEXTURES)
		{
			// NOTE(GameChaos): 0 is reserved for missing texture texture
			face.texture = textureIndex + 1;
		}
		state->faces[state->faceCount++] = face;
	}
	else
	{
		ASSERT(0);
	}
}

internal void DebugGfxAddBrushSide(Verts *poly, v3 normal, s32 textureIndex = -1, v4 s = {}, v4 t = {})
{
	GfxState *state = g_gfxState;
	if (state->brushSideCount < SRC_MAX_MAP_BRUSHSIDES
		&& poly->vertCount > 0
		&& poly->vertCount < SRC_MAX_SIDE_VERTS)
	{
		GfxPoly side = {};
		if (textureIndex < 0)
		{
			DebugGfxTextureVecsFromNormal(normal, &s, &t);
		}
		side.verts = VertsToGfxVertData(&state->arena, poly, normal, s, t);
		side.vertCount = poly->vertCount;
		if (textureIndex >= 0 && textureIndex < DEBUG_GFX_MAX_TEXTURES)
		{
			// NOTE(GameChaos): 0 is reserved for missing texture texture
			side.texture = textureIndex + 1;
		}
		state->brushSides[state->brushSideCount++] = side;
	}
	else
	{
		ASSERT(0);
	}
}

internal void DebugGfxAddBrush(s32 sideCount)
{
	GfxState *state = g_gfxState;
	if (state->brushCount < ARRAYCOUNT(state->brushes))
	{
		GfxBrush brush = {};
		brush.sideCount = sideCount;
		if (state->brushCount > 0)
		{
			brush.firstSide = (state->brushes[state->brushCount - 1].firstSide
							   + state->brushes[state->brushCount - 1].sideCount);
		}
		if (brush.firstSide + brush.sideCount < ARRAYCOUNT(state->brushSides))
		{
			state->brushes[state->brushCount++] = brush;
		}
		else
		{
			ASSERT(0);
		}
	}
	else
	{
		ASSERT(0);
	}
}

internal void DebugGfxAddMesh(Verts *poly, v3 normal, v3 wireColour = {0, 0, 0}, s32 textureIndex = -1, v4 s = {}, v4 t = {})
{
	GfxState *state = g_gfxState;
	if (state->meshCount < DEBUG_GFX_MAX_MESHES
		&& poly->vertCount > 0
		&& poly->vertCount < SRC_MAX_SIDE_VERTS)
	{
		ArenaTemp arenaTmp = ArenaBeginTemp(&state->arena);
		if (textureIndex < 0)
		{
			DebugGfxTextureVecsFromNormal(normal, &s, &t);
		}
		GfxVertData *tempVerts = VertsToGfxVertData(&state->arena, poly, normal, s, t);
		// NOTE(GameChaos): 0 is reserved for missing texture texture
		GfxMesh mesh = DebugGfxCreateMesh(tempVerts, poly->vertCount, textureIndex + 1, wireColour);
		state->meshes[state->meshCount++] = mesh;
		ArenaEndTemp(arenaTmp);
	}
	else
	{
		ASSERT(0);
	}
}

internal void DebugGfxLog(const char* message, void* user_data)
{
	Print("SOKOL: %s\n", message);
}

internal void DebugGfxInit(void *userData)
{
	GfxState *state = (GfxState *)userData;
	state->specificFaceIndex = -1;
	state->specificBrushIndex = -1;
	state->specificBrushSideIndex = -1;
	state->drawWorld = true;
	state->drawBrushes = true;
	sg_desc desc = {};
	desc.context = sapp_sgcontext();
	desc.buffer_pool_size = DEBUG_GFX_MAX_MESHES * 2;
	desc.image_pool_size = DEBUG_GFX_MAX_TEXTURES;
	desc.logger.log_cb = DebugGfxLog;
	sg_setup(&desc);
	
	simgui_desc_t imguiDesc = {};
	simgui_setup(&imguiDesc);
	
	u32 debugTexture[8 * 8] = {
		0xffdfdfdf, 0xffdfdfdf, 0xffdfdfdf, 0xffdfdfdf, 0xffbfbfbf, 0xffbfbfbf, 0xffbfbfbf, 0xffbfbfbf,
		0xffdfdfdf, 0xffdfdfdf, 0xffdfdfdf, 0xffdfdfdf, 0xffbfbfbf, 0xffbfbfbf, 0xffbfbfbf, 0xffbfbfbf,
		0xffdfdfdf, 0xffdfdfdf, 0xffdfdfdf, 0xffdfdfdf, 0xffbfbfbf, 0xffbfbfbf, 0xffbfbfbf, 0xffbfbfbf,
		0xffdfdfdf, 0xffdfdfdf, 0xffdfdfdf, 0xffdfdfdf, 0xffbfbfbf, 0xffbfbfbf, 0xffbfbfbf, 0xffbfbfbf,
		0xffbfbfbf, 0xffbfbfbf, 0xffbfbfbf, 0xffbfbfbf, 0xffdfdfdf, 0xffdfdfdf, 0xffdfdfdf, 0xffdfdfdf,
		0xffbfbfbf, 0xffbfbfbf, 0xffbfbfbf, 0xffbfbfbf, 0xffdfdfdf, 0xffdfdfdf, 0xffdfdfdf, 0xffdfdfdf,
		0xffbfbfbf, 0xffbfbfbf, 0xffbfbfbf, 0xffbfbfbf, 0xffdfdfdf, 0xffdfdfdf, 0xffdfdfdf, 0xffdfdfdf,
		0xffbfbfbf, 0xffbfbfbf, 0xffbfbfbf, 0xffbfbfbf, 0xffdfdfdf, 0xffdfdfdf, 0xffdfdfdf, 0xffdfdfdf,
    };
	
	// NOTE(GameChaos): 0 texture.
	DebugGfxAddTexture((u8 *)debugTexture, 8, 8);
	
	sg_pipeline_desc pipelineDesc = {};
	pipelineDesc.layout.attrs[ATTR_world_vs_iPos].format = SG_VERTEXFORMAT_FLOAT3;
	pipelineDesc.layout.attrs[ATTR_world_vs_iNormal].format = SG_VERTEXFORMAT_FLOAT3;
	pipelineDesc.layout.attrs[ATTR_world_vs_iUv].format = SG_VERTEXFORMAT_FLOAT2;
	pipelineDesc.shader = sg_make_shader(world_shader_desc(sg_query_backend()));
	pipelineDesc.index_type = SG_INDEXTYPE_UINT16;
	pipelineDesc.cull_mode = SG_CULLMODE_BACK;
	pipelineDesc.depth.compare = SG_COMPAREFUNC_LESS_EQUAL;
	pipelineDesc.depth.write_enabled = true;
	pipelineDesc.label = "pipeline";
	
	sg_pipeline_desc wirePipelineDesc = {};
	wirePipelineDesc.layout.attrs[ATTR_world_vs_iPos].format = SG_VERTEXFORMAT_FLOAT3;
	wirePipelineDesc.layout.attrs[ATTR_world_vs_iNormal].format = SG_VERTEXFORMAT_FLOAT3;
	wirePipelineDesc.layout.attrs[ATTR_world_vs_iUv].format = SG_VERTEXFORMAT_FLOAT2;
	wirePipelineDesc.shader = sg_make_shader(wire_shader_desc(sg_query_backend()));
	wirePipelineDesc.primitive_type = SG_PRIMITIVETYPE_LINES;
	wirePipelineDesc.index_type = SG_INDEXTYPE_UINT16;
	wirePipelineDesc.cull_mode = SG_CULLMODE_BACK;
	wirePipelineDesc.depth.compare = SG_COMPAREFUNC_LESS_EQUAL;
	wirePipelineDesc.depth.write_enabled = true;
	wirePipelineDesc.label = "pipeline";
	
	state->passAction = {};
	state->passAction.colors[0].action = SG_ACTION_CLEAR;
	state->passAction.colors[0].value = {0, 0, 0, 1};
	
	state->pipeline = sg_make_pipeline(&pipelineDesc);
	state->wirePipeline = sg_make_pipeline(&wirePipelineDesc);
	
	ArenaTemp arenaTemp = ArenaBeginTemp(&state->arena);
	s32 maxIndices = 512 * SRC_MAX_SIDE_VERTS; // SRC_MAX_MAP_BRUSHSIDES * SRC_MAX_SIDE_VERTS;
	u16 *indices = (u16 *)ArenaAlloc(&state->arena, maxIndices * sizeof(*indices));
	for (u16 i = 0; i < maxIndices - 2; i += 3)
	{
		indices[i + 0] = 0;
		indices[i + 1] = i / 3 + 1;
		indices[i + 2] = i / 3 + 2;
	}
	sg_buffer_desc indexDesc = {};
	indexDesc.type = SG_BUFFERTYPE_INDEXBUFFER;
	indexDesc.data.ptr = indices;
	indexDesc.data.size = sizeof(*indices) * maxIndices;
	
	u16 *wireIndices = (u16 *)ArenaAlloc(&state->arena, maxIndices * sizeof(*wireIndices));
	s32 wireIndexCount = 0;
	wireIndices[wireIndexCount++] = 0;
	wireIndices[wireIndexCount++] = 1;
	for (u16 i = 2; i < maxIndices - 1; i += 4)
	{
		wireIndices[i + 0] = (i + 2) / 4 + 0;
		wireIndices[i + 1] = (i + 2) / 4 + 1;
		wireIndices[i + 2] = (i + 2) / 4 + 1;
		wireIndices[i + 3] = 0;
	}
	sg_buffer_desc wireIndexDesc = {};
	wireIndexDesc.type = SG_BUFFERTYPE_INDEXBUFFER;
	wireIndexDesc.data.ptr = wireIndices;
	wireIndexDesc.data.size = sizeof(*wireIndices) * maxIndices;
	
	state->indexBuffer = sg_make_buffer(&indexDesc);
	state->wireIndexBuffer = sg_make_buffer(&wireIndexDesc);
	
	ArenaEndTemp(arenaTemp);
	
	BSPMain(state->argCount, state->arguments);
}

internal void DrawMesh(GfxState *state, GfxMesh *mesh, mat4 mvp, b32 solid)
{
	sg_bindings bindings = {};
	bindings.vertex_buffers[0] = mesh->vertexBuffer;
	s32 indexCount = mesh->indexCount;
	if (indexCount > 0)
	{
		bindings.index_buffer = mesh->indexBuffer;
	}
	else
	{
		indexCount = (mesh->vertCount - 2) * 3;
		bindings.index_buffer = state->indexBuffer;
	}
	if (solid)
	{
		bindings.fs_images[SLOT_tex] = state->textures[0].img;
	}
	else
	{
		bindings.fs_images[SLOT_tex] = state->textures[mesh->texture].img;
	}
	sg_apply_bindings(&bindings);
	
	world_vs_params_t vs_params = {};
	vs_params.mvp = mvp;
	vs_params.uCamOrigin = state->origin;
	
	sg_range vsParamsRange = SG_RANGE(vs_params);
	sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_world_vs_params, &vsParamsRange);
	
	sg_draw(0, indexCount, 1);
}

internal void DrawMeshWire(GfxState *state, GfxMesh *mesh, mat4 mvp)
{
	sg_bindings bindings = {};
	bindings.vertex_buffers[0] = mesh->vertexBuffer;
	s32 indexCount = mesh->wireIndexCount;
	if (indexCount > 0)
	{
		bindings.index_buffer = mesh->wireIndexBuffer;
	}
	else
	{
		indexCount = (mesh->vertCount - 2) * 4 + 2;
		bindings.index_buffer = state->wireIndexBuffer;
	}
	sg_apply_bindings(&bindings);
	
	wire_vs_params_t vs_params = {};
	vs_params.mvp = mvp;
	vs_params.uColour = mesh->wireColour;
	
	sg_range vsParamsRange = SG_RANGE(vs_params);
	sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_world_vs_params, &vsParamsRange);
	
	sg_draw(0, indexCount, 1);
}

internal b32 ImGuiCustomSlider(const char *label, s32 *value, s32 min, s32 max, s32 defaultValue)
{
	b32 result = ImGui::SliderInt(label, value, min, max);
	ImGui::SameLine();
	ImVec2 buttonSize = {ImGui::GetFrameHeight(), ImGui::GetFrameHeight()};
	ImGui::PushButtonRepeat(true);
	ImGui::PushID(label + 1);
	if (ImGui::Button("-", buttonSize))
	{
		(*value)--;
		result = true;
	}
	ImGui::PopID();
	ImGui::SameLine();
	ImGui::PushID(label + 2);
	if (ImGui::Button("+", buttonSize))
	{
		(*value)++;
		result = true;
	}
	ImGui::PopID();
	ImGui::PopButtonRepeat();
	ImGui::SameLine();
	ImGui::PushID(label + 3);
	if (ImGui::Button("Reset"))
	{
		(*value) = defaultValue;
		result = true;
	}
	ImGui::PopID();
	(*value) = CLAMP(min, (*value), max);
	return result;
}

inline GfxRayHit RayPolygonIntersect(GfxPoly *polygon, v3 rayPos, v3 rayDir)
{
	GfxRayHit result = {};
	
	// check if ray and plane are parallel
	v3 normal = polygon->verts[0].normal;
	f32 rayDirDotNormal = Dot(normal, rayDir); 
    if (HMM_ABS(rayDirDotNormal) < 0.0001f) // almost 0 
	{
		return result;
	}
	
	result.distance = (Dot(normal, polygon->verts[0].pos) - Dot(normal, rayPos)) / rayDirDotNormal;
	
	// check if the triangle is behind the ray
	if (result.distance <= 0.0001f)
	{
		return result;
	}
	
	result.endPosition = rayPos + result.distance * rayDir; 
	result.hit = true; // assume that we're inside
	
	// check if end point is inside the polygon
	for (s32 v = 0; v < polygon->vertCount; v++)
	{
		v3 vert0 = polygon->verts[v].pos;
		v3 vert1 = polygon->verts[(v + 1) % polygon->vertCount].pos;
		v3 edge = vert1 - vert0;
		v3 vp0 = result.endPosition - vert0;
		v3 cross = Cross(vp0, edge);
		f32 dot = Dot(normal, cross);
		if (dot < 0.0f)
		{
			// ray doesn't hit the polygon
			result.hit = false;
			break;
		}
	}
	
	return result;
}

internal void UpdateSpecificBrush(GfxState *state)
{
	//state->specificBrushIndex = CLAMP(-1, state->specificBrushIndex, state->brushCount - 1);
	if (state->specificBrushIndex >= 0)
	{
		// create specific brush mesh
		GfxBrush brush = state->brushes[state->specificBrushIndex];
		// NOTE(GameChaos): destroy buffers
		for (s32 i = 0; i < ARRAYCOUNT(state->specificBrushMeshes); i++)
		{
			GfxMesh *mesh = &state->specificBrushMeshes[i];
			sg_destroy_buffer(mesh->vertexBuffer);
			sg_destroy_buffer(mesh->indexBuffer);
			sg_destroy_buffer(mesh->wireIndexBuffer);
		}
		// NOTE(GameChaos): create new buffers
		for (s32 i = 0; i < brush.sideCount; i++)
		{
			GfxMesh *mesh = &state->specificBrushMeshes[i];
			*mesh = {};
			GfxPoly *side = &state->brushSides[brush.firstSide + i];
			*mesh = DebugGfxCreateMesh(side->verts, side->vertCount, side->texture, {0, 1, 1});
		}
	}
}

internal void UpdateSpecificFace(GfxState *state)
{
	//state->specificFaceIndex = CLAMP(-1, state->specificFaceIndex, state->faceCount - 1);
	if (state->specificFaceIndex >= 0)
	{
		// create specific face mesh
		GfxMesh *mesh = &state->specificFaceMesh;
		sg_destroy_buffer(mesh->vertexBuffer);
		sg_destroy_buffer(mesh->indexBuffer);
		sg_destroy_buffer(mesh->wireIndexBuffer);
		*mesh = {};
		GfxPoly *face = &state->faces[state->specificFaceIndex];
		*mesh = DebugGfxCreateMesh(face->verts, face->vertCount, face->texture, {1, 1, 0});
	}
}

internal void DebugGfxFrame(void *userData)
{
	GfxState *state = (GfxState *)userData;
	
	simgui_frame_desc_t imguiFrameDesc = {
		sapp_width(),
		sapp_height(),
		sapp_frame_duration(),
		sapp_dpi_scale()
	};
	simgui_new_frame(&imguiFrameDesc);
	
	if (state->textureCount <= 1)
	{
		return;
	}
	
	if (state->worldMeshCount <= 0 && state->faceCount > 0)
	{
		// make face mesh!
		ArenaTemp arenaTmp = ArenaBeginTemp(&state->arena);
		s64 maxVerts = SRC_MAX_MAP_FACES * SRC_MAX_SIDE_VERTS;
		GfxVertData *tempVerts = (GfxVertData *)ArenaAlloc(&state->arena, maxVerts * sizeof(*tempVerts));
		u16 *indices = (u16 *)ArenaAlloc(&state->arena, maxVerts * sizeof(*indices));
		u16 *wireIndices = (u16 *)ArenaAlloc(&state->arena, maxVerts * sizeof(*wireIndices));
		for (s32 tex = 0; tex < state->textureCount; tex++)
		{
			s32 indexCount = 0;
			s32 wireIndexCount = 0;
			s32 vertCount = 0;
			for (s32 faceInd = 0; faceInd < state->faceCount; faceInd++)
			{
				GfxPoly *face = &state->faces[faceInd];
				if (face->texture == tex)
				{
					for (u16 i = (u16)vertCount;
						 i < vertCount + face->vertCount - 2;
						 i++)
					{
						indices[indexCount++] = (u16)vertCount;
						indices[indexCount++] = i + 1;
						indices[indexCount++] = i + 2;
					}
					
					for (u16 i = 0;
						 i < face->vertCount;
						 i++)
					{
						wireIndices[wireIndexCount++] = i + (u16)vertCount;
						wireIndices[wireIndexCount++] = (i + 1) % (u16)face->vertCount + (u16)vertCount;
					}
					
					Mem_Copy(face->verts, &tempVerts[vertCount], face->vertCount * sizeof(*face->verts));
					vertCount += face->vertCount;
					ASSERT(vertCount < maxVerts);
				}
			}
			
			if (vertCount > 0)
			{
				sg_buffer_desc vertexDesc = {};
				vertexDesc.type = SG_BUFFERTYPE_VERTEXBUFFER;
				vertexDesc.data.ptr = tempVerts;
				vertexDesc.data.size = vertCount * sizeof(*tempVerts);
				
				sg_buffer_desc indexDesc = {};
				indexDesc.type = SG_BUFFERTYPE_INDEXBUFFER;
				indexDesc.data.ptr = indices;
				indexDesc.data.size = sizeof(*indices) * indexCount;
				
				sg_buffer_desc wireIndexDesc = {};
				wireIndexDesc.type = SG_BUFFERTYPE_INDEXBUFFER;
				wireIndexDesc.data.ptr = wireIndices;
				wireIndexDesc.data.size = sizeof(*wireIndices) * wireIndexCount;
				
				GfxMesh mesh = {};
				mesh.vertexBuffer = sg_make_buffer(&vertexDesc);
				mesh.indexBuffer = sg_make_buffer(&indexDesc);
				mesh.wireIndexBuffer = sg_make_buffer(&wireIndexDesc);
				mesh.vertCount = vertCount;
				mesh.indexCount = indexCount;
				mesh.wireIndexCount = wireIndexCount;
				mesh.texture = tex;
				mesh.wireColour = {1, 1, 0};
				state->worldMeshes[state->worldMeshCount++] = mesh;
			}
		}
		
		ArenaEndTemp(arenaTmp);
	}
	
	if (state->brushMeshCount <= 0 && state->brushSideCount > 0)
	{
		// make brush meshes!
		ArenaTemp arenaTmp = ArenaBeginTemp(&state->arena);
		s64 maxVerts = SRC_MAX_MAP_FACES * SRC_MAX_SIDE_VERTS;
		GfxVertData *tempVerts = (GfxVertData *)ArenaAlloc(&state->arena, maxVerts * sizeof(*tempVerts));
		u16 *indices = (u16 *)ArenaAlloc(&state->arena, maxVerts * sizeof(*indices));
		u16 *wireIndices = (u16 *)ArenaAlloc(&state->arena, maxVerts * sizeof(*wireIndices));
		for (s32 tex = 0; tex < state->textureCount; tex++)
		{
			s32 indexCount = 0;
			s32 wireIndexCount = 0;
			s32 vertCount = 0;
			for (s32 sideInd = 0; sideInd < state->brushSideCount; sideInd++)
			{
				GfxPoly *side = &state->brushSides[sideInd];
				if (side->texture == tex)
				{
					for (u16 i = (u16)vertCount;
						 i < vertCount + side->vertCount - 2;
						 i++)
					{
						indices[indexCount++] = (u16)vertCount;
						indices[indexCount++] = i + 1;
						indices[indexCount++] = i + 2;
					}
					
					for (u16 i = 0;
						 i < side->vertCount;
						 i++)
					{
						wireIndices[wireIndexCount++] = i + (u16)vertCount;
						wireIndices[wireIndexCount++] = (i + 1) % (u16)side->vertCount + (u16)vertCount;
					}
					
					Mem_Copy(side->verts, &tempVerts[vertCount], side->vertCount * sizeof(*side->verts));
					vertCount += side->vertCount;
					ASSERT(vertCount < maxVerts);
				}
			}
			
			if (vertCount > 0)
			{
				sg_buffer_desc vertexDesc = {};
				vertexDesc.type = SG_BUFFERTYPE_VERTEXBUFFER;
				vertexDesc.data.ptr = tempVerts;
				vertexDesc.data.size = vertCount * sizeof(*tempVerts);
				
				sg_buffer_desc indexDesc = {};
				indexDesc.type = SG_BUFFERTYPE_INDEXBUFFER;
				indexDesc.data.ptr = indices;
				indexDesc.data.size = sizeof(*indices) * indexCount;
				
				sg_buffer_desc wireIndexDesc = {};
				wireIndexDesc.type = SG_BUFFERTYPE_INDEXBUFFER;
				wireIndexDesc.data.ptr = wireIndices;
				wireIndexDesc.data.size = sizeof(*wireIndices) * wireIndexCount;
				
				GfxMesh mesh = {};
				mesh.vertexBuffer = sg_make_buffer(&vertexDesc);
				mesh.indexBuffer = sg_make_buffer(&indexDesc);
				mesh.wireIndexBuffer = sg_make_buffer(&wireIndexDesc);
				mesh.vertCount = vertCount;
				mesh.indexCount = indexCount;
				mesh.wireIndexCount = wireIndexCount;
				mesh.texture = tex;
				mesh.wireColour = {0, 1, 1};
				state->brushMeshes[state->brushMeshCount++] = mesh;
			}
		}
		
		ArenaEndTemp(arenaTmp);
	}
	
	GfxInput *input = &state->input;
	
	if (sapp_mouse_locked())
	{
		state->angles.x += input->mouseDelta.y * 0.022f * 3.0f;
		state->angles.y -= input->mouseDelta.x * 0.022f * 3.0f;
		state->angles.x = CLAMP(-89, state->angles.x, 89);
	}
	v3 forwards = {};
	v3 right = {};
	AnglesToVectors(state->angles, &forwards, &right, NULL);
	
	{
		v2 move = {
			(f32)!!input->moveForwards.isDown - (f32)!!input->moveBackwards.isDown,
			(f32)!!input->moveRight.isDown - (f32)!!input->moveLeft.isDown
		};
		{
			f32 len = Length(move);
			if (len > 0)
			{
				move = move / len;
			}
		}
		
		v3 moveDir = (forwards * move.x) + (right * move.y);
		float speed = 500.0f;
		if (input->leftShift.isDown)
		{
			speed *= 0.25f;
		}
		if (input->leftCtrl.isDown)
		{
			speed *= 4.0f;
		}
		state->origin += moveDir * ((f32)sapp_frame_duration() * speed);
	}
	
	mat4 proj = Perspective(DEBUG_GFX_FOV, sapp_widthf() / sapp_heightf(), DEBUG_GFX_NEARZ, DEBUG_GFX_FARZ);
	mat4 view = LookAt(state->origin, state->origin + forwards, Vec3(0, 0, 1));
	mat4 view_proj = MultiplyMat4(proj, view);
	ImGui::BeginDisabled(state->pickBrush || state->pickFace);
	sg_begin_default_pass(&state->passAction, sapp_width(), sapp_height());
	ImGui::Text("Wireframe: %i\nSolid type: %i", state->wireframe, state->solidDrawType);
	ImGui::Checkbox("Draw world faces", &state->drawWorld);
	ImGui::Checkbox("Draw brushes", &state->drawBrushes);
	ImGui::Text("Draw specific face");
	if (ImGuiCustomSlider("##Draw specific face", &state->specificFaceIndex, -1, state->faceCount - 1, -1))
	{
		UpdateSpecificFace(state);
	}
	
	ImGui::Text("Draw specific brush");
	if (ImGuiCustomSlider("##Draw specific brush", &state->specificBrushIndex, -1, state->brushCount - 1, -1))
	{
		UpdateSpecificBrush(state);
	}
	
	ImGui::Text("Draw specific brush side");
	if (ImGuiCustomSlider("##Draw specific brush side", &state->specificBrushSideIndex, -1, state->brushSideCount - 1, -1))
	{
		if (state->specificBrushSideIndex >= 0)
		{
			// create specific brush side mesh
			GfxMesh *mesh = &state->specificBrushSideMesh;
			sg_destroy_buffer(mesh->vertexBuffer);
			sg_destroy_buffer(mesh->indexBuffer);
			sg_destroy_buffer(mesh->wireIndexBuffer);
			*mesh = {};
			GfxPoly *side = &state->brushSides[state->specificBrushSideIndex];
			*mesh = DebugGfxCreateMesh(side->verts, side->vertCount, side->texture, {0, 1, 0});
		}
	}
	
	if (ImGui::Button("Pick face"))
	{
		state->specificFaceIndex = -1;
		state->pickFace = true;
	}
	
	if (ImGui::Button("Pick brush"))
	{
		state->specificBrushIndex = -1;
		state->pickBrush = true;
	}
	
	if (state->specificFaceIndex >= 0)
	{
		GfxPoly *face = &state->faces[state->specificFaceIndex];
		ImGui::Text("Face %i\ns: %f %f %f %f\nt: %f %f %f %f",
					state->specificFaceIndex,
					face->s.x, face->s.y, face->s.z, face->s.w,
					face->t.x, face->t.y, face->t.z, face->s.w);
	}
	
	if (state->specificBrushIndex >= 0)
	{
		GfxBrush brush = state->brushes[state->specificBrushIndex];
		ImGui::Text("Brush %i info:\nFirst side: %i\nSide count: %i",
					state->specificBrushIndex, brush.firstSide, brush.sideCount);
	}
	
	if (state->specificBrushSideIndex >= 0)
	{
		GfxPoly *side = &state->brushSides[state->specificBrushSideIndex];
		ImGui::Text("Brush side %i info:\nTexture: %i\nVert count: %i\nNormal: %f %f %f",
					state->specificBrushSideIndex, side->texture, side->vertCount,
					side->verts->normal.x, side->verts->normal.y, side->verts->normal.z);
	}
	
	ImGui::EndDisabled();
	
	if (state->pickBrush || state->pickFace)
	{
		if (input->mouse1.isDown)
		{
			mat4 viewCentred = LookAt({}, forwards, {0, 0, 1});
			mat4 inv = InvertMat4(proj * viewCentred);
			v3 rayDir = {
				(input->mousePos.x / sapp_widthf()) * 2 - 1,
				1 - (input->mousePos.y / sapp_heightf()) * 2,
				0
			};
			rayDir = (inv * Vec4(rayDir.x, rayDir.y, rayDir.z, 1)).xyz;
			
			if (state->pickBrush)
			{
				GfxRayHit closestHit = {};
				s32 hitBrushInd = -1;
				for (s32 brushInd = 0; brushInd < state->brushCount; brushInd++)
				{
					GfxBrush brush = state->brushes[brushInd];
					for (s32 sideInd = brush.firstSide;
						 sideInd < brush.firstSide + brush.sideCount;
						 sideInd++)
					{
						GfxRayHit hit = RayPolygonIntersect(&state->brushSides[sideInd], state->origin, rayDir);
						if (hit.hit && (!closestHit.hit || hit.distance < closestHit.distance))
						{
							closestHit = hit;
							hitBrushInd = brushInd;
						}
					}
				}
				if (closestHit.hit)
				{
					state->specificBrushIndex = hitBrushInd;
					UpdateSpecificBrush(state);
				}
				rayDir = rayDir;
			}
			else if (state->pickFace)
			{
				GfxRayHit closestHit = {};
				s32 hitFaceInd = -1;
				for (s32 faceInd = 0; faceInd < state->faceCount; faceInd++)
				{
					GfxRayHit hit = RayPolygonIntersect(&state->faces[faceInd], state->origin, rayDir);
					if (hit.hit && (!closestHit.hit || hit.distance < closestHit.distance))
					{
						closestHit = hit;
						hitFaceInd = faceInd;
					}
				}
				if (closestHit.hit)
				{
					state->specificFaceIndex = hitFaceInd;
					UpdateSpecificFace(state);
				}
				rayDir = rayDir;
			}
			state->pickFace = false;
			state->pickBrush = false;
		}
	}
	
	sg_apply_pipeline(state->pipeline);
	for (s32 i = 0; i < state->meshCount; i++)
	{
		GfxMesh *mesh = &state->meshes[i];
		if (state->solidDrawType <= 1)
		{
			DrawMesh(state, mesh, view_proj, state->solidDrawType);
		}
	}
	
	if (state->specificFaceIndex < 0)
	{
		if (state->drawWorld)
		{
			for (s32 i = 0; i < state->worldMeshCount; i++)
			{
				GfxMesh *mesh = &state->worldMeshes[i];
				if (state->solidDrawType <= 1)
				{
					DrawMesh(state, mesh, view_proj, state->solidDrawType);
				}
			}
		}
	}
	else
	{
		GfxMesh *mesh = &state->specificFaceMesh;
		if (state->solidDrawType <= 1)
		{
			DrawMesh(state, mesh, view_proj, state->solidDrawType);
		}
	}
	
	if (state->specificBrushIndex < 0)
	{
		if (state->drawBrushes)
		{
			for (s32 i = 0; i < state->brushMeshCount; i++)
			{
				GfxMesh *mesh = &state->brushMeshes[i];
				if (state->solidDrawType <= 1)
				{
					DrawMesh(state, mesh, view_proj, state->solidDrawType);
				}
			}
		}
	}
	else
	{
		GfxBrush brush = state->brushes[state->specificBrushIndex];
		for (s32 i = 0; i < brush.sideCount; i++)
		{
			GfxMesh *mesh = &state->specificBrushMeshes[i];
			if (state->solidDrawType <= 1)
			{
				DrawMesh(state, mesh, view_proj, state->solidDrawType);
			}
		}
	}
	
	if (state->specificBrushSideIndex >= 0)
	{
		GfxMesh *mesh = &state->specificBrushSideMesh;
		if (state->solidDrawType <= 1)
		{
			DrawMesh(state, mesh, view_proj, state->solidDrawType);
		}
	}
	
	if (state->wireframe)
	{
		sg_apply_pipeline(state->wirePipeline);
		for (s32 i = 0; i < state->meshCount; i++)
		{
			GfxMesh *mesh = &state->meshes[i];
			DrawMeshWire(state, mesh, view_proj);
		}
		
		if (state->specificFaceIndex < 0)
		{
			if (state->drawWorld)
			{
				for (s32 i = 0; i < state->worldMeshCount; i++)
				{
					GfxMesh *mesh = &state->worldMeshes[i];
					DrawMeshWire(state, mesh, view_proj);
				}
			}
		}
		else
		{
			GfxMesh *mesh = &state->specificFaceMesh;
			DrawMeshWire(state, mesh, view_proj);
		}
		
		if (state->specificBrushIndex < 0)
		{
			if (state->drawBrushes)
			{
				for (s32 i = 0; i < state->brushMeshCount; i++)
				{
					GfxMesh *mesh = &state->brushMeshes[i];
					DrawMeshWire(state, mesh, view_proj);
				}
			}
		}
		else
		{
			GfxBrush brush = state->brushes[state->specificBrushIndex];
			for (s32 i = 0; i < brush.sideCount; i++)
			{
				GfxMesh *mesh = &state->specificBrushMeshes[i];
				DrawMeshWire(state, mesh, view_proj);
			}
		}
		
		if (state->specificBrushSideIndex >= 0)
		{
			GfxMesh *mesh = &state->specificBrushSideMesh;
			DrawMeshWire(state, mesh, view_proj);
		}
	}
	simgui_render();
	sg_end_pass();
	sg_commit();
	
	// NOTE(GameChaos): finish input processing
	for (s32 i = 0; i < ARRAYCOUNT(input->buttons); i++)
	{
		input->buttons[i].actionCount = 0;
	}
	input->mouseDelta.x = 0;
	input->mouseDelta.y = 0;
	input->mouseScroll.x = 0;
	input->mouseScroll.y = 0;
}

internal void DebugGfxCleanup()
{
	simgui_shutdown();
}

internal void DebugGfxEvent(const sapp_event *event)
{
	simgui_handle_event(event);
	
#define MOUSE_BUTTON_HELPER(button, buttonCode) \
if (event->mouse_button == buttonCode)\
{\
button.isDown = down;\
button.actionCount++;\
}
	
#define KEYBOARD_BUTTON_HELPER(button, keyCode) \
if (event->key_code == keyCode)\
{\
button.isDown = down;\
button.actionCount++;\
}
	
	GfxInput *input = &g_gfxState->input;
	switch (event->type)
	{
		case SAPP_EVENTTYPE_MOUSE_MOVE:
		{
			input->mousePos.x = event->mouse_x;
			input->mousePos.y = event->mouse_y;
			input->mouseDelta.x += event->mouse_dx;
			input->mouseDelta.y += event->mouse_dy;
		} break;
		
		case SAPP_EVENTTYPE_MOUSE_SCROLL:
		{
			// TODO: is this 4.0 scroll step just a windows thing?
			input->mouseScroll.x = event->scroll_x / 4.0f;
			input->mouseScroll.y = event->scroll_y / 4.0f;
		} break;
		
		case SAPP_EVENTTYPE_MOUSE_DOWN:
		case SAPP_EVENTTYPE_MOUSE_UP:
		{
			b32 down = event->type == SAPP_EVENTTYPE_MOUSE_DOWN;
			MOUSE_BUTTON_HELPER(input->mouse1, SAPP_MOUSEBUTTON_LEFT);
			MOUSE_BUTTON_HELPER(input->mouse2, SAPP_MOUSEBUTTON_RIGHT);
		} break;
		
		case SAPP_EVENTTYPE_KEY_DOWN:
		case SAPP_EVENTTYPE_KEY_UP:
		{
			if (!event->key_repeat)
			{
				b32 down = event->type == SAPP_EVENTTYPE_KEY_DOWN;
				if (down && event->key_code == SAPP_KEYCODE_ESCAPE)
				{
					sapp_lock_mouse(!sapp_mouse_locked());
				}
				
				if (down && event->key_code == SAPP_KEYCODE_F11)
				{
					sapp_toggle_fullscreen();
				}
				
				if (down && event->key_code == SAPP_KEYCODE_F1)
				{
					g_gfxState->solidDrawType = (g_gfxState->solidDrawType + 1) % 3;
				}
				if (down && event->key_code == SAPP_KEYCODE_F2)
				{
					g_gfxState->wireframe = !g_gfxState->wireframe;
				}
				
				KEYBOARD_BUTTON_HELPER(input->moveForwards, SAPP_KEYCODE_W);
				KEYBOARD_BUTTON_HELPER(input->moveBackwards, SAPP_KEYCODE_S);
				KEYBOARD_BUTTON_HELPER(input->moveLeft, SAPP_KEYCODE_A);
				KEYBOARD_BUTTON_HELPER(input->moveRight, SAPP_KEYCODE_D);
				KEYBOARD_BUTTON_HELPER(input->leftShift, SAPP_KEYCODE_LEFT_SHIFT);
				KEYBOARD_BUTTON_HELPER(input->leftCtrl, SAPP_KEYCODE_LEFT_CONTROL);
			}
		} break;
		
		default:
		{
			
		} break;
	}
#undef MOUSE_BUTTON_HELPER
#undef KEYBOARD_BUTTON_HELPER
}

internal void DebugGfxMain(s32 argCount, char *arguments[])
{
	Arena arena = ArenaCreate(GIGABYTES(4));
	g_gfxState = (GfxState *)ArenaAlloc(&arena, sizeof(*g_gfxState));
	g_gfxState->arena = arena;
	g_gfxState->argCount = argCount;
	g_gfxState->arguments = arguments;
	
	sapp_desc desc = {};
	desc.user_data = g_gfxState;
	desc.init_userdata_cb = DebugGfxInit;
	desc.frame_userdata_cb = DebugGfxFrame;
	desc.cleanup_cb = DebugGfxCleanup;
	desc.event_cb = DebugGfxEvent;
	desc.logger.log_cb = DebugGfxLog;
	desc.width = 1366;
	desc.height = 768;
	desc.window_title = "Debug";
	desc.icon.sokol_default = true;
	sapp_run(&desc);
}