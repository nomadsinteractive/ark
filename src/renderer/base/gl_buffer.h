#ifndef ARK_RENDERER_BASE_GL_BUFFER_H_
#define ARK_RENDERER_BASE_GL_BUFFER_H_

#include "core/base/api.h"
#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/inf/gl_resource.h"

#include "platform/gl/gl.h"

namespace ark {

class ARK_API GLBuffer {
public:
    typedef std::function<void(void*, size_t)> UploadFunc;

    class ARK_API Uploader {
    public:
        virtual ~Uploader() = default;

        virtual size_t size() = 0;
        virtual void upload(const UploadFunc& uploader) = 0;
    };

    class ARK_API ByteArrayUploader : public Uploader {
    public:
        ByteArrayUploader(const bytearray& bytes);

        virtual size_t size() override;
        virtual void upload(const UploadFunc& uploader) override;

    private:
        bytearray _bytes;
    };

private:
    class Recycler : public GLResource {
    public:
        Recycler(GLuint id);

        virtual uint32_t id() override;
        virtual void prepare(GraphicsContext&) override;
        virtual void recycle(GraphicsContext&) override;

    private:
        GLuint _id;
    };

    class Stub : public GLResource {
    public:
        Stub(const sp<GLRecycler>& recycler, const sp<Uploader>& uploader, GLenum type, GLenum usage);
        ~Stub();

        GLenum type() const;
        GLenum usage() const;
        uint32_t size() const;

        virtual GLuint id() override;
        virtual void prepare(GraphicsContext&) override;
        virtual void recycle(GraphicsContext&) override;

        void prepare(GraphicsContext& graphicsContext, const sp<Uploader>& transientUploader);

    private:
        void upload(GraphicsContext& graphicsContext, Uploader& uploader);

    private:
        sp<GLRecycler> _recycler;
        sp<Uploader> _uploader;

        GLenum _type;
        GLenum _usage;

        GLuint _id;
        uint32_t _size;
    };

public:
    class Snapshot {
    public:
        Snapshot() = default;
        Snapshot(const sp<Stub>& stub, const sp<Uploader>& uploader);
        DEFAULT_COPY_AND_ASSIGN(Snapshot);

        uint32_t id() const;
        GLenum type() const;

        void prepare(GraphicsContext& graphicsContext) const;

    private:
        sp<Stub> _stub;
        sp<Uploader> _uploader;
    };

public:
    GLBuffer(const sp<GLRecycler>& recycler, const sp<GLBuffer::Uploader>& uploader, GLenum type, GLenum usage);
    GLBuffer(const GLBuffer& other, uint32_t size);
    GLBuffer() noexcept;
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(GLBuffer);

    explicit operator bool() const;

    template<typename T> uint32_t length() const {
        return _size / sizeof(T);
    }

    uint32_t size() const;

    GLenum type() const;
    Snapshot snapshot(const sp<Uploader>& uploader = nullptr) const;

    GLuint id() const;
    void prepare(GraphicsContext&) const;

private:
    sp<Stub> _stub;
    uint32_t _size;

    friend class GLResourceManager;
};

}

#endif
