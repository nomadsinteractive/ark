#include "core/base/plugin.h"

#include "core/ark.h"
#include "core/base/api.h"

#include "generated/bgfx_plugin.h"

using namespace ark;
using namespace ark::plugin::bgfx;

extern "C" ARK_API Plugin* __ark_yoga_initialize__(Ark&);

Plugin* __ark_yoga_initialize__(Ark& /*ark*/)
{
    return new BgfxPlugin();
}
