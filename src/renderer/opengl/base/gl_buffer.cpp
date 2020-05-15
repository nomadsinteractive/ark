#include "renderer/opengl/base/gl_buffer.h"

#include "core/util/log.h"

#include "renderer/base/recycler.h"
#include "renderer/inf/uploader.h"

#include "renderer/opengl/util/gl_util.h"

namespace ark {
namespace opengl {

GLBuffer::GLBuffer(Buffer::Type type, Buffer::Usage usage, const sp<Recycler>& recycler)
    : _type(GLUtil::toBufferType(type)), _usage(usage == Buffer::USAGE_DYNAMIC ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW), _recycler(recycler), _id(0)
{
}

GLBuffer::~GLBuffer()
{
    _recycler->recycle(*this);
}

void GLBuffer::doUpload(GraphicsContext& /*graphicsContext*/, Uploader& uploader)
{
    glBindBuffer(_type, _id);
    GLint bufsize = 0;
    glGetBufferParameteriv(_type, GL_BUFFER_SIZE, &bufsize);

    _size = uploader.size();
    if(static_cast<size_t>(bufsize) < _size)
        glBufferData(_type, static_cast<GLsizeiptr>(_size), nullptr, _usage);
    Uploader::UploadFunc func = [this](void* data, size_t size, size_t offset) {
        DASSERT(data);
        DCHECK(offset + size <= _size, "GLBuffer data overflow");
        glBufferSubData(_type, static_cast<GLsizeiptr>(offset), static_cast<GLsizeiptr>(size), data);
    };
    uploader.upload(func);
    glBindBuffer(_type, 0);
}

uint64_t GLBuffer::id()
{
    return _id;
}

void GLBuffer::upload(GraphicsContext& graphicsContext, const sp<Uploader>& uploader)
{
    if(_id == 0)
        glGenBuffers(1, &_id);

    if(uploader)
    {
        DWARN(_usage != GL_STATIC_DRAW || _size == 0, "Uploading transient data to GL_STATIC_DRAW GLBuffer");
        doUpload(graphicsContext, uploader);
    }
}

Resource::RecycleFunc GLBuffer::recycle()
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
}
