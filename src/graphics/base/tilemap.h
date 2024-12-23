#pragma once

#include <vector>

#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/render_layer.h"
#include "graphics/inf/renderer.h"
#include "graphics/inf/render_batch.h"

#include "app/forwarding.h"

namespace ark {

class ARK_API Tilemap {
public:
// [[script::bindings::auto]]
    Tilemap(sp<Tileset> tileset, sp<RenderLayer> renderLayer = nullptr, sp<Importer<Tilemap>> importer = nullptr, sp<Outputer<Tilemap>> outputer = nullptr);

// [[script::bindings::property]]
    const sp<Tileset>& tileset() const;
// [[script::bindings::property]]
    const sp<RenderLayer>& renderLayer() const;

// [[script::bindings::property]]
    const sp<Storage>& storage() const;

// [[script::bindings::auto]]
    sp<TilemapLayer> makeLayer(const String& name, uint32_t colCount, uint32_t rowCount, sp<Vec3> position = nullptr, sp<Boolean> visible = nullptr, sp<CollisionFilter> collisionFilter = nullptr, float zorder = 0);

// [[script::bindings::auto]]
    void clear();

//  [[script::bindings::property]]
    const std::vector<sp<TilemapLayer>>& layers() const;

//  [[script::bindings::auto]]
    void load(const sp<Readable>& src);
//  [[script::bindings::auto]]
    void load(const String& src);

//  [[script::bindings::auto]]
    void addLayer(sp<TilemapLayer> layer, float zorder = 0);
//  [[script::bindings::auto]]
    void removeLayer(const sp<TilemapLayer>& layer);

    void jsonLoad(const Json& json);
    Json jsonDump() const;

//  [[script::bindings::auto]]
    std::vector<std::array<int32_t, 2>> findRoute(const std::array<int32_t, 2>& start, const std::array<int32_t, 2>& goal);

//  [[plugin::builder]]
    class BUILDER : public Builder<Tilemap> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        sp<Tilemap> build(const Scope& args) override;

    private:
        sp<Builder<Tileset>> _tileset;
        SafeBuilder<RenderLayer> _render_layer;
        SafeBuilder<Importer<Tilemap>> _importer;
        SafeBuilder<Outputer<Tilemap>> _outputer;
    };

private:

    class Stub : public RenderBatch {
    public:

        virtual std::vector<sp<LayerContext>>& snapshot(const RenderRequest& renderRequest) override;

        std::vector<sp<TilemapLayer>> _layers;
        sp<Scrollable> _scrollable;
        std::vector<sp<LayerContext>> _layer_contexts;
    };

private:
    sp<RenderLayer> _render_layer;
    sp<Tileset> _tileset;
    sp<Storage> _storage;

    sp<Stub> _stub;
    sp<LayerContext> _layer_context;

    friend class TilemapLayer;
    friend class BUILDER;
};

}
