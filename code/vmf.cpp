
// TODO: don't use global variables!
global SrcLeaf g_leaves[SRC_MAX_MAP_LEAFS];
global SrcNode g_nodes[SRC_MAX_MAP_NODES];
global Face g_faces[SRC_MAX_MAP_FACES];
global SrcModel g_models[SRC_MAX_MAP_MODELS];
global u16 g_leafBrushes[SRC_MAX_MAP_LEAFBRUSHES];
global u16 g_leafFaces[SRC_MAX_MAP_LEAFFACES];
global Brush g_brushes[SRC_MAX_MAP_BRUSHES];
global SrcPlane g_planes[SRC_MAX_MAP_PLANES];
global Texture g_textures[SRC_MAX_MAP_PLANES];

global s64 g_hammerId = 1;

char g_vmfStart[] = "versioninfo\n"
"{\n"
"	\"editorversion\" \"400\"\n"
"	\"editorbuild\" \"0\"\n"
"	\"mapversion\" \"1\"\n"
"	\"formatversion\" \"100\"\n"
"	\"prefab\" \"0\"\n"
"}\n"
"visgroups\n"
"{\n"
"}\n"
"viewsettings\n"
"{\n"
"	\"bSnapToGrid\" \"1\"\n"
"	\"bShowGrid\" \"1\"\n"
"	\"bShowLogicalGrid\" \"0\"\n"
"	\"nGridSpacing\" \"64\"\n"
"}\n";

char g_vmfEnd[] = "cameras\n"
"{\n"
"	\"activecamera\" \"-1\"\n"
"}\n"
"cordons\n"
"{\n"
"	\"active\" \"0\"\n"
"}\n";

internal s64 Id(void)
{
	s64 result = g_hammerId++;
	return result;
}

internal void WriteBrush(str_builder *out, Brush brush, Texture *textures, s32 textureCount, b32 vertsPlusplus = true)
{
	StrbuilderCat(out, STR("\tsolid\n\t{\n"));
	StrbuilderPushFormat(out, "\t\t\"id\" \"%lli\"\n", Id());
	
	for (s32 sideInd = 0; sideInd < brush.sideCount; sideInd++)
	{
		BrushSide *side = &brush.sides[sideInd];
		StrbuilderCat(out, STR("\t\tside\n\t\t{\n"));
		StrbuilderPushFormat(out, "\t\t\t\"id\" \"%lli\"\n", Id());
		StrbuilderPushFormat(out, "\t\t\t\"plane\" \"(%f %f %f) (%f %f %f) (%f %f %f)\"\n",
							 side->polygon.verts[0].x, side->polygon.verts[0].y, side->polygon.verts[0].z, 
							 side->polygon.verts[1].x, side->polygon.verts[1].y, side->polygon.verts[1].z, 
							 side->polygon.verts[2].x, side->polygon.verts[2].y, side->polygon.verts[2].z);
		if (vertsPlusplus)
		{
			StrbuilderCat(out, STR("\t\t\tvertices_plus\n\t\t\t{\n"));
			for (s32 v = 0; v < side->polygon.vertCount; v++)
			{
				StrbuilderPushFormat(out, "\t\t\t\t\"v\" \"%f %f %f\"\n",
									 side->polygon.verts[v].x, side->polygon.verts[v].y, side->polygon.verts[v].z);
				
			}
			StrbuilderCat(out, STR("\t\t\t}\n"));
		}
		Texture *tex = &textures[side->texture];
		StrbuilderPushFormat(out, "\t\t\t\"material\" \"%s\"\n", tex->name);
		StrbuilderPushFormat(out, "\t\t\t\"uaxis\" \"[%f %f %f %f] %f\"\n",
							 tex->vecs[0].vec.x, tex->vecs[0].vec.y, tex->vecs[0].vec.z, tex->vecs[0].shift, tex->vecs[0].scale);
		StrbuilderPushFormat(out, "\t\t\t\"vaxis\" \"[%f %f %f %f] %f\"\n",
							 tex->vecs[1].vec.x, tex->vecs[1].vec.y, tex->vecs[1].vec.z, tex->vecs[1].shift, tex->vecs[1].scale);
		StrbuilderPushFormat(out, "%s", "\t\t\t\"rotation\" \"0\"\n");
		StrbuilderPushFormat(out, "%s", "\t\t\t\"lightmapscale\" \"16\"\n");
		StrbuilderPushFormat(out, "%s", "\t\t\t\"smoothing_groups\" \"0\"\n");
		StrbuilderCat(out, STR("\t\t}\n"));
	}
	
	StrbuilderCat(out, STR("\t}\n"));
}

