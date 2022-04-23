#include "graphics/base/tilemap.h"

#include <algorithm>

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/inf/storage.h"
#include "core/inf/variable.h"
#include "core/util/math.h"

#include "graphics/base/layer_context.h"
#include "graphics/base/render_object.h"
#include "graphics/base/size.h"
#include "graphics/base/tile.h"
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
            tilemapLayer->setFlag(static_cast<Tilemap::LayerFlag>(tilemapLayer->flag() | Tilemap::LAYER_FLAG_INVISIBLE));
            _tilemap.addLayer(tilemapLayer);
        }
        return renderer;
    }

    virtual void recycle(const sp<Renderer>& renderer) override {
        sp<TilemapLayer> tilemapLayer = renderer.as<TilemapLayer>();
        DWARN(tilemapLayer, "Tilemap's RendererMaker should return TilemapLayer instance, others will be ignored");
        _delegate->recycle(renderer);
        if(tilemapLayer)
            _tilemap.removeLayer(tilemapLayer);
    }

private:
    Tilemap& _tilemap;
    sp<RendererMaker> _delegate;
};

}

Tilemap::Tilemap(const sp<LayerContext>& layerContext, const sp<Size>& size, const sp<Tileset>& tileset, sp<Importer<Tilemap>> importer, sp<Outputer<Tilemap>> outputer)
    : _layer_context(layerContext), _size(size), _tileset(tileset), _storage(sp<Storage::Composite<Tilemap>>::make(*this, std::move(importer), std::move(outputer)))
{
    DASSERT(_layer_context);
}

void Tilemap::render(RenderRequest& renderRequest, const V3& position)
{
    _layer_context->renderRequest(position);

    if(_scrollable)
        _scrollable->render(renderRequest, V3(0));

    for(const sp<TilemapLayer>& i : _layers)
        if(!(i->flag() & LAYER_FLAG_INVISIBLE))
            i->render(renderRequest, V3());
}

const sp<Size>& Tilemap::size()
{
    return _size;
}

void Tilemap::clear()
{
    _layers.clear();
}

const sp<Tileset>& Tilemap::tileset() const
{
    return _tileset;
}

const sp<Storage>& Tilemap::storage() const
{
    return _storage;
}

void Tilemap::load(const sp<Readable>& readable)
{
    _storage->import(readable);
}

void Tilemap::load(const String& src)
{
    load(Ark::instance().openAsset(src));
}

sp<TilemapLayer> Tilemap::makeLayer(const String& name, uint32_t rowCount, uint32_t colCount, const sp<Vec3>& position, const sp<Vec3>& scroller, Tilemap::LayerFlag layerFlag)
{
    sp<TilemapLayer> layer = sp<TilemapLayer>::make(*this, name, rowCount, colCount, position, scroller, layerFlag);
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
      _tileset(factory.ensureBuilder<Tileset>(manifest, "tileset")), _importer(factory.getBuilder<Importer<Tilemap>>(manifest, "importer")), _outputer(factory.getBuilder<Outputer<Tilemap>>(manifest, "outputer")),
      _scrollable(factory.getBuilder<Scrollable>(manifest, "scrollable"))
{
}

sp<Tilemap> Tilemap::BUILDER::build(const Scope& args)
{
    sp<Tilemap> tilemap = sp<Tilemap>::make(_layer_context->build(args), _size->build(args), _tileset->build(args), _importer->build(args), _outputer->build(args));
    if(_scrollable)
    {
        sp<Scrollable> scrollable = _scrollable->build(args);
        scrollable->setRendererMaker(sp<TilemapLayerMaker>::make(tilemap, scrollable->rendererMaker()));
        tilemap->_scrollable = std::move(scrollable);
    }
    return tilemap;
}

}
