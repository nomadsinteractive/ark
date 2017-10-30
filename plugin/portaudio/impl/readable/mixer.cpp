#include "plugin/portaudio/impl/readable/mixer.h"

#include "core/base/future.h"
#include "core/impl/array/dynamic_array.h"

namespace ark {
namespace plugin {
namespace portaudio {

Mixer::Mixer(uint32_t bufferLength)
    : _buffer(sp<DynamicArray<int16_t>>::make(bufferLength)), _total_weight(0)
{
}

uint32_t Mixer::read(void* buffer, uint32_t size)
{
    uint32_t readSize = 0;
    int16_t* buf = reinterpret_cast<int16_t*>(buffer);
    int16_t* mixingBuf = _buffer->array();
    bool eof = false;

    DCHECK(_buffer->length() >= (size / 2), "Out of buffer, length: %d, available: %d", size / 2, _buffer->length());
    bool mixing = false;
    for(const sp<Source>& i : _sources)
    {
        uint32_t s = i->mix(mixingBuf, buf, size, _total_weight, mixing);
        if(s > readSize)
            readSize = s;
        mixing = true;
        eof = eof || i->future()->isDone();
    }
    if(eof)
    {
        _total_weight = 0;
        for(const sp<Source>& i : _sources.clear())
            if(!i->future()->isDone())
            {
                _sources.push(i);
                _total_weight += i->weight();
            }
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

Mixer::Source::Source(const sp<Readable>& readable, uint32_t weight)
    : _readable(readable), _weight(weight), _future(sp<Future>::make())
{
}

uint32_t Mixer::Source::mix(int16_t* in, int16_t* out, uint32_t size, uint32_t totalWeight, bool mixing)
{
    uint32_t readSize = _readable->read(in, size);
    if(readSize > 0 && !_future->isCancelled())
    {
        uint32_t len = readSize / 2;
        if(mixing)
            for(uint32_t i = 0; i < len; i++)
            {
                int32_t v = out[i] + in[i] * _weight / totalWeight;
                out[i] = static_cast<int16_t>(v);
            }
        else
            for(uint32_t i = 0; i < len; i++)
                out[i] = static_cast<int16_t>(in[i] * _weight / totalWeight);
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
