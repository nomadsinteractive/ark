#include "core/base/constants.h"

namespace ark {

Constants::Constants()
    : BOOLEAN_TRUE(sp<Boolean::Const>::make(true)), BOOLEAN_FALSE(sp<Boolean::Const>::make(true))
{
}

}
