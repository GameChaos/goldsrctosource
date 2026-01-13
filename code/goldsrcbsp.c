
#include "goldsrcbsp.h"

static_global const char* g_gsrcLumpNames[] = {
	"LUMP_ENTITIES",
	"LUMP_PLANES",
	"LUMP_TEXTURES",
	"LUMP_VERTICES",
	"LUMP_VISIBILITY",
	"LUMP_NODES",
	"LUMP_TEXINFO",
	"LUMP_FACES",
	"LUMP_LIGHTING",
	"LUMP_CLIPNODES",
	"LUMP_LEAVES",
	"LUMP_MARKSURFACES",
	"LUMP_EDGES",
	"LUMP_SURFEDGES",
	"LUMP_MODELS"
};

static_function bool GsrcImportBsp(Arena *arena, GsrcMapData *mapData)
{
	bool result = false;
	ASSERT(arena);
	ASSERT(arena->data);
	ASSERT(arena->bytes);
	ASSERT(mapData);
	if (mapData->fileData == NULL || mapData->fileDataSize <= 0)
	{
		ASSERT(0);
		return result;
	}
	
	mapData->header = (GsrcHeader *)mapData->fileData;
	
	if (mapData->header->version != 30)
	{
		// TODO: add a command line switch to try and convert anyway even if version doesn't match?
		Error("GoldSrc bsp version %i is not supported. Only version 30 is supported.\n", mapData->header->version);
		ASSERT(0);
		return result;
	}
	
	// NOTE(GameChaos): validate lumps and lengths
	Print("GoldSrc BSP lumps:\n");
	for (int i = 0; i < GSRC_HEADER_LUMPS; i++)
	{
		Print("%s %i\n", g_gsrcLumpNames[i], mapData->header->lump[i].offset);
		
		GsrcLump lump = mapData->header->lump[i];
		if (lump.offset + lump.length > (i64)mapData->fileDataSize)
		{
			Error("%s is out of bounds! Lump offset: %i, length: %i. BSP file size: %lli\n", g_gsrcLumpNames[i], lump.offset, lump.length, mapData->fileDataSize);
			return result;
		}
	}
	
	// LUMP_ENTITIES
	mapData->lumpEntities = StrFromSize((char *)mapData->fileData + mapData->header->lump[GSRC_LUMP_ENTITIES].offset, mapData->header->lump[GSRC_LUMP_ENTITIES].length);
	
	// LUMP_PLANES
	mapData->lumpPlanes = (GsrcPlane *)((u8 *)mapData->fileData + mapData->header->lump[GSRC_LUMP_PLANES].offset);
	mapData->planeCount = mapData->header->lump[GSRC_LUMP_PLANES].length / sizeof(GsrcPlane);
	
	// LUMP_TEXTURES
	mapData->textureLumpSize = mapData->header->lump[GSRC_LUMP_TEXTURES].length;
	mapData->lumpTextures.mipTextureCount = *(u32 *)((u8 *)mapData->fileData + mapData->header->lump[GSRC_LUMP_TEXTURES].offset);
	mapData->lumpTextureMemory = (u8 *)mapData->fileData + mapData->header->lump[GSRC_LUMP_TEXTURES].offset;
	
	// offsets. this is located 4 bytes after the start of the textures lump.
	mapData->lumpTextures.mipTextureOffsets = ((i32 *)mapData->lumpTextureMemory) + 1;
	
	mapData->lumpTextures.mipTextures = ArenaAlloc(arena, mapData->lumpTextures.mipTextureCount * sizeof(Wad3TextureHeader *));
	
	ASSERT(mapData->lumpTextures.mipTextures);
	if (mapData->lumpTextures.mipTextures == NULL)
	{
		Warning("mapData->lumpTextures.mipTextures is NULL.");
	}
	else
	{
		for (u32 i = 0;
			 i < mapData->lumpTextures.mipTextureCount;
			 i++)
		{
			if (mapData->lumpTextures.mipTextureOffsets[i] <= 0)
			{
				Error("Invalid mip texture offset %i for miptexture #%i\n",
					  mapData->lumpTextures.mipTextureOffsets[i], i);
				return result;
			}
			mapData->lumpTextures.mipTextures[i] = (Wad3TextureHeader *)((u8 *)mapData->lumpTextureMemory + mapData->lumpTextures.mipTextureOffsets[i]);
		}
	}
	
	// LUMP_VERTICES
	mapData->lumpVertices = (v3 *)((u8 *)mapData->fileData + mapData->header->lump[GSRC_LUMP_VERTICES].offset);
	mapData->vertexCount = mapData->header->lump[GSRC_LUMP_VERTICES].length / sizeof(v3);
	
	// LUMP_VISIBILITY
	mapData->lumpVIS = (u8 *)mapData->fileData + mapData->header->lump[GSRC_LUMP_VISIBILITY].offset;
	mapData->visLength = mapData->header->lump[GSRC_LUMP_VISIBILITY].length;
	
	// LUMP_NODES
	mapData->lumpNodes = (GsrcNode *)((u8 *)mapData->fileData + mapData->header->lump[GSRC_LUMP_NODES].offset);
	mapData->nodeCount = mapData->header->lump[GSRC_LUMP_NODES].length / sizeof(GsrcNode);
	
	// LUMP_TEXINFO
	mapData->lumpTexinfo = (GsrcTexinfo *)((u8 *)mapData->fileData + mapData->header->lump[GSRC_LUMP_TEXINFO].offset);
	mapData->texinfoCount = mapData->header->lump[GSRC_LUMP_TEXINFO].length / sizeof(GsrcTexinfo);
	
	// LUMP_FACES
	mapData->lumpFaces = (GsrcFace *)((u8 *)mapData->fileData + mapData->header->lump[GSRC_LUMP_FACES].offset);
	mapData->faceCount = mapData->header->lump[GSRC_LUMP_FACES].length / sizeof(GsrcFace);
	
	// LUMP_LIGHTING
	mapData->lumpLighting = (u8 *)mapData->fileData + mapData->header->lump[GSRC_LUMP_LIGHTING].offset;
	mapData->lightingLength = mapData->header->lump[GSRC_LUMP_LIGHTING].length;
	
#if defined(DEBUG_PRINT_GOLDSRC_IMPORT)
	for (int i = 0; i < mapData->lightingLength; i++)
	{
		Print("lighting: %i\n", mapData->lumpLighting[i]);
	}
#endif
	
	// LUMP_CLIPNODES
	mapData->lumpClipnodes = (GsrcClipnode *)((u8 *)mapData->fileData + mapData->header->lump[GSRC_LUMP_CLIPNODES].offset);
	mapData->clipnodeCount = mapData->header->lump[GSRC_LUMP_CLIPNODES].length / sizeof(GsrcClipnode);
	
#if defined(DEBUG_PRINT_GOLDSRC_IMPORT)
	for (int i = 0; i < mapData->clipnodeCount; i++)
	{
		Print("plane: %i\nchildren: %i %i\n",
			  mapData->lumpClipnodes[i].plane,
			  mapData->lumpClipnodes[i].children[0],
			  mapData->lumpClipnodes[i].children[1]
			  );
	}
#endif
	
	// LUMP_LEAVES
	mapData->lumpLeaves = (GsrcLeaf *)((u8 *)mapData->fileData + mapData->header->lump[GSRC_LUMP_LEAVES].offset);
	mapData->leafCount = mapData->header->lump[GSRC_LUMP_LEAVES].length / sizeof(GsrcLeaf);
	
#if defined(DEBUG_PRINT_GOLDSRC_IMPORT)
	for (int i = 0; i < mapData->leafCount; i++)
	{
		Print("contents: %i\nvisOffset: %i\nmins: %i %i %i\nmaxs: %i %i %i\nfirstMarkSurface: %i\nmarkSurfaces: %i\nambientLevels: %i %i %i %i\n",
			  mapData->lumpLeaves[i].contents,
			  mapData->lumpLeaves[i].visOffset,
			  mapData->lumpLeaves[i].mins[0],
			  mapData->lumpLeaves[i].mins[1],
			  mapData->lumpLeaves[i].mins[2],
			  mapData->lumpLeaves[i].maxs[0],
			  mapData->lumpLeaves[i].maxs[1],
			  mapData->lumpLeaves[i].maxs[2],
			  mapData->lumpLeaves[i].firstMarkSurface,
			  mapData->lumpLeaves[i].markSurfaces,
			  mapData->lumpLeaves[i].ambientLevels[0],
			  mapData->lumpLeaves[i].ambientLevels[1],
			  mapData->lumpLeaves[i].ambientLevels[2],
			  mapData->lumpLeaves[i].ambientLevels[3]
			  );
	}
#endif
	
	// LUMP_MARKSURFACES
	mapData->lumpMarksurfaces = (u16 *)((u8 *)mapData->fileData + mapData->header->lump[GSRC_LUMP_MARKSURFACES].offset);
	mapData->marksurfaceCount = mapData->header->lump[GSRC_LUMP_MARKSURFACES].length / sizeof(u16);
	
#if defined(DEBUG_PRINT_GOLDSRC_IMPORT)
	for (int i = 0; i < mapData->marksurfaceCount; i++)
	{
		Print("marksurface: %i\n", mapData->lumpMarksurfaces[i]);
	}
#endif
	
	// LUMP_EDGES
	mapData->lumpEdges = (GsrcEdge *)((u8 *)mapData->fileData + mapData->header->lump[GSRC_LUMP_EDGES].offset);
	mapData->edgeCount = mapData->header->lump[GSRC_LUMP_EDGES].length / sizeof(GsrcEdge);
	
#if defined(DEBUG_PRINT_GOLDSRC_IMPORT)
	for (int i = 0; i < mapData->edgeCount; i++)
	{
		Print("edges: %i %i\n", mapData->lumpEdges[i].vertex[0], mapData->lumpEdges[i].vertex[1]);
	}
#endif
	
	// LUMP_SURFEDGES
	mapData->lumpSurfEdges = (i32 *)((u8 *)mapData->fileData + mapData->header->lump[GSRC_LUMP_SURFEDGES].offset);
	mapData->surfEdgeCount = mapData->header->lump[GSRC_LUMP_SURFEDGES].length / sizeof(i32);
	
#if defined(DEBUG_PRINT_GOLDSRC_IMPORT)
	for (int i = 0; i < mapData->surfedgeCount; i++)
	{
		Print("surfedges: %i\n", mapData->surfEdges[i]);
	}
#endif
	
	// LUMP_MODELS
	mapData->lumpModels = (GsrcModel *)((u8 *)mapData->fileData + mapData->header->lump[GSRC_LUMP_MODELS].offset);
	mapData->modelCount = mapData->header->lump[GSRC_LUMP_MODELS].length / sizeof(GsrcModel);
	
#if defined(DEBUG_PRINT_GOLDSRC_IMPORT)
	for (int i = 0; i < mapData->modelCount; i++)
	{
		Print("mins: %f %f %f\nmaxs: %f %f %f\norigin: %f %f %f\nheadnodes: %i %i %i %i\nvisLeafs: %i\nfirstFaces: %i\nfaces: %i\n",
			  mapData->lumpModels[i].mins[0],
			  mapData->lumpModels[i].mins[1],
			  mapData->lumpModels[i].mins[2],
			  mapData->lumpModels[i].maxs[0],
			  mapData->lumpModels[i].maxs[1],
			  mapData->lumpModels[i].maxs[2],
			  mapData->lumpModels[i].origin.x,
			  mapData->lumpModels[i].origin.y,
			  mapData->lumpModels[i].origin.z,
			  mapData->lumpModels[i].headnodes[0],
			  mapData->lumpModels[i].headnodes[1],
			  mapData->lumpModels[i].headnodes[2],
			  mapData->lumpModels[i].headnodes[3],
			  mapData->lumpModels[i].visLeafs,
			  mapData->lumpModels[i].firstFace,
			  mapData->lumpModels[i].faces
			  );
	}
#endif
	
	return true;
}

