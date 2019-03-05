#include "audio-android/impl/audio_player/audio_player_android.h"

#include <oboe/Oboe.h>

#include "core/base/future.h"
#include "core/inf/executor.h"
#include "core/inf/readable.h"
#include "core/types/owned_ptr.h"
#include "core/util/log.h"

#include "renderer/base/resource_loader_context.h"

constexpr int32_t kBufferSizeAutomatic = 0;

namespace ark {
namespace plugin {
namespace audio_android {

namespace {

class PlayingStreamOboe final : public Runnable, oboe::AudioStreamCallback {
public:
    PlayingStreamOboe(const sp<Future>& future, const sp<Readable>& stream, AudioPlayer::PlayOption playOption)
        : _future(future), _stream(stream), _play_option(playOption) {
    }

    virtual void run() override {
        oboe::AudioStream* audioStream = createPlaybackStream();
        if(audioStream) {
            oboe::StreamState inputState = audioStream->getState();
            oboe::StreamState currentState = inputState;
            oboe::Result result = oboe::Result::OK;
            while (result == oboe::Result::OK && currentState != oboe::StreamState::Stopped && shouldContinue()) {

                if(currentState == oboe::StreamState::Disconnected) {
                    closeAudioStream(audioStream);
                    audioStream = createPlaybackStream();
                    inputState = audioStream->getState();
                }

                result = audioStream->waitForStateChange(inputState, &currentState, 2000000000);
                inputState = currentState;
            }

            closeAudioStream(audioStream);
        }
    }

    virtual oboe::DataCallbackResult onAudioReady(oboe::AudioStream* audioStream, void* audioData, int32_t numFrames) override {
        int32_t bufferSize = audioStream->getBufferSizeInFrames();

        if (_buffer_size_selection == kBufferSizeAutomatic) {
            _latency_tuner->tune();
        } else if (bufferSize != (_buffer_size_selection * _frames_per_burst)) {
            auto setBufferResult = audioStream->setBufferSizeInFrames(_buffer_size_selection * _frames_per_burst);
            if (setBufferResult == oboe::Result::OK) bufferSize = setBufferResult.value();
        }

        DASSERT(audioStream->getChannelCount() == _sample_channels);
        DASSERT(audioStream->getFormat() == oboe::AudioFormat::I16);

        uint32_t sizeToRead = static_cast<uint32_t>(sizeof(int16_t) * _sample_channels * numFrames);
        if (!_future->isCancelled()) {
            uint32_t readlen = _stream->read(audioData, sizeToRead);
            if(readlen == 0) {
                if(_play_option == AudioPlayer::PLAY_OPTION_LOOP_ON) {
                    _stream->seek(SEEK_SET, 0);
                    readlen = _stream->read(audioData, sizeToRead);
                }
                else
                    _future->done();
            }
            if(sizeToRead > readlen) {
                memset(static_cast<int8_t*>(audioData) + readlen, 0, sizeToRead - readlen);
            }
        }

        return shouldContinue() ? oboe::DataCallbackResult::Continue : oboe::DataCallbackResult::Stop;
    }

    virtual void onErrorAfterClose(oboe::AudioStream* audioStream, oboe::Result error) override {
        LOGE(oboe::convertToText(error));
    }

private:
    oboe::AudioStream* createPlaybackStream() {
        oboe::AudioStream* audioStream = nullptr;
        oboe::AudioStreamBuilder builder;
        setupPlaybackStreamParameters(&builder);

        oboe::Result result = builder.openStream(&audioStream);

        if (result == oboe::Result::OK && audioStream != nullptr) {

            _frames_per_burst = audioStream->getFramesPerBurst();

            int channelCount = audioStream->getChannelCount();
            if (channelCount != _sample_channels){
                LOGW("Requested %d channels but received %d", _sample_channels, channelCount);
            }

            // Set the buffer size to the burst size - this will give us the minimum possible latency
            audioStream->setBufferSizeInFrames(_frames_per_burst);

            // Create a latency tuner which will automatically tune our buffer size.
            _latency_tuner.reset(new oboe::LatencyTuner(*audioStream));
            // Start the stream - the dataCallback function will start being called
            result = audioStream->requestStart();
            if (result == oboe::Result::OK) {
                return audioStream;
            }
            LOGE("Error starting stream. %s", oboe::convertToText(result));
        } else {
            LOGE("Failed to create stream. Error: %s", oboe::convertToText(result));
        }
        return nullptr;
    }

