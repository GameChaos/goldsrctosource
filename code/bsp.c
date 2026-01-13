
typedef struct
{
	SrcTexinfo texinfo[SRC_MAX_MAP_TEXINFO];
	i32 texinfoCount;
	
	SrcTexdata texdata[SRC_MAX_MAP_TEXDATA];
	i32 texdataCount;
	
	char tdStringData[SRC_MAX_MAP_TEXDATA_STRING_DATA];
	i32 tdStringTable[SRC_MAX_MAP_TEXDATA_STRING_TABLE];
	i32 tdTableCount;
	
	SrcLeaf leaves[SRC_MAX_MAP_LEAFS];
	i32 leafCount;
	
	SrcNode nodes[SRC_MAX_MAP_NODES];
	i32 nodeCount;
	
	v3 vertices[SRC_MAX_MAP_VERTS];
	i32 vertexCount;
	
	v3 vertNormals[SRC_MAX_MAP_VERTNORMALS];
	i32 vertNormalCount;
	
	u32 vertNormalIndices[SRC_MAX_MAP_VERTNORMALINDICES];
	i32 vertNormalIndexCount;
	
	SrcEdge edges[SRC_MAX_MAP_EDGES];
	i32 edgeCount;
	
	SrcFace faces[SRC_MAX_MAP_FACES];
	i32 faceCount;
	
	SrcModel models[SRC_MAX_MAP_MODELS];
	bool modelIsLadder[SRC_MAX_MAP_MODELS];
	i32 modelCount;
	
	u16 leafBrushes[SRC_MAX_MAP_LEAFBRUSHES];
	u16 leafBrushCount;
	
	u16 leafFaces[SRC_MAX_MAP_LEAFFACES];
	u16 leafFaceCount;
	
	SrcLeafWaterData leafWaterData[SRC_MAX_MAP_LEAFWATERDATA];
	i32 leafWaterDataCount;
	
	i16 leafMinDistToWater[SRC_MAX_MAP_LEAFS];
	i32 leafMinDistToWaterCount;
	
	SrcBrush brushes[SRC_MAX_MAP_BRUSHES];
	i32 brushCount;
	
	SrcBrushSide brushSides[SRC_MAX_MAP_BRUSHSIDES];
	i32 brushSideCount;
	
	SrcPlane planes[SRC_MAX_MAP_PLANES];
	i32 planeCount;
} BspState;

static_global BspState g_bspConversionState = {};

static_function inline void *BufferPushDataAndSetLumpSize(FileWritingBuffer *buffer, SrcHeader *header, i32 lumpIndex, void *data, i32 bytes)
{
	void *result = buffer->memory + buffer->usedBytes;
	header->lump[lumpIndex].offset = (i32)buffer->usedBytes;
	header->lump[lumpIndex].length = bytes;
	if (!BufferPushData(buffer, data, bytes, true))
	{
		ASSERT(0);
		result = NULL;
	}
	
	return result;
}

