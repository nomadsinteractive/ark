#ifndef ARK_RENDERER_UTIL_INDEX_BUFFERS_H_
#define ARK_RENDERER_UTIL_INDEX_BUFFERS_H_

#include "core/impl/array/fixed_array.h"

#include "renderer/base/buffer.h"
#include "renderer/inf/uploader.h"

namespace ark {

class IndexBuffers {
public:
    class NinePatch : public Uploader {
    public:
        NinePatch(size_t objectCount);

        virtual void upload(const Uploader::UploadFunc& uploader) override;

        static Uploader::MakerFunc maker();

    private:
        size_t _object_count;
        FixedArray<glindex_t, 28> _boiler_plate;
    };

    class Quads : public Uploader {
    public:
        Quads(size_t objectCount);

        virtual void upload(const Uploader::UploadFunc& uploader) override;

        static Uploader::MakerFunc maker();

    private:
        size_t _object_count;
    };

    class Points : public Uploader {
    public:
        Points(size_t objectCount);

        virtual void upload(const Uploader::UploadFunc& uploader) override;

        static Uploader::MakerFunc maker();

    private:
        size_t _object_count;
    };

    static Buffer::Snapshot makeBufferSnapshot(const RenderController& renderController, Buffer::Name name, size_t objectCount);

};

}

#endif
