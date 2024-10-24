#include "graphics/base/tilemap.h"

#include <algorithm>

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/base/json.h"
#include "core/inf/storage.h"
#include "core/inf/variable.h"
#include "core/util/math.h"

#include "graphics/base/layer_context.h"
#include "graphics/base/tile.h"
#include "graphics/base/tilemap_layer.h"
#include "graphics/base/tileset.h"
#include "graphics/base/v2.h"
#include "graphics/inf/tile_maker.h"

#include "app/base/collision_filter.h"
#include "app/base/a_star.h"

namespace ark {

namespace {

typedef std::pair<int32_t, int32_t> Coordinate;

size_t getCoordinateHasher(int32_t col, int32_t row) {
    std::size_t seed = 0;
    Math::hashCombine(seed, col);
    Math::hashCombine(seed, row);
    return seed;
}

struct CoordinateHasher {
    size_t operator()(const Coordinate& str) const {
        return getCoordinateHasher(str.first, str.second);
    }
};

struct SearchingTilemap {
    SearchingTilemap(Tilemap& tilemap)
        : _tilemap(tilemap) {
    }

    bool isBlockage(int32_t col, int32_t row) {
        const auto iter = _blockages.find({col, row});
        if(iter != _blockages.end())
            return iter->second;

        for(TilemapLayer& i : _tilemap.layers())
            if(i.collisionFilter()) {
                const V3 layerPosition = i.position().val();
                int32_t dcol = col - static_cast<int32_t>(layerPosition.x());
                int32_t drow = row - static_cast<int32_t>(layerPosition.y());
                if(dcol >= 0 && drow >= 0 && dcol < i.colCount() && drow < i.rowCount()) {
                    if(static_cast<bool>(i.getTile(dcol, drow))) {
                        _blockages.insert(std::make_pair(Coordinate{col, row}, true));
                        return true;
                    }
            }
        }
        _blockages.insert(std::make_pair(Coordinate{col, row}, false));
        return false;
    }

    Tilemap& _tilemap;
    std::unordered_map<Coordinate, bool, CoordinateHasher> _blockages;
};

struct SearchingTilemapNode {
    SearchingTilemapNode(SearchingTilemap& searchingTilemap, int32_t col, int32_t row)
        : _searching_tilemap(searchingTilemap), _col(col), _row(row) {
    }

    V3 position() const {
        return V3(static_cast<float>(_col), static_cast<float>(_row), 0);
    }

    void visitAdjacentNodes(const std::function<void(SearchingTilemapNode, float)>& visitor) {
        for(int32_t i = -1; i < 2; ++i)
            for(int32_t j = -1; j < 2; ++j)
                if((i != 0 || j != 0) && !_searching_tilemap.isBlockage(_col + i, _row + j))
                    visitor(SearchingTilemapNode(_searching_tilemap, _col + i, _row + j), std::abs(i) + std::abs(j));
    }

    bool operator == (const SearchingTilemapNode& other) const {
        return _col == other._col && _row == other._row;
    }

    bool operator != (const SearchingTilemapNode& other) const {
        return _col != other._col || _row != other._row;
    }

    SearchingTilemap& _searching_tilemap;
    int32_t _col;
    int32_t _row;
};

struct SearchingTilemapNodeHasher {
    size_t operator()(const SearchingTilemapNode& str) const {
        return getCoordinateHasher(str._col, str._row);
    }
};

class TilemapLayerMaker : public RendererMaker {
public:
    TilemapLayerMaker(Tilemap& tilemap, sp<RendererMaker> delegate)
        : _tilemap(tilemap), _delegate(std::move(delegate)) {
    }

    virtual std::vector<Box> make(float x, float y) override {
        std::vector<Box> renderers = _delegate->make(x, y);
        for(const Box& i : renderers) {
            sp<TilemapLayer> tilemapLayer = i.as<TilemapLayer>();
            CHECK_WARN(tilemapLayer, "Tilemap's RendererMaker should return TilemapLayer instance, others will be ignored");
            if(tilemapLayer)
                _tilemap.addLayer(tilemapLayer);
        }
        return renderers;
    }

