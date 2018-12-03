#ifndef ARK_RENDERER_BASE_TEXTURE_H_
#define ARK_RENDERER_BASE_TEXTURE_H_

#include <unordered_map>

#include "core/base/api.h"
#include "core/base/bean_factory.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/inf/render_resource.h"

namespace ark {

class ARK_API Texture : public RenderResource {
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

    Texture(const sp<GLRecycler>& recycler, const sp<Size>& size, uint32_t target, Format format = FORMAT_AUTO, Feature features = FEATURE_DEFAULT);
    Texture(const sp<GLRecycler>& recycler, const sp<Size>& size, uint32_t target, const document& manifest);
    virtual ~Texture() override;

    virtual uint32_t id() override;
    virtual void prepare(GraphicsContext& graphicsContext) override;
    virtual void recycle(GraphicsContext&) override;

//  [[script::bindings::property]]
    int32_t width() const;
//  [[script::bindings::property]]
    int32_t height() const;
//  [[script::bindings::property]]
    int32_t depth() const;

//  [[script::bindings::property]]
    const sp<Size>& size() const;

    uint32_t target() const;

    void active(Pipeline& pipeline, uint32_t name);

    class Recycler : public RenderResource {
    public:
        Recycler(uint32_t id);

        virtual uint32_t id() override;
        virtual void prepare(GraphicsContext&) override;
        virtual void recycle(GraphicsContext&) override;

    private:
        uint32_t _id;
    };

    void setTexParameters(const document& doc);
    void setTexParameter(uint32_t name, int32_t value);

protected:
    virtual void doPrepareTexture(GraphicsContext& graphicsContext, uint32_t id) = 0;

    sp<GLRecycler> _recycler;
    sp<Size> _size;
    uint32_t _target;
    Format _format;
    Feature _features;

    uint32_t _id;
    std::unordered_map<uint32_t, int32_t> _tex_parameters;
};

}

#endif
