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
        Stub(const sp<GLRecycler>& recycler, const sp<Variable<bytearray>>& buffer, GLenum type, GLenum usage);
        ~Stub();

        GLenum type() const;
        GLenum usage() const;
        uint32_t size() const;

        void prepare(const bytearray& array);

        virtual GLuint id() override;
        virtual void prepare(GraphicsContext&) override;
        virtual void recycle(GraphicsContext&) override;

    private:
        sp<GLRecycler> _recycler;
        sp<Variable<bytearray>> _buffer;

        GLenum _type;
        GLenum _usage;

        GLuint _id;
        uint32_t _size;
    };

public:
    class Snapshot {
    public:
        Snapshot(const sp<Stub>& stub, const bytearray& array);
        Snapshot(const Snapshot& other);
        Snapshot(Snapshot&& other);

        uint32_t id() const;
        void prepare(GraphicsContext& graphicsContext) const;

    private:
        sp<Stub> _stub;
        bytearray _array;

    };

public:
    GLBuffer(const sp<GLRecycler>& recycler, const sp<Variable<bytearray>>& buffer, GLenum type, GLenum usage, uint32_t size = 0);
    GLBuffer(const GLBuffer& other) noexcept;
    GLBuffer(GLBuffer&& other) noexcept;
    GLBuffer() noexcept;

    explicit operator bool() const;

    const GLBuffer& operator =(const GLBuffer& other) noexcept;
    const GLBuffer& operator =(GLBuffer&& other) noexcept;

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
