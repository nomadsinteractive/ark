#include "renderer/base/gl_buffer.h"

#include "core/inf/array.h"
#include "core/inf/variable.h"
#include "core/base/memory_pool.h"
#include "core/base/object_pool.h"
#include "core/types/null.h"
#include "core/util/log.h"

#include "renderer/base/gl_recycler.h"
#include "renderer/base/gl_resource_manager.h"

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

size_t GLBuffer::Stub::size() const
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

GLBuffer::Snapshot::Snapshot(const sp<GLBuffer::Stub>& stub)
    : _stub(stub), _size(stub->size())
{
}

GLBuffer::Snapshot::Snapshot(const sp<GLBuffer::Stub>& stub, size_t size)
    : _stub(stub), _size(size)
{
}

GLBuffer::Snapshot::Snapshot(const sp<GLBuffer::Stub>& stub, const sp<Uploader>& uploader)
    : _stub(stub), _uploader(uploader), _size(uploader ? uploader->size() : stub->size())
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

size_t GLBuffer::Snapshot::size() const
{
    return _size;
}

void GLBuffer::Snapshot::prepare(GraphicsContext& graphicsContext) const
{
    _stub->prepare(graphicsContext, _uploader);
}

GLBuffer::GLBuffer(const sp<GLRecycler>& recycler, const sp<Uploader>& uploader, GLenum type, GLenum usage) noexcept
    : _stub(sp<Stub>::make(recycler, uploader, type, usage))
{
}

GLBuffer::GLBuffer() noexcept
    : _stub(nullptr)
{
}

size_t GLBuffer::size() const
{
    return _stub->size();
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

GLBuffer::Snapshot GLBuffer::snapshot(size_t size) const
{
    return Snapshot(_stub, size);
}

GLBuffer::Snapshot GLBuffer::snapshot() const
{
    return Snapshot(_stub);
}

GLuint GLBuffer::id() const
{
    return _stub->id();
}

void GLBuffer::prepare(GraphicsContext& graphicsContext) const
{
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

GLBuffer::ByteArrayListUploader::ByteArrayListUploader(const std::vector<bytearray>& bytesList)
    : _bytes_list(bytesList), _size(0)
{
    for(const bytearray& i : _bytes_list)
        _size += i->length();
}

size_t GLBuffer::ByteArrayListUploader::size()
{
    return _size;
}

void GLBuffer::ByteArrayListUploader::upload(const GLBuffer::UploadFunc& uploader)
{
    for(const bytearray& i : _bytes_list)
        uploader(i->buf(), i->length());
}

GLBuffer::Builder::Builder(const sp<MemoryPool>& memoryPool, const sp<ObjectPool>& objectPool, size_t stride, size_t growCapacity)
    : _memory_pool(memoryPool), _object_pool(objectPool), _stride(stride), _grow_capacity(growCapacity), _ptr(nullptr), _boundary(nullptr), _size(0)
{
}

void GLBuffer::Builder::setGrowCapacity(size_t growCapacity)
{
    _grow_capacity = growCapacity;
}

void GLBuffer::Builder::apply(const bytearray& buf)
{
    if(!buf)
        return;

    DCHECK(buf->length() <= _stride, "Varyings buffer overflow: stride: %d, varyings size: %d", _stride, buf->length());
    DCHECK(_ptr + _stride <= _boundary, "Varyings buffer out of bounds");
    memcpy(_ptr, buf->buf(), buf->length());
}

void GLBuffer::Builder::next()
{
    if(_ptr == _boundary)
        grow();
    else
        _ptr += _stride;

    _size += _stride;
    DCHECK(_ptr <= _boundary, "Array buffer out of bounds");
}

GLBuffer::Snapshot GLBuffer::Builder::snapshot(const GLBuffer& buffer) const
{
    return _buffers.size() == 1 ? buffer.snapshot(_object_pool->obtain<GLBuffer::ByteArrayUploader>(_buffers[0])) :
                                  buffer.snapshot(_object_pool->obtain<GLBuffer::ByteArrayListUploader>(_buffers));
}

size_t GLBuffer::Builder::stride() const
{
    return _stride;
}

size_t GLBuffer::Builder::size() const
{
    return _size;
}

void GLBuffer::Builder::grow()
{
    if(_buffers.size() % 4 == 3)
        _grow_capacity *= 2;

    const bytearray bytes = _memory_pool->allocate(_grow_capacity * _stride);
    _buffers.push_back(bytes);
    _ptr = bytes->buf();
    _boundary = _ptr + bytes->length();
}

}
