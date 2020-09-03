#include "app/base/level_loader.h"

#include <unordered_map>

#include "core/ark.h"

#include "core/base/bean_factory.h"
#include "core/util/documents.h"

#include "graphics/base/render_object.h"
#include "graphics/base/transform.h"

#include "app/base/application_context.h"
#include "app/base/application_resource.h"

namespace ark {

LevelLoader::LevelLoader(std::map<String, LevelLoader::InstanceLibrary> instanceLabraries)
    : _instance_libraries(std::move(instanceLabraries))
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
            if(instanceOf != -1)
            {
                const auto iter = typeMapping.find(instanceOf);
                DASSERT(iter != typeMapping.end());
                int32_t type = iter->second._type;
                const sp<Layer>& layer = iter->second._layer;
                const String& position = Documents::ensureAttribute(j, "position");
                const String& rotation = Documents::ensureAttribute(j, "rotation");
                const String& scale = Documents::ensureAttribute(j, "rotation");
                sp<Transform> transform = sp<Transform>::make(Transform::TYPE_LINEAR_3D, sp<Rotation>::make(nullptr, nullptr, parseVector<V4>(rotation)), parseVector<V3>(scale));
                sp<RenderObject> renderObject = sp<RenderObject>::make(type, parseVector<V3>(position), nullptr, transform);
                layer->addRenderObject(renderObject);
            }
        }
    }

}

LevelLoader::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
{
    for(const document& i : manifest->children("library"))
        _libraries.push_back({Documents::ensureAttribute(i, Constants::Attributes::NAME), Documents::ensureAttribute<int32_t>(i, Constants::Attributes::TYPE),
                              factory.ensureBuilder<Layer>(i, Constants::Attributes::LAYER)});
}

sp<LevelLoader> LevelLoader::BUILDER::build(const Scope& args)
{
    std::map<String, InstanceLibrary> instanceLibraries;
    for(const Library& i : _libraries)
        instanceLibraries[i._name] = {i._type, i._layer->build(args)};

    return sp<LevelLoader>::make(std::move(instanceLibraries));
}

}
