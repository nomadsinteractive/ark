#include "renderer/base/texture.h"

#include "core/ark.h"
#include "core/inf/array.h"
#include "core/impl/dictionary/dictionary_by_attribute_name.h"
#include "core/util/string_convert.h"
#include "core/util/documents.h"
#include "core/util/log.h"

#include "graphics/base/bitmap.h"
#include "graphics/components/size.h"

#include "renderer/base/render_controller.h"
#include "renderer/base/texture_bundle.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/util/render_util.h"

namespace ark {

namespace {

class UploaderClear final : public Texture::Uploader {
public:
    UploaderClear(const Size& size, const Texture::Format format)
        : _bitmap(static_cast<uint32_t>(size.widthAsFloat()), static_cast<uint32_t>(size.heightAsFloat()), static_cast<uint32_t>(size.widthAsFloat()) * RenderUtil::getPixelSize(format),
                  (format & Texture::FORMAT_RGBA) + 1, true) {
    }

    void initialize(GraphicsContext& graphicsContext, Texture::Delegate& delegate) override {
        delegate.uploadBitmap(graphicsContext, _bitmap, {_bitmap.byteArray()});
    }

    void update(GraphicsContext& graphicsContext, Texture::Delegate& delegate) override {
        delegate.clear(graphicsContext);
    }

private:
    Bitmap _bitmap;
};

uint32_t toBitFormat(const StringView str, const uint32_t bitsOffset)
{
    DASSERT(str.length() >= bitsOffset);
    if(str.length() == bitsOffset)
        return Texture::FORMAT_32_BIT;

    switch(const int32_t bits = std::strtol(str.data() + bitsOffset, nullptr, 10); bits)
    {
        case 8:
            return Texture::FORMAT_8_BIT;
        case 16:
            return Texture::FORMAT_16_BIT;
        case 24:
            return Texture::FORMAT_24_BIT;
        case 32:
            return Texture::FORMAT_32_BIT;
        default:
            FATAL("Unknow texture component type: %s", str.data());
        break;
    }
    return 0;
}

Texture::Filter getEnumValue(Dictionary<document>& dict, const String& name, BeanFactory& factory, const Scope& args, const Texture::Filter defValue)
{
    const document doc = dict.get(name);
    return doc ? Strings::eval<Texture::Filter>(factory.ensure<String>(doc, constants::VALUE, args)) : defValue;
}

}

struct Texture::Stub {
    sp<Delegate> _delegate;
    sp<Size> _size;
    sp<Uploader> _uploader;
    sp<Parameters> _parameters;

