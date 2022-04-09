#include "graphics/base/tile.h"

namespace ark {

Tile::Tile(int32_t id, int32_t type, sp<RenderObject> renderObject)
    : _id(id), _type(type), _render_object(std::move(renderObject))
{
}

int32_t Tile::id() const
{
    return _id;
}

int32_t Tile::type() const
{
    return _type;
}

void Tile::setType(int32_t type)
{
    _type = type;
}

const sp<RenderObject>& Tile::renderObject() const
{
    return _render_object;
}

void Tile::setRenderObject(sp<RenderObject> renderObject)
{
    _render_object = std::move(renderObject);
}

}
