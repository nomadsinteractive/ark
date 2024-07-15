#include "renderer/opengl/base/gl_buffer.h"

#include "core/inf/writable.h"
#include "core/util/log.h"

#include "renderer/base/recycler.h"

#include "renderer/opengl/util/gl_util.h"

namespace ark::opengl {

namespace {

class WritableGLBuffer final : public Writable {
public:
    WritableGLBuffer(GLenum type, size_t size)
        : _type(type), _size(size) {
    }

    uint32_t write(const void* buffer, uint32_t size, uint32_t offset) override {
        DASSERT(buffer);
        DCHECK(offset + size <= _size, "GLBuffer data overflow");
        glBufferSubData(_type, static_cast<GLsizeiptr>(offset), static_cast<GLsizeiptr>(size), buffer);
        return size;
    }

private:
    GLenum _type;
    size_t _size;
};

}

GLBuffer::GLBuffer(Buffer::Type type, Buffer::Usage usage, sp<Recycler> recycler)
    : _type(GLUtil::toBufferType(type)), _usage(usage == Buffer::USAGE_DYNAMIC ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW), _recycler(std::move(recycler)), _id(0)
{
}

GLBuffer::~GLBuffer()
{
    _recycler->recycle(recycle());
}

void GLBuffer::doUpload(GraphicsContext& /*graphicsContext*/, Uploader& uploader)
{
    GLint bufsize = 0;
    const volatile GLBufferBinder glBinder(_type, _id);
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

void GLBuffer::uploadBuffer(GraphicsContext& graphicsContext, Uploader& input)
{
    upload(graphicsContext);
    doUpload(graphicsContext, input);
}

void GLBuffer::downloadBuffer(GraphicsContext& /*graphicsContext*/, size_t offset, size_t size, void* ptr)
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
