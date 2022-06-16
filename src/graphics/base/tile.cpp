#include "graphics/base/tile.h"

#include "graphics/base/render_object.h"

namespace ark {

Tile::Tile(int32_t id, String type, int32_t shapeId, uint32_t width, uint32_t height, int32_t renderObjectId)
    : _id(id), _shape_id(shapeId), _type(std::move(type)), _width(width), _height(height), _render_object_id(renderObjectId)
{
}

Tile::Tile(int32_t id, String type, int32_t shapeId, uint32_t width, uint32_t height, sp<RenderObject> renderObject)
    : _id(id), _shape_id(shapeId), _type(std::move(type)), _width(width), _height(height), _render_object_id(renderObject->type()->val()), _render_object(std::move(renderObject))
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

uint32_t Tile::width() const
{
    return _width;
}

uint32_t Tile::height() const
{
    return _height;
}

const sp<RenderObject>& Tile::renderObject() const
{
    return _render_object;
}

void Tile::setRenderObject(sp<RenderObject> renderObject)
{
    _render_object = std::move(renderObject);
}

const sp<RenderObject>& Tile::ensureRenderObject()
{
    if(!_render_object)
        _render_object = sp<RenderObject>::make(_render_object_id);
    return _render_object;
}

}
