#pragma once

#include <map>
#include <vector>

#include "core/base/api.h"
#include "core/base/bean_factory.h"
#include "core/base/string.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

#include "app/forwarding.h"

namespace ark {

class ARK_API Level {
public:
//  [[script::bindings::auto]]
    Level(std::map<String, sp<Layer>> layers, std::map<String, sp<Camera>> cameras = {}, std::map<String, sp<Vec3>> lights = {});

//  [[script::bindings::auto]]
    void load(const String& src, const sp<Collider>& collider = nullptr, const std::map<String, sp<Shape>>& shapes = {});
//  [[script::bindings::auto]]
    sp<Layer> getLayer(const String& name) const;
//  [[script::bindings::auto]]
    sp<Camera> getCamera(const String& name) const;
//  [[script::bindings::auto]]
    sp<Vec3> getLight(const String& name) const;
//  [[script::bindings::auto]]
    sp<RenderObject> getRenderObject(const String& name) const;
//  [[script::bindings::auto]]
    sp<Rigidbody> getRigidBody(const String& name) const;

private:
    std::map<String, sp<Layer>> _layers;

    std::map<String, sp<Camera>> _cameras;
    std::map<String, sp<Vec3>> _lights;
    std::map<String, sp<RenderObject>> _render_objects;

    std::map<String, sp<Rigidbody>> _rigid_objects;
    std::vector<sp<Rigidbody>> _unnamed_rigid_objects;
};

}
