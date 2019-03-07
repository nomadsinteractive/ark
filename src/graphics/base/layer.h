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

#include "renderer/base/buffer.h"

namespace ark {

//[[script::bindings::auto]]
class ARK_API Layer : public Renderer {
public:
    struct Snapshot;

    struct Stub {
        Stub(const sp<RenderModel>& model, const sp<Shader>& shader, const sp<ResourceLoaderContext>& resourceLoaderContext);

        sp<RenderCommand> render(const Snapshot& snapshot, float x, float y);

        sp<RenderModel> _model;
        sp<Shader> _shader;
        sp<ResourceLoaderContext> _resource_loader_context;

        sp<MemoryPool> _memory_pool;
        sp<RenderController> _render_controller;
        sp<ShaderBindings> _shader_bindings;

        WeakRefList<LayerContext> _layer_contexts;

        uint32_t _stride;
        size_t _last_rendered_count;

    };

    struct UBOSnapshot {
        bytearray _dirty_flags;
        bytearray _buffer;
    };

    struct Snapshot {
        Snapshot(Snapshot&& other) = default;

        sp<RenderCommand> render(float x, float y);

        sp<Stub> _stub;
        std::vector<UBOSnapshot> _ubos;
        std::vector<RenderObject::Snapshot> _items;

        Buffer::Snapshot _index_buffer;

        DISALLOW_COPY_AND_ASSIGN(Snapshot);

    private:
        Snapshot(const sp<Stub>& stub);

        friend class Layer;
    };

public:
    Layer(const sp<RenderModel>& model, const sp<Shader>& shader, const sp<ResourceLoaderContext>& resourceLoaderContext);

    virtual void render(RenderRequest& renderRequest, float x, float y) override;

    const sp<RenderModel>& model() const;

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
        sp<Builder<RenderModel>> _model;
        sp<Builder<Shader>> _shader;
    };

//  [[plugin::resource-loader("layer")]]
    class RENDERER_BUILDER : public Builder<Renderer> {
    public:
        RENDERER_BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Renderer> build(const sp<Scope>& args) override;

    private:
        BUILDER _impl;
    };

private:
    sp<Stub> _stub;
};

}

#endif
