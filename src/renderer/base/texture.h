#ifndef ARK_RENDERER_BASE_TEXTURE_H_
#define ARK_RENDERER_BASE_TEXTURE_H_

#include "core/base/api.h"
#include "core/base/string.h"
#include "core/base/bean_factory.h"
#include "core/epi/notifier.h"
#include "core/inf/builder.h"
#include "core/types/safe_ptr.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/inf/resource.h"

namespace ark {

class ARK_API Texture final : public Resource {
public:
    enum Format {
        FORMAT_AUTO = 0x8000,
        FORMAT_R = 0,
        FORMAT_RG = 1,
        FORMAT_RGB = 2,
        FORMAT_RGBA = 3,
        FORMAT_SIGNED = 4,
        FORMAT_F16 = 8,
        FORMAT_F32 = 16
    };

    enum Feature {
        FEATURE_DEFAULT,
        FEATURE_MIPMAPS
    };

    enum Type {
        TYPE_2D,
        TYPE_CUBEMAP,
        TYPE_COUNT
    };

    enum CONSTANT {
        CONSTANT_NEAREST,
        CONSTANT_LINEAR,
        CONSTANT_LINEAR_MIPMAP,
        CONSTANT_CLAMP_TO_EDGE,
        CONSTANT_CLAMP_TO_BORDER,
        CONSTANT_MIRRORED_REPEAT,
        CONSTANT_REPEAT,
        CONSTANT_MIRROR_CLAMP_TO_EDGE,
        CONSTANT_COUNT
    };

    struct ARK_API Parameters {
        Parameters(Type type, const document& parameters = nullptr, Format format = FORMAT_AUTO, Feature features = FEATURE_DEFAULT);

        Type _type;
        Format _format;
        Feature _features;

        CONSTANT _min_filter;
        CONSTANT _mag_filter;

        CONSTANT _wrap_s;
        CONSTANT _wrap_t;
        CONSTANT _wrap_r;

    private:
        CONSTANT getEnumValue(Dictionary<document>& dict, const String& name, Texture::CONSTANT defValue);
    };

    class ARK_API Delegate : public Resource {
    public:
        Delegate(Type type);
        virtual ~Delegate() = default;

        Type type() const;

        virtual bool download(GraphicsContext& graphicsContext, Bitmap& bitmap) = 0;
        virtual void uploadBitmap(GraphicsContext& graphicsContext, uint32_t index, const Bitmap& bitmap) = 0;

    private:
        Type _type;
    };

    class ARK_API Uploader {
    public:
        virtual ~Uploader() = default;

        virtual void upload(GraphicsContext& graphicsContext, Delegate& delegate) = 0;

    };

    class ARK_API UploaderBitmap : public Uploader {
    public:
        UploaderBitmap(const bitmap& bitmap);

        virtual void upload(GraphicsContext& graphicsContext, Delegate& delegate) override;

    private:
        bitmap _bitmap;
    };

    Texture(const sp<Size>& size, const sp<Variable<sp<Delegate>>>& delegate, const sp<Parameters>& parameters);
    ~Texture() override;

    virtual uint64_t id() override;
    virtual void upload(GraphicsContext& graphicsContext, const sp<ark::Uploader>& uploader) override;
    virtual RecycleFunc recycle() override;

    Type type() const;

//  [[script::bindings::property]]
    int32_t width() const;
//  [[script::bindings::property]]
    int32_t height() const;
//  [[script::bindings::property]]
    int32_t depth() const;

//  [[script::bindings::property]]
    const sp<Size>& size() const;
    const sp<Parameters>& parameters() const;

    sp<Delegate> delegate() const;

    const Notifier& notifier() const;

public:

//  [[plugin::resource-loader::by-value]]
    class DICTIONARY : public Builder<Texture> {
    public:
        DICTIONARY(BeanFactory& factory, const String& value, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Texture> build(const Scope& args) override;

    private:
        sp<ResourceLoaderContext> _resource_loader_context;
        String _src;
    };

//  [[plugin::resource-loader]]
    class BUILDER : public Builder<Texture> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Texture> build(const Scope& args) override;

    private:
        sp<ResourceLoaderContext> _resource_loader_context;

        BeanFactory _factory;
        document _manifest;
        SafePtr<Builder<String>> _src;
        SafePtr<Builder<Texture::Uploader>> _uploader;
    };

private:
    sp<Size> _size;
    sp<Variable<sp<Delegate>>> _delegate;
    sp<Parameters> _parameters;

    Notifier _notifier;

};

}

#endif
