#pragma once

#include "core/inf/builder.h"
#include "core/inf/runnable.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"

#include "app/inf/audio_player.h"


namespace ark::plugin::audio_android {

class AudioPlayerAndroid : public AudioPlayer {
public:
    AudioPlayerAndroid(const sp<ResourceLoaderContext>& resourceLoaderContext);

    sp<Future> play(const sp<Readable>& source, AudioFormat format, PlayOption options) override;
    bool isAudioFormatSupported(AudioFormat format) override;

//  [[plugin::resource-loader("audio-android")]]
    class BUILDER : public Builder<AudioPlayer> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        sp<AudioPlayer> build(const Scope& args) override;

    private:
        sp<ResourceLoaderContext> _resource_loader_context;

    };

//  [[plugin::resource-loader]]
    class BUILDER_DEFAULT : public Builder<AudioPlayer> {
    public:
        BUILDER_DEFAULT(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        sp<AudioPlayer> build(const Scope& args) override;

    private:
        BUILDER _delegate;

    };

private:
    sp<Executor> _executor;
};

}
