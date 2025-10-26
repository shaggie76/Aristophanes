#include "AristophanesPch.h"

#include "DependencyGraph.h"
#include "Tag.h"
#include "iTunes.h"

// define ENABLE_PLAYLISTS

// #define ENABLE_BY_ARTIST
// #define ENABLE_BY_ALBUM

#define SKIP_VINYL_LINKS

static void CompileRE(pcre*& re, pcre_extra*& ree, const String& pattern, int options = 0)
{
    const char *error;
    int errorOffset;
    re = pcre_compile(pattern.c_str(), options, &error, &errorOffset, NULL);
    Assert(re);
    
    ree = pcre_study(re, options, &error); 
}

DependencyGraph::DependencyGraph(const String& rootDir, IiTunes* iTunes) :
    mRootDir(rootDir),
    mDepth(0),
    mDirNodes(),
    mPlaylistNodes(),
    mTrackNodes(),
    mTrackLinkNodes(),
    mPlaylistLinkNodes(),
    mFileNodes(),
    mTracksByNameSize(),
    mPlaylistsByNameSize(),
    mUnMixedTrackRE(NULL),
    mUnMixedTrackREE(NULL),
    mMixedTrackRE(NULL),
    mMixedTrackREE(NULL),
    mNormalTrackRE(NULL),
    mNormalTrackREE(NULL),
    mPlaylistRE1(NULL),
    mPlaylistREE1(NULL),
    mPlaylistRE2(NULL),
    mPlaylistREE2(NULL),
    mTrackLinkRE(NULL),
    mTrackLinkREE(NULL),
    mPlaylistLinkRE(NULL),
    mPlaylistLinkREE(NULL),
    mPlaylistEntryRE(NULL),
    mPlaylistEntryREE(NULL),
    mITunes(iTunes),
    mITLibraryPlaylist(NULL),
    mITPlaylist(NULL),
    mITTrackCollection(NULL)
{
    String escapedRootDir = rootDir;
    
    for(size_t i = 0; i < escapedRootDir.length(); ++i)
    {
        if(escapedRootDir[i] == '\\')
        {
            escapedRootDir.insert(i, 1, '\\');
            ++i;
        }
    }
    
    // UnMixed TrackNode: M:\By Label\Eye-Q Records\Various\1995 - Unmixed - Behind The Eye (Volume 2) [CD]\01 - Virtual Symmetry - The VS.mp3
    // <Label> <Year> <Album Title> <format> <track index> <track artist> <track title>
    CompileRE(mUnMixedTrackRE, mUnMixedTrackREE,  "^" + escapedRootDir + "\\\\By Label\\\\([^\\\\]*)\\\\Various\\\\(\\d{4}) - Unmixed - (.*) \\[(CD|DVD|Bluray|Vinyl|MP3)\\]\\\\(\\d{2,3}) - (.*) - (.*)\\.mp3$");
    
    // Mixed TrackNode: M:\By Label\United Records\Various\2000 - Armin van Buuren - 001 - A State of Trance [CD]\01 - Miller & Floyd - Colours (Humate Remix).mp3
    // <Label> <Year> <DJ> <Album Title> <format> <track index> <track artist> <track title>
    CompileRE(mMixedTrackRE, mMixedTrackREE,  "^" + escapedRootDir + "\\\\By Label\\\\([^\\\\]*)\\\\Various\\\\(\\d{4}) - (.*) - (.*) \\[(CD|DVD|Bluray|Vinyl|MP3)\\]\\\\(\\d{2,3}) - (.*) - (.*)\\.mp3$");

    // Normal TrackNode: M:\By Label\A&M Records\DJ Shadow\2002 - The Private Press [CD]\01 - (Letter From Home).mp3
    // <Label> <Year> <Artist> <Album Title> <format> <track index> <track title>
    CompileRE(mNormalTrackRE, mNormalTrackREE, "^" + escapedRootDir + "\\\\By Label\\\\([^\\\\]*)\\\\([^\\\\]*)\\\\(\\d{4}) - (.*) \\[(CD|DVD|Bluray|Vinyl|MP3)\\]\\\\(\\d{2,3}) - (.*)\\.mp3$");

    // Playlist: M:\By Label\A&M Records\DJ Shadow\2002 - The Private Press [CD]\00 - DJ Shadow - The Private Press [2002].m3u
    //           M:\By Label\A&M Records\DJ Shadow\All.m3u
    //           M:\By Label\A&M Records\DJ Shadow\CD.m3u
    //           M:\By Label\A&M Records\DJ Shadow\Vinyl.m3u
    //           M:\By Artist\All.m3u
    //           M:\By Label\CD.m3u
    //           M:\By Genre\Vinyl.m3u
    
    CompileRE(mPlaylistRE1, mPlaylistREE1, "^" + rootDir + "\\\\By Label\\\\.*\\[(CD|DVD|Bluray|Vinyl|MP3)\\]\\\\00 - .*\\.m3u$");
    CompileRE(mPlaylistRE2, mPlaylistREE2, ".*\\\\(CD|DVD|Bluray|Vinyl|MP3|All).m3u$");
    
    // TrackNode Link: M:\By Artist\ADSR\1994 - Primary\01 - Windswept.mp3
    CompileRE(mTrackLinkRE, mTrackLinkREE, "^" + rootDir + "\\\\By (Artist|Album|Genre)\\\\.*\\.mp3$");
    
    // Playlist Link : M:\By Artist\ADSR\1994 - Primary\00 - ADSR - Primary [1994].m3u
    CompileRE(mPlaylistLinkRE, mPlaylistLinkREE, ".*\\.m3u$");

    // #EXTINF:69,DJ Shadow - (Letter From Home) 
    // 01 - (Letter From Home).mp3
    // <duration> <fileName>
    CompileRE(mPlaylistEntryRE, mPlaylistEntryREE, "^#EXTINF:([0-9]+),[^\n]+\n([^\n]+)\n", PCRE_MULTILINE);
}

template<typename T>
inline void SafePcreFree(T*& p)
{
    if(p)
    {
        free(p);
        p = 0;
    }
}

DependencyGraph::~DependencyGraph()
{
    mDirNodes.clear();
    mPlaylistNodes.clear();
    mTrackNodes.clear();
    mTrackLinkNodes.clear();
    mPlaylistLinkNodes.clear();
    mFileNodes.clear();
    mTracksByNameSize.clear();
    mPlaylistsByNameSize.clear();
    
    SafePcreFree(mUnMixedTrackRE);
    SafePcreFree(mUnMixedTrackREE);
    SafePcreFree(mMixedTrackRE);
    SafePcreFree(mMixedTrackREE);
    SafePcreFree(mNormalTrackRE);
    SafePcreFree(mNormalTrackREE);
    SafePcreFree(mPlaylistRE1);
    SafePcreFree(mPlaylistREE1);
    SafePcreFree(mPlaylistRE2);
    SafePcreFree(mPlaylistREE2);
    SafePcreFree(mTrackLinkRE);
    SafePcreFree(mTrackLinkREE);
    SafePcreFree(mPlaylistLinkRE);
    SafePcreFree(mPlaylistLinkREE);
    SafePcreFree(mPlaylistEntryRE);
    SafePcreFree(mPlaylistEntryREE);
    
    SafeRelease(mITTrackCollection);
    SafeRelease(mITPlaylist);
    SafeRelease(mITLibraryPlaylist);
    
    mITunes = NULL;
}

