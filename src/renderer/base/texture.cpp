#include "renderer/base/texture.h"

#include "core/base/enum_map.h"
#include "core/inf/array.h"
#include "core/inf/variable.h"
#include "core/impl/dictionary/dictionary_by_attribute_name.h"
#include "core/util/string_convert.h"
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

class UploaderClear final : public Texture::Uploader {
public:
    UploaderClear(const Size& size, Texture::Format format)
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
    return static_cast<int32_t>(_size->widthAsFloat());
}

int32_t Texture::height() const
{
    return static_cast<int32_t>(_size->heightAsFloat());
}

int32_t Texture::depth() const
{
    return static_cast<int32_t>(_size->depthAsFloat());
}

const sp<Size>& Texture::size() const
{
    return _size;
}

const sp<Texture::Parameters>& Texture::parameters() const
{
    return _parameters;
}

void Texture::setParameters(sp<Parameters> parameters)
{
    _parameters = std::move(parameters);
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

template<> ARK_API Texture::Type StringConvert::eval<Texture::Type>(const String& str)
{
    if(str == "cubemap")
        return Texture::TYPE_CUBEMAP;
    DCHECK(str == "2d", "Unknow texture type: %s", str.c_str());
    return Texture::TYPE_2D;
}

template<> ARK_API Texture::Format StringConvert::eval<Texture::Format>(const String& str)
{
    constexpr std::array<std::pair<const char*, Texture::Format>, 11> formats = { {
            {"r", Texture::FORMAT_R},
            {"rg", Texture::FORMAT_RG},
            {"rgb", Texture::FORMAT_RGB},
            {"rgba", Texture::FORMAT_RGBA},
            {"signed", Texture::FORMAT_SIGNED},
            {"normalized", Texture::FORMAT_NORMALIZED},
            {"integer", Texture::FORMAT_INTEGER},
            {"float", Texture::FORMAT_FLOAT},
            {"8bit", Texture::FORMAT_8_BIT},
            {"16bit", Texture::FORMAT_16_BIT},
            {"32bit", Texture::FORMAT_32_BIT}
        }};
    if(str)
        return static_cast<Texture::Format>(BitSet<Texture::Format>::toBitSet(str, formats).bits());
    return Texture::FORMAT_AUTO;
}

template<> ARK_API Texture::Usage StringConvert::eval<Texture::Usage>(const String& str)
{
    constexpr std::array<std::pair<const char*, Texture::UsageBits>, 6> usages = { {
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

template<> ARK_API Texture::CONSTANT StringConvert::eval<Texture::CONSTANT>(const String& str)
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

template<> Texture::Flag StringConvert::eval<Texture::Flag>(const String& str)
{
    constexpr std::array<std::pair<const char*, Texture::Flag>, 2> flags = { {
        {"for_input", Texture::FLAG_FOR_INPUT},
        {"for_output", Texture::FLAG_FOR_OUTPUT},
    }};
    if(str)
        return static_cast<Texture::Flag>(BitSet<Texture::Flag>::toBitSet(str, flags).bits());
    return Texture::FLAG_FOR_INPUT;
}

Texture::Parameters::Parameters(Type type, const document& parameters, Format format, Texture::Feature features)
    : _type(type), _usage(parameters ? Documents::getAttribute<Texture::Usage>(parameters, "usage", Texture::USAGE_AUTO) : Texture::USAGE_AUTO),
      _format(parameters ? Documents::getAttribute<Texture::Format>(parameters, "format", format) : format),
      _features(parameters ? Documents::getAttribute<Texture::Feature>(parameters, "feature", features) : features),
      _flags(parameters ? Documents::getAttribute<Texture::Flag>(parameters, "flags", FLAG_FOR_INPUT) : FLAG_FOR_INPUT),
      _min_filter((features & Texture::FEATURE_MIPMAPS) ? CONSTANT_LINEAR_MIPMAP : CONSTANT_LINEAR), _mag_filter(CONSTANT_LINEAR),
      _wrap_s(CONSTANT_REPEAT), _wrap_t(CONSTANT_REPEAT), _wrap_r(CONSTANT_REPEAT)
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

Texture::CONSTANT Texture::Parameters::getEnumValue(Dictionary<document>& dict, const String& name, BeanFactory& factory, const Scope& args, Texture::CONSTANT defValue)
{
    const document doc = dict.get(name);
    return doc ? Strings::eval<Texture::CONSTANT>(factory.ensure<String>(doc, constants::VALUE, args)) : defValue;
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
    : _resource_loader_context(resourceLoaderContext), _factory(factory), _manifest(manifest), _src(factory.getBuilder<String>(manifest, constants::SRC)),
      _uploader(factory.getBuilder<Texture::Uploader>(manifest, "uploader")), _upload_strategy(Documents::getAttribute<RenderController::UploadStrategy>(manifest, "upload-strategy", {RenderController::US_ONCE_AND_ON_SURFACE_READY}).bits())
{
}

sp<Texture> Texture::BUILDER::build(const Scope& args)
{
    const Type type = Documents::getAttribute<Type>(_manifest, constants::TYPE, TYPE_2D);
    const sp<Texture::Parameters> parameters = sp<Texture::Parameters>::make(type, _manifest);
    parameters->loadParameters(_manifest, _factory, args);

    if(const sp<String> src = _src->build(args))
       return _resource_loader_context->textureBundle()->createTexture(*src, parameters);

    const sp<Size> size = _factory.ensureConcreteClassBuilder<Size>(_manifest, constants::SIZE)->build(args);
    CHECK(size->widthAsFloat() != 0 && size->heightAsFloat() != 0, "Cannot build texture from \"%s\"", Documents::toString(_manifest).c_str());
    sp<Uploader> uploader = _uploader->build(args);
    return _resource_loader_context->renderController()->createTexture(size, parameters, uploader ? std::move(uploader) : sp<Uploader>::make<UploaderClear>(size, parameters->_format), _upload_strategy);
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
