#include "AristophanesPch.h"

#include "ID3LibEx.h"

char* ID3_GetCopyright(const ID3_Tag* tag)
{
    char* sCopyright = NULL;
    if(NULL == tag)
    {
        return sCopyright;
    }

    ID3_Frame* frame = tag->Find(ID3FID_COPYRIGHT);
    if(frame != NULL)
    {
        sCopyright = ID3_GetString(frame, ID3FN_TEXT);
    }
    return sCopyright;
}

size_t ID3_RemoveCopyright(ID3_Tag* tag)
{
    size_t num_removed = 0;
    ID3_Frame* frame = NULL;

    if(NULL == tag)
    {
        return num_removed;
    }

    frame = tag->Find(ID3FID_COPYRIGHT);

    while(frame)
    {
        frame = tag->RemoveFrame(frame);
        delete frame;
        num_removed++;
        frame = tag->Find(ID3FID_COPYRIGHT);
    }

    return num_removed;
}

ID3_Frame* ID3_AddCopyright(ID3_Tag* tag, const char* text, bool replace)
{
    ID3_Frame* frame = NULL;
    if(NULL != tag && NULL != text && strlen(text) > 0)
    {
        if(replace)
        {
            ID3_RemoveCopyright(tag);
        }
        if(replace || tag->Find(ID3FID_COPYRIGHT) == NULL)
        {
            frame = new ID3_Frame(ID3FID_COPYRIGHT);
            if(frame)
            {
                frame->Field(ID3FN_TEXT) = text;
                tag->AttachFrame(frame);
            }
        }
    }

    return frame;
}

char* ID3_GetPublisher(const ID3_Tag* tag)
{
    char* sPublisher = NULL;
    if(NULL == tag)
    {
        return sPublisher;
    }

    ID3_Frame* frame = tag->Find(ID3FID_PUBLISHER);
    if(frame != NULL)
    {
        sPublisher = ID3_GetString(frame, ID3FN_TEXT);
    }
    return sPublisher;
}

size_t ID3_RemovePublisher(ID3_Tag* tag)
{
    size_t num_removed = 0;
    ID3_Frame* frame = NULL;

    if(NULL == tag)
    {
        return num_removed;
    }

    frame = tag->Find(ID3FID_PUBLISHER);

    while(frame)
    {
        frame = tag->RemoveFrame(frame);
        delete frame;
        num_removed++;
        frame = tag->Find(ID3FID_PUBLISHER);
    }

    return num_removed;
}

ID3_Frame* ID3_AddPublisher(ID3_Tag* tag, const char* text, bool replace)
{
    ID3_Frame* frame = NULL;
    if(NULL != tag && NULL != text && strlen(text) > 0)
    {
        if(replace)
        {
            ID3_RemovePublisher(tag);
        }
        if(replace || tag->Find(ID3FID_PUBLISHER) == NULL)
        {
            frame = new ID3_Frame(ID3FID_PUBLISHER);
            if(frame)
            {
                frame->Field(ID3FN_TEXT) = text;
                tag->AttachFrame(frame);
            }
        }
    }

    return frame;
}

char* ID3_GetComposer(const ID3_Tag* tag)
{
    char* sComposer = NULL;
    if(NULL == tag)
    {
        return sComposer;
    }

    ID3_Frame* frame = tag->Find(ID3FID_COMPOSER);
    if(frame != NULL)
    {
        sComposer = ID3_GetString(frame, ID3FN_TEXT);
    }
    return sComposer;
}

size_t ID3_RemoveComposer(ID3_Tag* tag)
{
    size_t num_removed = 0;
    ID3_Frame* frame = NULL;

    if(NULL == tag)
    {
        return num_removed;
    }

    frame = tag->Find(ID3FID_COMPOSER);

    while(frame)
    {
        frame = tag->RemoveFrame(frame);
        delete frame;
        num_removed++;
        frame = tag->Find(ID3FID_COMPOSER);
    }

    return num_removed;
}

ID3_Frame* ID3_AddComposer(ID3_Tag* tag, const char* text, bool replace)
{
    ID3_Frame* frame = NULL;
    if(NULL != tag && NULL != text && strlen(text) > 0)
    {
        if(replace)
        {
            ID3_RemoveComposer(tag);
        }
        if(replace || tag->Find(ID3FID_COMPOSER) == NULL)
        {
            frame = new ID3_Frame(ID3FID_COMPOSER);
            if(frame)
            {
                frame->Field(ID3FN_TEXT) = text;
                tag->AttachFrame(frame);
            }
        }
    }

    return frame;
}

char* ID3_GetReleaseDate(const ID3_Tag* tag)
{
    char* sReleaseDate = NULL;
    if(NULL == tag)
    {
        return sReleaseDate;
    }

    ID3_Frame* frame = tag->Find(ID3FID_DATE);
    if(frame != NULL)
    {
        sReleaseDate = ID3_GetString(frame, ID3FN_TEXT);
    }
    return sReleaseDate;
}

