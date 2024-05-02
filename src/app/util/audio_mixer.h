#pragma once

#include <vector>

#include "core/concurrent/lf_stack.h"
#include "core/inf/array.h"
#include "core/inf/readable.h"
#include "core/types/shared_ptr.h"

#include "app/inf/audio_player.h"

namespace ark {

class ARK_API AudioMixer : public Readable {
public:
    AudioMixer(uint32_t bufferLength);

    virtual uint32_t read(void* buffer, uint32_t size) override;
    virtual int32_t seek(int32_t position, int32_t whence) override;
    virtual int32_t remaining() override;

    sp<Future> addTrack(const sp<Readable>& readable, AudioPlayer::PlayOption option);
    bool empty() const;

private:
    class Track {
    public:
        Track(const sp<Readable>& readable);

        size_t read(int16_t* in, int32_t *out, size_t size) const;

        const sp<Future>& future() const;

    private:
        sp<Readable> _readable;

        sp<Future> _future;
    };

private:
    void ensureToneMapRange(uint32_t value);

private:
    LFStack<sp<Track>> _tracks;

    array<int16_t> _buffer;
    array<int32_t> _buffer_hdr;

    static const uint32_t TONE_MAP_WEIGHT_ONE = 32768;
    std::vector<int16_t> _tone_map;
};

}
