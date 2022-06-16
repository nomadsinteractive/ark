#ifndef ARK_GRAPHICS_BASE_TILE_H_
#define ARK_GRAPHICS_BASE_TILE_H_

#include "core/base/api.h"
#include "core/base/string.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

class ARK_API Tile {
public:
//  [[script::bindings::auto]]
    Tile(int32_t id, String type = "", int32_t shapeId = -1, uint32_t width = 0, uint32_t height = 0, int32_t renderObjectId = 0);
    Tile(int32_t id, String type, int32_t shapeId, uint32_t width, uint32_t height, sp<RenderObject> renderObject);

//  [[script::bindings::property]]
    int32_t id() const;

//  [[script::bindings::property]]
    const String& type() const;
//  [[script::bindings::property]]
    void setType(String type);

//  [[script::bindings::property]]
    int32_t shapeId() const;
//  [[script::bindings::property]]
    void setShapeId(int32_t shapeId);

//  [[script::bindings::property]]
    uint32_t width() const;
//  [[script::bindings::property]]
    uint32_t height() const;

//  [[script::bindings::property]]
    const sp<RenderObject>& renderObject() const;
//  [[script::bindings::property]]
    void setRenderObject(sp<RenderObject> renderObject);

    const sp<RenderObject>& ensureRenderObject();

private:
    int32_t _id;
    int32_t _shape_id;

    String _type;

    uint32_t _width;
    uint32_t _height;

    int32_t _render_object_id;
    sp<RenderObject> _render_object;
};

}

#endif
