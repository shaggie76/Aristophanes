#include "AristophanesPch.h"

#include "Resources.h"
#include "Assert.h"

HICON Resources::gIcon = NULL;

void Resources::Load()
{
    gIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(ICON_ARISTOPHANES));
    Assert(gIcon);
    
}

void Resources::Free()
{
    Assert(DestroyIcon(gIcon));
    gIcon = NULL;
}