internal f32 CrossV2(v2 a, v2 b)
{
	f32 result = a.x * b.y - a.y * b.x;
	return result;
}

// check if a point is on the LEFT side of an edge
internal b32 Inside(v2 p, v2 p1, v2 p2)
{
    b32 result = ((p2.y - p1.y) * p.x
				  + (p1.x - p2.x) * p.y
				  + (p2.x * p1.y - p1.x * p2.y)) < 0;
	return result;
}

internal v2 Intersection(v2 cp1, v2 cp2,
						 v2 s, v2 e)
{
	v2 dc = cp1 - cp2;
	v2 dp = s - e;
	
	f32 n1 = CrossV2(cp1, cp2);
	f32 n2 = CrossV2(s, e);
	f32 n3 = CrossV2(dc, dp);
	
	v2 result;
	if (n3 == 0)
	{
		result = (cp1 + cp2) * 0.5f;
	}
	else
	{
		result = (n1 * dp - n2 * dc) / n3;
	}
	return result;
}

internal f32 Polygon2DArea(Polygon2D *poly)
{
	f32 result = 0;
	s32 vert2 = poly->vertCount - 1;
	for (s32 vert1 = 0; vert1 < poly->vertCount; vert1++)
	{
		result += (poly->verts[vert2].x + poly->verts[vert1].x)
			* (poly->verts[vert2].y - poly->verts[vert1].y); 
		vert2 = vert1;
	}
	result *= 0.5f;
	return HMM_ABS(result);
}

internal void Polygon2DIntersect(Polygon2D *a, Polygon2D *b, Polygon2D *out)
{
	local_persist Polygon2D helpPoly;
	helpPoly.vertCount = 0;
	v2 cp1, cp2, s, e;
	
    // copy subject polygon to new polygon and set its size
    for (s32 i = 0; i < a->vertCount; i++)
	{
        out->verts[i] = a->verts[i];
	}
	
    out->vertCount = a->vertCount;
	
    for (s32 j = 0; j < b->vertCount; j++)
    {
        // copy new polygon to input polygon & set counter to 0
        for (s32 k = 0; k < out->vertCount; k++)
		{
			helpPoly.verts[k] = out->verts[k];
		}
        s32 counter = 0;
		
        // get clipping polygon edge
        cp1 = b->verts[j];
        cp2 = b->verts[(j + 1) % b->vertCount];
		
        for (s32 i = 0; i < out->vertCount; i++)
        {
            // get subject polygon edge
            s = helpPoly.verts[i];
            e = helpPoly.verts[(i + 1) % out->vertCount];
			
			b32 sInside = Inside(s, cp1, cp2);
			b32 eInside = Inside(e, cp1, cp2);
            if (sInside && eInside)
			{
                out->verts[counter++] = e;
			}
            else if (!sInside && eInside)
            {
                out->verts[counter++] = Intersection(cp1, cp2, s, e);
                out->verts[counter++] = e;
            }
            else if (sInside && !eInside)
			{
                out->verts[counter++] = Intersection(cp1, cp2, s, e);
			}
        }
        // set new polygon size
		ASSERT(counter < SRC_MAX_SIDE_VERTS);
        out->vertCount = counter;
    }
}

