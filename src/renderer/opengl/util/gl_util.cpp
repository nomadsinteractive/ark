#include "renderer/opengl/util/gl_util.h"

#include <unordered_map>

#include "core/types/global.h"
#include "core/util/math.h"

#include "graphics/base/bitmap.h"
#include "graphics/base/mat.h"
#include "graphics/util/matrix_util.h"

#include "renderer/base/buffer.h"
#include "renderer/base/graphics_context.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/render_engine_context.h"
#include "renderer/base/shader.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/base/texture.h"
#include "renderer/util/render_util.h"

#include "renderer/opengl/base/gl_buffer.h"
#include "renderer/opengl/base/gl_pipeline.h"
#include "renderer/opengl/base/gl_texture.h"
#include "renderer/opengl/util/gl_debug.h"

namespace ark {

namespace {

struct GLConstants {
    GLConstants() {
        _enums["nearest"] = GL_NEAREST;
        _enums["linear"] = GL_LINEAR;
        _enums["texture_mag_filter"] = GL_TEXTURE_MAG_FILTER;
        _enums["texture_min_filter"] = GL_TEXTURE_MIN_FILTER;
        _enums["texture_wrap_s"] = GL_TEXTURE_WRAP_S;
        _enums["texture_wrap_t"] = GL_TEXTURE_WRAP_T;
        _enums["texture_wrap_r"] = GL_TEXTURE_WRAP_R;
        _enums["clamp_to_edge"] = GL_CLAMP_TO_EDGE;
        _enums["clamp_to_border"] = GL_CLAMP_TO_BORDER;
        _enums["mirrored_repeat"] = GL_MIRRORED_REPEAT;
        _enums["repeat"] = GL_REPEAT;
        _enums["mirror_clamp_to_edge"] = GL_MIRROR_CLAMP_TO_EDGE;

        _enums["rgba"] = GL_RGBA;
        _enums["rgb"] = GL_RGB;
        _enums["alpha"] = GL_ALPHA;
        _enums["rg"] = GL_RG;

        _enums["always"] = GL_ALWAYS;
        _enums["never"] = GL_NEVER;
        _enums["equal"] = GL_EQUAL;
        _enums["not_equal"] = GL_NOTEQUAL;
        _enums["less"] = GL_LESS;
        _enums["greater"] = GL_GREATER;
        _enums["less_equal"] = GL_LEQUAL;
        _enums["greater_equal"] = GL_GEQUAL;

        _enums["keep"] = GL_KEEP;
        _enums["zero"] = GL_ZERO;
        _enums["replace"] = GL_REPLACE;
        _enums["incr"] = GL_INCR;
        _enums["decr"] = GL_DECR;

        _enums["cw"] = GL_CW;
        _enums["ccw"] = GL_CCW;
    }

