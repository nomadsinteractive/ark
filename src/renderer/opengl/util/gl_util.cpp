#include "renderer/opengl/util/gl_util.h"

#include <unordered_map>

#include "core/types/global.h"
#include "core/impl/array/preallocated_array.h"
#include "core/util/math.h"

#include "graphics/base/bitmap.h"
#include "graphics/base/matrix.h"

#include "renderer/base/gl_buffer.h"
#include "renderer/opengl/base/gl_pipeline.h"
#include "renderer/base/gl_resource_manager.h"
#include "renderer/base/shader.h"
#include "renderer/base/texture.h"
#include "renderer/opengl/util/gl_index_buffers.h"

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
    }

    std::unordered_map<String, GLenum> _enums;
};

}

extern uint32_t g_GLViewportWidth;
extern uint32_t g_GLViewportHeight;

GLenum GLUtil::getEnum(const String &name)
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

GLenum GLUtil::getTextureInternalFormat(int32_t format, const Bitmap& bitmap)
{
    const GLenum formats[] = {GL_R8, GL_R8_SNORM, GL_R16, GL_R16_SNORM, GL_R8, GL_R8, GL_R16F, GL_R16F,
                              GL_RG8, GL_RG8_SNORM, GL_RG16, GL_RG16_SNORM, GL_RG16F, GL_RG16F, GL_RG16F, GL_RG16F,
                              GL_RGB8, GL_RGB8_SNORM, GL_RGB16, GL_RGB16_SNORM, GL_RGB16F, GL_RGB16F, GL_RGB16F, GL_RGB16F,
                              GL_RGBA8, GL_RGBA8_SNORM, GL_RGBA16, GL_RGBA16_SNORM, GL_RGBA16F, GL_RGBA16F, GL_RGBA16F, GL_RGBA16F};
    uint32_t signedOffset = (format & Texture::FORMAT_SIGNED) == Texture::FORMAT_SIGNED ? 1 : 0;
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
        return flagSigned ? GL_SHORT: GL_UNSIGNED_SHORT;
    return flagSigned ? GL_INT : GL_FLOAT;
}

bytearray GLUtil::makeUnitCubeVertices()
{
    static float vertices[] = {
        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,

        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f
    };
    return sp<PreallocatedArray<uint8_t>>::make(reinterpret_cast<uint8_t*>(vertices), sizeof(vertices));
}

void GLUtil::renderCubemap(GraphicsContext& graphicsContext, uint32_t id, GLResourceManager& resourceManager, Shader& shader, Texture& texture, int32_t width, int32_t height)
{
    uint32_t captureFBO, captureRBO;
    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

    for (uint32_t i = 0; i < 6; ++i)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);

    const Matrix captureProjection = Matrix::perspective(Math::radians(90.0f), 1.0f, 0.1f, 10.0f);
    const Matrix captureViews[] = {
        Matrix::lookAt(V3(0.0f, 0.0f, 0.0f), V3( 1.0f,  0.0f,  0.0f), V3(0.0f, -1.0f,  0.0f)),
        Matrix::lookAt(V3(0.0f, 0.0f, 0.0f), V3(-1.0f,  0.0f,  0.0f), V3(0.0f, -1.0f,  0.0f)),
        Matrix::lookAt(V3(0.0f, 0.0f, 0.0f), V3( 0.0f,  1.0f,  0.0f), V3(0.0f,  0.0f,  1.0f)),
        Matrix::lookAt(V3(0.0f, 0.0f, 0.0f), V3( 0.0f, -1.0f,  0.0f), V3(0.0f,  0.0f, -1.0f)),
        Matrix::lookAt(V3(0.0f, 0.0f, 0.0f), V3( 0.0f,  0.0f,  1.0f), V3(0.0f, -1.0f,  0.0f)),
        Matrix::lookAt(V3(0.0f, 0.0f, 0.0f), V3( 0.0f,  0.0f, -1.0f), V3(0.0f, -1.0f,  0.0f))
    };

    shader.use(graphicsContext);

    const sp<GLPipeline> glPipeline = shader.pipeline();
    glPipeline->getUniform("u_Projection").setUniformMatrix4fv(1, GL_FALSE, captureProjection.value());
    texture.prepare(graphicsContext);
    texture.active(shader.pipeline(), 0);

    uint32_t vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLBuffer arrayBuffer(resourceManager.recycler(), sp<GLBuffer::ArrayUploader<uint8_t>>::make(GLUtil::makeUnitCubeVertices()), GL_ARRAY_BUFFER, GL_STATIC_DRAW);
    arrayBuffer.prepare(graphicsContext);
    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffer.id());
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12, 0);

    const GLBuffer::Snapshot indexBuffer = GLIndexBuffers::makeGLBufferSnapshot(resourceManager, GLBuffer::NAME_QUADS, 6);
    indexBuffer.prepare(graphicsContext);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer.id());

    glViewport(0, 0, width, height);
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    for (uint32_t i = 0; i < 6; ++i)
    {
        glPipeline->getUniform("u_View").setUniformMatrix4fv(1, GL_FALSE, captureViews[i].value());
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, id, 0);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawElements(GL_TRIANGLES, 36, GLIndexType, 0);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);

    glDeleteVertexArrays(1, &vao);
    glDeleteFramebuffers(1, &captureFBO);
    glDeleteRenderbuffers(1, &captureRBO);

    glViewport(0, 0, g_GLViewportWidth, g_GLViewportHeight);
}

}
