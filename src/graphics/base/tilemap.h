#ifndef ARK_APP_BASE_TILEMAP_H_
#define ARK_APP_BASE_TILEMAP_H_

#include "core/base/api.h"
#include "core/inf/variable.h"
#include "core/types/safe_ptr.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/inf/renderer.h"
#include "graphics/inf/block.h"

#include "app/forwarding.h"

namespace ark {

class ARK_API Tilemap : public Renderer, Block {
public:
// [[script::bindings::auto]]
    Tilemap(const sp<LayerContext>& layerContext, uint32_t width, uint32_t height, const sp<Tileset>& tileset);
    ~Tilemap() override;

    virtual void render(RenderRequest& renderRequest, float x, float y) override;
// [[script::bindings::property]]
    virtual const SafePtr<Size>& size() override;

// [[script::bindings::auto]]
    const sp<RenderObject>& getTile(uint32_t rowId, uint32_t colId) const;
// [[script::bindings::auto]]
    int32_t getTileType(uint32_t rowId, uint32_t colId) const;
// [[script::bindings::auto]]
    const sp<RenderObject>& getTileByPosition(float x, float y) const;
// [[script::bindings::auto]]
    void setTile(uint32_t rowId, uint32_t colId, const sp<RenderObject>& renderObject);
// [[script::bindings::auto]]
    void setTile(uint32_t rowId, uint32_t colId, int32_t tileId);

// [[script::bindings::auto]]
    void clear();

// [[script::bindings::property]]
    const sp<Tileset>& tileset() const;

// [[script::bindings::property]]
    uint32_t colCount() const;
// [[script::bindings::property]]
    uint32_t rowCount() const;

// [[script::bindings::property]]
    const sp<Vec>& position() const;
// [[script::bindings::property]]
    void setPosition(const sp<Vec>& position);

// [[script::bindings::property]]
    const sp<Vec>& scroller() const;
// [[script::bindings::property]]
    void setScroller(const sp<Vec>& scroller);

private:
    sp<LayerContext> _layer_context;
    SafePtr<Size> _size;
    sp<Tileset> _tileset;

    SafePtr<Vec> _position;
    SafePtr<Vec> _scroller;

    uint32_t _col_count;
    uint32_t _row_count;

    sp<RenderObject>* _tiles;

};

}

#endif