internal b32 VmfFromGoldsource(Arena *arena, Arena *tempArena, GsrcMapData *mapData, char *outputPath, char *modPath, char *valvePath, char *assetPath)
{
	b32 result = false;
	ASSERT(outputPath);
	ASSERT(mapData);
	if (!outputPath)
	{
		Error("Invalid output path provided to converter!\n");
		return result;
	}
	
	if (!arena || !tempArena || !mapData)
	{
		return result;
	}
	
	ArenaTemp arenaTemp = ArenaBeginTemp(arena);
	// load WADs
	local_persist Wad3 wads[MAX_WADS] = {};
	s32 wadCount = LoadWads(arena, modPath, valvePath, wads);
	
	EntList gsrcEnts = GsrcParseEntities(arena, mapData->lumpEntities);
	// NOTE(GameChaos): don't convert to source entities for now. vmfs don't require conversion anyway.
	EntList srcEnts = gsrcEnts;
	
	// convert skybox textures
	FileWritingBuffer texBuffer = BufferCreate(tempArena, 8192 * 8192 * 3);
	if (assetPath)
	{
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
			for (s32 side = 0; side < SKY_SIDE_COUNT; side++)
			{
				BufferReset(&texBuffer);
				
				b32 gotTexture = GsrcGetSkyTexture(tempArena, &texBuffer, skyname->value, modPath, valvePath, (SkySide)side);
				if (gotTexture)
				{
					char vtfFileName[128];
					Format(vtfFileName, sizeof(vtfFileName), "materials/skybox/%.*s%s.vtf",
						   skyname->value.length, skyname->value.data, g_skySides[side]);
					
					char pathBuffer[512];
					Format(pathBuffer, sizeof(pathBuffer), "%s", assetPath);
					AppendToPath(pathBuffer, sizeof(pathBuffer), vtfFileName);
					WriteEntireFile(pathBuffer, texBuffer.memory, (u32)texBuffer.usedBytes);
					
					char texturePath[128];
					s32 texturePathLen = Format(texturePath, sizeof(texturePath), "skybox/%.*s%s",
												skyname->value.length, skyname->value.data, g_skySides[side]);
					
					char vmt[512];
					s32 vmtFileLength = MakeSkyTextureVmt(vmt, sizeof(vmt), texturePath);
					
					char vmtFileName[128];
					Format(vmtFileName, sizeof(vmtFileName), "materials/%s.vmt", texturePath);
					Format(pathBuffer, sizeof(pathBuffer), "%s", assetPath);
					AppendToPath(pathBuffer, sizeof(pathBuffer), vmtFileName);
					
					WriteEntireFile(pathBuffer, vmt, vmtFileLength);
				}
				else
				{
					// TODO: make blank vmt when loading fails?
					//ASSERT(0);
				}
			}
		}
	}
	
	// convert GSRC_LUMP_TEXTURES into files
	// TODO: this should be merged with bsp.cpp somehow
#ifdef DEBUG_GRAPHICS
	if (true)
#else
	if (assetPath)
