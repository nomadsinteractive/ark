#include "renderer/opengl/util/gl_util.h"

#include <unordered_map>

#include "core/base/enums.h"
#include "core/types/global.h"
#include "core/util/uploader_type.h"
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
#include "renderer/base/shared_indices.h"
#include "renderer/base/texture.h"
#include "renderer/util/render_util.h"

#include "renderer/opengl/base/gl_buffer.h"
#include "renderer/opengl/base/gl_pipeline.h"
#include "renderer/opengl/base/gl_texture.h"
#include "renderer/opengl/util/gl_debug.h"

namespace ark {

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

GLenum GLUtil::toCompareFunc(PipelineBindings::CompareFunc func)
{
    const GLenum glFuncs[PipelineBindings::COMPARE_FUNC_LENGTH] = {GL_ZERO, GL_ALWAYS, GL_NEVER, GL_EQUAL, GL_NOTEQUAL, GL_LESS, GL_GREATER, GL_LEQUAL, GL_GEQUAL};
    DCHECK(func < PipelineBindings::COMPARE_FUNC_LENGTH, "Unknow compare func: %d", func);
    return glFuncs[func];
}

GLenum GLUtil::toStencilFunc(PipelineBindings::StencilFunc func)
{
    const GLenum glFuncs[PipelineBindings::STENCIL_FUNC_LENGTH] = {GL_KEEP, GL_ZERO, GL_REPLACE, GL_INCR, GL_INCR_WRAP, GL_DECR, GL_DECR_WRAP, GL_INVERT};
    DCHECK(func < PipelineBindings::STENCIL_FUNC_LENGTH, "Unknow stencil func: %d", func);
    return glFuncs[func];
}

GLenum GLUtil::toBlendFactor(PipelineBindings::BlendFactor blendFactor)
{
    switch(blendFactor) {
    case PipelineBindings::BLEND_FACTOR_DEFAULT:
        return std::numeric_limits<GLenum>::max();
    case PipelineBindings::BLEND_FACTOR_ZERO:
        return GL_ZERO;
    case PipelineBindings::BLEND_FACTOR_ONE:
        return GL_ONE;
    case PipelineBindings::BLEND_FACTOR_SRC_COLOR:
        return GL_SRC_COLOR;
    case PipelineBindings::BLEND_FACTOR_ONE_MINUS_SRC_COLOR:
        return GL_ONE_MINUS_SRC_COLOR;
    case PipelineBindings::BLEND_FACTOR_DST_COLOR:
        return GL_DST_COLOR;
    case PipelineBindings::BLEND_FACTOR_ONE_MINUS_DST_COLOR:
        return GL_ONE_MINUS_DST_COLOR;
    case PipelineBindings::BLEND_FACTOR_SRC_ALPHA:
        return GL_SRC_ALPHA;
    case PipelineBindings::BLEND_FACTOR_ONE_MINUS_SRC_ALPHA:
        return GL_ONE_MINUS_SRC_ALPHA;
    case PipelineBindings::BLEND_FACTOR_DST_ALPHA:
        return GL_DST_ALPHA;
    case PipelineBindings::BLEND_FACTOR_ONE_MINUS_DST_ALPHA:
        return GL_ONE_MINUS_DST_ALPHA;
    case PipelineBindings::BLEND_FACTOR_CONST_COLOR:
        return GL_CONSTANT_COLOR;
    case PipelineBindings::BLEND_FACTOR_CONST_ALPHA:
        return GL_CONSTANT_ALPHA;
    default:
        break;
    }
    FATAL("Unknow BlendFacor: %d", blendFactor);
    return GL_ZERO;
}

GLenum GLUtil::toFrontFaceType(PipelineBindings::FrontFaceType face)
{
    const GLenum glFaceTypes[PipelineBindings::FRONT_FACE_TYPE_LENGTH] = {GL_FRONT_AND_BACK, GL_FRONT, GL_BACK};
    DCHECK(face < PipelineBindings::FRONT_FACE_TYPE_LENGTH, "Unknow front face type: %d", face);
    return glFaceTypes[face];
}

GLenum GLUtil::getEnum(const String& name)
{
    return Enums<GLenum>::instance().ensureEnum(name);
}

GLenum GLUtil::getEnum(const String& name, GLenum defValue)
{
    return Enums<GLenum>::instance().toEnumOrDefault(name, defValue);
}

GLenum GLUtil::getEnum(const document& manifest, const String& name)
{
    return getEnum(Documents::ensureAttribute(manifest, name));
}

GLenum GLUtil::getEnum(const document& manifest, const String& name, GLenum defValue)
{
    return getEnum(Documents::getAttribute(manifest, name), defValue);
}

GLenum GLUtil::getTextureInternalFormat(Texture::Usage usage, Texture::Format format, const Bitmap& bitmap)
{
    return getTextureInternalFormat(usage, format, bitmap.channels(), bitmap.rowBytes() / bitmap.width() / bitmap.channels());
}

GLenum GLUtil::getTextureInternalFormat(Texture::Usage usage, Texture::Format format, uint32_t channelSize, uint32_t componentSize)
{
    struct GLComponentFormat {
        GLenum bit8;
        GLenum bit16;
        GLenum bit32;
    };

    static const GLComponentFormat uFormats[] = {
        {GL_R8UI, GL_R16UI, GL_R32UI},
        {GL_RG8UI, GL_RG16UI, GL_RG32UI},
        {GL_RGB8UI, GL_RGB16UI, GL_RGB32UI},
        {GL_RGBA8UI, GL_RGBA16UI, GL_RGBA32UI}
    };
    static const GLComponentFormat iFormats[] = {
        {GL_R8I, GL_R16I, GL_R32I},
        {GL_RG8I, GL_RG16I, GL_RG32I},
        {GL_RGB8I, GL_RGB16I, GL_RGB32I},
        {GL_RGBA8I, GL_RGBA16I, GL_RGBA32I}
    };
    static const GLComponentFormat fFormats[] = {
        {GL_ZERO, GL_R16F, GL_R32F},
        {GL_ZERO, GL_RG16F, GL_RG32F},
        {GL_ZERO, GL_RGB16F, GL_RGB32F},
        {GL_ZERO, GL_RGBA16F, GL_RGBA32F}
    };
    static const GLComponentFormat nFormats[] = {
        {GL_R8_SNORM, GL_R16_SNORM, GL_ZERO},
        {GL_RG8_SNORM, GL_RG16_SNORM, GL_ZERO},
        {GL_RGB8_SNORM, GL_RGB16_SNORM, GL_ZERO},
        {GL_RGBA8_SNORM, GL_RGBA16_SNORM, GL_ZERO}
    };

    switch(usage & Texture::USAGE_DEPTH_STENCIL_ATTACHMENT)
    {
    case Texture::USAGE_COLOR_ATTACHMENT: {
        bool isSigned = format & Texture::FORMAT_SIGNED;
        uint32_t cs = channelSize - 1;

        CHECK(componentSize == 1 || componentSize == 2 || componentSize == 4, "Illegal color component size %d", componentSize);
        if(format & Texture::FORMAT_AUTO)
        {
            if(componentSize == 1)
                return nFormats[cs].bit8;
            if(componentSize == 2)
                return fFormats[cs].bit16;
            return fFormats[cs].bit32;
        }

        if(format & Texture::FORMAT_INTEGER)
        {
            if(componentSize == 1)
                return isSigned ? iFormats[cs].bit8 : uFormats[cs].bit8;
            if(componentSize == 2)
                return isSigned ? iFormats[cs].bit16 : uFormats[cs].bit16;
            return isSigned ? iFormats[cs].bit32 : uFormats[cs].bit32;
        }

        if(format & Texture::FORMAT_FLOAT)
        {
            if(componentSize == 2)
                return fFormats[cs].bit16;
            return fFormats[cs].bit32;
        }

        if(componentSize == 1)
            return nFormats[cs].bit8;
        ASSERT(componentSize == 2);
        return nFormats[cs].bit16;
    }
    case Texture::USAGE_DEPTH_ATTACHMENT:
        return GL_DEPTH_COMPONENT;
    case Texture::USAGE_STENCIL_ATTACHMENT:
    case Texture::USAGE_DEPTH_STENCIL_ATTACHMENT:
        return GL_DEPTH_STENCIL;
    }
    WARN("Unknow texture usage: %d", usage);
    return GL_R8;
}

GLenum GLUtil::getTextureFormat(Texture::Usage usage, Texture::Format format, uint8_t channels)
{
    switch(usage & Texture::USAGE_DEPTH_STENCIL_ATTACHMENT)
    {
    case Texture::USAGE_COLOR_ATTACHMENT: {
        bool isInteger = format & Texture::FORMAT_INTEGER;
        const static GLenum fChannels[] = {GL_RED, GL_RG, GL_RGB, GL_RGBA};
        const static GLenum iChannels[] = {GL_RED_INTEGER, GL_RG_INTEGER, GL_RGB_INTEGER, GL_RGBA_INTEGER};
        const GLenum* formatByChannels = isInteger ? iChannels : fChannels;
        DCHECK(channels < 5, "Unknown bitmap format: (channels = %d)", static_cast<uint32_t>(channels));
        return format == Texture::FORMAT_AUTO ? formatByChannels[channels - 1] : formatByChannels[static_cast<uint32_t>(format & Texture::FORMAT_RGBA)];
    }
    case Texture::USAGE_DEPTH_ATTACHMENT:
        return GL_DEPTH_COMPONENT;
    case Texture::USAGE_STENCIL_ATTACHMENT:
    case Texture::USAGE_DEPTH_STENCIL_ATTACHMENT:
        return GL_DEPTH_STENCIL;
    }
    return GL_RGBA;
}

GLenum GLUtil::getPixelType(int32_t format, const Bitmap& bitmap)
{
    bool flagSigned = (format & Texture::FORMAT_SIGNED) == Texture::FORMAT_SIGNED;
    uint32_t componentSize = bitmap.rowBytes() / bitmap.width() / bitmap.channels();
    if(componentSize == 1)
        return flagSigned ? GL_BYTE : GL_UNSIGNED_BYTE;
    if(componentSize == 2)
    {
        if(format == Texture::FORMAT_AUTO || format & Texture::FORMAT_FLOAT)
            return GL_HALF_FLOAT;
        return flagSigned ? GL_SHORT: GL_UNSIGNED_SHORT;
    }
    DCHECK(componentSize == 4, "Unsupported component size: %d, only [1, 2, 4] are allowed here", componentSize);
    if(format == Texture::FORMAT_AUTO || format & Texture::FORMAT_FLOAT)
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
    texture.upload(graphicsContext);

    glPipeline->activeTexture(texture, "u_Texture0", 0);

    uint32_t vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    Buffer vertexBuffer = renderController.makeVertexBuffer(Buffer::USAGE_STATIC);
    Buffer::Snapshot indexBufferSnapshot = renderController.getSharedIndices(RenderController::SHARED_INDICES_QUAD)->snapshot(renderController, 6, 6);

    const Buffer::Snapshot vertexBufferSnapshot = vertexBuffer.snapshot(UploaderType::create(RenderUtil::makeUnitCubeVertices(false)));
    vertexBufferSnapshot.upload(graphicsContext);
    glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(vertexBuffer.id()));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12, nullptr);

    indexBufferSnapshot.upload(graphicsContext);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLuint>(indexBufferSnapshot.id()));

    glViewport(0, 0, width, height);
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    for(uint32_t i = 0; i < 6; ++i)
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
    ::glTexImage2D(static_cast<GLenum>(GL_TEXTURE_CUBE_MAP_POSITIVE_X + index), 0, static_cast<GLint>(GL_RGBA8), n, n, 0, GL_RGBA, GL_FLOAT, data);
}

