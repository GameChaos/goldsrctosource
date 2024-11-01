
#include <string.h>

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

internal ReadFileResult ReadEntireFile(Arena *arena, const char *filePath)
{
	ReadFileResult result = {0};
	
	FILE *file = fopen(filePath, "rb");
	if (file)
	{
		fseek(file, 0, SEEK_END);
		s64 fileSize = ftell(file);
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

internal b32 WriteEntireFile(const char *filename, const void *memory, s64 bytes)
{
	bool result = false;
	FILE *file = fopen(filename, "wb");
	if (file)
	{
		if (fwrite(memory, 1, bytes, file) == bytes)
		{
			result = true;
		}
		fclose(file);
	}
	return result;
}

internal size_t StringLength(const char *string)
{
	return strlen(string);
}

internal void Plat_WriteToStdout(const char *str, s64 len)
{
	fwrite(str, len, 1, stdout);
}
