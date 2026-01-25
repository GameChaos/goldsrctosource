
#define _DEFAULT_SOURCE

#include "goldsrctosource.c"
#include <stdio.h>
#include "cstdlib_goldsrctosource.c"

#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <strings.h>
#include <errno.h>

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
			else
			{
				continue;
			}
			
			Format(fileInfo.path, sizeof(fileInfo.path), "%s", path);
			AppendToPath(fileInfo.path, sizeof(fileInfo.path), entry->d_name);
			out[count++] = fileInfo;
		}
		closedir(directory);
	}
	return count;
}

// https://stackoverflow.com/a/70358229
static_function bool Plat_MakeDirectories(char *path)
{
	char *nextSeparator = strchr(path, '/');
	bool result = true;
	while (nextSeparator != NULL)
	{
		i32 dirPathLen = (i32)(nextSeparator - path);
		Format(g_charBuffer, sizeof(g_charBuffer), "%.*s", dirPathLen, path);
		if (mkdir(g_charBuffer, S_IRWXU | S_IRWXG | S_IROTH) == -1)
		{
			if (errno != EEXIST)
			{
				result = false;
				break;
			}
		}
		nextSeparator = strchr(nextSeparator + 1, '/');
	}
	
	return result;
}

static_function void *Plat_MemReserve(i64 bytes)
{
	void *result = mmap(NULL, bytes, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (result == MAP_FAILED)
	{
		result = NULL;
	}
	return result;
}

static_function void Plat_MemCommit(void *address, i64 bytes)
{
	mprotect(address, bytes, PROT_READ | PROT_WRITE);
}

static_function void Plat_MemDecommit(void *address, i64 bytes)
{
	madvise(address, bytes, MADV_DONTNEED);
	mprotect(address, bytes, PROT_NONE);
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

static_function bool StringEqualsLen(const char *a, const char *b, i32 count, bool caseSensitive)
{
	if (caseSensitive)
	{
		return strncmp(a, b, count) == 0;
	}
	return strncasecmp(a, b, count) == 0;
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