struct ScanThreadArgs
{
    String      mRootDir;
    HANDLE      mWritePipe;
};

struct ScanFile
{
    char        mName[MAX_PATH];
    size_t      mNameLen;
    bool        mIsDir;
    FILETIME    mTime;
    QWORD       mSize;
};

static void ScanDirectory(const char* dir, HANDLE writePipe, float minProgress, float maxProgress)
{
    if(gCancel)
    {
        return;
    }
    
    char wildCard[MAX_PATH];
    
    size_t chars = Str::Copy(wildCard, ARRAY_COUNT(wildCard), dir);
    Str::Copy(wildCard + chars, ARRAY_COUNT(wildCard) - chars, "\\*");

    WIN32_FIND_DATA findData;
    HANDLE findHandle;
    
    gProgress = minProgress;

    size_t fileCount = 0;

    // Count the number of files in the directory:

    findHandle = FindFirstFile(wildCard, &findData);

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
        gProgress = maxProgress;
        return;
    }
    
    float progressPerFile = (maxProgress - minProgress) / static_cast<float>(fileCount);
    
    // Actually scan the files:
    
    findHandle = FindFirstFile(wildCard, &findData);

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
    
        ScanFile scanFile;
        
        chars = Str::Copy(scanFile.mName, ARRAY_COUNT(scanFile.mName), dir);
        chars += Str::Copy(scanFile.mName + chars, ARRAY_COUNT(scanFile.mName) - chars, "\\");
        chars += Str::Copy(scanFile.mName + chars, ARRAY_COUNT(scanFile.mName) - chars, findData.cFileName);
    
        scanFile.mNameLen = chars;
    
        scanFile.mIsDir = ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);
        scanFile.mTime = findData.ftLastWriteTime;
        scanFile.mSize = (static_cast<QWORD>(findData.nFileSizeHigh) << 32) | static_cast<QWORD>(findData.nFileSizeLow);

        DWORD bytesWritten = 0;
        Assert(WriteFile(writePipe, &scanFile, sizeof(scanFile), &bytesWritten, NULL));
        Assert(bytesWritten == sizeof(scanFile));

        if(scanFile.mIsDir)
        {
            float subMinProgress = gProgress;
            float subMaxProgress = gProgress + progressPerFile;
            ScanDirectory(scanFile.mName, writePipe, subMinProgress, subMaxProgress);
        }
        else
        {
            gProgress += progressPerFile;
        }
        Util::Zeroize(&findData, sizeof(findData));
    } while(FindNextFile(findHandle, &findData));
    
    Assert(FindClose(findHandle));
    
    gProgress = maxProgress;
}

static DWORD WINAPI ScanThreadProc(LPVOID voidArgs)
{
    ScanThreadArgs* args = reinterpret_cast<ScanThreadArgs*>(voidArgs);
    
    String byLabel = args->mRootDir + "\\By Label";
    ScanDirectory(byLabel.c_str(), args->mWritePipe, 0.f, 0.25f);

#ifdef ENABLE_BY_ARTIST
    String byArtist = args->mRootDir + "\\By Artist";
    ScanDirectory(byArtist.c_str(), args->mWritePipe, 0.25f, 0.5f);
#endif

#ifdef ENABLE_BY_ALBUM
    String byAlbum = args->mRootDir + "\\By Album";
    ScanDirectory(byAlbum.c_str(), args->mWritePipe, 0.5f, 0.75f);
#endif

    String byGenre = args->mRootDir + "\\By Genre";
    ScanDirectory(byGenre.c_str(), args->mWritePipe, 0.75f, 1.f);
    
    SafeCloseHandle(args->mWritePipe);
    
    return(0);
}

void DependencyGraph::ScanDirectories()
{
    gTask = "Scanning directories...";
    gProgress = 0.f;

    HANDLE readPipe = NULL;
    HANDLE writePipe = NULL;
        
    Assert(CreatePipe(&readPipe, &writePipe, NULL, 1024 * sizeof(ScanFile)));
    Assert(readPipe);
    Assert(writePipe);
    
    ScanThreadArgs scanThreadArgs;
    scanThreadArgs.mRootDir = mRootDir;
    scanThreadArgs.mWritePipe = writePipe;

    HANDLE scanThread = CreateThread(NULL, 64 * sizeof(ScanFile), ScanThreadProc, &scanThreadArgs, 0, NULL);
    Assert(scanThread);

    ScanFile scanFile;
    DWORD bytesRead = 0;
    
    while(ReadFile(readPipe, &scanFile, sizeof(scanFile), &bytesRead, NULL))
    {
        Assert(bytesRead == sizeof(scanFile));
        ParseFile(scanFile);
    }
    
    SafeCloseHandle(readPipe);
    Assert(WaitForSingleObject(scanThread, INFINITE) == WAIT_OBJECT_0);
    SafeCloseHandle(scanThread);
}

void DependencyGraph::LoadITunesDatabase(StringArray& missingFiles)
{
    Assert(!FAILED(mITunes->get_LibraryPlaylist(&mITLibraryPlaylist)));
    Assert(!FAILED(mITLibraryPlaylist->QueryInterface(IID_IITPlaylist, reinterpret_cast<LPVOID*>(&mITPlaylist))));
    Assert(!FAILED(mITPlaylist->get_Tracks(&mITTrackCollection)));

    long trackCount = 0;    
    Assert(!FAILED(mITTrackCollection->get_Count(&trackCount)));
    
    gProgress = 0.f;
    gTask = "Loading iTunes database...";

    if(!trackCount)
    {
        gProgress = 1.f;
        return;
    }
    
    float progressPerFile = 1.f / static_cast<float>(trackCount);

    for(long i = 1; i <= trackCount; ++i)
    {
        if(gCancel)
        {
            return;
        }

        gProgress += progressPerFile;

        IITFileOrCDTrack* track = NULL;

        {
            IITTrack* trackBase = NULL;
            Assert(!FAILED(mITTrackCollection->get_Item(i, &trackBase)));
            
            bool isFile = (trackBase->QueryInterface(IID_IITFileOrCDTrack, reinterpret_cast<LPVOID*>(&track)) == S_OK);
            
            SafeRelease(trackBase);
            
            if(!isFile || !track)
            {
                continue;        
            }
        }
        
        Assert(track);
                
        BSTR uniLocation = NULL;
        
        if(FAILED(track->get_Location(&uniLocation)) || !uniLocation)
        {
            SafeRelease(track);
            continue;
        }

        char location[MAX_PATH] = "";
        Str::UnicodeToAnsi(location, ARRAY_COUNT(location), uniLocation);
        SysFreeString(uniLocation);
        
        // Find the corresponding TrackNode:
        
        TrackNode node(location);
        TrackNodeSet::iterator f = mTrackNodes.find(node);
        
        if(f == mTrackNodes.end())
        {
            missingFiles.push_back(location);
            SafeRelease(track);
            continue;
        }
        
        const TrackNode& trackNode = *f;
        trackNode.mIITTrack = track;
    }
    
    gProgress = 1.f;
}

