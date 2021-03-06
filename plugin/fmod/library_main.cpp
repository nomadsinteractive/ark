#include "core/base/plugin.h"

#include "core/ark.h"
#include "core/base/api.h"
#include "core/base/plugin_manager.h"
#include "core/types/shared_ptr.h"

#include "generated/fmod_plugin.h"

using namespace ark;
using namespace ark::plugin::fmod;

extern "C" ARK_API Plugin* __ark_fmod_initialize__(Ark&);

Plugin* __ark_fmod_initialize__(Ark& /*ark*/)
{
    return new FmodPlugin();
}
