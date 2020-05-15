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

GLTexture::GLTexture(const sp<Recycler>& recycler, const sp<Size>& size, uint32_t target, Texture::Type type, const sp<Texture::Parameters>& parameters, const sp<Texture::Uploader>& uploader)
    : Texture::Delegate(type), _recycler(recycler), _size(size), _target(target), _parameters(parameters), _uploader(uploader), _id(0)
{
}

GLTexture::~GLTexture()
{
    _recycler->recycle(*this);
}

void GLTexture::upload(GraphicsContext& graphicsContext, const sp<Uploader>& /*uploader*/)
{
    if(_id == 0)
        glGenTextures(1, &_id);

    glBindTexture(static_cast<GLenum>(_target), _id);

    if(_uploader)
        _uploader->upload(graphicsContext, *this);

    if(_parameters->_features & Texture::FEATURE_MIPMAPS)
        glGenerateMipmap(static_cast<GLenum>(_target));

    static const GLenum glParameters[Texture::CONSTANT_COUNT] = {GL_NEAREST, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_BORDER, GL_MIRRORED_REPEAT, GL_REPEAT, GL_MIRROR_CLAMP_TO_EDGE};

    glTexParameteri(static_cast<GLenum>(_target), GL_TEXTURE_MIN_FILTER, static_cast<GLint>(glParameters[_parameters->_min_filter]));
    glTexParameteri(static_cast<GLenum>(_target), GL_TEXTURE_MAG_FILTER, static_cast<GLint>(glParameters[_parameters->_mag_filter]));
    glTexParameteri(static_cast<GLenum>(_target), GL_TEXTURE_WRAP_S, static_cast<GLint>(glParameters[_parameters->_wrap_s]));
    glTexParameteri(static_cast<GLenum>(_target), GL_TEXTURE_WRAP_T, static_cast<GLint>(glParameters[_parameters->_wrap_t]));
    glTexParameteri(static_cast<GLenum>(_target), GL_TEXTURE_WRAP_R, static_cast<GLint>(glParameters[_parameters->_wrap_r]));
}

Resource::RecycleFunc GLTexture::recycle()
{
    uint32_t id = _id;
    _id = 0;
    return [id](GraphicsContext&) {
        LOGD("Deleting GLTexture[%d]", id);
        glDeleteTextures(1, &id);
    };
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

uint64_t GLTexture::id()
{
    return _id;
}

}
