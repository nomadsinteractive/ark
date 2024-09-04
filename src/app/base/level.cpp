#include "app/base/level.h"

#include <unordered_map>

#include "core/ark.h"

#include "core/base/named_type.h"

#include "graphics/base/camera.h"
#include "graphics/base/layer.h"
#include "graphics/base/render_object.h"
#include "graphics/base/transform.h"
#include "graphics/util/vec3_type.h"

#include "app/base/application_context.h"
#include "app/base/application_bundle.h"
#include "app/base/rigid_body.h"
#include "app/inf/collider.h"
#include "app/traits/shape.h"


namespace ark {

namespace {

struct Library {
    sp<Collider> _rigid_body_collider;
    V3 _dimensions;
};

template<typename T> T parseVector(const String& value) {
    T vector(0);
    const std::vector<String> splitted = Strings::unwrap(value, '(', ')').split(',');
    CHECK(splitted.size() <= sizeof(T) / sizeof(float), "Vector \"%s\" has more components than its target value(Vec%d)", value.c_str(), sizeof(T) / sizeof(float));
    for(size_t i = 0; i < splitted.size(); ++i)
        vector[i] = Strings::eval<float>(splitted.at(i));
    return vector;
}

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

sp<Transform> makeTransform(const String& rotation, const String& scale)
{
    const V3 s = scale ? parseVector<V3>(scale) : V3(1.0f);
    const V4 quat = parseVector<V4>(rotation);
    return sp<Transform>::make(Transform::TYPE_LINEAR_3D, sp<Rotation>::make(quat), sp<Vec3>::make<Vec3::Const>(s));
}

std::tuple<String, int32_t, sp<RenderObject>> makeRenderObject(const document& manifest, bool visible)
{
    const Global<Constants> globalConstants;
    String name = Documents::getAttribute(manifest, constants::NAME);
    int32_t type = name.hash();
    const String& position = Documents::ensureAttribute(manifest, constants::POSITION);
    const String& scale = Documents::ensureAttribute(manifest, "scale");
    const String& rotation = Documents::ensureAttribute(manifest, constants::ROTATION);
    sp<Transform> transform = makeTransform(rotation, scale);
    return {std::move(name), type, sp<RenderObject>::make(type, sp<Vec3>::make<Vec3::Const>(parseVector<V3>(position)), nullptr, std::move(transform), nullptr, visible ? globalConstants->BOOLEAN_TRUE : globalConstants->BOOLEAN_FALSE)};
}

sp<RigidBody> makeRigidBody(const Library& library, const sp<RenderObject>& renderObject, int32_t shapeId)
{
    if(!library._rigid_body_collider)
        return nullptr;

    const V3& dimension = library._dimensions;
    return library._rigid_body_collider->createBody(Collider::BODY_TYPE_STATIC, sp<Shape>::make(shapeId, sp<Vec3>::make<Vec3::Const>(V3(dimension.x(), dimension.y(), dimension.z()))), renderObject->position(),
                                                renderObject->transform()->rotation());
}

}

Level::Level(std::map<String, sp<Layer>> renderObjectLayers, std::map<String, sp<Collider>> rigidBodyLayers, std::map<String, sp<Camera>> cameras, std::map<String, sp<Vec3>> lights)
    : _render_object_layers(std::move(renderObjectLayers)), _rigid_body_layers(std::move(rigidBodyLayers)), _cameras(std::move(cameras)), _lights(std::move(lights))
{
}

void Level::load(const String& src)
{
    const document manifest = Ark::instance().applicationContext()->applicationBundle()->loadDocument(src);
    CHECK(manifest, "Cannot load manifest \"%s\"", src.c_str());

    std::unordered_map<int32_t, Library> libraryMapping;
    for(const document& i : manifest->children("library"))
    {
        const String& name = Documents::ensureAttribute(i, constants::NAME);
        const String& dimensions = Documents::ensureAttribute(i, "dimensions");
        const int32_t id = Documents::ensureAttribute<int32_t>(i, constants::ID);
        CHECK_WARN(libraryMapping.find(id) == libraryMapping.end(), "Overwriting instance library mapping(%d)", id);
        libraryMapping.emplace(id, Library{getCollider(name), parseVector<V3>(dimensions)});
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
                auto [name, type, renderObject] = makeRenderObject(j, visible);

                CHECK(layer, "Trying to load model instance into undefined Layer(%s)", layerName.c_str());
                layer->addRenderObject(renderObject);

                if(sp<RigidBody> rigidBody = makeRigidBody(iter->second, renderObject, type))
                {
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
                auto [name, type, renderObject] = makeRenderObject(j, visible);
                CHECK(layer, "Trying to load model instance into undefined Layer(%s)", layerName.c_str());
                layer->addRenderObject(renderObject);
                if(name)
                    _render_objects[name] = std::move(renderObject);
            }
            else if(clazz == "CAMERA")
            {
                const String& name = Documents::ensureAttribute(j, constants::NAME);
                const String& position = Documents::ensureAttribute(j, constants::POSITION);
                const String& rotation = Documents::ensureAttribute(j, constants::ROTATION);
                const float fov = Documents::ensureAttribute<float>(j, "fov");
                const float clipNear = Documents::ensureAttribute<float>(j, "clip-near");
                const float clipFar = Documents::ensureAttribute<float>(j, "clip-far");
                const sp<Transform> transform = makeTransform(rotation, "");
                const sp<Camera> camera = getCamera(name);
                DCHECK_WARN(camera, "Undefined camera(%s) in \"%s\"", name.c_str(), src.c_str());
                if(camera)
                {
                    const V3 p = parseVector<V3>(position);
                    const Transform::Snapshot ts = transform->snapshot();
                    camera->perspective(fov, 16.0 / 9, clipNear, clipFar);
                    camera->lookAt(p, ts.transform(V3(0, 0, 1)) + p, ts.transform(V3(0, 1, 0)));
                }
            }
            else if(clazz == "LIGHT")
            {
                const String& name = Documents::ensureAttribute(j, constants::NAME);
                const sp<Vec3> light = getLight(name);
                DCHECK_WARN(light, "Undefined light(%s) in \"%s\"", name.c_str(), src.c_str());
                if(light)
                {
                    const V3 position = parseVector<V3>(Documents::ensureAttribute(j, constants::POSITION));
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

sp<Collider> Level::getCollider(const String& name) const
{
    const auto iter = _rigid_body_layers.find(name);
    return iter != _rigid_body_layers.end() ? iter->second : nullptr;
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

sp<RigidBody> Level::getRigidBody(const String& name) const
{
    const auto iter = _rigid_objects.find(name);
    return iter != _rigid_objects.end() ? iter->second : nullptr;
}

Level::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _render_object_layers(loadNamedTypes<Layer>(factory, manifest, constants::RENDER_OBJECT, constants::LAYER)),
      _rigid_object_layers(loadNamedTypes<Collider>(factory, manifest, "rigid-body", "collider"))
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
    std::map<String, sp<Collider>> rigidBodies = loadNamedTypeInstances<Collider>(_rigid_object_layers, args);

    return sp<Level>::make(std::move(instanceLibraries), std::move(rigidBodies), std::move(cameras), std::move(lights));
}

}
