#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"
#include "renderer/base/pipeline_descriptor.h"
#include "renderer/base/texture.h"

#include "platform/gl/gl.h"

namespace ark::plugin::opengl {

class GLUtil {
public:
    static GLenum toEnum(Enum::RenderMode renderMode);
    static GLenum toBufferType(Buffer::Type type);
    static GLenum toShaderType(Enum::ShaderStageBit stage);
    static GLenum toCompareFunc(PipelineDescriptor::CompareFunc func);
    static GLenum toStencilFunc(PipelineDescriptor::StencilFunc func);
    static Optional<GLenum> toBlendFactor(PipelineDescriptor::BlendFactor blendFactor);
    static GLenum toFrontFaceType(PipelineDescriptor::FrontFaceType face);

    static GLenum getEnum(const String& name);
    static GLenum getEnum(const String& name, GLenum defValue);
    static GLenum getEnum(const document& manifest, const String& name);
    static GLenum getEnum(const document& manifest, const String& name, GLenum defValue);

    static GLenum getTextureInternalFormat(Texture::Usage usage, Texture::Format format, const Bitmap& bitmap);
    static GLenum getTextureInternalFormat(Texture::Usage usage, Texture::Format format, uint32_t channelSize, uint32_t componentSize);
    static GLenum getTextureFormat(Texture::Usage usage, Texture::Format format, uint8_t channels);
    static GLenum getPixelType(int32_t format, const Bitmap& bitmap);

    static void renderCubemap(GraphicsContext& graphicsContext, uint32_t id, RenderController& renderController, const sp<Pipeline>& pipeline, Texture& texture, int32_t width, int32_t height);

    static void glTexImage2D(uint32_t index, int32_t n, void* data);
};

class GLBufferBinder {
public:
    GLBufferBinder(GLenum target, GLuint buffer);
    GLBufferBinder(GLBufferBinder&& other);
    ~GLBufferBinder();
    DISALLOW_COPY_AND_ASSIGN(GLBufferBinder);

private:
    GLenum _target;
    GLuint _buffer;
};

}
