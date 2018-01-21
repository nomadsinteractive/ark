#ifndef ARK_GRAPHICS_INF_ALPHABET_H_
#define ARK_GRAPHICS_INF_ALPHABET_H_

#include "graphics/forwarding.h"

#include "graphics/forwarding.h"

namespace ark {

class Alphabet {
public:
    virtual ~Alphabet() = default;

    struct Metrics {
        int32_t width;
        int32_t height;
        int32_t bitmap_width;
        int32_t bitmap_height;
        int32_t bitmap_x;
        int32_t bitmap_y;
    };

    virtual bool load(uint32_t c, Metrics& metrics, bool loadGlyph, bool hasFallback) = 0;
    virtual void draw(const bitmap& image, int32_t x, int32_t y) = 0;
};

}

#endif
