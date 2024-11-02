
#define UNICODE

#include "goldsrctosource.c"

#include "cstdlib_goldsrctosource.c"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>

typedef wchar_t wchar;

static_global SYSTEM_INFO g_systemInfo = {0};
static_global wchar g_wcharBuffer[2048];

static_function i32 Win32Utf8ToUtf16(const char *utf8, wchar *utf16, i32 utf16Chars)
{
	if (utf16)
	{
		utf16[0] = L'\0';
	}
	i32 result = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, utf16, utf16Chars);
	return result;
}

static_function i32 Win32Utf16ToUtf8(const wchar *utf16, char *utf8, i32 utf8Chars)
{
	if (utf8)
	{
		utf8[0] = '\0';
	}
	i32 result = WideCharToMultiByte(CP_UTF8, 0, utf16, -1, utf8, utf8Chars, NULL, NULL);
	return result;
}

static_function bool StringEqualsW(wchar *a, wchar *b, bool caseSensitive)
{
	bool result = true;
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

static_function void Win32FixInconsistentSlashes(char *path)
{
	for (char *c = path; *c != 0; c++)
	{
		if (*c == '/')
		{
			*c = '\\';
		}
	}
}

static_function wchar *Win32GetFileExtension(wchar *file)
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

static_function void AppendToPath(char *path, i64 pathLength, const char *file)
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

static_function i32 GetDirectoryFiles(char *path, FileInfo *out, i32 maxFileCount, const char *fileExtFilter)
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

static_function void *Plat_MemReserve(i64 bytes)
{
	void *result = VirtualAlloc(NULL, bytes, MEM_RESERVE, PAGE_READWRITE);
	return result;
}

static_function void Plat_MemCommit(void *address, i64 bytes)
{
	VirtualAlloc(address, bytes, MEM_COMMIT, PAGE_READWRITE);
}

static_function void Plat_MemDecommit(void *address, i64 bytes)
{
	VirtualFree(address, bytes, MEM_DECOMMIT);
}

static_function void Plat_MemFree(void *address, i64 bytes)
{
	VirtualFree(address, bytes, MEM_RELEASE);
}

static_function i64 Plat_GetPageSize(void)
{
	i64 result = (i64)g_systemInfo.dwPageSize;
	return result;
}

static_function void Mem_Copy(const void *source, void *destination, size_t bytes)
{
	ASSERT(source);
	ASSERT(destination);
	ASSERT(bytes);
	CopyMemory(destination, source, bytes);
}

static_function bool Mem_Compare(const void *a, const void *b, i64 bytes)
{
	bool result = memcmp(a, b, bytes) == 0;
	return result;
}

static_function void Plat_MemSetToZero(void *destination, i64 bytes)
{
	__stosb((u8 *)destination, 0, bytes);
}

static_function bool StringEquals(const char *a, const char *b, bool caseSensitive)
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