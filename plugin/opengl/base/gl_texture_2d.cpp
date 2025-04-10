#include "opengl/base/gl_texture_2d.h"

#include "core/util/log.h"

#include "graphics/base/bitmap.h"
#include "graphics/components/size.h"

#include "renderer/base/texture.h"

#include "opengl/util/gl_util.h"

#include "platform/gl/gl.h"

namespace ark::plugin::opengl {

GLTexture2D::GLTexture2D(sp<Recycler> recycler, sp<Size> size, sp<Texture::Parameters> parameters)
    : GLTexture(std::move(recycler), std::move(size), static_cast<uint32_t>(GL_TEXTURE_2D), Texture::TYPE_2D, std::move(parameters))
{
}

GLTexture2D::~GLTexture2D()
{

}

bool GLTexture2D::download(GraphicsContext& /*graphicsContext*/, Bitmap& bitmap)
{
    DCHECK(static_cast<uint32_t>(_size->widthAsFloat()) == bitmap.width() && static_cast<uint32_t>(_size->heightAsFloat()) == bitmap.height(), "Size mismatch: texture(%d, %d) vs bitmap(%d, %d)",
           static_cast<uint32_t>(_size->widthAsFloat()), static_cast<uint32_t>(_size->heightAsFloat()), bitmap.width(), bitmap.height());
    GLenum textureFormat = GLUtil::getTextureFormat(Texture::USAGE_AUTO, Texture::FORMAT_AUTO, bitmap.channels());
    GLenum pixelType = GLUtil::getPixelType(Texture::FORMAT_AUTO, bitmap);
#ifdef ARK_PLATFORM_ANDROID
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _id, 0);
    glReadPixels(0, 0, bitmap.width(), bitmap.height(), textureFormat, pixelFormat, bitmap.at(0, 0));
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &fbo);
#else
    glBindTexture(GL_TEXTURE_2D, _id);
    glGetTexImage(GL_TEXTURE_2D, 0, textureFormat, pixelType, bitmap.at(0, 0));
    glBindTexture(GL_TEXTURE_2D, 0);
#endif
    return true;
}

void GLTexture2D::uploadBitmap(GraphicsContext& /*graphicContext*/, const Bitmap& bitmap, const Vector<sp<ByteArray>>& imagedata)
{
    DASSERT(imagedata.size() == 1);
    const uint8_t channels = bitmap.channels();
    const GLenum format = GLUtil::getTextureFormat(_parameters->_usage, _parameters->_format, channels);
    const GLenum pixelType = GLUtil::getPixelType(_parameters->_format, bitmap);
    const GLenum internalFormat = GLUtil::getTextureInternalFormat(_parameters->_usage, _parameters->_format, bitmap);
    const sp<ByteArray>& bytes = imagedata.at(0);
    glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(internalFormat), static_cast<int32_t>(bitmap.width()), static_cast<int32_t>(bitmap.height()), 0, format, pixelType, bytes ? bytes->buf() : nullptr);
    LOGD("Texture Uploaded, id = %d, width = %d, height = %d", static_cast<uint32_t>(id()), bitmap.width(), bitmap.height());
}

}
