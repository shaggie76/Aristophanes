#include "mp3Info.h"

#include <fstream>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <float.h>

using namespace std;

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
----------------------------------------------------------*/

int MP3Info::loadInfo( char srcMP3[256] ) {

   // open input-file stream to the specified file, name
   ifstream* ifile = new ifstream(srcMP3, ios::in | ios::binary);

   assert(ifile);

   // Get file size, by setting the pointer in the end and tell the position
   ifile->seekg(0,ios::end);
   fileSize = ifile->tellg();

   // Get srcMP3 into fileName variable
   strcpy(fileName,srcMP3);

   int pos = 0; // current position in file...


   /******************************************************/
   /* search and load the first frame-header in the file*/
   /******************************************************/

   char headerchars[4]; // char variable used for header-loading

   do {
      assert( !ifile->eof() );

      // read in four characters
      ifile->seekg(pos);
      ifile->read(headerchars, 4);

      // move file-position forward
      pos++;

      // convert four chars to CFrameHeader structure
      header.LoadHeader(headerchars);

   }
   while(!header.IsValidHeader() );  // test for correct header

   // to correct the position to be right after the frame-header
   // we'll need to add another 3 to the current position
   pos += 3;


   /******************************************************/
   /* check for an vbr-header, to ensure the info from a*/
   /* vbr-mp3 is correct                                */
   /******************************************************/

   char vbrchars[12];

   // determine offset from first frame-header
   // it depends on two things, the mpeg-version
   // and the mode(stereo/mono)

   if(header.GetVersionIndex()==3 ) {  // mpeg version 1

      if(header.GetModeIndex()==3 ) pos += 17; // Single Channel
      else                           pos += 32;

   } else {                             // mpeg version 2 or 2.5

      if(header.GetModeIndex()==3 ) pos +=  9; // Single Channel
      else                           pos += 17;

   }

   // read next twelve bits in
   ifile->seekg(pos);
   ifile->read(vbrchars, 12);

   // turn 12 chars into a CVBitRate class structure
   bIsVBitRate = vbr.LoadHeader(vbrchars);

   ifile->close();
   delete ifile;
   return 0;

}





float MP3Info::GetBitrate() {

   if(bIsVBitRate) {

      // Get average frame size by deviding fileSize by the number of frames
      float medFrameSize =(float)fileSize /(float)GetNumberOfFrames();

      /* Now using the formula for FrameSizes which looks different,
         depending on which mpeg version we're using, for mpeg v1:

         FrameSize = 12*  BitRate / SampleRate + Padding(if there is padding)

         for mpeg v2 the same thing is:

         FrameSize = 144*  BitRate / SampleRate + Padding(if there is padding)

         remember that bitrate is in kbps and sample rate in Hz, so we need to
         multiply our BitRate with 1000.

         For our purpose, just Getting the average frame size, will make the
         padding obsolete, so our formula looks like:

         FrameSize =(mpeg1?12:144)*  1000*  BitRate / SampleRate;
      */

      return((float)(
( medFrameSize* (float)header.GetFrequency() ) /
( 1000.0* ((header.GetLayerIndex()==3) ? 12.0 : 144.0))
                  ));

   }
   else return(float)(header.GetBitrate() );

}

int MP3Info::GetLengthInSeconds() {

   float f;

   // kiloBitFileSize to match kiloBitPerSecond in bitrate...
   float kiloBitFileSize =(8.0f* (float)fileSize) / 1000;

   f =((float)(kiloBitFileSize)) /((float)(GetBitrate()));

   return(int)(f);
}

void MP3Info::GetFormattedLength(char* input) {

   //  s  = complete number of seconds
   int s  = GetLengthInSeconds();

   //  ss = seconds to display
   int ss = s%60;

   //  m  = complete number of minutes
   int m  =(s-ss)/60;

   //  mm = minutes to display
   int mm = m%60;

   //  h  = complete number of hours
   int h =(m-mm)/60;

   char szTime[16]; // temporary string

   // make a "hh:mm:ss" if there is any hours, otherwise
   // make it   "mm:ss"
   if(h>0) sprintf(szTime,"%02d:%02d:%02d", h,mm,ss);
   else     sprintf(szTime,     "%02d:%02d",   mm,ss);

   // copy to the inputstring
   strcpy(input, szTime);

}

