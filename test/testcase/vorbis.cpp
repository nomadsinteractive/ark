#include "test/base/test_case.h"

#include <chrono>
#include <thread>

#include "core/base/api.h"
#include "core/base/bean_factory.h"
#include "core/base/future.h"
#include "core/base/plugin_manager.h"
#include "core/base/thread.h"
#include "core/inf/readable.h"
#include "core/inf/runnable.h"
#include "core/types/global.h"

#include "core/base/resource_loader.h"
#include "app/base/application_context.h"
#include "app/inf/audio_player.h"

#include "platform/platform.h"


namespace ark {
namespace unittest {

class VorbisTestCase : public TestCase {
public:
    virtual int launch() override {
        const sp<ResourceLoader> resourceLoader = getResourceLoader();
        const Scope args;
        const sp<AudioPlayer> portaudioPlayer = resourceLoader->load<AudioPlayer>("audio_player", args);
        const sp<Future> f1 = AudioPlayer::play(portaudioPlayer, "test.ogg", AudioPlayer::PLAY_OPTION_DEFAULT);
        std::this_thread::sleep_for(std::chrono::milliseconds(800));
        const sp<Future> f2 = AudioPlayer::play(portaudioPlayer, "test.ogg", AudioPlayer::PLAY_OPTION_DEFAULT);
        while(!f1->isDone() || !f2->isDone())
            std::this_thread::sleep_for(std::chrono::milliseconds(100));


        const sp<AudioPlayer> fmodPlayer = resourceLoader->load<AudioPlayer>("fmod_player", args);
        const sp<Future> f3 = AudioPlayer::play(fmodPlayer, "test.ogg", AudioPlayer::PLAY_OPTION_DEFAULT);
        std::this_thread::sleep_for(std::chrono::milliseconds(800));
        const sp<Future> f4 = AudioPlayer::play(fmodPlayer, "test.ogg", AudioPlayer::PLAY_OPTION_DEFAULT);
        while(!f3->isDone())
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

        return 0;
    }
};

}
}

ark::unittest::TestCase* vorbis_create() {
    return new ark::unittest::VorbisTestCase();
}
