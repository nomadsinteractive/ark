#include "renderer/opengl/base/gl_framebuffer.h"

#include "core/collection/table.h"

#include "renderer/base/recycler.h"
#include "renderer/base/texture.h"
#include "renderer/opengl/util/gl_debug.h"

#include "platform/gl/gl.h"

namespace ark {
namespace opengl {

GLFramebuffer::GLFramebuffer(const sp<Recycler>& recycler, std::vector<sp<Texture>> textures)
    : _recycler(recycler), _textures(std::move(textures)), _id(0), _render_buffer_id(0)
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

    sp<Texture> depthTexture;
    std::vector<GLenum> drawAttachments;
    std::vector<GLenum> depthAttachments;
    GLenum depthInternalformat;

    for(const sp<Texture>& i : _textures)
    {
        Texture::Usage usage = i->parameters()->_usage;
        i->upload(graphicsContext, nullptr);
        GLenum attachment = GL_COLOR_ATTACHMENT0;
        if(usage == Texture::USAGE_COLOR_ATTACHMENT)
        {
            attachment = static_cast<GLenum>(GL_COLOR_ATTACHMENT0 + (bindings++));
            drawAttachments.push_back(attachment);
        }
        else if(usage & Texture::USAGE_DEPTH_ATTACHMENT & Texture::USAGE_STENCIL_ATTACHMENT)
        {
            attachment = GL_DEPTH_STENCIL_ATTACHMENT;
            depthTexture = i;
            depthAttachments.push_back(GL_DEPTH_ATTACHMENT);
            depthAttachments.push_back(GL_STENCIL_ATTACHMENT);
            depthInternalformat = GL_DEPTH24_STENCIL8;
        }
        else if (usage & Texture::USAGE_DEPTH_ATTACHMENT)
        {
            attachment = GL_DEPTH_ATTACHMENT;
            depthTexture = i;
            depthAttachments.push_back(attachment);
            depthInternalformat = GL_DEPTH_COMPONENT;
        }
        else if (usage & Texture::USAGE_STENCIL_ATTACHMENT)
        {
            attachment = GL_STENCIL_ATTACHMENT;
            depthTexture = i;
            depthAttachments.push_back(attachment);
            depthInternalformat = GL_STENCIL_COMPONENTS;
        }
        attachments.push_back(i->id(), attachment);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, _id);
    for(const auto& i : attachments)
        glFramebufferTexture2D(GL_FRAMEBUFFER, i.second, GL_TEXTURE_2D, static_cast<GLuint>(i.first), 0);

    glDrawBuffers(static_cast<GLsizei>(drawAttachments.size()), drawAttachments.data());

    if(depthAttachments.size() > 0)
    {
        if(_render_buffer_id == 0)
            glGenRenderbuffers(1, &_render_buffer_id);
        glBindRenderbuffer(GL_RENDERBUFFER, _render_buffer_id);
        glRenderbufferStorage(GL_RENDERBUFFER, depthInternalformat, static_cast<GLsizei>(depthTexture->width()), static_cast<GLsizei>(depthTexture->height()));
        for(GLenum i : depthAttachments)
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, i, GL_RENDERBUFFER, _render_buffer_id);
    }

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        GLDebug::glCheckError("FrameBuffer");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Resource::RecycleFunc GLFramebuffer::recycle()
{
    uint32_t id = _id;
    uint32_t renderBufferId = _render_buffer_id;
    _id = 0;
    return [id, renderBufferId](GraphicsContext&) {
        LOGD("Deleting GLFramebuffer[%d]", id);
        glDeleteFramebuffers(1, &id);

        if(renderBufferId > 0) {
            LOGD("Deleting GLRenderbuffer[%d]", renderBufferId);
            glDeleteRenderbuffers(1, &renderBufferId);
        }
    };
}

const std::vector<sp<Texture>>& GLFramebuffer::textures() const
{
    return _textures;
}

}
}
