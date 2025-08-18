#include "graphics/base/tile.h"

#include "graphics/components/render_object.h"

namespace ark {

Tile::Tile(int32_t id, const NamedHash& type, sp<Shape> shape, sp<RenderObject> renderObject)
    : _id(id), _type(std::move(type)), _shape(std::move(shape)), _render_object(std::move(renderObject))
{
}

int32_t Tile::id() const
{
    return _id;
}

NamedHash Tile::type() const
{
    return _type;
}

void Tile::setType(const NamedHash& type)
{
    _type = std::move(type);
}

const sp<Shape>& Tile::shape() const
{
    return _shape;
}

void Tile::setShape(sp<Shape> shape)
{
    _shape = std::move(shape);
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
