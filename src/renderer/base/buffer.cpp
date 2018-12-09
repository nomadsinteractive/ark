#include "renderer/base/buffer.h"

#include "core/inf/array.h"
#include "core/inf/variable.h"
#include "core/base/memory_pool.h"
#include "core/base/object_pool.h"
#include "core/types/null.h"
#include "core/util/log.h"

#include "renderer/base/gl_recycler.h"
#include "renderer/base/gl_resource_manager.h"

namespace ark {

Buffer::Stub::Stub(const sp<GLRecycler>& recycler, const sp<Uploader>& uploader, GLenum type, GLenum usage)
    : _recycler(recycler), _uploader(uploader), _type(type), _usage(usage), _id(0), _size(_uploader ? _uploader->size() : 0)
{
}

Buffer::Stub::~Stub()
{
    if(_id > 0)
        _recycler->recycle(*this);
}

size_t Buffer::Stub::size() const
{
    return _size;
}

void Buffer::Stub::upload(GraphicsContext& graphicsContext, const sp<Uploader>& transientUploader)
{
    if(_id == 0)
    {
        glGenBuffers(1, &_id);
        if(_uploader && !transientUploader)
        {
            doUpload(graphicsContext, _uploader);
            return;
        }
    }
    if(transientUploader)
    {
        DWARN(_usage != GL_STATIC_DRAW, "Uploading transient data to GL_STATIC_DRAW GLBuffer");
        doUpload(graphicsContext, transientUploader);
        _uploader = transientUploader;
    }
}

void Buffer::Stub::doUpload(GraphicsContext& /*graphicsContext*/, Buffer::Uploader& uploader)
{
    glBindBuffer(_type, _id);
    GLint bufsize = 0;
    glGetBufferParameteriv(_type, GL_BUFFER_SIZE, &bufsize);

    _size = uploader.size();
    if(static_cast<size_t>(bufsize) < _size)
        glBufferData(_type, static_cast<GLsizeiptr>(_size), nullptr, _usage);
    size_t offset = 0;
    const UploadFunc func = [&offset, this](void* data, size_t size) {
        DASSERT(data);
        DCHECK(offset + size <= _size, "GLBuffer data overflow");
        glBufferSubData(_type, static_cast<GLsizeiptr>(offset), static_cast<GLsizeiptr>(size), data);
        offset += size;
    };
    uploader.upload(func);
    glBindBuffer(_type, 0);
}

GLuint Buffer::Stub::id()
{
    return _id;
}

void Buffer::Stub::upload(GraphicsContext& graphicsContext)
{
    upload(graphicsContext, nullptr);
}

RenderResource::Recycler Buffer::Stub::recycle()
{
    uint32_t id = _id;
    _id = 0;
    _size = 0;
    return [id](GraphicsContext&) {
        LOGD("Deleting GLBuffer[%d]", id);
        glDeleteBuffers(1, &id);
    };
}

Buffer::Snapshot::Snapshot(const sp<Buffer::Stub>& stub)
    : _stub(stub), _size(stub->size())
{
}

Buffer::Snapshot::Snapshot(const sp<Buffer::Stub>& stub, size_t size)
    : _stub(stub), _size(size)
{
}

Buffer::Snapshot::Snapshot(const sp<Buffer::Stub>& stub, const sp<Uploader>& uploader)
    : _stub(stub), _uploader(uploader), _size(uploader->size())
{
}

uint32_t Buffer::Snapshot::id() const
{
    return _stub->id();
}

size_t Buffer::Snapshot::size() const
{
    return _size;
}

void Buffer::Snapshot::prepare(GraphicsContext& graphicsContext) const
{
    _stub->upload(graphicsContext, _uploader);
}

Buffer::Buffer(const sp<GLRecycler>& recycler, const sp<Uploader>& uploader, GLenum type, GLenum usage) noexcept
    : _stub(sp<Stub>::make(recycler, uploader, type, usage))
{
}

Buffer::Buffer() noexcept
    : _stub(nullptr)
{
}

size_t Buffer::size() const
{
    return _stub->size();
}

Buffer::operator bool() const
{
    return static_cast<bool>(_stub);
}

Buffer::Snapshot Buffer::snapshot(const sp<Uploader>& uploader) const
{
    return Snapshot(_stub, uploader);
}

Buffer::Snapshot Buffer::snapshot(size_t size) const
{
    return Snapshot(_stub, size);
}

Buffer::Snapshot Buffer::snapshot() const
{
    return Snapshot(_stub);
}

GLuint Buffer::id() const
{
    return _stub->id();
}

void Buffer::upload(GraphicsContext& graphicsContext) const
{
    _stub->upload(graphicsContext);
}

Buffer::Builder::Builder(const sp<MemoryPool>& memoryPool, const sp<ObjectPool>& objectPool, size_t stride, size_t growCapacity)
    : _memory_pool(memoryPool), _object_pool(objectPool), _stride(stride), _grow_capacity(growCapacity), _ptr(nullptr), _boundary(nullptr), _size(0)
{
}

void Buffer::Builder::setGrowCapacity(size_t growCapacity)
{
    _grow_capacity = growCapacity;
}

void Buffer::Builder::apply(const bytearray& buf)
{
    if(!buf)
        return;

    DCHECK(buf->length() <= _stride, "Varyings buffer overflow: stride: %d, varyings size: %d", _stride, buf->length());
    DCHECK(_ptr + _stride <= _boundary, "Varyings buffer out of bounds");
    memcpy(_ptr, buf->buf(), buf->length());
}

void Buffer::Builder::next()
{
    if(_ptr == _boundary)
        grow();
    else
        _ptr += _stride;

    _size += _stride;
    DCHECK(_ptr <= _boundary, "Array buffer out of bounds");
}

sp<Buffer::Uploader> Buffer::Builder::makeUploader() const
{
    if(_buffers.size() == 1)
        return _object_pool->obtain<Buffer::ByteArrayUploader>(_buffers[0]);
    return _object_pool->obtain<Buffer::ByteArrayListUploader>(_buffers);
}

size_t Buffer::Builder::stride() const
{
    return _stride;
}

size_t Buffer::Builder::size() const
{
    return _size;
}

void Buffer::Builder::grow()
{
    if(_buffers.size() % 4 == 3)
        _grow_capacity *= 2;

    const bytearray bytes = _memory_pool->allocate(_grow_capacity * _stride);
    _buffers.push_back(bytes);
    _ptr = bytes->buf();
    _boundary = _ptr + bytes->length();
}

Buffer::Uploader::Uploader(size_t size)
    : _size(size)
{
}

size_t Buffer::Uploader::size() const
{
    return _size;
}

}
