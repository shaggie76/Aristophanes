#include "AristophanesPch.h"

#include <algorithm>
#include "Str.h"

size_t Str::Len(const char* s)
{
    return(strlen(s));
}

int Str::Compare(const char* s1, const char* s2)
{
    return(strcmp(s1, s2));
}

int Str::Compare(const char* s1, const char* s2, size_t maxLength)
{
    return(strncmp(s1, s2, maxLength));
}

int Str::LooseCompare(const char* s1, const char* s2)
{
    return(_stricmp(s1, s2));
}
  
int Str::LooseCompare(const char* s1, const char* s2, size_t maxLength)
{
    return(_strnicmp(s1, s2, maxLength));
}

size_t Str::Copy(char* buffer, size_t bufferSize, const char* s2)
{
    size_t chars = 0;

    // Need at least room for at least one char and a NULL to be worth while.
    if(bufferSize <= 1)
    {
        if(bufferSize != 0)
        {
            *buffer = '\0';
        }
        return(0);
    }
    
    while(*s2 && (chars < (bufferSize - 1)))
    {
        *buffer = *s2;
        buffer++;
        s2++;
        chars++;
    }
    
    *buffer = '\0';
    
    return(chars);
}

size_t Str::Append(char* buffer, size_t bufferSize, const char* s2)
{
    size_t chars = 0;
    
    // Find the end of the buffer (safely):
    while(*buffer && (bufferSize > 0))
    {
        buffer++;
        bufferSize--;
        chars++;
    }
    
    Assert((bufferSize > 0) && "Str::Append() destination buffer is not terminated!");
   
    return(chars + Str::Copy(buffer, bufferSize, s2));
}
    
size_t Str::Print(char* buffer, size_t bufferSize, const char* format, ...)
{
    va_list args = NULL;
    va_start(args, format);
    size_t chars = Str::PrintArgs(buffer, bufferSize, format, args);
    va_end(args);
    return(chars);
}

size_t Str::PrintArgs(char* buffer, size_t bufferSize, const char* format, va_list args)
{
    if(!*format)
    {
        return(0);
    }
    // Need at least room for at least one char and a NULL to be worth while.
    if(bufferSize <= 1)
    {
        return(0);
    }

    // Leave room for the NULL.

    int rc = _vsnprintf(buffer, bufferSize - 1, format, args);

    if(rc >= 0)
    {
        return(std::min(static_cast<size_t>(rc), bufferSize - 1));
    }
    else
    {
        buffer[bufferSize - 1] = '\0';
        return(bufferSize - 1);
    }
}

void Str::ToUpper(char* buffer, size_t bufferSize)
{
    for(size_t offset = 0; offset < bufferSize; offset++)
    {
        if(buffer[offset] == '\0')
        {
            return;
        }
    
        buffer[offset] = static_cast<char>(toupper(buffer[offset]));
    }
}

void Str::ToLower(char* buffer, size_t bufferSize)
{
    for(size_t offset = 0; offset < bufferSize; offset++)
    {
        if(buffer[offset] == '\0')
        {
            return;
        }
        buffer[offset] = static_cast<char>(tolower(buffer[offset]));
    }
}

char* Str::Find(const char* buffer, const char* subString)
{
    char* result = strstr(const_cast<char*>(buffer), subString);
    return(result);
}

char* Str::Find(const char* buffer, char searchChar)
{
    char* result = strchr(const_cast<char*>(buffer), searchChar);
    return(result);
}

const char* Str::BaseName(const char* fileName)
{
    const char* p = fileName;
    
    size_t length = 0;
    
    while(*p)
    {
        p++;
        length++;
        
        Assert(length < MAX_PATH);
        if(length >= MAX_PATH)
        {
            return(fileName);
        }
    }
    
    p--;

    while((p != fileName) && (*p != '\\') && (*p != '/'))
    {
        p--;
    }

    if((*p == '\\') || (*p == '/'))
    {
        p++;
    }

    return(p);
}

