#ifndef ARK_APP_INF_AUDIO_PLAYER_H_
#define ARK_APP_INF_AUDIO_PLAYER_H_

#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

namespace ark {

class AudioPlayer {
public:
    virtual ~AudioPlayer() = default;

    virtual sp<Future> play(const sp<Readable>& source) = 0;
};

}

#endif