    std::unordered_map<String, GLenum> _enums;
};

}

GLenum GLUtil::toEnum(ModelLoader::RenderMode renderMode)
{
    static const GLenum models[ModelLoader::RENDER_MODE_COUNT] = {GL_LINES, GL_POINTS, GL_TRIANGLES, GL_TRIANGLE_STRIP};
    DCHECK(renderMode >= 0 && renderMode < ModelLoader::RENDER_MODE_COUNT, "Unknown Mode: %d", renderMode);
    return models[renderMode];
}

GLenum GLUtil::toBufferType(Buffer::Type type)
{
    static const GLenum types[Buffer::TYPE_COUNT] = {GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER, GL_DRAW_INDIRECT_BUFFER, GL_SHADER_STORAGE_BUFFER};
    DCHECK(type >= 0 && type < Buffer::TYPE_COUNT, "Unknown buffer type: %d", type);
    return types[type];
}

GLenum GLUtil::toShaderType(PipelineInput::ShaderStage stage)
{
#ifndef ANDROID
    static const GLenum types[PipelineInput::SHADER_STAGE_COUNT] = {GL_VERTEX_SHADER, GL_TESS_CONTROL_SHADER, GL_TESS_EVALUATION_SHADER, GL_GEOMETRY_SHADER, GL_FRAGMENT_SHADER, GL_COMPUTE_SHADER};
#else
    static const GLenum types[PipelineInput::SHADER_STAGE_COUNT] = {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER, GL_COMPUTE_SHADER};
#endif
    DASSERT(stage >= 0 && stage < PipelineInput::SHADER_STAGE_COUNT);
    return types[stage];
}

GLenum GLUtil::getEnum(const String& name)
{
    const Global<GLConstants> constants;
    const auto iter = constants->_enums.find(name);
    DCHECK(iter != constants->_enums.end(), "Bad GLenum name \"%s\"", name.c_str());
    return iter->second;
}

GLenum GLUtil::getEnum(const String& name, GLenum defValue)
{
    const Global<GLConstants> constants;
    const auto iter = constants->_enums.find(name);
    return iter != constants->_enums.end() ? iter->second : defValue;
}

GLenum GLUtil::getEnum(const document& manifest, const String& name)
{
    return getEnum(Documents::ensureAttribute(manifest, name));
}

GLenum GLUtil::getEnum(const document& manifest, const String& name, GLenum defValue)
{
    return getEnum(Documents::getAttribute(manifest, name), defValue);
}

GLenum GLUtil::getTextureInternalFormat(int32_t format, const Bitmap& bitmap)
{
    static const GLenum formats[] = {GL_R8, GL_R8_SNORM, GL_R16, GL_R16_SNORM, GL_R8, GL_R8, GL_R16F, GL_R16F,
                                     GL_RG8, GL_RG8_SNORM, GL_RG16, GL_RG16_SNORM, GL_RG16F, GL_RG16F, GL_RG16F, GL_RG16F,
                                     GL_RGB8, GL_RGB8_SNORM, GL_RGB16, GL_RGB16_SNORM, GL_RGB16F, GL_RGB16F, GL_RGB16F, GL_RGB16F,
                                     GL_RGBA8, GL_RGBA8_SNORM, GL_RGBA16, GL_RGBA16_SNORM, GL_RGBA16F, GL_RGBA16F, GL_RGBA16F, GL_RGBA16F};
    uint32_t signedOffset = (format & Texture::FORMAT_SIGNED) == Texture::FORMAT_SIGNED ? 1 : ((format & Texture::FORMAT_F16) == Texture::FORMAT_F16 ? 4 : 0);
    uint32_t byteCount = bitmap.rowBytes() / bitmap.width() / bitmap.channels();
    uint32_t channel8 = (bitmap.channels() - 1) * 8;
    DCHECK(byteCount > 0 && byteCount <= 4, "Unsupported color depth: %d", byteCount * 8);
    return formats[channel8 + (byteCount - 1) * 2 + signedOffset];
}

GLenum GLUtil::getTextureFormat(int32_t format, uint8_t channels)
{
    const GLenum formatByChannels[] = {GL_RED, GL_RG, GL_RGB, GL_RGBA};
    DCHECK(channels < 5, "Unknown bitmap format: (channels = %d)", static_cast<uint32_t>(channels));
    return format == Texture::FORMAT_AUTO ? formatByChannels[channels - 1] : formatByChannels[static_cast<uint32_t>(format & Texture::FORMAT_RGBA)];
}

GLenum GLUtil::getPixelFormat(int32_t format, const Bitmap& bitmap)
{
    bool flagSigned = (format & Texture::FORMAT_SIGNED) == Texture::FORMAT_SIGNED;
    uint32_t byteCount = bitmap.rowBytes() / bitmap.width() / bitmap.channels();
    if(byteCount == 1)
        return flagSigned ? GL_BYTE : GL_UNSIGNED_BYTE;
    if(byteCount == 2)
    {
        if(format & Texture::FORMAT_F16)
            return GL_HALF_FLOAT;
        return flagSigned ? GL_SHORT: GL_UNSIGNED_SHORT;
    }
    if(format & Texture::FORMAT_F32)
        return GL_FLOAT;
    return flagSigned ? GL_INT : GL_UNSIGNED_INT;
}

void GLUtil::renderCubemap(GraphicsContext& graphicsContext, uint32_t id, RenderController& renderController, const sp<Pipeline>& pipeline, Texture& texture, int32_t width, int32_t height)
{
    uint32_t captureFBO, captureRBO;
    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

    for (uint32_t i = 0; i < 6; ++i)
        ::glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA8, width, height, 0, GL_RGB, GL_FLOAT, nullptr);

