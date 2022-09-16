#ifndef ARK_RENDERER_OPENGL_UTIL_GL_UTIL_H_
#define ARK_RENDERER_OPENGL_UTIL_GL_UTIL_H_

#include "core/forwarding.h"
#include "core/base/api.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/pipeline_input.h"
#include "renderer/base/texture.h"
#include "renderer/inf/model_loader.h"

#include "platform/gl/gl.h"

namespace ark {

class ARK_API GLUtil {
public:
    static GLenum toEnum(ModelLoader::RenderMode renderMode);
    static GLenum toBufferType(Buffer::Type type);
    static GLenum toShaderType(PipelineInput::ShaderStage stage);
    static GLenum toCompareFunc(PipelineBindings::CompareFunc func);
    static GLenum toStencilFunc(PipelineBindings::StencilFunc func);
    static GLenum toFrontFaceType(PipelineBindings::FrontFaceType face);

    static GLenum getEnum(const String& name);
    static GLenum getEnum(const String& name, GLenum defValue);
    static GLenum getEnum(const document& manifest, const String& name);
    static GLenum getEnum(const document& manifest, const String& name, GLenum defValue);

    static GLenum getTextureInternalFormat(Texture::Usage usage, Texture::Format format, const Bitmap& bitmap);
    static GLenum getTextureFormat(Texture::Usage usage, Texture::Format format, uint8_t channels);
    static GLenum getPixelType(int32_t format, const Bitmap& bitmap);

    static void renderCubemap(GraphicsContext& graphicsContext, uint32_t id, RenderController& renderController, const sp<Pipeline>& pipeline, Texture& texture, int32_t width, int32_t height);

    static void glTexImage2D(uint32_t index, int32_t n, void* data);
};

}

#endif
