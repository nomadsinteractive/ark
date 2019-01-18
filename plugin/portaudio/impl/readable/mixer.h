#ifndef ARK_PLUGIN_PORTAUDIO_IMPL_READABLE_MIXER_H_
#define ARK_PLUGIN_PORTAUDIO_IMPL_READABLE_MIXER_H_

#include <vector>

#include "core/concurrent/lf_stack.h"
#include "core/inf/array.h"
#include "core/inf/readable.h"
#include "core/types/shared_ptr.h"

namespace ark {
namespace plugin {
namespace portaudio {

class Mixer : public Readable {
public:
    Mixer(uint32_t bufferLength);

    virtual uint32_t read(void* buffer, uint32_t size) override;
    virtual int32_t seek(int32_t position, int32_t whence) override;
    virtual int32_t remaining() override;

    sp<Future> post(const sp<Readable>& readable, uint32_t weight);
    bool empty() const;

private:
    class Source {
    public:
        Source(const sp<Readable>& readable, uint32_t weight);

        size_t accumulate(int16_t* in, int32_t *out, size_t size) const;
        uint32_t weight() const;

        const sp<Future>& future() const;

    private:
        sp<Readable> _readable;
        uint32_t _weight;

        sp<Future> _future;
    };

private:
    void ensureToneMapRange(int32_t value);

private:
    LFStack<sp<Source>> _sources;

    array<int16_t> _buffer;
    array<int32_t> _buffer_hdr;

    uint32_t _total_weight;

    static const int32_t TONE_MAP_WEIGHT_ONE = 32768;
    std::vector<int16_t> _tone_map;
};

}
}
}

#endif
