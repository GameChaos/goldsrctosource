
#define UNICODE

#include "goldsrctosource.cpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <shlobj_core.h>

#define STB_SPRINTF_IMPLEMENTATION
#include "stb/stb_sprintf.h"

typedef wchar_t wchar;

global SYSTEM_INFO g_systemInfo = {0};
global wchar g_wcharBuffer[2048];

inline f32 f32floor(f32 value)
{
	f32 result = floorf(value);
	return result;
}

inline f32 f32ceil(f32 value)
{
	f32 result = ceilf(value);
	return result;
}

internal ReadFileResult ReadEntireFile(Arena *arena, char *filePath)
{
	ReadFileResult result = {};
	
	HANDLE fileHandle = CreateFileA(filePath, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	if (filePath)
	{
		if (fileHandle != INVALID_HANDLE_VALUE)
		{
			LARGE_INTEGER fileSize;
			if (GetFileSizeEx(fileHandle, &fileSize))
			{
				ASSERT(fileSize.QuadPart <= 0xFFFFFFFF);
				u32 fileSize32 = (u32)fileSize.QuadPart;
				result.contents = ArenaAlloc(arena, fileSize32);
				if (result.contents)
				{
					DWORD bytesRead;
					if (ReadFile(fileHandle, result.contents, fileSize32, &bytesRead, 0)
						&& fileSize32 == bytesRead)
					{
						result.size = fileSize32;
					}
					else
					{
						if (result.contents)
						{
							VirtualFree(result.contents, 0, MEM_RELEASE);
						}
						result.contents = 0;
						Error("ReadEntireFile(): Couldn't read contents of file %s", filePath);
					}
				}
				else
				{
					Error("ReadEntireFile(): Couldn't allocate memory for file %s", filePath);
				}
			}
			else
			{
				Error("ReadEntireFile(): Couldn't get filesize of %s", filePath);
			}
			CloseHandle(fileHandle);
		}
		else
		{
			Error("ReadEntireFile(): Couldn't open file %s for reading", filePath);
		}
	}
	else
	{
		Error("ReadEntireFile(): Invalid path provided.");
	}
	
	return result;
}

internal s32 GetFilePath(char *file, char *out, s32 destSize)
{
	char *name = NULL;
	s32 result = GetFullPathNameA(file, destSize, out, &name);
	if (name)
	{
		*name = '\0';
	}
	return result;
}

// limited to max 4 gigs FOR NOW
internal b32 WriteEntireFile(char *filename, void *memory, u32 bytes)
{
	b32 result = false;
    
	char path[248];
	GetFilePath(filename, path, sizeof(path));
	DWORD dirCreate = SHCreateDirectoryExA(NULL, path, NULL);
	if (dirCreate == ERROR_SUCCESS
		|| dirCreate == ERROR_FILE_EXISTS
		|| dirCreate == ERROR_ALREADY_EXISTS)
	{
		HANDLE fileHandle = CreateFileA(filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
		if (fileHandle != INVALID_HANDLE_VALUE)
		{
			DWORD bytesWritten;
			if (WriteFile(fileHandle, memory, bytes, &bytesWritten, 0))
			{
				result = (bytesWritten == bytes);
			}
			else
			{
				ASSERT(0);
			}
			
			CloseHandle(fileHandle);
		}
		else
		{
			ASSERT(0);
		}
	}
	else
	{
		ASSERT(0);
	}
	
	return result;
}

internal void AppendToPath(char *path, s64 pathLength, char *file)
{
	size_t pathStrLen = strlen(path);
	// TODO: this could be more robust probably
	if (path[pathStrLen - 1] == '\\' || path[pathStrLen - 1] == '/'
		|| pathStrLen == 0)
	{
		Format(path + pathStrLen, (pathLength - pathStrLen), "%s", file);
	}
	else
	{
		Format(path + pathStrLen, (pathLength - pathStrLen), "\\%s", file);
	}
}

#ifdef GC_DEBUG
struct DebugFopenPayload
{
	HANDLE fileHandle;
	char *buffer;
	DWORD bytesWritten;
};

internal File DebugFopen(char *path)
{
	File result = {};
	HANDLE fileHandle = CreateFileA(path, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    if (fileHandle != INVALID_HANDLE_VALUE)
    {
        result.handle = (u64)fileHandle;
    }
    else
    {
		ASSERT(0);
    }
	return result;
}

internal char *DebugFprintfCb_(const char *buf, void *user, int len)
{
	ASSERT(buf);
	ASSERT(user);
	DebugFopenPayload *payload = (DebugFopenPayload *)user;
	DWORD bytesWritten = 0;
	WriteFile(payload->fileHandle, buf, len, &bytesWritten, 0);
	payload->bytesWritten += bytesWritten;
	return (char *)payload->buffer;
}

internal s32 DebugFprintf(File file, char *format, ...)
{
	s32 result = 0;
	HANDLE fileHandle = (HANDLE)file.handle;
	va_list args;
	va_start(args, format);
    if (fileHandle != INVALID_HANDLE_VALUE)
    {
		char buffer[STB_SPRINTF_MIN];
		DebugFopenPayload payload = {fileHandle, buffer, 0};
		
		stbsp_vsprintfcb(DebugFprintfCb_, &payload, buffer, format, args);
		result = payload.bytesWritten;
    }
    else
    {
		ASSERT(0);
    }
	va_end(args);
	return result;
}

internal s32 DebugFwrite(File file, void *data, s32 bytes)
{
	s32 result = 0;
	HANDLE fileHandle = (HANDLE)file.handle;
    if (fileHandle != INVALID_HANDLE_VALUE)
    {
		DWORD bytesWritten = 0;
		WriteFile(fileHandle, data, bytes, &bytesWritten, 0);
		result = bytesWritten;
    }
    else
    {
		ASSERT(0);
    }
	return result;
}

internal void DebugFclose(File *file)
{
	CloseHandle((HANDLE)file->handle);
	file->handle = (u64)INVALID_HANDLE_VALUE;
}
#endif

internal s32 Win32Utf8ToUtf16(char *utf8, wchar *utf16, s32 utf16Chars)
{
	if (utf16)
	{
		utf16[0] = L'\0';
	}
	s32 result = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, utf16, utf16Chars);
	return result;
}

internal s32 Win32Utf16ToUtf8(wchar *utf16, char *utf8, s32 utf8Chars)
{
	if (utf8)
	{
		utf8[0] = '\0';
	}
	s32 result = WideCharToMultiByte(CP_UTF8, 0, utf16, -1, utf8, utf8Chars, NULL, NULL);
	return result;
}

internal b32 StringEqualsW(wchar *a, wchar *b, b32 caseSensitive)
{
	b32 result = true;
	if (caseSensitive)
	{
		result = wcscmp(a, b) == 0;
	}
	else
	{
		for (wchar *c1 = a, *c2 = b;
			 *c1 && *c2;
			 c1++, c2++)
		{
			wchar lower1 = *c1 <= L'z' ? *c1 & 0xdf : *c1;
			wchar lower2 = *c2 <= L'z' ? *c2 & 0xdf : *c1;
			if (lower1 != lower2)
			{
				result = false;
				break;
			}
		}
	}
	return result;
}
internal void FixInconsistentSlashes(char *path)
{
	for (char *c = path; *c != 0; c++)
	{
		if (*c == '/')
		{
			*c = '\\';
		}
	}
}

internal wchar *Win32GetFileExtension(wchar *file)
{
	wchar *result = NULL;
	if (file)
	{
		u64 strLen = wcslen(file);
		if (strLen)
		{
			for (wchar *c = file + strLen - 1;
				 c != file;
				 c--)
			{
				if (*c == L'.')
				{
					result = c;
					break;
				}
			}
		}
	}
	return result;
}

internal u32 GetDirectoryFiles(char *path, FileInfo *out, u32 maxFileCount, char *fileExtFilter)
{
	WIN32_FIND_DATA findFileData = {};
	char wildcardPath[1024] = "";
	wchar fileExtFilterW[128] = L"";
	
	Format(wildcardPath, sizeof(wildcardPath), "%s", path);
	AppendToPath(wildcardPath, sizeof(wildcardPath), "*");
	
	Win32Utf8ToUtf16(path, g_wcharBuffer, ARRAYCOUNT(g_wcharBuffer));
	Win32Utf8ToUtf16(fileExtFilter, fileExtFilterW, ARRAYCOUNT(fileExtFilterW));
	
	HANDLE findFile = FindFirstFile(g_wcharBuffer, &findFileData);
	
	u32 result = 0;
	if (findFile != INVALID_HANDLE_VALUE)
	{
		// NOTE: skip "." and ".." entries
		FindNextFile(findFile, &findFileData);
		while (FindNextFile(findFile, &findFileData) && result < maxFileCount)
		{
			if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				wchar *ext = Win32GetFileExtension(findFileData.cFileName);
				if (fileExtFilter != NULL)
				{
					if (!ext || !StringEqualsW(ext, fileExtFilterW, false))
					{
						continue;
					}
				}
				FileInfo fileInfo = {};
				Format(fileInfo.path, sizeof(fileInfo.path), "%s", path);
				char filename[128];
				
				Win32Utf16ToUtf8(findFileData.cFileName, filename, ARRAYCOUNT(filename));
				AppendToPath(fileInfo.path, sizeof(fileInfo.path), filename);
				FixInconsistentSlashes(fileInfo.path);
				out[result++] = fileInfo;
			}
		}
	}
	FindClose(findFile);
	return result;
}

inline s64 AlignUp_(s64 val, s64 granularity)
{
    s64 result = (val / granularity) * granularity + granularity;
	return result;
}

inline s64 AlignDown_(s64 val, s64 granularity)
{
    s64 result = (val / granularity) * granularity;
	return result;
}

internal Arena ArenaCreate(s64 bytes)
{
	Arena result = {0};
	
	ASSERT(bytes);
	result.data = VirtualAlloc(NULL, bytes, MEM_RESERVE, PAGE_READWRITE);
	result.bytes = bytes;
	ASSERT(result.data);
	
	return result;
}

internal void *ArenaAlloc(Arena *arena, s64 bytes)
{
	// TODO: clear out memory when allocating?
	void *result = NULL;
	ASSERT(bytes <= arena->bytes - arena->allocPos);
	ASSERT(arena);
	if (bytes <= arena->bytes - arena->allocPos)
	{
		result = (u8 *)arena->data + arena->allocPos;
		VirtualAlloc((u8 *)arena->data + arena->allocPos, bytes, MEM_COMMIT, PAGE_READWRITE);
		arena->allocPos += bytes;
	}
	
	return result;
}

internal void ArenaReset(Arena *arena)
{
	ASSERT(arena->data);
	ASSERT(arena->bytes);
	VirtualFree(arena->data, arena->bytes, MEM_DECOMMIT);
	arena->allocPos = 0;
}

internal void ArenaResetTo(Arena *arena, s64 pos)
{
	if (pos >= 0 && pos < arena->allocPos)
	{
		s64 decommitStart = AlignUp_(pos, g_systemInfo.dwPageSize);
		s64 decommitEnd = AlignUp_(arena->bytes, g_systemInfo.dwPageSize);
		if (decommitStart < decommitEnd)
		{
			VirtualFree((u8 *)arena->data + decommitStart,
						decommitEnd - decommitStart, MEM_DECOMMIT);
		}
		
		arena->allocPos = pos;
	}
}

internal void ArenaFree(Arena *arena)
{
	ASSERT(arena->data);
	ASSERT(arena->bytes);
	VirtualFree(arena->data, 0, MEM_RELEASE);
	arena->data = 0;
	arena->allocPos = 0;
	arena->bytes = 0;
}

internal ArenaTemp ArenaBeginTemp(Arena *arena)
{
	ASSERT(arena);
	ASSERT(arena->data);
	ASSERT(arena->bytes);
	ArenaTemp result = {
		arena,
		arena->allocPos
	};
	
	return result;
}

internal void ArenaEndTemp(ArenaTemp temp)
{
	if (temp.originalAllocPos != temp.arena->allocPos)
	{
		s64 decommitStart = AlignUp_(temp.originalAllocPos, g_systemInfo.dwPageSize);
		s64 decommitEnd = AlignUp_(temp.arena->bytes, g_systemInfo.dwPageSize);
		if (decommitStart < decommitEnd)
		{
			VirtualFree((u8 *)temp.arena->data + decommitStart,
						decommitEnd - decommitStart, MEM_DECOMMIT);
		}
		
		temp.arena->allocPos = temp.originalAllocPos;
	}
}

internal void Mem_Copy(void *source, void *destination, size_t bytes, size_t destSize)
{
	// TODO: determine if this succeeds
	size_t size = bytes;
	if (destSize != 0 && bytes > destSize)
	{
		size = destSize;
	}
	ASSERT(destSize >= bytes);
	CopyMemory(destination, source, size);
}

internal void Mem_Copy(void *source, void *destination, size_t bytes)
{
	ASSERT(source);
	ASSERT(destination);
	ASSERT(bytes);
	CopyMemory(destination, source, bytes);
}

internal b32 Mem_Compare(void *a, void *b, s64 bytes)
{
	b32 result = memcmp(a, b, bytes) == 0;
	return result;
}

internal void Mem_SetToZero(void *destination, u64 bytes)
{
	__stosb((u8 *)destination, 0, bytes);
}

internal void PrintToStdoutLen(const char *str, s32 len)
{
	HANDLE stdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (stdOut != NULL && stdOut != INVALID_HANDLE_VALUE)
	{
		DWORD written = 0;
		WriteConsoleA(stdOut, str, len, &written, NULL);
	}
}

internal void PrintString(const char *str)
{
	ASSERT(str);
	u64 len = strlen(str);
	ASSERT(len <= S32_MAX);
	PrintToStdoutLen(str, (s32)len);
}

internal char *PrintCb_(const char *buf, void *user, int len)
{
	ASSERT(buf);
	ASSERT(user);
	PrintToStdoutLen(buf, len);
	return (char *)user;
}

internal void Print(char *format, ...)
{
	va_list args;
	va_start(args, format);
	
	char buffer[STB_SPRINTF_MIN];
	stbsp_vsprintfcb(PrintCb_, buffer, buffer, format, args);
	
	va_end(args);
}

internal void Vprint(char *format, va_list args)
{
	char buffer[STB_SPRINTF_MIN];
	stbsp_vsprintfcb(PrintCb_, buffer, buffer, format, args);
}

internal s32 Format(char *buffer, size_t maxlen, char *format, ...)
{
	va_list args;
	va_start(args, format);
	
	s32 result = stbsp_vsnprintf(buffer, (s32)maxlen, format, args);
	va_end(args);
	return result;
}

internal s32 Vformat(char *buffer, size_t maxlen, char *format, va_list va)
{
	s32 result = stbsp_vsnprintf(buffer, (s32)maxlen, format, va);
	return result;
}

internal size_t StringLength(char *string)
{
	return strlen(string);
}

internal b32 StringEquals(char *a, char *b, b32 caseSensitive)
{
	b32 result = true;
	if (caseSensitive)
	{
		result = strcmp(a, b) == 0;
	}
	else
	{
		for (char *c1 = a, *c2 = b;
			 *c1 && *c2;
			 c1++, c2++)
		{
			char lower1 = *c1 <= 'z' ? *c1 & 0xdf : *c1;
			char lower2 = *c2 <= 'z' ? *c2 & 0xdf : *c1;
			if (lower1 != lower2)
			{
				result = false;
				break;
			}
		}
	}
	return result;
}

int main(int argc, char **argv)
{
	GetSystemInfo(&g_systemInfo);
#ifdef DEBUG_GRAPHICS
	DebugGfxMain(argc, argv);
#else
	BSPMain(argc, argv);
#ifdef GC_DEBUG
	// pause
	getc();
#endif
#endif
	ExitProcess(0);
}