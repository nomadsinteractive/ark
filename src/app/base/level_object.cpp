#include "app/base/level_object.h"

#include "core/base/constants.h"
#include "core/base/named_hash.h"
#include "core/types/global.h"
#include "core/util/documents.h"

#include "graphics/base/transform_3d.h"
#include "graphics/components/render_object.h"
#include "graphics/util/vec3_type.h"
#include "graphics/util/vec4_type.h"

#include "app/base/level_library.h"
#include "app/inf/collider.h"

namespace ark {

namespace {

sp<Transform3D> makeTransform(const Optional<V4>& rotation, const Optional<V3>& scale)
{
    sp<Vec3> s;
    const V4 quat = rotation ? rotation.value() : constants::QUATERNION_ONE;
    if(scale)
        if(const V3 sv = scale.value(); sv != constants::SCALE_ONE)
            s = sp<Vec3>::make<Vec3::Const>(sv);
    return sp<Transform3D>::make(sp<Vec4>::make<Vec4::Const>(quat), std::move(s));
}

std::pair<sp<RenderObject>, sp<Transform3D>> makeRenderObject(const LevelObject& obj, HashId type)
{
    const Global<Constants> globalConstants;
    sp<Transform3D> transform = makeTransform(obj.rotation(), obj.scale());
    sp<RenderObject> renderObject = sp<RenderObject>::make(type, sp<Vec3>::make<Vec3::Const>(obj.position()), nullptr, transform, nullptr, obj.visible() ? globalConstants->BOOLEAN_TRUE : globalConstants->BOOLEAN_FALSE);
    return {std::move(renderObject), std::move(transform)};
}

sp<Rigidbody> makeRigidBody(LevelLibrary& library, const sp<Collider>& collider, const LevelObject& obj, const Rigidbody::BodyType bodyType, const sp<CollisionFilter>& collisionFilter)
{
    if(!collider)
        return nullptr;

    if(!library.shape())
        library.setShape(collider->createShape(library.name()));

    const sp<Transform3D> transform = makeTransform(obj.rotation(), obj.scale());
    if(bodyType != Rigidbody::BODY_TYPE_DYNAMIC)
        return Collider::createBody(collider, bodyType, library.shape(), obj.renderObject()->position(), transform->rotation().wrapped(), collisionFilter);

    sp<Rigidbody> rigidbody = Collider::createBody(collider, bodyType, library.shape(), Vec3Type::freeze(obj.renderObject()->position()), Vec4Type::freeze(transform->rotation().wrapped()), collisionFilter);
    obj.renderObject()->setPosition(rigidbody->position().wrapped());
    transform->setRotation(rigidbody->rotation().wrapped());
    return rigidbody;
}

}

LevelObject::LevelObject(const sp<Level::Stub>& level, const document& manifest)
    : _level(level), _name(Documents::getAttribute(manifest, constants::NAME)), _type(TYPE_INSTANCE), _visible(Documents::getAttribute<bool>(manifest, constants::VISIBLE, true)),
      _position(Documents::getAttribute<V3>(manifest, constants::POSITION, V3())), _scale(Documents::getAttributeOptional<V3>(manifest, constants::SCALE)),
      _rotation(Documents::getAttributeOptional<V4>(manifest, constants::ROTATION)), _args(Documents::getAttribute(manifest, "args")), _instance_of(Documents::getAttribute<int32_t>(manifest, "instance-of", -1))
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

const String& LevelObject::args() const
{
    return _args;
}

sp<LevelLibrary> LevelObject::library() const
{
    const auto iter = _level->_libraries.find(_instance_of);
    return iter != _level->_libraries.end() ? iter->second : nullptr;
}

sp<RenderObject> LevelObject::createRenderObject()
{
    if(_instance_of != -1)
    {
        auto [renderObject, transform] = makeRenderObject(*this, ensureLibrary()->_name.hash());
        _render_object = std::move(renderObject);
    }
    else if(_type == TYPE_ELEMENT)
    {
        auto [renderObject, _] = makeRenderObject(*this, _name.hash());
        _render_object = std::move(renderObject);
    }
    return _render_object;
}

sp<Rigidbody> LevelObject::createRigidbody(const sp<Collider>& collider, const Rigidbody::BodyType bodyType, const sp<CollisionFilter>& collisionFilter)
{
    if(_instance_of != -1)
        _rigidbody = makeRigidBody(ensureLibrary(), collider, *this, bodyType, collisionFilter);
    return _rigidbody;
}

const sp<LevelLibrary>& LevelObject::ensureLibrary() const
{
    const auto iter = _level->_libraries.find(_instance_of);
    ASSERT(iter != _level->_libraries.end());
    return iter->second;
}

const sp<RenderObject>& LevelObject::renderObject() const
{
    return _render_object;
}

const sp<Rigidbody>& LevelObject::rigidbody() const
{
    return _rigidbody;
}

}
