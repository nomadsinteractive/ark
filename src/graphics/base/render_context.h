#ifndef ARK_GRAPHICS_BASE_RENDER_CONTEXT_H_
#define ARK_GRAPHICS_BASE_RENDER_CONTEXT_H_

#include "core/base/api.h"
#include "core/collection/expirable_item_list.h"

#include "graphics/forwarding.h"
#include "graphics/base/layer_context.h"

namespace ark {

class RenderContext {
public:
    RenderContext();

    void renderRequest(const V2& position);

    void addRenderObject(const sp<RenderObject>& renderObject);
    void addRenderObject(const sp<RenderObject>& renderObject, const sp<Boolean>& expired);
    void removeRenderObject(const sp<RenderObject>& renderObject);

    void clear();

    bool takeSnapshot(LayerContext::Snapshot& output, MemoryPool& memoryPool);

private:
    class RenderObjectVaildator {
    public:
        RenderObjectVaildator(const sp<RenderObject>& obj, const sp<Boolean>& expired);

        bool operator ()(const sp<RenderObject>& obj) const;

    private:
        sp<Boolean> _expired;
    };

private:
    size_t _last_rendered_count;
    bool _render_requested;
    V2 _position;
    ListWithValidator<RenderObject, RenderObjectVaildator> _items;
};

}

#endif
