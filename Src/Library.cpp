#include "AristophanesPch.h"

#include "Library.h"

#include "DependencyGraph.h"
#include "StrayFiles.h"
#include "Resources.h"
#include "ID3LibEx.h"
#include "Tag.h"

#include <vector>

static HWND sDialogWindow = NULL;

const UINT_PTR DIALOG_UPDATE_TIMER = 1;
const UINT DIALOG_UPDATE_TIME = 200; // milliseconds

static int sDialogProgress = 0;
static String sDialogTask;

const UINT WM_USER_CLOSE = WM_USER + 1;

static INT_PTR CALLBACK DialogProc(HWND windowHandle, UINT msg, WPARAM wParam, LPARAM)
{
    sDialogWindow = windowHandle;
    
    switch(msg)
    {
        case WM_INITDIALOG:
        {
            RECT dialogRect, parentRect;
            INT x, y, dx, dy;

            BringWindowToTop(windowHandle);
            SetForegroundWindow(windowHandle);

            Assert(GetWindowRect(windowHandle, &dialogRect));
            Assert(SystemParametersInfo(SPI_GETWORKAREA, 0, &parentRect, 0));

            dx = dialogRect.right - dialogRect.left;
            dy = dialogRect.bottom - dialogRect.top;

            x = parentRect.left + ((parentRect.right - parentRect.left) - dx) / 2;
            y = parentRect.top + ((parentRect.bottom - parentRect.top) - dy) / 2;

            MoveWindow(windowHandle, x, y, dx, dy, FALSE);

            HWND progressBar = GetDlgItem(windowHandle, CONTROL_PROGRESS);
            Assert(progressBar);

            SendMessage(progressBar, PBM_SETRANGE, 0, MAKELPARAM(0, 100)); 
            SendMessage(progressBar, PBM_SETSTEP, 1, 0); 

            Assert(SetTimer(windowHandle, DIALOG_UPDATE_TIMER, DIALOG_UPDATE_TIME, NULL));
        
            SendMessage(windowHandle, WM_TIMER, DIALOG_UPDATE_TIMER, 0);
        
            SendMessage(windowHandle, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(Resources::gIcon));
            SendMessage(windowHandle, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(Resources::gIcon));

            return(FALSE);
        }
        
        case WM_COMMAND:
        {
            if(HIWORD(wParam) == BN_CLICKED)
            {
                if(LOWORD(wParam) == CONTROL_CANCEL)
                {
                    Assert(EndDialog(windowHandle, 0));
                    return(TRUE);   
                }
            }
            break;
        }
        
        case WM_CLOSE:
        {
            Assert(EndDialog(windowHandle, 0));
            return(TRUE);   
        }
        
        case WM_USER_CLOSE:
        {
            Assert(EndDialog(windowHandle, 1));
            return(TRUE);
        }
        
        case WM_DESTROY:
        {
            KillTimer(windowHandle, DIALOG_UPDATE_TIMER);
            return(TRUE);
        }

        case WM_TIMER:
        {
            if(wParam == DIALOG_UPDATE_TIMER)
            {
                int newDialogProgress = static_cast<int>(gProgress * 100.f);
                
                Assert(newDialogProgress >= 0);
                if(newDialogProgress > 100)
                {
                    newDialogProgress = 100;
                }
                
                if(sDialogProgress != newDialogProgress)
                {
                    sDialogProgress = newDialogProgress;
                
                    HWND progressBar = GetDlgItem(windowHandle, CONTROL_PROGRESS);
                    Assert(progressBar);

                    SendMessage(progressBar, PBM_SETPOS, static_cast<WPARAM>(newDialogProgress), 0); 
                }
                
                if(sDialogTask != gTask)
                {
                    sDialogTask = gTask;
  
                    Assert(SetDlgItemText(windowHandle, CONTROL_TASK, sDialogTask.c_str()));
                }
                
                return(TRUE);
            }
            
            return(FALSE);
        }
    }
    
    return(FALSE);
}

