#ifndef ARK_APP_BASE_TILEMAP_H_
#define ARK_APP_BASE_TILEMAP_H_

#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
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
// [[script::bindings::auto]]
    Tilemap(const sp<LayerContext>& layerContext, uint32_t width, uint32_t height, const sp<Tileset>& tileset, const sp<TilemapImporter>& importer = nullptr);
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

//  [[script::bindings::auto]]
    void load(const sp<Readable>& readable);
//  [[script::bindings::auto]]
    void load(const String& src);

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

    SafePtr<Vec> _position;
    SafePtr<Vec> _scroller;

    uint32_t _col_count;
    uint32_t _row_count;

    sp<RenderObject>* _tiles;

};

}

#endif
