#include "graphics/base/tilemap.h"

#include <algorithm>

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/inf/importer.h"
#include "core/inf/variable.h"
#include "core/util/math.h"

#include "graphics/base/layer_context.h"
#include "graphics/base/render_object.h"
#include "graphics/base/size.h"
#include "graphics/base/tilemap_layer.h"
#include "graphics/base/tileset.h"
#include "graphics/base/v2.h"
#include "graphics/impl/frame/scrollable.h"
#include "graphics/util/vec2_type.h"

namespace ark {

namespace {

class TilemapLayerMaker : public RendererMaker {
public:
    TilemapLayerMaker(Tilemap& tilemap, sp<RendererMaker> delegate)
        : _tilemap(tilemap), _delegate(std::move(delegate)) {
    }

    virtual sp<Renderer> make(int32_t x, int32_t y) override {
        sp<Renderer> renderer = _delegate->make(x, y);
        sp<TilemapLayer> tilemapLayer = renderer.as<TilemapLayer>();
        DWARN(tilemapLayer, "Tilemap's RendererMaker should return TilemapLayer instance, others will be ignored");
        if(tilemapLayer) {
            tilemapLayer->setFlag(static_cast<Tilemap::LayerFlag>(tilemapLayer->flag() | Tilemap::LAYER_FLAG_SCROLLABLE));
            _tilemap.addLayer(tilemapLayer);
        }
        return renderer;
    }

    virtual void recycle(const sp<Renderer>& renderer) override {
        sp<TilemapLayer> tilemapLayer = renderer.as<TilemapLayer>();
        DWARN(tilemapLayer, "Tilemap's RendererMaker should return TilemapLayer instance, others will be ignored");
        if(tilemapLayer)
            _tilemap.removeLayer(tilemapLayer);
    }

private:
    Tilemap& _tilemap;
    sp<RendererMaker> _delegate;

};

}

Tilemap::Tilemap(const sp<LayerContext>& layerContext, const sp<Size>& size, const sp<Tileset>& tileset, const sp<TilemapImporter>& importer)
    : _layer_context(layerContext), _size(size), _tileset(tileset), _importer(importer)
{
    DASSERT(_layer_context);
}

void Tilemap::render(RenderRequest& renderRequest, const V3& position)
{
    _layer_context->renderRequest(position);

    if(_scrollable)
        _scrollable->render(renderRequest, V3(0));

    for(const sp<TilemapLayer>& i : _layers)
        i->render(renderRequest, V3(0));
}

const sp<Size>& Tilemap::size()
{
    return _size;
}

const sp<RenderObject>& Tilemap::getTile(uint32_t rowId, uint32_t colId) const
{
    return _layers.back()->getTile(rowId, colId);
}

int32_t Tilemap::getTileType(uint32_t rowId, uint32_t colId) const
{
    const sp<RenderObject>& renderObject = getTile(rowId, colId);
    return renderObject ? renderObject->type()->val() : -1;
}

void Tilemap::setTile(uint32_t rowId, uint32_t colId, const sp<RenderObject>& renderObject)
{
    _layers.back()->setTile(rowId, colId, renderObject);
}

void Tilemap::setTile(uint32_t rowId, uint32_t colId, int32_t tileId)
{
    const sp<RenderObject>& tile = _tileset->getTile(tileId);
    DCHECK(tile, "TileId %d does not exist", tileId);
    setTile(rowId, colId, tile);
}

void Tilemap::clear()
{
    _layers.clear();
}

const sp<Tileset>& Tilemap::tileset() const
{
    return _tileset;
}

const sp<Vec3>& Tilemap::scroller() const
{
    return _scroller;
}

void Tilemap::setScroller(const sp<Vec3>& scroller)
{
    _scroller = scroller;
}

void Tilemap::load(const sp<Readable>& readable)
{
    DASSERT(_importer);
    _importer->import(*this, readable);
}

void Tilemap::load(const String& src)
{
    load(Ark::instance().openAsset(src));
}

sp<TilemapLayer> Tilemap::makeLayer(uint32_t rowCount, uint32_t colCount, const sp<Vec3>& position, Tilemap::LayerFlag layerFlag)
{
    sp<TilemapLayer> layer = sp<TilemapLayer>::make(*this, rowCount, colCount, position, layerFlag);
    _layers.push_back(layer);
    return layer;
}

void Tilemap::addLayer(const sp<TilemapLayer>& layer)
{
    layer->_layer_context = _layer_context;
    _layers.push_back(layer);
}

void Tilemap::removeLayer(const sp<TilemapLayer>& layer)
{
    const auto iter = std::find(_layers.begin(), _layers.end(), layer);
    DCHECK(iter != _layers.end(), "Layer does not belong to this Tilemap");
    _layers.erase(iter);
}

const std::list<sp<TilemapLayer>>& Tilemap::layers() const
{
    return _layers;
}

Tilemap::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _layer_context(sp<LayerContext::BUILDER>::make(factory, manifest, Layer::TYPE_TRANSIENT)), _size(factory.ensureConcreteClassBuilder<Size>(manifest, Constants::Attributes::SIZE)),
      _tileset(factory.ensureBuilder<Tileset>(manifest, "tileset")), _importer(factory.getBuilder<TilemapImporter>(manifest, "importer")), _scrollable(manifest->getChild("scrollable")),
      _renderer_maker(_scrollable ? factory.ensureBuilder<RendererMaker>(_scrollable, "renderer-maker") : nullptr)
{
}

sp<Tilemap> Tilemap::BUILDER::build(const Scope& args)
{
    sp<Tilemap> tilemap = sp<Tilemap>::make(_layer_context->build(args), _size->build(args), _tileset->build(args), _importer->build(args));
    if(_scrollable)
        tilemap->_scrollable = sp<Scrollable>::make(tilemap->_scroller, sp<TilemapLayerMaker>::make(tilemap, _renderer_maker->build(args)), tilemap->_size, Scrollable::Params(_scrollable));
    return tilemap;
}

}
