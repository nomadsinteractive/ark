#include "app/util/audio_mixer.h"

#include <string.h>

#include "core/base/future.h"
#include "core/inf/array.h"
#include "core/util/math.h"

namespace ark {

namespace  {

constexpr uint32_t TONE_MAP_WEIGHT_ONE = 32768;

class LoopReadable final : public Readable {
public:
    LoopReadable(const sp<Readable>& delegate)
        : _delegate(delegate) {
    }

    uint32_t read(void* buffer, const uint32_t size) override
    {
        uint8_t* buf = static_cast<uint8_t*>(buffer);
        uint32_t sizeRead = 0;
        bool rewinded = false;
        while(sizeRead < size) {
            uint32_t sizeToRead = size - sizeRead;
            uint32_t s = _delegate->read(buf + sizeRead, sizeToRead);
            sizeRead += s;
            DCHECK(!rewinded || s > 0, "Failed to read %s bytes after rewinded, empty source?", sizeToRead);
            if(s < sizeToRead) {
                _delegate->seek(0, SEEK_SET);
                rewinded = true;
            }
            else
                rewinded = false;
        }
        return sizeRead;
    }

    int32_t seek(int32_t position, int32_t whence) override {
        return _delegate->seek(position, whence);
    }

    int32_t remaining() override
    {
        return _delegate->remaining();
    }

    uint32_t position() override
    {
        return _delegate->position();
    }

private:
    sp<Readable> _delegate;
};

}


AudioMixer::AudioMixer(uint32_t bufferLength)
    : _buffer(sp<Array<int16_t>::Allocated>::make(bufferLength)), _buffer_hdr(sp<Array<int32_t>::Allocated>::make(bufferLength))
{
    ensureToneMapRange(TONE_MAP_WEIGHT_ONE);
}

uint32_t AudioMixer::read(void* buffer, const uint32_t size)
{
    bool eof = false;
    size_t sizeRead = 0;
    int16_t* out = static_cast<int16_t*>(buffer);
    int16_t* buf = _buffer->buf();
    int32_t* bufHdr = _buffer_hdr->buf();
    memset(bufHdr, 0, _buffer_hdr->size());

    DCHECK(_buffer->length() >= (size / 2), "Out of buffer, length: %d, available: %d", size / 2, _buffer->length());
    for(const sp<Track>& i : _tracks)
    {
        if(const size_t s = i->read(buf, bufHdr, size); s > sizeRead)
            sizeRead = s;
        eof = eof || i->future()->isDoneOrCanceled()->val();
    }

    uint32_t maxValue = 0;
    for(size_t i = 0; i < sizeRead / 2; ++i)
    {
        const uint32_t absValue = static_cast<uint32_t>(std::abs(bufHdr[i]));
        if(absValue > maxValue)
        {
            maxValue = absValue;
            ensureToneMapRange(maxValue);
        }
        const int16_t ldrValue = _tone_map.at(absValue);
        out[i] = bufHdr[i] > 0 ? ldrValue : -ldrValue;
    }

    if(eof)
    {
        for(const sp<Track>& i : _tracks.clear())
            if(!i->future()->isDoneOrCanceled()->val())
                _tracks.push(i);
    }

    if(sizeRead < size)
        memset(static_cast<int8_t*>(buffer) + sizeRead, 0, size - sizeRead);

    return size;
}

int32_t AudioMixer::seek(const int32_t position, const int32_t whence)
{
    return 0;
}

int32_t AudioMixer::remaining()
{
    return 0;
}

uint32_t AudioMixer::position()
{
    return 0;
}

sp<Future> AudioMixer::addTrack(sp<Readable> readable, sp<Future> future, const AudioPlayer::PlayOption option)
{
    const sp<Track> source = sp<Track>::make(option.has(AudioPlayer::PLAY_OPTION_LOOP) ? sp<Readable>::make<LoopReadable>(readable) : readable, std::move(future));
    _tracks.push(source);
    return source->future();
}

bool AudioMixer::empty() const
{
    return _tracks.empty();
}

void AudioMixer::ensureToneMapRange(const uint32_t value)
{
    if(const size_t size = _tone_map.size(); value >= size)
    {
        uint32_t remainder;
        const uint32_t divmod = Math::divmod(value, TONE_MAP_WEIGHT_ONE, remainder);
        const uint32_t newSize = (divmod + 1) * TONE_MAP_WEIGHT_ONE /*+ (remainder ? TONE_MAP_WEIGHT_ONE : 0)*/;
        _tone_map.resize(newSize);

        for(size_t i = size; i < _tone_map.size(); ++i)
            _tone_map[i] = static_cast<int16_t>(Math::tanh(static_cast<float>(i) / TONE_MAP_WEIGHT_ONE) * TONE_MAP_WEIGHT_ONE);
    }
}

AudioMixer::Track::Track(sp<Readable> readable, sp<Future> future)
    : _readable(std::move(readable)), _future(future ? std::move(future) : sp<Future>::make())
{
}

size_t AudioMixer::Track::read(int16_t* in, int32_t* out, const size_t size) const
{
    const size_t readSize = _readable->read(in, size);
    if(readSize > 0 && !_future->isCanceled()->val())
    {
        const size_t len = readSize / 2;
        for(size_t i = 0; i < len; i++)
            out[i] += in[i];
    }
    else
        _future->notify();
    return readSize;
}

const sp<Future>& AudioMixer::Track::future() const
{
    return _future;
}

}
