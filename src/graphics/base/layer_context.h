#ifndef ARK_GRAPHICS_BASE_LAYER_CONTEXT_H_
#define ARK_GRAPHICS_BASE_LAYER_CONTEXT_H_

#include "core/base/api.h"
#include "core/collection/filtered_list.h"

#include "graphics/forwarding.h"
#include "graphics/base/v2.h"
#include "graphics/base/layer.h"

namespace ark {

class LayerContext {
public:
    LayerContext();

    void renderRequest(const V2& position);

    void addRenderObject(const sp<RenderObject>& renderObject);
    void addRenderObject(const sp<RenderObject>& renderObject, const sp<Disposable>& lifecylce);
    void removeRenderObject(const sp<RenderObject>& renderObject);

    void clear();

    void takeSnapshot(Layer::Snapshot& output, MemoryPool& memoryPool);

private:
    class RenderObjectFilter {
    public:
        RenderObjectFilter(const sp<RenderObject>& renderObject, const sp<Disposable>& disposed);

         FilterAction operator()(const sp<RenderObject>& renderObject) const;

    private:
        sp<Disposable> _lifecycle;
    };

private:
    bool _render_requested;
    V2 _position;
    FilteredList<RenderObject, RenderObjectFilter> _items;
};

}

#endif
