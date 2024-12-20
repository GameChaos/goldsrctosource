
static_global char g_skySides[SKY_SIDE_COUNT][3] = {
	"up",
	"dn",
	"lf",
	"rt",
	"bk",
	"ft",
};

static_function i32 LoadWads(Arena *arena, char *modPath, char *valvePath, Wad3 out[MAX_WADS])
{
	FileInfo wadFileInfo[MAX_WADS] = {};
	
	i32 result = 0;
	i32 wadCount = GetDirectoryFiles(modPath, wadFileInfo, ARRAYCOUNT(wadFileInfo), ".wad");
	for (i32 i = 0; i < wadCount; i++)
	{
		ASSERT(result < MAX_WADS);
		out[result++] = Wad3FromFile(arena, wadFileInfo[i].path);
	}
	
	i32 wadCount2 = GetDirectoryFiles(valvePath, wadFileInfo, ARRAYCOUNT(wadFileInfo), ".wad");
	for (i32 i = 0; i < wadCount2; i++)
	{
		ASSERT(result < MAX_WADS);
		out[result++] = Wad3FromFile(arena, wadFileInfo[i].path);
	}
	return result;
}

// TODO: check if correct
static_function v2i VtfGetMipDimensions(v2i textureSize, i32 mipLevel)
{
	v2i result = {0};
	result.x = textureSize.x >> mipLevel;
	result.y = textureSize.y >> mipLevel;
	return result;
}

// TODO: check if correct
static_function i32 VtfGetMipCount(v2i textureSize)
{
	i32 result = 0;
	
	i32 width = textureSize.x;
	i32 height = textureSize.y;
	while (width >= 1 && height >= 1)
	{
		width = GCM_MAX(width, 1);
		height = GCM_MAX(height, 1);
		result++;
		width >>= 1;
		height >>= 1;
	}
	
	return result;
}

static_function VtfHeader VtfDefaultHeader(void)
{
	VtfHeader result = {};
	
	result.signature = VTF_SIGNATURE;
	result.version[0] = 7;
	result.version[1] = 1;
	result.headerSize = sizeof(result);
	result.frames = 1;
	result.reflectivity = (v3){0.5f, 0.5f, 0.5f};
	// no low res (dxt1) image.
	result.lowResImageFormat = U32_MAX;
	result.lowResImageWidth = 0;
	result.lowResImageHeight = 0;
	
	return result;
}

