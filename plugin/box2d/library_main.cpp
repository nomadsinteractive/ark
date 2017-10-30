#include "core/base/plugin.h"

#include "core/ark.h"
#include "core/base/api.h"
#include "core/base/plugin_manager.h"
#include "core/types/shared_ptr.h"

#include "generated/box2d_plugin.h"

using namespace ark;
using namespace ark::plugin::box2d;

extern "C" ARK_API void __ark_box2d_initialize__(Ark&, PluginManager& pm);

void __ark_box2d_initialize__(Ark& ark, PluginManager& pm)
{
    Ark::push(ark);
    pm.addPlugin(sp<Box2dPlugin>::make());
}
