#include "core/base/plugin.h"

#include "core/base/api.h"
#include "core/base/plugin_manager.h"
#include "core/types/shared_ptr.h"

#include "generated/noise_plugin.h"

using namespace ark;

extern "C" ARK_API Plugin* __ark_noise_initialize__(Ark&);

Plugin* __ark_noise_initialize__(Ark&)
{
    return new plugin::noise::NoisePlugin();
}
