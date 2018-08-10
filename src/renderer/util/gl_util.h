#ifndef ARK_RENDERER_UTIL_GL_UTIL_H_
#define ARK_RENDERER_UTIL_GL_UTIL_H_

#include "core/forwarding.h"

#include "graphics/forwarding.h"

#include "platform/gl/gl.h"

namespace ark {

class GLUtil {
public:
    static GLenum getEnum(const String& name);
    static GLenum getEnum(const String& name, GLenum defValue);

    static GLenum getTextureInternalFormat(int32_t format, const Bitmap& bitmap);
    static GLenum getTextureFormat(int32_t format, uint8_t channels);
    static GLenum getPixelFormat(int32_t format, const Bitmap& bitmap);
};

}

#endif
