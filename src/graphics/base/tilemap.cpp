#include "graphics/base/tilemap.h"

#include <algorithm>

#include "core/ark.h"

#include "core/inf/importer.h"
#include "core/inf/variable.h"
#include "core/util/math.h"

#include "graphics/base/layer_context.h"
#include "graphics/base/size.h"
#include "graphics/base/tileset.h"
#include "graphics/base/v2.h"

namespace ark {

Tilemap::Tilemap(const sp<LayerContext>& layerContext, uint32_t width, uint32_t height, const sp<Tileset>& tileset, const sp<TilemapImporter>& importer)
    : _layer_context(layerContext), _size(sp<Size>::make(static_cast<float>(width), static_cast<float>(height))), _tileset(tileset), _importer(importer),
      _col_count(width / _tileset->tileWidth()), _row_count(height / _tileset->tileHeight())
{
    DASSERT(_layer_context);
    _tiles = new sp<RenderObject>[_col_count * _row_count];
}

Tilemap::~Tilemap()
{
    delete[] _tiles;
}

void Tilemap::render(RenderRequest& /*renderRequest*/, float x, float y)
{
    const V position = _position->val();
    const V scroll = _scroller->val();
    float sx = scroll.x() - position.x(), sy = scroll.y() - position.y();
    float fx, cx, fy, cy, tileWidth = static_cast<float>(_tileset->tileWidth()), tileHeight = static_cast<float>(_tileset->tileHeight());
    Math::modBetween(sx, sx + _size->width(), tileWidth, fx, cx);
    Math::modBetween(sy, sy + _size->height(), tileHeight, fy, cy);
    int32_t rowIdStart = static_cast<int32_t>(fy / tileHeight);
    int32_t colIdStart = static_cast<int32_t>(fx / tileWidth);
    int32_t rowIdEnd = std::min<int32_t>(static_cast<int32_t>(_row_count), static_cast<int32_t>(cy / tileHeight));
    int32_t colIdEnd = std::min<int32_t>(static_cast<int32_t>(_col_count), static_cast<int32_t>(cx / tileWidth));

    _layer_context->renderRequest(V2(x, y));

    float ox = sx - fx - tileWidth / 2.0f, oy = sy - fy - tileHeight / 2.0f;
    for(int32_t i = rowIdStart; i < rowIdEnd; i++)
    {
        if(i >= 0)
        {
            float dy = (i - rowIdStart) * tileHeight - oy;
            for(int32_t j = colIdStart; j < colIdEnd; j++)
                if(j >= 0)
                {
                    const sp<RenderObject>& renderObject = _tiles[i * _col_count + j];
                    if(renderObject)
                        _layer_context->draw((j - colIdStart) * tileWidth - ox, dy, renderObject);
                }
        }
    }
}

const SafePtr<Size>& Tilemap::size()
{
    return _size;
}

const sp<RenderObject>& Tilemap::getTile(uint32_t rowId, uint32_t colId) const
{
    DCHECK(rowId < _row_count && colId < _col_count, "Invaild tile id:(%d, %d), tile map size(%d, %d)", rowId, colId, _row_count, _col_count);
    return _tiles[rowId * _col_count + colId];
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
    DCHECK(rowId < _row_count && colId < _col_count, "Invaild tile id:(%d, %d), tile map size(%d, %d)", rowId, colId, _row_count, _col_count);
    _tiles[rowId * _col_count + colId] = renderObject;
}

void Tilemap::setTile(uint32_t rowId, uint32_t colId, int32_t tileId)
{
    const sp<RenderObject>& tile = _tileset->getTile(tileId);
    DCHECK(tile, "TileId %d does not exist", tileId);
    setTile(rowId, colId, tile);
}

void Tilemap::clear()
{
    delete[] _tiles;
    _tiles = new sp<RenderObject>[_col_count * _row_count];
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

const sp<Vec>& Tilemap::position() const
{
    return _position;
}

void Tilemap::setPosition(const sp<Vec>& position)
{
    _position = position;
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