static DWORD WINAPI DialogThreadProc(LPVOID)
{
    if(!DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(DIALOG_PROGRESS), NULL, DialogProc))
    {
        sDialogWindow = NULL;
        gCancel = true;
        return(1);
    }

    sDialogWindow = NULL;
    return(0);
}

static void PurgeDirectory(const char* dir, float minProgress, float maxProgress, const char* extension = NULL)
{
    if(gCancel)
    {
        return;
    }
    
    size_t extensionLen = 0;
    
    if(extension)
    {
        extensionLen = Str::Len(extension);
    }
    
    char wildcard[MAX_PATH];
    
    size_t chars = Str::Copy(wildcard, ARRAY_COUNT(wildcard), dir);
    chars += Str::Copy(wildcard + chars, ARRAY_COUNT(wildcard) - chars, "\\*");
    
    WIN32_FIND_DATA findData;
    HANDLE findHandle;
    
    gProgress = minProgress;

    size_t fileCount = 0;

    // Count the number of files in the directory:

    findHandle = FindFirstFile(wildcard, &findData);

    if(findHandle == INVALID_HANDLE_VALUE)
    {
        gProgress = maxProgress;
        return;
    }
    
    do
    {
        if((findData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) || (findData.cFileName[0] == '.'))
        {
            continue;
        }

        ++fileCount;
    } while(FindNextFile(findHandle, &findData));
    
    Assert(FindClose(findHandle));
    
    if(fileCount == 0)
    {
        if(!extension)
        {
            Assert(RemoveDirectory(dir));
        }
        gProgress = maxProgress;
        return;
    }
    
    float progressPerFile = (maxProgress - minProgress) / static_cast<float>(fileCount);
    
    // Actually scan the files:
    
    findHandle = FindFirstFile(wildcard, &findData);

    if(findHandle == INVALID_HANDLE_VALUE)
    {
        return;
    }
    
    do
    {
        if((findData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) || (findData.cFileName[0] == '.'))
        {
            continue;
        }

        if(gCancel)
        {
            break;
        }
        
        if(extension && !(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            size_t nameLen = Str::Len(findData.cFileName);
            
            if(nameLen < extensionLen)
            {
                continue;
            }
            
            const char* nameExtension = findData.cFileName + (nameLen - extensionLen);
            
            if(Str::LooseCompare(nameExtension, extension))
            {
                continue;
            }
        }
    
        char fileName[MAX_PATH] = "";
        
        chars = Str::Copy(fileName, ARRAY_COUNT(fileName), dir);
        chars += Str::Copy(fileName + chars, ARRAY_COUNT(fileName) - chars, "\\");
        chars += Str::Copy(fileName + chars, ARRAY_COUNT(fileName) - chars, findData.cFileName);
    
        if(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            float subMinProgress = gProgress;
            float subMaxProgress = gProgress + progressPerFile;
            PurgeDirectory(fileName, subMinProgress, subMaxProgress, extension);
        }
        else
        {
            Assert(DeleteFile(fileName));
        }

        Util::Zeroize(&findData, sizeof(findData));
    } while(FindNextFile(findHandle, &findData));
    
    Assert(FindClose(findHandle));

    if(!extension)
    {
        Assert(RemoveDirectory(dir));
    }
    
    gProgress = maxProgress;
}

struct CleanLibrary
{
    void operator()(const DependencyGraph* dependencyGraph)
    {
        const char* rootDir = dependencyGraph->RootDir().c_str();
        char purgeDir[MAX_PATH] = "";

        gTask = "Purging synthetic files...";

        Str::Print(purgeDir, ARRAY_COUNT(purgeDir), "%s\\By Artist", rootDir);
        PurgeDirectory(purgeDir, 0.f, 0.2f);

        Str::Print(purgeDir, ARRAY_COUNT(purgeDir), "%s\\By Album", rootDir);
        PurgeDirectory(purgeDir, 0.2f, 0.4f);

        Str::Print(purgeDir, ARRAY_COUNT(purgeDir), "%s\\By Genre", rootDir);
        PurgeDirectory(purgeDir, 0.4f, 0.6f);

        Str::Print(purgeDir, ARRAY_COUNT(purgeDir), "%s\\By Label", rootDir);
        PurgeDirectory(purgeDir, 0.6f, 0.8f, ".m3u");

        Str::Print(purgeDir, ARRAY_COUNT(purgeDir), "%s\\Mixable", rootDir);
        PurgeDirectory(purgeDir, 0.8f, 1.f);
    }
};

struct Phase1
{
    void operator()(DependencyGraph* dependencyGraph)
    {
        dependencyGraph->ScanDirectories();
        
        StringArray strayFiles;
        dependencyGraph->GetStrayFiles(strayFiles);
        
        if(!strayFiles.empty())
        {
            ShowStrayFiles(strayFiles);
            gCancel = true;
        }
        
        StringArray missingFiles;
        dependencyGraph->LoadITunesDatabase(missingFiles);
        
        //if(!missingFiles.empty())
        //{
        //    ShowStrayFiles(missingFiles);
        //    gCancel = true;
        //}
        
        dependencyGraph->ConnectGraph();
    }
};

struct Phase2
{
    void operator()(DependencyGraph* dependencyGraph)
    {
        dependencyGraph->TagTracks();
        dependencyGraph->UpdateITunesDatabase();
        // dependencyGraph->ConnectLinkedPlaylists();
        // dependencyGraph->ConnectMergedPlaylists();
        // dependencyGraph->WriteLeafPlaylists();
        // dependencyGraph->MakeDirs();
        // dependencyGraph->MakeLinks();
        // dependencyGraph->WriteMergedPlaylists();
    }
};

typedef std::vector<DependencyGraph*> Graphs;

static void UpdateLibrary(const std::vector<String>& rootDirs, bool cleanFirst, IiTunes* iTunes)
{
    // Create a thread to manage the progress dialog:
    HANDLE dialogThread = CreateThread(NULL, 16 * 1024, DialogThreadProc, NULL, 0, NULL);
    Assert(dialogThread);
    Assert(SetThreadPriority(dialogThread, THREAD_PRIORITY_TIME_CRITICAL));
    
    Graphs graphs;
    graphs.reserve(rootDirs.size());

    for(std::vector<String>::const_iterator i = rootDirs.begin(); i != rootDirs.end(); ++i)
    {
        graphs.push_back(new DependencyGraph(*i, iTunes));
    }
    
    if(cleanFirst)
    {
        std::for_each(graphs.begin(), graphs.end(), CleanLibrary());
    }
    
    std::for_each(graphs.begin(), graphs.end(), Phase1());
    
    for(Graphs::iterator first = graphs.begin(); first != graphs.end(); ++first)
    {
        const DependencyGraph* firstGraph = *first;
        
        for(Graphs::iterator second = first + 1; second != graphs.end(); ++second)
        {
            DependencyGraph* secondGraph = *second;
        
            secondGraph->MarkDuplicates(*firstGraph);
        }
    }
            
    std::for_each(graphs.begin(), graphs.end(), Phase2());

    for(Graphs::iterator i = graphs.begin(); i != graphs.end(); ++i)
    {
        SafeDelete(*i);
    }
            
    if(sDialogWindow)
    {
        SendMessage(sDialogWindow, WM_USER_CLOSE, 0, 0);
    }

    Assert(WaitForSingleObject(dialogThread, INFINITE) == WAIT_OBJECT_0);
    SafeCloseHandle(dialogThread);
}

void Library::Update(const std::vector<String>& rootDirs, IiTunes* iTunes)
{
    UpdateLibrary(rootDirs, false, iTunes);
}

void Library::Rebuild(const std::vector<String>& rootDirs, IiTunes* iTunes)
{
    UpdateLibrary(rootDirs, true, iTunes);
}


