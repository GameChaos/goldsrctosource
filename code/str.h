/* date = November 10th 2024 0:02 pm */

#ifndef STR_H
#define STR_H

#define STR(strLiteral) ((str){sizeof(strLiteral) - 1, strLiteral})

typedef struct
{
	i64 length;
	const char *data;
} str;

typedef struct
{
	i64 length;
	i64 storage;
	char *data;
} str_builder;

static_function str StrFromSize(const char *cstring, i64 length);
static_function bool StrEquals(str a, str b, bool caseSensitive);
static_function void StrPrint(str string);
static_function str_builder StrbuilderCreate(Arena *arena, i64 bytes);
static_function str_builder StrbuilderCreateFromData(void *data, i64 bytes);
static_function str StrbuilderGetStr(str_builder builder);
static_function bool StrbuilderCat(str_builder *a, str b);
static_function i32 StrbuilderFormat(str_builder *string, const char *format, ...);
static_function i32 StrbuilderPushFormat(str_builder *string, const char *format, ...);
static_function bool StrbuilderClear(str_builder *string);

#endif //STR_H
