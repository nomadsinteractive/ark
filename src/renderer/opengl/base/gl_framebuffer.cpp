#include "renderer/opengl/base/gl_framebuffer.h"

#include "core/collection/table.h"

#include "renderer/base/recycler.h"
#include "renderer/base/texture.h"
#include "renderer/opengl/util/gl_debug.h"

#include "platform/gl/gl.h"

namespace ark {
namespace opengl {

GLFramebuffer::GLFramebuffer(const sp<Recycler>& recycler, std::vector<sp<Texture>> textures)
    : _recycler(recycler), _textures(std::move(textures)), _id(0)
{
}

GLFramebuffer::~GLFramebuffer()
{
    _recycler->recycle(*this);
}

uint64_t GLFramebuffer::id()
{
    return _id;
}

void GLFramebuffer::upload(GraphicsContext& graphicsContext, const sp<Uploader>& /*uploader*/)
{
    if(_id == 0)
        glGenFramebuffers(1, &_id);

    uint32_t bindings = 0;
    Table<uint64_t, GLenum> attachments;
    std::vector<GLenum> drawBuffers;
    for(const sp<Texture>& i : _textures)
    {
        Texture::Usage usage = i->parameters()->_usage;
        i->upload(graphicsContext, nullptr);
        GLenum attachment = GL_COLOR_ATTACHMENT0;
        if(usage == Texture::USAGE_COLOR_ATTACHMENT)
        {
            attachment = static_cast<GLenum>(GL_COLOR_ATTACHMENT0 + (bindings++));
            drawBuffers.push_back(attachment);
        }
        else if(usage & Texture::USAGE_DEPTH_ATTACHMENT & Texture::USAGE_STENCIL_ATTACHMENT)
            attachment = GL_DEPTH_STENCIL_ATTACHMENT;
        else if (usage & Texture::USAGE_DEPTH_ATTACHMENT)
            attachment = GL_DEPTH_ATTACHMENT;
        else if (usage & Texture::USAGE_STENCIL_ATTACHMENT)
            attachment = GL_STENCIL_ATTACHMENT;
        attachments.push_back(i->id(), attachment);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, _id);
    for(const auto& i : attachments)
        glFramebufferTexture2D(GL_FRAMEBUFFER, i.second, GL_TEXTURE_2D, static_cast<GLuint>(i.first), 0);

    glDrawBuffers(static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        GLDebug::glCheckError("FrameBuffer");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Resource::RecycleFunc GLFramebuffer::recycle()
{
    uint32_t id = _id;
    _id = 0;
    return [id](GraphicsContext&) {
        LOGD("Deleting GLFramebuffer[%d]", id);
        glDeleteFramebuffers(1, &id);
    };
}

const std::vector<sp<Texture>>& GLFramebuffer::textures() const
{
    return _textures;
}

}
}
