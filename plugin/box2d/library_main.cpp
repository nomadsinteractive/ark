#include "core/base/plugin.h"

#include "core/ark.h"
#include "core/base/api.h"
#include "core/base/plugin_manager.h"
#include "core/types/shared_ptr.h"

#include "generated/box2d_plugin.h"

using namespace ark;
using namespace ark::plugin::box2d;

extern "C" ARK_API Plugin* __ark_box2d_initialize__(Ark&);

Plugin* __ark_box2d_initialize__(Ark& ark)
{
    return new Box2dPlugin();
}
