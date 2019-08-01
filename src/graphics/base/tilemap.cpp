#include "graphics/base/tilemap.h"

#include <algorithm>

#include "core/ark.h"

#include "core/inf/importer.h"
#include "core/inf/variable.h"
#include "core/util/math.h"

#include "graphics/base/layer_context.h"
#include "graphics/base/size.h"
#include "graphics/base/tilemap_layer.h"
#include "graphics/base/tileset.h"
#include "graphics/base/v2.h"

namespace ark {

Tilemap::Tilemap(const sp<LayerContext>& layerContext, uint32_t width, uint32_t height, const sp<Tileset>& tileset, const sp<TilemapImporter>& importer)
    : _layer_context(layerContext), _size(sp<Size>::make(static_cast<float>(width), static_cast<float>(height))), _tileset(tileset), _importer(importer),
      _col_count(width / _tileset->tileWidth()), _row_count(height / _tileset->tileHeight())
{
    DASSERT(_layer_context);
    makeLayer();
}

void Tilemap::render(RenderRequest& /*renderRequest*/, float x, float y)
{
    _layer_context->renderRequest(V2(x, y));

    const V scroll = _scroller->val();
    for(const sp<TilemapLayer>& i : _layers)
        i->render(_layer_context, scroll, _size->width(), _size->height());
}

const SafePtr<Size>& Tilemap::size()
{
    return _size;
}

const sp<RenderObject>& Tilemap::getTile(uint32_t rowId, uint32_t colId) const
{
    DCHECK(rowId < _row_count && colId < _col_count, "Invaild tile id:(%d, %d), tile map size(%d, %d)", rowId, colId, _row_count, _col_count);
    return _layers.back()->_tiles[rowId * _col_count + colId];
}

int32_t Tilemap::getTileType(uint32_t rowId, uint32_t colId) const
{
    const sp<RenderObject>& renderObject = getTile(rowId, colId);
    return renderObject ? renderObject->type()->val() : -1;
}

const sp<RenderObject>& Tilemap::getTileByPosition(float x, float y) const
{
    return getTile(static_cast<uint32_t>(y / _tileset->tileHeight()), static_cast<uint32_t>(x / _tileset->tileWidth()));
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

uint32_t Tilemap::colCount() const
{
    return _col_count;
}

uint32_t Tilemap::rowCount() const
{
    return _row_count;
}

const sp<Vec>& Tilemap::scroller() const
{
    return _scroller;
}

void Tilemap::setScroller(const sp<Vec>& scroller)
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

sp<TilemapLayer> Tilemap::makeLayer(uint32_t rowCount, uint32_t colCount, const sp<Tileset>& tileset, const sp<Vec>& position, Tilemap::LayerFlag layerFlag)
{
    sp<TilemapLayer> layer = sp<TilemapLayer>::make(rowCount ? rowCount : _row_count, colCount ? colCount : _col_count, tileset ? tileset : _tileset, position, layerFlag);
    _layers.push_back(layer);
    return layer;
}

void Tilemap::addLayer(const sp<TilemapLayer>& layer)
{
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
    : _layer_context(sp<LayerContext::BUILDER>::make(factory, manifest, true)), _width(factory.ensureBuilder<Integer>(manifest, Constants::Attributes::WIDTH)),
      _height(factory.ensureBuilder<Integer>(manifest, Constants::Attributes::HEIGHT)), _tileset(factory.ensureBuilder<Tileset>(manifest, "tileset")),
      _importer(factory.getBuilder<TilemapImporter>(manifest, "importer"))
{
}

sp<Tilemap> Tilemap::BUILDER::build(const sp<Scope>& args)
{
    return sp<Tilemap>::make(_layer_context->build(args), _width->build(args)->val(), _height->build(args)->val(), _tileset->build(args), _importer->build(args));
}

}