static_function void GsrcMipTextureToVtf(Arena *tempArena, FileWritingBuffer *out, GsrcMipTexture mipTexture, u8 *mipTextureData)
{
	bool result = false;
	
	bool transparent = mipTexture.name[0] == '{';
	
	VtfHeader vtfHeader = VtfDefaultHeader();
	vtfHeader.frames = 1;
	vtfHeader.width = (u16)mipTexture.width;
	vtfHeader.height = (u16)mipTexture.height;
	
	i32 pixels = (i32)mipTexture.width * (i32)mipTexture.height;
	u8 *palette = mipTextureData + 2 + mipTexture.offsets[0] + pixels + (pixels >> 2) + (pixels >> 4) + (pixels >> 6);
	
	if (transparent)
	{
		vtfHeader.highResImageFormat = VTF_FORMAT_BGRA8888;
		vtfHeader.flags |= TEXTUREFLAGS_EIGHTBITALPHA;
	}
	else
	{
		vtfHeader.highResImageFormat = VTF_FORMAT_BGR888;
	}
	
	v2i textureSize = {
		(i32)mipTexture.width,
		(i32)mipTexture.height
	};
	
	vtfHeader.mipmapCount = VtfGetMipCount(textureSize);
	BufferPushData(out, &vtfHeader, sizeof(vtfHeader), false);
	
	i32 channels = 3;
	if (transparent)
	{
		channels = 4;
	}
	// generate mipmaps
	// NOTE(GameChaos): mipmaps are stored smallest (1x1) to largest
	ArenaTemp arenaTmp = ArenaBeginTemp(tempArena);
	u8 *largestMip = ArenaAlloc(tempArena, (i64)mipTexture.width * mipTexture.height * channels);
	for (i32 pix = 0; pix < pixels; pix++)
	{
		i32 indexOffset = mipTexture.offsets[0] + pix;
		u8 rgb[] = {
			palette[mipTextureData[indexOffset] * 3 + 0],
			palette[mipTextureData[indexOffset] * 3 + 1],
			palette[mipTextureData[indexOffset] * 3 + 2],
		};
		if (transparent)
		{
			if (rgb[0] == 0 && rgb[1] == 0 && rgb[2] == 255)
			{
				largestMip[pix * channels + 3] = 0;
				rgb[2] = 0;
			}
			else
			{
				largestMip[pix * channels + 3] = 255;
			}
		}
		
		for (i32 i = 0; i < 3; i++)
		{
			// apply sRGB OETF. goldsrc rendering is weird....
			//largestMip[pix * channels + i] = (u8)(powf(rgb[2 - i] / 255.0f, 1.0f / 2.2f) * 255.0f);
			f32 f = rgb[2 - i] / 255.0f;
			f = powf(f, 1.0f / 2.2f);
			//f = sqrtf(f);
			largestMip[pix * channels + i] = (u8)(f * 255.0f);
		}
	}
	
	// TODO: mip 0 is correct reflectivity
	u8 *tempImgDataRgb888 = ArenaAlloc(tempArena, (i64)pixels * channels);
	for (i32 mip = vtfHeader.mipmapCount - 1;
		 mip >= 0;
		 mip--)
	{
		v2i mipSize = VtfGetMipDimensions(textureSize, mip);
		u32 mipPixels = mipSize.x * mipSize.y;
		
		i32 alphaChannel = transparent ? 3 : STBIR_ALPHA_CHANNEL_NONE;
		
		stbir_resize_uint8_srgb(largestMip, (i32)mipTexture.width, (i32)mipTexture.height, 0,
								tempImgDataRgb888, mipSize.x, mipSize.y, 0,
								channels, alphaChannel, 0);
		BufferPushData(out, tempImgDataRgb888, mipPixels * channels, false);
	}
	ArenaEndTemp(arenaTmp);
}

static_function aabb GsrcModelsToSrcModels(GsrcMapData *mapData, SrcModel *outModels, i32 *outModelCount, SrcPlane outBboxPlanes[6])
{
	aabb mapAabb = {};
	for (i32 i = 0; i < mapData->modelCount; i++)
	{
		SrcModel model = {};
		model.mins = mapData->lumpModels[i].mins;
		model.maxs = mapData->lumpModels[i].maxs;
		model.headnode = mapData->lumpModels[i].headnodes[0];
		model.firstFace = mapData->lumpModels[i].firstFace;
		model.numFaces = mapData->lumpModels[i].faces;
		if (i == 0)
		{
			mapAabb.mins = v3sub(model.mins, (v3){32, 32, 32});
			mapAabb.maxs = v3add(model.maxs, (v3){32, 32, 32});
		}
		outModels[(*outModelCount)++] = model;
	}
	
	// +x
	outBboxPlanes[0] = (SrcPlane){{1, 0, 0}, mapAabb.maxs.x, SRC_PLANE_X};
	// -x
	outBboxPlanes[1] = (SrcPlane){{-1, 0, 0}, -mapAabb.mins.x, SRC_PLANE_X};
	// +y
	outBboxPlanes[2] = (SrcPlane){{0, 1, 0}, mapAabb.maxs.y, SRC_PLANE_Y};
	// -y
	outBboxPlanes[3] = (SrcPlane){{0, -1, 0}, -mapAabb.mins.y, SRC_PLANE_Y};
	// +z
	outBboxPlanes[4] = (SrcPlane){{0, 0, 1}, mapAabb.maxs.z, SRC_PLANE_Z};
	// -z
	outBboxPlanes[5] = (SrcPlane){{0, 0, -1}, -mapAabb.mins.z, SRC_PLANE_Z};
	
	return mapAabb;
}

