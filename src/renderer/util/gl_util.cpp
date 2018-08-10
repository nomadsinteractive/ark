#include "renderer/util/gl_util.h"

#include <unordered_map>

#include "core/types/global.h"

#include "graphics/base/bitmap.h"

#include "renderer/base/gl_texture.h"

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
    uint32_t signedOffset = (format & GLTexture::FORMAT_SIGNED) == GLTexture::FORMAT_SIGNED ? 1 : 0;
    uint32_t byteCount = bitmap.rowBytes() / bitmap.width() / bitmap.channels();
    uint32_t channel8 = (bitmap.channels() - 1) * 8;
    DCHECK(byteCount > 0 && byteCount <= 4, "Unsupported color depth: %d", byteCount * 8);
    return formats[channel8 + (byteCount - 1) * 2 + signedOffset];
}

GLenum GLUtil::getTextureFormat(int32_t format, uint8_t channels)
{
    const GLenum formatByChannels[] = {GL_RED, GL_RG, GL_RGB, GL_RGBA};
    DCHECK(channels < 5, "Unknown bitmap format: (channels = %d)", static_cast<uint32_t>(channels));
    return format == GLTexture::FORMAT_AUTO ? formatByChannels[channels - 1] : formatByChannels[static_cast<uint32_t>(format & GLTexture::FORMAT_RGBA)];
}

GLenum GLUtil::getPixelFormat(int32_t format, const Bitmap& bitmap)
{
    bool flagSigned = (format & GLTexture::FORMAT_SIGNED) == GLTexture::FORMAT_SIGNED;
    uint32_t byteCount = bitmap.rowBytes() / bitmap.width() / bitmap.channels();
    if(byteCount == 1)
        return flagSigned ? GL_BYTE : GL_UNSIGNED_BYTE;
    if(byteCount == 2)
        return flagSigned ? GL_SHORT: GL_UNSIGNED_SHORT;
    return flagSigned ? GL_INT : GL_FLOAT;
}

}
