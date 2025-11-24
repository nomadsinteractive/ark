#include "opengl/util/gl_util.h"

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
#include "renderer/base/texture.h"
#include "renderer/util/render_util.h"

#include "opengl/base/gl_buffer.h"
#include "opengl/base/gl_pipeline.h"
#include "opengl/base/gl_texture.h"
#include "opengl/util/gl_debug.h"

namespace ark::plugin::opengl {

GLenum GLUtil::toEnum(const enums::DrawMode renderMode)
{
    constexpr GLenum models[enums::DRAW_MODE_COUNT] = {GL_LINES, GL_POINTS, GL_TRIANGLES, GL_TRIANGLE_STRIP};
    DCHECK(renderMode >= 0 && renderMode < enums::DRAW_MODE_COUNT, "Unknown Mode: %d", renderMode);
    return models[renderMode];
}

GLenum GLUtil::toShaderType(const enums::ShaderStageBit stage)
{
#ifndef ANDROID
    constexpr GLenum types[enums::SHADER_STAGE_BIT_COUNT] = {GL_VERTEX_SHADER, GL_TESS_CONTROL_SHADER, GL_TESS_EVALUATION_SHADER, GL_GEOMETRY_SHADER, GL_FRAGMENT_SHADER, GL_COMPUTE_SHADER};
#else
    constexpr GLenum types[Enum::SHADER_STAGE_COUNT] = {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER, GL_COMPUTE_SHADER};
#endif
    DASSERT(stage >= 0 && stage < enums::SHADER_STAGE_BIT_COUNT);
    return types[stage];
}

GLenum GLUtil::toCompareFunc(const PipelineDescriptor::CompareFunc func)
{
    constexpr GLenum glFuncs[PipelineDescriptor::COMPARE_FUNC_LENGTH] = {GL_ZERO, GL_ALWAYS, GL_NEVER, GL_EQUAL, GL_NOTEQUAL, GL_LESS, GL_GREATER, GL_LEQUAL, GL_GEQUAL};
    DCHECK(func < PipelineDescriptor::COMPARE_FUNC_LENGTH, "Unknow compare func: %d", func);
    return glFuncs[func];
}

GLenum GLUtil::toStencilFunc(PipelineDescriptor::StencilFunc func)
{
    constexpr GLenum glFuncs[PipelineDescriptor::STENCIL_FUNC_LENGTH] = {GL_KEEP, GL_ZERO, GL_REPLACE, GL_INCR, GL_INCR_WRAP, GL_DECR, GL_DECR_WRAP, GL_INVERT};
    DCHECK(func < PipelineDescriptor::STENCIL_FUNC_LENGTH, "Unknow stencil func: %d", func);
    return glFuncs[func];
}

Optional<GLenum> GLUtil::toBlendFactor(PipelineDescriptor::BlendFactor blendFactor)
{
    switch(blendFactor) {
    case PipelineDescriptor::BLEND_FACTOR_DEFAULT:
        return {};
    case PipelineDescriptor::BLEND_FACTOR_ZERO:
        return GL_ZERO;
    case PipelineDescriptor::BLEND_FACTOR_ONE:
        return GL_ONE;
    case PipelineDescriptor::BLEND_FACTOR_SRC_COLOR:
        return GL_SRC_COLOR;
    case PipelineDescriptor::BLEND_FACTOR_ONE_MINUS_SRC_COLOR:
        return GL_ONE_MINUS_SRC_COLOR;
    case PipelineDescriptor::BLEND_FACTOR_DST_COLOR:
        return GL_DST_COLOR;
    case PipelineDescriptor::BLEND_FACTOR_ONE_MINUS_DST_COLOR:
        return GL_ONE_MINUS_DST_COLOR;
    case PipelineDescriptor::BLEND_FACTOR_SRC_ALPHA:
        return GL_SRC_ALPHA;
    case PipelineDescriptor::BLEND_FACTOR_ONE_MINUS_SRC_ALPHA:
        return GL_ONE_MINUS_SRC_ALPHA;
    case PipelineDescriptor::BLEND_FACTOR_DST_ALPHA:
        return GL_DST_ALPHA;
    case PipelineDescriptor::BLEND_FACTOR_ONE_MINUS_DST_ALPHA:
        return GL_ONE_MINUS_DST_ALPHA;
    case PipelineDescriptor::BLEND_FACTOR_CONST_COLOR:
        return GL_CONSTANT_COLOR;
    case PipelineDescriptor::BLEND_FACTOR_CONST_ALPHA:
        return GL_CONSTANT_ALPHA;
    default:
        break;
    }
    FATAL("Unknow BlendFacor: %d", blendFactor);
    return {};
}

GLenum GLUtil::toFrontFaceType(PipelineDescriptor::FrontFaceType face)
{
    constexpr GLenum glFaceTypes[PipelineDescriptor::FRONT_FACE_TYPE_LENGTH] = {GL_FRONT_AND_BACK, GL_FRONT, GL_BACK};
    DCHECK(face < PipelineDescriptor::FRONT_FACE_TYPE_LENGTH, "Unknow front face type: %d", face);
    return glFaceTypes[face];
}

constexpr enums::LookupTable<GLenum, 34> glEnumTable = {{
    {"nearest", GL_NEAREST},
    {"linear", GL_LINEAR},
    {"texture_mag_filter", GL_TEXTURE_MAG_FILTER},
    {"texture_min_filter", GL_TEXTURE_MIN_FILTER},
    {"texture_wrap_s", GL_TEXTURE_WRAP_S},
    {"texture_wrap_t", GL_TEXTURE_WRAP_T},
    {"texture_wrap_r", GL_TEXTURE_WRAP_R},
    {"clamp_to_edge", GL_CLAMP_TO_EDGE},
    {"clamp_to_border", GL_CLAMP_TO_BORDER},
    {"mirrored_repeat", GL_MIRRORED_REPEAT},
    {"repeat", GL_REPEAT},
    {"mirror_clamp_to_edge", GL_MIRROR_CLAMP_TO_EDGE},
    {"rgba", GL_RGBA},
    {"rgb", GL_RGB},
    {"alpha", GL_ALPHA},
    {"rg", GL_RG},
    {"always", GL_ALWAYS},
    {"never", GL_NEVER},
    {"equal", GL_EQUAL},
    {"not_equal", GL_NOTEQUAL},
    {"less", GL_LESS},
    {"greater", GL_GREATER},
    {"less_equal", GL_LEQUAL},
    {"greater_equal", GL_GEQUAL},
    {"keep", GL_KEEP},
    {"zero", GL_ZERO},
    {"replace", GL_REPLACE},
    {"incr", GL_INCR},
    {"decr", GL_DECR},
    {"cw", GL_CW},
    {"ccw", GL_CCW},
    {"front", GL_FRONT},
    {"back", GL_BACK},
    {"front_and_back", GL_FRONT_AND_BACK}
}};

GLenum GLUtil::getEnum(const String& name)
{
    return enums::lookup(glEnumTable, name);
}

GLenum GLUtil::getEnum(const String& name, const GLenum defValue)
{
    return enums::lookup(glEnumTable, name, defValue);
}

GLenum GLUtil::getEnum(const document& manifest, const String& name)
{
    return getEnum(Documents::ensureAttribute(manifest, name));
}

GLenum GLUtil::getEnum(const document& manifest, const String& name, GLenum defValue)
{
    return getEnum(Documents::getAttribute(manifest, name), defValue);
}

GLenum GLUtil::getTextureInternalFormat(const Texture::Usage usage, const Texture::Format format, const Bitmap& bitmap)
{
    return getTextureInternalFormat(usage, format, bitmap.channels(), bitmap.rowBytes() / bitmap.width() / bitmap.channels());
}

GLenum GLUtil::getTextureInternalFormat(const Texture::Usage usage, const Texture::Format format, const uint32_t channelSize, const uint32_t componentSize)
{
    struct GLComponentFormat {
        GLenum bit8;
        GLenum bit16;
        GLenum bit32;
    };

    constexpr GLComponentFormat uFormats[] = {
        {GL_R8UI, GL_R16UI, GL_R32UI},
        {GL_RG8UI, GL_RG16UI, GL_RG32UI},
        {GL_RGB8UI, GL_RGB16UI, GL_RGB32UI},
        {GL_RGBA8UI, GL_RGBA16UI, GL_RGBA32UI}
    };
    constexpr GLComponentFormat iFormats[] = {
        {GL_R8I, GL_R16I, GL_R32I},
        {GL_RG8I, GL_RG16I, GL_RG32I},
        {GL_RGB8I, GL_RGB16I, GL_RGB32I},
        {GL_RGBA8I, GL_RGBA16I, GL_RGBA32I}
    };
    constexpr GLComponentFormat fFormats[] = {
        {GL_ZERO, GL_R16F, GL_R32F},
        {GL_ZERO, GL_RG16F, GL_RG32F},
        {GL_ZERO, GL_RGB16F, GL_RGB32F},
        {GL_ZERO, GL_RGBA16F, GL_RGBA32F}
    };
    constexpr GLComponentFormat nFormats[] = {
        {GL_R8_SNORM, GL_R16_SNORM, GL_ZERO},
        {GL_RG8_SNORM, GL_RG16_SNORM, GL_ZERO},
        {GL_RGB8_SNORM, GL_RGB16_SNORM, GL_ZERO},
        {GL_RGBA8_SNORM, GL_RGBA16_SNORM, GL_ZERO}
    };

    switch(usage.bits() & Texture::USAGE_DEPTH_STENCIL_ATTACHMENT)
    {
    case Texture::USAGE_AUTO: {
        const bool isSigned = format.contains(Texture::FORMAT_SIGNED);
        const uint32_t cs = channelSize - 1;

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
        if(format & Texture::FORMAT_FLOAT)
        {
            CHECK(componentSize == 4, "Component size must be 4 for float depth format", componentSize);
            return GL_DEPTH_COMPONENT32F;
        }
        if(componentSize == 4)
            return GL_DEPTH_COMPONENT32;
        if(componentSize == 2)
            return GL_DEPTH_COMPONENT16;
        return GL_DEPTH_COMPONENT;
    case Texture::USAGE_STENCIL_ATTACHMENT:
    case Texture::USAGE_DEPTH_STENCIL_ATTACHMENT:
        return GL_DEPTH_STENCIL;
    }
    WARN("Unknow texture usage: %d", usage);
    return GL_R8;
}

GLenum GLUtil::getTextureFormat(const Texture::Usage usage, const Texture::Format format, const uint8_t channels)
{
    switch(usage.bits() & Texture::USAGE_DEPTH_STENCIL_ATTACHMENT)
    {
        case Texture::USAGE_AUTO:
        {
            const bool isInteger = format.contains(Texture::FORMAT_INTEGER);
            constexpr GLenum fChannels[] = {GL_RED, GL_RG, GL_RGB, GL_RGBA};
            constexpr GLenum iChannels[] = {GL_RED_INTEGER, GL_RG_INTEGER, GL_RGB_INTEGER, GL_RGBA_INTEGER};
            const GLenum* formatByChannels = isInteger ? iChannels : fChannels;
            DCHECK(channels < 5, "Unknown bitmap format: (channels = %d)", static_cast<uint32_t>(channels));
            return format == Texture::FORMAT_AUTO ? formatByChannels[channels - 1] : formatByChannels[(format & Texture::FORMAT_RGBA).bits()];
        }
        case Texture::USAGE_DEPTH_ATTACHMENT:
            return GL_DEPTH_COMPONENT;
        case Texture::USAGE_STENCIL_ATTACHMENT:
        case Texture::USAGE_DEPTH_STENCIL_ATTACHMENT:
            return GL_DEPTH_STENCIL;
    }
    return GL_RGBA;
}

GLenum GLUtil::getPixelType(const Texture::Format format, const Bitmap& bitmap)
{
    const bool flagSigned = (format & Texture::FORMAT_SIGNED) == Texture::FORMAT_SIGNED;
    const uint32_t componentSize = bitmap.rowBytes() / bitmap.width() / bitmap.channels();
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

void GLUtil::glTexImage2D(const uint32_t index, const int32_t n, const void* data)
{
    ::glTexImage2D(static_cast<GLenum>(GL_TEXTURE_CUBE_MAP_POSITIVE_X + index), 0, static_cast<GLint>(GL_RGBA8), n, n, 0, GL_RGBA, GL_FLOAT, data);
}

GLBufferBinder::GLBufferBinder(const GLenum target, const GLuint buffer)
    : _target(target), _buffer(buffer)
{
    glBindBuffer(_target, buffer);
}

GLBufferBinder::~GLBufferBinder()
{
    if(_buffer)
        glBindBuffer(_target, 0);
}

}
