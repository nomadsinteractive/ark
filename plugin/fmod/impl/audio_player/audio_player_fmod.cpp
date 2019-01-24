#include "fmod/impl/audio_player/audio_player_fmod.h"

#include <thread>

#include "core/inf/executor.h"
#include "core/inf/readable.h"
#include "core/base/future.h"
#include "core/impl/boolean/boolean_by_weak_ref.h"
#include "core/util/log.h"

#include "renderer/base/resource_loader_context.h"

namespace ark {
namespace plugin {
namespace fmod {

namespace {

class PlayingStream : public Runnable {
public:
    PlayingStream(FMOD::System* system, const sp<Future>& future, const sp<Readable>& stream, AudioPlayer::PlayOption playOption)
        : _system(system), _future(future), _stream(stream), _play_option(playOption), _sound(nullptr), _channel(nullptr) {
    }

    virtual void run() override {
        FMOD_CREATESOUNDEXINFO info;
        memset(&info, 0, sizeof(info));
        info.cbsize = sizeof(info);
        info.fileuseropen = _file_open_callback;
        info.fileuserclose = _file_close_callback;
        info.fileuserread = _file_read_callback;
        info.fileuserseek = _file_seek_callback;
        info.fileuserdata = new sp<Readable>(_stream);

        FMOD_MODE mode = _play_option == AudioPlayer::PLAY_OPTION_LOOP_ON ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF;
        FMOD_RESULT result = _system->createStream("", mode, &info, &_sound);
        DCHECK(result == FMOD_OK, "FMOD error: %d", result);

        result = _system->playSound(_sound, nullptr, false, &_channel);
        DCHECK(result == FMOD_OK, "FMOD error: %d", result);

        bool playing = true;
        while(playing) {
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
            result = _system->update();
            DCHECK(result == FMOD_OK, "FMOD error: %d", result);
            result = _channel->isPlaying(&playing);
            if(!playing)
                break;
            DCHECK(result == FMOD_OK, "FMOD error: %d", result);
            if(_future->isCancelled()) {
                playing = false;
                _channel->stop();
            }
        }
        _future->done();
        _sound->release();
    }

private:
    static FMOD_RESULT F_CALLBACK _file_open_callback(const char* /*name*/, unsigned int* filesize, void** handle, void* userdata) {
        sp<Readable>& stream = *reinterpret_cast<sp<Readable>*>(userdata);
        *handle = &stream;
        *filesize = static_cast<uint32_t>(stream->remaining());
        return FMOD_OK;
    }

    static FMOD_RESULT F_CALLBACK _file_close_callback(void* handle, void* /*userdata*/) {
        sp<Readable>* stream = reinterpret_cast<sp<Readable>*>(handle);
        delete stream;
        return FMOD_OK;
    }

    static FMOD_RESULT F_CALLBACK _file_read_callback(void* handle, void* buffer, unsigned int sizebytes, unsigned int *bytesread, void* /*userdata*/) {
        sp<Readable>& stream = *reinterpret_cast<sp<Readable>*>(handle);
        *bytesread = stream->read(buffer, sizebytes);
        return FMOD_OK;
    }

    static FMOD_RESULT F_CALLBACK _file_seek_callback(void* handle, unsigned int pos, void* /*userdata*/) {
        sp<Readable>& stream = *reinterpret_cast<sp<Readable>*>(handle);
        stream->seek(static_cast<int32_t>(pos), SEEK_SET);
        return FMOD_OK;
    }

private:
    FMOD::System* _system;

    sp<Future> _future;
    sp<Readable> _stream;
    AudioPlayer::PlayOption _play_option;

    FMOD::Sound* _sound;
    FMOD::Channel* _channel;
};

}

AudioPlayerFMOD::AudioPlayerFMOD(const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _executor(resourceLoaderContext->executor()), _system(sp<FMODSystem>::make())
{
    resourceLoaderContext->renderController()->addPreUpdateRequest(_system, sp<BooleanByWeakRef<FMODSystem>>::make(_system, 1));
}

sp<Future> AudioPlayerFMOD::play(const sp<Readable>& source, AudioFormat /*format*/, PlayOption options)
{
    const sp<Future> future = sp<Future>::make();
    sp<PlayingStream> stream = sp<PlayingStream>::make(_system->instance(), future, source, options);
    _executor->execute(stream);
    return future;
}

bool AudioPlayerFMOD::isAudioFormatSupported(AudioPlayer::AudioFormat /*format*/)
{
    return true;
}

AudioPlayerFMOD::BUILDER::BUILDER(BeanFactory& /*factory*/, const document& /*manifest*/, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_loader_context(resourceLoaderContext)
{
}

sp<AudioPlayer> AudioPlayerFMOD::BUILDER::build(const sp<Scope>& /*args*/)
{
    return sp<AudioPlayerFMOD>::make(_resource_loader_context);
}

AudioPlayerFMOD::FMODSystem::FMODSystem()
{
    FMOD_RESULT result = FMOD::System_Create(&_instance);
    DCHECK(result == FMOD_OK, "FMOD error: %d", result);

    uint32_t version;
    result = _instance->getVersion(&version);
    DCHECK(result == FMOD_OK, "FMOD error: %d", result);
    DCHECK(version >= FMOD_VERSION, "FMOD lib version %08x doesn't match header version %08x", version, FMOD_VERSION);

    result = _instance->init(16, FMOD_INIT_NORMAL, nullptr);
    DCHECK(result == FMOD_OK, "FMOD error: %d", result);
}

AudioPlayerFMOD::FMODSystem::~FMODSystem()
{
    FMOD_RESULT result = _instance->close();
    DCHECK(result == FMOD_OK, "FMOD error: %d", result);
    result = _instance->release();
    DCHECK(result == FMOD_OK, "FMOD error: %d", result);
}

void AudioPlayerFMOD::FMODSystem::run()
{
    _instance->update();
}

FMOD::System* AudioPlayerFMOD::FMODSystem::instance() const
{
    return _instance;
}

}
}
}
