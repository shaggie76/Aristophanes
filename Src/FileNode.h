#ifndef FILE_NODE_H
#define FILE_NODE_H

#include <windows.h>
#include <limits.h>
#include <vector>

#include "Str.h"

enum OriginalFormat
{
    OF_ASSORTED,
    OF_VINYL,
    OF_CD,
    OF_MP3,
    OF_DVD,
    OF_BLURAY
};

struct FileNode
{
    FileNode(const char* name, const FILETIME& time, const QWORD& size);
    explicit FileNode(const char* name);
    virtual ~FileNode();
    
    char                mName[MAX_PATH];
    size_t              mFileNameStart;
    mutable FILETIME            mTime;
    QWORD               mSize;
};

struct DirNode : public FileNode
{
    DirNode(const char* name, const FILETIME& time, const QWORD& size);
    explicit DirNode(const char* name);
    virtual ~DirNode();

    size_t              mDepth;
};

struct PlaylistNode;
struct TrackNode;

struct TrackInterface : public FileNode
{
    TrackInterface(const char* name, const FILETIME& time, const QWORD& size);
    explicit TrackInterface(const char* name);
    virtual ~TrackInterface();

    virtual const TrackNode* GetTrackNode() const = 0;
};

interface IITFileOrCDTrack;

struct TrackNode : public TrackInterface
{
    TrackNode(const char* name, const FILETIME& time, const QWORD& size);
    explicit TrackNode(const char* name);
    virtual ~TrackNode();
    
    virtual const TrackNode* GetTrackNode() const;

    char                mLabel[MAX_PATH];

    char                mAlbumArtist[MAX_PATH]; // Blink-182, Various - Unmixed, Various - DJ Mouse

    size_t              mAlbumYear;
    char                mAlbumTitle[MAX_PATH];
    OriginalFormat      mOriginalFormat;
    size_t              mSequenceNumber;
    char                mTrackArtist[MAX_PATH];
    char                mTrackName[MAX_PATH];
    mutable size_t              mDuration;
    
    mutable char                mGenre[MAX_PATH];
    mutable char                mGrouping[MAX_PATH];

    mutable size_t              mRating;
    
    bool                mCompilation;
    bool                mMixed;
    
    mutable const PlaylistNode*       mPlaylist;
    
    mutable IITFileOrCDTrack*   mIITTrack;
    mutable bool                mRedundantTrack; // Already accounted for in another graph.
};

struct TrackLinkNode : public TrackInterface
{
    TrackLinkNode(const char* name, const FILETIME& time, const QWORD& size);
    explicit TrackLinkNode(const char* name);
    virtual ~TrackLinkNode();
    
    virtual const TrackNode* GetTrackNode() const;

    mutable const TrackNode*    mSource;
    mutable bool                mSkipAlbumPlaylist;
};

typedef std::vector<const TrackInterface*> TrackNodes;

struct PlaylistNode : public FileNode
{
    PlaylistNode(const char* name, const FILETIME& time, const QWORD& size);
    explicit PlaylistNode(const char* name);
    virtual ~PlaylistNode();

    bool IsDirty() const;

    mutable OriginalFormat      mOriginalFormat;
    mutable size_t              mDepth;
    mutable bool                mLeafNode;
    
    mutable TrackNodes          mTrackNodes;
};

struct PlaylistLinkNode : public PlaylistNode
{
    PlaylistLinkNode(const char* name, const FILETIME& time, const QWORD& size);
    explicit PlaylistLinkNode(const char* name);
    virtual ~PlaylistLinkNode();

    mutable const PlaylistNode*       mSource;
};

inline bool operator<(const FileNode& a, const FileNode& b)
{
    return(Str::Compare(a.mName, b.mName) < 0);
}

struct NameSizeLess
{
    bool operator()(const FileNode* a, const FileNode* b) const
    {
        if(a->mSize < b->mSize)
        {
            return(true);
        }

        if(a->mSize > b->mSize)
        {
            return(false);
        }
        
        return(Str::Compare(a->mName + a->mFileNameStart, b->mName + b->mFileNameStart) < 0);
    }
};

#endif // FILE_NODE_H
