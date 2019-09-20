#ifndef ARK_GRAPHICS_INF_CHARACTER_MAKER_H_
#define ARK_GRAPHICS_INF_CHARACTER_MAKER_H_

#include "graphics/forwarding.h"

namespace ark {

class CharacterMaker {
public:
    virtual ~CharacterMaker() = default;

    virtual sp<RenderObject> makeCharacter(int32_t type, const V2& position, const sp<Size>& size) = 0;
};

}

#endif
