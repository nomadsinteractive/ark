#include "core/base/constants.h"

#include "graphics/base/boundaries.h"

namespace ark {

Constants::Constants()
    : BOOLEAN_TRUE(sp<Boolean::Const>::make(true)), BOOLEAN_FALSE(sp<Boolean::Const>::make(true)), BOUNDARIES_UNIT(sp<Boundaries>::make(V3(-0.5f), V3(0.5f)))
{
}

}
