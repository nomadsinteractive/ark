#include "audio-android/impl/audio_player/audio_player_android.h"

#include "core/inf/executor.h"
#include "core/inf/readable.h"
#include "core/base/future.h"
#include "core/util/log.h"

#include "renderer/base/resource_loader_context.h"

#define BUFFER_SIZE_AUTOMATIC 0

namespace ark {
namespace plugin {
namespace audio_android {

namespace {

class PlayingStream final : public Runnable {
public:
    PlayingStream(const sp<Future>& future, const sp<Readable>& stream, AudioPlayer::PlayOption playOption)
        : _future(future), _stream(stream), _play_option(playOption) {
    }

    virtual void run() override {
        AAudioStream* audioStream = createPlaybackStream();
        if(audioStream) {
            aaudio_result_t result = AAUDIO_OK;
            aaudio_stream_state_t currentState = AAudioStream_getState(audioStream);
            aaudio_stream_state_t inputState = currentState;
            while (result == AAUDIO_OK && currentState !=  AAUDIO_STREAM_STATE_STOPPED && shouldContinue()) {

                if(currentState == AAUDIO_STREAM_STATE_DISCONNECTED) {
                    closeAudioStream(audioStream);
                    audioStream = createPlaybackStream();
                    inputState = AAudioStream_getState(audioStream);
                }

                result = AAudioStream_waitForStateChange(audioStream, inputState, &currentState, 2000000000);
                inputState = currentState;
            }

            closeAudioStream(audioStream);
        }
    }

private:
    aaudio_data_callback_result_t dataCallback(AAudioStream* stream, void* audioData, int32_t numFrames) {
        int32_t underrunCount = AAudioStream_getXRunCount(stream);
        aaudio_result_t bufferSize = AAudioStream_getBufferSizeInFrames(stream);
        bool hasUnderrunCountIncreased = false;
        bool shouldChangeBufferSize = false;

        if (underrunCount > _play_stream_underrun_count) {
            _play_stream_underrun_count = underrunCount;
            hasUnderrunCountIncreased = true;
        }

        if (hasUnderrunCountIncreased && _buffer_size_selection == BUFFER_SIZE_AUTOMATIC) {

            /**
            * This is a buffer size tuning algorithm. If the number of underruns (i.e. instances where
            * we were unable to supply sufficient data to the stream) has increased since the last callback
            * we will try to increase the buffer size by the burst size, which will give us more protection
            * against underruns in future, at the cost of additional latency.
            */
            bufferSize += _frames_per_burst; // Increase buffer size by one burst
            shouldChangeBufferSize = true;
        } else if (_buffer_size_selection > 0 && (_buffer_size_selection * _frames_per_burst) != bufferSize) {
            // If the buffer size selection has changed then update it here
            bufferSize = _buffer_size_selection * _frames_per_burst;
            shouldChangeBufferSize = true;
        }

        if (shouldChangeBufferSize) {
            LOGD("Setting buffer size to %d", bufferSize);
            bufferSize = AAudioStream_setBufferSizeInFrames(stream, bufferSize);
            if (bufferSize > 0) {
                _buf_size_in_frames = bufferSize;
            } else {
                LOGE("Error setting buffer size: %s", AAudio_convertResultToText(bufferSize));
            }
        }

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

        return shouldContinue() ? AAUDIO_CALLBACK_RESULT_CONTINUE : AAUDIO_CALLBACK_RESULT_STOP;
    }

    void errorCallback(AAudioStream* stream, aaudio_result_t error) {
        LOGD("errorCallback result: %s", AAudio_convertResultToText(error));
    }

    static aaudio_data_callback_result_t _data_callback(AAudioStream* stream, void* userData, void* audioData, int32_t numFrames) {
        DASSERT(userData && audioData);
        PlayingStream* audioStream = reinterpret_cast<PlayingStream*>(userData);
        return audioStream->dataCallback(stream, audioData, numFrames);
    }

    static void _error_callback(AAudioStream* stream, void* userData, aaudio_result_t error) {
        DASSERT(userData);
        PlayingStream* audioStream = reinterpret_cast<PlayingStream*>(userData);
        audioStream->errorCallback(stream, error);
    }

