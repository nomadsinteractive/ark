#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/bit_set.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API AudioPlayer {
public:
//  [[script::bindings::enumeration]]
    enum PlayOptionBits {
        PLAY_OPTION_LOOP = 1
    };
    typedef BitSet<PlayOptionBits> PlayOption;

    enum AudioFormat {
        AUDIO_FORMAT_AUTO,
        AUDIO_FORMAT_OGG,
        AUDIO_FORMAT_WAV,
        AUDIO_FORMAT_PCM
    };

    virtual ~AudioPlayer() = default;

    virtual sp<Future> play(sp<Readable> source, sp<Future> future, AudioFormat format, PlayOption options) = 0;
    virtual bool isAudioFormatSupported(AudioFormat format) = 0;

//  [[script::bindings::classmethod]]
    static sp<Future> play(const sp<AudioPlayer>& self, const String& src, sp<Future> future = nullptr, AudioPlayer::PlayOption options = {});
};

}