    virtual void recycle(const Box& renderer) override {
        sp<TilemapLayer> tilemapLayer = renderer.as<TilemapLayer>();
        CHECK_WARN(tilemapLayer, "Tilemap's RendererMaker should return TilemapLayer instance, others will be ignored");
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

static bool _tilemapLayerCompareLess(const TilemapLayer& a, const TilemapLayer& b)
{
    return a.zorder() < b.zorder();
}

static bool _tilemapLayerCompareGreater(const TilemapLayer& a, const TilemapLayer& b)
{
    return a.zorder() > b.zorder();
}

static sp<CollisionFilter> toCollisionFilter(sp<Json> jCollisionFilter)
{
    if(!jCollisionFilter)
        return nullptr;

    uint32_t categoryBits = static_cast<uint32_t>(jCollisionFilter->getInt("category", 1));
    uint32_t maskBits = static_cast<uint32_t>(jCollisionFilter->getInt("mask", -1));
    uint32_t groupIndex = static_cast<uint32_t>(jCollisionFilter->getInt("group_index", 0));
    return sp<CollisionFilter>::make(categoryBits, maskBits, groupIndex);
}

Tilemap::Tilemap(sp<Tileset> tileset, sp<RenderLayer> renderLayer, sp<Importer<Tilemap>> importer, sp<Outputer<Tilemap>> outputer)
    : _render_layer(std::move(renderLayer)), _tileset(std::move(tileset)), _storage(sp<TilemapStorage>::make(*this, std::move(importer), std::move(outputer))),
      _stub(sp<Stub>::make())/*, _layer_context(_render_layer ? _render_layer->addLayerContext(_stub, nullptr, nullptr) : nullptr)*/
{
    if(_render_layer)
        _render_layer->addRenderBatch(_stub);
}

void Tilemap::clear()
{
    _stub->_layers.clear();
}

const sp<Tileset>& Tilemap::tileset() const
{
    return _tileset;
}

const sp<RenderLayer>& Tilemap::renderLayer() const
{
    return _render_layer;
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

sp<TilemapLayer> Tilemap::makeLayer(const String& name, uint32_t colCount, uint32_t rowCount, sp<Vec3> position, sp<Boolean> visible, sp<CollisionFilter> collisionFilter, float zorder)
{
    sp<TilemapLayer> layer = sp<TilemapLayer>::make(_tileset, name, colCount, rowCount, position, std::move(visible), std::move(collisionFilter));
    addLayer(layer, zorder);
    return layer;
}

void Tilemap::addLayer(sp<TilemapLayer> layer, float zorder)
{
    if(_render_layer)
        layer->setLayerContext(_render_layer->makeLayerContext(nullptr, layer->position().wrapped(), layer->visible().wrapped(), nullptr));
    layer->_stub->_zorder = zorder;
    _stub->_layers.insert(std::upper_bound(_stub->_layers.begin(), _stub->_layers.end(), layer, _tilemapLayerCompareGreater), std::move(layer));
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
        uint32_t colCount = static_cast<uint32_t>(layer->getInt("width"));
        uint32_t rowCount = static_cast<uint32_t>(layer->getInt("height"));
        float x = layer->getFloat("x", 0);
        float y = layer->getFloat("y", 0);
        float z = layer->getFloat("z", 0);
        sp<Json> jCollisionFilter = layer->get("collision_filter");

        const sp<TilemapLayer> tilemapLayer = makeLayer(std::move(name), colCount, rowCount, sp<Vec3::Const>::make(V3(x, y, z)), nullptr, toCollisionFilter(jCollisionFilter));
        const sp<Json> data = layer->get("data");
        DASSERT(data);
        DASSERT(data->isArray());
        uint32_t row = 1;
        uint32_t col = 0;
        data->foreach([&tilemapLayer, &row, &col, rowCount, colCount](const Json& idx) {
            int32_t type = idx.toInt();
            if(type > 0) {
                tilemapLayer->setTile(col, rowCount - row, type - 1);
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
        const V3 position = i->position().val();
        jLayer.setString("name", i->name());
        jLayer.setInt("width", static_cast<int32_t>(colCount));
        jLayer.setInt("height", static_cast<int32_t>(rowCount));
        jLayer.setFloat("x", position.x());
        jLayer.setFloat("y", position.y());
        jLayer.setFloat("z", position.z());

        const sp<CollisionFilter>& collisionFilter = i->collisionFilter();
        if(collisionFilter)
        {
            Json jCollisionFilter;
            jCollisionFilter.setInt("category", static_cast<int32_t>(collisionFilter->categoryBits()));
            jCollisionFilter.setInt("mask", static_cast<int32_t>(collisionFilter->maskBits()));
            jCollisionFilter.setInt("group_index", static_cast<int32_t>(collisionFilter->groupIndex()));
            jLayer.set("collision_filter", jCollisionFilter);
        }

        std::vector<int32_t> tiles(colCount * rowCount, 0);
        i->foreachTile([&tiles, rowCount, colCount] (uint32_t col, uint32_t row, const sp<Tile>& tile) {
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

std::vector<std::array<int32_t, 2>> Tilemap::findRoute(const std::array<int32_t, 2>& start, const std::array<int32_t, 2>& goal)
{
    std::vector<std::array<int32_t, 2>> result;
    SearchingTilemap searchingTilemap(*this);
    AStar<SearchingTilemapNode, SearchingTilemapNodeHasher> astar(SearchingTilemapNode(searchingTilemap, start[0], start[1]), SearchingTilemapNode(searchingTilemap, goal[0], goal[1]));
    for(const SearchingTilemapNode& i : astar.findRoute())
        result.push_back({i._col, i._row});
    return result;
}

const std::vector<sp<TilemapLayer>>& Tilemap::layers() const
{
    return _stub->_layers;
}

Tilemap::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _tileset(factory.ensureBuilder<Tileset>(manifest, "tileset")), _render_layer(factory.getBuilder<RenderLayer>(manifest, constants::RENDER_LAYER)),
      _importer(factory.getBuilder<Importer<Tilemap>>(manifest, "importer")), _outputer(factory.getBuilder<Outputer<Tilemap>>(manifest, "outputer"))
{
}

sp<Tilemap> Tilemap::BUILDER::build(const Scope& args)
{
    sp<Tilemap> tilemap = sp<Tilemap>::make(_tileset->build(args), _render_layer->build(args), _importer->build(args), _outputer->build(args));
    return tilemap;
}

std::vector<sp<LayerContext>>& Tilemap::Stub::snapshot(const RenderRequest& renderRequest)
{
    _layer_contexts.clear();
    for(TilemapLayer& i : _layers)
        if(i._layer_context)
            _layer_contexts.push_back(i._layer_context);
    return _layer_contexts;
}

}
