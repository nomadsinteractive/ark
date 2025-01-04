#include "miniaudio/impl/audio_player/audio_player_miniaudio.h"

#ifdef ARK_MINIAUDIO_UNIMPLEMENTED
#define MINIAUDIO_IMPLEMENTATION
#endif
#include <thread>
#include <miniaudio.h>

#include "app/base/application_context.h"
#include "core/base/future.h"
#include "core/inf/executor.h"
#include "core/inf/readable.h"
#include "core/inf/runnable.h"
#include "core/impl/executor/executor_thread_pool.h"
#include "core/util/log.h"

#include "renderer/base/resource_loader_context.h"

#include "app/util/audio_mixer.h"


namespace ark::plugin::miniaudio {

static ma_result _decoder_read_proc(ma_decoder* pDecoder, void* pBufferOut, size_t bytesToRead, size_t* pBytesRead);
static ma_result _decoder_seek_proc(ma_decoder* pDecoder, ma_int64 byteOffset, ma_seek_origin origin);
static void _data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);

class AudioPlayerMiniAudio::MADevice : public Runnable {
public:
    MADevice(uint32_t channels, uint32_t sampleRate)
        : _audio_mixer(sp<AudioMixer>::make(channels * sampleRate)) {
        _device_config = ma_device_config_init(ma_device_type_playback);
        _device_config.playback.format   = ma_format_s16;
        _device_config.playback.channels = channels;
        _device_config.sampleRate        = sampleRate;
        _device_config.dataCallback      = _data_callback;
        _device_config.pUserData         = this;

        ma_result result = ma_device_init(nullptr, &_device_config, &_device);
        DCHECK(result == MA_SUCCESS, "Failed to open playback device. Error code: %d", result);

        ma_decoder_config inConfig{};
        inConfig.format = _device_config.playback.format;
        inConfig.channels = channels;
        inConfig.sampleRate = sampleRate;
        result = ma_decoder_init(_decoder_read_proc, _decoder_seek_proc, _audio_mixer.get(), &inConfig, &_decoder);
        DCHECK(result == MA_SUCCESS, "Failed to create decoder. Error code: %d", result);

    }
    ~MADevice() override {
        ma_device_uninit(&_device);
        ma_decoder_uninit(&_decoder);
    }

    virtual void run() override {
        ma_result result = ma_device_start(&_device);
        DCHECK(result == MA_SUCCESS, "ma_device_start failed. Error code: %d", result);
        while(!_audio_mixer->empty())
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        LOGD("Audio play done");
    }

    ma_device_config _device_config;
    ma_device _device;
    ma_decoder _decoder;

    sp<AudioMixer> _audio_mixer;
};

void _data_callback(ma_device* pDevice, void* pOutput, const void* /*pInput*/, ma_uint32 frameCount)
{
    ma_uint64 framesRead;
    AudioPlayerMiniAudio::MADevice* stub = reinterpret_cast<AudioPlayerMiniAudio::MADevice*>(pDevice->pUserData);
    ma_decoder_read_pcm_frames(&stub->_decoder, pOutput, frameCount, &framesRead);
}

ma_result _decoder_read_proc(ma_decoder* pDecoder, void* pBufferOut, size_t bytesToRead, size_t* pBytesRead)
{
    AudioMixer* readable = reinterpret_cast<AudioMixer*>(pDecoder->pUserData);
    *pBytesRead = readable->read(pBufferOut, bytesToRead);
    return MA_SUCCESS;
}

ma_result _decoder_seek_proc(ma_decoder* pDecoder, ma_int64 byteOffset, ma_seek_origin origin)
{
    AudioMixer* readable = static_cast<AudioMixer*>(pDecoder->pUserData);
    DCHECK(origin == ma_seek_origin_start || origin == ma_seek_origin_current, "ma_seek_origin should be either ma_seek_origin_start or ma_seek_origin_current");
    return readable->seek(byteOffset, origin == ma_seek_origin_start ? SEEK_SET : SEEK_CUR) == 0 ? MA_SUCCESS : MA_ERROR;
}

AudioPlayerMiniAudio::AudioPlayerMiniAudio()
    : _executor(Ark::instance().applicationContext()->executorThreadPool())
{
}

sp<Future> AudioPlayerMiniAudio::play(const sp<Readable>& source, AudioFormat format, PlayOption options)
{
    DASSERT(format == AudioPlayer::AUDIO_FORMAT_PCM);

    sp<AudioPlayerMiniAudio::MADevice> device = _device.lock();
    bool newRequest = !static_cast<bool>(device);

    if(newRequest)
    {
        device = sp<AudioPlayerMiniAudio::MADevice>::make(2, 44100);
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
