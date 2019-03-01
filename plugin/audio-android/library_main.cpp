#include "core/base/plugin.h"

#include "core/ark.h"
#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "generated/audio_android_plugin.h"

using namespace ark;
using namespace ark::plugin::audio_android;

extern "C" ARK_API Plugin* __ark_audio_android_initialize__(Ark&);

Plugin* __ark_audio_android_initialize__(Ark& /*ark*/)
{
    return new AudioAndroidPlugin();
}
