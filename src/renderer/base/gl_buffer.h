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
    class ARK_API Uploader {
    public:
        virtual ~Uploader() = default;

        virtual size_t size() = 0;
        virtual void upload(GraphicsContext& graphicsContext, GLenum target, GLsizeiptr size) = 0;
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

        void prepare(GraphicsContext& graphicsContext, const bytearray& transientData);

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
        Snapshot(const sp<Stub>& stub, const bytearray& array);
        Snapshot(const Snapshot& other) = default;
        Snapshot(Snapshot&& other) = default;

        uint32_t id() const;
        void prepare(GraphicsContext& graphicsContext) const;

    private:
        sp<Stub> _stub;
        bytearray _array;

    };

public:
    GLBuffer(const sp<GLRecycler>& recycler, const sp<Variable<bytearray>>& buffer, GLenum type, GLenum usage, uint32_t size);
    GLBuffer(const sp<GLRecycler>& recycler, const sp<GLBuffer::Uploader>& uploader, GLenum type, GLenum usage);
    GLBuffer(const GLBuffer& other) noexcept = default;
    GLBuffer(GLBuffer&& other) noexcept = default;
    GLBuffer() noexcept;

    explicit operator bool() const;

    GLBuffer& operator =(const GLBuffer& other) noexcept = default;
    GLBuffer& operator =(GLBuffer&& other) noexcept = default;

    template<typename T> uint32_t length() const {
        return (_size ? _size : _stub->size()) / sizeof(T);
    }

    void setSize(uint32_t size);

    GLenum type() const;
    Snapshot snapshot(const bytearray& array) const;

    GLuint id() const;
    void prepare(GraphicsContext&) const;
    void recycle(GraphicsContext&) const;

private:
    sp<Stub> _stub;
    uint32_t _size;

    friend class GLResourceManager;
};

}

#endif
