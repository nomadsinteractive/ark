#pragma once

#include <string>

#include "graphics/forwarding.h"

namespace ark {

class GlyphMaker {
public:
    virtual ~GlyphMaker() = default;

    virtual Vector<sp<Glyph>> makeGlyphs(const std::wstring& text) = 0;

};

}
