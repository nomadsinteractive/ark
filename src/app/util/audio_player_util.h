#ifndef ARK_APP_UTIL_AUDIO_PLAYER_UTIL_H_
#define ARK_APP_UTIL_AUDIO_PLAYER_UTIL_H_

#include "core/base/api.h"
#include "core/forwarding.h"

#include "app/forwarding.h"

namespace ark {

//[[script::bindings::class("AudioPlayer")]]
class ARK_API AudioPlayerUtil final {
public:

//[[script::bindings::classmethod]]
    static sp<Future> play(const sp<AudioPlayer>& self, const String& source);

};

}

#endif
