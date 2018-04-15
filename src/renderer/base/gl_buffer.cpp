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

GLBuffer::Stub::Stub(const sp<GLRecycler>& recycler, const sp<GLBuffer::Uploader>& uploader, GLenum type, GLenum usage)
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

void GLBuffer::Stub::prepare(GraphicsContext& graphicsContext, const bytearray& transientData)
{
    if(_id == 0)
    {
        glGenBuffers(1, &_id);
        if(_uploader)
        {
            upload(graphicsContext, _uploader);
            return;
        }
    }
    if(transientData) {
        DWARN(_usage != GL_STATIC_DRAW, "Uploading transient data to GL_STATIC_DRAW GLBuffer");
        uint32_t osize = _size;
        glBindBuffer(_type, _id);
        _size = transientData->length();
        if(osize < transientData->length())
            glBufferData(_type, _size, transientData->buf(), _usage);
        else
            glBufferSubData(_type, 0, _size, transientData->buf());
        glBindBuffer(_type, 0);
    }
}

void GLBuffer::Stub::upload(GraphicsContext& graphicsContext, Uploader& uploader)
{
    bool recreate = _size < uploader.size();
    glBindBuffer(_type, _id);
    _size = uploader.size();
    if(recreate)
        glBufferData(_type, _size, nullptr, _usage);
    uploader.upload(graphicsContext, _type);
    glBindBuffer(_type, 0);
}

GLuint GLBuffer::Stub::id()
{
    return _id;
}

void GLBuffer::Stub::prepare(GraphicsContext& graphicsContext)
{
    prepare(graphicsContext, bytearray::null());
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
    _stub->prepare(graphicsContext, _array);
}

GLBuffer::GLBuffer(const sp<GLRecycler>& recycler, const sp<GLBuffer::Uploader>& uploader, GLenum type, GLenum usage)
    : _stub(sp<Stub>::make(recycler, uploader, type, usage)), _size(uploader ? uploader->size() : 0)
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
