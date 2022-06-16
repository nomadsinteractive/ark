#include "renderer/opengl/base/gl_texture.h"

#include "core/inf/array.h"
#include "core/inf/variable.h"
#include "core/util/conversions.h"
#include "core/util/documents.h"
#include "core/util/log.h"

#include "graphics/base/bitmap.h"
#include "graphics/base/size.h"

#include "renderer/base/recycler.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/texture_bundle.h"
#include "renderer/base/graphics_context.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/opengl/util/gl_util.h"

namespace ark {
namespace opengl {

GLTexture::GLTexture(sp<Recycler> recycler, sp<Size> size, uint32_t target, Texture::Type type, sp<Texture::Parameters> parameters)
    : Texture::Delegate(type), _recycler(std::move(recycler)), _size(std::move(size)), _target(target), _parameters(std::move(parameters)), _id(0)
{
}

GLTexture::~GLTexture()
{
    if(_id)
        _recycler->recycle(doRecycle());
}

void GLTexture::upload(GraphicsContext& graphicsContext, const sp<Texture::Uploader>& uploader)
{
    if(_id == 0)
    {
        glGenTextures(1, &_id);
        LOGD("Generating GLTexture[%d]", _id);
    }

    glBindTexture(static_cast<GLenum>(_target), _id);

    if(uploader)
        uploader->upload(graphicsContext, *this);

    if(_parameters->_features & Texture::FEATURE_MIPMAPS)
        glGenerateMipmap(static_cast<GLenum>(_target));

    static const GLenum glParameters[Texture::CONSTANT_COUNT] = {GL_NEAREST, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_BORDER, GL_MIRRORED_REPEAT, GL_REPEAT, GL_MIRROR_CLAMP_TO_EDGE};

    glTexParameteri(static_cast<GLenum>(_target), GL_TEXTURE_MIN_FILTER, static_cast<GLint>(glParameters[_parameters->_min_filter]));
    glTexParameteri(static_cast<GLenum>(_target), GL_TEXTURE_MAG_FILTER, static_cast<GLint>(glParameters[_parameters->_mag_filter]));
    glTexParameteri(static_cast<GLenum>(_target), GL_TEXTURE_WRAP_S, static_cast<GLint>(glParameters[_parameters->_wrap_s]));
    glTexParameteri(static_cast<GLenum>(_target), GL_TEXTURE_WRAP_T, static_cast<GLint>(glParameters[_parameters->_wrap_t]));
    glTexParameteri(static_cast<GLenum>(_target), GL_TEXTURE_WRAP_R, static_cast<GLint>(glParameters[_parameters->_wrap_r]));
}

ResourceRecycleFunc GLTexture::recycle()
{
    return doRecycle();
}

int32_t GLTexture::width() const
{
    return static_cast<int32_t>(_size->width());
}

int32_t GLTexture::height() const
{
    return static_cast<int32_t>(_size->height());
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
    _id = 0;
    return [id](GraphicsContext&) {
        LOGD("Deleting GLTexture[%d]", id);
        glDeleteTextures(1, &id);
    };
}

uint64_t GLTexture::id()
{
    return _id;
}

}
}
