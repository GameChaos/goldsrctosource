
#include <string.h>

static_function ReadFileResult ReadEntireFile(Arena *arena, const char *filePath)
{
	ReadFileResult result = {0};
	
	FILE *file = fopen(filePath, "rb");
	if (file)
	{
		fseek(file, 0, SEEK_END);
		i64 fileSize = ftell(file);
		fseek(file, 0, SEEK_SET);
		
		if (fileSize)
		{
			void *data = ArenaAlloc(arena, fileSize);
			if (data && fread(data, fileSize, 1, file) > 0)
			{
				result.size = fileSize;
				result.contents = data;
			}
		}
		fclose(file);
	}
	
	return result;
}

static_function bool WriteEntireFile(const char *filename, const void *memory, i64 bytes)
{
	bool result = false;
	FILE *file = fopen(filename, "wb");
	if (file)
	{
		if ((i64)fwrite(memory, 1, bytes, file) == bytes)
		{
			result = true;
		}
		fclose(file);
	}
	return result;
}

static_function size_t StringLength(const char *string)
{
	return strlen(string);
}

static_function void Plat_WriteToStdout(const char *str, i64 len)
{
	fwrite(str, len, 1, stdout);
}
