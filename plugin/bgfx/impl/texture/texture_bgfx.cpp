#include "bgfx/impl/texture/texture_bgfx.h"

#include "core/inf/array.h"

#include "graphics/base/bitmap.h"

#include "renderer/util/render_util.h"

namespace ark::plugin::bgfx {

namespace {

::bgfx::TextureFormat::Enum getTextureInternalFormat(Texture::Usage usage, Texture::Format format, uint32_t channelSize, uint32_t componentSize)
{
    struct ComponentFormat {
        ::bgfx::TextureFormat::Enum bit8;
        ::bgfx::TextureFormat::Enum bit16;
        ::bgfx::TextureFormat::Enum bit32;
    };

    constexpr ComponentFormat uFormats[] = {
        {::bgfx::TextureFormat::R8U, ::bgfx::TextureFormat::R16U, ::bgfx::TextureFormat::R32U},
        {::bgfx::TextureFormat::RG8U, ::bgfx::TextureFormat::RG16U, ::bgfx::TextureFormat::RG32U},
        {::bgfx::TextureFormat::RGB8U, ::bgfx::TextureFormat::Unknown, ::bgfx::TextureFormat::Unknown},
        {::bgfx::TextureFormat::RGBA8U, ::bgfx::TextureFormat::RGBA16U, ::bgfx::TextureFormat::RGBA32U}
    };
    constexpr ComponentFormat iFormats[] = {
        {::bgfx::TextureFormat::R8I, ::bgfx::TextureFormat::R16I, ::bgfx::TextureFormat::R32I},
        {::bgfx::TextureFormat::RG8I, ::bgfx::TextureFormat::RG16I, ::bgfx::TextureFormat::RG32I},
        {::bgfx::TextureFormat::RGB8I, ::bgfx::TextureFormat::Unknown, ::bgfx::TextureFormat::Unknown},
        {::bgfx::TextureFormat::RGBA8I, ::bgfx::TextureFormat::RGBA16I, ::bgfx::TextureFormat::RGBA32I}
    };
    constexpr ComponentFormat fFormats[] = {
        {::bgfx::TextureFormat::Unknown, ::bgfx::TextureFormat::R16F, ::bgfx::TextureFormat::R32F},
        {::bgfx::TextureFormat::Unknown, ::bgfx::TextureFormat::RG16F, ::bgfx::TextureFormat::RG32F},
        {::bgfx::TextureFormat::Unknown, ::bgfx::TextureFormat::Unknown, ::bgfx::TextureFormat::Unknown},
        {::bgfx::TextureFormat::Unknown, ::bgfx::TextureFormat::RGBA16F, ::bgfx::TextureFormat::RGBA32F}
    };
    constexpr ComponentFormat nFormats[] = {
        {::bgfx::TextureFormat::R8, ::bgfx::TextureFormat::R16, ::bgfx::TextureFormat::Unknown},
        {::bgfx::TextureFormat::RG8, ::bgfx::TextureFormat::RG16, ::bgfx::TextureFormat::Unknown},
        {::bgfx::TextureFormat::RGB8, ::bgfx::TextureFormat::Unknown, ::bgfx::TextureFormat::Unknown},
        {::bgfx::TextureFormat::RGBA8, ::bgfx::TextureFormat::RGBA16, ::bgfx::TextureFormat::Unknown}
    };

    switch(usage & Texture::USAGE_DEPTH_STENCIL_ATTACHMENT)
    {
    case Texture::USAGE_GENERAL: {
        const bool isSigned = format & Texture::FORMAT_SIGNED;
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
        return ::bgfx::TextureFormat::D32F;
    case Texture::USAGE_STENCIL_ATTACHMENT:
    case Texture::USAGE_DEPTH_STENCIL_ATTACHMENT:
        return ::bgfx::TextureFormat::D24S8;
    default:
        break;
    }
    WARN("Unknow texture usage: %d", usage);
    return ::bgfx::TextureFormat::Unknown;
}

}

TextureBgfx::TextureBgfx(Texture::Type type, uint32_t width, uint32_t height, sp<Texture::Parameters> parameters)
    : ResourceBase(type), _width(width), _height(height), _parameters(std::move(parameters))
{
}

void TextureBgfx::upload(GraphicsContext& graphicsContext, const sp<Texture::Uploader>& uploader)
{
    if(uploader)
    {
        if(_handle)
            uploader->update(graphicsContext, *this);
        else
            uploader->initialize(graphicsContext, *this);
    }
}

void TextureBgfx::clear(GraphicsContext& graphicsContext)
{
    const uint32_t bitmapSize = RenderUtil::getChannelSize(_parameters->_format) * RenderUtil::getComponentSize(_parameters->_format) * _width * _height;
    const ::bgfx::Memory* mem = ::bgfx::alloc(bitmapSize);
    memset(mem->data, 0, bitmapSize);
    ::bgfx::updateTexture2D(_handle, 0, 0, 0, 0, _width, _height, mem);
}

bool TextureBgfx::download(GraphicsContext& graphicsContext, Bitmap& bitmap)
{
    FATAL("Unimplemented");
    return false;
}

void TextureBgfx::uploadBitmap(GraphicsContext& graphicsContext, const Bitmap& bitmap, const std::vector<sp<ByteArray>>& imagedata)
{
    if(!_handle)
    {
        const bool hasMips = _parameters->_features & Texture::FEATURE_MIPMAPS;
        const uint32_t channelSize = bitmap.channels();
        const uint32_t componentSize = RenderUtil::getComponentSize(_parameters->_format);
        _handle.reset(::bgfx::createTexture2D(_width, _height, hasMips, 1, getTextureInternalFormat(_parameters->_usage, _parameters->_format, channelSize, componentSize)));
    }
    const sp<ByteArray>& data = imagedata.at(0);
    ::bgfx::updateTexture2D(_handle, 0, 0, 0, 0, _width, _height, ::bgfx::copy(data->buf(), data->size()));
}

const sp<Texture::Parameters>& TextureBgfx::parameters() const
{
    return _parameters;
}

}
