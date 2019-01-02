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

    enum Parameter {
        PARAMETER_NEAREST,
        PARAMETER_LINEAR,
        PARAMETER_LINEAR_MIPMAP,
        PARAMETER_CLAMP_TO_EDGE,
        PARAMETER_CLAMP_TO_BORDER,
        PARAMETER_MIRRORED_REPEAT,
        PARAMETER_REPEAT,
        PARAMETER_MIRROR_CLAMP_TO_EDGE,
        PARAMETER_COUNT
    };

    struct ARK_API Parameters {
        Parameters(const document& parameters = nullptr, Format format = FORMAT_AUTO, Feature features = FEATURE_DEFAULT);

        Format _format;
        Feature _features;

        Parameter _min_filter;
        Parameter _mag_filter;

        Parameter _wrap_s;
        Parameter _wrap_t;
        Parameter _wrap_r;
    };

    Texture(const sp<Size>& size, const sp<Variable<sp<Resource>>>& resource, Type type);
    virtual ~Texture() override;

    virtual uintptr_t id() override;
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

    sp<Resource> resource() const;

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
    sp<Variable<sp<Resource>>> _resource;
    Type _type;
};

}

#endif
