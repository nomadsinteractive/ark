#pragma once

#include "core/base/api.h"
#include "core/base/named_hash.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

class ARK_API Tile {
public:
//  [[script::bindings::auto]]
    Tile(int32_t id, const NamedHash& type = {0}, sp<Shape> shape = nullptr, sp<RenderObject> renderObject = nullptr);

//  [[script::bindings::property]]
    int32_t id() const;

//  [[script::bindings::property]]
    NamedHash type() const;
//  [[script::bindings::property]]
    void setType(const NamedHash& type);

//  [[script::bindings::property]]
    const sp<Shape>& shape() const;
//  [[script::bindings::property]]
    void setShape(sp<Shape> shape);

//  [[script::bindings::property]]
    const sp<RenderObject>& renderObject() const;
//  [[script::bindings::property]]
    void setRenderObject(sp<RenderObject> renderObject);

private:
    int32_t _id;
    NamedHash _type;
    sp<Shape> _shape;
    sp<RenderObject> _render_object;
};

}
