#ifndef ARK_RENDERER_BASE_TEXTURE_H_
#define ARK_RENDERER_BASE_TEXTURE_H_

#include <unordered_map>

#include "core/base/api.h"
#include "core/base/string.h"
#include "core/base/bean_factory.h"
#include "core/inf/builder.h"
#include "core/types/safe_ptr.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/inf/resource.h"

namespace ark {

class ARK_API Texture : public Resource {
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

    struct ARK_API Parameters {
        Parameters(Format format, Feature features);

        void setTexParameter(uint32_t name, int32_t value);

        Format _format;
        Feature _features;

        std::unordered_map<uint32_t, int32_t> _tex_parameters;
    };

    Texture(const sp<Size>& size, const sp<Resource>& resource, Type type);
    virtual ~Texture() override;

    virtual uint32_t id() override;
    virtual void upload(GraphicsContext& graphicsContext) override;
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

    const sp<Resource>& resource() const;

public:

//  [[plugin::resource-loader::by-value("texture")]]
    class DICTIONARY : public Builder<Texture> {
    public:
        DICTIONARY(BeanFactory& factory, const String& value, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Texture> build(const sp<Scope>& args) override;

    private:
        sp<ResourceLoaderContext> _resource_loader_context;
        String _src;
    };

//  [[plugin::resource-loader("texture")]]
    class BUILDER : public Builder<Texture> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Texture> build(const sp<Scope>& args) override;

    private:
        sp<ResourceLoaderContext> _resource_loader_context;

        BeanFactory _factory;
        document _manifest;
        SafePtr<Builder<String>> _src;
        sp<Texture::Parameters> _parameters;
    };

private:
    sp<Size> _size;
    sp<Resource> _resource;
    Type _type;
};

}

#endif