#endif
	{
		for (u32 i = 0; i < mapData->lumpTextures.mipTextureCount; i++)
		{
			if (mapData->lumpTextures.mipTextureOffsets[i] <= 0)
			{
				continue;
			}
			GsrcMipTexture mipTexture = *mapData->lumpTextures.mipTextures[i];
			u8 *textureData = (u8 *)mapData->lumpTextures.mipTextures[i];
			// NOTE(GameChaos): wad3 texture header and bsp miptexture structs are
			// the exact same and they point to the exact same data as well!
			static_assert(sizeof(Wad3TextureHeader) == sizeof(GsrcMipTexture), "");
			if (mipTexture.offsets[0] <= 0)
			{
				FindTextureResult find = FindTextureInWads(wads, wadCount, mipTexture.name);
				if (find.found)
				{
					mipTexture = *(GsrcMipTexture *)wads[find.wadIndex].textures[find.textureIndex];
					textureData = (u8 *)wads[find.wadIndex].textures[find.textureIndex];
				}
				else
				{
					Warning("Couldn't find texture %s from wad files :(\n", mipTexture.name);
					continue;
				}
			}
			
			BufferReset(&texBuffer);
			GsrcMipTextureToVtf(tempArena, &texBuffer, mipTexture, textureData);
#ifdef DEBUG_GRAPHICS
			// TODO: both bsp.cpp and this can double texture count cos they both convert textures separately.
			// do something about this!
			DebugGfxAddMiptexture(tempArena, mipTexture, textureData);
#endif
			
			char pathBuffer[512];
#ifdef DEBUG_GRAPHICS
			if (assetPath)
#endif
			{
				Format(pathBuffer, sizeof(pathBuffer), "%s", assetPath);
				char vtfFileName[64];
				Format(vtfFileName, sizeof(vtfFileName), CONVERTED_MATERIAL_PATH "%s.vtf", mipTexture.name);
				AppendToPath(pathBuffer, sizeof(pathBuffer), vtfFileName);
				WriteEntireFile(pathBuffer, texBuffer.memory, (u32)texBuffer.usedBytes);
			}
			
			char vmt[512];
			// NOTE(GameChaos): { means transparent
			b32 transparent = mipTexture.name[0] == '{';
			s32 vmtFileLength = MakeVmt(vmt, sizeof(vmt), mipTexture.name, transparent);
			
			if (assetPath)
			{
				Format(pathBuffer, sizeof(pathBuffer), assetPath);
				char vmtFileName[64];
				s32 vmtFileNameLen = Format(vmtFileName, sizeof(vmtFileName), CONVERTED_MATERIAL_PATH "%s.vmt", mipTexture.name);
				AppendToPath(pathBuffer, sizeof(pathBuffer), vmtFileName);
				WriteEntireFile(pathBuffer, vmt, vmtFileLength);
			}
		}
	}
	
	//
	// g_textures
	//
	
	s32 textureCount = 0;
	// NOTE(GameChaos): first texture is nodraw
	{
		Texture texture = {};
		texture.vecs[0].vec = Vec3(1, 0, 0);
		texture.vecs[1].vec = Vec3(0, 1, 0);
		texture.vecs[0].scale = 0.25f;
		texture.vecs[1].scale = 0.25f;
		Format(texture.name, sizeof(texture.name), "%s", "tools/toolsnodraw");
		g_textures[textureCount++] = texture;
	}
	for (s32 i = 0; i < mapData->texinfoCount; i++)
	{
		Texture texture = {};
		
		for (s32 st = 0; st < 2; st++)
		{
			for (s32 xyz = 0; xyz < 3; xyz++)
			{
				texture.vecs[st].vec[xyz] = mapData->lumpTexinfo[i].vecs[st][xyz];
			}
			texture.vecs[st].shift = mapData->lumpTexinfo[i].vecs[st][3];
		}
		for (s32 st = 0; st < 2; st++)
		{
			f32 len = Length(texture.vecs[st].vec);
			texture.vecs[st].scale = 1;
			if (len > 0)
			{
				f32 invLen = (1.0f / len);
				texture.vecs[st].vec *= invLen;
				texture.vecs[st].scale = invLen;
			}
		}
		char *texName = mapData->lumpTextures.mipTextures[mapData->lumpTexinfo[i].miptex]->name;
		Format(texture.name, sizeof(texture.name), CONVERTED_MATERIAL_FOLDER "%s", texName);
		g_textures[textureCount++] = texture;
	}
	
	s32 planeCount = 0;
	// NOTE: source and goldsrc plane structs are the same, that's why we can do this.
	Mem_Copy(mapData->lumpPlanes, g_planes, sizeof(*mapData->lumpPlanes) * mapData->planeCount,
			 sizeof(*g_planes) * SRC_MAX_MAP_PLANES);
	planeCount = mapData->planeCount;
	
	s32 faceCount = 0;
	for (s32 i = 0; i < mapData->faceCount; i++)
	{
		Face *face = &g_faces[faceCount++];
		*face = {};
		face->plane = g_planes[mapData->lumpFaces[i].plane];
		if (mapData->lumpFaces[i].planeSide)
		{
			face->plane.normal = -face->plane.normal;
			face->plane.distance = -face->plane.distance;
		}
		face->bounds.mins = Vec3(F32_MAX, F32_MAX, F32_MAX);
		face->bounds.maxs = Vec3(-F32_MAX, -F32_MAX, -F32_MAX);
		s32 firstEdge = mapData->lumpFaces[i].firstEdge;
		s32 edges = mapData->lumpFaces[i].edges;
		// TODO: verify that the polygon generated is corrected
		for (s32 surfEdgeInd = firstEdge;
			 surfEdgeInd < firstEdge + edges;
			 surfEdgeInd++)
		{
			s32 surfEdge = mapData->lumpSurfEdges[surfEdgeInd];
			GsrcEdge edge = mapData->lumpEdges[HMM_ABS(surfEdge)];
			v3 vertex;
			if (surfEdge < 0)
			{
				vertex = mapData->lumpVertices[edge.vertex[1]];
			}
			else
			{
				vertex = mapData->lumpVertices[edge.vertex[0]];
			}
			face->polygon.verts[face->polygon.vertCount++] = vertex;
			for (s32 xyz = 0; xyz < 3; xyz++)
			{
				face->bounds.mins[xyz] = HMM_MIN(face->bounds.mins[xyz], vertex[xyz]);
				face->bounds.maxs[xyz] = HMM_MAX(face->bounds.maxs[xyz], vertex[xyz]);
			}
		}
		face->texture = mapData->lumpFaces[i].texInfoIndex + 1;
	}
	local_persist b32 vertDone[SRC_MAX_MAP_VERTS] = {};
	
	SrcPlane mapBboxPlanes[6];
	s32 modelCount = 0;
	aabb mapAabb = GsrcModelsToSrcModels(mapData, g_models, &modelCount, mapBboxPlanes);
	
	//EntList srcEnts = GsrcEntitiesToSrcEntities(arena, gsrcEnts, NULL);
	
	// ====================================================================
	// NOTE: this bit generates new brushes and planes and brush leaf data.
	// ====================================================================
	
	//
	// LUMP_NODES
	// LUMP_LEAFS
	// LUMP_BRUSHES
	// LUMP_LEAFBRUSHES
	// LUMP_LEAFFACES
	//
	
	// first convert nodes
	s32 nodeCount = 0;
	for (s32 i = 0; i < mapData->nodeCount; i++)
	{
		SrcNode node = {};
		
		node.plane = mapData->lumpNodes[i].plane;
		node.children[0] = mapData->lumpNodes[i].children[0];
		node.children[1] = mapData->lumpNodes[i].children[1];
		for (s32 j = 0; j < 3; j++)
		{
			node.mins[j] = mapData->lumpNodes[i].mins[j];
			node.maxs[j] = mapData->lumpNodes[i].maxs[j];
		}
		node.firstFace = mapData->lumpNodes[i].firstFace;
		node.faceCount = mapData->lumpNodes[i].faceCount;
		// TODO: assign a proper value!
		node.area = 0;
		
		g_nodes[nodeCount++] = node;
	}
	
	s32 leafCount = 0;
	u16 leafBrushCount = 0;
	u16 leafFaceCount = 0;
	s16 leafCluster = 0;
	
	// NOTE(GameChaos): index 0 solid leaf with no brushes, clusters,
	// faces or areas. can be shared between brushmodels.
	{
		SrcLeaf nullLeaf = {};
		nullLeaf.contents = SRC_CONTENTS_SOLID;
		nullLeaf.leafWaterDataIndex = -1;
		AddLeaf(g_leaves, &leafCount, nullLeaf);
	}
	
	s32 brushCount = 0;
	
	SrcLeaf nullLeaf = {};
	for (s32 model = 0; model < mapData->modelCount; model++)
	{
		ArenaTemp arenaTmp = ArenaBeginTemp(tempArena);
		GsrcBspTreeIterator *iter = GsrcBspTreeIteratorBegin(tempArena, mapData, model);
		s32 gsrcNodeIndex;
		while (GsrcBspTreeNext(iter, mapData, &gsrcNodeIndex))
		{
			for (s32 child = 0; child < 2; child++)
			{
				GsrcNode *gsrcNode = &mapData->lumpNodes[gsrcNodeIndex];
				s32 gsrcLeafIndex = -(s32)gsrcNode->children[child] - 1;
				if (gsrcLeafIndex >= 0)
				{
					GsrcLeaf gsrcLeaf = mapData->lumpLeaves[gsrcLeafIndex];
					// make a new leaf!
					SrcLeaf leaf = {};
					// set the index correct for node
					g_nodes[gsrcNodeIndex].children[child] = -leafCount - 1;
					
					if (gsrcLeaf.contents != GSRC_CONTENTS_EMPTY)
					{
						// TODO: something something LUMP_LEAFWATERDATA something LUMP_LEAFMINDISTTOWATER something
						leaf.contents = GsrcContentsToSrcContents(gsrcLeaf.contents);
						leaf.cluster = -1;
						leaf.area = 0;
					}
					else
					{
						leaf.area = 0;
						leaf.cluster = leafCluster++;
					}
					leaf.flags = SRC_LEAF_FLAGS_SKY2D;
					for (s32 j = 0; j < 3; j++)
					{
						leaf.mins[j] = gsrcLeaf.mins[j];
						leaf.maxs[j] = gsrcLeaf.maxs[j];
					}
					
					leaf.firstLeafFace = leafFaceCount;
					// NOTE(GameChaos): loop through the faces and set their texinfo flags semi-properly.
					for (s32 markSurf = gsrcLeaf.firstMarkSurface;
						 markSurf < gsrcLeaf.firstMarkSurface + gsrcLeaf.markSurfaces;
						 markSurf++)
					{
						// NOTE: LUMP_MARKSURFACES == LUMP_LEAFFACES
						u16 faceInd = mapData->lumpMarksurfaces[markSurf];
						g_leafFaces[leafFaceCount++] = faceInd;
					}
					leaf.leafFaces = leafFaceCount - leaf.firstLeafFace;
					
					if (gsrcLeaf.contents != GSRC_CONTENTS_EMPTY)
					{
						// make a brush!
						local_persist SrcPlane planes[256];
						s32 maxClipPlanes = ARRAYCOUNT(planes);
						s32 clipPlaneCount = GetLeafClipPlanes(mapData, mapBboxPlanes, iter->parents, gsrcNodeIndex,
															   child, maxClipPlanes, planes);
						
						local_persist Verts poly = {};
						Brush brush = {};
						brush.contents = leaf.contents;
						brush.model = model;
						
						for (s32 planeInd = 0; planeInd < clipPlaneCount; planeInd++)
						{
							poly.vertCount = 0;
							if (MakePolygon(planes, clipPlaneCount, planeInd, &poly))
							{
								BrushSide *side = (BrushSide *)ArenaAlloc(arena, sizeof(BrushSide));
								if (brush.sides == NULL)
								{
									brush.sides = side;
								}
								brush.sideCount++;
								*side = {};
								
								side->polygon = poly;
								side->plane = planes[planeInd];
							}
						}
						
						// NOTE(GameChaos): calculate brush mins/maxs
						v3 mins = Vec3(F32_MAX, F32_MAX, F32_MAX);
						v3 maxs = Vec3(-F32_MAX, -F32_MAX, -F32_MAX);
						for (s32 i = 0; i < brush.sideCount; i++)
						{
							for (s32 vert = 0; vert < brush.sides[i].polygon.vertCount; vert++)
							{
								for (s32 xyz = 0; xyz < 3; xyz++)
								{
									mins[xyz] = HMM_MIN(brush.sides[i].polygon.verts[vert][xyz], mins[xyz]);
									maxs[xyz] = HMM_MAX(brush.sides[i].polygon.verts[vert][xyz], maxs[xyz]);
								}
							}
						}
						brush.bounds.mins = mins;
						brush.bounds.maxs = maxs;
						
						if (brush.sideCount >= 4)
						{
							leaf.firstLeafBrush = leafBrushCount;
							g_leafBrushes[leafBrushCount++] = (u16)brushCount;
							leaf.leafBrushes = 1;
							
							g_brushes[brushCount++] = brush;
						}
						else
						{
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
					
					AddLeaf(g_leaves, &leafCount, leaf);
				}
			}
		}
		g_models[model].origin = {};
		ArenaEndTemp(arenaTmp);
	}
	
	// NOTE(GameChaos): match face textures with brush sides
	s32 sideCount = 0;
	for (s32 brushInd = 0; brushInd < brushCount; brushInd++)
	{
		Brush *brush = &g_brushes[brushInd];
		
		for (s32 brushSideInd = 0; brushSideInd < brush->sideCount; brushSideInd++)
		{
			sideCount++;
			BrushSide *side = &brush->sides[brushSideInd];
			v3 nonParallel = GetNonParallelVector(side->plane.normal);
			v3 i = Cross(side->plane.normal, nonParallel);
			v3 j = Cross(side->plane.normal, i);
			local_persist Polygon2D a;
			a.vertCount = 0;
			for (s32 v = side->polygon.vertCount - 1; v >= 0; v--)
			{
				a.verts[a.vertCount++] = Vec2(Dot(side->polygon.verts[v], i), Dot(side->polygon.verts[v], j));
			}
#ifdef DEBUG_GRAPHICS
			// NOTE(GameChaos): visualise polygon intersection
			s32 intersectCount = 0;
			s32 debugSide = 7729;
#endif
			f32 sideArea = Polygon2DArea(&a);
			if (sideArea > 0)
			{
				f32 sideAreaInv = 1.0f / sideArea;
				s32 validTextures[64];
				f32 validTextureArea[64];
				s32 validTexCount = 0;
				GsrcModel *model = &mapData->lumpModels[brush->model];
				for (s32 faceInd = model->firstFace;
					 faceInd < model->firstFace + model->faces;
					 faceInd++)
				{
					//s32 faceInd = mapData->lumpMarksurfaces[markSurf];
					Face *face = &g_faces[faceInd];
					if (PlaneEquals(side->plane, face->plane.normal, face->plane.distance)
						&& AabbCheck(brush->bounds, face->bounds))
					{
						Verts *polys[2] = {&side->polygon, &face->polygon};
						local_persist Polygon2D b;
						b.vertCount = 0;
						for (s32 v = face->polygon.vertCount - 1; v >= 0; v--)
						{
							b.verts[b.vertCount++] = Vec2(Dot(face->polygon.verts[v], i), Dot(face->polygon.verts[v], j));
						}
						local_persist Polygon2D intersected;
						intersected.vertCount = 0;
						Polygon2DIntersect(&b, &a, &intersected);
						b32 intersects = intersected.vertCount >= 3;
						if (intersects)
						{
							f32 intersectedArea = Polygon2DArea(&intersected);
#ifdef DEBUG_GRAPHICS
							//Print("side %i intersected with face %i\n", sideCount, faceInd);
							// NOTE(GameChaos): visualise polygon intersection
							if (sideCount == debugSide)
							{
								Print("side %i intersected with face %i area %f verts %i\n",
									  sideCount, faceInd, intersectedArea, intersected.vertCount);
								intersectCount++;
								local_persist Verts b3d;
								local_persist Verts intersected3d;
								b3d.vertCount = 0;
								intersected3d.vertCount = 0;
								for (s32 v = b.vertCount - 1;
									 v >= 0; v--)
								{
									b3d.verts[b3d.vertCount].xy = b.verts[v];
									b3d.verts[b3d.vertCount].z = (f32)intersectCount * 32 + 16;
									b3d.vertCount++;
								}
								for (s32 v = intersected.vertCount - 1;
									 v >= 0; v--)
								{
									intersected3d.verts[intersected3d.vertCount].xy = intersected.verts[v];
									intersected3d.verts[intersected3d.vertCount].z = (f32)intersectCount * 32 + 32;
									intersected3d.vertCount++;
								}
								v3 normal = {0, 0, 1};
								v4 s = {0.25f, 0, 0, 0};
								v4 t = {0, 0.25f, 0, 0};
								s32 texture = mapData->lumpTexinfo[face->texture - 1].miptex;
								// TODO: DebugGfxAddMesh2D
								DebugGfxAddMesh(&b3d, normal, {0, 1, 0}, texture, s, t);
								DebugGfxAddMesh(&intersected3d, normal, {0, 0, 1}, -1, s, t);
							}
#endif
							b32 found = false;
							for (s32 tex = 0; tex < validTexCount; tex++)
							{
								if (face->texture == validTextures[tex])
								{
									validTextureArea[tex] += intersectedArea * sideAreaInv;
									found = true;
									break;
								}
							}
							ASSERT(validTexCount < ARRAYCOUNT(validTextures));
							if (!found && validTexCount < ARRAYCOUNT(validTextures))
							{
								validTextures[validTexCount] = face->texture;
								validTextureArea[validTexCount] = intersectedArea * sideAreaInv;
								validTexCount++;
							}
							// NOTE(GameChaos): if no textures are found, then just use one
							//side->texture = face->texture;
						}
					}
					//Face *face
				}
				
				
				f32 biggestArea = -F32_INFINITY;
				s32 biggestInd = S32_MIN;
				for (s32 tex = 0; tex < validTexCount; tex++)
				{
					if (validTextureArea[tex] > biggestArea)
					{
						biggestArea = validTextureArea[tex];
						biggestInd = tex;
					}
				}
				if (biggestInd >= 0)
				{
					side->texture = validTextures[biggestInd];
				}
				
				s32 texture = -1;
				if (side->texture > 0)
				{
					GsrcTexinfo texinfo = mapData->lumpTexinfo[side->texture - 1];
					texture = (s32)texinfo.miptex;
				}
#ifdef DEBUG_GRAPHICS
				// NOTE(GameChaos): visualise polygon intersection
				if (sideCount == debugSide)
				{
					Print("side %i intersect count %i detected texture %i\n",
						  sideCount, intersectCount, side->texture);
					// NOTE(GameChaos): visualise polygon intersection
					local_persist Verts a3d;
					a3d.vertCount = 0;
					for (s32 v = a.vertCount - 1;
						 v >= 0; v--)
					{
						a3d.verts[a3d.vertCount].xy = a.verts[v];
						a3d.verts[a3d.vertCount].z = 0;
						a3d.vertCount++;
					}
					v3 normal = {0, 0, 1};
					DebugGfxAddMesh(&a3d, normal, {1, 0, 0}, texture, {0.25f, 0, 0, 0}, {0, 0.25f, 0, 0});
				}
#endif
			}
			if (side->texture > 0)
			{
				GsrcTexinfo texinfo = mapData->lumpTexinfo[side->texture - 1];
				v4 s = *(v4 *)texinfo.vecs[0];
				v4 t = *(v4 *)texinfo.vecs[1];
				DebugGfxAddBrushSide(&side->polygon, side->plane.normal, (s32)texinfo.miptex, s, t);
			}
			else
			{
				DebugGfxAddBrushSide(&side->polygon, side->plane.normal);
			}
		}
		DebugGfxAddBrush(brush->sideCount);
	}
	
	str_builder vmf = StrbuilderCreate(arena, GIGABYTES(1));
	StrbuilderCat(&vmf, StrFromSize(g_vmfStart, sizeof(g_vmfStart) - 1));
	
	for (s32 i = 0; i < srcEnts.entCount; i++)
	{
		EntProperties *ent = &srcEnts.ents[i];
		if (StrEquals(ent->classname, STR("worldspawn")))
		{
			StrbuilderCat(&vmf, STR("world\n{\n"));
			StrbuilderPushFormat(&vmf, "\t\"id\" \"%lli\"\n", Id());
			StrbuilderCat(&vmf, STR("\t\"mapversion\" \"1\"\n"));
			StrbuilderPushFormat(&vmf, "\t\"classname\" \"%.*s\"\n", ent->classname.length, ent->classname.data);
			for (s32 prop = 0; prop < ent->propertyCount; prop++)
			{
				StrbuilderPushFormat(&vmf, "\t\"%.*s\" \"%.*s\"\n",
									 ent->properties[prop].key.length, ent->properties[prop].key.data,
									 ent->properties[prop].value.length,ent->properties[prop].value.data);
			}
			// NOTE(GameChaos): write world solids
			for (s32 brushInd = 0; brushInd < brushCount; brushInd++)
			{
				Brush brush = g_brushes[brushInd];
				if (brush.model == 0)
				{
					WriteBrush(&vmf, brush, g_textures, textureCount, false);
				}
			}
			StrbuilderCat(&vmf, STR("}\n"));
		}
		else
		{
			StrbuilderCat(&vmf, STR("entity\n{\n"));
			StrbuilderPushFormat(&vmf, "\t\"id\" \"%lli\"\n", Id());
			StrbuilderPushFormat(&vmf, "\t\"classname\" \"%.*s\"\n", ent->classname.length, ent->classname.data);
			for (s32 prop = 0; prop < ent->propertyCount; prop++)
			{
				StrbuilderPushFormat(&vmf, "\t\"%.*s\" \"%.*s\"\n",
									 ent->properties[prop].key.length, ent->properties[prop].key.data,
									 ent->properties[prop].value.length,ent->properties[prop].value.data);
			}
			// NOTE(GameChaos): write solids
			if (ent->model > 0)
			{
				for (s32 brushInd = 0; brushInd < brushCount; brushInd++)
				{
					Brush brush = g_brushes[brushInd];
					if (brush.model == ent->model)
					{
						WriteBrush(&vmf, brush, g_textures, textureCount, false);
					}
				}
			}
			StrbuilderCat(&vmf, STR("}\n"));
		}
	}
	StrbuilderCat(&vmf, StrFromSize(g_vmfEnd, sizeof(g_vmfEnd) - 1));
	
	if (WriteEntireFile(outputPath, vmf.data, (u32)vmf.length))
	{
		PrintString("VMF export finished!\n");
	}
	ArenaEndTemp(arenaTemp);
	return result;
}
