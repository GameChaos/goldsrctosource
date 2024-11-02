/* date = November 1st 2024 7:29 pm */

#ifndef PRINTING_H
#define PRINTING_H

static_function void Print(const char *format, ...);
static_function void VPrint(const char *format, va_list args);
static_function i32 Format(char *buffer, i32 maxlen, const char *format, ...);
static_function i32 VFormat(char *buffer, i32 maxlen, const char *format, va_list args);
static_function void PrintString(const char *str);

#endif //PRINTING_H
