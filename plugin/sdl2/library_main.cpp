#include "core/base/plugin.h"

#include "core/ark.h"
#include "core/base/api.h"

#include "generated/sdl2_plugin.h"

using namespace ark;
using namespace ark::plugin::sdl2;

extern "C" ARK_API Plugin* __ark_sdl2_initialize__(Ark&);

Plugin* __ark_sdl2_initialize__(Ark& /*ark*/)
{
    return new Sdl2Plugin();
}
