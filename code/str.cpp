
#define STR(strLiteral) (str{sizeof(strLiteral) - 1, strLiteral})

struct str
{
	u64 length;
	char *data;
};

struct str_builder
{
	u64 length;
	u64 storage;
	char *data;
};

internal str StrFromSize(char *cstring, u64 length)
{
	str result = {length, cstring};
	return result;
}

internal b32 StrEquals(str a, str b, b32 caseSensitive = true)
{
	b32 result = true;
	if (a.length == b.length)
	{
		char *aEnd = a.data + a.length;
		char *bEnd = b.data + b.length;
		if (caseSensitive)
		{
			result = Mem_Compare(a.data, b.data, a.length);
		}
		else
		{
			for (char *c1 = a.data, *c2 = b.data;
				 c1 < aEnd && c2 < bEnd;
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
	}
	else
	{
		result = false;
	}
	return result;
}

internal str_builder StrbuilderCreate(Arena *arena, s64 bytes)
{
	str_builder result = {};
	result.data = (char *)ArenaAlloc(arena, bytes);
	if (result.data)
	{
		result.storage = bytes;
	}
	return result;
}

internal str_builder StrbuilderCreateFromData(void *data, s64 bytes)
{
	str_builder result = {};
	result.data = (char *)data;
	if (result.data)
	{
		result.storage = bytes;
	}
	return result;
}

internal str StrbuilderGetStr(str_builder builder)
{
	str result = {builder.length, builder.data};
	return result;
}

internal b32 StrbuilderCat(str_builder *a, str b)
{
	b32 result = false;
	if (a->storage - a->length >= b.length)
	{
		Mem_Copy(b.data, a->data + a->length, b.length, a->storage - a->length);
		a->length += b.length;
	}
	return result;
}

internal s32 StrbuilderFormat(str_builder *string, char *format, ...)
{
	va_list args;
	va_start(args, format);
	
	s32 result = Vformat(string->data, string->storage, format, args);
	string->length += result;
	va_end(args);
	return result;
}

internal s32 StrbuilderPushFormat(str_builder *string, char *format, ...)
{
	va_list args;
	va_start(args, format);
	
	s32 result = Vformat(string->data + string->length, string->storage - string->length, format, args);
	string->length += result;
	va_end(args);
	return result;
}

internal b32 StrbuilderClear(str_builder *string)
{
	b32 result = false;
	if (string && string->data)
	{
		string->data[0] = '\0';
		string->length = 0;
	}
	return result;
}