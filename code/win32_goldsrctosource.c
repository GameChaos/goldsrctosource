
#define UNICODE

#include "goldsrctosource.cpp"

#include "cstdlib_goldsrctosource.cpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>

typedef wchar_t wchar;

global SYSTEM_INFO g_systemInfo = {0};
global wchar g_wcharBuffer[2048];

internal s32 Win32Utf8ToUtf16(const char *utf8, wchar *utf16, s32 utf16Chars)
{
	if (utf16)
	{
		utf16[0] = L'\0';
	}
	s32 result = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, utf16, utf16Chars);
	return result;
}

internal s32 Win32Utf16ToUtf8(const wchar *utf16, char *utf8, s32 utf8Chars)
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

internal void Win32FixInconsistentSlashes(char *path)
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

internal void AppendToPath(char *path, s64 pathLength, const char *file)
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

internal s32 GetDirectoryFiles(char *path, FileInfo *out, s32 maxFileCount, const char *fileExtFilter)
{
	WIN32_FIND_DATA findFileData = {};
	char wildcardPath[1024] = "";
	wchar fileExtFilterW[128] = L"";
	
	Format(wildcardPath, sizeof(wildcardPath), "%s", path);
	AppendToPath(wildcardPath, sizeof(wildcardPath), "*");
	
	Win32Utf8ToUtf16(wildcardPath, g_wcharBuffer, ARRAYCOUNT(g_wcharBuffer));
	Win32Utf8ToUtf16(fileExtFilter, fileExtFilterW, ARRAYCOUNT(fileExtFilterW));
	
	HANDLE findFile = FindFirstFile(g_wcharBuffer, &findFileData);
	
	u32 result = 0;
	if (findFile != INVALID_HANDLE_VALUE)
	{
		// NOTE: skip "." and ".." entries
		FindNextFile(findFile, &findFileData);
		while (FindNextFile(findFile, &findFileData) && result < maxFileCount)
		{
			FileInfo fileInfo = {};
			
			if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				fileInfo.isFolder = true;
			}
			else
			{
				wchar *ext = Win32GetFileExtension(findFileData.cFileName);
				if (fileExtFilter != NULL)
				{
					if (!ext || !StringEqualsW(ext, fileExtFilterW, false))
					{
						continue;
					}
				}
			}
			
			char filename[260];
			Format(fileInfo.path, sizeof(fileInfo.path), "%s", path);
			Win32Utf16ToUtf8(findFileData.cFileName, filename, ARRAYCOUNT(filename));
			AppendToPath(fileInfo.path, sizeof(fileInfo.path), filename);
			Win32FixInconsistentSlashes(fileInfo.path);
			out[result++] = fileInfo;
		}
	}
	FindClose(findFile);
	return result;
}

internal void *Plat_MemReserve(s64 bytes)
{
	void *result = VirtualAlloc(NULL, bytes, MEM_RESERVE, PAGE_READWRITE);
	return result;
}

internal void Plat_MemCommit(void *address, s64 bytes)
{
	VirtualAlloc(address, bytes, MEM_COMMIT, PAGE_READWRITE);
}

internal void Plat_MemDecommit(void *address, s64 bytes)
{
	VirtualFree(address, bytes, MEM_DECOMMIT);
}

internal void Plat_MemFree(void *address, s64 bytes)
{
	VirtualFree(address, bytes, MEM_RELEASE);
}

internal s64 Plat_GetPageSize(void)
{
	s64 result = (s64)g_systemInfo.dwPageSize;
	return result;
}

internal void Mem_Copy(const void *source, void *destination, size_t bytes)
{
	ASSERT(source);
	ASSERT(destination);
	ASSERT(bytes);
	CopyMemory(destination, source, bytes);
}

internal b32 Mem_Compare(const void *a, const void *b, s64 bytes)
{
	b32 result = memcmp(a, b, bytes) == 0;
	return result;
}

internal void Plat_MemSetToZero(void *destination, s64 bytes)
{
	__stosb((u8 *)destination, 0, bytes);
}

internal b32 StringEquals(const char *a, const char *b, b32 caseSensitive)
{
	if (caseSensitive)
	{
		return strcmp(a, b);
	}
	return _stricmp(a, b);
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
	getc(stdin);
#endif
#endif
	ExitProcess(0);
}