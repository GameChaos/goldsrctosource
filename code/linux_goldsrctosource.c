
#define _DEFAULT_SOURCE

#include "goldsrctosource.cpp"
#include <stdio.h>
#include "cstdlib_goldsrctosource.cpp"

#include <sys/mman.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <strings.h>

global char g_charBuffer[2048];

internal const char *Linux_GetFileExtension(const char *file)
{
	const char *result = NULL;
	if (file)
	{
		u64 strLen = strlen(file);
		if (strLen)
		{
			for (const char *c = file + strLen - 1;
				 c != file;
				 c--)
			{
				if (*c == '.')
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
	if (path[pathStrLen - 1] == '/' || pathStrLen == 0)
	{
		Format(path + pathStrLen, (pathLength - pathStrLen), "%s", file);
	}
	else
	{
		Format(path + pathStrLen, (pathLength - pathStrLen), "/%s", file);
	}
}

internal s32 GetDirectoryFiles(char *path, FileInfo *out, s32 maxFileCount, const char *fileExtFilter)
{
	DIR *directory = opendir(path);
	s32 count = 0;
	if (directory && out)
	{
		errno = 0;
		struct dirent *entry = NULL;
		while ((entry = readdir(directory)) && count < maxFileCount)
		{
			if (StringEquals(entry->d_name, ".", true)
				|| StringEquals(entry->d_name, "..", true))
			{
				continue;
			}
			
			FileInfo fileInfo = {};
			// TODO: does a symlink act as a normal file?
			if (entry->d_type == DT_REG || entry->d_type == DT_LNK)
			{
				const char *ext = Linux_GetFileExtension(entry->d_name);
				// NOTE(GameChaos): be case insensitive about extensions...
				if (fileExtFilter && (!ext || !StringEquals(ext, fileExtFilter, false)))
				{
					continue;
				}
			}
			else if (entry->d_type == DT_DIR)
			{
				fileInfo.isFolder = true;
			}
			
			// TODO: is this the absolute path?
			Format(fileInfo.path, sizeof(fileInfo.path), "%s", path);
			ASSERT(!"TODO: test if this is the absolue path!");
			AppendToPath(fileInfo.path, sizeof(fileInfo.path), entry->d_name);
			out[count++] = fileInfo;
		}
		closedir(directory);
	}
	return count;
}


internal void *Plat_MemReserve(s64 bytes)
{
	void *result = mmap(NULL, bytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
	return result;
}

internal void Plat_MemCommit(void *address, s64 bytes)
{
	// NOTE(GameChaos): not applicable on linux
}

internal void Plat_MemDecommit(void *address, s64 bytes)
{
	// NOTE(GameChaos): not applicable on linux?
}

internal void Plat_MemFree(void *address, s64 bytes)
{
	munmap(address, bytes);
}

internal s64 Plat_GetPageSize(void)
{
	s64 result = sysconf(_SC_PAGESIZE);
	return result;
}

internal void Mem_Copy(const void *source, void *destination, size_t bytes)
{
	ASSERT(source);
	ASSERT(destination);
	ASSERT(bytes);
	memcpy(destination, source, bytes);
}

internal b32 Mem_Compare(const void *a, const void *b, s64 bytes)
{
	b32 result = memcmp(a, b, bytes) == 0;
	return result;
}

internal void Plat_MemSetToZero(void *destination, s64 bytes)
{
	memset((u8 *)destination, 0, bytes);
}

internal b32 StringEquals(const char *a, const char *b, b32 caseSensitive)
{
	if (caseSensitive)
	{
		return strcmp(a, b);
	}
	return strcasecmp(a, b);
}

int main(int argc, char **argv)
{
#ifdef DEBUG_GRAPHICS
	DebugGfxMain(argc, argv);
#else
	BSPMain(argc, argv);
#ifdef GC_DEBUG
	// pause
	getc(stdin);
#endif
#endif
}