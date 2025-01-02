#include "graphics/base/tile.h"

#include "graphics/components/render_object.h"

namespace ark {

Tile::Tile(int32_t id, String type, int32_t shapeId, sp<RenderObject> renderObject)
    : _id(id), _shape_id(shapeId), _type(std::move(type)), _render_object(std::move(renderObject))
{
}

int32_t Tile::id() const
{
    return _id;
}

const String& Tile::type() const
{
    return _type;
}

void Tile::setType(String type)
{
    _type = std::move(type);
}

int32_t Tile::shapeId() const
{
    return _shape_id;
}

void Tile::setShapeId(int32_t type)
{
    _shape_id = type;
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
