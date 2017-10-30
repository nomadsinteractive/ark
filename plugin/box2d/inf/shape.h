#ifndef ARK_PLUGIN_BOX2D_IMPL_SHAPE_H_
#define ARK_PLUGIN_BOX2D_IMPL_SHAPE_H_

#include <Box2D/Box2D.h>

namespace ark {
namespace plugin {
namespace box2d {

//[[script::bindings::auto]]
class Shape {
public:
    virtual ~Shape() = default;

    virtual void apply(b2Body* body, float density, float friction) = 0;

};

}
}
}

#endif
