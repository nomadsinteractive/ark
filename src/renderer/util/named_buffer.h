#ifndef ARK_RENDERER_UTIL_NAMED_BUFFER_H_
#define ARK_RENDERER_UTIL_NAMED_BUFFER_H_

#include <functional>

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"
#include "renderer/inf/uploader.h"

namespace ark {

class NamedBuffer {
public:
    enum Name {
        NAME_NONE,
        NAME_QUADS,
        NAME_NINE_PATCH,
        NAME_POINTS,
        NAME_COUNT
    };

public:
    NamedBuffer(const Buffer& buffer, Uploader::MakerFunc maker, std::function<size_t(size_t)> sizeCalculator);

    const Buffer& buffer() const;

    void reset();

    Buffer::Snapshot snapshot(RenderController& renderController, size_t objectCount, size_t reservedIfInsufficient = 0);

public:
    class NinePatch {
    public:
        static Uploader::MakerFunc maker();
        static sp<NamedBuffer> make(RenderController& renderController);
    };

    class Quads {
    public:
        static Uploader::MakerFunc maker();
        static sp<NamedBuffer> make(RenderController& renderController);
    };

    class Points : public Uploader {
    public:
        Points(size_t objectCount);

        virtual void upload(const Uploader::UploadFunc& uploader) override;

        static sp<NamedBuffer> make(RenderController& renderController);

    private:
        size_t _object_count;
    };

    class Concat : public Uploader {
    public:
        Concat(size_t objectCount, size_t vertexCount, const array<element_index_t>& indices);

        virtual void upload(const Uploader::UploadFunc& uploader) override;

    private:
        size_t _object_count;
        size_t _vertex_count;
        array<element_index_t> _indices;
    };

    class Degenerate : public Uploader {
    public:
        Degenerate(size_t objectCount, size_t vertexCount, const array<element_index_t>& indices);

        virtual void upload(const Uploader::UploadFunc& uploader) override;

    private:
        size_t _object_count;
        size_t _vertex_count;
        array<element_index_t> _indices;
    };

private:
    Buffer _buffer;
    Uploader::MakerFunc _maker;
    std::function<size_t(size_t)> _size_calculator;

    size_t _object_count;
};

}

#endif
