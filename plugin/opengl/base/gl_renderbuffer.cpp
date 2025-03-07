#include "opengl/base/gl_renderbuffer.h"

#include "core/util/log.h"

#include "renderer/base/recycler.h"

#include "platform/gl/gl.h"

namespace ark::plugin::opengl {

GLRenderbuffer::GLRenderbuffer(const sp<Recycler>& recycler)
    : _recycler(recycler), _id(0)
{
}

GLRenderbuffer::~GLRenderbuffer()
{
    _recycler->recycle(*this);
}

uint64_t GLRenderbuffer::id()
{
    return _id;
}

void GLRenderbuffer::upload(GraphicsContext& /*graphicsContext*/)
{
    if(_id == 0)
        glGenRenderbuffers(1, &_id);
}

ResourceRecycleFunc GLRenderbuffer::recycle()
{
    uint32_t id = _id;
    _id = 0;
    return [id](GraphicsContext&) {
        LOGD("Deleting GLRenderbuffer[%d]", id);
        glDeleteRenderbuffers(1, &id);
    };
}

}