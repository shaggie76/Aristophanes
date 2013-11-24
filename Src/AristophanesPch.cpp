#include "AristophanesPch.h"

bool gCancel = false;
std::string gTask;
float gProgress = 0.f;

void Util::Zeroize(void* memory, size_t size)
{
    memset(memory, 0, size);
}

void Util::Randomize(void* memory, size_t size)
{
    BYTE* b = reinterpret_cast<BYTE*>(memory);
    
    do
    {
        *b = static_cast<BYTE>(rand() & 0xFF);
        ++b;
        --size;
    } while(size);
    
}

static BOOL CALLBACK EnumWindowsProc(HWND windowHandle, LPARAM vpBestParent)
{
    HWND* bestParent = reinterpret_cast<HWND*>(vpBestParent);

    DWORD processId = 0;

    GetWindowThreadProcessId(windowHandle, &processId);
    
    if(processId == GetCurrentProcessId())
    {
        if(!(*bestParent) || IsChild(windowHandle, *bestParent))
        {
            (*bestParent) = windowHandle;
        }
    }
    
    return(TRUE);
}

HWND FindBestParent()
{
    HWND rc = NULL;
    EnumWindows(&EnumWindowsProc, reinterpret_cast<LPARAM>(&rc));
    return(rc);
}
