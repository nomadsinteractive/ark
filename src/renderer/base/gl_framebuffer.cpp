#include "renderer/base/gl_framebuffer.h"

#include "renderer/base/gl_recycler.h"
#include "renderer/base/texture.h"
#include "renderer/opengl/util/gl_debug.h"

#include "platform/gl/gl.h"

namespace ark {

GLFramebuffer::GLFramebuffer(const sp<GLRecycler>& recycler, const sp<Texture>& texture)
    : _recycler(recycler), _texture(texture), _id(0)
{
}

GLFramebuffer::~GLFramebuffer()
{
    if(_id)
        _recycler->recycle(*this);
}

uint32_t GLFramebuffer::id()
{
    return _id;
}

void GLFramebuffer::upload(GraphicsContext& graphicsContext)
{
    if(_id == 0)
        glGenFramebuffers(1, &_id);

    if(!_texture->id())
        _texture->upload(graphicsContext);

    glBindFramebuffer(GL_FRAMEBUFFER, _id);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _texture->id(), 0);
    const GLenum drawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, drawBuffers);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        GLDebug::glCheckError("FrameBuffer");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

RenderResource::Recycler GLFramebuffer::recycle()
{
    uint32_t id = _id;
    _id = 0;
    return [id](GraphicsContext&) {
        LOGD("Deleting GLFramebuffer[%d]", id);
        glDeleteFramebuffers(1, &id);
    };
}

const sp<Texture>& GLFramebuffer::texture() const
{
    return _texture;
}

}
