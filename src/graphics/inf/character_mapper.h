#ifndef ARK_GRAPHICS_INF_CHARACTER_MAPPER_H_
#define ARK_GRAPHICS_INF_CHARACTER_MAPPER_H_

#include "graphics/forwarding.h"

namespace ark {

class CharacterMapper {
public:
    virtual ~CharacterMapper() = default;

    virtual int32_t mapCharacter(int32_t c) = 0;
};

}

#endif
