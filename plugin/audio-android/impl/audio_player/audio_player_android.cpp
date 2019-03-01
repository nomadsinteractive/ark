#include "audio-android/impl/audio_player/audio_player_android.h"

#include <thread>
#include <mutex>

#include "core/inf/executor.h"
#include "core/inf/readable.h"
#include "core/base/future.h"
#include "core/impl/boolean/boolean_by_weak_ref.h"
#include "core/util/log.h"

#include "renderer/base/resource_loader_context.h"

#define BUFFER_SIZE_AUTOMATIC 0

namespace ark {
namespace plugin {
namespace audio_android {

namespace {

class PlayingStream : public Runnable {
public:
    PlayingStream(const sp<Future>& future, const sp<Readable>& stream, AudioPlayer::PlayOption playOption)
        : _future(future), _stream(stream), _play_option(playOption), _audio_stream(nullptr) {
    }

    virtual void run() override {
        createPlaybackStream();

        aaudio_result_t result = AAUDIO_OK;
        aaudio_stream_state_t currentState = AAudioStream_getState(_audio_stream);
        aaudio_stream_state_t inputState = currentState;
        while (result == AAUDIO_OK && currentState !=  AAUDIO_STREAM_STATE_STOPPED) {
            result = AAudioStream_waitForStateChange(_audio_stream, inputState, &currentState, 20000000000);
            inputState = currentState;
        }

        closeOutputStream();
    }

private:
    aaudio_data_callback_result_t dataCallback(AAudioStream *stream, void *audioData, int32_t numFrames) {
        DASSERT(stream == _audio_stream);

        int32_t underrunCount = AAudioStream_getXRunCount(_audio_stream);
        aaudio_result_t bufferSize = AAudioStream_getBufferSizeInFrames(_audio_stream);
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

        memset(audioData, 0, sizeof(int16_t) * _sample_channels_ * numFrames);
        if (!_future->isCancelled()) {
            uint32_t readlen = _stream->read(audioData, sizeof(int16_t) * _sample_channels_ * numFrames);
            if(readlen == 0) {
                _future->cancel();
            }
        }

        return _future->isCancelled() ? AAUDIO_CALLBACK_RESULT_STOP : AAUDIO_CALLBACK_RESULT_CONTINUE;
    }

    void errorCallback(AAudioStream *stream, aaudio_result_t error) {
        assert(stream == _audio_stream);
        LOGD("errorCallback result: %s", AAudio_convertResultToText(error));

        aaudio_stream_state_t streamState = AAudioStream_getState(_audio_stream);
        if (streamState == AAUDIO_STREAM_STATE_DISCONNECTED) {
            std::function<void(void)> restartStream = std::bind(&PlayingStream::restartStream, this);
            std::thread streamRestartThread(restartStream);
            streamRestartThread.detach();
        }
    }

    static aaudio_data_callback_result_t _data_callback(AAudioStream* stream, void* userData, void* audioData, int32_t numFrames) {
        DASSERT(userData && audioData);
        PlayingStream* audioStream = reinterpret_cast<PlayingStream*>(userData);
        return audioStream->dataCallback(stream, audioData, numFrames);
    }

    static void _error_callback(AAudioStream* stream, void* userData, aaudio_result_t error) {
        DASSERT(userData);
        PlayingStream *audioStream = reinterpret_cast<PlayingStream*>(userData);
        audioStream->errorCallback(stream, error);
    }

    void createPlaybackStream() {
        AAudioStreamBuilder* builder = createStreamBuilder();

        if (builder != nullptr) {

            setupPlaybackStreamParameters(builder);

            aaudio_result_t result = AAudioStreamBuilder_openStream(builder, &_audio_stream);

            if (result == AAUDIO_OK && _audio_stream != nullptr) {

                if(_sample_format != AAudioStream_getFormat(_audio_stream)) {
                    LOGW("Sample format is not AAUDIO_FORMAT_PCM_I16");
                }

                _sample_rate = AAudioStream_getSampleRate(_audio_stream);
                _frames_per_burst = AAudioStream_getFramesPerBurst(_audio_stream);

                // Set the buffer size to the burst size - this will give us the minimum possible latency
                AAudioStream_setBufferSizeInFrames(_audio_stream, _frames_per_burst);
                _buf_size_in_frames = _frames_per_burst;

                // Start the stream - the dataCallback function will start being called
                result = AAudioStream_requestStart(_audio_stream);
                if (result != AAUDIO_OK) {
                    LOGE("Error starting stream. %s", AAudio_convertResultToText(result));
                }

                // Store the underrun count so we can tune the latency in the dataCallback
                _play_stream_underrun_count = AAudioStream_getXRunCount(_audio_stream);

            } else {
                LOGE("Failed to create stream. Error: %s", AAudio_convertResultToText(result));
            }

            AAudioStreamBuilder_delete(builder);

        } else {
            LOGE("Unable to obtain an AAudioStreamBuilder object");
        }
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
        AAudioStreamBuilder_setSampleRate(builder, 44100);
        AAudioStreamBuilder_setChannelCount(builder, _sample_channels_);

        // We request EXCLUSIVE mode since this will give us the lowest possible latency.
        // If EXCLUSIVE mode isn't available the builder will fall back to SHARED mode.
        AAudioStreamBuilder_setSharingMode(builder, AAUDIO_SHARING_MODE_EXCLUSIVE);
        AAudioStreamBuilder_setPerformanceMode(builder, AAUDIO_PERFORMANCE_MODE_LOW_LATENCY);
        AAudioStreamBuilder_setDirection(builder, AAUDIO_DIRECTION_OUTPUT);
        AAudioStreamBuilder_setDataCallback(builder, _data_callback, this);
        AAudioStreamBuilder_setErrorCallback(builder, _error_callback, this);
    }

    void restartStream() {

        LOGD("Restarting stream");

        if (_restarting_lock.try_lock()){
            closeOutputStream();
            createPlaybackStream();
            _restarting_lock.unlock();
        } else {
            LOGW("Restart stream operation already in progress - ignoring this request");
            // We were unable to obtain the restarting lock which means the restart operation is currently
            // active. This is probably because we received successive "stream disconnected" events.
            // Internal issue b/63087953
        }
    }

    void closeOutputStream() {
      if (_audio_stream != nullptr){
            aaudio_result_t result = AAudioStream_requestStop(_audio_stream);
            if (result != AAUDIO_OK){
                LOGE("Error stopping output stream. %s", AAudio_convertResultToText(result));
            }

            result = AAudioStream_close(_audio_stream);
            if (result != AAUDIO_OK){
                LOGE("Error closing output stream. %s", AAudio_convertResultToText(result));
            }
        }
    }

private:
    sp<Future> _future;
    sp<Readable> _stream;
    AudioPlayer::PlayOption _play_option;

    int32_t _sample_rate;
    int16_t _sample_channels_ = 2;
    aaudio_format_t _sample_format = AAUDIO_FORMAT_PCM_I16;

    AAudioStream* _audio_stream;

    int32_t _play_stream_underrun_count;
    int32_t _buf_size_in_frames;
    int32_t _frames_per_burst;
    double _current_output_latency_millis = 0;
    int32_t _buffer_size_selection = BUFFER_SIZE_AUTOMATIC;

    std::mutex _restarting_lock;
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
