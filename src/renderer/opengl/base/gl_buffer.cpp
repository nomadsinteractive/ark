#include "renderer/opengl/base/gl_buffer.h"

#include "core/util/log.h"

#include "renderer/base/recycler.h"
#include "renderer/inf/uploader.h"

namespace ark {
namespace opengl {

GLBuffer::GLBuffer(Buffer::Type type, Buffer::Usage usage, const sp<Recycler>& recycler, const sp<Uploader>& uploader)
    : Buffer::Delegate(uploader ? uploader->size() : 0), _type(type == Buffer::TYPE_VERTEX ? GL_ARRAY_BUFFER : GL_ELEMENT_ARRAY_BUFFER),
      _usage(usage == Buffer::USAGE_DYNAMIC ? GL_DYNAMIC_DRAW : GL_STATIC_READ), _recycler(recycler), _uploader(uploader), _id(0)
{
}

GLBuffer::~GLBuffer()
{
    if(_id > 0)
        _recycler->recycle(*this);
}

void GLBuffer::reload(GraphicsContext& graphicsContext, const sp<Uploader>& transientUploader)
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

void GLBuffer::doUpload(GraphicsContext& /*graphicsContext*/, Uploader& uploader)
{
    glBindBuffer(_type, _id);
    GLint bufsize = 0;
    glGetBufferParameteriv(_type, GL_BUFFER_SIZE, &bufsize);

    _size = uploader.size();
    if(static_cast<size_t>(bufsize) < _size)
        glBufferData(_type, static_cast<GLsizeiptr>(_size), nullptr, _usage);
    size_t offset = 0;
    const Uploader::UploadFunc func = [&offset, this](void* data, size_t size) {
        DASSERT(data);
        DCHECK(offset + size <= _size, "GLBuffer data overflow");
        glBufferSubData(_type, static_cast<GLsizeiptr>(offset), static_cast<GLsizeiptr>(size), data);
        offset += size;
    };
    uploader.upload(func);
    glBindBuffer(_type, 0);
}

uintptr_t GLBuffer::id()
{
    return _id;
}

void GLBuffer::upload(GraphicsContext& graphicsContext)
{
    reload(graphicsContext, nullptr);
}

Resource::RecycleFunc GLBuffer::recycle()
{
    uint32_t id = _id;
    _id = 0;
    _size = 0;
    return [id](GraphicsContext&) {
        LOGD("Deleting GLBuffer[%d]", id);
        glDeleteBuffers(1, &id);
    };
}

}
}
