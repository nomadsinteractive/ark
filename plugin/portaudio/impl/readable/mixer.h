#ifndef ARK_PLUGIN_PORTAUDIO_IMPL_READABLE_MIXER_H_
#define ARK_PLUGIN_PORTAUDIO_IMPL_READABLE_MIXER_H_

#include "core/concurrent/lock_free_stack.h"
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

        uint32_t mix(int16_t* in, int16_t* out, uint32_t size, uint32_t totalWeight, bool mixing);
        uint32_t weight() const;

        const sp<Future>& future() const;

    private:
        sp<Readable> _readable;
        uint32_t _weight;

        sp<Future> _future;
    };

private:
    LockFreeStack<sp<Source>> _sources;

    array<int16_t> _buffer;
    uint32_t _total_weight;

};

}
}
}

#endif