void DependencyGraph::ConnectGraph()
{
    for(TrackNodeSet::iterator i = mTrackNodes.begin(); i != mTrackNodes.end(); ++i)
    {
        if(gCancel)
        {
            return;
        }

        const TrackNode& track = *i;

        AddPlaylistConnection(track);

#ifdef  SKIP_VINYL_LINKS
        if(track.mOriginalFormat == OF_VINYL)
        {
            continue; 
        }
#endif

#ifdef ENABLE_BY_ARTIST
        AddArtistConnections(track);
#endif

#ifdef ENABLE_BY_ALBUM
        AddAlbumConnections(track);
#endif
    }
    
    /*
    for(TrackLinkNodeSet::iterator i = mTrackLinkNodes.begin(); i != mTrackLinkNodes.end(); ++i)
    {
        if(gCancel)
        {
            return;
        }
    
        const TrackLinkNode& link = *i;
        
        if(link.mSource)
        {
            continue;
        }
    
        FileSetByNameSize::iterator f = mTracksByNameSize.find(&link);
        Assert(f != mTracksByNameSize.end());
        
        link.mSource = static_cast<const TrackNode*>(*f);
    }

    for(PlaylistLinkNodeSet::iterator i = mPlaylistLinkNodes.begin(); i != mPlaylistLinkNodes.end(); ++i)
    {
        if(gCancel)
        {
            return;
        }
    
        const PlaylistLinkNode& link = *i;
        
        if(link.mSource)
        {
            continue;
        }
    
        FileSetByNameSize::iterator f = mPlaylistsByNameSize.find(&link);
        Assert(f != mPlaylistsByNameSize.end());
        
        link.mSource = static_cast<const PlaylistNode*>(*f);
    }
    */
}

void DependencyGraph::TagTracks()
{
    gProgress = 0.f;
    gTask = "";

    size_t dirtyTracks = 0;

    for(TrackNodeSet::iterator i = mTrackNodes.begin(); i != mTrackNodes.end(); ++i)
    {
        if(gCancel)
        {
            return;
        }
    
        const TrackNode& track = *i;

        Assert(track.mPlaylist);
        const PlaylistNode& playlist = *(track.mPlaylist);
    
        if(IsZero(playlist.mTime) || (CompareFileTime(&playlist.mTime, &track.mTime) < 0))
        {
            ++dirtyTracks;
        }
    }

    float progressPerTrack = 1.f / static_cast<float>(dirtyTracks);

    FILETIME newTime;    
    GetSystemTimeAsFileTime(&newTime);
    
    for(TrackNodeSet::iterator i = mTrackNodes.begin(); i != mTrackNodes.end(); ++i)
    {
        if(gCancel)
        {
            return;
        }

        const TrackNode& track = *i;

        Assert(track.mPlaylist);
        const PlaylistNode& playlist = *(track.mPlaylist);
    
        if(!IsZero(playlist.mTime) && (CompareFileTime(&playlist.mTime, &track.mTime) >= 0))
        {
            continue;
        }
        
        gTask = String("Tagging ") + track.mName;
        
        LoadITunes(track);
        UpdateTag(track);
        
        track.mTime = newTime;
        
        // Now that we know the track's genres we should make the appropriate links:
#ifdef SKIP_VINYL_LINKS
        if(track.mOriginalFormat != OF_VINYL)
        {
            AddGenreConnections(track);
        }
#endif

        gProgress += progressPerTrack;
    }

    gProgress = 1.f;
}

static inline BSTR MakeBStr(const char* string)
{
    OLECHAR oleChar[MAX_PATH];
    
    Str::AnsiToUnicode(oleChar, ARRAY_COUNT(oleChar), string);
    
    BSTR b = SysAllocString(oleChar);
    return(b);
}

void DependencyGraph::UpdateITunesDatabase()
{
    gProgress = 0.f;
    gTask = "";

    size_t dirtyTracks = 0;

    for(TrackNodeSet::iterator i = mTrackNodes.begin(); i != mTrackNodes.end(); ++i)
    {
        if(gCancel)
        {
            return;
        }
    
        const TrackNode& track = *i;

        Assert(track.mPlaylist);
        const PlaylistNode& playlist = *(track.mPlaylist);
    
        if(IsZero(playlist.mTime) || (CompareFileTime(&playlist.mTime, &track.mTime) < 0))
        {
            ++dirtyTracks;
        }
    }

    float progressPerTrack = 1.f / static_cast<float>(dirtyTracks);

    FILETIME newTime;    
    GetSystemTimeAsFileTime(&newTime);
    
    for(TrackNodeSet::iterator i = mTrackNodes.begin(); i != mTrackNodes.end(); ++i)
    {
        if(gCancel)
        {
            return;
        }

        const TrackNode& track = *i;

        Assert(track.mPlaylist);
        const PlaylistNode& playlist = *(track.mPlaylist);
    
        if(!IsZero(playlist.mTime) && (CompareFileTime(&playlist.mTime, &track.mTime) >= 0))
        {
            continue;
        }
        
        gTask = String("Updating iTunes ") + track.mName;
        
        UpdateITunes(track);
        
        gProgress += progressPerTrack;
    }

    gProgress = 1.f;
}

void DependencyGraph::ConnectLinkedPlaylists()
{
    for(TrackLinkNodeSet::iterator i = mTrackLinkNodes.begin(); i != mTrackLinkNodes.end(); ++i)
    {
        if(gCancel)
        {
            return;
        }

        const TrackLinkNode& link = *i;
        
        if(link.mSkipAlbumPlaylist)
        {
            continue;
        }
        
        Assert(link.mSource);
        Assert(link.mSource->mPlaylist);

        char linkPlaylistName[MAX_PATH];
        size_t chars = Str::DirName(linkPlaylistName, ARRAY_COUNT(linkPlaylistName), link.mName);
        chars += Str::Copy(linkPlaylistName + chars, ARRAY_COUNT(linkPlaylistName) - chars, "\\");
        Str::FileName(linkPlaylistName + chars, ARRAY_COUNT(linkPlaylistName) - chars, link.mSource->mPlaylist->mName);

        const PlaylistLinkNode& linkPlaylist = GetPlaylistLink(linkPlaylistName);

        linkPlaylist.mSource = link.mSource->mPlaylist;
        linkPlaylist.mOriginalFormat = link.mSource->mPlaylist->mOriginalFormat;
        linkPlaylist.mTrackNodes.push_back(&link);
    }
}

void DependencyGraph::ConnectMergedPlaylists()
{
#ifdef ENABLE_MERGED_PLAYLISTS
    for(size_t curDepth = mDepth; curDepth > 0; --curDepth)
    {
        for(PlaylistNodeSet::iterator i = mPlaylistNodes.begin(); i != mPlaylistNodes.end(); ++i)
        {
            if(gCancel)
            {
                return;
            }

            const PlaylistNode& playlist = *i;
            
            if(playlist.mDepth != curDepth)
            {
                continue;
            }
            
            AddPlaylistConnections(playlist);
        }

        for(PlaylistLinkNodeSet::iterator i = mPlaylistLinkNodes.begin(); i != mPlaylistLinkNodes.end(); ++i)
        {
            if(gCancel)
            {
                return;
            }

            const PlaylistNode& playlist = *i;
            
            if(playlist.mDepth != curDepth)
            {
                continue;
            }
            
            AddPlaylistConnections(playlist);
        }
    }
#endif // ENABLE_MERGED_PLAYLISTS
}

