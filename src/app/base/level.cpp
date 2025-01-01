#include "app/base/level.h"

#include <map>

#include "core/ark.h"
#include "core/base/named_hash.h"
#include "core/types/global.h"

#include "graphics/base/camera.h"
#include "graphics/base/layer.h"
#include "graphics/traits/quaternion.h"
#include "graphics/base/render_object.h"
#include "graphics/base/transform_3d.h"
#include "graphics/util/vec3_type.h"
#include "graphics/util/vec4_type.h"

#include "renderer/base/render_engine.h"

#include "app/base/application_context.h"
#include "app/base/application_bundle.h"
#include "app/base/rigidbody.h"
#include "app/inf/collider.h"
#include "app/traits/shape.h"

namespace ark {

namespace {

struct Library {
    NamedHash _id;
    sp<Vec3> _size;
    sp<Shape> _shape;
};

enum ObjectClass {
    OBJECT_CLASS_INSTANCE,
    OBJECT_CLASS_MESH,
    OBJECT_CLASS_CAMERA,
    OBJECT_CLASS_LIGHT
};

struct Object {
    document _manifest;

    String _name;
    ObjectClass _class;

    bool _visible;

    V3 _position;
    Optional<V3> _scale;
    Optional<V4> _rotation;

    Collider::BodyType _body_type;
    int32_t _instance_of;