int MP3Info::GetNumberOfFrames() {

   if(!bIsVBitRate) {

      /* Now using the formula for FrameSizes which looks different,
         depending on which mpeg version we're using, for layer 1:

         FrameSize = 12*  BitRate / SampleRate + Padding(if there is padding)

         for layer 2 & 3 the same thing is:

         FrameSize = 144*  BitRate / SampleRate + Padding(if there is padding)

         remember that bitrate is in kbps and sample rate in Hz, so we need to
         multiply our BitRate with 1000.

         For our purpose, just Getting the average frame size, will make the
         padding obsolete, so our formula looks like:

         FrameSize =(layer1?12:144)*  1000*  BitRate / SampleRate;
      */

      float medFrameSize =(float)(
((header.GetLayerIndex()==3) ? 12 : 144 )* 
(
(1000.0* (float)header.GetBitrate() ) /
(float)header.GetFrequency()
            )
                                  );

      return(int)(fileSize/medFrameSize);

   }
   else return vbr.GetNumberOfFrames();

}

void MP3Info::GetVersion(char* input) {

   char versionchar[32]; // temporary string
   char tempchar2[4]; // layer
   int i;

   // call CFrameHeader member function
   float ver = header.GetVersion();

   // create the layer information with the amounts of I
   for(i=0; i<header.GetLayer(); i++ ) tempchar2[i] = 'I';
   tempchar2[i] = '\0';

   // combine strings
   sprintf(versionchar,"MPEG %g Layer %s",(double)ver, tempchar2);

   // copy result into inputstring
   strcpy(input, versionchar);

}

void MP3Info::GetMode(char* input) {

   char modechar[32]; // temporary string

   // call CFrameHeader member function
   header.GetMode(modechar);

   // copy result into inputstring
   strcpy(input, modechar);

}


/* ----------------------------------------------------------
   FrameHeader class is used to retrieve a MP3's FrameHeader
   and load that into a usable structure.

   This code will be well commented, so that everyone can
   understand, as it's made for the public and not for
   private use, although private use is allowed. :)

   all functions specified both in the header and .cpp file
   will have explanations in both locations.

   everything here by: Gustav "Grim Reaper" Munkby
http://home.swipnet.se/grd/
grd@swipnet.se
----------------------------------------------------------*/



// This function is quite easy to understand, it loads
// 4 chars of information into the FrameHeader class
// The validity is not tested, so qith this function
// an invalid FrameHeader could be retrieved
void MP3Info::FrameHeader::LoadHeader(char c[4]) {

   // this thing is quite interesting, it works like the following
   // c[0] = 00000011
   // c[1] = 00001100
   // c[2] = 00110000
   // c[3] = 11000000
   // the operator << means that we'll move the bits in that direction
   // 00000011 << 24 = 00000011000000000000000000000000
   // 00001100 << 16 =         000011000000000000000000
   // 00110000 << 24 =                 0011000000000000
   // 11000000       =                         11000000
   //                +_________________________________
   //                  00000011000011000011000011000000

   bithdr =(unsigned long)(
((c[0] & 255) << 24) |
((c[1] & 255) << 16) |
((c[2] & 255) <<  8) |
((c[3] & 255)      )
                           );

}


// This function is a supplement to the LoadHeader
// function, the only purpose is to detect if the
// header loaded by LoadHeader is a valid header
// or just four different chars
bool MP3Info::FrameHeader::IsValidHeader() {

   return(((GetFrameSync()      & 2047)==2047) &&
((GetVersionIndex()   &    3)!=   1) &&
((GetLayerIndex()     &    3)!=   0) &&
((GetBitrateIndex()   &   15)!=   0) &&  // due to lack of support of the .mp3 format
         // no "public" .mp3's should contain information
         // like this anyway... :)
((GetBitrateIndex()   &   15)!=  15) &&
((GetFrequencyIndex() &    3)!=   3) &&
((GetEmphasisIndex()  &    3)!=   2)    );

}

// this returns the MPEG version[1.0-2.5]
float MP3Info::FrameHeader::GetVersion() {

   // a table to convert the indexes into
   // something informative...
   float table[4] = {
      2.5, 0.0, 2.0, 1.0
   };

   // return modified value
   return table[GetVersionIndex()];

}


// this returns the Layer[1-3]
int MP3Info::FrameHeader::GetLayer() {

   // when speaking of layers there is a
   // cute coincidence, the Layer always
   // eauals 4 - layerIndex, so that's what
   // we will return
   return( 4 - GetLayerIndex() );

}


