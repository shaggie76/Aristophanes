#ifndef CMP3INFO_H
#define CMP3INFO_H

/* ----------------------------------------------------------
   MP3Info class is your complete guide to the
   MP3 file format in the C++ language. It's a large class
   with three different, quite large "sub-classes" so it's
   a fair amount of code to look into.

   This code will be well commented, so that everyone can
   understand, as it's made for the public and not for
   private use, although private use is allowed. :)

   all functions specified both in the header and .cpp file
   will have explanations in both locations.

   everything here by: Gustav "Grim Reaper" Munkby
    http://home.swipnet.se/grd/
    grd@swipnet.se
---------------------------------------------------------- */

class MP3Info
{
public:
    // function to load a file into this structure
    // the argument passed is the path to a MP3 file
    int loadInfo( char srcMP3[256] );

    // functions used to get information about the "file"
    int GetFileSize() { return fileSize; };

    // information that is avaliable in FrameHeader & VBR header
    void GetVersion(char* input);
    float GetBitrate();
    int GetFrequency() { return header.GetFrequency(); };
    void GetMode(char* input);

    int GetNumberOfFrames();

    // functions to calculate the length of the song
    // and to present it nicely
    int GetLengthInSeconds();
    void GetFormattedLength(char* input);

    // just to know what kind of file it is.
    bool IsVBR() { return bIsVBitRate; };

private:
    class FrameHeader
    {
    public:

        // This function is quite easy to understand, it loads
        // 4 chars of information into the CFrameHeader class
        // The validity is not tested, so qith this function
        // an invalid FrameHeader could be retrieved
        void LoadHeader(char c[4]);

        // This function is a supplement to the LoadHeader
        // function, the only purpose is to detect if the
        // header loaded by LoadHeader is a valid header
        // or just four different chars
        bool IsValidHeader();

        // Following är functions to get the "indexes" for the various
        // information avaliable. To know which meaning the different
        // bits you need to look into a table, instead of having to
        // do this everytime these functions get the correct index
        // from the correct bits. :)
        int GetFrameSync()     { return ((bithdr>>21) & 2047); };
        int GetVersionIndex()  { return ((bithdr>>19) & 3);  };
        int GetLayerIndex()    { return ((bithdr>>17) & 3);  };
        int GetProtectionBit() { return ((bithdr>>16) & 1);  };
        int GetBitrateIndex()  { return ((bithdr>>12) & 15); };
        int GetFrequencyIndex(){ return ((bithdr>>10) & 3);  };
        int GetPaddingBit()    { return ((bithdr>> 9) & 1);  };
        int GetPrivateBit()    { return ((bithdr>> 8) & 1);  };
        int GetModeIndex()     { return ((bithdr>> 6) & 3);  };
        int GetModeExtIndex()  { return ((bithdr>> 4) & 3);  };
        int GetCoprightBit()   { return ((bithdr>> 3) & 1);  };
        int GetOrginalBit()    { return ((bithdr>> 2) & 1);  };
        int GetEmphasisIndex() { return ((bithdr    ) & 3);  };


        // now comes som function to make life easier once again
        // you don't even have to know what the different indexes
        // means. to get the version, just use the function
        // getVersion. You can't have it easier


        // this returns the MPEG version[1.0-2.5]
        float GetVersion();

        // this returns the Layer[1-3]
        int GetLayer();

        // this returns the current bitrate[8-448 kbps]
        int GetBitrate();

        // this returns the current frequency[8000-48000 Hz]
        int GetFrequency();

        // the purpose of getMode is to get information about
        // the current playing mode, such as:
        // "Joint Stereo"
        void GetMode(char* input);

        private:

        // this contains the orginal header (bit-by-bit) information
        // declared private because there is not really any reason
        // to use it, as all the "indexes" functions exists
        unsigned long bithdr;
    };

    // these are the "sub-classes"
    FrameHeader header;

    class VBitRate
    {
    public:
        // This function is quite easy to understand, it loads
        // 12 chars of information into the CVBitRate class
        bool LoadHeader(char inputheader[12]);

        // this is the only value-retrieving function in this class
        // it returns the Number of Frames[0 -> oo] (oo == eternety)
        int GetNumberOfFrames() { return frames; };

    private:
        // this is the varable holding the number of frames
        int frames;
    };

    VBitRate vbr;

    // just to know what kind of file it is
    bool bIsVBitRate;

    // the file information can not be found elsewhere
    char fileName[256];
    int fileSize;
};

#endif
