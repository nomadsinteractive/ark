#ifndef ARK_APP_BASE_TILE_MAP_H_
#define ARK_APP_BASE_TILE_MAP_H_

#include "core/base/api.h"
#include "core/types/safe_ptr.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/inf/renderer.h"
#include "graphics/inf/block.h"

#include "app/forwarding.h"

namespace ark {

//[[core::class]]
class ARK_API TileMap : public Renderer, Block {
public:
// [[script::bindings::auto]]
    TileMap(const sp<Layer>& layer, const sp<VV>& scroller, uint32_t width, uint32_t height, uint32_t tileWidth, uint32_t tileHeight);
    ~TileMap();

    virtual void render(RenderCommandPipeline& pipeline, float x, float y) override;
// [[script::bindings::property]]
    virtual const sp<Size>& size() override;

// [[script::bindings::auto]]
    const sp<RenderObject>& getTile(uint32_t rowId, uint32_t colId) const;
// [[script::bindings::auto]]
    int32_t getTileType(uint32_t rowId, uint32_t colId) const;
// [[script::bindings::auto]]
    const sp<RenderObject>& getTileByPosition(float x, float y) const;
// [[script::bindings::auto]]
    void setTile(uint32_t rowId, uint32_t colId, const sp<RenderObject>& renderObject);

// [[script::bindings::property]]
    uint32_t tileWidth() const;
// [[script::bindings::property]]
    uint32_t tileHeight() const;

// [[script::bindings::property]]
    uint32_t colCount() const;
// [[script::bindings::property]]
    uint32_t rowCount() const;

private:
    sp<Layer> _layer;
    sp<Size> _size;
    SafePtr<VV> _scroller;

    sp<Size> _tile_size;

    uint32_t _tile_width;
    uint32_t _tile_height;
    uint32_t _col_count;
    uint32_t _row_count;

    sp<RenderObject>* _tiles;

};

}

#endif