    const M4 captureProjection = MatrixUtil::perspective(Math::radians(90.0f), 1.0f, 0.1f, 10.0f);
    const M4 captureViews[] = {
        MatrixUtil::lookAt(V3(0.0f, 0.0f, 0.0f), V3( 1.0f,  0.0f,  0.0f), V3(0.0f, -1.0f,  0.0f)),
        MatrixUtil::lookAt(V3(0.0f, 0.0f, 0.0f), V3(-1.0f,  0.0f,  0.0f), V3(0.0f, -1.0f,  0.0f)),
        MatrixUtil::lookAt(V3(0.0f, 0.0f, 0.0f), V3( 0.0f,  1.0f,  0.0f), V3(0.0f,  0.0f,  1.0f)),
        MatrixUtil::lookAt(V3(0.0f, 0.0f, 0.0f), V3( 0.0f, -1.0f,  0.0f), V3(0.0f,  0.0f, -1.0f)),
        MatrixUtil::lookAt(V3(0.0f, 0.0f, 0.0f), V3( 0.0f,  0.0f,  1.0f), V3(0.0f, -1.0f,  0.0f)),
        MatrixUtil::lookAt(V3(0.0f, 0.0f, 0.0f), V3( 0.0f,  0.0f, -1.0f), V3(0.0f, -1.0f,  0.0f))
    };

    const sp<opengl::GLPipeline> glPipeline = pipeline;
    glUseProgram(static_cast<GLuint>(glPipeline->id()));
    glPipeline->getUniform("u_Projection").setUniformMatrix4fv(1, GL_FALSE, captureProjection.value());
    texture.upload(graphicsContext, nullptr);

    glPipeline->activeTexture(texture, 0);

    uint32_t vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    Buffer vertexBuffer = renderController.makeVertexBuffer(Buffer::USAGE_STATIC);
    Buffer indexBuffer = renderController.makeIndexBuffer(Buffer::USAGE_STATIC);

    const Buffer::Snapshot vertexBufferSnapshot = vertexBuffer.snapshot(sp<Uploader::Array<uint8_t>>::make(RenderUtil::makeUnitCubeVertices(false)));
    vertexBufferSnapshot.upload(graphicsContext);
    glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(vertexBuffer.id()));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12, nullptr);

    const Buffer::Snapshot indexBufferSnapshot = indexBuffer.snapshot(SharedBuffer::Quads::maker()(6));
    indexBufferSnapshot.upload(graphicsContext);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLuint>(indexBufferSnapshot.id()));

    glViewport(0, 0, width, height);
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    for (uint32_t i = 0; i < 6; ++i)
    {
        glPipeline->getUniform("u_View").setUniformMatrix4fv(1, GL_FALSE, reinterpret_cast<const GLfloat*>(&captureViews[i]));
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, id, 0);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawElements(GL_TRIANGLES, 36, GLIndexType, nullptr);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);

    glDeleteVertexArrays(1, &vao);
    glDeleteFramebuffers(1, &captureFBO);
    glDeleteRenderbuffers(1, &captureRBO);

    const RenderEngineContext::Resolution& resolution = graphicsContext.renderContext()->displayResolution();
    glViewport(0, 0, static_cast<GLsizei>(resolution.width), static_cast<GLsizei>(resolution.height));
}

void GLUtil::glTexImage2D(uint32_t index, int32_t n, void* data)
{
    ::glTexImage2D(static_cast<GLenum>(GL_TEXTURE_CUBE_MAP_POSITIVE_X + index), 0, (GLint) GL_RGBA8, n, n, 0, GL_RGBA, GL_FLOAT, data);
}

}
