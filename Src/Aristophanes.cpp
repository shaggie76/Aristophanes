#include "AristophanesPch.h"

#include "Library.h"
#include "Resources.h"

#include <sys/types.h>
#include <sys/stat.h>
    
BOOL CALLBACK IntroDialogProc(HWND windowHandle, UINT msg, WPARAM wParam, LPARAM)
{
    switch(msg)
    {
        case WM_INITDIALOG:
        {
            RECT dialogRect;
            RECT parentRect;
            INT x, y, dx, dy;

            GetWindowRect(windowHandle, &dialogRect);
            SystemParametersInfo(SPI_GETWORKAREA, 0, &parentRect, 0);

            dx = dialogRect.right - dialogRect.left;
            dy = dialogRect.bottom - dialogRect.top;

            x = parentRect.left + (parentRect.right - parentRect.left - dx) / 2;
            y = parentRect.top + (parentRect.bottom - parentRect.top - dy) / 2;

            MoveWindow(windowHandle, x, y, dx, dy, FALSE);

            SendMessage(windowHandle, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(Resources::gIcon));
            SendMessage(windowHandle, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(Resources::gIcon));

            CheckRadioButton(windowHandle, CONTROL_REBUILD_LIBRARY, CONTROL_UPDATE_LIBRARY, CONTROL_REBUILD_LIBRARY);

            return(TRUE);
        }

        case WM_COMMAND:
        {
            switch(LOWORD(wParam))
            {
                case CONTROL_OK:
                {
                    static int knownButtons[] =
                    {
                        CONTROL_UPDATE_LIBRARY,
                        CONTROL_REBUILD_LIBRARY
                    };

                    for(size_t i = 0; i < ARRAY_COUNT(knownButtons); ++i)
                    {
                        if(IsDlgButtonChecked(windowHandle, knownButtons[i]))
                        {
                            EndDialog(windowHandle, knownButtons[i]);
                            break;
                        }
                    }

                    return(TRUE);
                }

                case CONTROL_SETTINGS:
                {
                    return(TRUE);
                }

                case CONTROL_CANCEL:
                {
                    EndDialog(windowHandle, CONTROL_CANCEL);
                    return(TRUE);
                }
            }
            break;
        }

        case WM_CLOSE:
        {
            EndDialog(windowHandle, CONTROL_CANCEL);
            return(TRUE);
        }
    }

    return(FALSE);
}

static bool DirExists(const char* dirName)
{
    struct _stat s = {0};
    return(!_stat(dirName, &s) && ((s.st_mode & _S_IFMT) == _S_IFDIR));
}

int APIENTRY WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    InitCommonControls();
    Assert(!FAILED(CoInitialize(NULL)));

    Resources::Load();
    
    std::vector<String> rootDirs;
    
    if(DirExists("C:\\Music"))
    {
        rootDirs.push_back("C:\\Music\\Glen's Music");
        rootDirs.push_back("C:\\Music\\Erin's Music");
    }
    else if(DirExists("D:\\Music"))
    {
        rootDirs.push_back("D:\\Music\\Glen's Music");
        rootDirs.push_back("D:\\Music\\Erin's Music");
    }
    else if
    (
        DirExists("C:\\Glen's Music") &&
        DirExists("C:\\Erin's Music")
    )
    {
        rootDirs.push_back("C:\\Glen's Music");
        rootDirs.push_back("C:\\Erin's Music");
    }
    else if
    (
        DirExists("P:\\Glen's Music") &&
        DirExists("P:\\Erin's Music")
    )
    {
        rootDirs.push_back("P:\\Glen's Music");
        rootDirs.push_back("P:\\Erin's Music");
    }
    else
    {
        return(1);
    }
    
    IiTunes* iTunes = NULL;

    Assert(!FAILED(CoCreateInstance(CLSID_iTunesApp, NULL, CLSCTX_LOCAL_SERVER, IID_IiTunes, (PVOID *)&iTunes)));

    switch(DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(DIALOG_INTRO), FindBestParent(), IntroDialogProc))
    {
        case CONTROL_UPDATE_LIBRARY:
        {
            Library::Update(rootDirs, iTunes);
            break;
        }

        case CONTROL_REBUILD_LIBRARY:
        {
            Library::Rebuild(rootDirs, iTunes);
            break;
        }

        default:
        {
            break;
        }
    }
    
    if(iTunes)
    {
        iTunes->Quit();
        SafeRelease(iTunes);
    }

    Resources::Free();

    CoUninitialize();

    return(0);
}
