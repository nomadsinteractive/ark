#include "app/base/level_object.h"

#include "core/util/documents.h"

namespace ark {

LevelObject::LevelObject(const document& manifest)
    : _name(Documents::getAttribute(manifest, constants::NAME)), _type(TYPE_INSTANCE), _visible(Documents::getAttribute<bool>(manifest, constants::VISIBLE, true)),
      _position(Documents::getAttribute<V3>(manifest, constants::POSITION, V3())), _scale(Documents::getAttributeOptional<V3>(manifest, constants::SCALE)),
      _rotation(Documents::getAttributeOptional<V4>(manifest, constants::ROTATION)), _instance_of(Documents::getAttribute<int32_t>(manifest, "instance-of", -1))
{
    if(const String clazz = Documents::getAttribute(manifest, constants::CLASS); clazz == "MESH")
        _type = TYPE_ELEMENT;
    else if(clazz == "CAMERA")
        _type = TYPE_CAMERA;
    else if(clazz == "LIGHT")
        _type = TYPE_LIGHT;
}

const String& LevelObject::name() const
{
    return _name;
}

LevelObject::Type LevelObject::type() const
{
    return _type;
}

bool LevelObject::visible() const
{
    return _visible;
}

const V3& LevelObject::position() const
{
    return _position;
}

const Optional<V3>& LevelObject::scale() const
{
    return _scale;
}

const Optional<V4>& LevelObject::rotation() const
{
    return _rotation;
}

int32_t LevelObject::instanceOf() const
{
    return _instance_of;
}

const sp<Entity>& LevelObject::entity() const
{
    return _entity;
}

void LevelObject::setEntity(sp<Entity> entity)
{
    _entity = std::move(entity);
}

const sp<RenderObject>& LevelObject::renderObject() const
{
    return _render_object;
}

}
