#include "app/base/level_layer.h"

#include <bx/typetraits.h>

#include "core/base/named_hash.h"
#include "core/types/global.h"

#include "graphics/base/transform_3d.h"
#include "graphics/components/layer.h"
#include "graphics/components/render_object.h"
#include "graphics/util/vec3_type.h"
#include "graphics/util/vec4_type.h"

#include "app/base/level_library.h"
#include "app/base/level_object.h"
#include "app/inf/collider.h"
#include "app/util/collider_type.h"

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

sp<Rigidbody> makeRigidBody(LevelLibrary& library, const sp<Collider>& collider, const LevelObject& obj, Rigidbody::BodyType bodyType, const Map<String, sp<Shape>>& shapes, const sp<CollisionFilter>& collisionFilter)
{
    if(!collider)
        return nullptr;

    if(!library.shape())
    {
        if(const auto iter = shapes.find(library.name()); iter != shapes.end())
            library.setShape(iter->second);
        else
            library.setShape(collider->createShape(library.name(), library.size()));
    }

    const sp<Transform3D> transform = makeTransform(obj.rotation(), obj.scale());
    if(bodyType != Rigidbody::BODY_TYPE_DYNAMIC)
        return ColliderType::createBody(collider, bodyType, library.shape(), obj.renderObject()->position(), transform->rotation().wrapped(), collisionFilter);

    sp<Rigidbody> rigidbody = ColliderType::createBody(collider, bodyType, library.shape(), Vec3Type::freeze(obj.renderObject()->position()), Vec4Type::freeze(transform->rotation().wrapped()), collisionFilter);
    obj.renderObject()->setPosition(rigidbody->position().wrapped());
    transform->setRotation(rigidbody->rotation().wrapped());
    return rigidbody;
}

}

LevelLayer::LevelLayer(sp<Map<int32_t, sp<LevelLibrary>>> libraries, String name, Vector<sp<LevelObject>> objects)
    : _libraries(std::move(libraries)), _name(std::move(name)), _objects(std::move(objects))
{
    for(const sp<LevelObject>& i : _objects)
        if(i->name())
            _objects_by_name.emplace(i->name(), i);
}

const String& LevelLayer::name() const
{
    return _name;
}

const Vector<sp<LevelObject>>& LevelLayer::objects() const
{
    return _objects;
}

sp<LevelObject> LevelLayer::getObject(StringView name) const
{
    const auto iter = _objects_by_name.find(name);
    return iter != _objects_by_name.end() ? iter->second : nullptr;
}

void LevelLayer::createRenderObjects(Layer& layer) const
{
    for(const sp<LevelObject>& i : _objects)
        if(LevelObject& obj = *i; obj._instance_of != -1)
        {
            const auto iter = _libraries->find(obj._instance_of);
            ASSERT(iter != _libraries->end());
            const sp<LevelLibrary>& library = iter->second;
            auto [renderObject, transform] = makeRenderObject(obj, library->_name.hash());
            layer.addRenderObject(renderObject);
            obj._render_object = std::move(renderObject);
        }
        else if(obj._type == LevelObject::TYPE_ELEMENT)
        {
            auto [renderObject, _] = makeRenderObject(obj, obj._name.hash());
            layer.addRenderObject(renderObject);
            obj._render_object = std::move(renderObject);
        }
}

void LevelLayer::createRigidbodies(const sp<Collider>& collider, Rigidbody::BodyType bodyType, const Map<String, sp<Shape>>& shapes, const sp<CollisionFilter>& collisionFilter) const
{
    for(const sp<LevelObject>& i : _objects)
        if(LevelObject& obj = *i; obj._instance_of != -1)
        {
            const auto iter = _libraries->find(obj._instance_of);
            ASSERT(iter != _libraries->end());
            const sp<LevelLibrary>& library = iter->second;
            obj._rigidbody = makeRigidBody(library, collider, obj, bodyType, shapes, collisionFilter);
        }
}

}
