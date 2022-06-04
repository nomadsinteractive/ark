#include "graphics/base/tilemap.h"

#include <algorithm>

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/base/json.h"
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

    virtual std::vector<sp<Renderer>> make(int32_t x, int32_t y) override {
        std::vector<sp<Renderer>> renderers = _delegate->make(x, y);
        for(const sp<Renderer>& i : renderers) {
            sp<TilemapLayer> tilemapLayer = i.as<TilemapLayer>();
            DWARN(tilemapLayer, "Tilemap's RendererMaker should return TilemapLayer instance, others will be ignored");
            if(tilemapLayer) {
                tilemapLayer->setFlag(static_cast<Tilemap::LayerFlag>(tilemapLayer->flag() | Tilemap::LAYER_FLAG_INVISIBLE));
                _tilemap.addLayer(tilemapLayer);
            }
        }
        return renderers;
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


class TilemapStorage : public Storage::Composite<Tilemap> {
public:
    TilemapStorage(Tilemap& tilemap, sp<Importer<Tilemap>> importer, sp<Outputer<Tilemap>> outputer)
        : Storage::Composite<Tilemap>(tilemap, std::move(importer), std::move(outputer)) {
    }

    virtual void jsonLoad(const Json& json) override {
        _obj.jsonLoad(json);
    }

    virtual Json jsonDump() override {
        return _obj.jsonDump();
    }

};

}

Tilemap::Tilemap(sp<LayerContext> layerContext, sp<Size> size, sp<Tileset> tileset, sp<Importer<Tilemap>> importer, sp<Outputer<Tilemap>> outputer)
    : _layer_context(std::move(layerContext)), _size(std::move(size)), _tileset(std::move(tileset)), _storage(sp<TilemapStorage>::make(*this, std::move(importer), std::move(outputer))),
      _stub(sp<Stub>::make())
{
}

const sp<Size>& Tilemap::size()
{
    return _size;
}

void Tilemap::clear()
{
    _stub->_layers.clear();
}

const sp<Tileset>& Tilemap::tileset() const
{
    return _tileset;
}

const sp<Storage>& Tilemap::storage() const
{
    return _storage;
}

sp<Renderer> Tilemap::makeRenderer(const sp<Layer>& layer) const
{
    DCHECK(layer || _layer_context, "No LayerContext specified");
    return sp<TilemapRenderer>::make(layer ? layer->context() : _layer_context, _stub);
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
    sp<TilemapLayer> layer = sp<TilemapLayer>::make(_layer_context, _tileset, name, rowCount, colCount, position, scroller, layerFlag);
    _stub->_layers.push_back(layer);
    return layer;
}

void Tilemap::addLayer(sp<TilemapLayer> layer)
{
    layer->_layer_context = _layer_context;
    _stub->_layers.push_back(std::move(layer));
}

void Tilemap::removeLayer(const sp<TilemapLayer>& layer)
{
    const auto iter = std::find(_stub->_layers.begin(), _stub->_layers.end(), layer);
    DCHECK(iter != _stub->_layers.end(), "Layer does not belong to this Tilemap");
    _stub->_layers.erase(iter);
}

void Tilemap::jsonLoad(const Json& json)
{
    clear();

    const sp<Json> layers = json.get("layers");
    DASSERT(layers->isArray());
    for(uint32_t i = 0; i < layers->size(); ++i)
    {
        const sp<Json> layer = layers->at(static_cast<int32_t>(i));
        DASSERT(layer);
        String name = layer->getString("name");
        uint32_t rowCount = static_cast<uint32_t>(layer->getInt("height"));
        uint32_t colCount = static_cast<uint32_t>(layer->getInt("width"));

        const sp<TilemapLayer> tilemapLayer = makeLayer(std::move(name), rowCount, colCount);
        const sp<Json> data = layer->get("data");
        DASSERT(data);
        DASSERT(data->isArray());
        uint32_t row = 1;
        uint32_t col = 0;
        data->foreach([&tilemapLayer, &row, &col, rowCount, colCount](const Json& idx) {
            int32_t type = idx.toInt();
            if(type > 0) {
                tilemapLayer->setTile(rowCount - row, col, type - 1);
            }
            if(++col == colCount) {
                col = 0;
                ++row;
            }
            return true;
        });
    }
}

Json Tilemap::jsonDump() const
{
    Json jLayers;

    for(const sp<TilemapLayer>& i : layers())
    {
        Json jLayer;
        uint32_t rowCount = i->rowCount();
        uint32_t colCount = i->colCount();
        jLayer.setString("name", i->name());
        jLayer.setInt("width", static_cast<int32_t>(colCount));
        jLayer.setInt("height", static_cast<int32_t>(rowCount));

        std::vector<int32_t> tiles(colCount * rowCount, 0);
        i->foreachTile([&tiles, rowCount, colCount] (uint32_t row, uint32_t col, const sp<Tile>& tile) {
            tiles[(rowCount - 1 - row) * colCount + col] = tile->id() + 1;
            return true;
        });
        jLayer.set("data", sp<Json>::make(sp<IntArray>::make<IntArray::Vector>(std::move(tiles))));

        jLayers.append(jLayer);
    }

    Json jTilemap;
    jTilemap.set("layers", jLayers);
    return jTilemap;
}

const std::list<sp<TilemapLayer>>& Tilemap::layers() const
{
    return _stub->_layers;
}

Tilemap::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _layer(factory.getBuilder<Layer>(manifest, Constants::Attributes::LAYER)), _size(factory.ensureConcreteClassBuilder<Size>(manifest, Constants::Attributes::SIZE)),
      _tileset(factory.ensureBuilder<Tileset>(manifest, "tileset")), _importer(factory.getBuilder<Importer<Tilemap>>(manifest, "importer")), _outputer(factory.getBuilder<Outputer<Tilemap>>(manifest, "outputer")),
      _scrollable(factory.getBuilder<Scrollable>(manifest, "scrollable"))
{
}

sp<Tilemap> Tilemap::BUILDER::build(const Scope& args)
{
    sp<Layer> layer = _layer->build(args);
    sp<Tilemap> tilemap = sp<Tilemap>::make(layer ? layer->context() : nullptr, _size->build(args), _tileset->build(args), _importer->build(args), _outputer->build(args));
    if(_scrollable)
    {
        sp<Scrollable> scrollable = _scrollable->build(args);
        scrollable->setRendererMaker(sp<TilemapLayerMaker>::make(tilemap, scrollable->rendererMaker()));
        tilemap->_stub->_scrollable = std::move(scrollable);
    }
    return tilemap;
}

Tilemap::TilemapRenderer::TilemapRenderer(sp<LayerContext> layerContext, sp<Stub> stub)
    : _layer_context(std::move(layerContext)), _stub(std::move(stub))
{
}

void Tilemap::TilemapRenderer::render(RenderRequest& renderRequest, const V3& position)
{
    _layer_context->renderRequest(position);

    if(_stub->_scrollable)
        _stub->_scrollable->render(renderRequest, V3(0));

    for(const sp<TilemapLayer>& i : _stub->_layers)
        if(!(i->flag() & LAYER_FLAG_INVISIBLE))
            i->render(renderRequest, V3());
}

}
