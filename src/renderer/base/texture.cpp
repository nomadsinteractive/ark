#include "renderer/base/texture.h"

#include "core/inf/array.h"
#include "core/inf/variable.h"
#include "core/impl/dictionary/dictionary_by_attribute_name.h"
#include "core/util/conversions.h"
#include "core/util/documents.h"
#include "core/util/log.h"

#include "graphics/base/bitmap.h"
#include "graphics/base/size.h"

#include "renderer/base/recycler.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/texture_bundle.h"
#include "renderer/base/graphics_context.h"
#include "renderer/base/resource_loader_context.h"

namespace ark {

namespace {

class ImageTextureUploader : public Texture::Uploader {
public:
    ImageTextureUploader(const sp<Variable<bitmap>>& bitmap)
        : _bitmap(bitmap) {
    }

    virtual void upload(GraphicsContext& graphicContext, Texture::Delegate& delegate) override {
        const sp<Bitmap> bitmap = _bitmap->val();
        delegate.uploadBitmap(graphicContext, 0, bitmap);
    }

private:
    sp<Variable<bitmap>> _bitmap;
};


}

Texture::Texture(const sp<Size>& size, const sp<Variable<sp<Delegate>>>& delegate, const sp<Parameters>& parameters)
    : _size(size), _delegate(delegate), _parameters(parameters)
{
}

Texture::~Texture()
{
}

void Texture::upload(GraphicsContext& graphicsContext, const sp<ark::Uploader>& uploader)
{
    _delegate->val()->upload(graphicsContext, uploader);
    _notifier.notify();
}

Resource::RecycleFunc Texture::recycle()
{
    return _delegate->val()->recycle();
}

Texture::Type Texture::type() const
{
    return _parameters->_type;
}

uint64_t Texture::id()
{
    return _delegate->val()->id();
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

const sp<Texture::Parameters>& Texture::parameters() const
{
    return _parameters;
}

sp<Texture::Delegate> Texture::delegate() const
{
    return _delegate->val();
}

const Notifier& Texture::notifier() const
{
    return _notifier;
}

template<> ARK_API Texture::Type Conversions::to<String, Texture::Type>(const String& str)
{
    if(str == "cubemap")
        return Texture::TYPE_CUBEMAP;
    DCHECK(str == "2d", "Unknow texture type: %s", str.c_str());
    return Texture::TYPE_2D;
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

template<> ARK_API Texture::CONSTANT Conversions::to<String, Texture::CONSTANT>(const String& str)
{
    if(str)
    {
        if(str == "nearest")
            return Texture::CONSTANT_NEAREST;
        if(str == "linear")
            return Texture::CONSTANT_LINEAR;
        if(str == "linear_mipmap")
            return Texture::CONSTANT_LINEAR_MIPMAP;
        if(str == "clamp_to_edge")
            return Texture::CONSTANT_CLAMP_TO_EDGE;
        if(str == "clamp_to_border")
            return Texture::CONSTANT_CLAMP_TO_BORDER;
        if(str == "mirrored_repeat")
            return Texture::CONSTANT_MIRRORED_REPEAT;
        if(str == "repeat")
            return Texture::CONSTANT_REPEAT;
        if(str == "mirror_clamp_to_edge")
            return Texture::CONSTANT_MIRROR_CLAMP_TO_EDGE;
    }
    DFATAL("Unknow TextureParameter: %s", str.c_str());
    return Texture::CONSTANT_NEAREST;
}

Texture::Parameters::Parameters(Type type, const document& parameters, Format format, Texture::Feature features)
    : _type(type), _format(parameters ? Documents::getAttribute<Texture::Format>(parameters, "format", format) : format),
      _features(parameters ? Documents::getAttribute<Texture::Feature>(parameters, "feature", features) : features),
      _min_filter((features & Texture::FEATURE_MIPMAPS) ? CONSTANT_LINEAR_MIPMAP : CONSTANT_LINEAR), _mag_filter(CONSTANT_LINEAR),
      _wrap_s(CONSTANT_CLAMP_TO_EDGE), _wrap_t(CONSTANT_CLAMP_TO_EDGE), _wrap_r(CONSTANT_CLAMP_TO_EDGE)
{
    if(parameters)
    {
        DictionaryByAttributeName byName(parameters, Constants::Attributes::NAME);
        _min_filter = getEnumValue(byName, "min_filter", _min_filter);
        _mag_filter = getEnumValue(byName, "mag_filter", _mag_filter);
        _wrap_s = getEnumValue(byName, "wrap_s", _wrap_s);
        _wrap_t = getEnumValue(byName, "wrap_t", _wrap_t);
        _wrap_r = getEnumValue(byName, "wrap_r", _wrap_r);
    }
}

Texture::CONSTANT Texture::Parameters::getEnumValue(Dictionary<document>& dict, const String& name, Texture::CONSTANT defValue)
{
    const document doc = dict.get(name);
    return doc ? Documents::getAttribute<Texture::CONSTANT>(doc, Constants::Attributes::VALUE, defValue) : defValue;
}

Texture::Delegate::Delegate(Texture::Type type)
    : _type(type)
{
}

Texture::Type Texture::Delegate::type() const
{
    return _type;
}

Texture::DICTIONARY::DICTIONARY(BeanFactory& /*factory*/, const String& value, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_loader_context(resourceLoaderContext), _src(value)
{
}

sp<Texture> Texture::DICTIONARY::build(const Scope& /*args*/)
{
    return _resource_loader_context->textureBundle()->getTexture(_src);
}


Texture::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_loader_context(resourceLoaderContext), _factory(factory), _manifest(manifest), _src(factory.getBuilder<String>(manifest, Constants::Attributes::SRC)),
      _uploader(factory.getBuilder<Texture::Uploader>(manifest, "uploader"))
{
}

sp<Texture> Texture::BUILDER::build(const Scope& args)
{
    Type type = Documents::getAttribute<Type>(_manifest, Constants::Attributes::TYPE, TYPE_2D);
    const sp<Texture::Parameters> parameters = sp<Texture::Parameters>::make(type, _manifest);

    const sp<String> src = _src->build(args);
    if(src)
       return _resource_loader_context->textureBundle()->createTexture(*src, parameters);

    const sp<Size> size = _factory.ensureConcreteClassBuilder<Size>(_manifest, Constants::Attributes::SIZE)->build(args);
    return _resource_loader_context->renderController()->createTexture(size, parameters, _uploader->build(args));
}

Texture::UploaderBitmap::UploaderBitmap(const bitmap& bitmap)
    : _bitmap(bitmap)
{
}

void Texture::UploaderBitmap::upload(GraphicsContext& graphicsContext, Texture::Delegate& delegate)
{
    delegate.uploadBitmap(graphicsContext, 0, _bitmap);
}

}