template<> ARK_API void Enums<GLenum>::initialize(std::map<String, GLenum>& enums)
{
    enums["nearest"] = GL_NEAREST;
    enums["linear"] = GL_LINEAR;
    enums["texture_mag_filter"] = GL_TEXTURE_MAG_FILTER;
    enums["texture_min_filter"] = GL_TEXTURE_MIN_FILTER;
    enums["texture_wrap_s"] = GL_TEXTURE_WRAP_S;
    enums["texture_wrap_t"] = GL_TEXTURE_WRAP_T;
    enums["texture_wrap_r"] = GL_TEXTURE_WRAP_R;
    enums["clamp_to_edge"] = GL_CLAMP_TO_EDGE;
    enums["clamp_to_border"] = GL_CLAMP_TO_BORDER;
    enums["mirrored_repeat"] = GL_MIRRORED_REPEAT;
    enums["repeat"] = GL_REPEAT;
    enums["mirror_clamp_to_edge"] = GL_MIRROR_CLAMP_TO_EDGE;

    enums["rgba"] = GL_RGBA;
    enums["rgb"] = GL_RGB;
    enums["alpha"] = GL_ALPHA;
    enums["rg"] = GL_RG;

    enums["always"] = GL_ALWAYS;
    enums["never"] = GL_NEVER;
    enums["equal"] = GL_EQUAL;
    enums["not_equal"] = GL_NOTEQUAL;
    enums["less"] = GL_LESS;
    enums["greater"] = GL_GREATER;
    enums["less_equal"] = GL_LEQUAL;
    enums["greater_equal"] = GL_GEQUAL;

    enums["keep"] = GL_KEEP;
    enums["zero"] = GL_ZERO;
    enums["replace"] = GL_REPLACE;
    enums["incr"] = GL_INCR;
    enums["decr"] = GL_DECR;

    enums["cw"] = GL_CW;
    enums["ccw"] = GL_CCW;

    enums["front"] = GL_FRONT;
    enums["back"] = GL_BACK;
    enums["front_and_back"] = GL_FRONT_AND_BACK;
}

GLBufferBinder::GLBufferBinder(GLenum target, GLuint buffer)
    : _target(target), _buffer(buffer)
{
    glBindBuffer(_target, _buffer);
}

GLBufferBinder::GLBufferBinder(GLBufferBinder&& other)
    : _target(other._target), _buffer(other._buffer)
{
    other._buffer = 0;
}

GLBufferBinder::~GLBufferBinder()
{
    if(_buffer)
        glBindBuffer(_target, 0);
}

}
