#include "miniaudio/impl/audio_player/audio_player_miniaudio.h"

#ifdef ARK_MINIAUDIO_UNIMPLEMENTED
#define MINIAUDIO_IMPLEMENTATION
#endif
#define MA_NO_DECODING
#define MA_NO_ENCODING
#include <miniaudio.h>

#include <thread>

#include "core/ark.h"
#include "core/base/future.h"
#include "core/inf/executor.h"
#include "core/inf/readable.h"
#include "core/inf/runnable.h"
#include "core/util/log.h"

#include "app/base/application_context.h"
#include "app/util/audio_mixer.h"


namespace ark::plugin::miniaudio {

static void _data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);

class AudioPlayerMiniAudio::MADevice final : public Runnable {
public:
    MADevice(const uint32_t channels, const uint32_t sampleRate)
        : _audio_mixer(sp<AudioMixer>::make(channels * sampleRate)), _bytes_per_frame(ma_get_bytes_per_frame(ma_format_s16, channels)) {
        _device_config = ma_device_config_init(ma_device_type_playback);
        _device_config.playback.format   = ma_format_s16;
        _device_config.playback.channels = channels;
        _device_config.sampleRate        = sampleRate;
        _device_config.dataCallback      = _data_callback;
        _device_config.pUserData         = this;

        const ma_result result = ma_device_init(nullptr, &_device_config, &_device);
        CHECK(result == MA_SUCCESS, "Failed to open playback device. Error code: %d", result);
    }
    ~MADevice() override
    {
        ma_device_uninit(&_device);
    }

    void run() override
    {
        const ma_result result = ma_device_start(&_device);
        CHECK(result == MA_SUCCESS, "ma_device_start failed. Error code: %d", result);

        Boolean& quitting = Ark::instance().applicationContext()->quitting();
        while(!_audio_mixer->empty() && !quitting.val())
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        LOGD("Audio play done");
    }

    sp<AudioMixer> _audio_mixer;
    uint32_t _bytes_per_frame;

    ma_device_config _device_config;
    ma_device _device;
};

void _data_callback(ma_device* pDevice, void* pOutput, const void* /*pInput*/, const ma_uint32 frameCount)
{
    const AudioPlayerMiniAudio::MADevice* stub = static_cast<const AudioPlayerMiniAudio::MADevice*>(pDevice->pUserData);
    stub->_audio_mixer->read(pOutput, stub->_bytes_per_frame * frameCount);
}

AudioPlayerMiniAudio::AudioPlayerMiniAudio()
    : _executor(Ark::instance().applicationContext()->threadPoolExecutor())
{
}

sp<Future> AudioPlayerMiniAudio::play(const sp<Readable>& source, const AudioFormat format, const PlayOption options)
{
    ASSERT(format == AudioPlayer::AUDIO_FORMAT_PCM);

    sp<MADevice> device = _device.lock();
    const bool newRequest = !static_cast<bool>(device);
    if(newRequest)
    {
        device = sp<MADevice>::make(2, 44100);
        _device = device;
    }

    const sp<Future> future = device->_audio_mixer->addTrack(source, options);

    if(newRequest)
        _executor->execute(device);

    return future;
}

bool AudioPlayerMiniAudio::isAudioFormatSupported(AudioPlayer::AudioFormat format)
{
    return format == AUDIO_FORMAT_PCM;
}

sp<AudioPlayer> AudioPlayerMiniAudio::BUILDER::build(const Scope& /*args*/)
{
    return sp<AudioPlayerMiniAudio>::make();
}

sp<AudioPlayer> AudioPlayerMiniAudio::BUILDER_DEFAULT::build(const Scope& args)
{
    return sp<AudioPlayerMiniAudio>::make();
}

}
