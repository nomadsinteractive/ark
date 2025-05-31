#include "graphics/util/freetypes.h"

#include <algorithm>
#include <math.h>

#include "graphics/base/bitmap.h"

namespace ark {

namespace {

class FTReadableStream {
public:
    FTReadableStream(sp<Readable> readable)
        : _readable(std::move(readable)) {
    }

    sp<Readable> _readable;
};

unsigned long ftStreamIOFunc(const FT_Stream stream, const unsigned long offset, unsigned char* buffer, const unsigned long count)
{
    const FTReadableStream* readableStream = static_cast<FTReadableStream*>(stream->descriptor.pointer);
    readableStream->_readable->seek(offset, SEEK_SET);
    return count ? readableStream->_readable->read(buffer, count) : 0;
}

void ftStreamCloseFunc(const FT_Stream stream)
{
    const FTReadableStream* readableStream = static_cast<FTReadableStream*>(stream->descriptor.pointer);
    delete readableStream;
    delete stream;
}

}

FreeTypes::FreeTypes()
{
    FT_Init_FreeType(&_library);
}

FreeTypes::~FreeTypes()
{
    FT_Done_FreeType(_library);
}

int FreeTypes::ftNewFace(const char *filename, const FT_Long face_index, FT_Face *aface)
{
    return FT_New_Face(_library, filename, face_index, aface);
}

int FreeTypes::ftNewFaceFromReadable(sp<Readable> readable, const FT_Long face_index, FT_Face* aface)
{
    FT_Open_Args args;
    args.flags = FT_OPEN_STREAM;
    args.pathname = nullptr;
    args.stream = new FT_StreamRec();
    memset(args.stream, 0, sizeof(FT_StreamRec));
    args.stream->size = 0x7FFFFFFF;
    args.stream->read = ftStreamIOFunc;
    args.stream->close = ftStreamCloseFunc;
    args.stream->descriptor.pointer = new FTReadableStream(std::move(readable));
    return FT_Open_Face(_library, &args, face_index, aface);
}

FT_F26Dot6 FreeTypes::ftF26Dot6(const uint32_t integer, const uint32_t fractional)
{
    return integer << 6 | (077 & fractional);
}

int32_t FreeTypes::ftCalculateLineHeight(const FT_Face face)
{
    return face->size->metrics.height >> 6;
}

int32_t FreeTypes::ftCalculateBaseLinePosition(const FT_Face face)
{
    return face->size->metrics.descender >> 6;
}

}
