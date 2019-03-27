#ifndef ARK_GRAPHICS_BASE_LAYER_CONTEXT_H_
#define ARK_GRAPHICS_BASE_LAYER_CONTEXT_H_

#include "core/base/api.h"
#include "core/collection/filtered_list.h"
#include "core/inf/builder.h"

#include "graphics/forwarding.h"
#include "graphics/base/v2.h"
#include "graphics/base/render_layer.h"

namespace ark {

class ARK_API LayerContext {
private:
    struct Item {
        Item(float x, float y, const sp<RenderObject>& renderObject);
        DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Item);

        float _x, _y;
        sp<RenderObject> _render_object;
    };

public:
    LayerContext(const sp<RenderModel>& renderModel, const sp<Notifier>& notifier);

    const sp<RenderModel>& renderModel() const;

    void renderRequest(const V2& position);

    void draw(float x, float y, const sp<RenderObject>& renderObject);

// [[script::bindings::auto]]
    void addRenderObject(const sp<RenderObject>& renderObject, const sp<Disposed>& disposed = sp<Disposed>::null());
// [[script::bindings::auto]]
    void removeRenderObject(const sp<RenderObject>& renderObject);
// [[script::bindings::auto]]
    void clear();

    void takeSnapshot(RenderLayer::Snapshot& output, MemoryPool& memoryPool);

    class BUILDER : public Builder<LayerContext> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, bool makeContext);

        virtual sp<LayerContext> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Layer>> _layer;
        sp<Builder<RenderLayer>> _render_layer;
        bool _make_context;
    };

private:
    class RenderObjectFilter {
    public:
        RenderObjectFilter(const sp<RenderObject>& renderObject, const sp<Disposed>& disposed, const sp<Notifier>& notifier);

        FilterAction operator()(const sp<RenderObject>& renderObject) const;

    private:
        sp<Disposed> _disposed;
        sp<Notifier> _notifier;
    };

private:
    sp<RenderModel> _render_model;
    sp<Notifier> _notifier;

    bool _render_requested;
    V2 _position;
    std::vector<Item> _transient_items;
    FilteredList<RenderObject, RenderObjectFilter> _items;
};

}

#endif