void DependencyGraph::WriteLeafPlaylists()
{
    gProgress = 0.f;
    gTask = "";
    
    size_t dirtyPlaylists = 0;

    for(PlaylistNodeSet::iterator i = mPlaylistNodes.begin(); i != mPlaylistNodes.end(); ++i)
    {
        if(gCancel)
        {
            return;
        }

        const PlaylistNode& playlist = *i;
        
        if(playlist.mLeafNode && playlist.IsDirty())
        {
            ++dirtyPlaylists;
        }
    }

    float progressPerPlaylist = 1.f / static_cast<float>(dirtyPlaylists);
    
    FILETIME newTime;    
    GetSystemTimeAsFileTime(&newTime);

    for(PlaylistNodeSet::iterator i = mPlaylistNodes.begin(); i != mPlaylistNodes.end(); ++i)
    {
        if(gCancel)
        {
            return;
        }

        const PlaylistNode& playlist = *i;
        
        if(!playlist.mLeafNode || !playlist.IsDirty())
        {
            continue;
        }
        
#ifdef ENABLE_PLAYLISTS        
        gTask = String("Creating ") + playlist.mName;
        WritePlaylist(playlist);
#endif

        playlist.mTime = newTime;

        gProgress += progressPerPlaylist;
    }

    gProgress = 1.f;
}

void DependencyGraph::MakeDirs()
{
    gProgress = 0.f;
    gTask = "";
    
    size_t dirtyDirs = 0;

    for(DirNodeSet::iterator i = mDirNodes.begin(); i != mDirNodes.end(); ++i)
    {
        if(gCancel)
        {
            return;
        }

        const DirNode& dir = *i;

        if(!dir.mTime.dwLowDateTime && !dir.mTime.dwHighDateTime)
        {
            ++dirtyDirs;
        }
    }
    
    float progressPerDir = 1.f / static_cast<float>(dirtyDirs);
    
    FILETIME newTime;    
    GetSystemTimeAsFileTime(&newTime);

    for(size_t curDepth = 0; curDepth <= mDepth; ++curDepth)
    {
        for(DirNodeSet::iterator i = mDirNodes.begin(); i != mDirNodes.end(); ++i)
        {
            if(gCancel)
            {
                return;
            }
            
            const DirNode& dir = *i;
            
            if(dir.mDepth != curDepth)
            {
                continue;
            }

            if(dir.mTime.dwLowDateTime || dir.mTime.dwHighDateTime)
            {
                continue;
            }

            gTask = String("Creating ") + dir.mName;

            Assert(CreateDirectory(dir.mName, NULL));
            dir.mTime = newTime;
            
            gProgress += progressPerDir;
        }
    }
        
    gProgress = 1.f;
}

void DependencyGraph::MakeLinks()
{
    gProgress = 0.f;
    
    size_t dirtyLinks = 0;
    
    for(TrackLinkNodeSet::iterator i = mTrackLinkNodes.begin(); i != mTrackLinkNodes.end(); ++i)
    {
        if(gCancel)
        {
            return;
        }

        const TrackLinkNode& link = *i;
        
        if(!link.mTime.dwLowDateTime && !link.mTime.dwHighDateTime && link.mSource)
        {
            ++dirtyLinks;
        }
    }

#ifdef ENABLE_PLAYLISTS
    for(PlaylistLinkNodeSet::iterator i = mPlaylistLinkNodes.begin(); i != mPlaylistLinkNodes.end(); ++i)
    {
        if(gCancel)
        {
            return;
        }

        const PlaylistLinkNode& link = *i;

        if(link.mTime.dwLowDateTime || link.mTime.dwHighDateTime || !link.mSource)
        {
            continue;
        }
        
        ++dirtyLinks;
    }
#endif

    if(!dirtyLinks)
    {
        gProgress = 1.f;
        return;
    }
    
    float progressPerLink = 1.f / static_cast<float>(dirtyLinks);
    
    FILETIME newTime;    
    GetSystemTimeAsFileTime(&newTime);

    for(TrackLinkNodeSet::iterator i = mTrackLinkNodes.begin(); i != mTrackLinkNodes.end(); ++i)
    {
        if(gCancel)
        {
            return;
        }
        
        const TrackLinkNode& link = *i;
        
        if(link.mTime.dwLowDateTime || link.mTime.dwHighDateTime || !link.mSource)
        {
            continue;
        }

        gTask = String("Creating ") + link.mName;
        
        Assert(CreateHardLink(link.mName, link.mSource->mName, NULL));

        link.mTime = newTime;
        
        gProgress += progressPerLink;
    }

#ifdef ENABLE_PLAYLISTS
    for(PlaylistLinkNodeSet::iterator i = mPlaylistLinkNodes.begin(); i != mPlaylistLinkNodes.end(); ++i)
    {
        if(gCancel)
        {
            return;
        }
        
        const PlaylistLinkNode& link = *i;
        
        if(link.mTime.dwLowDateTime || link.mTime.dwHighDateTime || !link.mSource)
        {
            continue;
        }

        gTask = String("Creating ") + link.mName;
        
        Assert(CreateHardLink(link.mName, link.mSource->mName, NULL));

        link.mTime = newTime;

        gProgress += progressPerLink;
    }
#endif

    gProgress = 1.f;
}

void DependencyGraph::WriteMergedPlaylists()
{
    gProgress = 0.f;
    gTask = "";
    
    size_t dirtyPlaylists = 0;

    for(PlaylistNodeSet::iterator i = mPlaylistNodes.begin(); i != mPlaylistNodes.end(); ++i)
    {
        if(gCancel)
        {
            return;
        }

        const PlaylistNode& playlist = *i;
        
        if(!playlist.mLeafNode && playlist.IsDirty())
        {
            ++dirtyPlaylists;
        }
    }

    float progressPerPlaylist = 1.f / static_cast<float>(dirtyPlaylists);
    
    FILETIME newTime;    
    GetSystemTimeAsFileTime(&newTime);

    for(size_t curDepth = mDepth; curDepth > 0; --curDepth)
    {
        for(PlaylistNodeSet::iterator i = mPlaylistNodes.begin(); i != mPlaylistNodes.end(); ++i)
        {
            if(gCancel)
            {
                return;
            }

            const PlaylistNode& playlist = *i;
            
            if(playlist.mDepth != curDepth)
            {
                continue;
            }
            
            if(playlist.mLeafNode || !playlist.IsDirty())
            {
                continue;
            }

#ifdef ENABLE_PLAYLISTS        
            gTask = String("Creating ") + playlist.mName;
            WritePlaylist(playlist);
#endif

            playlist.mTime = newTime;

            gProgress += progressPerPlaylist;
        }
    }

    gProgress = 1.f;
}

void DependencyGraph::GetStrayFiles(StringArray& strayFiles) const
{
    strayFiles.resize(0);
    
    if(mFileNodes.empty())
    {
        return;
    }

    strayFiles.reserve(mFileNodes.size());
    
    for(FileNodeSet::const_iterator i = mFileNodes.begin(); i != mFileNodes.end(); ++i)
    {
        strayFiles.resize(strayFiles.size() + 1);
        strayFiles.back() = i->mName;
    }
}

