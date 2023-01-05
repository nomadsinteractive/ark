#include "core/base/plugin.h"

#include "core/ark.h"
#include "core/base/api.h"

#include "generated/yoga_plugin.h"

using namespace ark;
using namespace ark::plugin::yoga;

extern "C" ARK_API Plugin* __ark_yoga_initialize__(Ark&);

Plugin* __ark_yoga_initialize__(Ark& /*ark*/)
{
    return new YogaPlugin();
}
