#include "renderer/opengl/base/gl_texture_2d.h"

#include "core/inf/variable.h"
#include "core/util/log.h"

#include "graphics/base/bitmap.h"
#include "graphics/base/size.h"

#include "renderer/base/texture.h"

#include "renderer/opengl/util/gl_util.h"

#include "platform/gl/gl.h"

namespace ark {

GLTexture2D::GLTexture2D(const sp<Recycler>& recycler, const sp<Size>& size, const sp<Texture::Parameters>& parameters, const sp<Texture::Uploader>& uploader)
    : GLTexture(recycler, size, static_cast<uint32_t>(GL_TEXTURE_2D), Texture::TYPE_2D, parameters, uploader)
{
}

bool GLTexture2D::download(GraphicsContext& /*graphicsContext*/, Bitmap& bitmap)
{
    DCHECK(static_cast<uint32_t>(_size->width()) == bitmap.width() && static_cast<uint32_t>(_size->height()) == bitmap.height(), "Size mismatch: texture(%d, %d) vs bitmap(%d, %d)",
           static_cast<uint32_t>(_size->width()), static_cast<uint32_t>(_size->height()), bitmap.width(), bitmap.height());
    GLenum textureFormat = GLUtil::getTextureFormat(Texture::FORMAT_AUTO, bitmap.channels());
    GLenum pixelFormat = GLUtil::getPixelFormat(Texture::FORMAT_AUTO, bitmap);
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
    glGetTexImage(GL_TEXTURE_2D, 0, textureFormat, pixelFormat, bitmap.at(0, 0));
    glBindTexture(GL_TEXTURE_2D, 0);
#endif
    return true;
}

void GLTexture2D::uploadBitmap(GraphicsContext& /*graphicContext*/, const Bitmap& bitmap, const std::vector<sp<ByteArray>>& imagedata)
{
    DASSERT(imagedata.size() == 1);
    uint8_t channels = bitmap.channels();
    GLenum format = GLUtil::getTextureFormat(_parameters->_format, channels);
    GLenum pixelFormat = GLUtil::getPixelFormat(_parameters->_format, bitmap);
    GLenum internalFormat = GLUtil::getTextureInternalFormat(_parameters->_format, bitmap);
    glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(internalFormat), static_cast<int32_t>(bitmap.width()), static_cast<int32_t>(bitmap.height()), 0, format, pixelFormat, imagedata.at(0)->buf());
    LOGD("Texture Uploaded, id = %d, width = %d, height = %d", static_cast<uint32_t>(id()), bitmap.width(), bitmap.height());
}

}