void DependencyGraph::MarkDuplicates(const DependencyGraph& other)
{
    gProgress = 0.f;
    gTask = "Isolating Duplicates";
    float progressPerTrack = 1.f / static_cast<float>(other.mTrackNodes.size());

    for(TrackNodeSet::const_iterator i = other.mTrackNodes.begin(); i != other.mTrackNodes.end(); ++i)
    {
        const TrackNode& otherTrack = *i;
        MarkDuplicates(otherTrack);
        gProgress += progressPerTrack;
    }
    
    gProgress = 1.f;
}

static bool TracksAreDuplicate(const TrackNode& a, const TrackNode& b)
{
    return
    (
        (a.mAlbumYear == b.mAlbumYear) &&
        (a.mSequenceNumber == b.mSequenceNumber) &&
        (!Str::Compare(a.mTrackName, b.mTrackName)) &&
        (!Str::Compare(a.mTrackArtist, b.mTrackArtist)) &&
        (!Str::Compare(a.mAlbumTitle, b.mAlbumTitle)) &&
        (!Str::Compare(a.mAlbumArtist, b.mAlbumArtist))
    );
}

void DependencyGraph::MarkDuplicates(const TrackNode& otherTrack)
{
    for(TrackNodeSet::iterator i = mTrackNodes.begin(); i != mTrackNodes.end(); ++i)
    {
        const TrackNode& track = *i;

        if(TracksAreDuplicate(otherTrack, track))
        {
            track.mRedundantTrack = true;
            track.mIITTrack = otherTrack.mIITTrack;
            
            if(track.mIITTrack)
            {
                track.mIITTrack->AddRef();
            }
            
            return; // Assume only one duplicate per collection.
        }
    }
}

void DependencyGraph::AddPlaylistConnection(const TrackNode& track)
{
    char playlistName[MAX_PATH];
    
    size_t chars = Str::DirName(playlistName, ARRAY_COUNT(playlistName), track.mName);
    Str::Print(playlistName + chars, ARRAY_COUNT(playlistName) - chars, "\\00 - %s - %s [%d].m3u", track.mAlbumArtist, track.mAlbumTitle, track.mAlbumYear);
    
    const PlaylistNode& playlist = GetPlaylist(playlistName, track.mOriginalFormat);
    playlist.mTrackNodes.push_back(&track);
    playlist.mLeafNode = true;
    
    track.mPlaylist = &playlist;
}

void DependencyGraph::AddArtistConnections(const TrackNode& track)
{
    char byArtistDir[MAX_PATH];
    Str::Print(byArtistDir, ARRAY_COUNT(byArtistDir), "%s\\By Artist\\%s\\%d - %s", mRootDir.c_str(), track.mAlbumArtist, track.mAlbumYear, track.mAlbumTitle);

    MakeDirNodes(byArtistDir);

    char byArtistTrackName[MAX_PATH];
    size_t chars = Str::Copy(byArtistTrackName, ARRAY_COUNT(byArtistTrackName), byArtistDir);
    chars += Str::Copy(byArtistTrackName + chars, ARRAY_COUNT(byArtistTrackName) - chars, "\\");
    Str::FileName(byArtistTrackName + chars, ARRAY_COUNT(byArtistTrackName) - chars, track.mName);
    
    const TrackLinkNode& byArtistTrackLink = GetTrackLink(byArtistTrackName);
    byArtistTrackLink.mSource = &track;

    Assert(track.mPlaylist);

    char byArtistPlaylistName[MAX_PATH];
    chars = Str::Copy(byArtistPlaylistName, ARRAY_COUNT(byArtistPlaylistName), byArtistDir);
    chars += Str::Copy(byArtistPlaylistName + chars, ARRAY_COUNT(byArtistPlaylistName) - chars, "\\");
    Str::FileName(byArtistPlaylistName + chars, ARRAY_COUNT(byArtistPlaylistName) - chars, track.mPlaylist->mName);

    const PlaylistLinkNode& byArtistPlaylistLink = GetPlaylistLink(byArtistPlaylistName);
    byArtistPlaylistLink.mSource = track.mPlaylist;
}

void DependencyGraph::AddAlbumConnections(const TrackNode& track)
{
    char byAlbumDir[MAX_PATH];
    Str::Print(byAlbumDir, ARRAY_COUNT(byAlbumDir), "%s\\By Album\\%s - %s [%d]", mRootDir.c_str(), track.mAlbumArtist, track.mAlbumTitle, track.mAlbumYear);
    MakeDirNodes(byAlbumDir);

    char byAlbumTrackName[MAX_PATH];
    size_t chars = Str::Copy(byAlbumTrackName, ARRAY_COUNT(byAlbumTrackName), byAlbumDir);
    chars += Str::Copy(byAlbumTrackName + chars, ARRAY_COUNT(byAlbumTrackName) - chars, "\\");
    Str::FileName(byAlbumTrackName + chars, ARRAY_COUNT(byAlbumTrackName) - chars, track.mName);
    
    const TrackLinkNode& byAlbumTrackLink = GetTrackLink(byAlbumTrackName);
    byAlbumTrackLink.mSource = &track;

    Assert(track.mPlaylist);

    char byAlbumPlaylistName[MAX_PATH];
    chars = Str::Copy(byAlbumPlaylistName, ARRAY_COUNT(byAlbumPlaylistName), byAlbumDir);
    chars += Str::Copy(byAlbumPlaylistName + chars, ARRAY_COUNT(byAlbumPlaylistName) - chars, "\\");
    Str::FileName(byAlbumPlaylistName + chars, ARRAY_COUNT(byAlbumPlaylistName) - chars, track.mPlaylist->mName);

    const PlaylistLinkNode& byAlbumPlaylistLink = GetPlaylistLink(byAlbumPlaylistName);
    byAlbumPlaylistLink.mSource = track.mPlaylist;
}

void DependencyGraph::AddGenreConnections(const TrackNode& track)
{
    if(!track.mGenre[0])
    {
        return;
    }

    char byGenreDir[MAX_PATH];
    Str::Print(byGenreDir, ARRAY_COUNT(byGenreDir), "%s\\By Genre\\%s\\%s - %s [%d]", mRootDir.c_str(), track.mGenre, track.mAlbumArtist, track.mAlbumTitle, track.mAlbumYear);
    MakeDirNodes(byGenreDir);
    
    for(size_t i = 0; byGenreDir[i]; ++i)
    {
        if(byGenreDir[i] == '/')
        {
            byGenreDir[i] = '\\';
        }
    }

    char byGenreTrackName[MAX_PATH];
    size_t chars = Str::Copy(byGenreTrackName, ARRAY_COUNT(byGenreTrackName), byGenreDir);
    chars += Str::Copy(byGenreTrackName + chars, ARRAY_COUNT(byGenreTrackName) - chars, "\\");
    Str::FileName(byGenreTrackName + chars, ARRAY_COUNT(byGenreTrackName) - chars, track.mName);
    
    const TrackLinkNode& byGenreTrackLink = GetTrackLink(byGenreTrackName);
    byGenreTrackLink.mSource = &track;

    Assert(track.mPlaylist);

    char byGenrePlaylistName[MAX_PATH];
    chars = Str::Copy(byGenrePlaylistName, ARRAY_COUNT(byGenrePlaylistName), byGenreDir);
    chars += Str::Copy(byGenrePlaylistName + chars, ARRAY_COUNT(byGenrePlaylistName) - chars, "\\");
    Str::FileName(byGenrePlaylistName + chars, ARRAY_COUNT(byGenrePlaylistName) - chars, track.mPlaylist->mName);

    const PlaylistLinkNode& byGenrePlaylistLink = GetPlaylistLink(byGenrePlaylistName);
    byGenrePlaylistLink.mSource = track.mPlaylist;
}

