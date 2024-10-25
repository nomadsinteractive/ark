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
    template<typename T> struct NamedLayerBuilder {
        String _name;
        sp<Builder<T>> _builder;
    };

    typedef NamedLayerBuilder<Layer> RenderObjectLayer;
    typedef NamedLayerBuilder<Collider> RigidBodyLayer;

public:
//  [[script::bindings::auto]]
    Level(std::map<String, sp<Layer>> renderObjectLayers, std::map<String, sp<Camera>> cameras = {}, std::map<String, sp<Vec3>> lights = {});

//  [[script::bindings::auto]]
    void load(const String& src, const sp<Collider>& collider = nullptr, const std::map<String, String>& shapeIdAliases = {});
//  [[script::bindings::auto]]
    sp<Layer> getLayer(const String& name) const;
//  [[script::bindings::auto]]
    sp<Camera> getCamera(const String& name) const;
//  [[script::bindings::auto]]
    sp<Vec3> getLight(const String& name) const;
//  [[script::bindings::auto]]
    sp<RenderObject> getRenderObject(const String& name) const;
//  [[script::bindings::auto]]
    sp<RigidBody> getRigidBody(const String& name) const;

//  [[plugin::builder]]
    class BUILDER final : public Builder<Level> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        sp<Level> build(const Scope& args) override;

    private:
        std::vector<RenderObjectLayer> _render_object_layers;
        std::vector<std::pair<String, sp<Builder<Camera>>>> _cameras;
        std::vector<std::pair<String, sp<Builder<Vec3>>>> _lights;
    };

private:
    std::map<String, sp<Layer>> _render_object_layers;

    std::map<String, sp<Camera>> _cameras;
    std::map<String, sp<Vec3>> _lights;
    std::map<String, sp<RenderObject>> _render_objects;

    std::map<String, sp<RigidBody>> _rigid_objects;
    std::vector<sp<RigidBody>> _unnamed_rigid_objects;
};

}
