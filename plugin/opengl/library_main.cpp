#include "core/base/plugin.h"

#include "core/ark.h"
#include "core/base/api.h"

#include "generated/opengl_plugin.h"

using namespace ark;
using namespace ark::plugin::opengl;

extern "C" ARK_API Plugin* __ark_opengl_initialize__(Ark&);

Plugin* __ark_opengl_initialize__(Ark& /*ark*/)
{
    return new OpenglPlugin();
}
