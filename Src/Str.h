#ifndef CORE_STR_H
#define CORE_STR_H

#include <cstdarg>

namespace Str
{
    extern size_t Len(const char* s);

    extern int Compare(const char* s1, const char* s2);
    extern int Compare(const char* s1, const char* s2, size_t maxLength);

    extern int LooseCompare(const char* s1, const char* s2);
    extern int LooseCompare(const char* s1, const char* s2, size_t maxLength);

    // The following functions return the length of the string in buffer (ie: Str::Len(buffer)).
    extern size_t Copy(char* buffer, size_t bufferSize, const char* s2);
    extern size_t Append(char* buffer, size_t bufferSize, const char* s2);
    
    extern size_t Print(char* buffer, size_t bufferSize, const char* format, ...);
    extern size_t PrintArgs(char* buffer, size_t bufferSize, const char* format, va_list args);

    extern void ToUpper(char* buffer, size_t bufferSize);
    extern void ToLower(char* buffer, size_t bufferSize);

    extern char* Find(const char* buffer, const char* subString);
    extern char* Find(const char* buffer, char searchChar);

    // Filename manipulation routines (C:\Dir\SubDir\FileName.extension)
    extern const char* BaseName(const char* fileName); // FileName.extension
    extern size_t FileName(char* buffer, size_t bufferSize, const char* fileName); // FileName.extension
    extern size_t DirName(char* buffer, size_t bufferSize, const char* fileName);  // C:\Dir\SubDir
    extern size_t BaseName(char* buffer, size_t bufferSize, const char* fileName); // C:\Dir\SubDir\FileName
    extern const char* Extension(const char* fileName); // .extension

    extern size_t ItoA(char* buffer, size_t bufferSize, int value, int radix);
 
    extern void UnicodeToAnsi(char* dest, size_t destSize, const wchar_t* src);
    extern void AnsiToUnicode(wchar_t* dest, size_t bufferSize, const char* src);
}

#endif // CORE_STR_H

