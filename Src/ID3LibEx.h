#ifndef ID3_LIB_EX_H
#define ID3_LIB_EX_H

#include <id3/tag.h>
#include <id3/misc_support.h>

extern char* ID3_GetCopyright(const ID3_Tag* tag);
extern size_t ID3_RemoveCopyright(ID3_Tag* tag);
extern ID3_Frame* ID3_AddCopyright(ID3_Tag* tag, const char* text, bool replace);

extern char* ID3_GetPublisher(const ID3_Tag* tag);
extern size_t ID3_RemovePublisher(ID3_Tag* tag);
extern ID3_Frame* ID3_AddPublisher(ID3_Tag* tag, const char* text, bool replace);

extern char* ID3_GetReleaseDate(const ID3_Tag* tag);
extern size_t ID3_RemoveReleaseDate(ID3_Tag* tag);
extern ID3_Frame* ID3_AddReleaseDate(ID3_Tag* tag, const char* text, bool replace);

extern char* ID3_GetEncodedBy(const ID3_Tag* tag);
extern size_t ID3_RemoveEncodedBy(ID3_Tag* tag);
extern ID3_Frame* ID3_AddEncodedBy(ID3_Tag* tag, const char* text, bool replace);

extern size_t ID3_GetTraktorRating(const ID3_Tag* tag);
extern size_t ID3_RemoveTraktorRating(ID3_Tag* tag);
extern ID3_Frame* ID3_AddTraktorRating(ID3_Tag* tag, size_t rating, bool replace);

#endif // ID3_LIB_EX_H
