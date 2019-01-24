#include "portaudio/impl/audio_player/audio_player_portaudio.h"

#include <algorithm>

#include <portaudio.h>
#include <pa_ringbuffer.h>
#include <pa_util.h>
#include <pa_hostapi.h>

#include "core/inf/executor.h"
#include "core/inf/runnable.h"
#include "core/inf/readable.h"
#include "core/base/future.h"
#include "core/util/log.h"

#include "renderer/base/resource_loader_context.h"

#include "plugin/portaudio/impl/readable/mixer.h"

#define SAMPLE_RATE  (44100)
#define NUM_CHANNELS    (2)
#define NUM_WRITES_PER_BUFFER   (4)

#ifdef ANDROID

extern "C" PaError PaSkeleton_Initialize(PaUtilHostApiRepresentation **hostApi, PaHostApiIndex index);
extern "C" PaError PaOpenSLES_Initialize(PaUtilHostApiRepresentation **hostApi, PaHostApiIndex index);

static void __portaudio_android_bootstrap__()
{
    if(paHostApiInitializers[0] == PaSkeleton_Initialize)
        paHostApiInitializers[0] = PaOpenSLES_Initialize;
}

#endif

namespace ark {
namespace plugin {
namespace portaudio {

static int _playCallback( const void *inputBuffer, void *outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo* timeInfo,
                         PaStreamCallbackFlags statusFlags,
                         void *userData);

static uint32_t _nextPowerOf2(uint32_t val)
{
    val--;
    val = (val >> 1) | val;
    val = (val >> 2) | val;
    val = (val >> 4) | val;
    val = (val >> 8) | val;
    val = (val >> 16) | val;
    return ++val;
}


class PlayRequestWriteRunnable : public Runnable {
public:
    PlayRequestWriteRunnable(const sp<PortaudioDeviceStream>& stream);

    virtual void run() override;

private:
    sp<PortaudioDeviceStream> _stream;
};

class PortaudioDeviceStream {
public:
    PortaudioDeviceStream(const sp<AudioPlayerPortaudio::Stub>& stub, uint32_t sampleRate, uint32_t numChannels, float bufLengthInSec)
        : _stub(stub), _sample_rate(sampleRate), _num_channels(numChannels), _frame_index(0), _stream(nullptr), _stopped(false) {
        uint32_t numSamples = _nextPowerOf2(static_cast<uint32_t>(_sample_rate * _num_channels * bufLengthInSec));
        uint32_t numBytes = numSamples * sizeof(uint16_t);
        _ring_buffer_data = PaUtil_AllocateMemory( numBytes );
        DCHECK(_ring_buffer_data, "Could not allocate ring buffer data.");

        if (PaUtil_InitializeRingBuffer(&_ring_buffer, sizeof(uint16_t), numSamples, _ring_buffer_data) < 0)
            DFATAL("Failed to initialize ring buffer. Size is not power of 2 ??\n");

        _mixer = sp<Mixer>::make(numSamples);
    }
    ~PortaudioDeviceStream() {
        if(_stream)
            Pa_CloseStream(_stream);
        if(_ring_buffer_data)
            PaUtil_FreeMemory(_ring_buffer_data);
    }

    void write() {
        const ring_buffer_size_t elementsInBuffer = PaUtil_GetRingBufferWriteAvailable(&_ring_buffer);

        if (elementsInBuffer >= _ring_buffer.bufferSize / NUM_WRITES_PER_BUFFER)
        {
            void* ptr[2] = {0};
            ring_buffer_size_t sizes[2] = {0};

            /* By using PaUtil_GetRingBufferWriteRegions, we can write directly into the ring buffer */
            PaUtil_GetRingBufferWriteRegions(&_ring_buffer, elementsInBuffer, ptr + 0, sizes + 0, ptr + 1, sizes + 1);
            ring_buffer_size_t itemsReadFromFile = 0;
            for (int i = 0; i < 2 && ptr[i]; ++i)
            {
                uint32_t bytesRead = _mixer->read(ptr[i], _ring_buffer.elementSizeBytes * sizes[i]);
                if(bytesRead > 0)
                    itemsReadFromFile += bytesRead;
                else
                {
                    _stopped = true;
                    return;
                }
            }

            PaUtil_AdvanceRingBufferWriteIndex(&_ring_buffer, itemsReadFromFile / _ring_buffer.elementSizeBytes);
        }
    }

    void read(void* wptr, uint32_t framesPerBuffer) {
        ring_buffer_size_t elementsToPlay = PaUtil_GetRingBufferReadAvailable(&_ring_buffer);
        ring_buffer_size_t elementsToRead = std::min(elementsToPlay, (ring_buffer_size_t)(framesPerBuffer * _num_channels));

        _frame_index += PaUtil_ReadRingBuffer(&_ring_buffer, wptr, elementsToRead);
    }

