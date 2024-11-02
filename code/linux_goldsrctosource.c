
#define _DEFAULT_SOURCE

#include "goldsrctosource.c"
#include <stdio.h>
#include "cstdlib_goldsrctosource.c"

#include <sys/mman.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <strings.h>

static_global char g_charBuffer[2048];

static_function const char *Linux_GetFileExtension(const char *file)
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

static_function void AppendToPath(char *path, i64 pathLength, const char *file)
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

static_function i32 GetDirectoryFiles(const char *path, FileInfo *out, i32 maxFileCount, const char *fileExtFilter)
{
	DIR *directory = opendir(path);
	i32 count = 0;
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
			
			Format(fileInfo.path, sizeof(fileInfo.path), "%s", path);
			AppendToPath(fileInfo.path, sizeof(fileInfo.path), entry->d_name);
			out[count++] = fileInfo;
		}
		closedir(directory);
	}
	return count;
}


static_function void *Plat_MemReserve(i64 bytes)
{
	void *result = mmap(NULL, bytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
	return result;
}

static_function void Plat_MemCommit(void *address, i64 bytes)
{
	// NOTE(GameChaos): not applicable on linux
}

static_function void Plat_MemDecommit(void *address, i64 bytes)
{
	// NOTE(GameChaos): not applicable on linux?
}

static_function void Plat_MemFree(void *address, i64 bytes)
{
	munmap(address, bytes);
}

static_function i64 Plat_GetPageSize(void)
{
	i64 result = sysconf(_SC_PAGESIZE);
	return result;
}

static_function void Mem_Copy(const void *source, void *destination, size_t bytes)
{
	ASSERT(source);
	ASSERT(destination);
	ASSERT(bytes);
	memcpy(destination, source, bytes);
}

static_function bool Mem_Compare(const void *a, const void *b, i64 bytes)
{
	bool result = memcmp(a, b, bytes) == 0;
	return result;
}

static_function void Plat_MemSetToZero(void *destination, i64 bytes)
{
	memset((u8 *)destination, 0, bytes);
}

static_function bool StringEquals(const char *a, const char *b, bool caseSensitive)
{
	if (caseSensitive)
	{
		return strcmp(a, b) == 0;
	}
	return strcasecmp(a, b) == 0;
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