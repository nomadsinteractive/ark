#pragma once

#include "core/base/api.h"
#include "core/base/string.h"
#include "core/types/shared_ptr.h"
#include "core/inf/variable.h"

#include "graphics/forwarding.h"

#include "app/forwarding.h"

namespace ark {

class ARK_API LevelLibrary {
public:
    LevelLibrary(int32_t id, String name, sp<Vec3> size, sp<Shape> shape = nullptr);

//  [[script::bindings::property]]
    int32_t id() const;
//  [[script::bindings::property]]
    const String& name() const;
//  [[script::bindings::property]]
    const sp<Vec3>& size() const;
//  [[script::bindings::property]]
    const sp<Shape>& shape() const;
//  [[script::bindings::property]]
    void setShape(sp<Shape> shape);

private:
    int32_t _id;
    String _name;
    sp<Vec3> _size;
    sp<Shape> _shape;

    friend class Level;
    friend class LevelLayer;
    friend class LevelObject;
};

}