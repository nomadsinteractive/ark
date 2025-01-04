#pragma once

#include "core/inf/builder.h"
#include "core/types/weak_ptr.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"

#include "app/inf/audio_player.h"


namespace ark::plugin::miniaudio {

class AudioPlayerMiniAudio final : public AudioPlayer {
public:
    AudioPlayerMiniAudio();

    sp<Future> play(const sp<Readable>& source, AudioFormat format, PlayOption options) override;
    bool isAudioFormatSupported(AudioFormat format) override;

//  [[plugin::builder("miniaudio")]]
    class BUILDER final : public Builder<AudioPlayer> {
    public:
        BUILDER() = default;

        sp<AudioPlayer> build(const Scope& args) override;

    };

//  [[plugin::builder]]
    class BUILDER_DEFAULT final : public Builder<AudioPlayer> {
    public:
        BUILDER_DEFAULT() = default;

        sp<AudioPlayer> build(const Scope& args) override;

    };

public:
    class MADevice;

private:
    sp<Executor> _executor;

    WeakPtr<MADevice> _device;
};

}
