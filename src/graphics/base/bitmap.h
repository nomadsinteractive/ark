#ifndef ARK_GRAPHICS_BASE_BITMAP_H_
#define ARK_GRAPHICS_BASE_BITMAP_H_

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

class ARK_API Bitmap {
public:
//  [[script::bindings::auto]]
    Bitmap(uint32_t width, uint32_t height, uint32_t rowBytes, uint8_t channels);
    Bitmap(uint32_t width, uint32_t height, uint32_t rowBytes, uint8_t channels, const bytearray& bytes);
    ~Bitmap();

//  [[script::bindings::property]]
    uint32_t width() const;
//  [[script::bindings::property]]
    uint32_t height() const;
//  [[script::bindings::property]]
    uint8_t channels() const;

//  [[script::bindings::property]]
    uint32_t rowBytes() const;

//  [[script::bindings::property]]
    const sp<Array<uint8_t>>& bytes() const;

    uint8_t* at(uint32_t x, uint32_t y) const;

    void draw(void* buf, uint32_t width, uint32_t height, int32_t x, int32_t y, int32_t stride);

private:
    uint32_t _width;
    uint32_t _height;
    uint32_t _row_bytes;
    uint8_t _channels;

    bytearray _bytes;
};

}


#endif
