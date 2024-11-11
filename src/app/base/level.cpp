#include "app/base/level.h"

#include <map>

#include "core/ark.h"

#include "core/base/named_hash.h"

#include "graphics/base/camera.h"
#include "graphics/base/layer.h"
#include "graphics/base/quaternion.h"
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
    String _name;
    int32_t _type;
    sp<Vec3> _dimensions;
    sp<Shape> _shape;
};

template<typename T> std::vector<Level::NamedLayerBuilder<T>> loadNamedTypes(BeanFactory& factory, const document& manifest, const String& name, const String& builderName) {
    std::vector<Level::NamedLayerBuilder<T>> namedTypes;
    for(const document& i : manifest->children(name))
        namedTypes.push_back({Documents::ensureAttribute(i, constants::NAME), factory.ensureBuilder<T>(i, builderName)});
    return namedTypes;
}

template<typename T> std::map<String, sp<T>> loadNamedTypeInstances(const std::vector<Level::NamedLayerBuilder<T>>& namedTypes, const Scope& args) {
    std::map<String, sp<T>> instances;
    for(const Level::NamedLayerBuilder<T>& i : namedTypes)
        instances[i._name] = i._builder->build(args);
    return instances;
}

sp<Transform3D> makeTransform(const String& rotation, const String& scale)
{
    sp<Vec3> s;
    const V4 quat = Strings::eval<V4>(rotation);
    if(scale)
        if(const V3 sv = Strings::eval<V3>(scale); sv != V3(1.0f))
            s = sp<Vec3>::make<Vec3::Const>(sv);
    return sp<Transform3D>::make(sp<Vec4>::make<Vec4::Const>(quat), std::move(s));
}

std::pair<sp<RenderObject>, sp<Transform3D>> makeRenderObject(const document& manifest, HashId type, bool visible)
{
    const Global<Constants> globalConstants;
    const V3 position = Documents::ensureAttribute<V3>(manifest, constants::POSITION);
    const String& scale = Documents::getAttribute(manifest, "scale");
    const String& rotation = Documents::ensureAttribute(manifest, constants::ROTATION);
    sp<Transform3D> transform = makeTransform(rotation, scale);
    sp<RenderObject> renderObject = sp<RenderObject>::make(type, sp<Vec3>::make<Vec3::Const>(position), nullptr, transform, nullptr, visible ? globalConstants->BOOLEAN_TRUE : globalConstants->BOOLEAN_FALSE);
    return {std::move(renderObject), std::move(transform)};
}

sp<Rigidbody> makeRigidBody(Library& library, const sp<Collider>& collider, RenderObject& renderObject, Transform3D& transform, Collider::BodyType bodyType, const std::map<String, sp<Shape>>& shapes)
{
    if(!collider)
        return nullptr;

    if(!library._shape)
        if(const auto iter = shapes.find(library._name); iter != shapes.end())
            library._shape = iter->second;

    if(bodyType != Collider::BODY_TYPE_DYNAMIC)
        return collider->createBody(bodyType, library._shape, renderObject.position(), transform.rotation().wrapped());

    sp<Rigidbody> rigidbody = collider->createBody(bodyType, library._shape, Vec3Type::freeze(renderObject.position()), Vec4Type::freeze(transform.rotation().wrapped()));
    renderObject.setPosition(rigidbody->position().wrapped());
    transform.setRotation(rigidbody->quaternion().wrapped());
    return rigidbody;
}

}

