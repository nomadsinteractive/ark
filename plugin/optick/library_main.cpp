#include "core/base/plugin.h"

#include "core/ark.h"
#include "core/base/api.h"

#include "generated/optick_plugin.h"

using namespace ark;
using namespace ark::plugin::optick;

extern "C" ARK_API Plugin* __ark_optick_initialize__(Ark&);

Plugin* __ark_optick_initialize__(Ark& /*ark*/)
{
    return new OptickPlugin();
}
