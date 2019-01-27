#ifndef ARK_RENDERER_IMPL_RENDER_MODEL_RENDER_MODEL_NINE_PATCH_H_
#define ARK_RENDERER_IMPL_RENDER_MODEL_RENDER_MODEL_NINE_PATCH_H_

#include "core/inf/builder.h"
#include "core/collection/by_index.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/rect.h"

#include "renderer/forwarding.h"
#include "renderer/base/render_controller.h"
#include "renderer/inf/render_model.h"

namespace ark {

class GLModelNinePatch : public RenderModel {
private:
    struct Item {
        Item(const Rect& bounds, const Rect& patches, uint32_t textureWidth, uint32_t textureHeight);
        Item(const Item& other) = default;
        Item() = default;

        uint16_t _x[4];
        uint16_t _y[4];

        Rect _paddings;
    };

public:
    GLModelNinePatch(const RenderController& renderController, const document& manifest, const sp<Atlas>& atlas);

    virtual sp<ShaderBindings> makeShaderBindings(const RenderController& renderController, const sp<PipelineLayout>& pipelineLayout) override;
    virtual void postSnapshot(RenderController& renderController, Layer::Snapshot& snapshot) override;

    virtual void start(ModelBuffer& buf, RenderController& renderController, const Layer::Snapshot& layerContext) override;
    virtual void load(ModelBuffer& buf, int32_t type, const V& size) override;

//  [[plugin::resource-loader("nine-patch")]]
    class BUILDER : public Builder<RenderModel> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<RenderModel> build(const sp<Scope>& args) override;

    private:
        document _manifest;
        sp<Builder<Atlas>> _atlas;
        sp<ResourceLoaderContext> _resource_loader_context;
    };

private:
    Rect getPatches(const document& doc, const Rect& bounds) const;

private:
    sp<Atlas> _atlas;
    sp<NamedBuffer> _index_buffer;

    ByIndex<Item> _nine_patch_items;
};

}

#endif
