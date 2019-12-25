#ifndef ARK_GRAPHICS_INF_RENDER_LAYER_H_
#define ARK_GRAPHICS_INF_RENDER_LAYER_H_

#include <vector>

#include "core/collection/list.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/layer.h"
#include "graphics/base/rect.h"
#include "graphics/inf/renderable.h"
#include "graphics/inf/renderer.h"

#include "renderer/base/buffer.h"

namespace ark {

class ARK_API RenderLayer : public Renderer {
public:
    struct Snapshot;

private:
    struct Stub {
        Stub(const sp<RenderModel>& renderModel, const sp<Shader>& shader, const sp<Vec4>& scissor, const sp<ResourceLoaderContext>& resourceLoaderContext);

        sp<RenderCommand> render(const Snapshot& snapshot, float x, float y);

        sp<RenderModel> _render_model;
        sp<ModelLoader> _model_loader;
        sp<Shader> _shader;
        sp<Vec4> _scissor;
        sp<ResourceLoaderContext> _resource_loader_context;

        sp<RenderController> _render_controller;
        sp<ShaderBindings> _shader_bindings;

        sp<Notifier> _notifier;
        sp<Boolean> _dirty;
        WeakRefList<LayerContext> _layer_contexts;
        sp<Layer> _layer;

        uint32_t _stride;

    };

public:
    struct UBOSnapshot {
        ByteArray::Borrowed _dirty_flags;
        ByteArray::Borrowed _buffer;
    };

    enum SnapshotFlag {
        SNAPSHOT_FLAG_RELOAD,
        SNAPSHOT_FLAG_DYNAMIC_UPDATE,
        SNAPSHOT_FLAG_STATIC_MODIFIED,
        SNAPSHOT_FLAG_STATIC_REUSE
    };

    struct Snapshot {
        Snapshot(Snapshot&& other) = default;

        sp<RenderCommand> render(const RenderRequest& renderRequest, const V3& position);

        sp<Stub> _stub;
        std::vector<UBOSnapshot> _ubos;
        std::vector<Renderable::Snapshot> _items;

        Buffer::Snapshot _index_buffer;

        Rect _scissor;

        SnapshotFlag _flag;

        DISALLOW_COPY_AND_ASSIGN(Snapshot);

    private:
        Snapshot(RenderRequest& renderRequest, const sp<Stub>& stub);

        void doRenderModelSnapshot(const RenderRequest& renderRequest, DrawingBuffer& buf) const;
        void doModelLoaderSnapshot(const RenderRequest& renderRequest, DrawingBuffer& buf) const;

        friend class RenderLayer;
    };

public:
    RenderLayer(const sp<RenderModel>& model, const sp<Shader>& shader, const sp<Vec4>& scissor, const sp<ResourceLoaderContext>& resourceLoaderContext);

    virtual void render(RenderRequest& renderRequest, const V3& position) override;

    const sp<RenderModel>& model() const;

    Snapshot snapshot(RenderRequest& renderRequest) const;

//  [[script::bindings::property]]
    const sp<Layer>& layer() const;
//  [[script::bindings::property]]
    const sp<LayerContext>& context() const;

//[[script::bindings::auto]]
    sp<LayerContext> makeContext(Layer::Type layerType) const;
//[[script::bindings::auto]]
    sp<Layer> makeLayer(Layer::Type layerType) const;

//  [[plugin::resource-loader]]
    class BUILDER : public Builder<RenderLayer> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext, sp<Builder<RenderModel>> renderModel, sp<Builder<Shader>> shader = nullptr);

        virtual sp<RenderLayer> build(const Scope& args) override;

    private:
        sp<ResourceLoaderContext> _resource_loader_context;
        sp<Builder<RenderModel>> _model;
        sp<Builder<Shader>> _shader;
        SafePtr<Builder<Vec4>> _scissor;
    };

//  [[plugin::resource-loader("render-layer")]]
    class RENDERER_BUILDER : public Builder<Renderer> {
    public:
        RENDERER_BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        BUILDER _impl;
    };

private:
    RenderLayer(const sp<Stub>& stub);

private:
    sp<Stub> _stub;

    friend class Layer;
};

}

#endif
