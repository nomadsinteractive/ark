#ifndef ARK_APP_INF_AUDIO_PLAYER_H_
#define ARK_APP_INF_AUDIO_PLAYER_H_

#include "core/base/api.h"
#include "core/forwarding.h"

namespace ark {

class ARK_API AudioPlayer {
public:
//  [[script::bindings::enumeration]]
    enum PlayOption {
        PLAY_OPTION_DEFAULT = -1,
        PLAY_OPTION_LOOP_OFF = 0,
        PLAY_OPTION_LOOP_ON = 1
    };

    enum AudioFormat {
        AUDIO_FORMAT_AUTO,
        AUDIO_FORMAT_OGG,
        AUDIO_FORMAT_WAV,
        AUDIO_FORMAT_PCM
    };

    virtual ~AudioPlayer() = default;

    virtual sp<Future> play(const sp<Readable>& source, AudioFormat format, PlayOption options) = 0;
    virtual bool isAudioFormatSupported(AudioFormat format) = 0;

public:
//[[script::bindings::classmethod]]
    static sp<Future> play(const sp<AudioPlayer>& self, const String& src, AudioPlayer::PlayOption options = AudioPlayer::PLAY_OPTION_DEFAULT);

private:

};

}

#endif
