#pragma once

#include "core/base/api.h"
#include "core/base/string.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

#include "app/forwarding.h"

namespace ark {

class ARK_API Level {
public:
//  [[script::bindings::auto]]
    Level(const String& src);

//  [[script::bindings::property]]
    const Map<int32_t, sp<LevelLibrary>>& libraries() const;

//  [[script::bindings::property]]
    const Vector<sp<LevelLayer>>& layers();
//  [[script::bindings::auto]]
    sp<LevelLayer> getLayer(StringView name) const;

//  [[script::bindings::property]]
    const Map<String, sp<Camera>>& cameras() const;
//  [[script::bindings::auto]]
    sp<Camera> getCamera(const String& name) const;

//  [[script::bindings::property]]
    const Map<String, sp<Vec3>>& lights() const;
//  [[script::bindings::auto]]
    sp<Vec3> getLight(const String& name) const;

private:
    void doLoad(const String& src);

    struct Stub {
        Map<int32_t, sp<LevelLibrary>> _libraries;
        Map<String, sp<Camera>> _cameras;
        Map<String, sp<Vec3>> _lights;
    };

private:
    sp<Stub> _stub;
    Vector<sp<LevelLayer>> _layers;
    Map<StringView, sp<LevelLayer>> _layers_by_name;

    friend class LevelLayer;
    friend class LevelObject;
};

}
