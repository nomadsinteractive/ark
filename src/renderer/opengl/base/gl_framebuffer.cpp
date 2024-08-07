#include "renderer/opengl/base/gl_framebuffer.h"

#include "core/collection/table.h"

#include "renderer/base/recycler.h"
#include "renderer/base/texture.h"

#include "renderer/opengl/base/gl_texture.h"
#include "renderer/opengl/base/gl_renderbuffer.h"
#include "renderer/opengl/util/gl_debug.h"

#include "platform/gl/gl.h"

namespace ark::opengl {

GLFramebuffer::GLFramebuffer(sp<Recycler> recycler, std::vector<sp<Texture>> colorAttachments, sp<Texture> depthStencilAttachments)
    : _recycler(std::move(recycler)), _color_attachments(std::move(colorAttachments)), _depth_stencil_attachment(depthStencilAttachments), _id(0)
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

void GLFramebuffer::upload(GraphicsContext& graphicsContext)
{
    if(_id == 0)
    {
        glGenFramebuffers(1, &_id);
        LOGD("Generating GLFramebuffer[%d]", _id);
    }

    uint32_t bindings = 0;
    Table<uint64_t, GLenum> attachments;

    sp<Texture> depthTexture;
    std::vector<GLenum> depthInputs;
    GLenum depthInternalformat;

    uint32_t idx = 0;
    std::vector<GLenum> drawBuffers;

    for(const sp<Texture>& i : _color_attachments)
    {
        const Texture::Usage usage = i->parameters()->_usage;
        DASSERT(usage & Texture::USAGE_ATTACHMENT);
        DASSERT(i->id() != 0);
        GLenum attachment = static_cast<GLenum>(GL_COLOR_ATTACHMENT0 + (bindings++));
        attachments.push_back(i->id(), attachment);
        drawBuffers.push_back(static_cast<GLenum>(GL_COLOR_ATTACHMENT0 + (idx++)));
    }

    if(_depth_stencil_attachment)
    {
        uint32_t usage = _depth_stencil_attachment->parameters()->_usage & Texture::USAGE_DEPTH_STENCIL_ATTACHMENT;
        DASSERT(_depth_stencil_attachment->id() != 0);
        const GLenum glAttachments[] = {GL_COLOR_ATTACHMENT0, GL_DEPTH_ATTACHMENT, GL_STENCIL_ATTACHMENT, GL_DEPTH_STENCIL_ATTACHMENT};
        DASSERT(usage & (Texture::USAGE_DEPTH_ATTACHMENT | Texture::USAGE_STENCIL_ATTACHMENT));
        if(_depth_stencil_attachment->parameters()->_flags & Texture::FLAG_FOR_INPUT)
        {
            depthTexture = _depth_stencil_attachment;
            if(usage == Texture::USAGE_DEPTH_STENCIL_ATTACHMENT)
            {
                depthInputs.push_back(GL_DEPTH_ATTACHMENT);
                depthInputs.push_back(GL_STENCIL_ATTACHMENT);
                depthInternalformat = GL_DEPTH24_STENCIL8;
            }
            else if (usage & Texture::USAGE_DEPTH_ATTACHMENT)
            {
                depthInputs.push_back(glAttachments[usage]);
                depthInternalformat = GL_DEPTH_COMPONENT;
            }
            else if (usage & Texture::USAGE_STENCIL_ATTACHMENT)
            {
#ifndef ANDROID
                depthInputs.push_back(glAttachments[usage]);
                depthInternalformat = GL_STENCIL_COMPONENTS;
#else
                LOGW("GL_STENCIL_COMPONENTS unsupported");
                depthTexture = nullptr;
#endif
            }
            else
                LOGE("Unknow depth-stencil texture usage: %d", usage);
        }
        attachments.push_back(_depth_stencil_attachment->id(), glAttachments[usage]);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, _id);
    glDrawBuffers(static_cast<uint32_t>(drawBuffers.size()), drawBuffers.data());
    for(const auto& [k, v] : attachments)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, v, GL_TEXTURE_2D, static_cast<GLuint>(k), 0);
        LOGD("glFramebufferTexture2D, attachment: %d, id: %d", v, k);
    }

    if(depthInputs.size() > 0 && depthTexture->parameters()->_flags & Texture::FLAG_FOR_OUTPUT)
    {
        sp<GLTexture> gltex = depthTexture->delegate();
        sp<GLRenderbuffer> renderbuffer = gltex->renderbuffer();
        if(!renderbuffer)
        {
            renderbuffer = sp<GLRenderbuffer>::make(_recycler);
            gltex->setRenderbuffer(renderbuffer);
            renderbuffer->upload(graphicsContext);
        }
        glBindRenderbuffer(GL_RENDERBUFFER, static_cast<GLuint>(renderbuffer->id()));
        glRenderbufferStorage(GL_RENDERBUFFER, depthInternalformat, static_cast<GLsizei>(depthTexture->width()), static_cast<GLsizei>(depthTexture->height()));
        for(GLenum i : depthInputs)
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, i, GL_RENDERBUFFER, static_cast<GLuint>(renderbuffer->id()));
    }

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        GLDebug::glCheckError("FrameBuffer");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

ResourceRecycleFunc GLFramebuffer::recycle()
{
    uint32_t id = _id;
    _id = 0;
    return [id](GraphicsContext&) {
        LOGD("Deleting GLFramebuffer[%d]", id);
        glDeleteFramebuffers(1, &id);
    };
}

}
