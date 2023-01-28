#ifndef ARK_GRAPHICS_BASE_TILEMAP_LAYER_H_
#define ARK_GRAPHICS_BASE_TILEMAP_LAYER_H_

#include "core/base/api.h"
#include "core/base/timestamp.h"
#include "core/types/shared_ptr.h"
#include "core/types/safe_var.h"

#include "graphics/forwarding.h"
#include "graphics/base/rect.h"
#include "graphics/base/render_layer.h"
#include "graphics/base/tilemap.h"
#include "graphics/impl/renderable/renderable_passive.h"


namespace ark {

class ARK_API TilemapLayer {
public:
// [[script::bindings::auto]]
    TilemapLayer(sp<Tileset> tileset, String name, uint32_t colCount, uint32_t rowCount, sp<Vec3> position = nullptr, sp<Vec3> scroller = nullptr, sp<Boolean> visible = nullptr, Tilemap::LayerFlag flag = Tilemap::LAYER_FLAG_DEFAULT);

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
    Tilemap::LayerFlag flags() const;
// [[script::bindings::property]]
    void setFlag(Tilemap::LayerFlag flags);

// [[script::bindings::property]]
    uint32_t colCount() const;
// [[script::bindings::property]]
    uint32_t rowCount() const;

// [[script::bindings::property]]
    const SafeVar<Boolean>& visible() const;
// [[script::bindings::property]]
    void setVisible(sp<Boolean> visible);

// [[script::bindings::auto]]
    const sp<Tile>& getTile(uint32_t colId, uint32_t rowId) const;
// [[script::bindings::auto]]
    const sp<Tile>& getTileByPosition(float x, float y) const;
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

    struct LayerTile {
        LayerTile() = default;
        LayerTile(sp<Tile> tile, sp<RenderObject> renderable);

        sp<Tile> _tile;
        sp<RenderObject> _renderable;
        Renderable::State _state;
    };

    class Stub : public RenderableBatch {
    public:
        Stub(size_t colCount, size_t rowCount, sp<Tileset> tileset, SafeVar<Vec3> position, float zorder);
        DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Stub);

        virtual bool preSnapshot(const RenderRequest& renderRequest, LayerContext& lc) override;
        virtual void snapshot(const RenderRequest& renderRequest, const LayerContext& lc, RenderLayerSnapshot& output) override;

        size_t _col_count;
        size_t _row_count;
        sp<Tileset> _tileset;
        SafeVar<Vec3> _position;
        std::vector<LayerTile> _layer_tiles;
        float _zorder;
        Timestamp _timestamp;
    };

private:
    String _name;
    uint32_t _col_count;
    uint32_t _row_count;
    sp<Size> _size;

    SafeVar<Boolean> _visible;

    Tilemap::LayerFlag _flags;

    sp<Stub> _stub;

    friend class Tilemap;
};

}

#endif
