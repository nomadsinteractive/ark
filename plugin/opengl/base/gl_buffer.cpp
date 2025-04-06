#include "opengl/base/gl_buffer.h"

#include "core/inf/writable.h"
#include "core/util/log.h"

#include "renderer/base/recycler.h"

#include "opengl/util/gl_util.h"

namespace ark::plugin::opengl {

namespace {

class WritableGLBuffer final : public Writable {
public:
    WritableGLBuffer(const GLenum type, const size_t size)
        : _type(type), _size(size) {
    }

    uint32_t write(const void* buffer, const uint32_t size, const uint32_t offset) override
    {
        DASSERT(buffer);
        DCHECK(offset + size <= _size, "GLBuffer data overflow");
        glBufferSubData(_type, static_cast<GLsizeiptr>(offset), static_cast<GLsizeiptr>(size), buffer);
        return size;
    }

private:
    GLenum _type;
    size_t _size;
};

GLenum toBufferType(const Buffer::Type type)
{
    constexpr GLenum types[Buffer::TYPE_COUNT] = {GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER, GL_DRAW_INDIRECT_BUFFER, GL_SHADER_STORAGE_BUFFER};
    DCHECK(type >= 0 && type < Buffer::TYPE_COUNT, "Unknown buffer type: %d", type);
    return types[type];
}

GLenum toGLUsage(const Buffer::Type type, const Buffer::Usage usage)
{
    if(type == Buffer::TYPE_STORAGE)
        return GL_DYNAMIC_READ;
    return usage.has(Buffer::USAGE_BIT_DYNAMIC) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
}

}

GLBuffer::GLBuffer(const Buffer::Type type, const Buffer::Usage usage, sp<Recycler> recycler)
    : _type(toBufferType(type)), _usage(toGLUsage(type, usage)), _recycler(std::move(recycler)), _id(0)
{
}

GLBuffer::~GLBuffer()
{
    _recycler->recycle(recycle());
}

void GLBuffer::doUpload(GraphicsContext& /*graphicsContext*/, Uploader& uploader)
{
    GLint bufsize = 0;
    const GLBufferBinder glBinder(_type, _id);
    glGetBufferParameteriv(_type, GL_BUFFER_SIZE, &bufsize);
    DCHECK_WARN(_usage != GL_STATIC_DRAW || bufsize == 0, "Uploading transient data to GL_STATIC_DRAW GLBuffer");

    _size = uploader.size();
    if(static_cast<size_t>(bufsize) < _size)
        glBufferData(_type, static_cast<GLsizeiptr>(_size), nullptr, _usage);
    WritableGLBuffer writer(_type, _size);
    uploader.upload(writer);
}

uint64_t GLBuffer::id()
{
    return _id;
}

void GLBuffer::upload(GraphicsContext& /*graphicsContext*/)
{
    if(_id == 0)
        glGenBuffers(1, &_id);
}

void GLBuffer::uploadBuffer(GraphicsContext& graphicsContext, Uploader& uploader)
{
    upload(graphicsContext);
    doUpload(graphicsContext, uploader);
}

void GLBuffer::downloadBuffer(GraphicsContext& /*graphicsContext*/, const size_t offset, const size_t size, void* ptr)
{
    const volatile GLBufferBinder glBinder(_type, _id);

    glGetBufferSubData(_type, static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(size), ptr);
}

ResourceRecycleFunc GLBuffer::recycle()
{
    uint32_t id = _id;
    _id = 0;
    _size = 0;
    return [id](GraphicsContext&) {
        LOGD("Recycling GLBuffer[%d]", id);
        glDeleteBuffers(1, &id);
    };
}

}
