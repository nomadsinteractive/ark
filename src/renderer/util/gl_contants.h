#ifndef ARK_RENDERER_UTIL_GL_CONTANTS_H_
#define ARK_RENDERER_UTIL_GL_CONTANTS_H_

#include <map>

#include "core/base/string.h"

#include "graphics/forwarding.h"

#include "renderer/base/gl_texture.h"

#include "platform/gl/gl.h"

namespace ark {

class GLConstants {
public:
    GLConstants();

    GLenum getEnum(const String& name);
    GLenum getEnum(const String& name, GLenum defValue);

    static GLenum getTextureInternalFormat(GLTexture::Format format, const Bitmap& bitmap);
    static GLenum getTextureFormat(GLTexture::Format format, uint8_t channels);
    static GLenum getPixelFormat(GLTexture::Format format, const Bitmap& bitmap);

private:
    void initConstants();

private:
    std::map<String, GLenum> _constants;
};

}

#endif