    bool stopped() const {
        return _stopped;
    }

    uint32_t numChannels() const {
        return _num_channels;
    }

    uint32_t frameIndex() const {
        return _frame_index;
    }

    sp<Future> post(const sp<PortaudioDeviceStream>& self, const sp<Executor>& executor, const sp<Readable>& source, uint32_t weight) {
        bool empty = _mixer->empty();
        const sp<Future> future = _mixer->post(source, weight);
        if(empty) {
            const PaError paError = start(self);
            if(paError == paNoError)
                executor->execute(sp<PlayRequestWriteRunnable>::make(self));
            else
            {
                LOGE("Audio play error: %d, message: %s", paError, Pa_GetErrorText(paError));
                future->cancel();
            }
        }
        return future;
    }

private:
    PaError start(const sp<PortaudioDeviceStream>& self) {
        PaStreamParameters outputParameters;

        outputParameters.device = Pa_GetDefaultOutputDevice(); /* default output device */
        if (outputParameters.device == paNoDevice) {
            fprintf(stderr,"Error: No default output device.\n");
        }
        outputParameters.channelCount = _num_channels;                     /* stereo output */
        outputParameters.sampleFormat = paInt16;
        outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
        outputParameters.hostApiSpecificStreamInfo = nullptr;

        int err = Pa_OpenStream(&_stream, nullptr, &outputParameters, _sample_rate, paFramesPerBufferUnspecified, paClipOff, _playCallback, new sp<PortaudioDeviceStream>(self));
        return err != paNoError ? err : Pa_StartStream(_stream);
    }

private:
    PaUtilRingBuffer _ring_buffer;
    void* _ring_buffer_data;
    sp<AudioPlayerPortaudio::Stub> _stub;
    uint32_t _sample_rate;
    uint32_t _num_channels;

    uint32_t _frame_index;
    PaStream* _stream;
    bool _stopped;

    sp<Mixer> _mixer;

};

PlayRequestWriteRunnable::PlayRequestWriteRunnable(const sp<PortaudioDeviceStream>& stream)
    : _stream(stream)
{
}

void PlayRequestWriteRunnable::run()
{
    while(!_stream->stopped()) {
        _stream->write();
        Pa_Sleep(20);
    }
}

int _playCallback(const void* /*inputBuffer*/, void* outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo* /*timeInfo*/,
                         PaStreamCallbackFlags /*statusFlags*/,
                         void* userData)
{
    const sp<PortaudioDeviceStream>* stream = reinterpret_cast<sp<PortaudioDeviceStream>*>(userData);
    const sp<PortaudioDeviceStream>& s = *stream;

    s->read(outputBuffer, framesPerBuffer);

    int ret = s->stopped() ? paComplete : paContinue;
    if(s->stopped())
        delete stream;
    return ret;
}

class AudioPlayerPortaudio::Stub {
public:
    Stub()
        : _initialize_result(Pa_Initialize()) {
    }
    ~Stub() {
        if(_initialize_result == paNoError)
            Pa_Terminate();
    }

    int device() const {
        return _device;
    }

private:
    int _initialize_result;
    int _device;
};

AudioPlayerPortaudio::AudioPlayerPortaudio(const sp<Executor>& executor)
    : _stub(sp<Stub>::make()), _executor(executor)
{
}

AudioPlayerPortaudio::~AudioPlayerPortaudio()
{
}

sp<Future> AudioPlayerPortaudio::play(const sp<Readable>& source, AudioFormat format, PlayOption options)
{
    sp<PortaudioDeviceStream> stream = _pa_stream.lock();
    if(!stream)
    {
        stream = sp<PortaudioDeviceStream>::make(_stub, SAMPLE_RATE, NUM_CHANNELS, 0.5f);
        _pa_stream = stream;
    }

    return stream->post(stream, _executor, source, 1);
}

bool AudioPlayerPortaudio::isAudioFormatSupported(AudioPlayer::AudioFormat format)
{
    return format == AUDIO_FORMAT_PCM;
}

AudioPlayerPortaudio::BUILDER::BUILDER(BeanFactory& /*parent*/, const document& /*doc*/, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_loader_context(resourceLoaderContext)
{
}


sp<AudioPlayer> AudioPlayerPortaudio::BUILDER::build(const sp<Scope>& /*args*/)
{
#ifdef ANDROID
    __portaudio_android_bootstrap__();
#endif
    return sp<AudioPlayerPortaudio>::make(_resource_loader_context->executor());
}

}
}
}
