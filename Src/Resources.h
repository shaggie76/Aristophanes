#ifndef RESOURCES_H
#define RESOURCES_H

#include <windows.h>

#include "resource.h"

namespace Resources
{
    void Load();
    void Free();

    extern HICON gIcon;
}

#endif // RESOURCES_H