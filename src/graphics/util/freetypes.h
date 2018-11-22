#ifndef ARK_RENDERER_OPENGL_UTIL_FREETYPES_H_
#define ARK_RENDERER_OPENGL_UTIL_FREETYPES_H_

#include <stdint.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "core/base/api.h"
#include "core/types/shared_ptr.h"
#include "core/inf/readable.h"

#include "graphics/forwarding.h"

namespace ark {

class FreeTypes {
public:
    FreeTypes();
    ~FreeTypes();

    int ftNewFace(const char* filepathname, FT_Long face_index, FT_Face* aface);
    int ftNewFaceFromReadable(const sp<Readable>& readable, FT_Long face_index, FT_Face* aface);

    static FT_F26Dot6 ftF26Dot6(uint32_t integer, uint32_t fractional);
    static uint32_t ftCalculateLineHeight(FT_Face face);
    static int32_t ftCalculateBaseLinePosition(FT_Face face);

private:
    class FTReadableStream {
    public:
        FTReadableStream(const sp<Readable>& readable);
        sp<Readable> _readable;
    };

private:
    static unsigned long ftStreamIOFunc(FT_Stream stream, unsigned long offset, unsigned char*  buffer, unsigned long count);
    static void ftStreamCloseFunc(FT_Stream stream);

private:
    FT_Library _library;
};

}

#endif
