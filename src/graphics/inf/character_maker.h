#ifndef ARK_GRAPHICS_INF_CHARACTER_MAKER_H_
#define ARK_GRAPHICS_INF_CHARACTER_MAKER_H_

#include "graphics/forwarding.h"
#include "graphics/base/v2.h"

namespace ark {

class CharacterMaker {
public:
    virtual ~CharacterMaker() = default;

    virtual sp<RenderObject> makeCharacter(int32_t type, const V3& position, const sp<Size>& size) = 0;
    virtual V2 scale() = 0;

};

}

#endif
