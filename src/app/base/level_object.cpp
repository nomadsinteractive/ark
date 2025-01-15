#include "app/base/level_object.h"

#include "core/util/documents.h"

namespace ark {

LevelObject::LevelObject(document manifest): _manifest(std::move(manifest)), _name(Documents::getAttribute(_manifest, constants::NAME)), _type(TYPE_INSTANCE), _visible(Documents::getAttribute<bool>(_manifest, constants::VISIBLE, true)),
                                             _position(Documents::getAttribute<V3>(_manifest, constants::POSITION, V3())), _scale(Documents::getAttributeOptional<V3>(_manifest, constants::SCALE)),
                                             _rotation(Documents::getAttributeOptional<V4>(_manifest, constants::ROTATION)), _body_type(Documents::getAttribute<Rigidbody::BodyType>(_manifest, "rigidbody_type", Rigidbody::BODY_TYPE_NONE)),
                                             _instance_of(Documents::getAttribute<int32_t>(_manifest, "instance-of", -1))
{
    if(const String clazz = Documents::getAttribute(_manifest, constants::CLASS); clazz == "MESH")
        _type = TYPE_MESH;
    else if(clazz == "CAMERA")
        _type = TYPE_CAMERA;
    else if(clazz == "LIGHT")
        _type = TYPE_LIGHT;
}

const document& LevelObject::manifest() const
{
    return _manifest;
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
