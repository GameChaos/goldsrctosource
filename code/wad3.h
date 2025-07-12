/* date = November 1st 2022 6:11 pm */

#ifndef WAD3_H
#define WAD3_H

#define WAD3_MAGIC FOURCC_TO_INT('W', 'A', 'D', '3')

enum
{
	WAD3_IMAGE_TYPE_TEXTURE = 0x43,
};

typedef struct
{
	u32 magic; // WAD3_MAGIC, "WAD3"
	u32 dirEntries;
	u32 firstDirEntryOffset;
} Wad3Header;

typedef struct
{
	u32 offset;
	u32 size;
	u32 uncompressedSize;
	u8 type;
	u8 compressed;
	u16 padding_;
	char name[16];
} Wad3DirEntry;

typedef struct
{
	char name[16];
	u32 width;
	u32 height;
	u32 offsets[4];
} Wad3TextureHeader;

typedef struct
{
	bool valid;
	u32 entryCount;
	Wad3DirEntry *entries;
	u32 textureCount;
	Wad3TextureHeader **textures;
	u8 *fileData;
} Wad3;

typedef struct
{
	bool found;
	u32 wadIndex;
	u32 textureIndex;
} FindTextureResult;

static_function Wad3 Wad3FromBuffer(Arena *arena, u8 *data, u64 bytes, char *path/* = NULL*/);
static_function Wad3 Wad3FromFile(Arena *arena, char *path);

#endif //WAD3_H
