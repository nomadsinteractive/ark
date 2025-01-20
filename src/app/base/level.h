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

    void load(const String& src);

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
    sp<Map<int32_t, sp<LevelLibrary>>> _libraries;
    Vector<sp<LevelLayer>> _layers;
    Map<StringView, sp<LevelLayer>> _layers_by_name;
    Map<String, sp<Camera>> _cameras;
    Map<String, sp<Vec3>> _lights;
};

}