// this returns the current bitrate[8-448 kbps]
int MP3Info::FrameHeader::GetBitrate() {

   // a table to convert the indexes into
   // something informative...
   const int table[2][3][16] = {
      {         //MPEG 2 & 2.5
         {0,  8, 16, 24, 32, 40, 48, 56, 64, 80, 96,112,128,144,160,0}, //Layer III
         {0,  8, 16, 24, 32, 40, 48, 56, 64, 80, 96,112,128,144,160,0}, //Layer II
         {0, 32, 48, 56, 64, 80, 96,112,128,144,160,176,192,224,256,0}  //Layer I
      },{       //MPEG 1
         {0, 32, 40, 48, 56, 64, 80, 96,112,128,160,192,224,256,320,0}, //Layer III
         {0, 32, 48, 56, 64, 80, 96,112,128,160,192,224,256,320,384,0}, //Layer II
         {0, 32, 64, 96,128,160,192,224,256,288,320,352,384,416,448,0}  //Layer I
      }
   };

   // the bitrate is not only dependent of the bitrate index,
   // the bitrate also varies with the MPEG version and Layer version
   return table[(GetVersionIndex() & 1)][(GetLayerIndex() -1)][GetBitrateIndex()];

}


// this returns the current frequency[8000-48000 Hz]
int MP3Info::FrameHeader::GetFrequency() {

   // a table to convert the indexes into
   // something informative...
   int table[4][3] = {

      {32000, 16000,  8000}, //MPEG 2.5
      {    0,     0,     0}, //reserved
      {22050, 24000, 16000}, //MPEG 2
      {44100, 48000, 32000}  //MPEG 1

   };

   // the frequency is not only dependent of the bitrate index,
   // the bitrate also varies with the MPEG version
   return table[GetVersionIndex()][GetFrequencyIndex()];

}

// the purpose of GetMode is to Get information about
// the current playing mode, such as:
// "Joint Stereo"
void MP3Info::FrameHeader::GetMode(char* input) {

   // here you could use a array of strings instead
   // but I think this method is nicer, at least
   // when not dealing with that many variations
   switch(GetModeIndex()) {
      default:
         strcpy(input, "Stereo");
         break;

      case 1:
         strcpy(input, "Joint Stereo");
         break;

      case 2:
         strcpy(input, "Dual Channel");
         break;

      case 3:
         strcpy(input, "Single Channel");
         break;
   }

}

/* ----------------------------------------------------------
   CFrameHeader class is used to retrieve a VBR's Header
   and load that into a usable structure.

   This code will be well commented, so that everyone can
   understand, as it's made for the public and not for
   private use, although private use is allowed. :)

   all functions specified both in the header and .cpp file
   will have explanations in both locations.

   everything here by: Gustav "Grim Reaper" Munkby
http://home.swipnet.se/grd/
grd@swipnet.se
----------------------------------------------------------*/


// flags to know what information who could
// be found in the headers
#define FRAMES_FLAG    0x0001
#define BYTES_FLAG     0x0002
#define TOC_FLAG       0x0004
#define VBR_SCALE_FLAG 0x0008

// This function is quite easy to understand, it loads
// 12 chars of information into the VBitRate class
bool MP3Info::VBitRate::LoadHeader(char inputheader[12]) {

   // The Xing VBR headers always begin with the four
   // chars "Xing" so this tests wether we have a VBR
   // header or not
   if(memcmp(inputheader, "Xing", 4) ) {

      frames = -1;
      return false;

   }

   // now we will Get the flags and number of frames,
   // this is done in the same way as the FrameHeader
   // is generated in the CFrameHeader class
   // if you're curious about how it works, go and look
   // there

   // here we Get the flags from the next four bytes
   int flags =(int)(
((inputheader[4] & 255) << 24) |
((inputheader[5] & 255) << 16) |
((inputheader[6] & 255) <<  8) |
((inputheader[7] & 255)      )
                    );

   // if this tag contains the number of frames, load
   // that number into storage, if not something will
   // be wrong when calculating the bitrate and length
   // of the music
   if(flags & FRAMES_FLAG ) {
      frames =(int)(
((inputheader[ 8] & 255) << 24) |
((inputheader[ 9] & 255) << 16) |
((inputheader[10] & 255) <<  8) |
((inputheader[11] & 255)      )
                    );
   } else {

      // returning -1 so an error would be obvious
      // not many people would believe in a bitrate
      // -21 kbps :)
      frames = -1;

      // this function was returning false before
      // as there is an error occuring, but in that
      // case the bitrate wouldn't be unbelievable
      // so that's why I changed my mind and let it
      // return true instead
      return true;

   }

   // if it Gets this far, everything went according
   // to plans, so we should return true!
   return true;

}
