#ifndef ARK_PLUGIN_FMOD_IMPL_AUDIO_PLAYER_AUDIO_PLAYER_FMOD_H_
#define ARK_PLUGIN_FMOD_IMPL_AUDIO_PLAYER_AUDIO_PLAYER_FMOD_H_

#include "core/inf/builder.h"
#include "core/inf/runnable.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"

#include "app/inf/audio_player.h"

#include <fmod.hpp>

namespace ark {
namespace plugin {
namespace fmod {

class AudioPlayerFMOD : public AudioPlayer {
public:
    AudioPlayerFMOD(const sp<ResourceLoaderContext>& resourceLoaderContext);

    virtual sp<Future> play(const sp<Readable>& source, AudioFormat format, PlayOption options) override;
    virtual bool isAudioFormatSupported(AudioFormat format) override;

//  [[plugin::resource-loader("fmod")]]
    class BUILDER : public Builder<AudioPlayer> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<AudioPlayer> build(const sp<Scope>& args) override;

    private:
        sp<ResourceLoaderContext> _resource_loader_context;

    };

private:
    class FMODSystem : public Runnable {
    public:
        FMODSystem();
        ~FMODSystem() override;

        virtual void run() override;

        FMOD::System* instance() const;

    private:
        FMOD::System* _instance;

    };

private:
    sp<Executor> _executor;

    sp<FMODSystem> _system;


};

}
}
}

#endif
