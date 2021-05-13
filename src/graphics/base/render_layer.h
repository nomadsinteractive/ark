#ifndef ARK_GRAPHICS_INF_RENDER_LAYER_H_
#define ARK_GRAPHICS_INF_RENDER_LAYER_H_

#include <vector>
#include <unordered_map>

#include "core/collection/list.h"
#include "core/types/shared_ptr.h"
#include "core/types/weak_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/layer.h"
#include "graphics/base/rect.h"
#include "graphics/inf/renderable.h"
#include "graphics/inf/renderer.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"

namespace ark {

//[[script::bindings::extends(Renderer)]]
class ARK_API RenderLayer : public Renderer {
public:
    struct Snapshot;

private:
    class LayerContextFilter {
    public:
        LayerContextFilter(const sp<LayerContext>& item, sp<Notifier> notifier);

        FilterAction operator() (uint64_t ) const;

    private:
        WeakPtr<LayerContext> _item;
        sp<Notifier> _notifier;
    };

    struct Stub {
        Stub(const sp<ModelLoader>& modelLoader, const sp<Shader>& shader, const sp<Vec4>& scissor, const sp<ResourceLoaderContext>& resourceLoaderContext);

        sp<RenderCommand> render(const Snapshot& snapshot, float x, float y);
        sp<LayerContext> makeLayerContext(Layer::Type layerType);

        sp<ModelLoader> _model_loader;
        sp<Shader> _shader;
        sp<Vec4> _scissor;

        sp<RenderController> _render_controller;
        sp<RenderCommandComposer> _render_command_composer;
        sp<ShaderBindings> _shader_bindings;

        sp<Notifier> _notifier;
        sp<Boolean> _dirty;
        List<LayerContext, LayerContextFilter> _layer_contexts;
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
        std::vector<Buffer::Snapshot> _ssbos;
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
    RenderLayer(const sp<ModelLoader>& modelLoader, const sp<Shader>& shader, const sp<Vec4>& scissor, const sp<ResourceLoaderContext>& resourceLoaderContext);

    virtual void render(RenderRequest& renderRequest, const V3& position) override;

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
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext, sp<Builder<ModelLoader>> modelLoader, sp<Builder<Shader>> shader = nullptr);

        virtual sp<RenderLayer> build(const Scope& args) override;

    private:
        sp<ResourceLoaderContext> _resource_loader_context;
        sp<Builder<ModelLoader>> _model_loader;
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
