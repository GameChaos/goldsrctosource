
#include "printing.h"

internal char *PrintCb_(const char *buf, void *user, int len)
{
	ASSERT(buf);
	ASSERT(user);
	Plat_WriteToStdout(buf, len);
	return (char *)user;
}

internal bool StringEquals(const char *a, const char *b, s64 maxChars)
{
	bool result = a && b && (strncmp(a, b, maxChars) == 0);
	return result;
}

__attribute__((format (printf, 1, 2)))
internal void Print(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	
	char buffer[STB_SPRINTF_MIN];
	stbsp_vsprintfcb(PrintCb_, buffer, buffer, format, args);
	
	va_end(args);
}

internal void VPrint(const char *format, va_list args)
{
	char buffer[STB_SPRINTF_MIN];
	stbsp_vsprintfcb(PrintCb_, buffer, buffer, format, args);
}

__attribute__((format (printf, 3, 4)))
internal s32 Format(char *buffer, s32 maxlen, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	s32 result = stbsp_vsnprintf(buffer, maxlen, format, args);
	va_end(args);
	return result;
}

internal s32 VFormat(char *buffer, s32 maxlen, const char *format, va_list args)
{
	s32 result = stbsp_vsnprintf(buffer, maxlen, format, args);
	return result;
}

internal void PrintString(const char *str)
{
	ASSERT(str);
	s64 len = strlen(str);
	ASSERT(len <= S32_MAX);
	Print("%.*s", (s32)len, str);
}
