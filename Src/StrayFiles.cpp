#include "AristophanesPch.h"
#include "StrayFiles.h"

#include "Resources.h"

BOOL CALLBACK StrayFilesDialogProc(HWND windowHandle, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        case WM_INITDIALOG:
        {
            const StringArray& strayFiles = *reinterpret_cast<StringArray*>(lParam);
        
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

            HWND listHandle = GetDlgItem(windowHandle, CONTROL_STRAY_FILES);
            assert(listHandle);
            
            for(StringArray::const_iterator i = strayFiles.begin(); i != strayFiles.end(); ++i)
            {
                SendMessage(listHandle, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(i->c_str()));
            }

            SendMessage(windowHandle, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(Resources::gIcon));
            SendMessage(windowHandle, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(Resources::gIcon));

            return(TRUE);
        }

        case WM_COMMAND:
        {
            switch(LOWORD(wParam))
            {
                case CONTROL_OK:
                {
                    EndDialog(windowHandle, CONTROL_OK);
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

void ShowStrayFiles(const StringArray& strayFiles)
{
    DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(DIALOG_STRAY_FILES), FindBestParent(), StrayFilesDialogProc, reinterpret_cast<LPARAM>(&strayFiles));
}
