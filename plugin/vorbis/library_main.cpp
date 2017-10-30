#include "core/base/plugin.h"

#include "core/ark.h"
#include "core/base/api.h"
#include "core/base/plugin_manager.h"
#include "core/types/shared_ptr.h"

#include "generated/vorbis_plugin.h"

using namespace ark;
using namespace ark::plugin::vorbis;

extern "C" ARK_API void __ark_vorbis_initialize__(Ark& ark, PluginManager& pm);

void __ark_vorbis_initialize__(Ark& ark, PluginManager& pm)
{
    Ark::push(ark);
    pm.addPlugin(sp<VorbisPlugin>::make());
}
