
#include "wad3.h"

internal Wad3 Wad3FromBuffer(Arena *arena, u8 *data, u64 bytes, char *path)
{
	Wad3 result = {};
	if (data && bytes > sizeof(Wad3Header))
	{
		Wad3Header *header = (Wad3Header *)data;
		
		if (header->magic == WAD3_MAGIC)
		{
			if (header->firstDirEntryOffset + header->dirEntries * sizeof(Wad3DirEntry) <= bytes)
			{
				s64 originalArenaPos = arena->allocPos;
				result.entryCount = header->dirEntries;
				result.entries = (Wad3DirEntry **)ArenaAlloc(arena, result.entryCount * sizeof(*result.entries));
				result.textures = (Wad3TextureHeader **)ArenaAlloc(arena, result.entryCount * sizeof(*result.textures));
				for (u32 i = 0; i < header->dirEntries; i++)
				{
					Wad3DirEntry *entry = (Wad3DirEntry *)&data[header->firstDirEntryOffset + i * sizeof(*entry)];
					result.entries[i] = entry;
					if (entry->type != WAD3_IMAGE_TYPE_TEXTURE)
					{
						continue;
					}
					if (entry->offset + sizeof(Wad3TextureHeader) < bytes)
					{
						result.textures[result.textureCount++] = (Wad3TextureHeader *)&data[entry->offset];
					}
					else
					{
						LOG_ERROR("Wad3 texture offset is out of bounds! %s\n", path ? path : "");
						ArenaResetTo(arena, originalArenaPos);
						result = {};
						ASSERT(0);
						break;
					}
				}
				if (result.textureCount == 0)
				{
					// NOTE(GameChaos): don't care about wads with no textures
					ArenaResetTo(arena, originalArenaPos);
					result = {};
				}
				else
				{
					result.valid = true;
					result.fileData = data;
				}
			}
			else
			{
				ASSERT(0);
				LOG_ERROR("Wad3 first directory entry is out of bounds! %s\n", path ? path : "");
			}
		}
		else
		{
			ASSERT(0);
			LOG_ERROR("Wad3 magic number is invalid! %s\n", path ? path : "");
		}
	}
	return result;
}

internal Wad3 Wad3FromFile(Arena *arena, char *path)
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
internal u32 Wad3FindTexture(Wad3 wad, char *name, s32 nameStrlen)
{
	u32 result = 0;
	if (name && wad.valid && nameStrlen)
	{
		for (u32 i = 0; i < wad.textureCount; i++)
		{
			Wad3TextureHeader *textureHeader = wad.textures[i];
			// TODO: does this need to be case sensitive?
			if (StringEquals(name, textureHeader->name))
			{
				result = i + 1;
				break;
			}
		}
	}
	return result;
}

internal FindTextureResult FindTextureInWads(Wad3 *wads, s32 wadCount, char *name)
{
	s32 nameStrlen = (s32)StringLength(name);
	FindTextureResult result = {};
	if (nameStrlen)
	{
		for (s32 i = 0; i < wadCount; i++)
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
