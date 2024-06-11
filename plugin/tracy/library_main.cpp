#include "core/base/plugin.h"

#include "core/base/api.h"

#include "generated/tracy_plugin.h"

using namespace ark;
using namespace ark::plugin::tracy;

extern "C" ARK_API Plugin* __ark_tracy_initialize__(Ark&);

Plugin* __ark_tracy_initialize__(Ark& /*ark*/)
{
    return new TracyPlugin();
}
