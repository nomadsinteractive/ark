#ifndef ARK_GRAPHICS_INF_GLYPH_MAKER_H_
#define ARK_GRAPHICS_INF_GLYPH_MAKER_H_

#include <vector>
#include <string>

#include "graphics/forwarding.h"

namespace ark {

class GlyphMaker {
public:
    virtual ~GlyphMaker() = default;

    virtual std::vector<sp<Glyph>> makeGlyphs(const std::wstring& text) = 0;

};

}

#endif
