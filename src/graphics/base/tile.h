#ifndef ARK_GRAPHICS_BASE_TILE_H_
#define ARK_GRAPHICS_BASE_TILE_H_

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

class ARK_API Tile {
public:
//  [[script::bindings::auto]]
    Tile(int32_t id, int32_t shapeId = 0, sp<RenderObject> renderObject = nullptr);

//  [[script::bindings::property]]
    int32_t id() const;

//  [[script::bindings::property]]
    int32_t type() const;
//  [[script::bindings::property]]
    void setType(int32_t type);

//  [[script::bindings::property]]
    const sp<RenderObject>& renderObject() const;
//  [[script::bindings::property]]
    void setRenderObject(sp<RenderObject> renderObject);

private:
    int32_t _id;
    int32_t _type;
    sp<RenderObject> _render_object;
};

}

#endif
