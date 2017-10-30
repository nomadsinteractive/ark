#include "core/base/plugin.h"

#include "core/base/api.h"
#include "core/base/plugin_manager.h"
#include "core/types/shared_ptr.h"

using namespace ark;

extern "C" ARK_API void __ark_noise_initialize__(Ark&, PluginManager& pm);

void __ark_noise_initialize__(Ark&, PluginManager& pm)
{
}
