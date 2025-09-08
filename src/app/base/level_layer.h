#pragma once

#include "core/base/api.h"
#include "core/base/string.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "app/forwarding.h"
#include "app/base/level.h"
#include "app/components/rigidbody.h"

namespace ark {

class ARK_API LevelLayer {
public:
    LevelLayer(const sp<Level::Stub>& level, String name, Vector<sp<LevelObject>> objects);

//  [[script::bindings::property]]
    const String& name() const;
//  [[script::bindings::property]]
    const Vector<sp<LevelObject>>& objects() const;
//  [[script::bindings::auto]]
    sp<LevelObject> getObject(StringView name) const;

//  [[script::bindings::auto]]
    void createRenderObjects(const sp<Layer>& layer = nullptr) const;
    [[deprecated]]
//  [[script::bindings::auto]]
    void createRigidbodies(const sp<Collider>& collider, Rigidbody::BodyType bodyType, const sp<CollisionFilter>& collisionFilter = nullptr) const;

private:
    sp<Level::Stub> _level;
    String _name;
    Vector<sp<LevelObject>> _objects;
    Map<StringView, sp<LevelObject>> _objects_by_name;
};

}
