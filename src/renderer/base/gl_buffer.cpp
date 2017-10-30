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

GLBuffer::Stub::Stub(const sp<GLRecycler>& recycler, const sp<Variable<bytearray>>& buffer, GLenum type, GLenum usage)
    : _recycler(recycler), _buffer(buffer), _type(type), _usage(usage), _id(0), _size(0)
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

void GLBuffer::Stub::prepare(const bytearray& array)
{
    if(_id == 0)
        glGenBuffers(1, &_id);
    if(array) {
        glBindBuffer(_type, _id);
        _size = array->length();
        glBufferData(_type, _size, array->array(), _usage);
        glBindBuffer(_type, 0);
    }
}

GLuint GLBuffer::Stub::id()
{
    return _id;
}

void GLBuffer::Stub::prepare(GraphicsContext&)
{
    prepare(_buffer ? _buffer->val() : bytearray::null());
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

GLBuffer::Snapshot::Snapshot(const sp<GLBuffer::Stub>& stub, const bytearray& array)
    : _stub(stub), _array(array)
{
}

GLBuffer::Snapshot::Snapshot(const GLBuffer::Snapshot& other)
    : _stub(other._stub), _array(other._array)
{
}

GLBuffer::Snapshot::Snapshot(GLBuffer::Snapshot&& other)
    : _stub(std::move(other._stub)), _array(std::move(other._array))
{
}

uint32_t GLBuffer::Snapshot::id() const
{
    return _stub->id();
}

void GLBuffer::Snapshot::prepare(GraphicsContext&) const
{
    _stub->prepare(_array);
}

GLBuffer::GLBuffer(const sp<GLRecycler>& recycler, const sp<Variable<bytearray>>& buffer, GLenum type, GLenum usage, uint32_t size)
    : _stub(sp<Stub>::make(recycler, buffer, type, usage)), _size(size)
{
}

GLBuffer::GLBuffer(const GLBuffer& other) noexcept
    : _stub(other._stub), _size(other._size)
{
}

GLBuffer::GLBuffer(GLBuffer&& other) noexcept
    : _stub(std::move(other._stub)), _size(other._size)
{
}

GLBuffer::GLBuffer() noexcept
    : _stub(nullptr), _size(0)
{
}

const GLBuffer& GLBuffer::operator =(const GLBuffer& other) noexcept
{
    _stub = other._stub;
    _size = other._size;
    return *this;
}

const GLBuffer& GLBuffer::operator =(GLBuffer&& other) noexcept
{
    _stub = std::move(other._stub);
    _size = other._size;
    return *this;
}

void GLBuffer::setSize(uint32_t size)
{
    _size = size;
}

GLBuffer::operator bool() const
{
    return !!_stub;
}

GLenum GLBuffer::type() const
{
    return _stub->type();
}

GLBuffer::Snapshot GLBuffer::snapshot(const bytearray& array) const
{
    return Snapshot(_stub, array);
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

void GLBuffer::recycle(GraphicsContext& graphicsContext) const
{
    _stub->recycle(graphicsContext);
}

}