    Timestamp _timestamp;
};

Texture::Texture(sp<Bitmap> bitmap, const Format textureFormat, const enums::UploadStrategy uploadStrategy, sp<Future> future)
    : Texture(*Ark::instance().renderController()->createTexture2d(std::move(bitmap), textureFormat, uploadStrategy, std::move(future)))
{
}

Texture::Texture(sp<Delegate> delegate, sp<Size> size, sp<Uploader> uploader, sp<Parameters> parameters)
    : _stub(sp<Stub>::make(Stub{std::move(delegate), std::move(size), std::move(uploader), std::move(parameters)}))
{
}

void Texture::upload(GraphicsContext& graphicsContext)
{
    _stub->_delegate->upload(graphicsContext, _stub->_uploader);
}

ResourceRecycleFunc Texture::recycle()
{
    return _stub->_delegate->recycle();
}

Texture::Type Texture::type() const
{
    return _stub->_parameters->_type;
}

Texture::Usage Texture::usage() const
{
    return _stub->_parameters->_usage;
}

uint64_t Texture::id()
{
    return _stub->_delegate->id();
}

int32_t Texture::width() const
{
    return static_cast<int32_t>(_stub->_size->widthAsFloat());
}

int32_t Texture::height() const
{
    return static_cast<int32_t>(_stub->_size->heightAsFloat());
}

int32_t Texture::depth() const
{
    return static_cast<int32_t>(_stub->_size->depthAsFloat());
}

const sp<Size>& Texture::size() const
{
    return _stub->_size;
}

const sp<Texture::Parameters>& Texture::parameters() const
{
    return _stub->_parameters;
}

void Texture::setParameters(sp<Parameters> parameters)
{
    _stub->_parameters = std::move(parameters);
    _stub->_timestamp.markDirty();
}

const sp<Texture::Delegate>& Texture::delegate() const
{
    return _stub->_delegate;
}

void Texture::reset(const Texture& texture)
{
    _stub->_delegate = texture._stub->_delegate;
    _stub->_size = texture._stub->_size;
    _stub->_timestamp.markDirty();
}

const sp<Texture::Uploader>& Texture::uploader() const
{
    return _stub->_uploader;
}

bool Texture::update(const uint64_t timestamp) const
{
    return _stub->_timestamp.update(timestamp);
}

template<> ARK_API Texture::Type StringConvert::eval<Texture::Type>(const String& str)
{
    if(str == "cubemap")
        return Texture::TYPE_CUBEMAP;
    DCHECK(str == "2d", "Unknow texture type: %s", str.c_str());
    return Texture::TYPE_2D;
}

template<> ARK_API Texture::Format StringConvert::eval<Texture::Format>(const String& str)
{
    constexpr BitSet<Texture::Format>::LookupTable<11> formats = {{
            {"r", Texture::FORMAT_R},
            {"rg", Texture::FORMAT_RG},
            {"rgb", Texture::FORMAT_RGB},
            {"rgba", Texture::FORMAT_RGBA},
            {"signed", Texture::FORMAT_SIGNED},
            {"normalized", Texture::FORMAT_NORMALIZED},
            {"integer", Texture::FORMAT_INTEGER},
            {"8bit", Texture::FORMAT_8_BIT},
            {"16bit", Texture::FORMAT_16_BIT},
            {"24bit", Texture::FORMAT_24_BIT},
            {"32bit", Texture::FORMAT_32_BIT}
        }};
    if(str)
    {
        uint32_t format = 0;
        for(const String& i : str.split('|'))
            if(const Texture::Format f = enums::lookup(formats, i, Texture::FORMAT_AUTO); f == Texture::FORMAT_AUTO)
            {
                if(i.startsWith("int"))
                    format |= Texture::FORMAT_INTEGER | Texture::FORMAT_SIGNED | toBitFormat(i, 3);
                else if(i.startsWith("uint"))
                    format |= Texture::FORMAT_INTEGER | toBitFormat(i, 4);
                else if(i.startsWith("float"))
                    format |= Texture::FORMAT_FLOAT | toBitFormat(i, 5);
            }
            else
                format |= static_cast<uint32_t>(f);
        return static_cast<Texture::Format>(format);
    }
    return Texture::FORMAT_AUTO;
}

template<> ARK_API Texture::Usage StringConvert::eval<Texture::Usage>(const String& str)
{
    constexpr Texture::Usage::LookupTable<6> usages = {{
        {"general", Texture::USAGE_AUTO},
        {"depth", Texture::USAGE_DEPTH_ATTACHMENT},
        {"stencil", Texture::USAGE_DEPTH_STENCIL_ATTACHMENT},
        {"color_attachment", Texture::USAGE_COLOR_ATTACHMENT},
        {"sampler", Texture::USAGE_SAMPLER},
        {"storage", Texture::USAGE_STORAGE}
    }};
    if(str)
        return Texture::Usage::toBitSet(str, usages);
    return {Texture::USAGE_AUTO};
}

template<> ARK_API Texture::Feature StringConvert::eval<Texture::Feature>(const String& str)
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

template<> ARK_API Texture::Filter StringConvert::eval<Texture::Filter>(const String& str)
{
    if(str)
    {
        if(str == "nearest")
            return Texture::FILTER_NEAREST;
        if(str == "linear")
            return Texture::FILTER_LINEAR;
        if(str == "linear_mipmap")
            return Texture::FILTER_LINEAR_MIPMAP;
        if(str == "clamp_to_edge")
            return Texture::FILTER_CLAMP_TO_EDGE;
        if(str == "clamp_to_border")
            return Texture::FILTER_CLAMP_TO_BORDER;
        if(str == "mirrored_repeat")
            return Texture::FILTER_MIRRORED_REPEAT;
        if(str == "repeat")
            return Texture::FILTER_REPEAT;
        if(str == "mirror_clamp_to_edge")
            return Texture::FILTER_MIRROR_CLAMP_TO_EDGE;
    }
    DFATAL("Unknow TextureParameter: %s", str.c_str());
    return Texture::FILTER_NEAREST;
}

Texture::Parameters::Parameters(Type type, const document& parameters, Format format, Texture::Feature features)
    : _type(type), _usage(parameters ? Documents::getAttribute<Texture::Usage>(parameters, "usage", Texture::USAGE_AUTO) : Texture::USAGE_AUTO),
      _format(parameters ? Documents::getAttribute<Texture::Format>(parameters, "format", format) : format),
      _features(parameters ? Documents::getAttribute<Texture::Feature>(parameters, "feature", features) : features),
      _min_filter((features & Texture::FEATURE_MIPMAPS) ? FILTER_LINEAR_MIPMAP : FILTER_LINEAR), _mag_filter(FILTER_LINEAR),
      _wrap_s(FILTER_REPEAT), _wrap_t(FILTER_REPEAT), _wrap_r(FILTER_REPEAT)
{
}

void Texture::Parameters::loadParameters(const document& parameters, BeanFactory& factory, const Scope& args)
{
    DictionaryByAttributeName byName(parameters, constants::NAME);
    _min_filter = getEnumValue(byName, "min_filter", factory, args, _min_filter);
    _mag_filter = getEnumValue(byName, "mag_filter", factory, args, _mag_filter);
    _wrap_s = getEnumValue(byName, "wrap_s", factory, args, _wrap_s);
    _wrap_t = getEnumValue(byName, "wrap_t", factory, args, _wrap_t);
    _wrap_r = getEnumValue(byName, "wrap_r", factory, args, _wrap_r);
}

Texture::Delegate::Delegate(const Type type)
    : _type(type)
{
}

Texture::Type Texture::Delegate::type() const
{
    return _type;
}

Texture::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_loader_context(resourceLoaderContext), _factory(factory), _manifest(manifest), _src(factory.getBuilder<String>(manifest, constants::SRC)), _bitmap(factory.getBuilder<Bitmap>(manifest, constants::BITMAP)),
      _uploader(factory.getBuilder<Texture::Uploader>(manifest, constants::UPLOADER)), _upload_strategy(Documents::getAttribute<enums::UploadStrategy>(manifest, "upload-strategy", {enums::UPLOAD_STRATEGY_ONCE_AND_ON_SURFACE_READY}).bits())
{
}

sp<Texture> Texture::BUILDER::build(const Scope& args)
{
    const Type type = Documents::getAttribute<Type>(_manifest, constants::TYPE, TYPE_2D);
    const sp<Parameters> parameters = sp<Texture::Parameters>::make(type, _manifest);
    parameters->loadParameters(_manifest, _factory, args);

    if(const sp<String> src = _src.build(args))
       return _resource_loader_context->textureBundle()->createTexture(*src, parameters);

    if(sp<Bitmap> bitmap = _bitmap.build(args))
        return _resource_loader_context->renderController()->createTexture2d(std::move(bitmap), parameters->_format, enums::UploadStrategy(_upload_strategy));

    const sp<Size> size = _factory.ensureConcreteClassBuilder<Size>(_manifest, constants::SIZE)->build(args);
    CHECK(size->widthAsFloat() != 0 && size->heightAsFloat() != 0, "Cannot build texture from \"%s\"", Documents::toString(_manifest).c_str());
    sp<Uploader> uploader = _uploader.build(args);
    return _resource_loader_context->renderController()->createTexture(size, parameters, uploader ? std::move(uploader) : sp<Uploader>::make<UploaderClear>(size, parameters->_format), enums::UploadStrategy(_upload_strategy));
}

Texture::UploaderBitmap::UploaderBitmap(sp<Bitmap> bitmap)
    : _bitmap(std::move(bitmap))
{
}

void Texture::UploaderBitmap::initialize(GraphicsContext& graphicsContext, Texture::Delegate& delegate)
{
    delegate.uploadBitmap(graphicsContext, _bitmap, {_bitmap->byteArray()});
}

void Texture::Uploader::update(GraphicsContext& graphicsContext, Delegate& delegate)
{
    initialize(graphicsContext, delegate);
}

}