static_function EntList GsrcEntitiesToSrcEntities(Arena *arena, EntList gsrcEnts, bool *modelIsLadder)
{
	EntList srcEnts = {};
	srcEnts.ents = ArenaAlloc(arena, gsrcEnts.entCount * sizeof(*srcEnts.ents) * 2);
	ASSERT(srcEnts.ents);
	// TODO: rehaul this function into a system where entity conversion is defined in a config file.
	for (i32 i = 0; i < gsrcEnts.entCount; i++)
	{
		EntProperties *gsrcEnt = &gsrcEnts.ents[i];
		EntProperties *ent = &srcEnts.ents[srcEnts.entCount];
		*ent = (EntProperties){};
		// TODO: add hammerid to all entities?
		if (StrEquals(gsrcEnt->classname, STR("worldspawn"), false))
		{
			for (i32 prop = 0; prop < gsrcEnt->propertyCount; prop++)
			{
				if (StrEquals(gsrcEnt->properties[prop].key, STR("skyname"), false))
				{
					EntPushProp(ent, gsrcEnt->properties[prop].key, gsrcEnt->properties[prop].value);
				}
				else if (StrEquals(gsrcEnt->properties[prop].key, STR("mapversion"), false))
				{
					EntPushProp(ent, gsrcEnt->properties[prop].key, gsrcEnt->properties[prop].value);
				}
			}
			// TODO: correct mins & maxs
			EntPushProp(ent, STR("world_maxs"),     STR("16383 16383 16383"));
			EntPushProp(ent, STR("world_mins"),     STR("-16383 -16383 -16383"));
			EntPushProp(ent, STR("detailmaterial"), STR("detail/detailsprites"));
			EntPushProp(ent, STR("detailvbsp"),     STR("detail.vbsp"));
			EntPushProp(ent, STR("maxpropscreenwidth"), STR("-1"));
			//EntPushProp(ent, STR("skyname"),        STR("sky_dust"));
			//EntPushProp(ent, STR("mapversion"),     STR("1"));
			EntPushProp(ent, STR("hammerid"),       STR("1"));
			
			ent->classname = STR("worldspawn");
		}
		else if (StrEquals(gsrcEnt->classname, STR("light_environment"), false))
		{
			for (i32 prop = 0; prop < gsrcEnt->propertyCount; prop++)
			{
				if (StrEquals(gsrcEnt->properties[prop].key, STR("_diffuse_light"), false))
				{
					EntPushProp(ent, STR("_ambient"), gsrcEnt->properties[prop].value);
				}
				else
				{
					EntPushProp(ent, gsrcEnt->properties[prop].key, gsrcEnt->properties[prop].value);
				}
			}
			ent->classname = STR("light_environment");
		}
		else if (StrEquals(gsrcEnt->classname, STR("light"), false))
		{
			EntConvertOneToOne(gsrcEnt, ent);
			ent->classname = STR("light");
		}
		else if (StrEquals(gsrcEnt->classname, STR("light_spot"), false))
		{
			for (i32 prop = 0; prop < gsrcEnt->propertyCount; prop++)
			{
				if (StrEquals(gsrcEnt->properties[prop].key, STR("_cone"), false))
				{
					EntPushProp(ent, STR("_inner_cone"), gsrcEnt->properties[prop].value);
				}
				if (StrEquals(gsrcEnt->properties[prop].key, STR("_cone2"), false))
				{
					EntPushProp(ent, STR("_cone"), gsrcEnt->properties[prop].value);
				}
				else
				{
					EntPushProp(ent, gsrcEnt->properties[prop].key, gsrcEnt->properties[prop].value);
				}
			}
			ent->classname = STR("light");
		}
		else if (StrEquals(gsrcEnt->classname, STR("info_player_start"), false))
		{
			// info_player_start can be directly converted with a few additions
			EntConvertOneToOne(gsrcEnt, ent);
			EntPushProp(ent, STR("enabled"), STR("1"));
			ent->classname = STR("info_player_counterterrorist");
		}
		else if (StrEquals(gsrcEnt->classname, STR("info_player_deathmatch"), false))
		{
			// lol, this is terrorist spawn in cs 1.6
			EntConvertOneToOne(gsrcEnt, ent);
			EntPushProp(ent, STR("enabled"), STR("1"));
			ent->classname = STR("info_player_terrorist");
		}
		else if (StrEquals(gsrcEnt->classname, STR("info_teleport_destination"), false))
		{
			// info_teleport_destination can be directly converted with no additions!
			EntConvertOneToOne(gsrcEnt, ent);
			ent->classname = STR("info_teleport_destination");
		}
		else if (StrEquals(gsrcEnt->classname, STR("info_target"), false))
		{
			// info_teleport_destination can be directly converted with no additions!
			EntConvertOneToOne(gsrcEnt, ent);
			ent->classname = STR("info_target");
		}
		else if (StrEquals(gsrcEnt->classname, STR("env_fog"), false))
		{
			f64 density = 0;
			for (i32 prop = 0; prop < gsrcEnt->propertyCount; prop++)
			{
				if (StrEquals(gsrcEnt->properties[prop].key, STR("density"), false))
				{
					const char *start = gsrcEnt->properties[prop].value.data;
					while (IsWhiteSpace(*start))
					{
						start++;
					}
					density = StrToF32(gsrcEnt->properties[prop].value);
					//const char *end = fast_double_parser::parse_number(start, &density);
				}
				else if (StrEquals(gsrcEnt->properties[prop].key, STR("rendercolor"), false))
				{
					EntPushProp(ent, STR("fogcolor"), gsrcEnt->properties[prop].value);
				}
			}
			if (density < 0.01f && density > 0)
			{
				EntPushProp(ent, STR("fogenable"), STR("1"));
				EntPushProp(ent, STR("fogstart"), STR("0"));
				str_builder fogend = StrbuilderCreate(arena, 32);
				StrbuilderFormat(&fogend, "%f", 1.5f / density);
				EntPushProp(ent, STR("fogend"), StrbuilderGetStr(fogend));
			}
			ent->classname = STR("env_fog_controller");
		}
#if 0
		else if (StrEquals(gsrcEnt->classname, STR("cycler_sprite"), false))
		{
			// NOTE(GameChaos): this entity is what is used for props in goldsrc
			// TODO: mdl conversion
			//EntConvertOneToOne(gsrcEnt, ent);
			ent->classname = STR("prop_dynamic");
		}
#endif
		else if (StrEquals(gsrcEnt->classname, STR("func_door"), false)
				 || StrEquals(gsrcEnt->classname, STR("func_door_rotating"), false)
				 || StrEquals(gsrcEnt->classname, STR("func_conveyor"), false)
				 || StrEquals(gsrcEnt->classname, STR("func_rotating"), false))
		{
			EntConvertCommonBrush(arena, gsrcEnt, ent);
			EntPushProp(ent, STR("spawnflags"), STR("1024")); // touch opens
			// NOTE(GameChaos): i'm tired of shitty cs 1.6 bhop blocks :)
			ent->classname = STR("func_brush");
			//ent->classname = STR("func_door");
		}
		else if (StrEquals(gsrcEnt->classname, STR("func_button"), false))
		{
			EntConvertCommonBrush(arena, gsrcEnt, ent);
			EntPushProp(ent, STR("spawnflags"), STR("1025"));
			ent->classname = STR("func_button");
		}
		else if (StrEquals(gsrcEnt->classname, STR("func_wall"), false)
				 || StrEquals(gsrcEnt->classname, STR("func_wall_toggle"), false))
		{
			EntConvertCommonBrush(arena, gsrcEnt, ent);
			EntPushProp(ent, STR("solidbsp"), STR("1"));
			ent->classname = STR("func_wall");
		}
		else if (StrEquals(gsrcEnt->classname, STR("func_illusionary"), false)
				 || StrEquals(gsrcEnt->classname, STR("func_train"), false))
		{
			EntConvertCommonBrush(arena, gsrcEnt, ent);
			ent->classname = STR("func_illusionary");
		}
		else if (StrEquals(gsrcEnt->classname, STR("func_water"), false))
		{
			EntConvertCommonBrush(arena, gsrcEnt, ent);
			ent->classname = STR("func_brush");
		}
		else if (StrEquals(gsrcEnt->classname, STR("func_ladder"), false))
		{
			ModelInfo info = EntConvertCommonBrush(arena, gsrcEnt, ent);
			if (modelIsLadder && info.model > 0 && info.model < SRC_MAX_MAP_MODELS)
			{
				modelIsLadder[info.model] = true;
			}
			EntPushProp(ent, STR("solidbsp"), STR("1"));
			ent->classname = STR("func_brush");
		}
		else if (StrEquals(gsrcEnt->classname, STR("func_breakable"), false))
		{
			EntConvertCommonBrush(arena, gsrcEnt, ent);
			ent->classname = STR("func_breakable");
		}
		else if (StrEquals(gsrcEnt->classname, STR("trigger_teleport"), false))
		{
			EntConvertCommonBrush(arena, gsrcEnt, ent);
			EntPushProp(ent, STR("spawnflags"), STR("4097"));
			ent->classname = STR("trigger_teleport");
		}
		else if (StrEquals(gsrcEnt->classname, STR("trigger_multiple"), false))
		{
			EntConvertCommonBrush(arena, gsrcEnt, ent);
			ent->classname = STR("trigger_multiple");
		}
		else if (StrEquals(gsrcEnt->classname, STR("trigger_once"), false))
		{
			EntConvertCommonBrush(arena, gsrcEnt, ent);
			ent->classname = STR("trigger_multiple");
		}
		else
		{
			// NOTE(GameChaos): for testing
			//EntConvertOneToOne(gsrcEnt, ent);
			//ent->classname = gsrcEnt->classname;
			Warning("Unknown entity \"%.*s\"\n", gsrcEnt->classname.length, gsrcEnt->classname.data);
		}
		if (ent->classname.data)
		{
			srcEnts.entCount++;
		}
	}
	
	return srcEnts;
}