    AAudioStream* createPlaybackStream() {
        AAudioStreamBuilder* builder = createStreamBuilder();
        AAudioStream* audioStream = nullptr;
        if (builder != nullptr) {
            setupPlaybackStreamParameters(builder);

            aaudio_result_t result = AAudioStreamBuilder_openStream(builder, &audioStream);

            if (result == AAUDIO_OK && audioStream != nullptr) {

                if(_sample_format != AAudioStream_getFormat(audioStream)) {
                    LOGW("Sample format is not AAUDIO_FORMAT_PCM_I16");
                }

                _sample_rate = AAudioStream_getSampleRate(audioStream);
                _frames_per_burst = AAudioStream_getFramesPerBurst(audioStream);

                // Set the buffer size to the burst size - this will give us the minimum possible latency
                AAudioStream_setBufferSizeInFrames(audioStream, _frames_per_burst);
                _buf_size_in_frames = _frames_per_burst;

                // Start the stream - the dataCallback function will start being called
                result = AAudioStream_requestStart(audioStream);
                if (result != AAUDIO_OK) {
                    LOGE("Error starting stream. %s", AAudio_convertResultToText(result));
                }

                // Store the underrun count so we can tune the latency in the dataCallback
                _play_stream_underrun_count = AAudioStream_getXRunCount(audioStream);

            } else {
                LOGE("Failed to create stream. Error: %s", AAudio_convertResultToText(result));
            }

            AAudioStreamBuilder_delete(builder);
            return audioStream;

        } else {
            LOGE("Unable to obtain an AAudioStreamBuilder object");
        }
        return nullptr;
    }

    AAudioStreamBuilder* createStreamBuilder() {
        AAudioStreamBuilder* builder = nullptr;
        aaudio_result_t result = AAudio_createStreamBuilder(&builder);
        if(result != AAUDIO_OK) {
            LOGE("Error creating stream builder: %s", AAudio_convertResultToText(result));
        }
        return builder;
    }

    void setupPlaybackStreamParameters(AAudioStreamBuilder* builder) {
        AAudioStreamBuilder_setDeviceId(builder, AAUDIO_UNSPECIFIED);
        AAudioStreamBuilder_setFormat(builder, _sample_format);
        AAudioStreamBuilder_setSampleRate(builder, _sample_rate);
        AAudioStreamBuilder_setChannelCount(builder, _sample_channels);

        // We request EXCLUSIVE mode since this will give us the lowest possible latency.
        // If EXCLUSIVE mode isn't available the builder will fall back to SHARED mode.
        AAudioStreamBuilder_setSharingMode(builder, AAUDIO_SHARING_MODE_EXCLUSIVE);
        AAudioStreamBuilder_setPerformanceMode(builder, AAUDIO_PERFORMANCE_MODE_LOW_LATENCY);
        AAudioStreamBuilder_setDirection(builder, AAUDIO_DIRECTION_OUTPUT);
        AAudioStreamBuilder_setDataCallback(builder, _data_callback, this);
        AAudioStreamBuilder_setErrorCallback(builder, _error_callback, this);
    }

    void closeAudioStream(AAudioStream* audioStream) {
        DASSERT(audioStream);
        aaudio_result_t result = AAudioStream_requestStop(audioStream);
        if (result != AAUDIO_OK) {
            LOGE("Error stopping output stream. %s", AAudio_convertResultToText(result));
        }

        result = AAudioStream_close(audioStream);
        if (result != AAUDIO_OK) {
            LOGE("Error closing output stream. %s", AAudio_convertResultToText(result));
        }
    }

    bool shouldContinue() const {
        return !_future->isCancelled() && !_future->isDone();
    }

private:
    sp<Future> _future;
    sp<Readable> _stream;
    AudioPlayer::PlayOption _play_option;

    int32_t _sample_rate = 44100;
    int16_t _sample_channels = 2;
    aaudio_format_t _sample_format = AAUDIO_FORMAT_PCM_I16;

    int32_t _play_stream_underrun_count;
    int32_t _buf_size_in_frames;
    int32_t _frames_per_burst;
    int32_t _buffer_size_selection = BUFFER_SIZE_AUTOMATIC;
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
    sp<PlayingStream> stream = sp<PlayingStream>::make(future, source, options);
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
