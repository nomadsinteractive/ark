#include "app/util/audio_mixer.h"

#include <string.h>

#include "core/base/future.h"
#include "core/inf/array.h"
#include "core/util/math.h"

namespace ark {

namespace  {

class LoopReadable : public Readable {
public:
    LoopReadable(const sp<Readable>& delegate)
        : _delegate(delegate) {
    }

    virtual uint32_t read(void* buffer, uint32_t size) override {
        uint8_t* buf = reinterpret_cast<uint8_t*>(buffer);
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

    virtual int32_t seek(int32_t position, int32_t whence) override {
        return _delegate->seek(position, whence);
    }

    virtual int32_t remaining() override {
        return _delegate->remaining();
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

uint32_t AudioMixer::read(void* buffer, uint32_t size)
{
    bool eof = false;
    size_t readSize = 0;
    int16_t* out = reinterpret_cast<int16_t*>(buffer);
    int16_t* buf = _buffer->buf();
    int32_t* bufHdr = _buffer_hdr->buf();
    memset(bufHdr, 0, _buffer_hdr->size());

    DCHECK(_buffer->length() >= (size / 2), "Out of buffer, length: %d, available: %d", size / 2, _buffer->length());
    for(const sp<Track>& i : _tracks)
    {
        size_t s = i->read(buf, bufHdr, size);
        if(s > readSize)
            readSize = s;
        eof = eof || i->future()->isDone();
    }

    uint32_t maxValue = 0;
    for(size_t i = 0; i < readSize / 2; ++i)
    {
        uint32_t absValue = static_cast<uint32_t>(std::abs(bufHdr[i]));
        if(absValue > maxValue)
        {
            maxValue = absValue;
            ensureToneMapRange(maxValue);
        }
        int16_t ldrValue = _tone_map.at(absValue);
        out[i] = bufHdr[i] > 0 ? ldrValue : -ldrValue;
    }

    if(eof)
    {
        for(const sp<Track>& i : _tracks.clear())
            if(!i->future()->isDone())
                _tracks.push(i);
    }
    return readSize;
}

int32_t AudioMixer::seek(int32_t /*position*/, int32_t /*whence*/)
{
    DFATAL("Unimplemented");
    return 0;
}

int32_t AudioMixer::remaining()
{
    DFATAL("Unimplemented");
    return 0;
}

sp<Future> AudioMixer::addTrack(const sp<Readable>& readable, AudioPlayer::PlayOption option)
{
    const sp<Track> source = sp<Track>::make(option == AudioPlayer::PLAY_OPTION_LOOP_ON ? sp<LoopReadable>::make(readable).cast<Readable>() : readable);
    _tracks.push(source);
    return source->future();
}

bool AudioMixer::empty() const
{
    return _tracks.empty();
}

void AudioMixer::ensureToneMapRange(uint32_t value)
{
    size_t size = _tone_map.size();
    if(value >= size)
    {
        uint32_t remainder;
        uint32_t divmod = Math::divmod(value, TONE_MAP_WEIGHT_ONE, remainder);
        uint32_t newSize = (divmod + 1) * TONE_MAP_WEIGHT_ONE /*+ (remainder ? TONE_MAP_WEIGHT_ONE : 0)*/;
        _tone_map.resize(newSize);

        for(size_t i = size; i < _tone_map.size(); ++i)
            _tone_map[i] = static_cast<int16_t>(Math::tanh(static_cast<float>(i) / TONE_MAP_WEIGHT_ONE) * TONE_MAP_WEIGHT_ONE);
    }
}

AudioMixer::Track::Track(const sp<Readable>& readable)
    : _readable(readable), _future(sp<Future>::make())
{
}

size_t AudioMixer::Track::read(int16_t* in, int32_t* out, size_t size) const
{
    size_t readSize = _readable->read(in, size);
    if(readSize > 0 && !_future->isCanceled())
    {
        size_t len = readSize / 2;
        for(size_t i = 0; i < len; i++)
            out[i] += in[i];
    }
    else
        _future->done();
    return readSize;
}

const sp<Future>& AudioMixer::Track::future() const
{
    return _future;
}

}