typedef struct
{
	i32 parents[GSRC_MAX_MAP_NODES];
	TraverseBspTreeNode stack[GSRC_MAX_MAP_NODES];
	i32 stackLength;
} GsrcBspTreeIterator;

static_function GsrcBspTreeIterator *GsrcBspTreeIteratorBegin(Arena *arena, GsrcMapData *mapData, i32 model)
{
	GsrcBspTreeIterator *result = ArenaAlloc(arena, sizeof(*result));
	for (i32 i = 0; i < ARRAYCOUNT(result->parents); i++)
	{
		result->parents[i] = I32_MIN;
	}
	result->stackLength = 0;
	
	result->stack[result->stackLength++] = (TraverseBspTreeNode){
		I32_MIN, mapData->lumpModels[model].headnodes[0]
	};
	return result;
}

static_function bool GsrcBspTreeNext(GsrcBspTreeIterator *iter, GsrcMapData *mapData, i32 *outNode)
{
	bool result = false;
	
	while (iter->stackLength && !result)
	{
		TraverseBspTreeNode currentNode = iter->stack[--iter->stackLength];
		iter->parents[currentNode.index] = currentNode.parent;
		GsrcNode node = mapData->lumpNodes[currentNode.index];
		for (i32 child = 0; child < 2; child++)
		{
			if (child == 0)
			{
				iter->stack[iter->stackLength++] = (TraverseBspTreeNode){
					-currentNode.index - 1, (i32)node.children[child]
				};
			}
			else
			{
				iter->stack[iter->stackLength++] = (TraverseBspTreeNode){
					currentNode.index, (i32)node.children[child]
				};
			}
			i32 gsrcLeafIndex = -(i32)node.children[child] - 1;
			if (gsrcLeafIndex >= 0)
			{
				--iter->stackLength;
			}
		}
		if (outNode)
		{
			*outNode = currentNode.index;
		}
		result = true;
	}
	
	return result;
}