void DependencyGraph::AddPlaylistConnections(const PlaylistNode& playlist)
{
    size_t chars;

    char playlistDir[MAX_PATH];
    char parentDir[MAX_PATH];
    Str::DirName(playlistDir, ARRAY_COUNT(playlistDir), playlist.mName);
    Str::DirName(parentDir, ARRAY_COUNT(parentDir), playlistDir);
    
    if(parentDir == mRootDir)
    {
        return;
    }

    if(playlist.mOriginalFormat == OF_CD)
    {
        char parentCdName[MAX_PATH];
        chars = Str::Copy(parentCdName, ARRAY_COUNT(parentCdName), parentDir);
        Str::Copy(parentCdName + chars, ARRAY_COUNT(parentCdName) - chars, "\\CD.m3u");
        
        const PlaylistNode& parentCd = GetPlaylist(parentCdName, playlist.mOriginalFormat);
        parentCd.mTrackNodes.insert(parentCd.mTrackNodes.end(), playlist.mTrackNodes.begin(), playlist.mTrackNodes.end());
    }
    else if(playlist.mOriginalFormat == OF_VINYL)
    {
        char parentVinylName[MAX_PATH];
        chars = Str::Copy(parentVinylName, ARRAY_COUNT(parentVinylName), parentDir);
        Str::Copy(parentVinylName + chars, ARRAY_COUNT(parentVinylName) - chars, "\\Vinyl.m3u");
        
        const PlaylistNode& parentVinyl = GetPlaylist(parentVinylName, playlist.mOriginalFormat);
        parentVinyl.mTrackNodes.insert(parentVinyl.mTrackNodes.end(), playlist.mTrackNodes.begin(), playlist.mTrackNodes.end());
    }
    else if(playlist.mOriginalFormat == OF_DVD)
    {
        char parentVinylName[MAX_PATH];
        chars = Str::Copy(parentVinylName, ARRAY_COUNT(parentVinylName), parentDir);
        Str::Copy(parentVinylName + chars, ARRAY_COUNT(parentVinylName) - chars, "\\DVD.m3u");
        
        const PlaylistNode& parentVinyl = GetPlaylist(parentVinylName, playlist.mOriginalFormat);
        parentVinyl.mTrackNodes.insert(parentVinyl.mTrackNodes.end(), playlist.mTrackNodes.begin(), playlist.mTrackNodes.end());
    }
    
    if((playlist.mOriginalFormat == OF_ASSORTED) || (playlist.mLeafNode))
    {
        char parentAllName[MAX_PATH];
        chars = Str::Copy(parentAllName, ARRAY_COUNT(parentAllName), parentDir);
        Str::Copy(parentAllName + chars, ARRAY_COUNT(parentAllName) - chars, "\\All.m3u");
        
        const PlaylistNode& parentAll = GetPlaylist(parentAllName, OF_ASSORTED);
        parentAll.mTrackNodes.insert(parentAll.mTrackNodes.end(), playlist.mTrackNodes.begin(), playlist.mTrackNodes.end());
    }
}

const PlaylistNode& DependencyGraph::GetPlaylist(const char* fileName, OriginalFormat originalFormat)
{
    PlaylistNode node(fileName);
    
    PlaylistNodeSet::iterator f = mPlaylistNodes.find(node);
    
    if(f != mPlaylistNodes.end())
    {
        node.mOriginalFormat = originalFormat;
        return(*f);
    }
    
    const PlaylistNode& newNode = *(mPlaylistNodes.insert(node).first);
    newNode.mOriginalFormat = originalFormat;
    mDepth = std::max(mDepth, newNode.mDepth);
    
    return(newNode);
}

const TrackLinkNode& DependencyGraph::GetTrackLink(const char* fileName)
{
    TrackLinkNode node(fileName);
    
    TrackLinkNodeSet::iterator f = mTrackLinkNodes.find(node);
    
    if(f != mTrackLinkNodes.end())
    {
        return(*f);
    }
    
    const TrackLinkNode& newNode = *(mTrackLinkNodes.insert(node).first);
    
    return(newNode);
}

const TrackLinkNode& DependencyGraph::GetTrackLinkEx(const char* fileName)
{
    TrackLinkNode node(fileName);
    
    {
        TrackLinkNodeSet::iterator f = mTrackLinkNodes.find(node);
    
        if(f != mTrackLinkNodes.end())
        {
            return(*f);
        }
    }
    
    // Do a case-insensitive search now:
    
    for(TrackLinkNodeSet::iterator f = mTrackLinkNodes.begin(); f != mTrackLinkNodes.end(); ++f)
    {   
        if(Str::LooseCompare(f->mName, fileName) == 0)
        {
            return(*f);
        }
    }
    
    const TrackLinkNode& newNode = *(mTrackLinkNodes.insert(node).first);
    
    return(newNode);
}

const PlaylistLinkNode& DependencyGraph::GetPlaylistLink(const char* fileName)
{
    PlaylistLinkNode node(fileName);
    
    PlaylistLinkNodeSet::iterator f = mPlaylistLinkNodes.find(node);
    
    if(f != mPlaylistLinkNodes.end())
    {
        return(*f);
    }
    
    const PlaylistLinkNode& newNode = *(mPlaylistLinkNodes.insert(node).first);
    mDepth = std::max(mDepth, newNode.mDepth);

    return(newNode);
}

void DependencyGraph::MakeDirNodes(const char* dirName)
{
    if(dirName == mRootDir)
    {
        return;
    }

    DirNode node(dirName);

    DirNodeSet::iterator f = mDirNodes.find(node);
    
    if(f != mDirNodes.end())
    {
        return;
    }

    mDirNodes.insert(node);
    
    char subDir[MAX_PATH];
    Str::DirName(subDir, ARRAY_COUNT(subDir), dirName);
    
    MakeDirNodes(subDir);
}

