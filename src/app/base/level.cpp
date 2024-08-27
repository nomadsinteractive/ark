#include "app/base/level.h"

#include <unordered_map>

#include "core/ark.h"

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

Level::Level(std::map<String, sp<Camera>> cameras, std::map<String, sp<Vec3>> lights, std::map<String, RenderObjectLibrary::Instance> renderObjects, std::map<String, RigidBodyLibrary::Instance> rigidBodies)
    : _cameras(std::move(cameras)), _lights(std::move(lights)), _render_object_libraries(std::move(renderObjects)), _rigid_body_libraries(std::move(rigidBodies))
{
}

void Level::load(const String& src)
{
    const document manifest = Ark::instance().applicationContext()->applicationBundle()->loadDocument(src);
    std::unordered_map<int32_t, Library> libraryMapping;

    for(const document& i : manifest->children("library"))
    {
        const String& name = Documents::ensureAttribute(i, constants::NAME);
        const String& dimensions = Documents::ensureAttribute(i, "dimensions");
        const int32_t id = Documents::ensureAttribute<int32_t>(i, constants::ID);
        CHECK_WARN(libraryMapping.find(id) == libraryMapping.end(), "Overwriting instance library mapping(%d), originally mapped to type(%d)", id, libraryMapping.find(id)->second._render_object_instance->_type);

        const auto it1 = _render_object_libraries.find(name);
        DCHECK(it1 != _render_object_libraries.end(), "Cannot find instance library(%s)", name.c_str());
        const auto it2 = _rigid_body_libraries.find(name);
        libraryMapping.insert(std::make_pair(id, Library(it1->second, it2 == _rigid_body_libraries.end() ? nullptr : &it2->second, parseVector<V3>(dimensions))));
    }

    for(const document& i : manifest->children("layer"))
    {
        for(const document& j : i->children("render-object"))
        {
            int32_t instanceOf = Documents::getAttribute<int32_t>(j, "instance-of", -1);
            const String clazz = Documents::getAttribute(j, "class");
            if(instanceOf != -1)
            {
                const auto it1 = libraryMapping.find(instanceOf);
                DASSERT(it1 != libraryMapping.end());
                const Level::RenderObjectLibrary::Instance* instance = it1->second._render_object_instance;
                int32_t type = instance->_type;
                const sp<Layer>& layer = instance->_object;
                String name = Documents::getAttribute(j, "name");
                const String& position = Documents::ensureAttribute(j, "position");
                const String& scale = Documents::ensureAttribute(j, "scale");
                const String& rotation = Documents::ensureAttribute(j, "rotation");
                sp<Transform> transform = makeTransform(rotation, scale);
                sp<RenderObject> renderObject = sp<RenderObject>::make(type, sp<Vec3::Const>::make(parseVector<V3>(position)), nullptr, transform);
                sp<RigidBody> rigidBody = makeRigidBody(it1->second, renderObject);
                if(name)
                    _render_objects[name] = renderObject;
                if(rigidBody)
                {
                    if(name)
                        _rigid_objects[name] = std::move(rigidBody);
                    else
                        _unnamed_rigid_objects.push_back(std::move(rigidBody));
                }

                layer->addRenderObject(renderObject);
            }
            else if(clazz == "CAMERA")
            {
                const String& name = Documents::ensureAttribute(j, "name");
                const String& position = Documents::ensureAttribute(j, "position");
                const String& rotation = Documents::ensureAttribute(j, "rotation");
                const sp<Transform> transform = makeTransform(rotation, "");
                const sp<Camera> camera = getCamera(name);
                DCHECK_WARN(camera, "Undefined camera(%s) in \"%s\"", name.c_str(), src.c_str());
                if(camera)
                {
                    const V3 p = parseVector<V3>(position);
                    const Transform::Snapshot ts = transform->snapshot();
                    camera->lookAt(p, ts.transform(V3(0, 0, -1)) + p, ts.transform(V3(0, 1, 0)));
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

sp<RigidBody> Level::makeRigidBody(const Library& library, const sp<RenderObject>& renderObject) const
{
    const RigidBodyLibrary::Instance* rigidBodyLibray = library._rigid_body_instance;
    if(!rigidBodyLibray)
        return nullptr;

    const V3& dimension = library._dimensions;
    return rigidBodyLibray->_object->createBody(Collider::BODY_TYPE_STATIC, sp<Shape>::make(rigidBodyLibray->_type, sp<Vec3>::make<Vec3::Const>(V3(dimension.x(), dimension.y(), dimension.z()))), renderObject->position(),
                                                renderObject->transform()->rotation());
}

sp<Transform> Level::makeTransform(const String& rotation, const String& scale) const
{
    const V3 s = scale ? parseVector<V3>(scale) : V3(1.0f);
    const V4 quat = parseVector<V4>(rotation);
    return sp<Transform>::make(Transform::TYPE_LINEAR_3D, sp<Rotation>::make(quat), sp<Vec3::Const>::make(s));
}

Level::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _render_object_libraries(loadNamedTypes<Layer>(factory, manifest, constants::RENDER_OBJECT, constants::LAYER)),
      _rigid_object_libraries(loadNamedTypes<Collider>(factory, manifest, "rigid-body", "collider"))
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

    std::map<String, RenderObjectLibrary::Instance> instanceLibraries = loadNamedTypeInstances<Layer>(_render_object_libraries, args);
    std::map<String, RigidBodyLibrary::Instance> rigidBodies = loadNamedTypeInstances<Collider>(_rigid_object_libraries, args);

    return sp<Level>::make(std::move(cameras), std::move(lights), std::move(instanceLibraries), std::move(rigidBodies));
}

Level::Library::Library(const RenderObjectLibrary::Instance& renderObjectInstance, const RigidBodyLibrary::Instance* rigidBodyInstance, const V3& dimensions)
    : _render_object_instance(&renderObjectInstance), _rigid_body_instance(rigidBodyInstance), _dimensions(dimensions)
{
}

}
