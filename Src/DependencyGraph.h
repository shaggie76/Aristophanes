#ifndef DEPENDENCY_GRAPH_H
#define DEPENDENCY_GRAPH_H

#include "FileNode.h"
#include "pcre.h"

#include <set>
#include <string>
#include <vector>

typedef std::vector<std::string> StringArray;

// iTunes opaque pointer forward defs:
interface IiTunes;
interface IITLibraryPlaylist;
interface IITPlaylist;
interface IITTrackCollection;

class DependencyGraph
{
public:
    explicit DependencyGraph(const String& rootDir, IiTunes* iTunes);
    ~DependencyGraph();

    const String& RootDir() const { return(mRootDir); }

    void ScanDirectories();
    void LoadITunesDatabase(StringArray& missingFiles);
    void ConnectGraph();
    void TagTracks();
    void UpdateITunesDatabase();
    void ConnectLinkedPlaylists();
    void ConnectMergedPlaylists();
    void WriteLeafPlaylists();
    void MakeDirs();
    void MakeLinks();
    void WriteMergedPlaylists();
    
    void GetStrayFiles(StringArray& strayFiles) const;
    
    void MarkDuplicates(const DependencyGraph& other);

protected:
    
    void MarkDuplicates(const TrackNode& otherTrack);

    void AddPlaylistConnection(const TrackNode& track);
    void AddArtistConnections(const TrackNode& track);
    void AddAlbumConnections(const TrackNode& track);
    void AddGenreConnections(const TrackNode& track);
    void AddGroupingConnections(const TrackNode& track);
    void AddPlaylistConnections(const PlaylistNode& playlist);

    const PlaylistNode& GetPlaylist(const char* fileName, OriginalFormat originalFormat);
    const TrackLinkNode& GetTrackLink(const char* fileName);
    const TrackLinkNode& GetTrackLinkEx(const char* fileName);
    const PlaylistLinkNode& GetPlaylistLink(const char* fileName);
    void MakeDirNodes(const char* fileName);

    void ParseFile(const struct ScanFile& scanFile);

    void WritePlaylist(const PlaylistNode& playlist);
    void ReadPlaylist(const PlaylistNode& playlist);

    void LoadITunes(const TrackNode& track);
    void UpdateITunes(const TrackNode& track);

    String mRootDir;
    size_t mDepth;

    #define NodeSet(t) typedef std::set<t> t##Set; t##Set m##t##s
    
    NodeSet(DirNode);
    NodeSet(PlaylistNode);
    NodeSet(TrackNode);
    NodeSet(TrackLinkNode);
    NodeSet(PlaylistLinkNode);
    NodeSet(FileNode);
    
    typedef std::set<const FileNode*, NameSizeLess> FileSetByNameSize;
    FileSetByNameSize mTracksByNameSize;
    FileSetByNameSize mPlaylistsByNameSize;
    
    // UnMixed Track: M:\By Label\Eye-Q Records\Various\1995 - Unmixed - Behind The Eye (Volume 2) [CD]\01 - Virtual Symmetry - The VS.mp3
    pcre*       mUnMixedTrackRE;
    pcre_extra* mUnMixedTrackREE;
    
    // Mixed Track: M:\By Label\United Records\Various\2000 - Armin van Buuren - 001 - A State of Trance [CD]\01 - Miller & Floyd - Colours (Humate Remix).mp3
    pcre*       mMixedTrackRE;
    pcre_extra* mMixedTrackREE;

    // Normal Track: M:\By Label\A&M Records\DJ Shadow\2002 - The Private Press [CD]\01 - (Letter From Home).mp3
    pcre*       mNormalTrackRE;
    pcre_extra* mNormalTrackREE;
    
    // Playlist: M:\By Label\A&M Records\DJ Shadow\2002 - The Private Press [CD]\00 - DJ Shadow - The Private Press [2002].m3u

    pcre*       mPlaylistRE1;
    pcre_extra* mPlaylistREE1;

    // Playlist: M:\By Label\A&M Records\DJ Shadow\All.m3u
    //           M:\By Label\A&M Records\DJ Shadow\CD.m3u
    //           M:\By Label\A&M Records\DJ Shadow\Vinyl.m3u
    //           M:\By Artist\All.m3u
    //           M:\By Label\CD.m3u
    //           M:\By Genre\Vinyl.m3u

    pcre*       mPlaylistRE2;
    pcre_extra* mPlaylistREE2;

    // Track Link: M:\By Artist\ADSR\1994 - Primary\01 - Windswept.mp3
    pcre*       mTrackLinkRE;
    pcre_extra* mTrackLinkREE;
    
    // Playlist Link : M:\By Artist\ADSR\1994 - Primary\00 - ADSR - Primary [1994].m3u
    pcre*       mPlaylistLinkRE;
    pcre_extra* mPlaylistLinkREE;

    // Playlist entry:
    pcre*       mPlaylistEntryRE;
    pcre_extra* mPlaylistEntryREE;
    
    IiTunes*            mITunes;
    IITLibraryPlaylist* mITLibraryPlaylist;
    IITPlaylist*        mITPlaylist;
    IITTrackCollection* mITTrackCollection;
};

#endif // DEPENDENCY_GRAPH_H
