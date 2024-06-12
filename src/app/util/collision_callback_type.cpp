#include "collision_callback_type.h"

#include "core/base/string.h"

namespace ark {

sp<CollisionCallback> CollisionCallbackType::create(sp<CollisionCallback> value)
{
    return value;
}

sp<CollisionCallback> CollisionCallbackType::create(String value)
{
    return nullptr;
}

}
