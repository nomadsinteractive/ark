#include "renderer/opengl/base/gl_texture_2d.h"

#include "core/inf/variable.h"
#include "core/util/log.h"

#include "graphics/base/bitmap.h"
#include "graphics/base/size.h"

#include "renderer/base/texture.h"

#include "renderer/opengl/util/gl_util.h"

#include "platform/gl/gl.h"

namespace ark {

GLTexture2D::GLTexture2D(const sp<Recycler>& recycler, const sp<Size>& size, const sp<Texture::Parameters>& parameters, const sp<Variable<bitmap>>& bitmap)
    : GLTexture(recycler, size, static_cast<uint32_t>(GL_TEXTURE_2D), parameters), _bitmap(bitmap)
{
}

bool GLTexture2D::download(GraphicsContext& /*graphicsContext*/, Bitmap& bitmap)
{
    DCHECK(static_cast<uint32_t>(_size->width()) == bitmap.width() && static_cast<uint32_t>(_size->height()) == bitmap.height(), "Size mismatch: texture(%d, %d) vs bitmap(%d, %d)",
           static_cast<uint32_t>(_size->width()), static_cast<uint32_t>(_size->height()), bitmap.width(), bitmap.height());
    GLenum textureFormat = GLUtil::getTextureFormat(Texture::FORMAT_AUTO, bitmap.channels());
    GLenum pixelFormat = GLUtil::getPixelFormat(Texture::FORMAT_AUTO, bitmap);
    glBindTexture(GL_TEXTURE_2D, _id);
    glGetTexImage(GL_TEXTURE_2D, 0, textureFormat, pixelFormat, bitmap.at(0, 0));
    glBindTexture(GL_TEXTURE_2D, 0);
    return true;
}

void GLTexture2D::upload(GraphicsContext& /*graphicContext*/, uint32_t /*index*/, const Bitmap& bitmap)
{
    uint8_t channels = bitmap.channels();
    GLenum format = GLUtil::getTextureFormat(_parameters->_format, channels);
    GLenum pixelFormat = GLUtil::getPixelFormat(_parameters->_format, bitmap);
    GLenum internalFormat = GLUtil::getTextureInternalFormat(_parameters->_format, bitmap);
    glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(internalFormat), static_cast<int32_t>(bitmap.width()), static_cast<int32_t>(bitmap.height()), 0, format, pixelFormat, bitmap.at(0, 0));
    LOGD("Texture Uploaded, id = %d, width = %d, height = %d", static_cast<uint32_t>(id()), bitmap.width(), bitmap.height());
}

void GLTexture2D::doPrepareTexture(GraphicsContext& /*graphicsContext*/, uint32_t id)
{
    const sp<Bitmap> bitmap = _bitmap ? _bitmap->val() : sp<Bitmap>::null();
    uint8_t channels = bitmap ? bitmap->channels() : 4;
    GLenum format = GLUtil::getTextureFormat(_parameters->_format, channels);
    GLenum pixelFormat = bitmap ? GLUtil::getPixelFormat(_parameters->_format, bitmap) : GL_UNSIGNED_BYTE;
    GLenum internalFormat = bitmap ? GLUtil::getTextureInternalFormat(_parameters->_format, bitmap) : GL_RGBA8;
    glTexImage2D(GL_TEXTURE_2D, 0, (GLint) internalFormat, width(), height(), 0, format, pixelFormat, bitmap ? bitmap->at(0, 0) : nullptr);
    LOGD("Uploaded, id = %d, width = %d, height = %d%s", id, width(), height(), bitmap ? "" : ", bitmap: nullptr");
}

}
