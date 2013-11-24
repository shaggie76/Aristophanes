#include "AristophanesPch.h"
#include "DependencyGraph.h"
#include "Str.h"

static const char* EXT_M3U_HEADER =  "#EXTM3U";

// #EXTINF:69,DJ Shadow - (Letter From Home) 
// 01 - (Letter From Home).mp3

void DependencyGraph::WritePlaylist(const PlaylistNode& playlist)
{
    const char* baseName = Str::BaseName(playlist.mName);
    Assert(baseName);
    size_t dirNameLen = baseName - playlist.mName;
    
    std::ofstream s;
    
    s.open(playlist.mName);
    Assert(s.is_open());
    
    s << EXT_M3U_HEADER << std::endl;

    for(TrackNodes::const_iterator i = playlist.mTrackNodes.begin(); i != playlist.mTrackNodes.end(); ++i)
    {
        const TrackInterface* trackIf = *i;
        DebugAssert(Str::Compare(trackIf->mName, playlist.mName, dirNameLen) == 0);
        const TrackNode* track = trackIf->GetTrackNode();

        if(!track->mDuration)
        {
            DebugAssert(track->mPlaylist);
            ReadPlaylist(*track->mPlaylist);
        }

        s << "#EXTINF:" << static_cast<int>(track->mDuration) << "," << track->mTrackArtist << " - " << track->mTrackName << std::endl;
        s << (trackIf->mName + dirNameLen) << std::endl;
    }
    
    s.close();
}

void DependencyGraph::ReadPlaylist(const PlaylistNode& playlist)
{
    Assert(playlist.mSize);
    
    Assert(playlist.mLeafNode);
    
    std::ifstream s;
    
    s.open(playlist.mName);
    Assert(s.is_open());
 
    String buffer;
    buffer.reserve(static_cast<size_t>(playlist.mSize));
    std::getline(s, buffer, '\0');
    s.close();   

    int offset = 0;
    
    const char* baseName = Str::BaseName(playlist.mName);
    Assert(baseName);
    size_t dirNameLen = (baseName - playlist.mName) + 1;

    for(;;)
    {
        int ovector[32];
        Util::Zeroize(ovector, sizeof(ovector));

        int matches = pcre_exec(mPlaylistEntryRE, mPlaylistEntryREE, buffer.c_str(), static_cast<int>(buffer.size()), offset, 0, ovector, ARRAY_COUNT(ovector));
         
        if(matches < 0)
        {
            break;
        }

        offset = ovector[1];

        int token = 1;

        char digitBuffer[8] = "";
        pcre_copy_substring(buffer.c_str(), ovector, matches, token++, digitBuffer, ARRAY_COUNT(digitBuffer)); 
        size_t duration = atoi(digitBuffer);
        Assert(duration);
        
        char trackName[MAX_PATH] = "";
        size_t chars = Str::Copy(trackName, std::min(dirNameLen, ARRAY_COUNT(trackName)), playlist.mName);
        pcre_copy_substring(buffer.c_str(), ovector, matches, token++, trackName + chars, static_cast<int>(ARRAY_COUNT(trackName) - chars)); 
        Assert(Str::Len(trackName));
        
        for(TrackNodes::iterator i = playlist.mTrackNodes.begin(); i != playlist.mTrackNodes.end(); ++i)
        {
            const TrackInterface* trackIf = *i;
            const TrackNode* track = trackIf->GetTrackNode();
            
            if(!Str::Compare(track->mName, trackName))
            {
                track->mDuration = duration;
                duration = 0;
                break;
            }
        }
        
        Assert(!duration);
    }
}