// NOTE(GameChaos): https://github.com/id-Software/Quake-III-Arena/blob/dbe4ddb10315479fc00086f08e25d968b4b43c49/q3map/map.c#L125
static_function bool AddBrushBevels(BspState *state, SrcBrush *brush, Verts *polys, i32 polyCount, v3 mins, v3 maxs)
{
	bool result = false;
	SrcBrushSide *brushSides = &state->brushSides[brush->firstSide];
	//
	// add the axial planes
	//
	i32 order = 0;
	bool added = false;
	for (i32 axis = 0; axis < 3; axis++)
	{
		for (i32 dir = -1; dir <= 1; dir += 2, order++)
		{
			// see if the plane is already present
			i32 sideInd;
			for (sideInd = 0; sideInd < brush->sides; sideInd++)
			{
				if (state->planes[brushSides[sideInd].plane].normal.e[axis] == dir)
				{
					break;
				}
			}
			
			if (sideInd == brush->sides)
			{
				ASSERT(state->brushSideCount == brush->sides + brush->firstSide);
				// add a new side
				if (brush->sides == SRC_MAX_BRUSH_SIDES)
				{
					FatalError("Exceeded maximum amount of brush sides!");
				}
				SrcBrushSide side = {};
				v3 normal = {};
				normal.e[axis] = (f32)dir;
				f32 dist;
				if (dir == 1)
				{
					dist = maxs.e[axis];
				}
				else
				{
					dist = -mins.e[axis];
				}
				
				SrcPlane plane = {normal, dist, axis};
				side.plane = (u16)AddPlane(state->planes, &state->planeCount, plane);
				side.bevel = true;
				AddBrushSide(state->brushSides, &state->brushSideCount, side);
				brush->sides++;
				// NOTE(GameChaos): "add" blank polygon
				polys[polyCount++] = (Verts){};
				added = true;
			}
			
			// if the plane is not in it canonical order, swap it
			if (sideInd != order)
			{
				SrcBrushSide sidetemp = brushSides[order];
				brushSides[order] = brushSides[sideInd];
				brushSides[sideInd] = sidetemp;
				
				// NOTE(GameChaos): static_persist because the struct is way too
				//  big to allocate on the stack
				static_persist Verts polytemp;
				polytemp = polys[order];
				polys[order] = polys[sideInd];
				polys[sideInd] = polytemp;
			}
		}
	}
	
	//
	// add the edge bevels
	//
	if (brush->sides == 6)
	{
		result = true;
		return result; // pure axial
	}
#if 1
	else
	{
#ifdef GC_DEBUG
		// NOTE(GameChaos): test if first 6 sides are axis aligned
		for (i32 dir = -1; dir <= 1; dir += 2)
		{
			for (i32 axis = 3; axis < 3; axis++)
			{
				ASSERT(state->planes[brushSides[axis * (dir + 1)].plane].normal.e[axis] == dir);
			}
		}
#endif
		ASSERT(brush->sides == polyCount);
		// test the non-axial plane edges
		// this code tends to cause some problems...
		for (i32 i = 6; i < brush->sides; i++)
		{
			SrcBrushSide *s = &brushSides[i];
			Verts *poly = &polys[i];
			if (poly->vertCount <= 0)
			{
				continue;
			}
			for (i32 j = 0; j < poly->vertCount; j++)
			{
				i32 k = (j + 1) % poly->vertCount;
				v3 vec = v3sub(poly->verts[j], poly->verts[k]);
				f32 vecLen = v3len(vec);
				if (vecLen < 0.5f)
				{
					continue;
				}
				vec = v3muls(vec, 1.0f / vecLen);
				vec = SnapVector(vec);
				for (k = 0; k < 3; k++)
				{
					if (vec.e[k] == -1 || vec.e[k] == 1)
					{
						break; // axial
					}
				}
				if (k != 3)
				{
					continue; // only test non-axial edges
				}
				
				// try the six possible slanted axials from this edge
				for (i32 axis = 0; axis < 3; axis++)
				{
					for (i32 dir = -1; dir <= 1; dir += 2)
					{
						// construct a plane
						v3 vec2 = {};
						vec2.e[axis] = (f32)dir;
						v3 normal = v3cross(vec, vec2);
						f32 normalLength = v3len(normal);
						if (normalLength < 0.5f)
						{
							continue;
						}
						normal = v3muls(normal, 1.0f / normalLength);
						f32 dist = v3dot(poly->verts[j], normal);
						
						// if all the points on all the sides are
						// behind this plane, it is a proper edge bevel
						for (k = 0; k < brush->sides && k < polyCount; k++)
						{
							// if this plane has allready been used, skip it
							if (PlaneEquals(state->planes[brushSides[k].plane], normal, dist))
							{
								break;
							}
							
							Verts *poly2 = &polys[k];
							if (poly2->vertCount <= 0)
							{
								continue;
							}
							i32 l;
							for (l = 0; l < poly2->vertCount; l++)
							{
								f32 d = v3dot(poly2->verts[l], normal) - dist;
								if (d > 0.1f)
								{
									break; // point in front
								}
							}
							if (l != poly2->vertCount)
							{
								break;
							}
						}
						
						if (k != brush->sides)
						{
							continue; // wasn't part of the outer hull
						}
						// add this plane
						if (brush->sides == SRC_MAX_BRUSH_SIDES)
						{
							FatalError("Exceeded maximum amount of brush sides!");
						}
						
						SrcBrushSide s2 = {};
						SrcPlane plane = {normal, dist, axis};
						s2.plane = (u16)AddPlane(state->planes, &state->planeCount, plane);
						s2.bevel = true;
						AddBrushSide(state->brushSides, &state->brushSideCount, s2);
						brush->sides++;
						// NOTE(GameChaos): "add" blank polygon
						polys[polyCount++] = (Verts){};
					}
				}
			}
		}
	}
#endif
	
	return result;
}

