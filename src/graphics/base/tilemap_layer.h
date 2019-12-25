#ifndef ARK_GRAPHICS_BASE_TILEMAP_LAYER_H_
#define ARK_GRAPHICS_BASE_TILEMAP_LAYER_H_

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/tilemap.h"
#include "graphics/impl/renderable/renderable_passive.h"


namespace ark {

class ARK_API TilemapLayer : public Renderer {
public:
// [[script::bindings::auto]]
    TilemapLayer(const Tilemap& tilemap, uint32_t rowCount, uint32_t colCount, const sp<Vec>& position = nullptr, Tilemap::LayerFlag flag = Tilemap::LAYER_FLAG_DEFAULT);

    virtual void render(RenderRequest& renderRequest, const V3& position) override;

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

// [[script::bindings::property]]
    uint32_t colCount() const;
// [[script::bindings::property]]
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
    void reset();

private:
    void viewportIntersect(float vs, float ve, float width, float& start, float& end);

private:
    uint32_t _col_count;
    uint32_t _row_count;
    sp<LayerContext> _layer_context;
    sp<Size> _size;
    sp<Tileset> _tileset;

    SafePtr<Vec> _position;
    SafePtr<Vec> _scroller;

    Tilemap::LayerFlag _flag;

    std::vector<sp<RenderablePassive>> _tiles;

    friend class Tilemap;
};

}

#endif
