#ifndef ARK_RENDERER_OPENGL_UTIL_GL_UTIL_H_
#define ARK_RENDERER_OPENGL_UTIL_GL_UTIL_H_

#include "core/forwarding.h"
#include "core/base/api.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/base/texture.h"

#include "platform/gl/gl.h"

namespace ark {

class ARK_API GLUtil {
public:
    static GLenum getEnum(const String& name);
    static GLenum getEnum(const String& name, GLenum defValue);

    static GLenum getTextureInternalFormat(int32_t format, const Bitmap& bitmap);
    static GLenum getTextureFormat(int32_t format, uint8_t channels);
    static GLenum getPixelFormat(int32_t format, const Bitmap& bitmap);

    static sp<Texture::Parameters> getTextureParameters(Texture::Format format = Texture::FORMAT_AUTO, Texture::Feature features = Texture::FEATURE_DEFAULT);
    static sp<Texture::Parameters> getTextureParameters(const document& manifest);

    static bytearray makeUnitCubeVertices();

    static void renderCubemap(GraphicsContext& graphicsContext, uint32_t id, RenderController& renderController, Shader& shader, Texture& texture, int32_t width, int32_t height);
};

}

#endif
