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
    void addRenderObject(const sp<RenderObject>& renderObject, const sp<Lifecycle>& lifecylce);
    void removeRenderObject(const sp<RenderObject>& renderObject);

    void clear();

    bool takeSnapshot(Layer::Snapshot& output, MemoryPool& memoryPool);

private:
    class RenderObjectFilter {
    public:
        RenderObjectFilter(const sp<RenderObject>& renderObject, const sp<Lifecycle>& disposed);

         FilterAction operator()(const sp<RenderObject>& renderObject) const;

    private:
        sp<Lifecycle> _lifecycle;
    };

private:
    size_t _last_rendered_count;
    bool _render_requested;
    V2 _position;
    FilteredList<RenderObject, RenderObjectFilter> _items;
};

}

#endif
