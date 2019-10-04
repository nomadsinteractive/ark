#ifndef ARK_GRAPHICS_BASE_LAYER_CONTEXT_H_
#define ARK_GRAPHICS_BASE_LAYER_CONTEXT_H_

#include "core/base/api.h"
#include "core/collection/list.h"
#include "core/inf/builder.h"
#include "core/inf/holder.h"

#include "graphics/forwarding.h"
#include "graphics/base/v2.h"
#include "graphics/base/render_layer.h"
#include "graphics/base/layer.h"

namespace ark {

//[[script::bindings::holder]]
class ARK_API LayerContext : public Holder{
private:
    struct Item {
        Item(const V3& position, const sp<RenderObject>& renderObject);
        DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Item);

        V3 _position;
        sp<RenderObject> _render_object;
    };

public:
    LayerContext(const sp<RenderModel>& renderModel, const sp<Notifier>& notifier, Layer::Type type);

    virtual void traverse(const Visitor& visitor) override;

    const sp<RenderModel>& renderModel() const;

    Layer::Type layerType() const;

    void renderRequest(const V3& position);

    void drawRenderObject(const V3& position, const sp<RenderObject>& renderObject);

//  [[script::bindings::auto]]
    void addRenderObject(const sp<RenderObject>& renderObject, const sp<Boolean>& disposed = sp<Boolean>::null());
//  [[script::bindings::auto]]
    void clear();

    void takeSnapshot(RenderLayer::Snapshot& output, MemoryPool& memoryPool);

    class BUILDER : public Builder<LayerContext> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, Layer::Type layerType);

        virtual sp<LayerContext> build(const Scope& args) override;

    private:
        sp<Builder<Layer>> _layer;
        sp<Builder<RenderLayer>> _render_layer;
        Layer::Type _layer_type;
    };

private:
    class RenderObjectFilter {
    public:
        RenderObjectFilter(const sp<RenderObject>& renderObject, const sp<Boolean>& disposed, const sp<Notifier>& notifier);

        FilterAction operator()(const sp<RenderObject>& renderObject) const;

    private:
        sp<Boolean> _disposed;
        sp<Notifier> _notifier;
    };

private:
    sp<RenderModel> _render_model;
    sp<Notifier> _notifier;
    Layer::Type _layer_type;

    bool _render_requested;
    V3 _position;
    std::vector<Item> _transient_items;
    List<RenderObject, RenderObjectFilter> _items;

};

}

#endif
