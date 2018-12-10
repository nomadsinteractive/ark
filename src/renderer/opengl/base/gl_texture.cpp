#include "renderer/opengl/base/gl_texture.h"

#include "core/inf/array.h"
#include "core/inf/variable.h"
#include "core/util/conversions.h"
#include "core/util/documents.h"
#include "core/util/log.h"

#include "graphics/base/bitmap.h"
#include "graphics/base/size.h"

#include "renderer/base/recycler.h"
#include "renderer/base/resource_manager.h"
#include "renderer/base/gl_texture_loader.h"
#include "renderer/base/graphics_context.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/opengl/util/gl_util.h"

namespace ark {

GLTexture::GLTexture(const sp<Recycler>& recycler, const sp<Size>& size, uint32_t target, const sp<Texture::Parameters>& parameters)
    : _recycler(recycler), _size(size), _target(target), _parameters(parameters), _id(0)
{
}

GLTexture::~GLTexture()
{
    if(_id > 0)
        _recycler->recycle(*this);
}

void GLTexture::upload(GraphicsContext& graphicsContext)
{
    if(_id == 0)
        glGenTextures(1, &_id);

    glBindTexture(static_cast<GLenum>(_target), _id);
    doPrepareTexture(graphicsContext, _id);

    if(_parameters->_features & Texture::FEATURE_MIPMAPS)
        glGenerateMipmap(static_cast<GLenum>(_target));

    for(const auto& i : _parameters->_tex_parameters)
        glTexParameteri(static_cast<GLenum>(_target), static_cast<GLenum>(i.first), static_cast<GLint>(i.second));
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

uint32_t GLTexture::id()
{
    return _id;
}

}
