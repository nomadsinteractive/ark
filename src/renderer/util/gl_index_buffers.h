#ifndef ARK_RENDERER_UTIL_GL_BUFFER_UPLOADERS_H_
#define ARK_RENDERER_UTIL_GL_BUFFER_UPLOADERS_H_

#include "core/impl/array/fixed_array.h"

#include "renderer/base/gl_buffer.h"

namespace ark {

class GLIndexBuffers {
public:

    class NinePatch : public GLBuffer::Uploader {
    public:
        NinePatch(size_t objectCount);

        virtual size_t size() override;
        virtual void upload(const GLBuffer::UploadFunc& uploader) override;

        static GLBuffer::UploadMakerFunc maker();

    private:
        size_t _object_count;
        FixedArray<glindex_t, 28> _boiler_plate;
    };

    class Quads : public GLBuffer::Uploader {
    public:
        Quads(size_t objectCount);

        virtual size_t size() override;
        virtual void upload(const GLBuffer::UploadFunc& uploader) override;

        static GLBuffer::UploadMakerFunc maker();

    private:
        size_t _object_count;
    };

    class Points : public GLBuffer::Uploader {
    public:
        Points(size_t objectCount);

        virtual size_t size() override;
        virtual void upload(const GLBuffer::UploadFunc& uploader) override;

        static GLBuffer::UploadMakerFunc maker();

    private:
        size_t _object_count;
    };

    static GLBuffer::Snapshot makeGLBufferSnapshot(GLResourceManager& resourceManager, GLBuffer::Name name, size_t objectCount);


};

}

#endif
