#ifndef ARK_GRAPHICS_INF_ALPHABET_H_
#define ARK_GRAPHICS_INF_ALPHABET_H_

#include "graphics/forwarding.h"

#include "graphics/forwarding.h"

namespace ark {

class Alphabet {
public:
    virtual ~Alphabet() = default;

    virtual bool load(uint32_t c, uint32_t& width, uint32_t& height, bool loadGlyph, bool hasFallback) = 0;
    virtual void draw(const bitmap& image, int32_t x, int32_t y) = 0;
};

}

#endif
