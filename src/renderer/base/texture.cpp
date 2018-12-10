#include "renderer/base/texture.h"

#include "core/inf/array.h"
#include "core/inf/variable.h"
#include "core/util/conversions.h"
#include "core/util/documents.h"
#include "core/util/log.h"

#include "graphics/base/bitmap.h"
#include "graphics/base/size.h"

#include "renderer/base/recycler.h"
#include "renderer/base/resource_manager.h"
#include "renderer/base/gl_texture_loader.h"
#include "renderer/base/graphics_context.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/opengl/util/gl_util.h"

namespace ark {

Texture::Texture(const sp<Size>& size, const sp<Resource>& resource)
    : _size(size), _resource(resource)
{
}

Texture::~Texture()
{
}

void Texture::upload(GraphicsContext& graphicsContext)
{
    _resource->upload(graphicsContext);
}

Resource::RecycleFunc Texture::recycle()
{
    return _resource->recycle();
}

uint32_t Texture::id()
{
    return _resource->id();
}

int32_t Texture::width() const
{
    return static_cast<int32_t>(_size->width());
}

int32_t Texture::height() const
{
    return static_cast<int32_t>(_size->height());
}

int32_t Texture::depth() const
{
    return static_cast<int32_t>(_size->depth());
}

const sp<Size>& Texture::size() const
{
    return _size;
}

const sp<Resource>& Texture::resource() const
{
    return _resource;
}

template<> ARK_API Texture::Format Conversions::to<String, Texture::Format>(const String& str)
{
    if(str)
    {
        Texture::Format format = Texture::FORMAT_R;
        for(const String& i : str.toLower().split('|'))
        {
            if(i == "r")
                format = static_cast<Texture::Format>(format | Texture::FORMAT_R);
            else if(i == "rg")
                format = static_cast<Texture::Format>(format | Texture::FORMAT_RG);
            else if(i == "rgb")
                format = static_cast<Texture::Format>(format | Texture::FORMAT_RGB);
            else if(i == "rgba")
                format = static_cast<Texture::Format>(format | Texture::FORMAT_RGBA);
            else if(i == "signed")
                format = static_cast<Texture::Format>(format | Texture::FORMAT_SIGNED);
            else
                DFATAL("Unknow texture format: %s", i.c_str());
        }
        return format;
    }
    return Texture::FORMAT_AUTO;
}

template<> ARK_API Texture::Feature Conversions::to<String, Texture::Feature>(const String& str)
{
    if(str)
    {
        Texture::Feature feature = Texture::FEATURE_DEFAULT;
        for(const String& i : str.toLower().split('|'))
        {
            if(i == "mipmaps")
                feature = static_cast<Texture::Feature>(feature | Texture::FEATURE_MIPMAPS);
            else
                DFATAL("Unknow texture feature: %s", i.c_str());
        }
        return feature;
    }
    return Texture::FEATURE_DEFAULT;
}

Texture::Parameters::Parameters(Format format, Texture::Feature features)
    : _format(format), _features(features)
{
    setTexParameter(static_cast<uint32_t>(GL_TEXTURE_MIN_FILTER), static_cast<int32_t>((features & Texture::FEATURE_MIPMAPS) ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR));
    setTexParameter(static_cast<uint32_t>(GL_TEXTURE_MAG_FILTER), static_cast<int32_t>(GL_LINEAR));
    setTexParameter(static_cast<uint32_t>(GL_TEXTURE_WRAP_S), static_cast<int32_t>(GL_CLAMP_TO_EDGE));
    setTexParameter(static_cast<uint32_t>(GL_TEXTURE_WRAP_T), static_cast<int32_t>(GL_CLAMP_TO_EDGE));
    setTexParameter(static_cast<uint32_t>(GL_TEXTURE_WRAP_R), static_cast<int32_t>(GL_CLAMP_TO_EDGE));
}

Texture::Parameters::Parameters(const document& manifest)
    : Parameters(Documents::getAttribute<Texture::Format>(manifest, "format", FORMAT_AUTO), Documents::getAttribute<Texture::Feature>(manifest, "feature", FEATURE_DEFAULT))
{
    for(const document& i : manifest->children("parameter"))
        _tex_parameters[static_cast<uint32_t>(GLUtil::getEnum(Documents::ensureAttribute(i, Constants::Attributes::NAME)))] = static_cast<int32_t>(GLUtil::getEnum(Documents::ensureAttribute(i, Constants::Attributes::VALUE)));
}

void Texture::Parameters::setTexParameter(uint32_t name, int32_t value)
{
    _tex_parameters[name] = value;
}

}