static_function SrcLeaf MakeSrcLeaf(GsrcMapData *mapData, GsrcLeaf gsrcLeaf, bool isLadder,
							 i16 *outCluster, u16 *outLeafFaces, u16 *outLeafFaceCount)
{
	SrcLeaf result = {};
	
	if (gsrcLeaf.contents != GSRC_CONTENTS_EMPTY)
	{
		// TODO: something something LUMP_LEAFWATERDATA something LUMP_LEAFMINDISTTOWATER something
		result.contents = GsrcContentsToSrcContents(gsrcLeaf.contents);
		if (isLadder)
		{
			result.contents = SRC_CONTENTS_SOLID | SRC_CONTENTS_LADDER;
		}
		
		result.cluster = -1;
		result.area = 0;
	}
	else
	{
		result.area = 0;
		result.cluster = (*outCluster)++;
	}
	result.flags = SRC_LEAF_FLAGS_SKY2D;
	for (i32 j = 0; j < 3; j++)
	{
		result.mins[j] = gsrcLeaf.mins[j];
		result.maxs[j] = gsrcLeaf.maxs[j];
	}
	
	result.firstLeafFace = (*outLeafFaceCount);
	// NOTE(GameChaos): loop through the faces and set their texinfo flags semi-properly.
	for (i32 markSurf = gsrcLeaf.firstMarkSurface;
		 markSurf < gsrcLeaf.firstMarkSurface + gsrcLeaf.markSurfaces;
		 markSurf++)
	{
		// NOTE: LUMP_MARKSURFACES == LUMP_LEAFFACES
		u16 faceInd = mapData->lumpMarksurfaces[markSurf];
		outLeafFaces[(*outLeafFaceCount)++] = faceInd;
	}
	result.leafFaces = (*outLeafFaceCount) - result.firstLeafFace;
	
	return result;
}

