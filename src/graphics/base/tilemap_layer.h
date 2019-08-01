#ifndef ARK_GRAPHICS_BASE_TILEMAP_LAYER_H_
#define ARK_GRAPHICS_BASE_TILEMAP_LAYER_H_

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/tilemap.h"


namespace ark {

class ARK_API TilemapLayer {
public:
// [[script::bindings::auto]]
    TilemapLayer(uint32_t rowCount, uint32_t colCount, const sp<Tileset>& tileset, const sp<Vec>& position, Tilemap::LayerFlag flag);
    ~TilemapLayer();

    void render(LayerContext& layerContext, const V& scroll, float width, float height);

// [[script::bindings::property]]
    const sp<Vec>& position() const;
// [[script::bindings::property]]
    void setPosition(const sp<Vec>& position);

// [[script::bindings::property]]
    const sp<Tileset>& tileset() const;

// [[script::bindings::property]]
    Tilemap::LayerFlag flag() const;
// [[script::bindings::property]]
    void setFlag(Tilemap::LayerFlag flag);

    uint32_t colCount() const;
    uint32_t rowCount() const;

// [[script::bindings::auto]]
    const sp<RenderObject>& getTile(uint32_t rowId, uint32_t colId) const;
// [[script::bindings::auto]]
    int32_t getTileType(uint32_t rowId, uint32_t colId) const;
// [[script::bindings::auto]]
    const sp<RenderObject>& getTileByPosition(float x, float y) const;
// [[script::bindings::auto]]
    void setTile(uint32_t row, uint32_t col, const sp<RenderObject>& renderObject);
// [[script::bindings::auto]]
    void setTile(uint32_t row, uint32_t col, int32_t tileId);

// [[script::bindings::auto]]
    void clear();

private:
    uint32_t _col_count;
    uint32_t _row_count;
    sp<Tileset> _tileset;

    SafePtr<Vec> _position;

    Tilemap::LayerFlag _flag;

    sp<RenderObject>* _tiles;

    friend class Tilemap;
};

}

#endif
