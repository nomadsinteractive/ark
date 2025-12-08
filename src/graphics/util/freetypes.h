#pragma once

#include <stdint.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "core/inf/readable.h"

#include "graphics/forwarding.h"

namespace ark {

class FreeTypes {
public:
    FreeTypes();
    ~FreeTypes();

    int ftNewFace(const char* filename, FT_Long face_index, FT_Face* aface);
    int ftNewFaceFromReadable(sp<Readable> readable, FT_Long face_index, FT_Face* aface);

    static FT_F26Dot6 ftF26Dot6(uint32_t integer, uint32_t fractional);
    static int32_t ftCalculateLineHeight(FT_Face face);
    static int32_t ftCalculateBaseLinePosition(FT_Face face);

private:
    FT_Library _library;
};

}
