
static_function str StrFromSize(const char *cstring, i64 length)
{
	str result = {length, cstring};
	return result;
}

static_function bool StrEquals(str a, str b, bool caseSensitive)
{
	bool result = true;
	if (a.length == b.length)
	{
		const char *aEnd = a.data + a.length;
		const char *bEnd = b.data + b.length;
		if (caseSensitive)
		{
			result = Mem_Compare(a.data, b.data, a.length);
		}
		else
		{
			for (const char *c1 = a.data, *c2 = b.data;
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

static_function void StrPrint(str string)
{
	ASSERT(string.length <= I32_MAX);
	// TODO: don't cast u64 to i32?
	Plat_WriteToStdout(string.data, (i32)string.length);
}

static_function str_builder StrbuilderCreate(Arena *arena, i64 bytes)
{
	str_builder result = {};
	result.data = ArenaAlloc(arena, bytes);
	if (result.data)
	{
		result.storage = bytes;
	}
	return result;
}

static_function str_builder StrbuilderCreateFromData(void *data, i64 bytes)
{
	str_builder result = {};
	result.data = (char *)data;
	if (result.data)
	{
		result.storage = bytes;
	}
	return result;
}

static_function str StrbuilderGetStr(str_builder builder)
{
	str result = {builder.length, builder.data};
	return result;
}

static_function bool StrbuilderCat(str_builder *a, str b)
{
	bool result = false;
	if (a->storage - a->length >= b.length)
	{
		Mem_Copy(b.data, a->data + a->length, GCM_MIN(b.length, a->storage - a->length));
		a->length += b.length;
	}
	return result;
}

static_function i32 StrbuilderFormat(str_builder *string, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	
	i32 result = VFormat(string->data, string->storage, format, args);
	string->length += result;
	va_end(args);
	return result;
}

static_function i32 StrbuilderPushFormat(str_builder *string, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	
	i32 result = VFormat(string->data + string->length, string->storage - string->length, format, args);
	string->length += result;
	va_end(args);
	return result;
}

static_function bool StrbuilderClear(str_builder *string)
{
	bool result = false;
	if (string && string->data)
	{
		string->data[0] = '\0';
		string->length = 0;
	}
	return result;
}
