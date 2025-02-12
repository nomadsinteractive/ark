#include "core/base/plugin.h"

#include "core/ark.h"
#include "core/base/api.h"

#include "generated/sdl3_plugin.h"

using namespace ark;
using namespace ark::plugin::sdl3;

extern "C" ARK_API Plugin* __ark_sdl3_initialize__(Ark&);

Plugin* __ark_sdl3_initialize__(Ark& /*ark*/)
{
    return new Sdl3Plugin();
}