static_function bool GsrcExportBsp(Arena *tempArena, char *filename, GsrcMapData *mapData)
{
	// TODO: check if filename is valid first
	// TODO: make this less jank?
	ASSERT(filename);
	ASSERT(mapData);
	
	ArenaTemp arenaTemp = ArenaBeginTemp(tempArena);
	FileWritingBuffer buffer = BufferCreate(tempArena, GIGABYTES(1));
	ASSERT(buffer.memory);
	ASSERT(buffer.size > 0);
	
	GsrcHeader *fileHeader = (GsrcHeader *)buffer.memory;
	
	// write header
	BufferPushData(&buffer, mapData->header, sizeof(GsrcHeader), true);
	
	// TODO: maybe make a big for loop for these.
	// TODO: maybe even use memcpy or something...
	
	//
	// LUMP_PLANES
	//
	fileHeader->lump[GSRC_LUMP_PLANES].offset = (u32)BufferGetSize(buffer);
	for (i32 i = 0; i < mapData->planeCount; i++)
	{
		BufferPushData(&buffer, (mapData->lumpPlanes + i), sizeof(GsrcPlane), true);
	}
	fileHeader->lump[GSRC_LUMP_PLANES].length = (u32)BufferGetSize(buffer) - fileHeader->lump[GSRC_LUMP_PLANES].offset;
	
	//
	// LUMP_LEAVES
	//
	fileHeader->lump[GSRC_LUMP_LEAVES].offset = (u32)BufferGetSize(buffer);
	for (i32 i = 0; i < mapData->leafCount; i++)
	{
		BufferPushData(&buffer, (mapData->lumpLeaves + i), sizeof(GsrcLeaf), true);
	}
	fileHeader->lump[GSRC_LUMP_LEAVES].length = (u32)BufferGetSize(buffer) - fileHeader->lump[GSRC_LUMP_LEAVES].offset;
	
	//
	// LUMP_VERTICES
	//
	fileHeader->lump[GSRC_LUMP_VERTICES].offset = (u32)BufferGetSize(buffer);
	for (i32 i = 0; i < mapData->vertexCount; i++)
	{
		BufferPushData(&buffer, (mapData->lumpVertices + i), sizeof(v3), true);
	}
	fileHeader->lump[GSRC_LUMP_VERTICES].length = (u32)BufferGetSize(buffer) - fileHeader->lump[GSRC_LUMP_VERTICES].offset;
	
	//
	// LUMP_NODES
	//
	fileHeader->lump[GSRC_LUMP_NODES].offset = (u32)BufferGetSize(buffer);
	for (i32 i = 0; i < mapData->nodeCount; i++)
	{
		BufferPushData(&buffer, (mapData->lumpNodes + i), sizeof(GsrcNode), true);
	}
	fileHeader->lump[GSRC_LUMP_NODES].length = (u32)BufferGetSize(buffer) - fileHeader->lump[GSRC_LUMP_NODES].offset;
	
	//
	// LUMP_TEXINFO
	//
	fileHeader->lump[GSRC_LUMP_TEXINFO].offset = (u32)BufferGetSize(buffer);
	for (i32 i = 0; i < mapData->texinfoCount; i++)
	{
		BufferPushData(&buffer, (mapData->lumpTexinfo + i), sizeof(GsrcTexinfo), true);
	}
	fileHeader->lump[GSRC_LUMP_TEXINFO].length = (u32)BufferGetSize(buffer) - fileHeader->lump[GSRC_LUMP_TEXINFO].offset;
	
	//
	// LUMP_FACES
	//
	fileHeader->lump[GSRC_LUMP_FACES].offset = (u32)BufferGetSize(buffer);
	for (i32 i = 0; i < mapData->faceCount; i++)
	{
		BufferPushData(&buffer, (mapData->lumpFaces + i), sizeof(GsrcFace), true);
	}
	fileHeader->lump[GSRC_LUMP_FACES].length = (u32)BufferGetSize(buffer) - fileHeader->lump[GSRC_LUMP_FACES].offset;
	
	//
	// LUMP_CLIPNODES
	//
	fileHeader->lump[GSRC_LUMP_CLIPNODES].offset = (u32)BufferGetSize(buffer);
	for (i32 i = 0; i < mapData->clipnodeCount; i++)
	{
		BufferPushData(&buffer, (mapData->lumpClipnodes + i), sizeof(GsrcClipnode), true);
	}
	fileHeader->lump[GSRC_LUMP_CLIPNODES].length = (u32)BufferGetSize(buffer) - fileHeader->lump[GSRC_LUMP_CLIPNODES].offset;
	
	//
	// LUMP_MARKSURFACES
	//
	fileHeader->lump[GSRC_LUMP_MARKSURFACES].offset = (u32)BufferGetSize(buffer);
	for (i32 i = 0; i < mapData->marksurfaceCount; i++)
	{
		BufferPushData(&buffer, (mapData->lumpMarksurfaces + i), sizeof(u16), true);
	}
	fileHeader->lump[GSRC_LUMP_MARKSURFACES].length = (u32)BufferGetSize(buffer) - fileHeader->lump[GSRC_LUMP_MARKSURFACES].offset;
	
	//
	// LUMP_SURFEDGES
	//
	fileHeader->lump[GSRC_LUMP_SURFEDGES].offset = (u32)BufferGetSize(buffer);
	for (i32 i = 0; i < mapData->surfEdgeCount; i++)
	{
		BufferPushData(&buffer, (mapData->lumpSurfEdges + i), sizeof(i32), true);
	}
	fileHeader->lump[GSRC_LUMP_SURFEDGES].length = (u32)BufferGetSize(buffer) - fileHeader->lump[GSRC_LUMP_SURFEDGES].offset;
	
	//
	// LUMP_EDGES
	//
	fileHeader->lump[GSRC_LUMP_EDGES].offset = (u32)BufferGetSize(buffer);
	for (i32 i = 0; i < mapData->edgeCount; i++)
	{
		BufferPushData(&buffer, (mapData->lumpEdges + i), sizeof(GsrcEdge), true);
	}
	fileHeader->lump[GSRC_LUMP_EDGES].length = (u32)BufferGetSize(buffer) - fileHeader->lump[GSRC_LUMP_EDGES].offset;
	
	//
	// LUMP_MODELS
	//
	fileHeader->lump[GSRC_LUMP_MODELS].offset = (u32)BufferGetSize(buffer);
	for (i32 i = 0; i < mapData->modelCount; i++)
	{
		BufferPushData(&buffer, (mapData->lumpModels + i), sizeof(GsrcModel), true);
	}
	fileHeader->lump[GSRC_LUMP_MODELS].length = (u32)BufferGetSize(buffer) - fileHeader->lump[GSRC_LUMP_MODELS].offset;
	//
	// LUMP_LIGHTING
	//
	fileHeader->lump[GSRC_LUMP_LIGHTING].offset = (u32)BufferGetSize(buffer);
	for (i32 i = 0; i < mapData->lightingLength; i++)
	{
		BufferPushData(&buffer, (mapData->lumpLighting + i), sizeof(u8), true);
	}
	fileHeader->lump[GSRC_LUMP_LIGHTING].length = (u32)BufferGetSize(buffer) - fileHeader->lump[GSRC_LUMP_LIGHTING].offset;
	//
	// LUMP_VISIBILITY
	//
	fileHeader->lump[GSRC_LUMP_VISIBILITY].offset = (u32)BufferGetSize(buffer);
	for (i32 i = 0; i < mapData->visLength; i++)
	{
		BufferPushData(&buffer, (mapData->lumpVIS + i), sizeof(u8), true);
	}
	fileHeader->lump[GSRC_LUMP_VISIBILITY].length = (u32)BufferGetSize(buffer) - fileHeader->lump[GSRC_LUMP_VISIBILITY].offset;
	
	//
	// LUMP_ENTITIES
	//
	fileHeader->lump[GSRC_LUMP_ENTITIES].offset = (u32)BufferGetSize(buffer);
	const char *ch = mapData->lumpEntities.data;
	
	do
	{
		buffer.memory[buffer.usedBytes++] = *ch;
		++ch;
	} while (*(ch - 1) != 0);
	fileHeader->lump[GSRC_LUMP_ENTITIES].length = (u32)BufferGetSize(buffer) - fileHeader->lump[GSRC_LUMP_ENTITIES].offset;
	
	//
	// LUMP_TEXTURES
	//
	fileHeader->lump[GSRC_LUMP_TEXTURES].offset = (u32)BufferGetSize(buffer);
	for (i32 i = 0; i < mapData->textureLumpSize; i++)
	{
		BufferPushData(&buffer, ((u8 *)mapData->lumpTextureMemory + i), sizeof(u8), true);
	}
	fileHeader->lump[GSRC_LUMP_TEXTURES].length = (u32)BufferGetSize(buffer) - fileHeader->lump[GSRC_LUMP_TEXTURES].offset;
	
	//
	//
	//
	
	size_t writtenDataSize = buffer.usedBytes;
	ASSERT(writtenDataSize > 0);
	
	WriteEntireFile(filename, buffer.memory, writtenDataSize);
	ArenaEndTemp(arenaTemp);
	
	return true;
}
