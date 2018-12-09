#ifndef ARK_RENDERER_OPENGL_UTIL_GL_BUFFER_UPLOADERS_H_
#define ARK_RENDERER_OPENGL_UTIL_GL_BUFFER_UPLOADERS_H_

#include "core/impl/array/fixed_array.h"

#include "renderer/base/buffer.h"

namespace ark {

class GLIndexBuffers {
public:

    class NinePatch : public Buffer::Uploader {
    public:
        NinePatch(size_t objectCount);

        virtual void upload(const Buffer::UploadFunc& uploader) override;

        static Buffer::UploadMakerFunc maker();

    private:
        size_t _object_count;
        FixedArray<glindex_t, 28> _boiler_plate;
    };

    class Quads : public Buffer::Uploader {
    public:
        Quads(size_t objectCount);

        virtual void upload(const Buffer::UploadFunc& uploader) override;

        static Buffer::UploadMakerFunc maker();

    private:
        size_t _object_count;
    };

    class Points : public Buffer::Uploader {
    public:
        Points(size_t objectCount);

        virtual void upload(const Buffer::UploadFunc& uploader) override;

        static Buffer::UploadMakerFunc maker();

    private:
        size_t _object_count;
    };

    static Buffer::Snapshot makeGLBufferSnapshot(GLResourceManager& resourceManager, Buffer::Name name, size_t objectCount);


};

}

#endif
