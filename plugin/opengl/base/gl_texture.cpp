#include "opengl/base/gl_texture.h"

#include "core/inf/array.h"
#include "core/util/log.h"

#include "graphics/base/bitmap.h"
#include "graphics/components/size.h"

#include "renderer/base/recycler.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/graphics_context.h"
#include "renderer/base/resource_loader_context.h"
#include "opengl/util/gl_util.h"

namespace ark::plugin::opengl {

GLTexture::GLTexture(sp<Recycler> recycler, sp<Size> size, const uint32_t target, const Texture::Type type, sp<Texture::Parameters> parameters)
    : Texture::Delegate(type), _recycler(std::move(recycler)), _size(std::move(size)), _target(target), _parameters(std::move(parameters)), _id(0), _fbo(0)
{
}

GLTexture::~GLTexture()
{
    if(_id)
        _recycler->recycle(doRecycle());
}

void GLTexture::upload(GraphicsContext& graphicsContext, const sp<Texture::Uploader>& uploader)
{
    const bool uninitialized = _id == 0;
    if(uninitialized)
    {
        constexpr GLenum glParameters[Texture::FILTER_COUNT] = {GL_NEAREST, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_BORDER, GL_MIRRORED_REPEAT, GL_REPEAT, GL_MIRROR_CLAMP_TO_EDGE};

        glGenTextures(1, &_id);
        LOGD("Generating GLTexture[%d]", _id);
        GL_CHECK_ERROR(glBindTexture(static_cast<GLenum>(_target), _id));
        glTexParameteri(static_cast<GLenum>(_target), GL_TEXTURE_MIN_FILTER, static_cast<GLint>(glParameters[_parameters->_min_filter]));
        glTexParameteri(static_cast<GLenum>(_target), GL_TEXTURE_MAG_FILTER, static_cast<GLint>(glParameters[_parameters->_mag_filter]));
        glTexParameteri(static_cast<GLenum>(_target), GL_TEXTURE_WRAP_S, static_cast<GLint>(glParameters[_parameters->_wrap_s]));
        glTexParameteri(static_cast<GLenum>(_target), GL_TEXTURE_WRAP_T, static_cast<GLint>(glParameters[_parameters->_wrap_t]));
        glTexParameteri(static_cast<GLenum>(_target), GL_TEXTURE_WRAP_R, static_cast<GLint>(glParameters[_parameters->_wrap_r]));
    }
    else
        GL_CHECK_ERROR(glBindTexture(static_cast<GLenum>(_target), _id));

    if(uploader)
    {
        if(uninitialized)
            uploader->initialize(graphicsContext, *this);
        else
            uploader->update(graphicsContext, *this);
    }

    if(_parameters->_features & Texture::FEATURE_MIPMAPS)
        GL_CHECK_ERROR(glGenerateMipmap(static_cast<GLenum>(_target)));
}

ResourceRecycleFunc GLTexture::recycle()
{
    return doRecycle();
}

void GLTexture::clear(GraphicsContext& /*graphicsContext*/)
{
    if(_id)
    {
        if(!_fbo)
        {
            glGenFramebuffers(1, &_fbo);
            GL_CHECK_ERROR(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo));
            GL_CHECK_ERROR(glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, _id, 0));
            GL_CHECK_ERROR(glDrawBuffer(GL_COLOR_ATTACHMENT0));
        }
        else
            GL_CHECK_ERROR(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo));
        constexpr GLuint clearColor[4] = {0, 0, 0, 0};
        GL_CHECK_ERROR(glClearBufferuiv(GL_COLOR, 0, clearColor));
        GL_CHECK_ERROR(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));
    }
}

int32_t GLTexture::width() const
{
    return static_cast<int32_t>(_size->widthAsFloat());
}

int32_t GLTexture::height() const
{
    return static_cast<int32_t>(_size->heightAsFloat());
}

uint32_t GLTexture::target() const
{
    return _target;
}

const sp<GLRenderbuffer>& GLTexture::renderbuffer() const
{
    return _renderbuffer;
}

void GLTexture::setRenderbuffer(sp<GLRenderbuffer> renderbuffer)
{
    _renderbuffer = std::move(renderbuffer);
}

ResourceRecycleFunc GLTexture::doRecycle()
{
    uint32_t id = _id;
    uint32_t fbo = _fbo;
    _id = 0;
    _fbo = 0;
    return [id, fbo](GraphicsContext&) {
        LOGD("Deleting GLTexture[%d]", id);
        glDeleteTextures(1, &id);
        if(fbo)
            glDeleteFramebuffers(1, &fbo);
    };
}

uint64_t GLTexture::id()
{
    return _id;
}

}
