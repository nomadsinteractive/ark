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
        ASSERT(buffer);
        CHECK(offset + size <= _size, "GLBuffer data overflow");
        GL_CHECK_ERROR(glBufferSubData(_type, static_cast<GLsizeiptr>(offset), static_cast<GLsizeiptr>(size), buffer));
        return size;
    }

private:
    GLenum _type;
    size_t _size;
};

GLenum toBufferGLTarget(const Buffer::Usage usage)
{
    constexpr GLenum types[] = {GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER, GL_DRAW_INDIRECT_BUFFER, GL_SHADER_STORAGE_BUFFER};
    for(uint32_t i = Buffer::USAGE_BIT_VERTEX; i <= Buffer::USAGE_BIT_STORAGE; ++i)
        if(usage.contains(static_cast<Buffer::UsageBits>(i)))
            return types[i];

    FATAL("Unknown buffer type: %d", usage.bits());
    return GL_ARRAY_BUFFER;
}

GLenum toBufferGLUsage(const Buffer::Usage usage)
{
    if(usage.contains(Buffer::USAGE_BIT_STORAGE))
        return GL_DYNAMIC_READ;
    return usage.contains(Buffer::USAGE_BIT_DYNAMIC) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
}

}

GLBuffer::GLBuffer(const Buffer::Usage usage, sp<Recycler> recycler)
    : _target(toBufferGLTarget(usage)), _usage(toBufferGLUsage(usage)), _recycler(std::move(recycler)), _id(0)
{
}

GLBuffer::~GLBuffer()
{
    _recycler->recycle(recycle());
}

void GLBuffer::doUpload(GraphicsContext& /*graphicsContext*/, Uploader& uploader)
{
    GLint bufsize = 0;
    const GLBufferBinder glBinder(_target, _id);
    glGetBufferParameteriv(_target, GL_BUFFER_SIZE, &bufsize);
    DCHECK_WARN(_usage != GL_STATIC_DRAW || bufsize == 0, "Uploading transient data to GL_STATIC_DRAW GLBuffer");

    _size = uploader.size();
    if(static_cast<size_t>(bufsize) < _size)
        GL_CHECK_ERROR(glBufferData(_target, static_cast<GLsizeiptr>(_size), nullptr, _usage));
    WritableGLBuffer writer(_target, _size);
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
    const GLBufferBinder glBinder(_target, _id);
    GL_CHECK_ERROR(glGetBufferSubData(_target, static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(size), ptr));
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
