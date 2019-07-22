#include "core/base/plugin.h"

#include "core/ark.h"
#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "generated/miniaudio_plugin.h"

using namespace ark;
using namespace ark::plugin::miniaudio;

extern "C" ARK_API Plugin* __ark_miniaudio_initialize__(Ark&);

Plugin* __ark_miniaudio_initialize__(Ark& /*ark*/)
{
    return new MiniaudioPlugin();
}
