#ifndef ARK_PLUGIN_BOX2D_IMPL_SHAPES_BOX_H_
#define ARK_PLUGIN_BOX2D_IMPL_SHAPES_BOX_H_

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "box2d/inf/shape.h"

namespace ark {
namespace plugin {
namespace box2d {

class Box : public Shape {
public:

    virtual void apply(b2Body* body, const sp<Size>& size, float density, float friction) override;

//  [[plugin::builder::by-value("box")]]
    class BUILDER : public Builder<Shape> {
    public:
        BUILDER();

        virtual sp<Shape> build(const sp<Scope>& args) override;

    };

};

}
}
}

#endif
