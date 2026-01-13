
#include "printing.h"
#include "cmacros.h"

// made from homogenous print by Alex Celeste: https://gist.github.com/AlexCeleste/4a996b5f505b7dd9d5f7492d168b133b

void MyFprintf_(FILE *stream, const char *format, i32 argc, str argv[static argc])
{
    i32 next_str = 0;
    for (const char *c = format; *c; ++c)
	{
        if (c[0] == '$' && c[1] != '$')
		{
            if (next_str >= argc)
			{
				//panic();
				// :( what do
				ASSERT(0);
				break;
			}
			fwrite(argv[next_str].data, 1, argv[next_str].length, stream);
#if 0
            for (const char * d = argv[next_str]; *d; ++ d)
			{
                putc(*d, stream);
            }
#endif
            ++next_str;
        }
		else if (c[0] != '$' || c[1] != '$')
		{
            putc(*c, stream);
        }
    }
}

enum { MY_FPRINTF_BUF_SIZE = 32 };

#define MyFprintf_(stream, format, ...) MyFprintf_(stream, format, M_NARGS(__VA_ARGS__), \
(str[]){ M_FOR_EACH(MY_FPRINTF_FORMAT_ARG, __VA_ARGS__) })

#define MY_FPRINTF_FORMAT_ARG(A) _Generic((0, A), \
bool: MyFormatBool_, \
i8: MyFormatI32_, \
i16: MyFormatI32_, \
i32: MyFormatI32_, \
i64: MyFormatI64_, \
u8: MyFormatU32_, \
u16: MyFormatU32_, \
u32: MyFormatU32_, \
u64: MyFormatU64_, \
f32: MyFormatF64_, \
f64: MyFormatF64_, \
char: MyFormatChar_, \
v2: MyFormatV2_, \
v3: MyFormatV3_, \
v4: MyFormatV4_, \
v2i: MyFormatV2i_, \
v3i: MyFormatV3i_, \
v4i: MyFormatV4i_, \
str: MyFormatStr_, \
char *: MyFormatString_, \
const char *: MyFormatString_)(A, (char[MY_FPRINTF_BUF_SIZE]){0}),

str MyFormatBool_(bool a, char buf[static MY_FPRINTF_BUF_SIZE])
{
	str result = StrFromSize(buf, Format(buf, MY_FPRINTF_BUF_SIZE, "%s", a ? "true" : "false"));
    return result;
}

str MyFormatI32_(i32 a, char buf[static MY_FPRINTF_BUF_SIZE])
{
	str result = StrFromSize(buf, Format(buf, MY_FPRINTF_BUF_SIZE, "%i", a));
    return result;
}

str MyFormatU32_(u32 a, char buf[static MY_FPRINTF_BUF_SIZE])
{
	str result = StrFromSize(buf, Format(buf, MY_FPRINTF_BUF_SIZE, "%u", a));
    return result;
}

str MyFormatI64_(i64 a, char buf[static MY_FPRINTF_BUF_SIZE])
{
	str result = StrFromSize(buf, Format(buf, MY_FPRINTF_BUF_SIZE, "%I64i", a));
    return result;
}

str MyFormatU64_(u64 a, char buf[static MY_FPRINTF_BUF_SIZE])
{
	str result = StrFromSize(buf, Format(buf, MY_FPRINTF_BUF_SIZE, "%I64u", a));
    return result;
}

str MyFormatF64_(double a, char buf[static MY_FPRINTF_BUF_SIZE])
{
    str result = StrFromSize(buf, Format(buf, MY_FPRINTF_BUF_SIZE, "%f", a));
    return result;
}

str MyFormatChar_(char a, char buf[static MY_FPRINTF_BUF_SIZE])
{
    str result = StrFromSize(buf, Format(buf, MY_FPRINTF_BUF_SIZE, "%c", a));
    return result;
}

str MyFormatV2_(v2 a, char buf[static MY_FPRINTF_BUF_SIZE])
{
    str result = StrFromSize(buf, Format(buf, MY_FPRINTF_BUF_SIZE, "%f %f", a.x, a.y));
    return result;
}

str MyFormatV3_(v3 a, char buf[static MY_FPRINTF_BUF_SIZE])
{
    str result = StrFromSize(buf, Format(buf, MY_FPRINTF_BUF_SIZE, "%f %f %f", a.x, a.y, a.z));
    return result;
}

str MyFormatV4_(v4 a, char buf[static MY_FPRINTF_BUF_SIZE])
{
    str result = StrFromSize(buf, Format(buf, MY_FPRINTF_BUF_SIZE, "%f %f %f %f", a.x, a.y, a.z, a.w));
    return result;
}

str MyFormatV2i_(v2i a, char buf[static MY_FPRINTF_BUF_SIZE])
{
    str result = StrFromSize(buf, Format(buf, MY_FPRINTF_BUF_SIZE, "%i %i", a.x, a.y));
    return result;
}

str MyFormatV3i_(v3i a, char buf[static MY_FPRINTF_BUF_SIZE])
{
    str result = StrFromSize(buf, Format(buf, MY_FPRINTF_BUF_SIZE, "%i %i %i", a.x, a.y, a.z));
    return result;
}

str MyFormatV4i_(v4i a, char buf[static MY_FPRINTF_BUF_SIZE])
{
    str result = StrFromSize(buf, Format(buf, MY_FPRINTF_BUF_SIZE, "%i %i %i %i", a.x, a.y, a.z, a.w));
    return result;
}

str MyFormatStr_(str a, char unused[])
{
	(void)unused;
	return a;
}

str MyFormatString_(const char * a, char unused[])
{
	(void)unused;
    str result = StrFromSize(a, strlen(a));
    return result;
}

#define MyPrintf(...) MyFprintf_(stdout, __VA_ARGS__)

#if 0
int main(void) {
    MyPrintf("Hello $!\n", "world");
    MyPrintf("There are $ arguments to this call. The remainder are $, $, $, $ and $.\n",
              6, "foo", "bar", (char)'c', 'd', 4.75);
}
#endif
static_function char *PrintCb_(const char *buf, void *user, int len)
{
	ASSERT(buf);
	ASSERT(user);
	Plat_WriteToStdout(buf, len);
	return (char *)user;
}

__attribute__((format (printf, 1, 2)))
static_function void Print(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	
	char buffer[STB_SPRINTF_MIN];
	stbsp_vsprintfcb(PrintCb_, buffer, buffer, format, args);
	
	va_end(args);
}

static_function void VPrint(const char *format, va_list args)
{
	char buffer[STB_SPRINTF_MIN];
	stbsp_vsprintfcb(PrintCb_, buffer, buffer, format, args);
}

__attribute__((format (printf, 3, 4)))
static_function i32 Format(char *buffer, i32 maxlen, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	i32 result = stbsp_vsnprintf(buffer, maxlen, format, args);
	va_end(args);
	return result;
}

static_function i32 VFormat(char *buffer, i32 maxlen, const char *format, va_list args)
{
	i32 result = stbsp_vsnprintf(buffer, maxlen, format, args);
	return result;
}

static_function void PrintString(const char *str)
{
	ASSERT(str);
	i64 len = strlen(str);
	ASSERT(len <= I32_MAX);
	Print("%.*s", (i32)len, str);
}
