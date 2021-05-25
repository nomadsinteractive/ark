#ifndef ARK_GRAPHICS_INF_CHARACTER_MAKER_H_
#define ARK_GRAPHICS_INF_CHARACTER_MAKER_H_

#include <vector>

#include "graphics/forwarding.h"
#include "graphics/base/v2.h"

namespace ark {

class CharacterMaker {
public:
    virtual ~CharacterMaker() = default;

    virtual std::vector<sp<RenderObject>> makeCharacter(const std::vector<Glyph>& glyphs) = 0;
    virtual V2 scale() = 0;

};

}

#endif
