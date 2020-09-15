#include "app/base/level_loader.h"

#include <unordered_map>

#include "core/ark.h"

#include "core/base/bean_factory.h"
#include "core/util/documents.h"

#include "graphics/base/render_object.h"
#include "graphics/base/transform.h"
#include "graphics/util/vec3_type.h"

#include "app/base/application_context.h"
#include "app/base/application_resource.h"

namespace ark {

LevelLoader::LevelLoader(std::map<String, sp<Camera>> cameras, std::map<String, sp<Vec3>> lights, std::map<String, LevelLoader::InstanceLibrary> instanceLabraries)
    : _cameras(std::move(cameras)), _lights(std::move(lights)), _instance_libraries(std::move(instanceLabraries))
{
}

void LevelLoader::load(const String& src)
{
    const document manifest = Ark::instance().applicationContext()->applicationResource()->loadDocument(src);
    std::unordered_map<int32_t, InstanceLibrary> typeMapping;

    for(const document& i : manifest->children("library"))
    {
        const String& name = Documents::ensureAttribute(i, Constants::Attributes::NAME);
        const int32_t id = Documents::ensureAttribute<int32_t>(i, Constants::Attributes::ID);
        DWARN(typeMapping.find(id) == typeMapping.end(), "Overwriting instance library mapping(%d), originally mapped to type(%d)", id, typeMapping.find(id)->second._type);

        const auto iter = _instance_libraries.find(name);
        DCHECK(iter != _instance_libraries.end(), "Cannot find instance library(%s)", name.c_str());
        typeMapping[id] = iter->second;
    }

    for(const document& i : manifest->children("layer"))
    {
        for(const document& j : i->children("render-object"))
        {
            int32_t instanceOf = Documents::getAttribute<int32_t>(j, "instance-of", -1);
            const String clazz = Documents::getAttribute(j, "class");
            if(instanceOf != -1)
            {
                const auto iter = typeMapping.find(instanceOf);
                DASSERT(iter != typeMapping.end());
                int32_t type = iter->second._type;
                const sp<Layer>& layer = iter->second._layer;
                String name = Documents::getAttribute(j, "name");
                const String& position = Documents::ensureAttribute(j, "position");
                const String& scale = Documents::ensureAttribute(j, "scale");
                const String& rotation = Documents::ensureAttribute(j, "rotation");
                sp<Transform> transform = makeTransform(rotation, scale);
                sp<RenderObject> renderObject = sp<RenderObject>::make(type, sp<Vec3::Const>::make(parseVector<V3>(position)), nullptr, transform);
                if(name)
                    _render_objects[name] = renderObject;
                layer->addRenderObject(renderObject);
            }
            else if(clazz == "CAMERA")
            {
                const String& name = Documents::ensureAttribute(j, "name");
                const String& position = Documents::ensureAttribute(j, "position");
                const String& rotation = Documents::ensureAttribute(j, "rotation");
                const sp<Transform> transform = makeTransform(rotation, "");
                const sp<Camera> camera = getCamera(name);
                DWARN(camera, "Undefined camera(%s) in \"%s\"", name.c_str(), src.c_str());
                if(camera)
                {
                    const V3 p = parseVector<V3>(position);
                    const Transform::Snapshot ts = transform->snapshot();
                    camera->lookAt(p, ts.transform(V3(0, 0, -1)) + p, ts.transform(V3(0, 1, 0)));
                }
            }
            else if(clazz == "LIGHT")
            {
                const String& name = Documents::ensureAttribute(j, Constants::Attributes::NAME);
                const sp<Vec3> light = getLight(name);
                DWARN(light, "Undefined light(%s) in \"%s\"", name.c_str(), src.c_str());
                if(light)
                {
                    const V3 position = parseVector<V3>(Documents::ensureAttribute(j, Constants::Attributes::POSITION));
                    Vec3Type::set(light, position);
                }
            }
        }
    }
}

sp<Camera> LevelLoader::getCamera(const String& name) const
{
    const auto iter = _cameras.find(name);
    return iter != _cameras.end() ? iter->second : nullptr;
}

sp<Vec3> LevelLoader::getLight(const String& name) const
{
    const auto iter = _lights.find(name);
    return iter != _lights.end() ? iter->second : nullptr;
}

sp<RenderObject> LevelLoader::getRenderObject(const String& name) const
{
    const auto iter = _render_objects.find(name);
    return iter != _render_objects.end() ? iter->second : nullptr;
}

sp<Transform> LevelLoader::makeTransform(const String& rotation, const String& scale) const
{
    const V3 s = scale ? parseVector<V3>(scale) : V3(1.0f);
    const V4 rot = parseVector<V4>(rotation);
    return sp<Transform>::make(Transform::TYPE_LINEAR_3D, sp<Rotation>::make(nullptr, nullptr, sp<Vec4::Const>::make(V4(rot.y(), rot.z(), rot.w(), rot.x()))), sp<Vec3::Const>::make(s));
}

LevelLoader::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
{
    for(const document& i : manifest->children("camera"))
        _cameras.push_back({Documents::ensureAttribute(i, Constants::Attributes::NAME), factory.ensureBuilder<Camera>(i, Constants::Attributes::REF)});
    for(const document& i : manifest->children("light"))
        _lights.push_back({Documents::ensureAttribute(i, Constants::Attributes::NAME), factory.ensureBuilder<Vec3>(i, Constants::Attributes::REF)});

    for(const document& i : manifest->children("library"))
        _libraries.push_back({Documents::ensureAttribute(i, Constants::Attributes::NAME), Documents::ensureAttribute<int32_t>(i, Constants::Attributes::TYPE),
                              factory.ensureBuilder<Layer>(i, Constants::Attributes::LAYER)});
}

sp<LevelLoader> LevelLoader::BUILDER::build(const Scope& args)
{
    std::map<String, sp<Camera>> cameras;
    for(const auto& i : _cameras)
        cameras[i.first] = i.second->build(args);

    std::map<String, sp<Vec3>> lights;
    for(const auto& i : _lights)
        lights[i.first] = i.second->build(args);

    std::map<String, InstanceLibrary> instanceLibraries;
    for(const Library& i : _libraries)
        instanceLibraries[i._name] = {i._type, i._layer->build(args)};

    return sp<LevelLoader>::make(std::move(cameras), std::move(lights), std::move(instanceLibraries));
}

}
