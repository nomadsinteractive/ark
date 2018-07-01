#include "plugin/portaudio/impl/readable/mixer.h"

#include "core/base/future.h"
#include "core/impl/array/dynamic_array.h"
#include "core/util/math.h"

namespace ark {
namespace plugin {
namespace portaudio {

Mixer::Mixer(uint32_t bufferLength)
    : _buffer(sp<DynamicArray<int16_t>>::make(bufferLength)), _buffer_hdr(sp<DynamicArray<int32_t>>::make(bufferLength)), _total_weight(0)
{
    ensureToneMapRange(TONE_MAP_WEIGHT_ONE);
}

uint32_t Mixer::read(void* buffer, uint32_t size)
{
    bool eof = false;
    size_t readSize = 0;
    int16_t* out = reinterpret_cast<int16_t*>(buffer);
    int16_t* buf = _buffer->buf();
    int32_t* bufHdr = _buffer_hdr->buf();
    memset(bufHdr, 0, _buffer_hdr->size());

    DCHECK(_buffer->length() >= (size / 2), "Out of buffer, length: %d, available: %d", size / 2, _buffer->length());
    for(const sp<Source>& i : _sources)
    {
        size_t s = i->accumulate(buf, bufHdr, size);
        if(s > readSize)
            readSize = s;
        eof = eof || i->future()->isDone();
    }

    int32_t maxValue = 0;
    for(size_t i = 0; i < readSize / 2; ++i)
    {
        int32_t absValue = std::abs(bufHdr[i]);
        if(absValue > maxValue)
        {
            maxValue = absValue;
            ensureToneMapRange(maxValue);
        }
        int16_t ldrValue = _tone_map[absValue];
        out[i] = bufHdr[i] > 0 ? ldrValue : -ldrValue;
    }

    if(eof)
    {
        for(const sp<Source>& i : _sources.clear())
            if(!i->future()->isDone())
                _sources.push(i);
    }
    return readSize;
}

int32_t Mixer::seek(int32_t /*position*/, int32_t /*whence*/)
{
    DFATAL("Unimplemented");
    return 0;
}

int32_t Mixer::remaining()
{
    DFATAL("Unimplemented");
    return 0;
}

sp<Future> Mixer::post(const sp<Readable>& readable, uint32_t weight)
{
    const sp<Source> source = sp<Source>::make(readable, weight);
    _sources.push(source);
    _total_weight += weight;
    return source->future();
}

bool Mixer::empty() const
{
    return _sources.empty();
}

void Mixer::ensureToneMapRange(int32_t value)
{
    int32_t size = _tone_map.size();
    if(value > size)
    {
        int32_t remainder;
        int32_t divmod = Math::divmod(value, TONE_MAP_WEIGHT_ONE, remainder);
        int32_t newSize = divmod * TONE_MAP_WEIGHT_ONE + (remainder ? TONE_MAP_WEIGHT_ONE : 0);
        _tone_map.resize(newSize);

        for(int32_t i = size; i < _tone_map.size(); ++i)
            _tone_map[i] = static_cast<int16_t>(Math::tanh(static_cast<float>(i) / TONE_MAP_WEIGHT_ONE) * TONE_MAP_WEIGHT_ONE);
    }
}

Mixer::Source::Source(const sp<Readable>& readable, uint32_t weight)
    : _readable(readable), _weight(weight), _future(sp<Future>::make())
{
}

size_t Mixer::Source::accumulate(int16_t* in, int32_t* out, size_t size) const
{
    size_t readSize = _readable->read(in, size);
    if(readSize > 0 && !_future->isCancelled())
    {
        size_t len = readSize / 2;
            for(size_t i = 0; i < len; i++)
                out[i] += in[i];
    }
    else
        _future->done();
    return readSize;
}

uint32_t Mixer::Source::weight() const
{
    return _weight;
}

const sp<Future>&Mixer::Source::future() const
{
    return _future;
}

}
}
}
