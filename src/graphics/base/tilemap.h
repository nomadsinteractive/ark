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
        LAYER_FLAG_SCROLLABLE = 2,
        LAYER_FLAG_DEFAULT = 0
    };

public:
// [[script::bindings::auto]]
    Tilemap(const sp<LayerContext>& layerContext, const sp<Size>& size, const sp<Tileset>& tileset, const sp<TilemapImporter>& importer = nullptr);

    virtual void render(RenderRequest& renderRequest, float x, float y) override;
// [[script::bindings::property]]
    virtual const SafePtr<Size>& size() override;


// [[script::bindings::auto]]
    sp<TilemapLayer> makeLayer(uint32_t rowCount, uint32_t colCount, const sp<Vec>& position = nullptr, Tilemap::LayerFlag layerFlag = Tilemap::LAYER_FLAG_DEFAULT);

// [[script::bindings::auto]]
    const sp<RenderObject>& getTile(uint32_t rowId, uint32_t colId) const;
// [[script::bindings::auto]]
    int32_t getTileType(uint32_t rowId, uint32_t colId) const;
// [[script::bindings::auto]]
    void setTile(uint32_t rowId, uint32_t colId, const sp<RenderObject>& renderObject);
// [[script::bindings::auto]]
    void setTile(uint32_t rowId, uint32_t colId, int32_t tileId);

// [[script::bindings::auto]]
    void clear();

// [[script::bindings::property]]
    const sp<Tileset>& tileset() const;

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
        sp<Builder<Size>> _size;
        sp<Builder<Tileset>> _tileset;
        sp<Builder<TilemapImporter>> _importer;

        document _scrollable;
        sp<Builder<RendererMaker>> _renderer_maker;
    };

private:
    sp<LayerContext> _layer_context;
    SafePtr<Size> _size;
    sp<Tileset> _tileset;
    sp<TilemapImporter> _importer;
    SafePtr<Vec> _scroller;

    std::list<sp<TilemapLayer>> _layers;
    sp<Scrollable> _scrollable;

    friend class TilemapLayer;
    friend class BUILDER;
};

}

#endif
