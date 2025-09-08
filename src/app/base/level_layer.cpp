#include "app/base/level_layer.h"

#include "core/base/named_hash.h"

#include "graphics/base/transform_3d.h"
#include "graphics/util/vec3_type.h"
#include "graphics/util/vec4_type.h"

#include "app/base/level_library.h"
#include "app/base/level_object.h"
#include "app/inf/collider.h"

namespace ark {

LevelLayer::LevelLayer(const sp<Level::Stub>& level, String name, Vector<sp<LevelObject>> objects)
    : _level(level), _name(std::move(name)), _objects(std::move(objects))
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

sp<LevelObject> LevelLayer::getObject(const StringView name) const
{
    const auto iter = _objects_by_name.find(name);
    return iter != _objects_by_name.end() ? iter->second : nullptr;
}

void LevelLayer::createRenderObjects(const sp<Layer>& layer) const
{
    for(const sp<LevelObject>& i : _objects)
        if(const sp<RenderObject>& ro = i->createRenderObject(); ro && layer)
            layer->add(ro);
}

void LevelLayer::createRigidbodies(const sp<Collider>& collider, const Rigidbody::BodyType bodyType, const sp<CollisionFilter>& collisionFilter) const
{
    for(const sp<LevelObject>& i : _objects)
        i->createRigidbody(collider, bodyType, collisionFilter);
}

}
