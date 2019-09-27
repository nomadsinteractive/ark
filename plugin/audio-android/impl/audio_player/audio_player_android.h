#ifndef ARK_PLUGIN_ANDROID_AUDIO_IMPL_AUDIO_PLAYER_AUDIO_PLAYER_ANDROID_H_
#define ARK_PLUGIN_ANDROID_AUDIO_IMPL_AUDIO_PLAYER_AUDIO_PLAYER_ANDROID_H_

#include "core/inf/builder.h"
#include "core/inf/runnable.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"

#include "app/inf/audio_player.h"


namespace ark {
namespace plugin {
namespace audio_android {

class AudioPlayerAndroid : public AudioPlayer {
public:
    AudioPlayerAndroid(const sp<ResourceLoaderContext>& resourceLoaderContext);

    virtual sp<Future> play(const sp<Readable>& source, AudioFormat format, PlayOption options) override;
    virtual bool isAudioFormatSupported(AudioFormat format) override;

//  [[plugin::resource-loader("audio-android")]]
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

private:
    sp<Executor> _executor;
};

}
}
}

#endif