    void closeAudioStream(oboe::AudioStream* audioStream) const {
        if (audioStream) {
            oboe::Result result = audioStream->requestStop();
            if (result != oboe::Result::OK) {
                LOGE("Error stopping output stream. %s", oboe::convertToText(result));
            }

            result = audioStream->close();
            if (result != oboe::Result::OK) {
                LOGE("Error closing output stream. %s", oboe::convertToText(result));
            }
        }
    }

    void setupPlaybackStreamParameters(oboe::AudioStreamBuilder* builder) {
        builder->setAudioApi(_audio_api);
        builder->setDeviceId(_audio_device_id);
        builder->setFormat(_sample_format);
        builder->setSampleRate(_sample_rate);
        builder->setChannelCount(_sample_channels);

        // We request EXCLUSIVE mode since this will give us the lowest possible latency.
        // If EXCLUSIVE mode isn't available the builder will fall back to SHARED mode.
        builder->setSharingMode(oboe::SharingMode::Exclusive);
        builder->setPerformanceMode(oboe::PerformanceMode::LowLatency);
        builder->setCallback(this);
    }

    bool shouldContinue() const {
        return !_future->isCancelled() && !_future->isDone();
    }

private:
    sp<Future> _future;
    sp<Readable> _stream;
    AudioPlayer::PlayOption _play_option;

    oboe::AudioApi _audio_api = oboe::AudioApi::Unspecified;
    int32_t _audio_device_id = oboe::kUnspecified;
    int32_t _sample_rate = 44100;
    int16_t _sample_channels = 2;
    oboe::AudioFormat _sample_format = oboe::AudioFormat::I16;

    int32_t _play_stream_underrun_count;
    int32_t _buf_size_in_frames;
    int32_t _frames_per_burst;
    int32_t _buffer_size_selection = kBufferSizeAutomatic;

    op<oboe::LatencyTuner> _latency_tuner;
};

}

AudioPlayerAndroid::AudioPlayerAndroid(const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _executor(resourceLoaderContext->executor())
{
}

sp<Future> AudioPlayerAndroid::play(const sp<Readable>& source, AudioFormat format, PlayOption options)
{
    DASSERT(format == AudioPlayer::AUDIO_FORMAT_PCM);
    const sp<Future> future = sp<Future>::make();
    sp<PlayingStreamOboe> stream = sp<PlayingStreamOboe>::make(future, source, options);
    _executor->execute(stream);
    return future;
}

bool AudioPlayerAndroid::isAudioFormatSupported(AudioPlayer::AudioFormat format)
{
    return format == AudioPlayer::AUDIO_FORMAT_PCM;
}

AudioPlayerAndroid::BUILDER::BUILDER(BeanFactory& /*factory*/, const document& /*manifest*/, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_loader_context(resourceLoaderContext)
{
}

sp<AudioPlayer> AudioPlayerAndroid::BUILDER::build(const sp<Scope>& /*args*/)
{
    return sp<AudioPlayerAndroid>::make(_resource_loader_context);
}

AudioPlayerAndroid::BUILDER_DEFAULT::BUILDER_DEFAULT(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _delegate(factory, manifest, resourceLoaderContext)
{
}

sp<AudioPlayer> AudioPlayerAndroid::BUILDER_DEFAULT::build(const sp<Scope>& args)
{
    return _delegate.build(args);
}

}
}
}