Level::Level(std::map<String, sp<Layer>> renderObjectLayers, std::map<String, sp<Camera>> cameras, std::map<String, sp<Vec3>> lights)
    : _render_object_layers(std::move(renderObjectLayers)), _cameras(std::move(cameras)), _lights(std::move(lights))
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
        libraryMapping.emplace(id, Library{name, static_cast<int32_t>(name.hash()), sp<Vec3>::make<Vec3::Const>(Strings::eval<V3>(dimensions))});
    }

    for(const document& i : manifest->children(constants::LAYER))
    {
        const String layerName = Documents::getAttribute(i, constants::NAME);
        const sp<Layer> layer = getLayer(layerName);
        for(const document& j : i->children("object"))
        {
            const int32_t instanceOf = Documents::getAttribute<int32_t>(j, "instance-of", -1);
            const String clazz = Documents::getAttribute(j, constants::CLASS);
            const bool visible = Documents::getAttribute<bool>(j, constants::VISIBLE, true);
            if(instanceOf != -1)
            {
                const auto iter = libraryMapping.find(instanceOf);
                ASSERT(iter != libraryMapping.end());
                Library& library = iter->second;
                String name = Documents::getAttribute(j, constants::NAME);
                auto [renderObject, transform] = makeRenderObject(j, library._type, visible);

                CHECK(layer, "Trying to load model instance into undefined Layer(%s)", layerName.c_str());
                layer->addRenderObject(renderObject);

                if(const Collider::BodyType bodyType = Documents::getAttribute<Collider::BodyType>(j, "rigidbody_type", Collider::BODY_TYPE_NONE); bodyType != Collider::BODY_TYPE_NONE)
                {
                    sp<Rigidbody> rigidBody = makeRigidBody(library, collider, renderObject, transform, bodyType, shapes);
                    if(name)
                        _rigid_objects[name] = std::move(rigidBody);
                    else
                        _unnamed_rigid_objects.push_back(std::move(rigidBody));
                }

                if(name)
                    _render_objects[name] = std::move(renderObject);
            }
            else if(clazz == "MESH")
            {
                const String& name = Documents::ensureAttribute(j, constants::NAME);
                auto [renderObject, _] = makeRenderObject(j, name.hash(), visible);
                CHECK(layer, "Trying to load model instance into undefined Layer(%s)", layerName.c_str());
                layer->addRenderObject(renderObject);
                if(name)
                    _render_objects[name] = std::move(renderObject);
            }
            else if(clazz == "CAMERA")
            {
                const String& name = Documents::ensureAttribute(j, constants::NAME);
                const sp<Camera> camera = getCamera(name);
                DCHECK_WARN(camera, "Undefined camera(%s) in \"%s\"", name.c_str(), src.c_str());
                if(camera)
                {
                    const String& position = Documents::ensureAttribute(j, constants::POSITION);
                    const String& rotation = Documents::ensureAttribute(j, constants::ROTATION);
                    const float fovy = Documents::ensureAttribute<float>(j, "fov_y");
                    const float clipNear = Documents::ensureAttribute<float>(j, "clip-near");
                    const float clipFar = Documents::ensureAttribute<float>(j, "clip-far");
                    const V3 p = Strings::eval<V3>(position);
                    const Quaternion quaternion(sp<Vec4>::make<Vec4::Const>(Strings::eval<V4>(rotation)));
                    const M4 matrix = quaternion.toMatrix()->val();
                    //After converting Blender coordinate system(RHS) from z-up to y-up, it becomes LHS coordinate system.
                    const V3 front = MatrixUtil::mul(matrix, V3(0, camera->isYUp() ? 1.0f : -1.0f, 0));
                    const V3 up = MatrixUtil::mul(matrix, V3(0, 0, 1.0f));
                    Camera c = Ark::instance().createCamera(Ark::COORDINATE_SYSTEM_LHS, Ark::instance().applicationContext()->renderEngine()->coordinateSystem() == Ark::COORDINATE_SYSTEM_LHS);
                    c.perspective(fovy, 16.0f / 9, clipNear, clipFar);
                    c.lookAt(p, p + front, up);
                    camera->assign(c);
                }
            }
            else if(clazz == "LIGHT")
            {
                const String& name = Documents::ensureAttribute(j, constants::NAME);
                const sp<Vec3> light = getLight(name);
                DCHECK_WARN(light, "Undefined light(%s) in \"%s\"", name.c_str(), src.c_str());
                if(light)
                {
                    const V3 position = Strings::eval<V3>(Documents::ensureAttribute(j, constants::POSITION));
                    Vec3Type::set(light, position);
                }
            }
        }
    }
}

sp<Layer> Level::getLayer(const String& name) const
{
    const auto iter = _render_object_layers.find(name);
    return iter != _render_object_layers.end() ? iter->second : nullptr;
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

Level::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _render_object_layers(loadNamedTypes<Layer>(factory, manifest, constants::RENDER_OBJECT, constants::LAYER))
{
    for(const document& i : manifest->children("camera"))
        _cameras.emplace_back(Documents::ensureAttribute(i, constants::NAME), factory.ensureBuilder<Camera>(i, constants::REF));
    for(const document& i : manifest->children("light"))
        _lights.emplace_back(Documents::ensureAttribute(i, constants::NAME), factory.ensureBuilder<Vec3>(i, constants::REF));
}

sp<Level> Level::BUILDER::build(const Scope& args)
{
    std::map<String, sp<Camera>> cameras;
    for(const auto& i : _cameras)
        cameras[i.first] = i.second->build(args);

    std::map<String, sp<Vec3>> lights;
    for(const auto& i : _lights)
        lights[i.first] = i.second->build(args);

    std::map<String, sp<Layer>> instanceLibraries = loadNamedTypeInstances<Layer>(_render_object_layers, args);

    return sp<Level>::make(std::move(instanceLibraries), std::move(cameras), std::move(lights));
}

}
