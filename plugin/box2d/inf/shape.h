#ifndef ARK_PLUGIN_BOX2D_INF_SHAPE_H_
#define ARK_PLUGIN_BOX2D_INF_SHAPE_H_

#include <Box2D/Box2D.h>

#include "graphics/base/size.h"

#include "box2d/forwarding.h"

namespace ark {
namespace plugin {
namespace box2d {

//[[script::bindings::auto]]
class Shape {
public:
    virtual ~Shape() = default;

    virtual void apply(b2Body* body, const sp<Size>& size, const BodyCreateInfo& createInfo) = 0;

};

}
}
}

#endif