static_function i32 GetLeafClipPlanes(GsrcMapData *mapData, SrcPlane bboxPlanes[6], i32 *leafParents, i32 gsrcNodeIndex, i32 child, i32 maxPlanes, SrcPlane *outPlanes)
{
	i32 result = 0; // NOTE(GameChaos): plane count
	// NOTE(GameChaos): make a list of clip planes to cut with
	for (i32 parent1 = child == 0 ? -gsrcNodeIndex - 1 : gsrcNodeIndex;
		 parent1 != I32_MIN;
		 parent1 = leafParents[PARENT_TO_INDEX(parent1)])
	{
		i32 index1 = PARENT_TO_INDEX(parent1);
		GsrcNode node2 = mapData->lumpNodes[index1];
		SrcPlane plane;
		plane.distance = mapData->lumpPlanes[node2.plane].distance;
		plane.normal = mapData->lumpPlanes[node2.plane].normal;
		plane.type = mapData->lumpPlanes[node2.plane].type;
		if (parent1 < 0)
		{
			plane.normal = v3negate(plane.normal);
			plane.distance = -plane.distance;
		}
		
		ASSERT(result < maxPlanes);
		outPlanes[result++] = plane;
	}
	
	// NOTE(GameChaos): add map bounds planes
	for (i32 i = 0; i < 6; i++)
	{
		ASSERT(result < maxPlanes);
		outPlanes[result++] = bboxPlanes[i];
	}
	
	// NOTE(GameChaos): remove duplicate planes and planes that shouldn't intersect
	for (i32 planeInd = 0; planeInd < result; planeInd++)
	{
		SrcPlane plane = outPlanes[planeInd];
		for (i32 planeInd2 = 0; planeInd2 < result; planeInd2++)
		{
			if (planeInd == planeInd2)
			{
				continue;
			}
			if (VecNearlyEquals(outPlanes[planeInd2].normal, plane.normal, 0.005f))
			{
				// NOTE(GameChaos): remove plane
				for (i32 p = planeInd2; p < result - 1; p++)
				{
					outPlanes[p] = outPlanes[p + 1];
				}
				result--;
				planeInd2--;
			}
		}
	}
	
	return result;
}

