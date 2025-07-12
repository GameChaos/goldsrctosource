
#include "wad3.h"

static_function Wad3 Wad3FromBuffer(Arena *arena, u8 *data, u64 bytes, char *path)
{
	Wad3 result = {};
	if (data && bytes > sizeof(Wad3Header))
	{
		Wad3Header *header = (Wad3Header *)data;
		
		if (header->magic == WAD3_MAGIC)
		{
			if (header->firstDirEntryOffset + header->dirEntries * sizeof(Wad3DirEntry) <= bytes)
			{
				i64 originalArenaPos = arena->allocPos;
				result.entryCount = header->dirEntries;
				result.entries = ArenaAlloc(arena, result.entryCount * sizeof(*result.entries));
				result.textures = ArenaAlloc(arena, result.entryCount * sizeof(*result.textures));
				for (u32 i = 0; i < header->dirEntries; i++)
				{
					Mem_Copy(&data[header->firstDirEntryOffset + i * sizeof(Wad3DirEntry)], &result.entries[i], sizeof(result.entries[i]));
					if (result.entries[i].type != WAD3_IMAGE_TYPE_TEXTURE)
					{
						continue;
					}
					if (result.entries[i].offset + sizeof(Wad3TextureHeader) < bytes)
					{
						result.textures[result.textureCount++] = (Wad3TextureHeader *)&data[result.entries[i].offset];
					}
					else
					{
						Error("Wad3 texture offset is out of bounds! %s\n", path ? path : "");
						ArenaResetTo(arena, originalArenaPos);
						result = (Wad3){};
						break;
					}
				}
				if (result.textureCount == 0)
				{
					// NOTE(GameChaos): don't care about wads with no textures
					ArenaResetTo(arena, originalArenaPos);
					result = (Wad3){};
				}
				else
				{
					result.valid = true;
					result.fileData = data;
				}
			}
			else
			{
				Error("Wad3 first directory entry is out of bounds! %s\n", path ? path : "");
			}
		}
		else
		{
			Error("Wad3 magic number is invalid! %s\n", path ? path : "");
		}
	}
	return result;
}

static_function Wad3 Wad3FromFile(Arena *arena, char *path)
{
	ReadFileResult file = ReadEntireFile(arena, path);
	Wad3 result = {};
	if (file.contents && file.size)
	{
		result = Wad3FromBuffer(arena, (u8 *)file.contents, file.size, path);
	}
	return result;
}

// NOTE(GameChaos): returns textureIndex + 1 if succeeded, 0 if not
static_function u32 Wad3FindTexture(Wad3 wad, char *name, i32 nameStrlen)
{
	u32 result = 0;
	if (name && wad.valid && nameStrlen)
	{
		for (u32 i = 0; i < wad.textureCount; i++)
		{
			Wad3TextureHeader *textureHeader = wad.textures[i];
			// TODO: does this need to be case sensitive?
			if (StringEquals(name, textureHeader->name, false))
			{
				result = i + 1;
				break;
			}
		}
	}
	return result;
}

static_function FindTextureResult FindTextureInWads(Wad3 *wads, i32 wadCount, char *name)
{
	i32 nameStrlen = (i32)StringLength(name);
	FindTextureResult result = {};
	if (nameStrlen)
	{
		for (i32 i = 0; i < wadCount; i++)
		{
			result.textureIndex = Wad3FindTexture(wads[i], name, nameStrlen);
			if (result.textureIndex)
			{
				result.found = true;
				result.wadIndex = i;
				// NOTE(GameChaos): Wad3FindTexture returns textureindex that's off by 1 (for falsiness :) )
				result.textureIndex--;
				break;
			}
		}
	}
	return result;
}
