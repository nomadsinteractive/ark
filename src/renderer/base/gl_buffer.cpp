#include "renderer/base/gl_buffer.h"

#include "core/inf/array.h"
#include "core/inf/variable.h"
#include "core/types/null.h"
#include "core/util/log.h"

#include "renderer/base/gl_recycler.h"

namespace ark {

GLBuffer::Recycler::Recycler(GLuint id)
    : _id(id)
{
}

uint32_t GLBuffer::Recycler::id()
{
    return _id;
}

void GLBuffer::Recycler::prepare(GraphicsContext&)
{
}

void GLBuffer::Recycler::recycle(GraphicsContext&)
{
    LOGD("Deleting GLBuffer[%d]", _id);
    glDeleteBuffers(1, &_id);
    _id = 0;
}

GLBuffer::Stub::Stub(const sp<GLRecycler>& recycler, const sp<Uploader>& uploader, GLenum type, GLenum usage)
    : _recycler(recycler), _uploader(uploader), _type(type), _usage(usage), _id(0), _size(0)
{
}

GLBuffer::Stub::~Stub()
{
    if(_id > 0)
        _recycler->recycle(sp<Recycler>::make(_id));
}

GLenum GLBuffer::Stub::type() const
{
    return _type;
}

GLenum GLBuffer::Stub::usage() const
{
    return _usage;
}

uint32_t GLBuffer::Stub::size() const
{
    return _size;
}

void GLBuffer::Stub::prepare(GraphicsContext& graphicsContext, const sp<Uploader>& transientUploader)
{
    if(_id == 0)
    {
        glGenBuffers(1, &_id);
        if(_uploader && !transientUploader)
        {
            upload(graphicsContext, _uploader);
            return;
        }
    }
    if(transientUploader)
    {
        DWARN(_usage != GL_STATIC_DRAW, "Uploading transient data to GL_STATIC_DRAW GLBuffer");
        upload(graphicsContext, transientUploader);
    }
}

void GLBuffer::Stub::upload(GraphicsContext& /*graphicsContext*/, GLBuffer::Uploader& uploader)
{
    size_t uploadSize = uploader.size();
    glBindBuffer(_type, _id);
    if(_size < uploadSize)
        glBufferData(_type, uploadSize, nullptr, _usage);
    _size = uploadSize;
    GLintptr offset = 0;
    const UploadFunc func = [&offset, this](void* data, size_t size) {
        NOT_NULL(data);
        DCHECK(offset + size <= _size, "GLBuffer data overflow");
        glBufferSubData(_type, offset, size, data);
        offset += size;
    };
    uploader.upload(func);
    glBindBuffer(_type, 0);
}

GLuint GLBuffer::Stub::id()
{
    return _id;
}

void GLBuffer::Stub::prepare(GraphicsContext& graphicsContext)
{
    prepare(graphicsContext, nullptr);
}

void GLBuffer::Stub::recycle(GraphicsContext&)
{
    if(_id != 0) {
        LOGD("Deleting GLBuffer[%d]", _id);
        glDeleteBuffers(1, &_id);
        _id = 0;
    }
    _size = 0;
}

GLBuffer::Snapshot::Snapshot(const sp<GLBuffer::Stub>& stub, const sp<Uploader>& uploader)
    : _stub(stub), _uploader(uploader)
{
}

uint32_t GLBuffer::Snapshot::id() const
{
    return _stub->id();
}

GLenum GLBuffer::Snapshot::type() const
{
    return _stub->type();
}

void GLBuffer::Snapshot::prepare(GraphicsContext& graphicsContext) const
{
    _stub->prepare(graphicsContext, _uploader);
}

GLBuffer::GLBuffer(const sp<GLRecycler>& recycler, const sp<Uploader>& uploader, GLenum type, GLenum usage)
    : _stub(sp<Stub>::make(recycler, uploader, type, usage)), _size(uploader ? uploader->size() : 0)
{
}

GLBuffer::GLBuffer(const GLBuffer& other, uint32_t size)
    : _stub(other._stub), _size(size)
{
}

GLBuffer::GLBuffer() noexcept
    : _stub(nullptr), _size(0)
{
}

uint32_t GLBuffer::size() const
{
    return _size;
}

GLBuffer::operator bool() const
{
    return !!_stub;
}

GLenum GLBuffer::type() const
{
    return _stub->type();
}

GLBuffer::Snapshot GLBuffer::snapshot(const sp<Uploader>& uploader) const
{
    return Snapshot(_stub, uploader);
}

GLuint GLBuffer::id() const
{
    return _stub->id();
}

void GLBuffer::prepare(GraphicsContext& graphicsContext) const
{
    if(_size == 0 || _size > _stub->size())
        _stub->prepare(graphicsContext);
}

GLBuffer::ByteArrayUploader::ByteArrayUploader(const bytearray& bytes)
    : _bytes(bytes)
{
}

size_t GLBuffer::ByteArrayUploader::size()
{
    return _bytes->length();
}

void GLBuffer::ByteArrayUploader::upload(const GLBuffer::UploadFunc& uploader)
{
    uploader(_bytes->buf(), _bytes->length());
}

}
