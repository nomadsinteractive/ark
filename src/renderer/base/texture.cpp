#include "renderer/base/texture.h"

#include "core/base/enums.h"
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
#include "renderer/util/render_util.h"

namespace ark {

namespace {

class BlankUploader : public Texture::Uploader {
public:
    BlankUploader(sp<Size> size, Texture::Format format)
        : _size(std::move(size)), _component_size(RenderUtil::getComponentSize(format)), _channels((format & Texture::FORMAT_RGBA) + 1) {
    }

    virtual void upload(GraphicsContext& graphicsContext, Texture::Delegate& delegate) override {
        uint32_t width = static_cast<uint32_t>(_size->width());
        Bitmap bitmap(width, static_cast<uint32_t>(_size->height()), width * _component_size, _channels, false);
        delegate.uploadBitmap(graphicsContext, bitmap, {nullptr});
    }

private:
    sp<Size> _size;
    uint32_t _component_size;
    uint8_t _channels;
};

}

Texture::Texture(sp<Delegate> delegate, sp<Size> size, sp<Texture::Uploader> uploader, sp<Parameters> parameters)
    : _delegate(std::move(delegate)), _size(std::move(size)), _uploader(std::move(uploader)), _parameters(std::move(parameters))
{
}

Texture::~Texture()
{
}

void Texture::upload(GraphicsContext& graphicsContext)
{
    _delegate->upload(graphicsContext, _uploader);
    _notifier.notify();
}

ResourceRecycleFunc Texture::recycle()
{
    return _delegate->recycle();
}

Texture::Type Texture::type() const
{
    return _parameters->_type;
}

Texture::Usage Texture::usage() const
{
    return _parameters->_usage;
}

uint64_t Texture::id()
{
    return _delegate->id();
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

const sp<Texture::Delegate>& Texture::delegate() const
{
    return _delegate;
}

void Texture::setDelegate(sp<Texture::Delegate> delegate)
{
    _delegate = std::move(delegate);
}

void Texture::setDelegate(sp<Delegate> delegate, sp<Size> size)
{
    _delegate = std::move(delegate);
    _size = std::move(size);
}

const sp<Texture::Uploader>& Texture::uploader() const
{
    return _uploader;
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
        return Enums<Texture::Format>::instance().toEnumCombo(str);
    return Texture::FORMAT_AUTO;
}

template<> ARK_API Texture::Usage Conversions::to<String, Texture::Usage>(const String& str)
{
    if(str)
        return Enums<Texture::Usage>::instance().toEnumCombo(str);
    return Texture::USAGE_COLOR_ATTACHMENT;
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

Texture::Parameters::Parameters(Type type, int, const document& parameters, Format format, Texture::Feature features)
    : _type(type), _usage(parameters ? Documents::getAttribute<Texture::Usage>(parameters, "usage", Texture::USAGE_COLOR_ATTACHMENT) : Texture::USAGE_COLOR_ATTACHMENT),
      _format(parameters ? Documents::getAttribute<Texture::Format>(parameters, "format", format) : format),
      _features(parameters ? Documents::getAttribute<Texture::Feature>(parameters, "feature", features) : features),
      _min_filter((features & Texture::FEATURE_MIPMAPS) ? CONSTANT_LINEAR_MIPMAP : CONSTANT_LINEAR), _mag_filter(CONSTANT_LINEAR),
      _wrap_s(CONSTANT_CLAMP_TO_EDGE), _wrap_t(CONSTANT_CLAMP_TO_EDGE), _wrap_r(CONSTANT_CLAMP_TO_EDGE)
{
}

void Texture::Parameters::loadParameters(const document& parameters, BeanFactory& factory, const Scope& args)
{
    DictionaryByAttributeName byName(parameters, Constants::Attributes::NAME);
    _min_filter = getEnumValue(byName, "min_filter", factory, args, _min_filter);
    _mag_filter = getEnumValue(byName, "mag_filter", factory, args, _mag_filter);
    _wrap_s = getEnumValue(byName, "wrap_s", factory, args, _wrap_s);
    _wrap_t = getEnumValue(byName, "wrap_t", factory, args, _wrap_t);
    _wrap_r = getEnumValue(byName, "wrap_r", factory, args, _wrap_r);
}

Texture::CONSTANT Texture::Parameters::getEnumValue(Dictionary<document>& dict, const String& name, BeanFactory& factory, const Scope& args, Texture::CONSTANT defValue)
{
    const document doc = dict.get(name);
    return doc ? Strings::parse<Texture::CONSTANT>(factory.ensure<String>(doc, Constants::Attributes::VALUE, args)) : defValue;
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
    const sp<Texture::Parameters> parameters = sp<Texture::Parameters>::make(type, 0, _manifest);
    parameters->loadParameters(_manifest, _factory, args);
    const sp<String> src = _src->build(args);
    if(src)
       return _resource_loader_context->textureBundle()->createTexture(*src, parameters);

    const sp<Size> size = _factory.ensureConcreteClassBuilder<Size>(_manifest, Constants::Attributes::SIZE)->build(args);
    DCHECK(size->width() != 0 && size->height() != 0, "Cannot build texture from \"%s\"", Documents::toString(_manifest).c_str());
    sp<Texture::Uploader> uploader = _uploader->build(args);
    return _resource_loader_context->renderController()->createTexture(size, parameters, uploader ? std::move(uploader) : makeBlankUploader(size, parameters));
}

sp<Texture::Uploader> Texture::BUILDER::makeBlankUploader(const sp<Size>& size, const Texture::Parameters& params)
{
    return sp<BlankUploader>::make(size, params._format);
}

Texture::UploaderBitmap::UploaderBitmap(const bitmap& bitmap)
    : _bitmap(bitmap)
{
}

void Texture::UploaderBitmap::upload(GraphicsContext& graphicsContext, Texture::Delegate& delegate)
{
    delegate.uploadBitmap(graphicsContext, _bitmap, {_bitmap->bytes()});
}

template<> ARK_API void Enums<Texture::Format>::initialize(std::map<String, Texture::Format>& enums)
{
    enums["r"] = Texture::FORMAT_R;
    enums["rg"] = Texture::FORMAT_RG;
    enums["rgb"] = Texture::FORMAT_RGB;
    enums["rgba"] = Texture::FORMAT_RGBA;
    enums["f16"] = Texture::FORMAT_F16;
    enums["f32"] = Texture::FORMAT_F32;
    enums["signed"] = Texture::FORMAT_SIGNED;
}

template<> ARK_API void Enums<Texture::Usage>::initialize(std::map<String, Texture::Usage>& enums)
{
    enums["color"] = Texture::USAGE_COLOR_ATTACHMENT;
    enums["depth"] = Texture::USAGE_DEPTH_ATTACHMENT;
    enums["stencil"] = Texture::USAGE_DEPTH_STENCIL_ATTACHMENT;
    enums["input_disabled"] = Texture::USAGE_INPUT_DISABLED;
    enums["output_disabled"] = Texture::USAGE_OUTPUT_DISABLED;
}

}
