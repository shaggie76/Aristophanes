#include "AristophanesPch.h"

#include "MP3Info.h"

#include "Tag.h"
#include "FileNode.h"
#include "ID3LibEx.h"
#include "Str.h"

static const char* GetFormatString(OriginalFormat of)
{
    switch(of)
    {
        case OF_VINYL:  return("Vinyl");
        case OF_CD:     return("CD");
        case OF_MP3:    return("MP3");
        case OF_DVD:    return("DVD");
        default:        Assert(!"Unknown format!"); return("Somewhere");
    }
}

void UpdateTag(const TrackNode& track)
{
    {
        MP3Info trackInfo;
        trackInfo.loadInfo(const_cast<char*>(track.mName));
        track.mDuration = trackInfo.GetLengthInSeconds();
    }
    
    bool wasModified = false;

    ID3_Tag fileTag(track.mName);

    char yearString[MAX_PATH];
    char trackString[MAX_PATH];
    char encodedByString[MAX_PATH];

    const char* artistName = ID3_GetArtist(&fileTag);
    const char* albumName = ID3_GetAlbum(&fileTag);
    const char* title = ID3_GetTitle(&fileTag);
    const char* year = ID3_GetYear(&fileTag);
    const char* releaseDate = ID3_GetReleaseDate(&fileTag);
    const char* comment = ID3_GetComment(&fileTag);
    const char* trackName = ID3_GetTrack(&fileTag);
    const char* copyright = ID3_GetCopyright(&fileTag);
    const char* publisher = ID3_GetPublisher(&fileTag);
    const char* encodedBy = ID3_GetEncodedBy(&fileTag);
    const char* genre = ID3_GetGenre(&fileTag);
    
    size_t rating = ID3_GetTraktorRating(&fileTag);

    Str::Print(yearString, ARRAY_COUNT(yearString), "%4d", track.mAlbumYear);
    Assert(track.mPlaylist);
    Str::Print(trackString, ARRAY_COUNT(trackString), "%d/%d", track.mSequenceNumber, track.mPlaylist->mTrackNodes.size());

    Str::Print(encodedByString, ARRAY_COUNT(encodedByString), "Shaggie76 [From %s]", GetFormatString(track.mOriginalFormat));

    if(fileTag.HasV1Tag())
    {
        fileTag.Strip(ID3TT_ID3V1);
        wasModified = true;
    }

    if((artistName == NULL) || (Str::Compare(artistName, track.mTrackArtist) != 0))
    {
        ID3_AddArtist(&fileTag, track.mTrackArtist, true);
        wasModified = true;
    }

    if((albumName == NULL) || (Str::Compare(albumName, track.mAlbumTitle) != 0))
    {
        ID3_AddAlbum(&fileTag, track.mAlbumTitle, true);
        wasModified = true;
    }

    if((title == NULL) || (Str::Compare(title, track.mTrackName) != 0))
    {
        ID3_AddTitle(&fileTag, track.mTrackName, true);
        wasModified = true;
    }

    if((year == NULL) || (Str::Compare(year, yearString) != 0))
    {
        ID3_AddYear(&fileTag, yearString, true);
        wasModified = true;
    }

    if((releaseDate == NULL) || (Str::Compare(releaseDate, yearString) != 0))
    {
        ID3_AddReleaseDate(&fileTag, yearString, true);
        wasModified = true;
    }

    if((trackName == NULL) || (Str::Compare(trackName, trackString) != 0))
    {
        ID3_AddTrack(&fileTag, static_cast<uchar>(track.mSequenceNumber), static_cast<uchar>(track.mPlaylist->mTrackNodes.size()), true);
        wasModified = true;
    }

    if((copyright == NULL) || (Str::Compare(copyright, track.mLabel) != 0))
    {
        ID3_AddCopyright(&fileTag, track.mLabel, true);
        wasModified = true;
    }

    if((publisher == NULL) || (Str::Compare(publisher, track.mLabel) != 0))
    {
        ID3_AddPublisher(&fileTag, track.mLabel, true);
        wasModified = true;
    }

    if((encodedBy == NULL) || (Str::Compare(encodedBy, encodedByString) != 0))
    {
        ID3_AddEncodedBy(&fileTag, encodedByString, true);
        wasModified = true;
    }

    if(comment != NULL)
    {
        ID3_RemoveComments(&fileTag);
        wasModified = true;
    }
    
    if(Str::Len(track.mGenre))
    {
        if(!genre || Str::Compare(track.mGenre, genre))
        {
            ID3_AddGenre(&fileTag, track.mGenre, true);
            wasModified = true;
        }
    }
    else if(genre)
    {
        Str::Copy(track.mGenre, ARRAY_COUNT(track.mGenre), genre);
    }
    else if(!Str::Compare(track.mLabel, "Movies"))
    {
        Str::Copy(track.mGenre, ARRAY_COUNT(track.mGenre), "Misc/Movie Score");
        ID3_AddGenre(&fileTag, track.mGenre, true);
        wasModified = true;
    }

    if(track.mRating && (rating != track.mRating))
    {
        ID3_AddTraktorRating(&fileTag, track.mRating, true);
        wasModified = true;
    }
    else if(rating)
    {
        track.mRating = rating;
    }
    
    const ID3_V2Spec SPEC_TYPE = ID3V2_3_0; // WinXP doesn't like 2.4 yet

    if(fileTag.GetSpec() != SPEC_TYPE)
    {
        fileTag.SetSpec(SPEC_TYPE);
        wasModified = true;
    }
    
    if(wasModified)
    {
        fileTag.Update(ID3TT_ID3V2);
    }

    delete[] artistName;
    delete[] albumName;
    delete[] title;
    delete[] year;
    delete[] releaseDate;
    delete[] comment;
    delete[] trackName;
    delete[] copyright;
    delete[] publisher;
    delete[] encodedBy;
    delete[] genre;
}

