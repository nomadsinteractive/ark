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
#include "renderer/base/texture_bundle.h"
#include "renderer/base/graphics_context.h"
#include "renderer/base/resource_loader_context.h"

namespace ark {

Texture::Texture(const sp<Size>& size, const sp<Resource>& resource, Type type)
    : _size(size), _resource(resource), _type(type)
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

Texture::Type Texture::type() const
{
    return _type;
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
}

void Texture::Parameters::setTexParameter(uint32_t name, int32_t value)
{
    _tex_parameters[name] = value;
}

Texture::DICTIONARY::DICTIONARY(BeanFactory& /*factory*/, const String& value, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_loader_context(resourceLoaderContext), _src(value)
{
}

sp<Texture> Texture::DICTIONARY::build(const sp<Scope>& /*args*/)
{
    return _resource_loader_context->textureLoader()->get(_src);
}


Texture::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_loader_context(resourceLoaderContext), _factory(factory), _manifest(manifest), _src(factory.getBuilder<String>(manifest, Constants::Attributes::SRC))
{
}

sp<Texture> Texture::BUILDER::build(const sp<Scope>& args)
{
    const sp<String> src = _src->build(args);
    if(src)
       return _resource_loader_context->textureLoader()->get(*src);

    const sp<Size> size = _factory.ensureConcreteClassBuilder<Size>(_manifest, Constants::Attributes::SIZE)->build(args);
    const sp<Recycler> recycler = _resource_loader_context->resourceManager()->recycler();
    return _resource_loader_context->renderController()->createTexture(static_cast<uint32_t>(size->width()), static_cast<uint32_t>(size->height()), nullptr);
}

}
