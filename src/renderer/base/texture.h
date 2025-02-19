#pragma once

#include "core/base/api.h"
#include "core/base/string.h"
#include "core/base/bean_factory.h"
#include "core/base/bit_set.h"
#include "core/base/timestamp.h"
#include "core/inf/builder.h"
#include "core/types/safe_builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/inf/resource.h"

namespace ark {

class ARK_API Texture final : public Resource {
public:
//  [[script::bindings::enumeration]]
    enum Format {
        FORMAT_AUTO = 0x8000,
        FORMAT_R = 0,
        FORMAT_RG = 1,
        FORMAT_RGB = 2,
        FORMAT_RGBA = 3,
        FORMAT_SIGNED = 8,
        FORMAT_NORMALIZED = 16,
        FORMAT_INTEGER = 32,
        FORMAT_FLOAT = 64,
        FORMAT_8_BIT = 128,
        FORMAT_16_BIT = 256,
        FORMAT_32_BIT = FORMAT_8_BIT | FORMAT_16_BIT
    };

    enum UsageBits {
        USAGE_AUTO = 0,
        USAGE_DEPTH_ATTACHMENT = 1,
        USAGE_STENCIL_ATTACHMENT = 2,
        USAGE_DEPTH_STENCIL_ATTACHMENT = 3,
        USAGE_COLOR_ATTACHMENT = 4,
        USAGE_ATTACHMENT = USAGE_DEPTH_STENCIL_ATTACHMENT | USAGE_COLOR_ATTACHMENT,
        USAGE_SAMPLER = 8,
        USAGE_STORAGE = 16
    };
    typedef BitSet<UsageBits> Usage;

    enum Feature {
        FEATURE_DEFAULT,
        FEATURE_MIPMAPS
    };

    enum Type {
        TYPE_2D,
        TYPE_CUBEMAP,
        TYPE_COUNT
    };

    enum Filter {
        FILTER_NEAREST,
        FILTER_LINEAR,
        FILTER_LINEAR_MIPMAP,
        FILTER_CLAMP_TO_EDGE,
        FILTER_CLAMP_TO_BORDER,
        FILTER_MIRRORED_REPEAT,
        FILTER_REPEAT,
        FILTER_MIRROR_CLAMP_TO_EDGE,
        FILTER_COUNT
    };

    struct Parameters {
        Parameters(Type type, const document& parameters = nullptr, Format format = FORMAT_AUTO, Feature features = FEATURE_DEFAULT);
        DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Parameters);

        void loadParameters(const document& parameters, BeanFactory& factory, const Scope& args);

        Type _type;
        Usage _usage;
        Format _format;
        Feature _features;

        Filter _min_filter;
        Filter _mag_filter;

        Filter _wrap_s;
        Filter _wrap_t;
        Filter _wrap_r;

    private:
        Filter getEnumValue(Dictionary<document>& dict, const String& name, BeanFactory& factory, const Scope& args, Texture::Filter defValue);
    };

    class Delegate;

    class ARK_API Uploader {
    public:
        virtual ~Uploader() = default;

        virtual void initialize(GraphicsContext& graphicsContext, Delegate& delegate) = 0;
        virtual void update(GraphicsContext& graphicsContext, Delegate& delegate);
    };

    class ARK_API Delegate {
    public:
        Delegate(Type type);
        virtual ~Delegate() = default;

        Type type() const;

        virtual uint64_t id() = 0;
        virtual void upload(GraphicsContext& graphicsContext, const sp<Uploader>& uploader) = 0;
        virtual ResourceRecycleFunc recycle() = 0;

        virtual void clear(GraphicsContext& graphicsContext) = 0;
        virtual bool download(GraphicsContext& graphicsContext, Bitmap& bitmap) = 0;
        virtual void uploadBitmap(GraphicsContext& graphicsContext, const Bitmap& bitmap, const std::vector<sp<ByteArray>>& imagedata) = 0;

    protected:
        Type _type;
    };

    class UploaderBitmap final : public Uploader {
    public:
        UploaderBitmap(sp<Bitmap> bitmap);

        void initialize(GraphicsContext& graphicsContext, Delegate& delegate) override;

    private:
        bitmap _bitmap;
    };

    Texture(sp<Delegate> delegate, sp<Size> size, sp<Uploader> uploader, sp<Parameters> parameters);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Texture);

    uint64_t id() override;
    void upload(GraphicsContext& graphicsContext) override;
    ResourceRecycleFunc recycle() override;

    Type type() const;
    Usage usage() const;

//  [[script::bindings::property]]
    int32_t width() const;
//  [[script::bindings::property]]
    int32_t height() const;
//  [[script::bindings::property]]
    int32_t depth() const;

//  [[script::bindings::property]]
    const sp<Size>& size() const;

    const sp<Parameters>& parameters() const;
    void setParameters(sp<Parameters> parameters);

    const sp<Delegate>& delegate() const;
    void setDelegate(sp<Delegate> delegate);
    void setDelegate(sp<Delegate> delegate, sp<Size> size);

    const sp<Uploader>& uploader() const;

    bool update(int64_t timestamp) const;

//  [[plugin::resource-loader]]
    class BUILDER final : public Builder<Texture> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        sp<Texture> build(const Scope& args) override;

    private:
        sp<ResourceLoaderContext> _resource_loader_context;

        BeanFactory _factory;
        document _manifest;
        SafeBuilder<String> _src;
        SafeBuilder<Uploader> _uploader;
        uint32_t _upload_strategy;
    };

private:
    sp<Delegate> _delegate;
    sp<Size> _size;
    sp<Uploader> _uploader;
    sp<Parameters> _parameters;

    Timestamp _timestamp;
};

}
