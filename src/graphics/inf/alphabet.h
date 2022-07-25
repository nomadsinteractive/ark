#ifndef ARK_GRAPHICS_INF_ALPHABET_H_
#define ARK_GRAPHICS_INF_ALPHABET_H_

#include "graphics/forwarding.h"
#include "graphics/base/font.h"

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

    virtual void setTextSize(const Font::TextSize& size) = 0;

    virtual bool measure(int32_t c, Metrics& metrics, bool hasFallback) = 0;
    virtual bool draw(uint32_t c, Bitmap& image, int32_t x, int32_t y) = 0;
};

}

#endif
