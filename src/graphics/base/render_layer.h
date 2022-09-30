#ifndef ARK_GRAPHICS_INF_RENDER_LAYER_H_
#define ARK_GRAPHICS_INF_RENDER_LAYER_H_

#include <deque>
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
    class Batch;

private:
    struct Stub {
        Stub(sp<ModelLoader> modelLoader, sp<Shader> shader, sp<Vec4> scissor, sp<RenderController> renderController);

        sp<LayerContext> makeLayerContext(sp<RenderLayer::Batch> batch, sp<ModelLoader> modelLoader, sp<Boolean> visible);

        sp<ModelLoader> _model_loader;
        sp<Shader> _shader;
        sp<Vec4> _scissor;

        sp<RenderController> _render_controller;
        sp<RenderCommandComposer> _render_command_composer;
        sp<ShaderBindings> _shader_bindings;

        SafeVar<Visibility> _visible;
        std::vector<sp<LayerContext>> _batch_groups;
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

        bool needsReload() const;

        sp<Stub> _stub;

        size_t _index_count;

        std::vector<UBOSnapshot> _ubos;
        std::vector<Buffer::Snapshot> _ssbos;
        std::deque<Renderable::Snapshot> _items;

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

    class ARK_API Batch {
    public:
        virtual ~Batch() = default;

        virtual bool preSnapshot(const RenderRequest& renderRequest, LayerContext& lc) = 0;
        virtual void snapshot(const RenderRequest& renderRequest, const LayerContext& lc, Snapshot& output) = 0;
    };

public:
    RenderLayer(sp<ModelLoader> modelLoader, sp<Shader> shader, sp<Vec4> scissor, sp<RenderController> renderController);

    virtual void render(RenderRequest& renderRequest, const V3& position) override;

    Snapshot snapshot(RenderRequest& renderRequest) const;

//  [[script::bindings::property]]
    const sp<Layer>& layer() const;
//  [[script::bindings::property]]
    const sp<LayerContext>& context() const;

//[[script::bindings::auto]]
    sp<Layer> makeLayer(sp<ModelLoader> modelLoader = nullptr, sp<Boolean> visible = nullptr) const;

    sp<LayerContext> makeContext(sp<RenderLayer::Batch> batch = nullptr, sp<ModelLoader> modelLoader = nullptr, sp<Boolean> visible = nullptr) const;

//  [[plugin::resource-loader]]
    class BUILDER : public Builder<RenderLayer> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext, sp<Builder<ModelLoader>> modelLoader, sp<Builder<Shader>> shader = nullptr);

        virtual sp<RenderLayer> build(const Scope& args) override;

    private:
        sp<ResourceLoaderContext> _resource_loader_context;
        std::vector<sp<Builder<Layer>>> _layers;
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
