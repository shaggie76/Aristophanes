#include "AristophanesPch.h"
#include "FileNode.h"
#include "Str.h"
#include "iTunes.h"

static size_t GetDepth(const char* fileName)
{
    size_t depth = 0;
    
    for(size_t i = 0; fileName[i]; ++i)
    {
        if(fileName[i] == '\\')
        {
            ++depth;
        }
    }
    
    return(depth);
}

FileNode::FileNode(const char* name, const FILETIME& time, const QWORD& size) :
    mTime(time),
    mSize(size)
{
    Str::Copy(mName, ARRAY_COUNT(mName), name);
    
    const char* baseName = Str::BaseName(mName);
    Assert(baseName);
    Assert(baseName > mName);
    mFileNameStart = baseName - mName;
}

FileNode::FileNode(const char* name)
{
    Util::Zeroize(&mTime, sizeof(mTime));
    Util::Zeroize(&mSize, sizeof(mSize));

    Str::Copy(mName, ARRAY_COUNT(mName), name);

    const char* baseName = Str::BaseName(mName);
    Assert(baseName);
    Assert(baseName > mName);
    mFileNameStart = baseName - mName;
}

FileNode::~FileNode()
{
}

DirNode::DirNode(const char* name, const FILETIME& time, const QWORD& size) :
    FileNode(name, time, size)
{
    mDepth = GetDepth(mName);
}

DirNode::DirNode(const char* name) :
    FileNode(name)
{
    mDepth = GetDepth(mName);
}

DirNode::~DirNode()
{
}

TrackInterface::TrackInterface(const char* name, const FILETIME& time, const QWORD& size) :
    FileNode(name, time, size)
{
}

TrackInterface::TrackInterface(const char* name) :
    FileNode(name)
{
}

TrackInterface::~TrackInterface()
{
}

TrackNode::TrackNode(const char* name, const FILETIME& time, const QWORD& size) :
    TrackInterface(name, time, size),
    mAlbumYear(0),
    mOriginalFormat(OF_ASSORTED),
    mSequenceNumber(0),
    mDuration(0),
    mRating(0),
    mCompilation(false),
    mMixed(false),
    mPlaylist(NULL),
    mIITTrack(NULL),
    mRedundantTrack(false)
{
    Util::Zeroize(mLabel, sizeof(mLabel));
    Util::Zeroize(mAlbumArtist, sizeof(mAlbumArtist));
    Util::Zeroize(mAlbumTitle, sizeof(mAlbumTitle));
    Util::Zeroize(mTrackArtist, sizeof(mTrackArtist));
    Util::Zeroize(mTrackName, sizeof(mTrackName));
    Util::Zeroize(mGenre, sizeof(mGenre));
}

TrackNode::TrackNode(const char* name) :
    TrackInterface(name),
    mAlbumYear(0),
    mOriginalFormat(OF_ASSORTED),
    mSequenceNumber(0),
    mDuration(0),
    mRating(0),
    mCompilation(false),
    mMixed(false),
    mPlaylist(NULL),
    mIITTrack(NULL),
    mRedundantTrack(false)
{
    Util::Zeroize(mLabel, sizeof(mLabel));
    Util::Zeroize(mAlbumArtist, sizeof(mAlbumArtist));
    Util::Zeroize(mAlbumTitle, sizeof(mAlbumTitle));
    Util::Zeroize(mTrackArtist, sizeof(mTrackArtist));
    Util::Zeroize(mTrackName, sizeof(mTrackName));
    Util::Zeroize(mGenre, sizeof(mGenre));
}

TrackNode::~TrackNode()
{
    SafeRelease(mIITTrack);
}

const TrackNode* TrackNode::GetTrackNode() const
{
    return(this);
}

TrackLinkNode::TrackLinkNode(const char* name, const FILETIME& time, const QWORD& size) :
    TrackInterface(name, time, size),
    mSource(NULL),
    mSkipAlbumPlaylist(false)
{
}

TrackLinkNode::TrackLinkNode(const char* name) :
    TrackInterface(name),
    mSource(NULL),
    mSkipAlbumPlaylist(false)
{
}

TrackLinkNode::~TrackLinkNode()
{
}

const TrackNode* TrackLinkNode::GetTrackNode() const
{
    return(mSource);
}

PlaylistNode::PlaylistNode(const char* name, const FILETIME& time, const QWORD& size) :
    FileNode(name, time, size),
    mOriginalFormat(OF_ASSORTED),
    mLeafNode(false),
    mTrackNodes()
{
    mDepth = GetDepth(mName);
}

PlaylistNode::PlaylistNode(const char* name) :
    FileNode(name),
    mOriginalFormat(OF_ASSORTED),
    mLeafNode(false),
    mTrackNodes()
{
    mDepth = GetDepth(mName);
}

PlaylistNode::~PlaylistNode()
{
}

bool PlaylistNode::IsDirty() const
{
    if(IsZero(mTime))
    {
        return(true);
    }
    
    for(TrackNodes::const_iterator i = mTrackNodes.begin(); i != mTrackNodes.end(); ++i)
    {
        const TrackInterface* track = *i;
    
        if(CompareFileTime(&mTime, &track->mTime) < 0)
        {
            return(true);
        }
    }

    return(false);
}

PlaylistLinkNode::PlaylistLinkNode(const char* name, const FILETIME& time, const QWORD& size) :
    PlaylistNode(name, time, size),
    mSource(NULL)
{
    mLeafNode = true;
}

PlaylistLinkNode::PlaylistLinkNode(const char* name) :
    PlaylistNode(name),
    mSource(NULL)
{
    mLeafNode = true;
}

PlaylistLinkNode::~PlaylistLinkNode()
{
}
