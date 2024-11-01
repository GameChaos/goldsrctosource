/* date = November 1st 2024 7:29 pm */

#ifndef PRINTING_H
#define PRINTING_H

internal bool StringEquals(const char *a, const char *b, s64 maxChars);
internal void Print(const char *format, ...);
internal void VPrint(const char *format, va_list args);
internal s32 Format(char *buffer, s32 maxlen, const char *format, ...);
internal s32 VFormat(char *buffer, s32 maxlen, const char *format, va_list args);
internal void PrintString(const char *str);

#endif //PRINTING_H