    Object(document manifest)
        : _manifest(std::move(manifest)), _name(Documents::getAttribute(_manifest, constants::NAME)), _class(OBJECT_CLASS_INSTANCE), _visible(Documents::getAttribute<bool>(_manifest, constants::VISIBLE, true)),
          _position(Documents::getAttribute<V3>(_manifest, constants::POSITION, V3())), _scale(Documents::getAttributeOptional<V3>(_manifest, constants::SCALE)),
          _rotation(Documents::getAttributeOptional<V4>(_manifest, constants::ROTATION)), _body_type(Documents::getAttribute<Collider::BodyType>(_manifest, "rigidbody_type", Collider::BODY_TYPE_NONE)),
          _instance_of(Documents::getAttribute<int32_t>(_manifest, "instance-of", -1))
    {
        const String clazz = Documents::getAttribute(_manifest, constants::CLASS);
        if(clazz == "MESH")
            _class = OBJECT_CLASS_MESH;
        else if(clazz == "CAMERA")
            _class = OBJECT_CLASS_CAMERA;
        else if(clazz == "LIGHT")
            _class = OBJECT_CLASS_LIGHT;
    }

};

sp<Transform3D> makeTransform(const Optional<V4>& rotation, const Optional<V3>& scale)
{
    sp<Vec3> s;
    const V4 quat = rotation ? rotation.value() : constants::QUATERNION_ONE;
    if(scale)
        if(const V3 sv = scale.value(); sv != constants::SCALE_ONE)
            s = sp<Vec3>::make<Vec3::Const>(sv);
    return sp<Transform3D>::make(sp<Vec4>::make<Vec4::Const>(quat), std::move(s));
}

std::pair<sp<RenderObject>, sp<Transform3D>> makeRenderObject(const Object& obj, HashId type)
{
    const Global<Constants> globalConstants;
    sp<Transform3D> transform = makeTransform(obj._rotation, obj._scale);
    sp<RenderObject> renderObject = sp<RenderObject>::make(type, sp<Vec3>::make<Vec3::Const>(obj._position), nullptr, transform, nullptr, obj._visible ? globalConstants->BOOLEAN_TRUE : globalConstants->BOOLEAN_FALSE);
    return {std::move(renderObject), std::move(transform)};
}

sp<Rigidbody> makeRigidBody(Library& library, const sp<Collider>& collider, RenderObject& renderObject, Transform3D& transform, Collider::BodyType bodyType, const std::map<String, sp<Shape>>& shapes)
{
    if(!collider)
        return nullptr;

    if(!library._shape)
    {
        if(const auto iter = shapes.find(library._id.name()); iter != shapes.end())
            library._shape = iter->second;
        else
            library._shape = collider->createShape(library._id, library._size);
    }
    if(bodyType != Collider::BODY_TYPE_DYNAMIC)
        return collider->createBody(bodyType, library._shape, renderObject.position(), transform.rotation().wrapped());

    sp<Rigidbody> rigidbody = collider->createBody(bodyType, library._shape, Vec3Type::freeze(renderObject.position()), Vec4Type::freeze(transform.rotation().wrapped()));
    renderObject.setPosition(rigidbody->position().wrapped());
    transform.setRotation(rigidbody->quaternion().wrapped());
    return rigidbody;
}

}

Level::Level(std::map<String, sp<Layer>> layers, std::map<String, sp<Camera>> cameras, std::map<String, sp<Vec3>> lights)
    : _layers(std::move(layers)), _cameras(std::move(cameras)), _lights(std::move(lights))
{
}

void Level::load(const String& src, const sp<Collider>& collider, const std::map<String, sp<Shape>>& shapes)
{
    const document manifest = Ark::instance().applicationContext()->applicationBundle()->loadDocument(src);
    CHECK(manifest, "Cannot load manifest \"%s\"", src.c_str());

    std::map<int32_t, Library> libraryMapping;
    for(const document& i : manifest->children("library"))
    {
        const String& name = Documents::ensureAttribute(i, constants::NAME);
        const String& dimensions = Documents::ensureAttribute(i, "dimensions");
        const int32_t id = Documents::ensureAttribute<int32_t>(i, constants::ID);
        CHECK_WARN(libraryMapping.find(id) == libraryMapping.end(), "Overwriting instance library mapping(%d)", id);
        libraryMapping.emplace(id, Library{name, sp<Vec3>::make<Vec3::Const>(Strings::eval<V3>(dimensions))});
    }

    for(const document& i : manifest->children(constants::LAYER))
    {
        const String layerName = Documents::getAttribute(i, constants::NAME);
        const sp<Layer> layer = getLayer(layerName);
        for(const document& j : i->children("object"))
        {
            const Object obj(j);
            if(obj._instance_of != -1)
            {
                const auto iter = libraryMapping.find(obj._instance_of);
                ASSERT(iter != libraryMapping.end());
                Library& library = iter->second;
                auto [renderObject, transform] = makeRenderObject(obj, library._id.hash());

                CHECK(layer, "Trying to load model instance into undefined Layer(%s)", layerName.c_str());
                layer->addRenderObject(renderObject);

                if(obj._body_type != Collider::BODY_TYPE_NONE)
                {
                    sp<Rigidbody> rigidBody = makeRigidBody(library, collider, renderObject, transform, obj._body_type, shapes);
                    if(obj._name)
                        _rigid_objects[obj._name] = std::move(rigidBody);
                    else
                        _unnamed_rigid_objects.push_back(std::move(rigidBody));
                }

                if(obj._name)
                    _render_objects[obj._name] = std::move(renderObject);
            }
            else if(obj._class == OBJECT_CLASS_MESH)
            {
                auto [renderObject, _] = makeRenderObject(obj, obj._name.hash());
                CHECK(layer, "Trying to load model instance into undefined Layer(%s)", layerName.c_str());
                layer->addRenderObject(renderObject);
                if(obj._name)
                    _render_objects[obj._name] = std::move(renderObject);
            }
            else if(obj._class == OBJECT_CLASS_CAMERA)
            {
                const sp<Camera> camera = getCamera(obj._name);
                DCHECK_WARN(camera, "Undefined camera(%s) in \"%s\"", obj._name.c_str(), src.c_str());
                if(camera)
                {
                    const float fovy = Documents::ensureAttribute<float>(obj._manifest, "fov_y");
                    const float clipNear = Documents::ensureAttribute<float>(obj._manifest, "clip-near");
                    const float clipFar = Documents::ensureAttribute<float>(obj._manifest, "clip-far");
                    const Quaternion quaternion(sp<Vec4>::make<Vec4::Const>(obj._rotation ? obj._rotation.value() : constants::QUATERNION_ONE));
                    const M4 matrix = quaternion.toMatrix()->val();
                    //After converting Blender coordinate system(RHS) from z-up to y-up, it becomes LHS coordinate system.
                    const V3 front = MatrixUtil::mul(matrix, V3(0, camera->isYUp() ? 1.0f : -1.0f, 0));
                    const V3 up = MatrixUtil::mul(matrix, V3(0, 0, 1.0f));
                    Camera c = Ark::instance().createCamera(Ark::COORDINATE_SYSTEM_LHS, Ark::instance().applicationContext()->renderEngine()->coordinateSystem() == Ark::COORDINATE_SYSTEM_LHS);
                    c.perspective(fovy, 16.0f / 9, clipNear, clipFar);
                    c.lookAt(obj._position, obj._position + front, up);
                    camera->assign(c);
                }
            }
            else if(obj._class == OBJECT_CLASS_LIGHT)
            {
                const sp<Vec3> light = getLight(obj._name);
                DCHECK_WARN(light, "Undefined light(%s) in \"%s\"", obj._name.c_str(), src.c_str());
                if(light)
                    Vec3Type::set(light, obj._position);
            }
        }
    }
}

sp<Layer> Level::getLayer(const String& name) const
{
    const auto iter = _layers.find(name);
    return iter != _layers.end() ? iter->second : nullptr;
}

sp<Camera> Level::getCamera(const String& name) const
{
    const auto iter = _cameras.find(name);
    return iter != _cameras.end() ? iter->second : nullptr;
}

sp<Vec3> Level::getLight(const String& name) const
{
    const auto iter = _lights.find(name);
    return iter != _lights.end() ? iter->second : nullptr;
}

sp<RenderObject> Level::getRenderObject(const String& name) const
{
    const auto iter = _render_objects.find(name);
    return iter != _render_objects.end() ? iter->second : nullptr;
}

sp<Rigidbody> Level::getRigidBody(const String& name) const
{
    const auto iter = _rigid_objects.find(name);
    return iter != _rigid_objects.end() ? iter->second : nullptr;
}

}
