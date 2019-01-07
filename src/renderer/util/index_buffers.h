#ifndef ARK_RENDERER_UTIL_INDEX_BUFFERS_H_
#define ARK_RENDERER_UTIL_INDEX_BUFFERS_H_

#include "core/inf/array.h"

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
        ark::Array<glindex_t>::Fixed<28> _boiler_plate;
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

    static Buffer::Snapshot snapshot(const Buffer& buffer, ResourceManager& resourceManager, Buffer::Name name, size_t objectCount);

private:

    static Buffer::Snapshot doSnapshot(const Buffer& buffer, ResourceManager& resourceManager, const Uploader::MakerFunc& maker, size_t reservedObjectCount, size_t size);

};

}

#endif
