#pragma once

#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/impl/builder/safe_builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "app/forwarding.h"

namespace ark {

class ARK_API Tilemap {
public:
// [[script::bindings::auto]]
    Tilemap(sp<Tileset> tileset, sp<Importer<Tilemap>> importer = nullptr, sp<Outputer<Tilemap>> outputer = nullptr);

// [[script::bindings::property]]
    const sp<Tileset>& tileset() const;

// [[script::bindings::property]]
    const sp<Storage>& storage() const;

// [[script::bindings::auto]]
    sp<TilemapLayer> makeLayer(const String& name, uint32_t colCount, uint32_t rowCount, sp<Vec3> position = nullptr, sp<Boolean> visible = nullptr, sp<CollisionFilter> collisionFilter = nullptr, float zorder = 0);

// [[script::bindings::auto]]
    void clear();

//  [[script::bindings::property]]
    const Vector<sp<TilemapLayer>>& layers() const;

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
    Vector<V2i> findRoute(const V2i& start, const V2i& goal);

//  [[plugin::builder]]
    class BUILDER final : public Builder<Tilemap> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        sp<Tilemap> build(const Scope& args) override;

    private:
        sp<Builder<Tileset>> _tileset;
        SafeBuilder<Importer<Tilemap>> _importer;
        SafeBuilder<Outputer<Tilemap>> _outputer;
    };

private:
    sp<Tileset> _tileset;
    sp<Storage> _storage;

    Vector<sp<TilemapLayer>> _layers;

    friend class TilemapLayer;
    friend class BUILDER;
};

}
