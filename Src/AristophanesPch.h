#ifndef ARISTOPHANES_PCH_H
#define ARISTOPHANES_PCH_H

#pragma warning(push)
#pragma warning(disable : 4702)
#include <windows.h>
#undef max
#undef min

#include <commctrl.h>
#include <assert.h>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable : 4100)
#include <id3/tag.h>
#include <id3/misc_support.h>
#pragma warning(pop)

typedef std::string String;
typedef std::stringstream SStream;

#define ARRAY_COUNT(a) (sizeof(a) / sizeof(a[0]))

#if defined(_lint)
    #define DoAssert(expression) { if(!(expression)) { abort(); } }
#else
    #define DoAssert(expression) { size_t a = (size_t)(expression); if(!a) { __asm { int 3 } } }
#endif 

#if !defined(NDEBUG)
    #define Assert(expression)      DoAssert(expression) //lint !e683: function 'Assert' #define'd, semantics may be lost
    #define DebugAssert(expression) DoAssert(expression) //lint !e683: function 'DebugAssert' #define'd, semantics may be lost
#else
    #define Assert(expression)      DoAssert(expression)
    #define DebugAssert(expression) if(expression){}
#endif

typedef unsigned __int64 QWORD;

inline bool IsZero(const FILETIME& time)
{
    return(!time.dwLowDateTime && !time.dwLowDateTime);
}

namespace Util
{
    __forceinline bool True()
    {
        return(true);
    }

    __forceinline bool False()
    {
        return(false);
    }

    __forceinline bool Debug()
    {
#ifdef _DEBUG
        return(true);
#else
        return(false);
#endif
    }

    __forceinline bool Release()
    {
#ifdef NDEBUG
        return(true);
#else
        return(false);
#endif
    }
    
    void Zeroize(void* memory, size_t size);
    void Randomize(void* memory, size_t size);
}

//
// safe deletion of pointers
//

template<typename T>
inline void SafeRelease(T*& p)
{
    if(p)
    {
        p->Release();
        p = 0;
    }
}

template<typename T>
inline void SafeCloseHandle(T& p)
{
    if(p)
    {
        Assert(CloseHandle(p));
        p = 0;
    }
}

template<typename T>
inline void SafeDelete(T*& p)
{
    if(p)
    {
        delete p;
        p = 0;
    }
}

template<typename T>
inline void SafeDeleteArray(T*& p)
{
    if(p)
    {
        delete[] p;
        p = 0;
    }
}

extern HWND FindBestParent();

extern bool gCancel;
extern String gTask;
extern float gProgress;

#endif // ARISTOPHANES_PCH_H
