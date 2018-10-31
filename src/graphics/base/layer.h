#ifndef ARK_GRAPHICS_INF_LAYER_H_
#define ARK_GRAPHICS_INF_LAYER_H_

#include <vector>

#include "core/collection/filtered_list.h"
#include "core/types/class.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/camera.h"
#include "graphics/base/render_object.h"
#include "graphics/inf/renderer.h"

namespace ark {

//[[script::bindings::auto]]
class ARK_API Layer : public Renderer {
private:
    struct Item {
        Item(float x, float y, const sp<RenderObject>& renderObject);
        DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Item);

        float x, y;
        sp<RenderObject> _render_object;
    };

public:
    struct Snapshot;

    struct Stub {
        Stub(const sp<GLModel>& model, const sp<GLShader>& shader, const sp<ResourceLoaderContext>& resourceLoaderContext);

        sp<RenderCommand> render(const Snapshot& snapshot, float x, float y);

        sp<GLModel> _model;
        sp<GLShader> _shader;
        sp<ResourceLoaderContext> _resource_loader_context;

        sp<MemoryPool> _memory_pool;
        sp<GLResourceManager> _resource_manager;
        sp<GLShaderBindings> _shader_bindings;

        std::vector<Item> _items;
        WeakRefList<LayerContext> _layer_contexts;
        size_t _last_rendered_count;
    };

    struct Snapshot {
        Snapshot(const sp<Stub>& stub);
        Snapshot(Snapshot&& other) = default;

        sp<RenderCommand> render(float x, float y) const;

        sp<Stub> _stub;
        Camera::Snapshot _camera;
        std::vector<RenderObject::Snapshot> _items;
        bool _dirty;

        DISALLOW_COPY_AND_ASSIGN(Snapshot);
    };

public:
    Layer(const sp<GLModel>& model, const sp<GLShader>& shader, const sp<ResourceLoaderContext>& resourceLoaderContext);

    virtual void render(RenderRequest& renderRequest, float x, float y) override;

    void draw(float x, float y, const sp<RenderObject>& renderObject);

    const sp<GLModel>& model() const;

    Snapshot snapshot() const;
    sp<LayerContext> makeContext();

    void measure(int32_t type, Size& size);

//  [[plugin::resource-loader]]
    class BUILDER : public Builder<Layer> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Layer> build(const sp<Scope>& args) override;

    private:
        sp<ResourceLoaderContext> _resource_loader_context;
        sp<Builder<GLModel>> _model;
        sp<Builder<GLShader>> _shader;
    };

private:
    sp<Stub> _stub;

};

}

#endif
