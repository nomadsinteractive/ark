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

#include "app/base/application_context.h"
#include "app/inf/audio_player.h"

#include "platform/platform.h"


namespace ark {
namespace unittest {

class VorbisTestCase : public TestCase {
public:
    virtual int launch() {
        const Global<PluginManager> pluginManager;
        pluginManager->load("ark-vorbis");
        pluginManager->load("ark-portaudio");
        const sp<ResourceLoader> resourceLoader = getResourceLoader();
        const sp<Readable> oggTest1 = resourceLoader->load<Readable>("ogg_test");
        const sp<Readable> oggTest2 = resourceLoader->load<Readable>("ogg_test");
        const sp<AudioPlayer> audioPlayer = resourceLoader->load<AudioPlayer>("audio_player");
        const sp<Future> f1 = audioPlayer->play(oggTest1);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        const sp<Future> f2 = audioPlayer->play(oggTest2);
        while(!f1->isDone() || !f2->isDone())
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        return 0;
    }
};

}
}

ark::unittest::TestCase* vorbis_create() {
    return new ark::unittest::VorbisTestCase();
}
