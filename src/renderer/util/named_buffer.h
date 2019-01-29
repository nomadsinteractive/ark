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
        NAME_QUADS,
        NAME_NINE_PATCH,
        NAME_POINTS,
        NAME_COUNT
    };

public:
    NamedBuffer(const Buffer& buffer, Uploader::MakerFunc maker, std::function<size_t(size_t)> sizeCalculator);

    const Buffer& buffer() const;

    void reset();

    Buffer::Snapshot snapshot(RenderController& resourceManager, size_t objectCount);

public:
    class NinePatch : public Uploader {
    public:
        NinePatch(size_t objectCount);

        virtual void upload(const Uploader::UploadFunc& uploader) override;

        static sp<NamedBuffer> make(RenderController& renderController);

    private:
        size_t _object_count;
        ark::Array<glindex_t>::Fixed<28> _boiler_plate;
    };

    class Quads : public Uploader {
    public:
        Quads(size_t objectCount);

        virtual void upload(const Uploader::UploadFunc& uploader) override;

        static Uploader::MakerFunc maker();
        static sp<NamedBuffer> make(RenderController& renderController);

    private:
        size_t _object_count;
    };

    class Points : public Uploader {
    public:
        Points(size_t objectCount);

        virtual void upload(const Uploader::UploadFunc& uploader) override;

        static sp<NamedBuffer> make(RenderController& renderController);

    private:
        size_t _object_count;
    };

private:
    Buffer _buffer;
    Uploader::MakerFunc _maker;
    std::function<size_t(size_t)> _size_calculator;

    size_t _object_count;
};

}

#endif
