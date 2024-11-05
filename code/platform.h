/* date = October 12th 2020 6:40 pm */

#ifndef PLATFORM_H
#define PLATFORM_H

typedef struct
{
	void *contents;
	i64 size;
} ReadFileResult;

typedef struct
{
	bool isFolder;
	char path[260];
} FileInfo;

static_function ReadFileResult ReadEntireFile(Arena *arena, const char *filePath);
static_function bool WriteEntireFile(const char *filename, const void *memory, i64 bytes);
static_function void AppendToPath(char *path, i64 pathLength, const char *file);
static_function i32 GetDirectoryFiles(const char *path, FileInfo *out, i32 maxFileCount, const char *fileExtFilter);

static_function void *Plat_MemReserve(i64 bytes);
static_function void Plat_MemCommit(void *address, i64 bytes);
static_function void Plat_MemDecommit(void *address, i64 bytes);
static_function void Plat_MemFree(void *address, i64 bytes);
static_function i64 Plat_GetPageSize(void);

static_function void Mem_Copy(const void *source, void *destination, size_t bytes);
static_function bool Mem_Compare(const void *a, const void *b, i64 bytes);
static_function void Plat_MemSetToZero(void *destination, i64 bytes);

static_function size_t StringLength(const char *string);
static_function bool StringEquals(const char *a, const char *b, bool caseSensitive);
static_function bool StringEqualsLen(const char *a, const char *b, i32 count, bool caseSensitive);
static_function void Plat_WriteToStdout(const char *str, i64 len);

#endif //PLATFORM_H
