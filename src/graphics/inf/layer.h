#ifndef ARK_GRAPHICS_INF_LAYER_H_
#define ARK_GRAPHICS_INF_LAYER_H_

#include <vector>

#include "core/collection/expirable_item_list.h"
#include "core/types/class.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/camera.h"
#include "graphics/base/layer_context.h"
#include "graphics/inf/renderer.h"

namespace ark {

//[[script::bindings::auto]]
//[[core::class]]
class ARK_API Layer {
private:
    struct Item {
        Item(float x, float y, const sp<RenderObject>& renderObject);
        DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Item);

        float x, y;
        sp<RenderObject> _render_object;
    };

//    class Stub {
//    public:
//        Stub(const sp<Camera>& camera, const sp<MemoryPool>& memoryPool);

//        void draw(float x, float y, const sp<RenderObject>& renderObject);
//        void clear();

//        sp<RenderContext> makeRenderContext();

//        friend class Layer;
//    };

public:
    struct Snapshot {
        Snapshot(const Layer& layer, MemoryPool& memoryPool);
        Snapshot(Snapshot&& other) = default;

        Camera::Snapshot _camera;
        std::vector<RenderObject::Snapshot> _items;
        bool _dirty;

        DISALLOW_COPY_AND_ASSIGN(Snapshot);
    };

public:
    Layer(const sp<Camera>& camera, const sp<MemoryPool>& memoryPool);
    virtual ~Layer() = default;

    void draw(float x, float y, const sp<RenderObject>& renderObject);
    void clear();

    Snapshot snapshot() const;
    sp<RenderContext> makeRenderContext();

    virtual sp<RenderCommand> render(const Snapshot& layerContext, float x, float y) = 0;

    class Renderer : public ark::Renderer {
    public:
        Renderer(const sp<Layer>& layer);

        virtual void render(RenderRequest& renderRequest, float x, float y) override;

    private:
        sp<Layer> _layer;
    };

private:
    sp<Camera> _camera;
    sp<MemoryPool> _memory_pool;

    std::vector<Item> _items;
    WeakRefList<RenderContext> _render_contexts;
    size_t _last_rendered_count;
};

}

#endif
