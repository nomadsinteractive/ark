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
        LAYER_FLAG_INVISIBLE = 4,
        LAYER_FLAG_DEFAULT = 0
    };

public:
// [[script::bindings::auto]]
    Tilemap(const sp<LayerContext>& layerContext, const sp<Size>& size, const sp<Tileset>& tileset, sp<Importer<Tilemap>> importer = nullptr, sp<Outputer<Tilemap>> outputer = nullptr);

    virtual void render(RenderRequest& renderRequest, const V3& position) override;
// [[script::bindings::property]]
    virtual const sp<Size>& size() override;

// [[script::bindings::property]]
    const sp<Tileset>& tileset() const;

// [[script::bindings::property]]
    const sp<Storage>& storage() const;

// [[script::bindings::auto]]
    sp<TilemapLayer> makeLayer(const String& name, uint32_t rowCount, uint32_t colCount, const sp<Vec3>& position = nullptr, const sp<Vec3>& scroller = nullptr, Tilemap::LayerFlag layerFlag = Tilemap::LAYER_FLAG_DEFAULT);

// [[script::bindings::auto]]
    void clear();

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

    void jsonLoad(const Json& json);
    Json jsonDump() const;

//  [[plugin::builder]]
    class BUILDER : public Builder<Tilemap> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Tilemap> build(const Scope& args) override;

    private:
        sp<Builder<LayerContext>> _layer_context;
        sp<Builder<Size>> _size;
        sp<Builder<Tileset>> _tileset;
        SafePtr<Builder<Importer<Tilemap>>> _importer;
        SafePtr<Builder<Outputer<Tilemap>>> _outputer;

        sp<Builder<Scrollable>> _scrollable;
    };

private:
    sp<LayerContext> _layer_context;
    SafePtr<Size> _size;
    sp<Tileset> _tileset;
    sp<Storage> _storage;

    std::list<sp<TilemapLayer>> _layers;
    sp<Scrollable> _scrollable;

    friend class TilemapLayer;
    friend class BUILDER;
};

}

#endif
