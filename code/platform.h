/* date = October 12th 2020 6:40 pm */

#ifndef PLATFORM_H
#define PLATFORM_H

struct ReadFileResult
{
	void *contents;
	u32 size;
};

struct FileInfo
{
	b32 isFolder;
	char path[260];
};

typedef struct Arena
{
	void *data;
	s64 allocPos;
	s64 bytes;
} Arena;

typedef struct ArenaTemp
{
	Arena *arena;
	s64 originalAllocPos;
} ArenaTemp;

#ifdef GC_DEBUG
struct File
{
	u64 handle;
};

internal File DebugFopen(char *path);
internal s32 DebugFprintf(File file, char *format, ...);
internal s32 DebugFwrite(File file, void *data, s32 bytes);
internal void DebugFclose(File *file);
#endif

inline f32 f32floor(f32 value);
inline f32 f32ceil(f32 value);
internal f32 Cos(f32 value);
internal f32 Sin(f32 value);
internal f32 Exp(f32 value);
internal f32 Log(f32 value);
// TODO:
internal f32 Acos(f32 value);

internal ReadFileResult ReadEntireFile(Arena *arena, char *filePath);
internal b32 WriteEntireFile(char *filename, void *memory, u32 bytes);
internal void AppendToPath(char *path, s64 pathLength, char *file);
internal u32 GetDirectoryFiles(char *path, FileInfo *out, u32 maxFileCount, char *fileExtFilter = NULL);

internal Arena ArenaCreate(s64 bytes);
internal void *ArenaAlloc(Arena *arena, s64 bytes);
internal void ArenaReset(Arena *arena);
internal void ArenaResetTo(Arena *arena, s64 pos);
internal void ArenaFree(Arena *arena);
internal ArenaTemp ArenaBeginTemp(Arena *arena);
internal void ArenaEndTemp(ArenaTemp temp);

internal void Mem_Copy(void *source, void *destination, size_t bytes, size_t destSize);
internal void Mem_Copy(void *source, void *destination, size_t bytes);
internal b32 Mem_Compare(void *a, void *b, s64 bytes);
internal void Mem_SetToZero(void *destination, u64 bytes);

internal void Print(char *format, ...);
internal s32 Format(char *buffer, size_t maxlen, char *format, ...);
internal s32 Vformat(char *buffer, size_t maxlen, char *format, va_list va);
internal size_t StringLength(char *string);
internal b32 StringEquals(char *a, char *b, b32 caseSensitive = true);

internal void FatalError(char *error);

internal void PlatformStart();

#endif //PLATFORM_H
