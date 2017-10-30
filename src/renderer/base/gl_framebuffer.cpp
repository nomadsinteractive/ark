#include "renderer/base/gl_framebuffer.h"

#include "renderer/base/gl_recycler.h"
#include "renderer/base/gl_texture.h"
#include "renderer/util/gl_debug.h"

#include "platform/gl/gl.h"

namespace ark {

GLFramebuffer::Recycler::Recycler(uint32_t id)
    : _id(id) {
}

uint32_t GLFramebuffer::Recycler::id()
{
    return _id;
}

void GLFramebuffer::Recycler::prepare(GraphicsContext&)
{
}

void GLFramebuffer::Recycler::recycle(GraphicsContext&)
{
    LOGD("Deleting GLFramebuffer[%d]", _id);
    glDeleteFramebuffers(1, &_id);
    _id = 0;
}

GLFramebuffer::GLFramebuffer(const sp<GLRecycler>& recycler, const sp<GLTexture>& texture)
    : _recycler(recycler), _texture(texture), _id(0)
{
}

GLFramebuffer::~GLFramebuffer()
{
    if(_id)
        _recycler->recycle(sp<Recycler>::make(_id));
}

uint32_t GLFramebuffer::id()
{
    return _id;
}

void GLFramebuffer::prepare(GraphicsContext& graphicsContext)
{
    if(_id == 0)
        glGenFramebuffers(1, &_id);

    if(!_texture->id())
        _texture->prepare(graphicsContext);

    glBindFramebuffer(GL_FRAMEBUFFER, _id);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _texture->id(), 0);
    const GLenum drawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, drawBuffers);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        GLDebug::glCheckError("FrameBuffer");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GLFramebuffer::recycle(GraphicsContext&)
{
    LOGD("Deleting GLFramebuffer[%d]", _id);
    glDeleteFramebuffers(1, &_id);
    _id = 0;
}

const sp<GLTexture>& GLFramebuffer::texture() const
{
    return _texture;
}

}
