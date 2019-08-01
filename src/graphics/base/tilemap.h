#ifndef ARK_GRAPHICS_BASE_TILEMAP_H_
#define ARK_GRAPHICS_BASE_TILEMAP_H_

#include <list>

#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/types/safe_ptr.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/inf/block.h"
#include "graphics/inf/renderer.h"

#include "app/forwarding.h"

namespace ark {

//[[script::bindings::extends(Renderer)]]
class ARK_API Tilemap : public Renderer, Block {
public:
//  [[script::bindings::enumeration]]
    enum LayerFlag {
        LAYER_FLAG_COLLIDABLE = 1,
        LAYER_FLAG_DEFAULT = 0
    };

public:
// [[script::bindings::auto]]
    Tilemap(const sp<LayerContext>& layerContext, uint32_t width, uint32_t height, const sp<Tileset>& tileset, const sp<TilemapImporter>& importer = nullptr);

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
    const sp<Vec>& scroller() const;
// [[script::bindings::property]]
    void setScroller(const sp<Vec>& scroller);

//  [[script::bindings::property]]
    const std::list<sp<TilemapLayer>>& layers() const;

//  [[script::bindings::auto]]
    void load(const sp<Readable>& readable);
//  [[script::bindings::auto]]
    void load(const String& src);

//  [[script::bindings::auto]]
    sp<TilemapLayer> makeLayer(uint32_t rowCount = 0, uint32_t colCount = 0, const sp<Tileset>& tileset = nullptr, const sp<Vec>& position = nullptr, Tilemap::LayerFlag layerFlag = Tilemap::LAYER_FLAG_DEFAULT);
//  [[script::bindings::auto]]
    void addLayer(const sp<TilemapLayer>& layer);
//  [[script::bindings::auto]]
    void removeLayer(const sp<TilemapLayer>& layer);

//  [[plugin::builder]]
    class BUILDER : public Builder<Tilemap> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Tilemap> build(const sp<Scope>& args) override;

    private:
        sp<Builder<LayerContext>> _layer_context;
        sp<Builder<Integer>> _width;
        sp<Builder<Integer>> _height;
        sp<Builder<Tileset>> _tileset;
        sp<Builder<TilemapImporter>> _importer;
    };

private:
    sp<LayerContext> _layer_context;
    SafePtr<Size> _size;
    sp<Tileset> _tileset;
    sp<TilemapImporter> _importer;

    SafePtr<Vec> _scroller;

    uint32_t _col_count;
    uint32_t _row_count;

    std::list<sp<TilemapLayer>> _layers;

    friend class TilemapLayer;
};

}

#endif