const char* Str::Extension(const char* fileName)
{
    Assert(fileName);

    const char* p = fileName;
    
    size_t length = 0;
    
    while(*p)
    {
        p++;
        length++;
        Assert(length < MAX_PATH);
        if(length >= MAX_PATH)
        {
            return(fileName);
        }
    }
    
    const char* end = p;
    p--;

    while(*p != '.')
    {
        if(p == fileName)
        {
            return(end);
        }
        
        if((*p == '/') || (*p == '\\'))
        {
            return(end);
        }
        
        p--;
    }

    return(p);
}

size_t Str::FileName(char* buffer, size_t bufferSize, const char* fileName)
{
    Assert(fileName);
    const char *baseName = BaseName(fileName);
    return(Str::Copy(buffer, bufferSize, baseName));
}

size_t Str::DirName(char* buffer, size_t bufferSize, const char* fileName)
{
    Assert(fileName);
    const char *baseName = BaseName(fileName);
    
    if(baseName > fileName)
    {
        baseName--;
        Assert((*baseName == '\\') || (*baseName == '/'));
    }
    
    size_t charsToCopy = std::min<size_t>(static_cast<size_t>(baseName - fileName) + 1, bufferSize);
    return(Str::Copy(buffer, charsToCopy, fileName));
}

size_t Str::BaseName(char* buffer, size_t bufferSize, const char* fileName)
{
    Assert(fileName);
    const char* extension = Str::Extension(fileName);
    size_t charsToCopy = std::min<size_t>(static_cast<size_t>(extension - fileName) + 1, bufferSize);
    return(Str::Copy(buffer, charsToCopy, fileName));
}

static size_t ItoA_Helper(char* buffer, size_t bufferSize, int value, int radix)
{
    Assert(value > 0);
    
    static const char* digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    
    int d = value / radix;
    int m = value % radix;
    
    size_t chars = 0;
    
    if(d > 0)
    {
        chars += ItoA_Helper(buffer, bufferSize, d, radix);
    }
    
    if(chars < bufferSize)
    {
        buffer[chars] = digits[m];
        ++chars;
    }
    
    return(chars);
}

size_t Str::ItoA(char* buffer, size_t bufferSize, int value, int radix)
{
    Assert(radix > 1);
    Assert(radix < 32);
    Assert(bufferSize > 2);

    --bufferSize; // Leave room for the '\0'

    size_t chars = 0;
    
    if(value == 0)
    {
        buffer[0] = '0';
        buffer[1] = '\0';
        return(1);
    }
    else if(value < 0)
    {
        buffer[chars] = '-';
        ++chars;
        value = -value;
    }
    
    chars += ItoA_Helper(&buffer[chars], bufferSize - chars, value, radix);
    buffer[chars] = '\0';
    
    return(chars);
}

void Str::UnicodeToAnsi(char* dest, size_t destSize, const wchar_t* src)
{
    size_t chars = 0;

    // Need at least room for at least one char and a NULL to be worth while.
    if(destSize <= 1)
    {
        if(destSize != 0)
        {
            *dest = '\0';
        }
        return;
    }
    
    while(*src && (chars < (destSize - 1)))
    {
        *dest = static_cast<char>(*src & 0xFF);
        dest++;
        src++;
        chars++;
    }
    
    *dest = '\0';
}
        
void Str::AnsiToUnicode(wchar_t* dest, size_t destSize, const char* src)
{
    size_t chars = 0;

    // Need at least room for at least one char and a NULL to be worth while.
    if(destSize <= 1)
    {
        if(destSize != 0)
        {
            *dest = '\0';
        }
        return;
    }
    
    while(*src && (chars < (destSize - 1)))
    {
        *dest = static_cast<unsigned short>(*src);
        dest++;
        src++;
        chars++;
    }
    
    *dest = '\0';
}


