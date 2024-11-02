/* date = October 12th 2020 6:40 pm */

#ifndef PLATFORM_H
#define PLATFORM_H

typedef struct
{
	void *contents;
	s64 size;
} ReadFileResult;

typedef struct
{
	b32 isFolder;
	char path[260];
} FileInfo;

internal ReadFileResult ReadEntireFile(Arena *arena, const char *filePath);
internal b32 WriteEntireFile(const char *filename, const void *memory, s64 bytes);
internal void AppendToPath(char *path, s64 pathLength, const char *file);
internal s32 GetDirectoryFiles(char *path, FileInfo *out, s32 maxFileCount, const char *fileExtFilter);

internal void *Plat_MemReserve(s64 bytes);
internal void Plat_MemCommit(void *address, s64 bytes);
internal void Plat_MemDecommit(void *address, s64 bytes);
internal void Plat_MemFree(void *address, s64 bytes);
internal s64 Plat_GetPageSize(void);

internal void Mem_Copy(const void *source, void *destination, size_t bytes);
internal b32 Mem_Compare(const void *a, const void *b, s64 bytes);
internal void Plat_MemSetToZero(void *destination, s64 bytes);

internal size_t StringLength(const char *string);
internal b32 StringEquals(const char *a, const char *b, b32 caseSensitive);
internal void Plat_WriteToStdout(const char *str, s64 len);

#endif //PLATFORM_H
