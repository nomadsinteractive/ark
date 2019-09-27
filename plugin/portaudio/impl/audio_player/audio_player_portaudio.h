#ifndef ARK_PLUGIN_PORTAUDIO_IMPL_AUDIO_PLAYER_AUDIO_PLAYER_PORTAUDIO_H_
#define ARK_PLUGIN_PORTAUDIO_IMPL_AUDIO_PLAYER_AUDIO_PLAYER_PORTAUDIO_H_

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"
#include "core/types/weak_ptr.h"

#include "renderer/forwarding.h"

#include "app/inf/audio_player.h"

namespace ark {
namespace plugin {
namespace portaudio {

class PortaudioDeviceStream;

class AudioPlayerPortaudio : public AudioPlayer {
public:
    AudioPlayerPortaudio(const sp<Executor>& threadPoolExecutor);
    ~AudioPlayerPortaudio();

    virtual sp<Future> play(const sp<Readable>& source, AudioFormat format, PlayOption options) override;
    virtual bool isAudioFormatSupported(AudioFormat format) override;

//  [[plugin::resource-loader("portaudio")]]
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
    class Stub;
    friend class PortaudioDeviceStream;

private:
    sp<Stub> _stub;
    sp<Executor> _executor;
    WeakPtr<PortaudioDeviceStream> _pa_stream;
};

}
}
}

#endif
