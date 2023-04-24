#pragma once

#include "core/base/api.h"
#include "core/base/timestamp.h"
#include "core/types/shared_ptr.h"
#include "core/types/safe_var.h"

#include "graphics/forwarding.h"
#include "graphics/base/rect.h"
#include "graphics/base/render_layer.h"
#include "graphics/base/tilemap.h"


namespace ark {

class ARK_API TilemapLayer {
public:
// [[script::bindings::auto]]
    TilemapLayer(sp<Tileset> tileset, String name, uint32_t colCount, uint32_t rowCount, sp<Vec3> position = nullptr, sp<Boolean> visible = nullptr, sp<CollisionFilter> collisionFilter = nullptr);

    bool getSelectionTileRange(const Rect& aabb, V3& selectionPosition, RectI& selectionRange) const;

// [[script::bindings::property]]
    const String& name() const;

// [[script::bindings::property]]
    const SafeVar<Vec3>& position() const;
// [[script::bindings::property]]
    void setPosition(sp<Vec3> position);

// [[script::bindings::property]]
    float zorder() const;

// [[script::bindings::property]]
    const sp<Tileset>& tileset() const;

// [[script::bindings::property]]
    const sp<CollisionFilter>& collisionFilter() const;
// [[script::bindings::property]]
    void setCollisionFilter(sp<CollisionFilter> collisionFilter);

    const sp<LayerContext>& layerContext() const;
    void setLayerContext(sp<LayerContext> layerContext);

// [[script::bindings::property]]
    uint32_t colCount() const;
// [[script::bindings::property]]
    uint32_t rowCount() const;

// [[script::bindings::property]]
    const SafeVar<Boolean>& visible() const;
// [[script::bindings::property]]
    void setVisible(sp<Boolean> visible);

// [[script::bindings::auto]]
    sp<Tile> getTile(uint32_t colId, uint32_t rowId) const;
// [[script::bindings::auto]]
    sp<Tile> getTileByPosition(float x, float y) const;
// [[script::bindings::auto]]
    void setTile(uint32_t col, uint32_t row, const sp<RenderObject>& renderObject);
// [[script::bindings::auto]]
    void setTile(uint32_t col, uint32_t row, int32_t tileId);
// [[script::bindings::auto]]
    void setTile(uint32_t col, uint32_t row, const sp<Tile>& tile);
// [[script::bindings::auto]]
    std::vector<int32_t> getTileRect(const RectI& rect) const;
// [[script::bindings::auto]]
    void setTileRect(const std::vector<int32_t>& tiles, const RectI& dest);

// [[script::bindings::auto]]
    void resize(uint32_t colCount, uint32_t rowCount, uint32_t offsetX = 0, uint32_t offsetY = 0);

// [[script::bindings::auto]]
    void clear();

// [[script::bindings::auto]]
    void foreachTile(const std::function<bool(uint32_t, uint32_t, const sp<Tile>&)>& callback) const;

private:
    void setTile(uint32_t col, uint32_t row, const sp<Tile>& tile, const sp<RenderObject>& renderObject);

    class Stub;

    class RenderableTile : public Renderable {
    public:
        RenderableTile(const sp<Stub>& stub, sp<Tile> tile, sp<RenderObject> renderable, const V3& position);

        virtual StateBits updateState(const RenderRequest& renderRequest) override;
        virtual Snapshot snapshot(const PipelineInput& pipelineInput, const RenderRequest& renderRequest, const V3& postTranslate, StateBits state) override;

        void dispose();

        sp<Stub> _stub;
        sp<Tile> _tile;
        sp<RenderObject> _renderable;
        V3 _position;
    };

    class Stub {
    public:
        Stub(size_t colCount, size_t rowCount, sp<Tileset> tileset, SafeVar<Vec3> position, float zorder);
        DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Stub);

        size_t _col_count;
        size_t _row_count;
        sp<Tileset> _tileset;
        SafeVar<Vec3> _position;
        float _zorder;
    };

private:
    String _name;
    uint32_t _col_count;
    uint32_t _row_count;
    sp<Size> _size;

    SafeVar<Boolean> _visible;
    sp<CollisionFilter> _collision_filter;
    sp<LayerContext> _layer_context;

    sp<Stub> _stub;

    std::vector<sp<RenderableTile>> _layer_tiles;

    friend class Tilemap;

};

}