void DependencyGraph::ParseFile(const ScanFile& scanFile)
{
    if(scanFile.mIsDir)
    {
        DirNode newNode(scanFile.mName, scanFile.mTime, scanFile.mSize);
        mDirNodes.insert(newNode);
        return;
    }
    
    int ovector[32];
    Util::Zeroize(ovector, sizeof(ovector));
    
    int matches;
    
    matches = pcre_exec(mUnMixedTrackRE, mUnMixedTrackREE, scanFile.mName, static_cast<int>(scanFile.mNameLen), 0, 0, ovector, ARRAY_COUNT(ovector));
    
    if(matches >= 0)
    {
        TrackNode newNode(scanFile.mName, scanFile.mTime, scanFile.mSize);
        
        int token = 1;
    
        Assert(pcre_copy_substring(scanFile.mName, ovector, matches, token++, newNode.mLabel, ARRAY_COUNT(newNode.mLabel)) > 0); 
    
        newNode.mCompilation = true;
        newNode.mMixed = false;
    
        char digitBuffer[8] = "";
        Assert(pcre_copy_substring(scanFile.mName, ovector, matches, token++, digitBuffer, ARRAY_COUNT(digitBuffer)) > 0); 
        newNode.mAlbumYear = atoi(digitBuffer);
        
        Str::Copy(newNode.mAlbumArtist, ARRAY_COUNT(newNode.mAlbumArtist), "Various");
        
        Assert(pcre_copy_substring(scanFile.mName, ovector, matches, token++, newNode.mAlbumTitle, ARRAY_COUNT(newNode.mAlbumTitle)) > 0); 

        char formatBuffer[8] = "";
        Assert(pcre_copy_substring(scanFile.mName, ovector, matches, token++, formatBuffer, ARRAY_COUNT(formatBuffer)) > 0); 

        if(Str::Compare(formatBuffer, "CD") == 0)
        {
            newNode.mOriginalFormat = OF_CD;
        }
        else if(Str::Compare(formatBuffer, "DVD") == 0)
        {
            newNode.mOriginalFormat = OF_DVD;
        }
        else if(Str::Compare(formatBuffer, "Vinyl") == 0)
        {
            newNode.mOriginalFormat = OF_VINYL;
        }
        else if(Str::Compare(formatBuffer, "MP3") == 0)
        {
            newNode.mOriginalFormat = OF_MP3;
        }
        else
        {
            newNode.mOriginalFormat = OF_ASSORTED;
        }

        Assert(pcre_copy_substring(scanFile.mName, ovector, matches, token++, digitBuffer, ARRAY_COUNT(digitBuffer)) > 0); 
        newNode.mSequenceNumber = atoi(digitBuffer);

        Assert(pcre_copy_substring(scanFile.mName, ovector, matches, token++, newNode.mTrackArtist, ARRAY_COUNT(newNode.mTrackArtist)) > 0); 
        Assert(pcre_copy_substring(scanFile.mName, ovector, matches, token++, newNode.mTrackName, ARRAY_COUNT(newNode.mTrackName)) > 0); 

        const TrackNode* nodePtr = &*(mTrackNodes.insert(newNode).first);
        Assert(nodePtr);
        // DebugAssert(mTracksByNameSize.find(nodePtr) == mTracksByNameSize.end());
        // mTracksByNameSize.insert(nodePtr);

        return;
    }
    
    matches = pcre_exec(mMixedTrackRE, mMixedTrackREE, scanFile.mName, static_cast<int>(scanFile.mNameLen), 0, 0, ovector, ARRAY_COUNT(ovector));
    
    if(matches >= 0)
    {
        TrackNode newNode(scanFile.mName, scanFile.mTime, scanFile.mSize);

        int token = 1;

        Assert(pcre_copy_substring(scanFile.mName, ovector, matches, token++, newNode.mLabel, ARRAY_COUNT(newNode.mLabel)) > 0); 
    
        newNode.mCompilation = true;
        newNode.mMixed = true;
    
        char digitBuffer[8] = "";
        Assert(pcre_copy_substring(scanFile.mName, ovector, matches, token++, digitBuffer, ARRAY_COUNT(digitBuffer)) > 0); 
        newNode.mAlbumYear = atoi(digitBuffer);
                
        Assert(pcre_copy_substring(scanFile.mName, ovector, matches, token++, newNode.mAlbumArtist, ARRAY_COUNT(newNode.mAlbumArtist)) > 0); 

        Assert(pcre_copy_substring(scanFile.mName, ovector, matches, token++, newNode.mAlbumTitle, ARRAY_COUNT(newNode.mAlbumTitle)) > 0); 

        char formatBuffer[8] = "";
        Assert(pcre_copy_substring(scanFile.mName, ovector, matches, token++, formatBuffer, ARRAY_COUNT(formatBuffer)) > 0); 

        if(Str::Compare(formatBuffer, "CD") == 0)
        {
            newNode.mOriginalFormat = OF_CD;
        }
        else if(Str::Compare(formatBuffer, "DVD") == 0)
        {
            newNode.mOriginalFormat = OF_DVD;
        }
        else if(Str::Compare(formatBuffer, "Vinyl") == 0)
        {
            newNode.mOriginalFormat = OF_VINYL;
        }
        else if(Str::Compare(formatBuffer, "MP3") == 0)
        {
            newNode.mOriginalFormat = OF_MP3;
        }
        else
        {
            newNode.mOriginalFormat = OF_ASSORTED;
        }

        Assert(pcre_copy_substring(scanFile.mName, ovector, matches, token++, digitBuffer, ARRAY_COUNT(digitBuffer)) > 0); 
        newNode.mSequenceNumber = atoi(digitBuffer);

        Assert(pcre_copy_substring(scanFile.mName, ovector, matches, token++, newNode.mTrackArtist, ARRAY_COUNT(newNode.mTrackArtist)) > 0); 
        Assert(pcre_copy_substring(scanFile.mName, ovector, matches, token++, newNode.mTrackName, ARRAY_COUNT(newNode.mTrackName)) > 0); 

        const TrackNode* nodePtr = &*(mTrackNodes.insert(newNode).first);
        Assert(nodePtr);
        // DebugAssert(mTracksByNameSize.find(nodePtr) == mTracksByNameSize.end());
        // mTracksByNameSize.insert(nodePtr);

        return;
    }
    
    matches = pcre_exec(mNormalTrackRE, mNormalTrackREE, scanFile.mName, static_cast<int>(scanFile.mNameLen), 0, 0, ovector, ARRAY_COUNT(ovector));
    
    if(matches >= 0)
    {
        TrackNode newNode(scanFile.mName, scanFile.mTime, scanFile.mSize);

        int token = 1;

        Assert(pcre_copy_substring(scanFile.mName, ovector, matches, token++, newNode.mLabel, ARRAY_COUNT(newNode.mLabel)) > 0); 
    
        newNode.mCompilation = false;
        newNode.mMixed = false;
                
        Assert(pcre_copy_substring(scanFile.mName, ovector, matches, token++, newNode.mAlbumArtist, ARRAY_COUNT(newNode.mAlbumArtist)) > 0); 
    
        Str::Copy(newNode.mTrackArtist, ARRAY_COUNT(newNode.mTrackArtist), newNode.mAlbumArtist); 
    
        char digitBuffer[8] = "";
        Assert(pcre_copy_substring(scanFile.mName, ovector, matches, token++, digitBuffer, ARRAY_COUNT(digitBuffer)) > 0); 
        newNode.mAlbumYear = atoi(digitBuffer);

        Assert(pcre_copy_substring(scanFile.mName, ovector, matches, token++, newNode.mAlbumTitle, ARRAY_COUNT(newNode.mAlbumTitle)) > 0); 

        char formatBuffer[8] = "";
        Assert(pcre_copy_substring(scanFile.mName, ovector, matches, token++, formatBuffer, ARRAY_COUNT(formatBuffer)) > 0); 

        if(Str::Compare(formatBuffer, "CD") == 0)
        {
            newNode.mOriginalFormat = OF_CD;
        }
        else if(Str::Compare(formatBuffer, "DVD") == 0)
        {
            newNode.mOriginalFormat = OF_DVD;
        }
        else if(Str::Compare(formatBuffer, "Bluray") == 0)
        {
            newNode.mOriginalFormat = OF_BLURAY;
        }
        else if(Str::Compare(formatBuffer, "Vinyl") == 0)
        {
            newNode.mOriginalFormat = OF_VINYL;
        }
        else if(Str::Compare(formatBuffer, "MP3") == 0)
        {
            newNode.mOriginalFormat = OF_MP3;
        }
        else
        {
            newNode.mOriginalFormat = OF_ASSORTED;
        }

        Assert(pcre_copy_substring(scanFile.mName, ovector, matches, token++, digitBuffer, ARRAY_COUNT(digitBuffer)) > 0); 
        newNode.mSequenceNumber = atoi(digitBuffer);

        Assert(pcre_copy_substring(scanFile.mName, ovector, matches, token++, newNode.mTrackName, ARRAY_COUNT(newNode.mTrackName)) > 0); 

        const TrackNode* nodePtr = &*(mTrackNodes.insert(newNode).first);
        Assert(nodePtr);
        // DebugAssert(mTracksByNameSize.find(nodePtr) == mTracksByNameSize.end());
        // mTracksByNameSize.insert(nodePtr);

        return;
    }
    
    matches = pcre_exec(mPlaylistRE1, mPlaylistREE1, scanFile.mName, static_cast<int>(scanFile.mNameLen), 0, 0, ovector, ARRAY_COUNT(ovector));
    
    if(matches < 0)
    {
        matches = pcre_exec(mPlaylistRE2, mPlaylistREE2, scanFile.mName, static_cast<int>(scanFile.mNameLen), 0, 0, ovector, ARRAY_COUNT(ovector));
    }
    
    if(matches >= 0)
    {
        PlaylistNode newNode(scanFile.mName, scanFile.mTime, scanFile.mSize);

        int token = 1;

        char formatBuffer[8] = "";
        Assert(pcre_copy_substring(scanFile.mName, ovector, matches, token++, formatBuffer, ARRAY_COUNT(formatBuffer)) > 0); 

        if(Str::Compare(formatBuffer, "CD") == 0)
        {
            newNode.mOriginalFormat = OF_CD;
        }
        else if(Str::Compare(formatBuffer, "Vinyl") == 0)
        {
            newNode.mOriginalFormat = OF_VINYL;
        }
        else if(Str::Compare(formatBuffer, "DVD") == 0)
        {
            newNode.mOriginalFormat = OF_DVD;
        }
        else if(Str::Compare(formatBuffer, "MP3") == 0)
        {
            newNode.mOriginalFormat = OF_MP3;
        }
        else
        {
            Assert(Str::Compare(formatBuffer, "All") == 0)
            newNode.mOriginalFormat = OF_ASSORTED;
        }
        
        const char* baseName = Str::BaseName(newNode.mName);
        Assert(baseName);
        
        if(baseName[0] == '0')
        {
            newNode.mLeafNode = true;
        }
        
        const PlaylistNode* nodePtr = &*(mPlaylistNodes.insert(newNode).first);
        Assert(nodePtr);

        if(newNode.mLeafNode)
        {
            // DebugAssert(mPlaylistsByNameSize.find(nodePtr) == mPlaylistsByNameSize.end());
            // mPlaylistsByNameSize.insert(nodePtr);
        }
        
        mDepth = std::max(mDepth, newNode.mDepth);

        return;
    }

    matches = pcre_exec(mTrackLinkRE, mTrackLinkREE, scanFile.mName, static_cast<int>(scanFile.mNameLen), 0, 0, ovector, ARRAY_COUNT(ovector));
    
    if(matches >= 0)
    {
        TrackLinkNode newNode(scanFile.mName, scanFile.mTime, scanFile.mSize);
        mTrackLinkNodes.insert(newNode);
        return;
    }

    matches = pcre_exec(mPlaylistLinkRE, mPlaylistLinkREE, scanFile.mName, static_cast<int>(scanFile.mNameLen), 0, 0, ovector, ARRAY_COUNT(ovector));
    
    if(matches >= 0)
    {
        PlaylistLinkNode newNode(scanFile.mName, scanFile.mTime, scanFile.mSize);
        mPlaylistLinkNodes.insert(newNode);

        mDepth = std::max(mDepth, newNode.mDepth);
        
        return;
    }
    
    FileNode newNode(scanFile.mName, scanFile.mTime, scanFile.mSize);
    mFileNodes.insert(newNode);
}

