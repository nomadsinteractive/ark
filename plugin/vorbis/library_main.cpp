#include "core/base/api.h"

#include "core/ark.h"
#include "generated/vorbis_plugin.h"

using namespace ark;
using namespace ark::plugin::vorbis;

extern "C" ARK_API Plugin* __ark_vorbis_initialize__(Ark&);

Plugin* __ark_vorbis_initialize__(Ark& ark)
{
    return new VorbisPlugin();
}