size_t ID3_RemoveReleaseDate(ID3_Tag* tag)
{
    size_t num_removed = 0;
    ID3_Frame* frame = NULL;

    if(NULL == tag)
    {
        return num_removed;
    }

    frame = tag->Find(ID3FID_DATE);

    while(frame)
    {
        frame = tag->RemoveFrame(frame);
        delete frame;
        num_removed++;
        frame = tag->Find(ID3FID_DATE);
    }

    return num_removed;
}

ID3_Frame* ID3_AddReleaseDate(ID3_Tag* tag, const char* text, bool replace)
{
    ID3_Frame* frame = NULL;
    if(NULL != tag && NULL != text && strlen(text) > 0)
    {
        if(replace)
        {
            ID3_RemoveReleaseDate(tag);
        }
        if(replace || tag->Find(ID3FID_DATE) == NULL)
        {
            frame = new ID3_Frame(ID3FID_DATE);
            if(frame)
            {
                frame->Field(ID3FN_TEXT) = text;
                tag->AttachFrame(frame);
            }
        }
    }

    return frame;
}

char* ID3_GetEncodedBy(const ID3_Tag* tag)
{
    char* sEncodedBy = NULL;
    if(NULL == tag)
    {
        return sEncodedBy;
    }

    ID3_Frame* frame = tag->Find(ID3FID_ENCODEDBY);
    if(frame != NULL)
    {
        sEncodedBy = ID3_GetString(frame, ID3FN_TEXT);
    }
    return sEncodedBy;
}

size_t ID3_RemoveEncodedBy(ID3_Tag* tag)
{
    size_t num_removed = 0;
    ID3_Frame* frame = NULL;

    if(NULL == tag)
    {
        return num_removed;
    }

    frame = tag->Find(ID3FID_ENCODEDBY);

    while(frame)
    {
        frame = tag->RemoveFrame(frame);
        delete frame;
        num_removed++;
        frame = tag->Find(ID3FID_ENCODEDBY);
    }

    return num_removed;
}

ID3_Frame* ID3_AddEncodedBy(ID3_Tag* tag, const char* text, bool replace)
{
    ID3_Frame* frame = NULL;
    if(NULL != tag && NULL != text && strlen(text) > 0)
    {
        if(replace)
        {
            ID3_RemoveEncodedBy(tag);
        }
        if(replace || tag->Find(ID3FID_ENCODEDBY) == NULL)
        {
            frame = new ID3_Frame(ID3FID_ENCODEDBY);
            if(frame)
            {
                frame->Field(ID3FN_TEXT) = text;
                tag->AttachFrame(frame);
            }
        }
    }

    return frame;
}

static const char* TRAKTOR_EMAIL_ADDR = "traktor@native-instruments.de";

static size_t StarsToTraktorRating(size_t stars)
{
    return(stars * 51);
}

static size_t TraktorToStarsRating(size_t stars)
{
    return(stars / 51);
}

size_t ID3_GetTraktorRating(const ID3_Tag* tag)
{
    if(NULL == tag)
    {
        return 0;
    }

    ID3_Frame* frame = tag->Find(ID3FID_POPULARIMETER, ID3FN_EMAIL, TRAKTOR_EMAIL_ADDR);
    if(frame != NULL)
    {
        size_t rating = TraktorToStarsRating(frame->GetField(ID3FN_RATING)->Get());
        return rating;
    }
    return 0;
}

size_t ID3_RemoveTraktorRating(ID3_Tag* tag)
{
    size_t num_removed = 0;
    ID3_Frame* frame = NULL;

    if(NULL == tag)
    {
        return num_removed;
    }

    frame = tag->Find(ID3FID_POPULARIMETER, ID3FN_EMAIL, TRAKTOR_EMAIL_ADDR);

    while(frame)
    {
        frame = tag->RemoveFrame(frame);
        delete frame;
        num_removed++;
        frame = tag->Find(ID3FID_POPULARIMETER, ID3FN_EMAIL, TRAKTOR_EMAIL_ADDR);
    }

    return num_removed;

}

ID3_Frame* ID3_AddTraktorRating(ID3_Tag* tag, size_t rating, bool replace)
{
    ID3_Frame* frame = NULL;
    if(NULL != tag)
    {
        if(replace)
        {
            ID3_RemoveTraktorRating(tag);
        }
        if(replace || tag->Find(ID3FID_POPULARIMETER, ID3FN_EMAIL, TRAKTOR_EMAIL_ADDR) == NULL)
        {
            frame = new ID3_Frame(ID3FID_POPULARIMETER);
            if(frame)
            {
                frame->Field(ID3FN_EMAIL) = TRAKTOR_EMAIL_ADDR;
                frame->Field(ID3FN_RATING) = uint32(StarsToTraktorRating(rating));
                frame->Field(ID3FN_COUNTER) = uint32(0);
                tag->AttachFrame(frame);
            }
        }
    }
    
    return frame;
}
