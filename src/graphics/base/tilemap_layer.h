#ifndef ARK_GRAPHICS_BASE_TILEMAP_LAYER_H_
#define ARK_GRAPHICS_BASE_TILEMAP_LAYER_H_

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/rect.h"
#include "graphics/base/tilemap.h"
#include "graphics/impl/renderable/renderable_passive.h"


namespace ark {

class ARK_API TilemapLayer : public Renderer {
public:
// [[script::bindings::auto]]
    TilemapLayer(Layer& layer, sp<Tileset> tileset, String name, uint32_t rowCount, uint32_t colCount, sp<Vec3> position = nullptr, sp<Vec3> scroller = nullptr, Tilemap::LayerFlag flag = Tilemap::LAYER_FLAG_DEFAULT);
    TilemapLayer(sp<LayerContext> layerContext, sp<Tileset> tileset, String name, uint32_t rowCount, uint32_t colCount, sp<Vec3> position, sp<Vec3> scroller, Tilemap::LayerFlag flag);

    virtual void render(RenderRequest& renderRequest, const V3& position) override;

    bool getSelectionTileRange(const Rect& aabb, V3& selectionPosition, RectI& selectionRange) const;

// [[script::bindings::property]]
    const String& name() const;

// [[script::bindings::property]]
    const sp<Vec3>& position() const;
// [[script::bindings::property]]
    void setPosition(const sp<Vec3>& position);

// [[script::bindings::property]]
    const sp<Tileset>& tileset() const;

// [[script::bindings::property]]
    Tilemap::LayerFlag flag() const;
// [[script::bindings::property]]
    void setFlag(Tilemap::LayerFlag flag);

// [[script::bindings::property]]
    uint32_t colCount() const;
// [[script::bindings::property]]
    uint32_t rowCount() const;

// [[script::bindings::property]]
    const sp<Vec3>& scroller() const;
// [[script::bindings::property]]
    void setScroller(const sp<Vec3>& scroller);

// [[script::bindings::auto]]
    const sp<Tile>& getTile(uint32_t rowId, uint32_t colId) const;
// [[script::bindings::auto]]
    std::vector<int32_t> getTileRect(const RectI& rect) const;
// [[script::bindings::auto]]
    const sp<Tile>& getTileByPosition(float x, float y) const;
// [[script::bindings::auto]]
    void setTile(uint32_t row, uint32_t col, const sp<RenderObject>& renderObject);
// [[script::bindings::auto]]
    void setTile(uint32_t row, uint32_t col, int32_t tileId);
// [[script::bindings::auto]]
    void setTile(uint32_t row, uint32_t col, const sp<Tile>& tile);
// [[script::bindings::auto]]
    void copyTiles(const std::vector<int32_t>& tiles, const RectI& dest = RectI());

// [[script::bindings::auto]]
    void reset();

// [[script::bindings::auto]]
    void foreachTile(const std::function<bool(uint32_t, uint32_t, const sp<Tile>&)>& callback) const;

private:
    void renderTiles(const V3& position, const RectI& renderRange);
    void setTile(uint32_t row, uint32_t col, const sp<Tile>& tile, const sp<RenderObject>& renderObject);

    struct LayerTile {
        LayerTile() = default;
        LayerTile(sp<Tile> tile, sp<RenderablePassive> renderable);

        sp<Tile> tile;
        sp<RenderablePassive> renderable;
    };

private:
    String _name;
    uint32_t _col_count;
    uint32_t _row_count;
    sp<LayerContext> _layer_context;
    sp<Tileset> _tileset;
    sp<Size> _size;

    SafePtr<Vec3> _position;
    SafePtr<Vec3> _scroller;

    Tilemap::LayerFlag _flag;

    std::vector<LayerTile> _layer_tiles;

    friend class Tilemap;
};

}

#endif
