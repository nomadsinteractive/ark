#pragma once

#include "core/inf/builder.h"
#include "core/types/weak_ptr.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"

#include "app/inf/audio_player.h"


namespace ark {
namespace plugin {
namespace miniaudio {

class AudioPlayerMiniAudio : public AudioPlayer {
public:
    AudioPlayerMiniAudio(const sp<ResourceLoaderContext>& resourceLoaderContext);
    ~AudioPlayerMiniAudio() override;

    virtual sp<Future> play(const sp<Readable>& source, AudioFormat format, PlayOption options) override;
    virtual bool isAudioFormatSupported(AudioFormat format) override;

//  [[plugin::resource-loader("miniaudio")]]
    class BUILDER : public Builder<AudioPlayer> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<AudioPlayer> build(const Scope& args) override;

    private:
        sp<ResourceLoaderContext> _resource_loader_context;

    };

//  [[plugin::resource-loader]]
    class BUILDER_DEFAULT : public Builder<AudioPlayer> {
    public:
        BUILDER_DEFAULT(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<AudioPlayer> build(const Scope& args) override;

    private:
        BUILDER _delegate;

    };

public:
    class MADevice;

private:
    sp<Executor> _executor;

    WeakPtr<MADevice> _device;
};

}
}
}
