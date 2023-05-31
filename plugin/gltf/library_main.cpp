#include "core/base/plugin.h"

#include "core/ark.h"
#include "core/base/api.h"

#include "generated/gltf_plugin.h"

using namespace ark;
using namespace ark::plugin::gltf;

extern "C" ARK_API Plugin* __ark_gltf_initialize__(Ark&);

Plugin* __ark_gltf_initialize__(Ark& ark)
{
    return new GltfPlugin();
}
