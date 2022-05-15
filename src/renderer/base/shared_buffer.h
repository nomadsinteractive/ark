#ifndef ARK_RENDERER_BASE_SHARED_BUFFER_H_
#define ARK_RENDERER_BASE_SHARED_BUFFER_H_

#include <functional>

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"
#include "renderer/inf/uploader.h"

namespace ark {

class SharedBuffer {
public:
    enum Name {
        NAME_NONE,
        NAME_QUADS,
        NAME_NINE_PATCH,
        NAME_POINTS,
        NAME_COUNT
    };

public:
    SharedBuffer(const Buffer& buffer, Uploader::MakerFunc maker, std::function<size_t(size_t)> sizeCalculator);

    Buffer::Snapshot snapshot(RenderController& renderController, size_t objectCount, size_t reservedIfInsufficient = 0);

public:

    class Quads {
    public:
        static Uploader::MakerFunc maker();
    };

    class Concat : public Uploader {
    public:
        Concat(size_t primitiveCount, size_t vertexCount, sp<Uploader> indices);

        virtual void upload(Writable& uploader) override;

    private:
        size_t _primitive_count;
        size_t _vertex_count;
        sp<Uploader> _indices;
    };

    class Degenerate : public Uploader {
    public:
        Degenerate(size_t primitiveCount, size_t vertexCount, sp<Uploader> indices);

        virtual void upload(Writable& uploader) override;

    private:
        size_t _primitive_count;
        size_t _vertex_count;
        sp<Uploader> _indices;
    };

private:
    Buffer _buffer;
    Uploader::MakerFunc _maker;
    std::function<size_t(size_t)> _size_calculator;

    size_t _primitive_count;
};

}

#endif
