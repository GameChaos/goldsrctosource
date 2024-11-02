
#include "printing.h"

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
