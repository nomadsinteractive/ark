#ifndef ARK_GRAPHICS_BASE_LAYER_CONTEXT_H_
#define ARK_GRAPHICS_BASE_LAYER_CONTEXT_H_

#include "core/base/api.h"
#include "core/collection/list_with_lifecycle.h"

#include "graphics/forwarding.h"
#include "graphics/base/v2.h"
#include "graphics/base/layer.h"

namespace ark {

class LayerContext {
public:
    LayerContext();

    void renderRequest(const V2& position);

    void addRenderObject(const sp<RenderObject>& renderObject);
    void addRenderObject(const sp<RenderObject>& renderObject, const sp<Boolean>& lifecylce);
    void removeRenderObject(const sp<RenderObject>& renderObject);

    void clear();

    bool takeSnapshot(Layer::Snapshot& output, MemoryPool& memoryPool);

private:
    class RenderObjectVaildator {
    public:
        RenderObjectVaildator(const sp<RenderObject>& obj, const sp<Boolean>& disposed);

        bool operator ()(const sp<RenderObject>& obj) const;

    private:
        sp<Boolean> _disposed;
    };

private:
    size_t _last_rendered_count;
    bool _render_requested;
    V2 _position;
    ListWithValidator<RenderObject, RenderObjectVaildator> _items;
};

}

#endif
