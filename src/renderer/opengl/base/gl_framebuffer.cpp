#include "renderer/opengl/base/gl_framebuffer.h"

#include "core/collection/table.h"

#include "renderer/base/recycler.h"
#include "renderer/base/texture.h"

#include "renderer/opengl/base/gl_texture.h"
#include "renderer/opengl/base/gl_renderbuffer.h"
#include "renderer/opengl/util/gl_debug.h"

#include "platform/gl/gl.h"

namespace ark {
namespace opengl {

GLFramebuffer::GLFramebuffer(const sp<Recycler>& recycler, std::vector<sp<Texture>> colorAttachments, std::vector<sp<Texture>> renderBufferAttachments)
    : _recycler(recycler), _color_attachments(std::move(colorAttachments)), _render_buffer_attachments(renderBufferAttachments), _id(0)
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
    std::vector<GLenum> depthAttachments;
    GLenum depthInternalformat;

    for(const sp<Texture>& i : _color_attachments)
    {
        Texture::Usage usage = i->parameters()->_usage;
        DASSERT(usage == Texture::USAGE_COLOR_ATTACHMENT);

        i->upload(graphicsContext, nullptr);
        GLenum attachment = static_cast<GLenum>(GL_COLOR_ATTACHMENT0 + (bindings++));
        attachments.push_back(i->id(), attachment);
    }
    for(const sp<Texture>& i : _render_buffer_attachments)
    {
        Texture::Usage usage = i->parameters()->_usage;
        i->upload(graphicsContext, nullptr);
        const GLenum glAttachments[] = {GL_COLOR_ATTACHMENT0, GL_DEPTH_ATTACHMENT, GL_STENCIL_ATTACHMENT, GL_DEPTH_STENCIL_ATTACHMENT};
        DASSERT(usage & (Texture::USAGE_DEPTH_ATTACHMENT | Texture::USAGE_STENCIL_ATTACHMENT));
        if(usage == Texture::USAGE_DEPTH_STENCIL_ATTACHMENT)
        {
            depthTexture = i;
            depthAttachments.push_back(GL_DEPTH_ATTACHMENT);
            depthAttachments.push_back(GL_STENCIL_ATTACHMENT);
            depthInternalformat = GL_DEPTH24_STENCIL8;
        }
        else if (usage & Texture::USAGE_DEPTH_ATTACHMENT)
        {
            depthTexture = i;
            depthAttachments.push_back(glAttachments[usage]);
            depthInternalformat = GL_DEPTH_COMPONENT;
        }
        else if (usage & Texture::USAGE_STENCIL_ATTACHMENT)
        {
#ifndef ANDROID
            depthTexture = i;
            depthAttachments.push_back(glAttachments[usage]);
            depthInternalformat = GL_STENCIL_COMPONENTS;
#else
            WARN(false, "GL_STENCIL_COMPONENTS unsupported");
#endif
        }
        attachments.push_back(i->id(), glAttachments[usage]);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, _id);
    for(const auto& i : attachments)
        glFramebufferTexture2D(GL_FRAMEBUFFER, i.second, GL_TEXTURE_2D, static_cast<GLuint>(i.first), 0);

    if(depthAttachments.size() > 0)
    {
        sp<GLTexture> gltex = depthTexture->delegate();
        sp<GLRenderbuffer> renderbuffer = gltex->renderbuffer();
        if(!renderbuffer)
        {
            renderbuffer = sp<GLRenderbuffer>::make(_recycler);
            gltex->setRenderbuffer(renderbuffer);
            renderbuffer->upload(graphicsContext, nullptr);
        }
        glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer->id());
        glRenderbufferStorage(GL_RENDERBUFFER, depthInternalformat, static_cast<GLsizei>(depthTexture->width()), static_cast<GLsizei>(depthTexture->height()));
        for(GLenum i : depthAttachments)
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, i, GL_RENDERBUFFER, renderbuffer->id());
    }

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

}
}