static_function bool BspFromGoldsource(Arena *arena, Arena *tempArena, GsrcMapData *mapData, SrcMapData *srcMapData, char *outputPath, char *modPath, char *valvePath)
{
	bool result = false;
	ASSERT(outputPath);
	ASSERT(mapData);
	ASSERT(srcMapData);
	if (!outputPath)
	{
		Error("Invalid output path provided to converter!\n");
		return result;
	}
	
	ArenaTemp arenaTemp = ArenaBeginTemp(arena);
	BspState *state = &g_bspConversionState;
	Plat_MemSetToZero(state, sizeof(*state));
	
	FileWritingBuffer buffer = BufferCreate(arena, GIGABYTES(1));
	ASSERT(buffer.memory);
	ASSERT(buffer.size > 0);
	if (!buffer.memory || buffer.size <= 0)
	{
		Error("Couldn't allocate memory for file buffer!\n");
		return result;
	}
	
	if (!arena || !tempArena || !mapData || !srcMapData)
	{
		return result;
	}
	
	srcMapData->fileDataSize = buffer.size;
	srcMapData->fileData = buffer.memory;
	
	srcMapData->header = (SrcHeader *)BufferPushSize(&buffer, sizeof(*srcMapData->header), true);
	SrcHeader *fileHeader = srcMapData->header;
	
	*fileHeader = (SrcHeader){
		.ident = SRC_BSPHEADER,
		.version = 21, // CSGO bsp
	};
	
	i32 mapflags = 0;
	BufferPushDataAndSetLumpSize(&buffer, fileHeader, SRC_LUMP_MAP_FLAGS, &mapflags, sizeof(mapflags));
	
	SrcLump *lump = (SrcLump *)&fileHeader->lump;
	
	// load WADs
	// NOTE(GameChaos): static_persist because the struct is way too
	//  big to allocate on the stack
	static_persist Wad3 wads[MAX_WADS] = {};
	i32 wadCount = LoadWads(arena, modPath, valvePath, wads);
	EntList gsrcEnts = GsrcParseEntities(arena, mapData->lumpEntities);
	
	//
	// LUMP_PAKFILE
	i64 maxZipBytes = mapData->lumpTextures.mipTextureCount * 2048 * 2048 * 3;
	// 1 vmt and 1 vtf for every miptexture + 6 sky vmts and 6 sky vtfs
	i64 maxZipFiles = mapData->lumpTextures.mipTextureCount * 2 + 6 + 6;
	ZipBuilder zipBuilder = ZipBuilderCreate(arena, maxZipBytes, maxZipFiles);
	
	FileWritingBuffer texBuffer = BufferCreate(tempArena, 8192 * 8192 * 3);
	
	// convert skybox textures
	EntProperties *worldspawn = EntListGetEnt(gsrcEnts, STR("worldspawn"));
	EntProperty *skyname = EntGetProperty(worldspawn, STR("skyname"));
	// TODO: is this the default sky?
	EntProperty defaultSky = {STR("skyname"), STR("desert")};
	if (!skyname)
	{
		skyname = &defaultSky;
	}
	//ASSERT(skyname);
	if (skyname)
	{
		for (i32 side = 0; side < SKY_SIDE_COUNT; side++)
		{
			BufferReset(&texBuffer);
			
			bool gotTexture = GsrcGetSkyTexture(tempArena, &texBuffer, skyname->value, modPath, valvePath, (SkySide)side);
			if (gotTexture)
			{
				char vtfFileName[256];
				i32 vtfFileNameLen = Format(vtfFileName, sizeof(vtfFileName), "materials/skybox/%.*s%s.vtf",
											(i32)skyname->value.length, skyname->value.data, g_skySides[side]);
				ZipBuilderAddFile(&zipBuilder, vtfFileName, vtfFileNameLen, texBuffer.memory, texBuffer.usedBytes);
				
				char texturePath[128];
				Format(texturePath, sizeof(texturePath), "skybox/%.*s%s",
					   (i32)skyname->value.length, skyname->value.data, g_skySides[side]);
				
				char vmt[512];
				i32 vmtFileLength = MakeSkyTextureVmt(vmt, sizeof(vmt), texturePath);
				
				char vmtFileName[128];
				i32 vmtFileNameLen = Format(vmtFileName, sizeof(vmtFileName), "%s.vmt", texturePath);
				ZipBuilderAddFile(&zipBuilder, vmtFileName, vmtFileNameLen, vmt, vmtFileLength);
			}
			else
			{
				// TODO: make blank vmt when loading fails?
				//ASSERT(0);
			}
		}
	}
	
	// convert GSRC_LUMP_TEXTURES into "files" in LUMP_PAKFILE
	for (u32 i = 0; i < mapData->lumpTextures.mipTextureCount; i++)
	{
		if (mapData->lumpTextures.mipTextureOffsets[i] <= 0)
		{
			continue;
		}
		Wad3TextureHeader mipTexture = *mapData->lumpTextures.mipTextures[i];
		u8 *textureData = (u8 *)mapData->lumpTextures.mipTextures[i];
		// NOTE(GameChaos): wad3 texture header and bsp miptexture structs are
		// the exact same and they point to the exact same data as well!
		static_assert(sizeof(Wad3TextureHeader) == sizeof(Wad3TextureHeader), "");
		if (mipTexture.offsets[0] <= 0)
		{
			FindTextureResult find = FindTextureInWads(wads, wadCount, mipTexture.name);
			if (find.found)
			{
				mipTexture = wads[find.wadIndex].textures[find.textureIndex].header;
				textureData = (u8 *)wads[find.wadIndex].textures[find.textureIndex].textureData;
			}
			else
			{
				Warning("Couldn't find texture %s from wad files :(\n", mipTexture.name);
#ifdef DEBUG_GRAPHICS
				// use an error texture as a placeholder
				u8 errorTexture[] = {
					255,0,255, 255,0,255, 0,0,0, 0,0,0,
					255,0,255, 255,0,255, 0,0,0, 0,0,0,
					0,0,0, 0,0,0, 255,0,255, 255,0,255,
					0,0,0, 0,0,0, 255,0,255, 255,0,255,
				};
				DebugGfxAddTexture(errorTexture, 4, 4, true);
#endif
				continue;
			}
		}
		
		bool transparent = mipTexture.name[0] == '{';
		
		char vtfFileName[256];
		i32 vtfFileNameLen = Format(vtfFileName, sizeof(vtfFileName), CONVERTED_MATERIAL_PATH "%s.vtf", mipTexture.name);
		BufferReset(&texBuffer);
		GsrcMipTextureToVtf(tempArena, &texBuffer, mipTexture, textureData);
#ifdef DEBUG_GRAPHICS
		// TODO: both vmf.cpp and this can double texture count cos they both convert textures separately.
		// do something about this!
		DebugGfxAddMiptexture(tempArena, mipTexture, textureData);
#endif
		ZipBuilderAddFile(&zipBuilder, vtfFileName, vtfFileNameLen, texBuffer.memory, texBuffer.usedBytes);
		
		char vmtFileName[256];
		i32 vmtFileNameLen = Format(vmtFileName, sizeof(vmtFileName), CONVERTED_MATERIAL_PATH "%s.vmt", mipTexture.name);
		
		char vmt[512];
		i32 vmtFileLength = MakeVmt(vmt, sizeof(vmt), mipTexture.name, transparent);
		
		ZipBuilderAddFile(&zipBuilder, vmtFileName, vmtFileNameLen, vmt, vmtFileLength);
	}
	// write zip directory
	FileWritingBuffer zipFile = ZipBuilderFinish(&zipBuilder);
	if (zipFile.memory && zipFile.size)
	{
		BufferPushDataAndSetLumpSize(&buffer, fileHeader, SRC_LUMP_PAKFILE,
									 zipFile.memory, (i32)zipFile.usedBytes);
	}
	
	//
	// LUMP_TEXDATA                        = 2 # ERROR: Map with no textures
	//
	
	for (i32 i = 0; i < (i64)mapData->lumpTextures.mipTextureCount; i++)
	{
		// TODO: nameStringTableID
		state->texdata[state->texdataCount++] = (SrcTexdata){
			{0.5f, 0.5f, 0.5f},
			i,
			(i32)mapData->lumpTextures.mipTextures[i]->width,
			(i32)mapData->lumpTextures.mipTextures[i]->height,
			(i32)mapData->lumpTextures.mipTextures[i]->width,
			(i32)mapData->lumpTextures.mipTextures[i]->height,
		};
	}
	BufferPushDataAndSetLumpSize(&buffer, fileHeader, SRC_LUMP_TEXDATA, state->texdata, sizeof(*state->texdata) * state->texdataCount);
	
	//
	// LUMP_TEXDATA_STRING_DATA            = 43 # ERROR: Unable to load corrupted map
	//
	
	char *tdStringData = state->tdStringData;
	char *tdStringDataEnd = state->tdStringData + SRC_MAX_MAP_TEXDATA_STRING_DATA;
	
	for (u32 i = 0; i < mapData->lumpTextures.mipTextureCount; i++)
	{
		if (mapData->lumpTextures.mipTextures[i] == NULL)
		{
			continue;
		}
		i32 offset = (i32)(tdStringData - state->tdStringData);
		state->tdStringTable[state->tdTableCount++] = offset;
		i32 len = (i32)(tdStringDataEnd - tdStringData);
		i32 formattedLen = 0;
		if (StringEquals(mapData->lumpTextures.mipTextures[i]->name, "sky", false))
		{
			formattedLen = Format(tdStringData, len, "%s", "tools/toolsskybox2d");
		}
		else
		{
			formattedLen = Format(tdStringData, len, CONVERTED_MATERIAL_FOLDER "%s", mapData->lumpTextures.mipTextures[i]->name);
		}
		Print("Texture: %s\n", mapData->lumpTextures.mipTextures[i]->name);
		tdStringData += formattedLen + 1;
	}
	BufferPushDataAndSetLumpSize(&buffer, fileHeader, SRC_LUMP_TEXDATA_STRING_DATA,
								 state->tdStringData, (i32)(tdStringData - state->tdStringData));
	
	//
	// LUMP_TEXDATA_STRING_TABLE           = 44 # straight up crash
	//
	// copy the whole array since it's contiguous, hooray!
	
	BufferPushDataAndSetLumpSize(&buffer, fileHeader, SRC_LUMP_TEXDATA_STRING_TABLE,
								 state->tdStringTable, sizeof(*state->tdStringTable) * state->tdTableCount);
	//
	// LUMP_TEXINFO                        = 6 # ERROR: Map with no texinfo
	//
	
	for (i32 i = 0; i < mapData->texinfoCount; i++)
	{
		SrcTexinfo texinfo = {};
		
		for (i32 xyzw = 0; xyzw < 4; xyzw++)
		{
			for (i32 st = 0; st < 2; st++)
			{
				texinfo.textureVecs[st][xyzw] = mapData->lumpTexinfo[i].vecs[st][xyzw];
				texinfo.lightmapVecs[st][xyzw] = mapData->lumpTexinfo[i].vecs[st][xyzw] / 16.0f;
			}
		}
		char *texName = mapData->lumpTextures.mipTextures[mapData->lumpTexinfo[i].miptex]->name;
		if (StringEquals(texName, "sky", false))
		{
			texinfo.flags |= SRC_SURF_NOLIGHT | SRC_SURF_SKY | SRC_SURF_SKY2D;
		}
		else if (texName[0] == '!')
		{
			texinfo.flags |= SRC_SURF_WARP;
		}
		else if (texName[0] == '{')
		{
			texinfo.flags |= SRC_SURF_TRANS;
		}
		texinfo.texdata = mapData->lumpTexinfo[i].miptex;
		state->texinfo[state->texinfoCount++] = texinfo;
	}
	
	//
	// LUMP_FACES                          = 7
	// LUMP_MODELS                         = 14 # ERROR: Map with no models
	// LUMP_VERTICES                       = 3 # Map loads, but renders incorrectly]
	// LUMP_VERTNORMALS
	// LUMP_VERTNORMALINDICES
	//
	// NOTE(GameChaos): csgo uses face lump v1 :)
	fileHeader->lump[SRC_LUMP_FACES].version = 1;
	fileHeader->lump[SRC_LUMP_VERTNORMALINDICES].version = 1; // v0 = u16 indices, v1 = u32 indices
	state->vertexCount = mapData->vertexCount;
	Mem_Copy(mapData->lumpVertices, state->vertices, sizeof(*mapData->lumpVertices) * mapData->vertexCount);
	
	state->edgeCount = mapData->edgeCount;
	Mem_Copy(mapData->lumpEdges, state->edges, sizeof(*mapData->lumpEdges) * mapData->edgeCount);
	
	
	// NOTE: source and goldsrc plane structs are the same, that's why we can do this.
	Mem_Copy(mapData->lumpPlanes, state->planes, sizeof(*mapData->lumpPlanes) * mapData->planeCount);
	state->planeCount = mapData->planeCount;
	
	for (i32 i = 0; i < mapData->faceCount; i++)
	{
		SrcFace face = {};
		
		face.planeIndex = mapData->lumpFaces[i].plane;
		face.side = (u8)mapData->lumpFaces[i].planeSide;
		face.onNode = 1; // TODO: real value of this
		face.firstEdge = mapData->lumpFaces[i].firstEdge;
		face.edges = mapData->lumpFaces[i].edges;
		face.texinfo = mapData->lumpFaces[i].texInfoIndex;
		face.dispinfo = -1;
		face.surfaceFogVolumeID = -1;
		for (int j = 0; j < 4; j++)
		{
			face.styles[j] = mapData->lumpFaces[i].styles[j];
		}
		face.lightOffset = mapData->lumpFaces[i].lightmapOffset / 3 * 4;
		face.area = 420.69f; // TODO: calculate area
		face.origFace = i;
		
		// NOTE(GameChaos): convert lighting offsets and stuff
		GsrcTexinfo texinfo = mapData->lumpTexinfo[face.texinfo];
		if (face.lightOffset >= 0 && !(texinfo.flags & 1))
		{
			v2 mins = v2fill(F32_MAX);
			v2 maxs = v2fill(-F32_MAX);
			
			for (i32 surfEdge = 0; surfEdge < face.edges; surfEdge++)
			{
				i32 edge = mapData->lumpSurfEdges[face.firstEdge + surfEdge];
				v3 vert;
				if (edge >= 0)
				{
					vert = mapData->lumpVertices[mapData->lumpEdges[edge].vertex[0]];
				}
				else
				{
					vert = mapData->lumpVertices[mapData->lumpEdges[-edge].vertex[1]];
				}
				
				for (i32 j = 0; j < 2; j++)
				{
					f32 val = (vert.e[0] * texinfo.vecs[j][0]
							   + vert.e[1] * texinfo.vecs[j][1]
							   + vert.e[2] * texinfo.vecs[j][2]
							   + texinfo.vecs[j][3]);
					if (val < mins.e[j])
					{
						mins.e[j] = val;
					}
					if (val > maxs.e[j])
					{
						maxs.e[j] = val;
					}
				}
			}
			i32 bmins[2];
			i32 bmaxs[2];
			i32 size[2];
			for (i32 j = 0; j < 2; j++)
			{
				bmins[j] = (i32)f32floor(mins.e[j] / 16.0f);
				bmaxs[j] = (i32)f32ceil(maxs.e[j] / 16.0f);
				size[j] = bmaxs[j] - bmins[j];
				
				if (bmins[j] != I32_MIN
					&& size[j] < SRC_MAX_BRUSH_LIGHTMAP_DIM_INCLUDING_BORDER && GCM_ABS(bmins[j]) < 16834)
				{
					face.lightmapTextureMinsInLuxels[j] = bmins[j];
					face.lightmapTextureSizeInLuxels[j] = size[j];
				}
				else
				{
					Warning("Too large lightmap size on face %i. Size is %i, max is %i\n",
							i, size[j], SRC_MAX_BRUSH_LIGHTMAP_DIM_INCLUDING_BORDER);
				}
			}
		}
		
		// vertex normals
		if (state->vertNormalCount < SRC_MAX_MAP_VERTNORMALS)
		{
			state->vertNormals[state->vertNormalCount++] = mapData->lumpPlanes[mapData->lumpFaces[i].plane].normal;
		}
		else
		{
			Warning("Ran out of vertex normals!\n");
			ASSERT(0);
		}
		for (i32 surfEdge = 0; surfEdge < face.edges; surfEdge++)
		{
			if (state->vertNormalIndexCount < SRC_MAX_MAP_VERTNORMALS)
			{
				state->vertNormalIndices[state->vertNormalIndexCount++] = state->vertNormalCount - 1;
			}
			else
			{
				Warning("Ran out of vertex normal indices!\n");
				ASSERT(0);
			}
		}
		face.numPrims = 0;
		face.firstPrimID = 0;
		face.smoothingGroups = 0;
		
		state->faces[state->faceCount++] = face;
	}
	SrcPlane mapBboxPlanes[6];
	aabb mapAabb = GsrcModelsToSrcModels(mapData, state->models, &state->modelCount, mapBboxPlanes);
	
	BufferPushDataAndSetLumpSize(&buffer, fileHeader, SRC_LUMP_FACES, state->faces, sizeof(*state->faces) * state->faceCount);
	BufferPushDataAndSetLumpSize(&buffer, fileHeader, SRC_LUMP_VERTEXES, state->vertices, sizeof(*state->vertices) * state->vertexCount);
	BufferPushDataAndSetLumpSize(&buffer, fileHeader, SRC_LUMP_VERTNORMALS, state->vertNormals, sizeof(*state->vertNormals) * state->vertNormalCount);
	BufferPushDataAndSetLumpSize(&buffer, fileHeader, SRC_LUMP_VERTNORMALINDICES, state->vertNormalIndices, sizeof(*state->vertNormalIndices) * state->vertNormalIndexCount);
	BufferPushDataAndSetLumpSize(&buffer, fileHeader, SRC_LUMP_EDGES, state->edges, sizeof(*state->edges) * state->edgeCount);
	
	//
	// LUMP_ENTITIES
	//
	
	{
		ArenaTemp arenaTemp2 = ArenaBeginTemp(tempArena);
		EntList srcEnts = GsrcEntitiesToSrcEntities(tempArena, gsrcEnts, state->modelIsLadder);
		
		// NOTE(GameChaos): now, convert the new entities to a big string!
		str_builder srcEntLump = StrbuilderCreate(tempArena, mapData->lumpEntities.length * 4);
		for (i32 i = 0; i < srcEnts.entCount; i++)
		{
			EntProperties *ent = &srcEnts.ents[i];
			StrbuilderCat(&srcEntLump, STR("{\n"));
			for (i32 prop = 0; prop < ent->propertyCount; prop++)
			{
				StrbuilderCat(&srcEntLump, STR("\""));
				StrbuilderCat(&srcEntLump, ent->properties[prop].key);
				StrbuilderCat(&srcEntLump, STR("\" \""));
				StrbuilderCat(&srcEntLump, ent->properties[prop].value);
				StrbuilderCat(&srcEntLump, STR("\"\n"));
			}
			StrbuilderCat(&srcEntLump, STR("\"classname\" \""));
			StrbuilderCat(&srcEntLump, ent->classname);
			StrbuilderCat(&srcEntLump, STR("\"\n}\n"));
		}
		
		BufferPushDataAndSetLumpSize(&buffer, fileHeader, SRC_LUMP_ENTITIES, srcEntLump.data, (i32)srcEntLump.length);
		ArenaEndTemp(arenaTemp2);
	}
	//
	// LUMP_LIGHTING
	//
	
	{
		// TODO: is lightmap exposure correct? does goldsrc encode lightmaps with the srgb oetf?
		i32 srcLightCount = 0;
		ArenaTemp arenaTmp = ArenaBeginTemp(tempArena);
		Rgbe8888 *srcLight = ArenaAlloc(arena, (i64)mapData->lightingLength / 3 * 4);
		for (u8 *c = mapData->lumpLighting;
			 c < mapData->lumpLighting + mapData->lightingLength;
			 c += 3)
		{
			Rgbe8888 colour = *(Rgbe8888 *)c;
			colour.exponent = 0;
			v3 lin = SrcRgbe8888ToLinear(colour);
			lin = v3muls(lin, 1.5f);
			for (i32 i = 0; i < 3; i++)
			{
				lin.e[i] = powf(lin.e[i], 2.2f);
				//lin[i] = lin[i] * lin[i];
				lin.e[i] = GCM_MIN(1, lin.e[i]);
			}
			colour = SrcLinearToRgbe8888(lin);
			srcLight[srcLightCount++] = colour;
		}
		
		fileHeader->lump[SRC_LUMP_LIGHTING].version = 1;
		BufferPushDataAndSetLumpSize(&buffer, fileHeader, SRC_LUMP_LIGHTING, srcLight, sizeof(*srcLight) * srcLightCount);
		ArenaEndTemp(arenaTmp);
	}
	
	//
	// LUMP_LEAF_AMBIENT_INDEX
	//
	
	{
		ArenaTemp arenaTmp = ArenaBeginTemp(tempArena);
		i32 ambientIndexCount = 0;
		SrcLeafAmbientIndex *dummyAmbientIndex = ArenaAlloc(tempArena, sizeof(*dummyAmbientIndex) * mapData->leafCount);
		for (i32 leaf = 0; leaf < mapData->leafCount; leaf++)
		{
			SrcLeafAmbientIndex ambientIndex = {};
			ambientIndex.ambientSampleCount = 1;
			ambientIndex.firstAmbientSample = 0;
			dummyAmbientIndex[ambientIndexCount++] = ambientIndex;
		}
		BufferPushDataAndSetLumpSize(&buffer, fileHeader, SRC_LUMP_LEAF_AMBIENT_INDEX,
									 dummyAmbientIndex, sizeof(*dummyAmbientIndex) * ambientIndexCount);
		ArenaEndTemp(arenaTmp);
	}
	
	//
	// LUMP_LEAF_AMBIENT_LIGHTING
	//
	
	// NOTE(GameChaos): dummy ambient lighting
	{
		SrcLeafAmbientLighting dummyAmbient = {};
		for (i32 i = 0; i < 6; i++)
		{
			dummyAmbient.cube.colour[i].r = 1;
			dummyAmbient.cube.colour[i].g = 1;
			dummyAmbient.cube.colour[i].b = 1;
		}
		dummyAmbient.x = 128;
		dummyAmbient.y = 128;
		dummyAmbient.z = 128;
		
		fileHeader->lump[SRC_LUMP_LEAF_AMBIENT_LIGHTING].version = 1;
		BufferPushDataAndSetLumpSize(&buffer, fileHeader, SRC_LUMP_LEAF_AMBIENT_LIGHTING, &dummyAmbient, sizeof(dummyAmbient));
	}
	
	//
	// LUMP_WORLDLIGHTS
	//
	// NOTE(GameChaos): dummy worldlight
	// TODO: use light_environment entity! and convert other light entities!
	{
		SrcWorldLight worldLight = {};
		worldLight.intensity = v3fill(255);
		worldLight.type = SRC_EMIT_SURFACE;
		worldLight.normal = v3normalise((v3){1, 2, -3});
		fileHeader->lump[SRC_LUMP_WORLDLIGHTS].version = 1;
		BufferPushDataAndSetLumpSize(&buffer, fileHeader, SRC_LUMP_WORLDLIGHTS, &worldLight, sizeof(worldLight));
	}
	
	// ====================================================================
	// NOTE: this bit generates new brushes and planes and brush leaf data.
	// ====================================================================
	
	//
	// LUMP_NODES
	// LUMP_LEAFS
	// LUMP_BRUSHES
	// LUMP_LEAFBRUSHES
	// LUMP_LEAFFACES
	// LUMP_LEAFWATERDATA
	// LUMP_LEAFMINDISTTOWATER
	//
	
	// first convert nodes
	for (i32 i = 0; i < mapData->nodeCount; i++)
	{
		SrcNode node = {};
		
		node.plane = mapData->lumpNodes[i].plane;
		node.children[0] = mapData->lumpNodes[i].children[0];
		node.children[1] = mapData->lumpNodes[i].children[1];
		for (i32 j = 0; j < 3; j++)
		{
			node.mins[j] = mapData->lumpNodes[i].mins[j];
			node.maxs[j] = mapData->lumpNodes[i].maxs[j];
		}
		node.firstFace = mapData->lumpNodes[i].firstFace;
		node.faceCount = mapData->lumpNodes[i].faceCount;
		// TODO: assign a proper value!
		node.area = 0;
		
		state->nodes[state->nodeCount++] = node;
	}
	
	fileHeader->lump[SRC_LUMP_LEAFS].version = 1;
	i16 leafCluster = 0;
	
	// NOTE(GameChaos): index 0 solid leaf with no brushes, clusters,
	// faces or areas. can be shared between brushmodels.
	{
		SrcLeaf nullLeaf = {};
		nullLeaf.contents = SRC_CONTENTS_SOLID;
		nullLeaf.leafWaterDataIndex = -1;
		AddLeaf(state->leaves, &state->leafCount, nullLeaf);
	}
	
	SrcLeaf nullLeaf = {};
	for (i32 model = 0; model < mapData->modelCount; model++)
	{
		ArenaTemp arenaTmp = ArenaBeginTemp(tempArena);
		i64 polysByteCount = sizeof(Verts) * SRC_MAX_BRUSH_SIDES;
		Verts *polys = ArenaAlloc(tempArena, polysByteCount);
		
		GsrcBspTreeIterator *iter = GsrcBspTreeIteratorBegin(tempArena, mapData, model);
		i32 gsrcNodeIndex;
		while (GsrcBspTreeNext(iter, mapData, &gsrcNodeIndex))
		{
			for (i32 child = 0; child < 2; child++)
			{
				GsrcNode *gsrcNode = &mapData->lumpNodes[gsrcNodeIndex];
				i32 gsrcLeafIndex = -(i32)gsrcNode->children[child] - 1;
				if (gsrcLeafIndex >= 0)
				{
					GsrcLeaf gsrcLeaf = mapData->lumpLeaves[gsrcLeafIndex];
					// set the index correct for node
					state->nodes[gsrcNodeIndex].children[child] = -state->leafCount - 1;
					
					// make a new leaf!
					SrcLeaf leaf = MakeSrcLeaf(mapData, gsrcLeaf, state->modelIsLadder[model],
											   &leafCluster, state->leafFaces, &state->leafFaceCount);
					SrcBrush brush = {};
					if (gsrcLeaf.contents != GSRC_CONTENTS_EMPTY)
					{
						// make a brush!
						brush.firstSide = state->brushSideCount;
						brush.contents = leaf.contents;
						i32 polyCount = 0;
						Plat_MemSetToZero(polys, polysByteCount);
						
						// NOTE(GameChaos): static_persist because the struct is way too
						//  big to allocate on the stack
						static_persist SrcPlane planes[256];
						i32 maxClipPlanes = ARRAYCOUNT(planes);
						i32 clipPlaneCount = GetLeafClipPlanes(mapData, mapBboxPlanes, iter->parents, gsrcNodeIndex,
															   child, maxClipPlanes, planes);
						
						for (i32 planeInd = 0; planeInd < clipPlaneCount; planeInd++)
						{
							polys[polyCount].vertCount = 0;
							if (MakePolygon(planes, clipPlaneCount, planeInd, &polys[polyCount]))
							{
								SrcBrushSide side = {};
								if (polyCount > SRC_MAX_BRUSH_SIDES)
								{
									FatalError("Exceeded maximum amount of sides on brush! (128)");
								}
								ASSERT(state->planeCount < SRC_MAX_MAP_PLANES);
								side.plane = (u16)AddPlane(state->planes, &state->planeCount, planes[planeInd]);
								AddBrushSide(state->brushSides, &state->brushSideCount, side);
								brush.sides++;
								polyCount++;
								ASSERT(brush.sides == polyCount);
							}
						}
						
						// NOTE(GameChaos): calculate brush mins/maxs
						v3 mins = v3fill(F32_MAX);
						v3 maxs = v3fill(-F32_MAX);
						for (i32 i = 0; i < polyCount; i++)
						{
							for (i32 vert = 0; vert < polys[i].vertCount; vert++)
							{
								for (i32 xyz = 0; xyz < 3; xyz++)
								{
									mins.e[xyz] = GCM_MIN(polys[i].verts[vert].e[xyz], mins.e[xyz]);
									maxs.e[xyz] = GCM_MAX(polys[i].verts[vert].e[xyz], maxs.e[xyz]);
								}
							}
						}
#ifdef DEBUG_GRAPHICS
						if (brush.sides >= 4)
						{
							for (i32 polyInd = 0; polyInd < polyCount; polyInd++)
							{
								v3 normal = state->planes[state->brushSides[brush.firstSide + polyInd].plane].normal;
								DebugGfxAddBrushSide(&polys[polyInd], normal, -1, (v4){}, (v4){});
							}
							DebugGfxAddBrush(brush.sides);
						}
#endif
						
						// NOTE(GameChaos): generate bevel planes based on polygons
						AddBrushBevels(state, &brush, polys, polyCount, mins, maxs);
						
						if (polyCount >= 4)
						{
							leaf.firstLeafBrush = state->leafBrushCount;
							state->leafBrushes[state->leafBrushCount++] = (u16)state->brushCount;
							leaf.leafBrushes = 1;
							
							state->brushes[state->brushCount++] = brush;
						}
						else
						{
							state->brushSideCount = brush.firstSide;
							Warning("Invalid brush with <4 sides! Leaf index %i\n", gsrcLeafIndex);
						}
					}
					
					// NOTE(GameChaos): source does NOT like when a CONTENTS_SOLID leaf doesn't have any brushes.
					// it'll crash with a "Cannot load corrupted map." message.
					if (leaf.leafBrushes == 0)
					{
						leaf.firstLeafBrush = 0;
						leaf.contents = SRC_CONTENTS_EMPTY;
					}
					
					if (leaf.contents & SRC_CONTENTS_WATER)
					{
						leaf.leafWaterDataIndex = state->leafWaterDataCount;
						ASSERT(state->leafWaterDataCount < (i64)sizeof(state->leafWaterData));
						SrcLeafWaterData waterData = {0};
						// TODO: bounds check!!!
						state->leafWaterData[state->leafWaterDataCount++] = waterData;
					}
					else
					{
						leaf.leafWaterDataIndex = -1;
					}
					
					AddLeaf(state->leaves, &state->leafCount, leaf);
				}
			}
		}
		state->models[model].origin = (v3){};
		ArenaEndTemp(arenaTmp);
	}
	for (i32 i = 0; i < state->leafCount; i++)
	{
		state->leafMinDistToWater[state->leafMinDistToWaterCount++] = -1;
	}
	BufferPushDataAndSetLumpSize(&buffer, fileHeader, SRC_LUMP_NODES, state->nodes, sizeof(*state->nodes) * state->nodeCount);
	BufferPushDataAndSetLumpSize(&buffer, fileHeader, SRC_LUMP_LEAFS, state->leaves, sizeof(*state->leaves) * state->leafCount);
	BufferPushDataAndSetLumpSize(&buffer, fileHeader, SRC_LUMP_LEAFBRUSHES, state->leafBrushes, sizeof(*state->leafBrushes) * state->leafBrushCount);
	BufferPushDataAndSetLumpSize(&buffer, fileHeader, SRC_LUMP_LEAFFACES, state->leafFaces, sizeof(*state->leafFaces) * state->leafFaceCount);
	BufferPushDataAndSetLumpSize(&buffer, fileHeader, SRC_LUMP_LEAFWATERDATA, state->leafWaterData, sizeof(*state->leafWaterData) * state->leafWaterDataCount);
	BufferPushDataAndSetLumpSize(&buffer, fileHeader, SRC_LUMP_LEAFMINDISTTOWATER, state->leafMinDistToWater, sizeof(*state->leafMinDistToWater) * state->leafMinDistToWaterCount);
	BufferPushDataAndSetLumpSize(&buffer, fileHeader, SRC_LUMP_MODELS, state->models, sizeof(*state->models) * state->modelCount);
	BufferPushDataAndSetLumpSize(&buffer, fileHeader, SRC_LUMP_TEXINFO, state->texinfo, sizeof(*state->texinfo) * state->texinfoCount);
	
	
	//
	// LUMP_ORIGINALFACES
	//
	
	BufferPushDataAndSetLumpSize(&buffer, fileHeader, SRC_LUMP_ORIGINALFACES, state->faces, sizeof(*state->faces) * state->faceCount);
	
	//
	// LUMP_FACEIDS
	//
	// TODO
	
	
	//
	// LUMP_SURFEDGES
	//
	
	BufferPushDataAndSetLumpSize(&buffer, fileHeader, SRC_LUMP_SURFEDGES,
								 mapData->lumpSurfEdges, sizeof(*mapData->lumpSurfEdges) * mapData->surfEdgeCount);
	
	//
	// LUMP_BRUSHES
	//
	BufferPushDataAndSetLumpSize(&buffer, fileHeader, SRC_LUMP_BRUSHES, state->brushes, sizeof(*state->brushes) * state->brushCount);
	
	// 
	// LUMP_BRUSHSIDES
	//
	
	BufferPushDataAndSetLumpSize(&buffer, fileHeader, SRC_LUMP_BRUSHSIDES,
								 state->brushSides, sizeof(*state->brushSides) * state->brushSideCount);
	
	//
	// LUMP_PLANES
	//
	
	BufferPushDataAndSetLumpSize(&buffer, fileHeader, SRC_LUMP_PLANES, state->planes, sizeof(*state->planes) * state->planeCount);
	
	//
	// LUMP_AREAS
	//
	
	// set placeholder data since areas aren't used.
	SrcArea areas[2] = {};
	// NOTE(GameChaos): 0 is solid, 1 is playable area apparently. don't know if this is needed at all
	areas[1].areaportalIndex = 1;
	BufferPushDataAndSetLumpSize(&buffer, fileHeader, SRC_LUMP_AREAS, areas, sizeof(areas));
	
	//
	// LUMP_AREAPORTALS
	//
	
	// set placeholder data since areaportals aren't used.
	SrcAreaportal areaportal = {};
	BufferPushDataAndSetLumpSize(&buffer, fileHeader, SRC_LUMP_AREAPORTALS, &areaportal, sizeof(areaportal));
	
	// save bsp
	result = WriteEntireFile(outputPath, buffer.memory, BufferGetSize(buffer));
	
	ArenaEndTemp(arenaTemp);
	return result;
}
