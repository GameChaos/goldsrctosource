
#include "shaders_compiled/world.h"
#include "shaders_compiled/wire.h"

#include "debug_render.h"

static_global GfxState *g_gfxState;

// NOTE(GameChaos): if rgb888 is true, then it converts to rgba8888, if rgb888 is false it expects rgba8888 data
static_function void DebugGfxAddTexture(u8 *data, i32 width, i32 height, bool rgb888/* = false*/)
{
	GfxState *state = g_gfxState;
	if (state->textureCount < DEBUG_GFX_MAX_TEXTURES)
	{
		i32 pixels = width * height;
		sg_image_desc desc = {};
		if (rgb888)
		{
			for (i32 i = 0; i < pixels; i++)
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
		//desc.min_filter = SG_FILTER_LINEAR_MIPMAP_LINEAR;
		//desc.mag_filter = SG_FILTER_LINEAR;
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

static_function void DebugGfxAddMiptexture(Arena *tempArena, GsrcMipTexture mipTexture, u8 *textureData)
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

static_function GfxVertData *VertsToGfxVertData(Arena *arena, Verts *poly, v3 normal, v4 s, v4 t)
{
	GfxVertData *result = (GfxVertData *)ArenaAlloc(arena, poly->vertCount * sizeof(*result));
	if (result)
	{
		for (i32 v = 0; v < poly->vertCount; v++)
		{
			GfxVertData vertData;
			vertData.pos = poly->verts[v];
			vertData.normal = normal;
			vertData.uv.x = v3dot(poly->verts[v], s.xyz) + s.w;
			vertData.uv.y = v3dot(poly->verts[v], t.xyz) + t.w;
			result[v] = vertData;
		}
	}
	
	return result;
}

static_function GfxMesh DebugGfxCreateMesh(GfxVertData *verts, i32 vertCount, i32 textureIndex, v3 wireColour/* = {0}*/)
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

static_function void DebugGfxTextureVecsFromNormal(v3 normal, v4 *sOut, v4 *tOut)
{
	v3 nonParallel = GetNonParallelVector(normal);
	v3 i = v3cross(normal, nonParallel);
	v3 j = v3cross(normal, i);
	sOut->xyz = v3muls(i, 0.25f);
	tOut->xyz = v3muls(j, 0.25f);
	sOut->w = 0;
	tOut->w = 0;
}
static_function void DebugGfxAddFace(Verts *poly, v3 normal, i32 textureIndex/* = -1*/, v4 s/* = {}*/, v4 t/* = {}*/)
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

static_function void DebugGfxAddBrushSide(Verts *poly, v3 normal, i32 textureIndex/* = -1*/, v4 s/* = {}*/, v4 t/* = {}*/)
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

static_function void DebugGfxAddBrush(i32 sideCount)
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

static_function void DebugGfxAddMesh(Verts *poly, v3 normal, v3 wireColour/* = {}*/, i32 textureIndex/* = -1*/, v4 s/* = {}*/, v4 t/* = {}*/)
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

static_function void DebugGfxInit(void *userData)
{
	GfxState *state = (GfxState *)userData;
	state->specificFaceIndex = -1;
	state->specificBrushIndex = -1;
	state->specificBrushSideIndex = -1;
	state->drawWorld = true;
	state->drawBrushes = true;
	
	sg_desc desc = {
		.environment = sglue_environment(),
		.buffer_pool_size = DEBUG_GFX_MAX_MESHES * 2,
		.image_pool_size = DEBUG_GFX_MAX_TEXTURES,
		.logger.func = slog_func,
	};
	sg_setup(&desc);
	state->sampler = sg_make_sampler(&(sg_sampler_desc){
										 .min_filter = SG_FILTER_LINEAR,
										 .mag_filter = SG_FILTER_LINEAR,
										 .wrap_u = SG_WRAP_REPEAT,
										 .wrap_v = SG_WRAP_REPEAT,
									 });
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
	DebugGfxAddTexture((u8 *)debugTexture, 8, 8, false);
	
	sg_pipeline_desc pipelineDesc = {
		.layout.attrs[ATTR_world_vs_iPos].format = SG_VERTEXFORMAT_FLOAT3,
		.layout.attrs[ATTR_world_vs_iNormal].format = SG_VERTEXFORMAT_FLOAT3,
		.layout.attrs[ATTR_world_vs_iUv].format = SG_VERTEXFORMAT_FLOAT2,
		.shader = sg_make_shader(world_shader_desc(sg_query_backend())),
		.index_type = SG_INDEXTYPE_UINT32,
		.cull_mode = SG_CULLMODE_BACK,
		.depth.compare = SG_COMPAREFUNC_LESS_EQUAL,
		.depth.write_enabled = true,
		.label = "pipeline",
	};
	
	sg_pipeline_desc wirePipelineDesc = {
		.layout.attrs[ATTR_world_vs_iPos].format = SG_VERTEXFORMAT_FLOAT3,
		.layout.attrs[ATTR_world_vs_iNormal].format = SG_VERTEXFORMAT_FLOAT3,
		.layout.attrs[ATTR_world_vs_iUv].format = SG_VERTEXFORMAT_FLOAT2,
		.shader = sg_make_shader(wire_shader_desc(sg_query_backend())),
		.primitive_type = SG_PRIMITIVETYPE_LINES,
		.index_type = SG_INDEXTYPE_UINT32,
		.cull_mode = SG_CULLMODE_BACK,
		.depth.compare = SG_COMPAREFUNC_LESS_EQUAL,
		.depth.write_enabled = true,
		.label = "pipeline",
	};
	
	state->passAction = (sg_pass_action){
		.colors[0].clear_value = {0, 0, 0, 1},
	};
	
	state->pipeline = sg_make_pipeline(&pipelineDesc);
	state->wirePipeline = sg_make_pipeline(&wirePipelineDesc);
	
	ArenaTemp arenaTemp = ArenaBeginTemp(&state->arena);
	i32 maxIndices = 512 * SRC_MAX_SIDE_VERTS; // SRC_MAX_MAP_BRUSHSIDES * SRC_MAX_SIDE_VERTS;
	u32 *indices = (u32 *)ArenaAlloc(&state->arena, maxIndices * sizeof(*indices));
	for (u32 i = 0; (i64)i < maxIndices - 2; i += 3)
	{
		indices[i + 0] = 0;
		indices[i + 1] = i / 3 + 1;
		indices[i + 2] = i / 3 + 2;
	}
	sg_buffer_desc indexDesc = {};
	indexDesc.type = SG_BUFFERTYPE_INDEXBUFFER;
	indexDesc.data.ptr = indices;
	indexDesc.data.size = sizeof(*indices) * maxIndices;
	
	u32 *wireIndices = (u32 *)ArenaAlloc(&state->arena, maxIndices * sizeof(*wireIndices));
	i32 wireIndexCount = 0;
	wireIndices[wireIndexCount++] = 0;
	wireIndices[wireIndexCount++] = 1;
	for (u32 i = 2; (i64)i < maxIndices - 1; i += 4)
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

static_function void DrawMesh(GfxState *state, GfxMesh *mesh, mat4 mvp, bool solid)
{
	sg_bindings bindings = {
		.vertex_buffers[0] = mesh->vertexBuffer,
		.fs = {
			.images[SLOT_tex] = solid ? state->textures[0].img : state->textures[mesh->texture].img,
			.samplers[SLOT_smp] = state->sampler,
		}
	};
	i32 indexCount = mesh->indexCount;
	if (indexCount > 0)
	{
		bindings.index_buffer = mesh->indexBuffer;
	}
	else
	{
		indexCount = (mesh->vertCount - 2) * 3;
		bindings.index_buffer = state->indexBuffer;
	}
	
	sg_apply_bindings(&bindings);
	
	world_vs_params_t vs_params = {};
	vs_params.mvp = mvp;
	vs_params.uCamOrigin = state->origin;
	
	sg_range vsParamsRange = SG_RANGE(vs_params);
	sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_world_vs_params, &vsParamsRange);
	
	sg_draw(0, indexCount, 1);
}

static_function void DrawMeshWire(GfxState *state, GfxMesh *mesh, mat4 mvp)
{
	sg_bindings bindings = {
		.vertex_buffers[0] = mesh->vertexBuffer
	};
	i32 indexCount = mesh->wireIndexCount;
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

static_function bool ImGuiCustomSlider(const char *label, i32 *value, i32 min, i32 max, i32 defaultValue)
{
	bool result = igSliderInt(label, value, min, max, "%d", 0);
	igSameLine(0, -1);
	ImVec2 buttonSize = {igGetFrameHeight(), igGetFrameHeight()};
	igPushItemFlag(ImGuiItemFlags_ButtonRepeat, true);
	igPushID_Str(label + 1);
	if (igButton("-", buttonSize))
	{
		(*value)--;
		result = true;
	}
	igPopID();
	igSameLine(0, -1);
	igPushID_Str(label + 2);
	if (igButton("+", buttonSize))
	{
		(*value)++;
		result = true;
	}
	igPopID();
	igPopItemFlag();
	igSameLine(0, -1);
	igPushID_Str(label + 3);
	if (igButton("Reset", (ImVec2){}))
	{
		(*value) = defaultValue;
		result = true;
	}
	igPopID();
	(*value) = GCM_CLAMP(min, (*value), max);
	return result;
}

static_function inline GfxRayHit RayPolygonIntersect(GfxPoly *polygon, v3 rayPos, v3 rayDir)
{
	GfxRayHit result = {};
	
	// check if ray and plane are parallel
	v3 normal = polygon->verts[0].normal;
	f32 rayDirDotNormal = v3dot(normal, rayDir); 
    if (GCM_ABS(rayDirDotNormal) < 0.0001f) // almost 0 
	{
		return result;
	}
	
	result.distance = (v3dot(normal, polygon->verts[0].pos) - v3dot(normal, rayPos)) / rayDirDotNormal;
	
	// check if the triangle is behind the ray
	if (result.distance <= 0.0001f)
	{
		return result;
	}
	
	result.endPosition = v3add(rayPos, v3muls(rayDir, result.distance)); 
	result.hit = true; // assume that we're inside
	
	// check if end point is inside the polygon
	for (i32 v = 0; v < polygon->vertCount; v++)
	{
		v3 vert0 = polygon->verts[v].pos;
		v3 vert1 = polygon->verts[(v + 1) % polygon->vertCount].pos;
		v3 edge = v3sub(vert1, vert0);
		v3 vp0 = v3sub(result.endPosition, vert0);
		v3 cross = v3cross(vp0, edge);
		f32 dot = v3dot(normal, cross);
		if (dot < 0.0f)
		{
			// ray doesn't hit the polygon
			result.hit = false;
			break;
		}
	}
	
	return result;
}

static_function void UpdateSpecificBrush(GfxState *state)
{
	//state->specificBrushIndex = GCM_CLAMP(-1, state->specificBrushIndex, state->brushCount - 1);
	if (state->specificBrushIndex >= 0)
	{
		// create specific brush mesh
		GfxBrush brush = state->brushes[state->specificBrushIndex];
		// NOTE(GameChaos): destroy buffers
		for (i32 i = 0; i < ARRAYCOUNT(state->specificBrushMeshes); i++)
		{
			GfxMesh *mesh = &state->specificBrushMeshes[i];
			sg_destroy_buffer(mesh->vertexBuffer);
			sg_destroy_buffer(mesh->indexBuffer);
			sg_destroy_buffer(mesh->wireIndexBuffer);
		}
		// NOTE(GameChaos): create new buffers
		for (i32 i = 0; i < brush.sideCount; i++)
		{
			GfxMesh *mesh = &state->specificBrushMeshes[i];
			*mesh = (GfxMesh){};
			GfxPoly *side = &state->brushSides[brush.firstSide + i];
			*mesh = DebugGfxCreateMesh(side->verts, side->vertCount, side->texture, (v3){0, 1, 1});
		}
	}
}

static_function void UpdateSpecificFace(GfxState *state)
{
	//state->specificFaceIndex = GCM_CLAMP(-1, state->specificFaceIndex, state->faceCount - 1);
	if (state->specificFaceIndex >= 0)
	{
		// create specific face mesh
		GfxMesh *mesh = &state->specificFaceMesh;
		sg_destroy_buffer(mesh->vertexBuffer);
		sg_destroy_buffer(mesh->indexBuffer);
		sg_destroy_buffer(mesh->wireIndexBuffer);
		*mesh = (GfxMesh){};
		GfxPoly *face = &state->faces[state->specificFaceIndex];
		*mesh = DebugGfxCreateMesh(face->verts, face->vertCount, face->texture, (v3){1, 1, 0});
	}
}

// from HandmadeMath.h
static_function mat4 Perspective(f32 fov, f32 aspectRatio, f32 nearDist, f32 farDist)
{
	// See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/gluPerspective.xml
	f32 cotangent = 1.0f / f32tan(fov * (GCM_PI / 360.0f));
	
	mat4 result = {
		.e[0][0] = cotangent / aspectRatio,
		.e[1][1] = cotangent,
		.e[2][3] = -1,
		.e[2][2] = (nearDist + farDist) / (nearDist - farDist),
		.e[3][2] = (2.0f * nearDist * farDist) / (nearDist - farDist),
		.e[3][3] = 0,
	};
	return result;
}

// from HandmadeMath.h
mat4 LookAt(v3 eye, v3 centre, v3 up)
{
    v3 f = v3normalise(v3sub(centre, eye));
    v3 s = v3normalise(v3cross(f, up));
    v3 u = v3cross(s, f);
	
    mat4 result = {
		.e[0][0] = s.x,
		.e[0][1] = u.x,
		.e[0][2] = -f.x,
		
		.e[1][0] = s.y,
		.e[1][1] = u.y,
		.e[1][2] = -f.y,
		
		.e[2][0] = s.z,
		.e[2][1] = u.z,
		.e[2][2] = -f.z,
		
		.e[3][0] = -v3dot(s, eye),
		.e[3][1] = -v3dot(u, eye),
		.e[3][2] = v3dot(f, eye),
		.e[3][3] = 1,
	};
    return result;
}

static_function void DebugGfxFrame(void *userData)
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
		i64 maxVerts = SRC_MAX_MAP_FACES * SRC_MAX_SIDE_VERTS;
		GfxVertData *tempVerts = (GfxVertData *)ArenaAlloc(&state->arena, maxVerts * sizeof(*tempVerts));
		u32 *indices = (u32 *)ArenaAlloc(&state->arena, maxVerts * sizeof(*indices));
		u32 *wireIndices = (u32 *)ArenaAlloc(&state->arena, maxVerts * sizeof(*wireIndices));
		for (i32 tex = 0; tex < state->textureCount; tex++)
		{
			i32 indexCount = 0;
			i32 wireIndexCount = 0;
			i32 vertCount = 0;
			for (i32 faceInd = 0; faceInd < state->faceCount; faceInd++)
			{
				GfxPoly *face = &state->faces[faceInd];
				if (face->texture == tex)
				{
					for (u32 i = (u32)vertCount;
						 (i64)i < vertCount + face->vertCount - 2;
						 i++)
					{
						indices[indexCount++] = (u32)vertCount;
						indices[indexCount++] = i + 1;
						indices[indexCount++] = i + 2;
					}
					
					for (u32 i = 0;
						 (i64)i < face->vertCount;
						 i++)
					{
						wireIndices[wireIndexCount++] = i + (u32)vertCount;
						wireIndices[wireIndexCount++] = (i + 1) % (u32)face->vertCount + (u32)vertCount;
					}
					
					Mem_Copy(face->verts, &tempVerts[vertCount], face->vertCount * sizeof(*face->verts));
					vertCount += face->vertCount;
					ASSERT(vertCount < maxVerts);
				}
			}
			
			if (vertCount > 0)
			{
				sg_buffer_desc vertexDesc = {
					.type = SG_BUFFERTYPE_VERTEXBUFFER,
					.data.ptr = tempVerts,
					.data.size = vertCount * sizeof(*tempVerts),
				};
				
				sg_buffer_desc indexDesc = {
					.type = SG_BUFFERTYPE_INDEXBUFFER,
					.data.ptr = indices,
					.data.size = sizeof(*indices) * indexCount,
				};
				
				sg_buffer_desc wireIndexDesc = {
					.type = SG_BUFFERTYPE_INDEXBUFFER,
					.data.ptr = wireIndices,
					.data.size = sizeof(*wireIndices) * wireIndexCount,
				};
				
				GfxMesh mesh = {
					.vertexBuffer = sg_make_buffer(&vertexDesc),
					.indexBuffer = sg_make_buffer(&indexDesc),
					.wireIndexBuffer = sg_make_buffer(&wireIndexDesc),
					.vertCount = vertCount,
					.indexCount = indexCount,
					.wireIndexCount = wireIndexCount,
					.texture = tex,
					.wireColour = {1, 1, 0},
				};
				state->worldMeshes[state->worldMeshCount++] = mesh;
			}
		}
		
		ArenaEndTemp(arenaTmp);
	}
	
	if (state->brushMeshCount <= 0 && state->brushSideCount > 0)
	{
		// make brush meshes!
		ArenaTemp arenaTmp = ArenaBeginTemp(&state->arena);
		i64 maxVerts = SRC_MAX_MAP_FACES * SRC_MAX_SIDE_VERTS * 10;
		GfxVertData *tempVerts = (GfxVertData *)ArenaAlloc(&state->arena, maxVerts * sizeof(*tempVerts));
		u32 *indices = (u32 *)ArenaAlloc(&state->arena, maxVerts * sizeof(*indices));
		u32 *wireIndices = (u32 *)ArenaAlloc(&state->arena, maxVerts * sizeof(*wireIndices));
		for (i32 tex = 0; tex < state->textureCount; tex++)
		{
			i32 indexCount = 0;
			i32 wireIndexCount = 0;
			i32 vertCount = 0;
			for (i32 sideInd = 0; sideInd < state->brushSideCount; sideInd++)
			{
				GfxPoly *side = &state->brushSides[sideInd];
				if (side->texture == tex)
				{
					for (u32 i = (u32)vertCount;
						 (i64)i < vertCount + side->vertCount - 2;
						 i++)
					{
						indices[indexCount++] = (u32)vertCount;
						indices[indexCount++] = i + 1;
						indices[indexCount++] = i + 2;
					}
					
					for (u32 i = 0;
						 (i64)i < side->vertCount;
						 i++)
					{
						wireIndices[wireIndexCount++] = i + (u32)vertCount;
						wireIndices[wireIndexCount++] = (i + 1) % (u32)side->vertCount + (u32)vertCount;
					}
					
					Mem_Copy(side->verts, &tempVerts[vertCount], side->vertCount * sizeof(*side->verts));
					vertCount += side->vertCount;
					ASSERT(vertCount < maxVerts);
				}
			}
			
			if (vertCount > 0)
			{
				sg_buffer_desc vertexDesc = {
					.type = SG_BUFFERTYPE_VERTEXBUFFER,
					.data.ptr = tempVerts,
					.data.size = vertCount * sizeof(*tempVerts),
				};
				
				sg_buffer_desc indexDesc = {
					.type = SG_BUFFERTYPE_INDEXBUFFER,
					.data.ptr = indices,
					.data.size = sizeof(*indices) * indexCount,
				};
				
				sg_buffer_desc wireIndexDesc = {
					.type = SG_BUFFERTYPE_INDEXBUFFER,
					.data.ptr = wireIndices,
					.data.size = sizeof(*wireIndices) * wireIndexCount,
				};
				
				GfxMesh mesh = {
					.vertexBuffer = sg_make_buffer(&vertexDesc),
					.indexBuffer = sg_make_buffer(&indexDesc),
					.wireIndexBuffer = sg_make_buffer(&wireIndexDesc),
					.vertCount = vertCount,
					.indexCount = indexCount,
					.wireIndexCount = wireIndexCount,
					.texture = tex,
					.wireColour = {0, 1, 1},
				};
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
		state->angles.x = GCM_CLAMP(-89, state->angles.x, 89);
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
			f32 len = v2len(move);
			if (len > 0)
			{
				move = v2divs(move, len);
			}
		}
		
		v3 moveDir = v3add(v3muls(forwards, move.x), v3muls(right, move.y));
		f32 speed = 500.0f;
		if (input->leftShift.isDown)
		{
			speed *= 0.25f;
		}
		if (input->leftCtrl.isDown)
		{
			speed *= 4.0f;
		}
		state->origin = v3add(state->origin, v3muls(moveDir, (f32)sapp_frame_duration() * speed));
	}
	
	mat4 proj = Perspective(DEBUG_GFX_FOV, sapp_widthf() / sapp_heightf(), DEBUG_GFX_NEARZ, DEBUG_GFX_FARZ);
	mat4 view = LookAt(state->origin, v3add(state->origin, forwards), (v3){0, 0, 1});
	mat4 viewProj = mat4mul(proj, view);
	igBeginDisabled(state->pickBrush || state->pickFace);
	//sg_begin_default_pass(&state->passAction, sapp_width(), sapp_height());
	sg_begin_pass(&(sg_pass){
		.action = state->passAction,
		.swapchain = sglue_swapchain(),
	});
	igText("Wireframe: %i\nSolid type: %i", state->wireframe, state->solidDrawType);
	igCheckbox("Draw world faces", &state->drawWorld);
	igCheckbox("Draw brushes", &state->drawBrushes);
	igText("Draw specific face");
	if (ImGuiCustomSlider("##Draw specific face", &state->specificFaceIndex, -1, state->faceCount - 1, -1))
	{
		UpdateSpecificFace(state);
	}
	
	igText("Draw specific brush");
	if (ImGuiCustomSlider("##Draw specific brush", &state->specificBrushIndex, -1, state->brushCount - 1, -1))
	{
		UpdateSpecificBrush(state);
	}
	
	igText("Draw specific brush side");
	if (ImGuiCustomSlider("##Draw specific brush side", &state->specificBrushSideIndex, -1, state->brushSideCount - 1, -1))
	{
		if (state->specificBrushSideIndex >= 0)
		{
			// create specific brush side mesh
			GfxMesh *mesh = &state->specificBrushSideMesh;
			sg_destroy_buffer(mesh->vertexBuffer);
			sg_destroy_buffer(mesh->indexBuffer);
			sg_destroy_buffer(mesh->wireIndexBuffer);
			*mesh = (GfxMesh){};
			GfxPoly *side = &state->brushSides[state->specificBrushSideIndex];
			*mesh = DebugGfxCreateMesh(side->verts, side->vertCount, side->texture, (v3){0, 1, 0});
		}
	}
	
	if (igButton("Pick face", (ImVec2){}))
	{
		state->specificFaceIndex = -1;
		state->pickFace = true;
	}
	
	if (igButton("Pick brush", (ImVec2){}))
	{
		state->specificBrushIndex = -1;
		state->pickBrush = true;
	}
	
	if (state->specificFaceIndex >= 0)
	{
		GfxPoly *face = &state->faces[state->specificFaceIndex];
		igText("Face %i\ns: %f %f %f %f\nt: %f %f %f %f",
					state->specificFaceIndex,
					face->s.x, face->s.y, face->s.z, face->s.w,
					face->t.x, face->t.y, face->t.z, face->s.w);
	}
	
	if (state->specificBrushIndex >= 0)
	{
		GfxBrush brush = state->brushes[state->specificBrushIndex];
		igText("Brush %i info:\nFirst side: %i\nSide count: %i",
					state->specificBrushIndex, brush.firstSide, brush.sideCount);
	}
	
	if (state->specificBrushSideIndex >= 0)
	{
		GfxPoly *side = &state->brushSides[state->specificBrushSideIndex];
		igText("Brush side %i info:\nTexture: %i\nVert count: %i\nNormal: %f %f %f",
					state->specificBrushSideIndex, side->texture, side->vertCount,
					side->verts->normal.x, side->verts->normal.y, side->verts->normal.z);
	}
	
	igEndDisabled();
	
	if (state->pickBrush || state->pickFace)
	{
		if (input->mouse1.isDown)
		{
			mat4 viewCentred = LookAt((v3){}, forwards, (v3){0, 0, 1});
			mat4 inv = mat4invert(mat4mul(proj, viewCentred));
			v3 rayDir = {
				(input->mousePos.x / sapp_widthf()) * 2 - 1,
				1 - (input->mousePos.y / sapp_heightf()) * 2,
				0
			};
			rayDir = mat4mulv4(inv, (v4){rayDir.x, rayDir.y, rayDir.z, 1}).xyz;
			
			if (state->pickBrush)
			{
				GfxRayHit closestHit = {};
				i32 hitBrushInd = -1;
				for (i32 brushInd = 0; brushInd < state->brushCount; brushInd++)
				{
					GfxBrush brush = state->brushes[brushInd];
					for (i32 sideInd = brush.firstSide;
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
				i32 hitFaceInd = -1;
				for (i32 faceInd = 0; faceInd < state->faceCount; faceInd++)
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
	for (i32 i = 0; i < state->meshCount; i++)
	{
		GfxMesh *mesh = &state->meshes[i];
		if (state->solidDrawType <= 1)
		{
			DrawMesh(state, mesh, viewProj, state->solidDrawType);
		}
	}
	
	if (state->specificFaceIndex < 0)
	{
		if (state->drawWorld)
		{
			for (i32 i = 0; i < state->worldMeshCount; i++)
			{
				GfxMesh *mesh = &state->worldMeshes[i];
				if (state->solidDrawType <= 1)
				{
					DrawMesh(state, mesh, viewProj, state->solidDrawType);
				}
			}
		}
	}
	else
	{
		GfxMesh *mesh = &state->specificFaceMesh;
		if (state->solidDrawType <= 1)
		{
			DrawMesh(state, mesh, viewProj, state->solidDrawType);
		}
	}
	
	if (state->specificBrushIndex < 0)
	{
		if (state->drawBrushes)
		{
			for (i32 i = 0; i < state->brushMeshCount; i++)
			{
				GfxMesh *mesh = &state->brushMeshes[i];
				if (state->solidDrawType <= 1)
				{
					DrawMesh(state, mesh, viewProj, state->solidDrawType);
				}
			}
		}
	}
	else
	{
		GfxBrush brush = state->brushes[state->specificBrushIndex];
		for (i32 i = 0; i < brush.sideCount; i++)
		{
			GfxMesh *mesh = &state->specificBrushMeshes[i];
			if (state->solidDrawType <= 1)
			{
				DrawMesh(state, mesh, viewProj, state->solidDrawType);
			}
		}
	}
	
	if (state->specificBrushSideIndex >= 0)
	{
		GfxMesh *mesh = &state->specificBrushSideMesh;
		if (state->solidDrawType <= 1)
		{
			DrawMesh(state, mesh, viewProj, state->solidDrawType);
		}
	}
	
	if (state->wireframe)
	{
		sg_apply_pipeline(state->wirePipeline);
		for (i32 i = 0; i < state->meshCount; i++)
		{
			GfxMesh *mesh = &state->meshes[i];
			DrawMeshWire(state, mesh, viewProj);
		}
		
		if (state->specificFaceIndex < 0)
		{
			if (state->drawWorld)
			{
				for (i32 i = 0; i < state->worldMeshCount; i++)
				{
					GfxMesh *mesh = &state->worldMeshes[i];
					DrawMeshWire(state, mesh, viewProj);
				}
			}
		}
		else
		{
			GfxMesh *mesh = &state->specificFaceMesh;
			DrawMeshWire(state, mesh, viewProj);
		}
		
		if (state->specificBrushIndex < 0)
		{
			if (state->drawBrushes)
			{
				for (i32 i = 0; i < state->brushMeshCount; i++)
				{
					GfxMesh *mesh = &state->brushMeshes[i];
					DrawMeshWire(state, mesh, viewProj);
				}
			}
		}
		else
		{
			GfxBrush brush = state->brushes[state->specificBrushIndex];
			for (i32 i = 0; i < brush.sideCount; i++)
			{
				GfxMesh *mesh = &state->specificBrushMeshes[i];
				DrawMeshWire(state, mesh, viewProj);
			}
		}
		
		if (state->specificBrushSideIndex >= 0)
		{
			GfxMesh *mesh = &state->specificBrushSideMesh;
			DrawMeshWire(state, mesh, viewProj);
		}
	}
	simgui_render();
	sg_end_pass();
	sg_commit();
	
	// NOTE(GameChaos): finish input processing
	for (i32 i = 0; i < ARRAYCOUNT(input->buttons); i++)
	{
		input->buttons[i].actionCount = 0;
	}
	input->mouseDelta.x = 0;
	input->mouseDelta.y = 0;
	input->mouseScroll.x = 0;
	input->mouseScroll.y = 0;
}

static_function void DebugGfxCleanup()
{
	simgui_shutdown();
}

static_function void DebugGfxEvent(const sapp_event *event)
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
			bool down = event->type == SAPP_EVENTTYPE_MOUSE_DOWN;
			MOUSE_BUTTON_HELPER(input->mouse1, SAPP_MOUSEBUTTON_LEFT);
			MOUSE_BUTTON_HELPER(input->mouse2, SAPP_MOUSEBUTTON_RIGHT);
		} break;
		
		case SAPP_EVENTTYPE_KEY_DOWN:
		case SAPP_EVENTTYPE_KEY_UP:
		{
			if (!event->key_repeat)
			{
				bool down = event->type == SAPP_EVENTTYPE_KEY_DOWN;
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

static_function void DebugGfxMain(i32 argCount, char *arguments[])
{
	Arena arena = ArenaCreate(GIGABYTES(4));
	g_gfxState = (GfxState *)ArenaAlloc(&arena, sizeof(*g_gfxState));
	g_gfxState->arena = arena;
	g_gfxState->argCount = argCount;
	g_gfxState->arguments = arguments;
	
	sapp_desc desc = {
		.user_data = g_gfxState,
		.init_userdata_cb = DebugGfxInit,
		.frame_userdata_cb = DebugGfxFrame,
		.cleanup_cb = DebugGfxCleanup,
		.event_cb = DebugGfxEvent,
		.logger.func = slog_func,
		.width = 1366,
		.height = 768,
		.window_title = "Debug",
		.icon.sokol_default = true,
	};
	sapp_run(&desc);
}