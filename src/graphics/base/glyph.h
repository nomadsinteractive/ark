#ifndef ARK_GRAPHICS_BASE_GLYPH_H_
#define ARK_GRAPHICS_BASE_GLYPH_H_

#include "core/base/api.h"

#include "graphics/base/v3.h"

namespace ark {

class ARK_API Glyph {
public:
    Glyph(int32_t character, const V3& position, const V2& size);

//  [[script::bindings::property]]
    int32_t character() const;

//  [[script::bindings::property]]
    const V3& position() const;

//  [[script::bindings::property]]
    const V2& size() const;

private:
    int32_t _character;
    V3 _position;
    V2 _size;
};

}

#endif