void DependencyGraph::LoadITunes(const TrackNode& track)
{
    if(!track.mIITTrack)
    {
        return;
        
    }
    BSTR uniGenre = NULL;
    if(!FAILED(track.mIITTrack->get_Genre(&uniGenre)) && uniGenre)
    {
        Str::UnicodeToAnsi(track.mGenre, ARRAY_COUNT(track.mGenre), uniGenre);
        SysFreeString(uniGenre);
    }
    
    long rating = 0;
    if(!FAILED(track.mIITTrack->get_Rating(&rating)) && rating)
    {
        Assert(rating >= 20);
        Assert(rating <= 100);
        Assert((rating % 20) == 0);
        track.mRating = rating / 20;
    }
}

void DependencyGraph::UpdateITunes(const TrackNode& track)
{
    if(track.mRedundantTrack)
    {
        return;
    }

    if(!track.mIITTrack)
    {
        IITOperationStatus* operationStatus = NULL;
        Assert(!FAILED(mITLibraryPlaylist->AddFile(MakeBStr(track.mName), &operationStatus)));
        Assert(operationStatus);
        
        VARIANT_BOOL inProgress = TRUE;
        Assert(!FAILED(operationStatus->get_InProgress(&inProgress)));
        while(inProgress)
        {
            Sleep(100);
            Assert(!FAILED(operationStatus->get_InProgress(&inProgress)));
        }
        
        IITTrackCollection* newCollection = NULL;
        
        Assert(!FAILED(operationStatus->get_Tracks(&newCollection)));

        long trackCount = 0;    
        Assert(!FAILED(newCollection->get_Count(&trackCount)));
        Assert(trackCount == 1);
        
        IITTrack* trackBase = NULL;
        Assert(!FAILED(newCollection->get_Item(1, &trackBase)));
        
        Assert(!FAILED(trackBase->QueryInterface(IID_IITFileOrCDTrack, reinterpret_cast<LPVOID*>(&track.mIITTrack))));
        Assert(track.mIITTrack);
        
        SafeRelease(trackBase);
        SafeRelease(newCollection);
        SafeRelease(operationStatus);
    }

    track.mIITTrack->put_Name(MakeBStr(track.mTrackName));
    track.mIITTrack->put_Artist(MakeBStr(track.mCompilation ? track.mTrackArtist : track.mAlbumArtist));

    track.mIITTrack->put_Compilation(track.mMixed ? VARIANT_TRUE : VARIANT_FALSE);

    track.mIITTrack->put_Composer(MakeBStr(track.mAlbumArtist));
    track.mIITTrack->put_Album(MakeBStr(track.mAlbumTitle));

    const char* grouping = "";
    switch(track.mOriginalFormat)
    {
        case OF_ASSORTED:   grouping = "Assorted"; break;
        case OF_VINYL:      grouping = "Vinyl"; break;
        case OF_CD:         grouping = "CD"; break;
        case OF_MP3:        grouping = "MP3"; break;
        case OF_DVD:        grouping = "DVD"; break;
        case OF_BLURAY:     grouping = "Blu-ray"; break;
    }
    track.mIITTrack->put_Grouping(MakeBStr(grouping));

    track.mIITTrack->put_Genre(MakeBStr(track.mGenre));
    track.mIITTrack->put_Rating(static_cast<long>(track.mRating * 20));
    track.mIITTrack->put_Year(static_cast<long>(track.mAlbumYear));
}
