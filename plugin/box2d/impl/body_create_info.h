#ifndef ARK_PLUGIN_BOX2D_IMPL_BODY_CREATE_INFO_H_
#define ARK_PLUGIN_BOX2D_IMPL_BODY_CREATE_INFO_H_

#include <Box2D/Box2D.h>

#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

#include "box2d/forwarding.h"

namespace ark {
namespace plugin {
namespace box2d {

class BodyCreateInfo {
public:
    BodyCreateInfo();
    BodyCreateInfo(const sp<Shape>& shape, float density, float friction, bool isSensor = false);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(BodyCreateInfo);

    b2FixtureDef toFixtureDef(const b2Shape* shape) const;

    sp<Shape> shape;
    float density;
    float friction;
    bool is_sensor;

    uint16_t category;
    uint16_t mask;
    int16_t group;
};

}
}
}

#endif