void PrintInformation(const ID3_Tag &myTag)
{
  SStream s;
  
  ID3_Tag::ConstIterator* iter = myTag.CreateIterator();
  const ID3_Frame* frame = NULL;
  while (NULL != (frame = iter->GetNext()))
  {
    const char* desc = frame->GetDescription();
    if (!desc) desc = "";
    s << "=== " << frame->GetTextID() << " (" << desc << "): ";
    ID3_FrameID eFrameID = frame->GetID();
    switch (eFrameID)
    {
      case ID3FID_ALBUM:
      case ID3FID_BPM:
      case ID3FID_COMPOSER:
      case ID3FID_CONTENTTYPE:
      case ID3FID_COPYRIGHT:
      case ID3FID_DATE:
      case ID3FID_PLAYLISTDELAY:
      case ID3FID_ENCODEDBY:
      case ID3FID_LYRICIST:
      case ID3FID_FILETYPE:
      case ID3FID_TIME:
      case ID3FID_CONTENTGROUP:
      case ID3FID_TITLE:
      case ID3FID_SUBTITLE:
      case ID3FID_INITIALKEY:
      case ID3FID_LANGUAGE:
      case ID3FID_SONGLEN:
      case ID3FID_MEDIATYPE:
      case ID3FID_ORIGALBUM:
      case ID3FID_ORIGFILENAME:
      case ID3FID_ORIGLYRICIST:
      case ID3FID_ORIGARTIST:
      case ID3FID_ORIGYEAR:
      case ID3FID_FILEOWNER:
      case ID3FID_LEADARTIST:
      case ID3FID_BAND:
      case ID3FID_CONDUCTOR:
      case ID3FID_MIXARTIST:
      case ID3FID_PARTINSET:
      case ID3FID_PUBLISHER:
      case ID3FID_TRACKNUM:
      case ID3FID_RECORDINGDATES:
      case ID3FID_NETRADIOSTATION:
      case ID3FID_NETRADIOOWNER:
      case ID3FID_SIZE:
      case ID3FID_ISRC:
      case ID3FID_ENCODERSETTINGS:
      case ID3FID_YEAR:
      {
        char *sText = ID3_GetString(frame, ID3FN_TEXT);
        s << sText << std::endl;
        delete [] sText;
        break;
      }
      case ID3FID_USERTEXT:
      {
        char 
        *sText = ID3_GetString(frame, ID3FN_TEXT), 
        *sDesc = ID3_GetString(frame, ID3FN_DESCRIPTION);
        s << "(" << sDesc << "): " << sText << std::endl;
        delete [] sText;
        delete [] sDesc;
        break;
      }
      case ID3FID_COMMENT:
      case ID3FID_UNSYNCEDLYRICS:
      {
        char 
        *sText = ID3_GetString(frame, ID3FN_TEXT), 
        *sDesc = ID3_GetString(frame, ID3FN_DESCRIPTION), 
        *sLang = ID3_GetString(frame, ID3FN_LANGUAGE);
        s << "(" << sDesc << ")[" << sLang << "]: "
             << sText << std::endl;
        delete [] sText;
        delete [] sDesc;
        delete [] sLang;
        break;
      }
      case ID3FID_WWWAUDIOFILE:
      case ID3FID_WWWARTIST:
      case ID3FID_WWWAUDIOSOURCE:
      case ID3FID_WWWCOMMERCIALINFO:
      case ID3FID_WWWCOPYRIGHT:
      case ID3FID_WWWPUBLISHER:
      case ID3FID_WWWPAYMENT:
      case ID3FID_WWWRADIOPAGE:
      {
        char *sURL = ID3_GetString(frame, ID3FN_URL);
        s << sURL << std::endl;
        delete [] sURL;
        break;
      }
      case ID3FID_WWWUSER:
      {
        char 
        *sURL = ID3_GetString(frame, ID3FN_URL),
        *sDesc = ID3_GetString(frame, ID3FN_DESCRIPTION);
        s << "(" << sDesc << "): " << sURL << std::endl;
        delete [] sURL;
        delete [] sDesc;
        break;
      }
      case ID3FID_INVOLVEDPEOPLE:
      {
        size_t nItems = frame->GetField(ID3FN_TEXT)->GetNumTextItems();
        for (size_t nIndex = 0; nIndex < nItems; nIndex++)
        {
          char *sPeople = ID3_GetString(frame, ID3FN_TEXT, nIndex);
          s << sPeople;
          delete [] sPeople;
          if (nIndex + 1 < nItems)
          {
            s << ", ";
          }
        }
        s << std::endl;
        break;
      }
      case ID3FID_PICTURE:
      {
        char
        *sMimeType = ID3_GetString(frame, ID3FN_MIMETYPE),
        *sDesc     = ID3_GetString(frame, ID3FN_DESCRIPTION),
        *sFormat   = ID3_GetString(frame, ID3FN_IMAGEFORMAT);
        size_t
        nPicType   = frame->GetField(ID3FN_PICTURETYPE)->Get(),
        nDataSize  = frame->GetField(ID3FN_DATA)->Size();
        s << "(" << sDesc << ")[" << sFormat << ", "
             << static_cast<unsigned>(nPicType) << "]: " << sMimeType << ", " << static_cast<unsigned>(nDataSize)
             << " bytes" << std::endl;
        delete [] sMimeType;
        delete [] sDesc;
        delete [] sFormat;
        break;
      }
      case ID3FID_GENERALOBJECT:
      {
        char 
        *sMimeType = ID3_GetString(frame, ID3FN_MIMETYPE), 
        *sDesc = ID3_GetString(frame, ID3FN_DESCRIPTION), 
        *sFileName = ID3_GetString(frame, ID3FN_FILENAME);
        size_t 
        nDataSize = frame->GetField(ID3FN_DATA)->Size();
        s << "(" << sDesc << ")[" 
             << sFileName << "]: " << sMimeType << ", " << static_cast<unsigned>(nDataSize)
             << " bytes" << std::endl;
        delete [] sMimeType;
        delete [] sDesc;
        delete [] sFileName;
        break;
      }
      case ID3FID_UNIQUEFILEID:
      {
        char *sOwner = ID3_GetString(frame, ID3FN_OWNER);
        size_t nDataSize = frame->GetField(ID3FN_DATA)->Size();
        s << sOwner << ", " << static_cast<unsigned>(nDataSize)
             << " bytes" << std::endl;
        delete [] sOwner;
        break;
      }
      case ID3FID_PLAYCOUNTER:
      {
        size_t nCounter = frame->GetField(ID3FN_COUNTER)->Get();
        s << static_cast<unsigned>(nCounter) << std::endl;
        break;
      }
      case ID3FID_POPULARIMETER:
      {
        char *sEmail = ID3_GetString(frame, ID3FN_EMAIL);
        size_t
        nCounter = frame->GetField(ID3FN_COUNTER)->Get(),
        nRating = frame->GetField(ID3FN_RATING)->Get();
        s << sEmail << ", counter=" 
             << static_cast<unsigned>(nCounter) << " rating=" << static_cast<unsigned>(nRating) << std::endl;
        delete [] sEmail;
        break;
      }
      case ID3FID_CRYPTOREG:
      case ID3FID_GROUPINGREG:
      {
        char *sOwner = ID3_GetString(frame, ID3FN_OWNER);
        size_t 
        nSymbol = frame->GetField(ID3FN_ID)->Get(),
        nDataSize = frame->GetField(ID3FN_DATA)->Size();
        s << "(" << static_cast<unsigned>(nSymbol) << "): " << sOwner
             << ", " << static_cast<unsigned>(nDataSize) << " bytes" << std::endl;
        break;
      }
      case ID3FID_AUDIOCRYPTO:
      case ID3FID_EQUALIZATION:
      case ID3FID_EVENTTIMING:
      case ID3FID_CDID:
      case ID3FID_MPEGLOOKUP:
      case ID3FID_OWNERSHIP:
      case ID3FID_PRIVATE:
      case ID3FID_POSITIONSYNC:
      case ID3FID_BUFFERSIZE:
      case ID3FID_VOLUMEADJ:
      case ID3FID_REVERB:
      case ID3FID_SYNCEDTEMPO:
      case ID3FID_METACRYPTO:
      {
        s << " (unimplemented)" << std::endl;
        break;
      }
      default:
      {
        s << " frame" << std::endl;
        break;
      }
    }
  }
  delete iter;
  OutputDebugString(s.str().c_str()); 
}

void DumpTag(const char* fileName)
{
    ID3_Tag fileTag(fileName);
    PrintInformation(fileTag);
}