static_function i32 MakeVmt(char *buffer, i32 bufferSize, char *textureName, bool transparent)
{
	i32 result = 0; // length of vmt file
	if (transparent)
	{
		// NOTE(GameChaos): $alphatestreference 0.3 mitigates dilation of transparent areas on smaller mips.
		result = Format(buffer, bufferSize, "LightmappedGeneric\r\n"
						"{\r\n"
						"\t$basetexture \"" CONVERTED_MATERIAL_FOLDER "%s\"\r\n"
						"\t$alphatest 1\r\n"
						"\t$alphatestreference 0.3\r\n"
						"}\r\n", textureName);
	}
	else
	{
		result = Format(buffer, bufferSize, "LightmappedGeneric\r\n"
						"{\r\n"
						"\t$basetexture \"" CONVERTED_MATERIAL_FOLDER "%s\"\r\n"
						"}\r\n", textureName);
	}
	return result;
}

static_function i32 MakeSkyTextureVmt(char *buffer, i32 bufferSize, char *texturePath)
{
	i32 result = 0; // length of vmt file
	
	result = Format(buffer, bufferSize, "UnlitGeneric\r\n"
					"{\r\n"
					"\t$basetexture \"%s\"\r\n"
					"\t$ignorez 1\r\n"
					"\t$nofog 1\r\n"
					"}\r\n", texturePath);
	
	return result;
}

static_function bool GsrcGetSkyTexture(Arena *tempArena,
							   FileWritingBuffer *out,
							   str skyname,
							   char *modPath,
							   char *valvePath,
							   SkySide side)
{
	bool result = false;
	
	char *basePaths[2] = {
		modPath,
		valvePath
	};
	
	char skyfacePath[512];
	char relativeSkyfacePath[128];
	Format(skyfacePath, sizeof(skyfacePath), "%s", modPath);
	Format(relativeSkyfacePath, sizeof(relativeSkyfacePath), "gfx/env/%.*s%s.tga",
		   (i32)skyname.length, skyname.data, g_skySides[side]);
	AppendToPath(skyfacePath, sizeof(skyfacePath), relativeSkyfacePath);
	
	v2i textureSize = {};
	u8 *texture = stbi_load(skyfacePath, &textureSize.x, &textureSize.y, NULL, 3);
	if (texture
		&& textureSize.x <= U16_MAX && textureSize.y <= U16_MAX
		&& textureSize.x > 0 && textureSize.y > 0)
	{
		VtfHeader vtfHeader = VtfDefaultHeader();
		vtfHeader.frames = 1;
		vtfHeader.width = (u16)textureSize.x;
		vtfHeader.height = (u16)textureSize.y;
		vtfHeader.highResImageFormat = VTF_FORMAT_RGB888;
		vtfHeader.mipmapCount = 1;
		
		vtfHeader.flags |= (TEXTUREFLAGS_CLAMPS
							| TEXTUREFLAGS_CLAMPT
							| TEXTUREFLAGS_NOMIP
							| TEXTUREFLAGS_NOLOD);
		
		i32 pixels = textureSize.x * textureSize.y;
		
		BufferPushData(out, &vtfHeader, sizeof(vtfHeader), false);
		
		// no mipmaps
		BufferPushData(out, texture, pixels * 3, false);
		
		result = true;
	}
	stbi_image_free(texture);
	
	return result;
}