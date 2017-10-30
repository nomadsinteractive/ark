#ifndef ARK_PLUGIN_BOX2D_IMPL_SHAPES_BALL_H_
#define ARK_PLUGIN_BOX2D_IMPL_SHAPES_BALL_H_

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "box2d/inf/shape.h"

namespace ark {
namespace plugin {
namespace box2d {

class Ball : public Shape {
public:
    Ball(float radius);

    virtual void apply(b2Body* body, float density, float friction) override;

//  [[plugin::builder("ball")]]
    class BUILDER : public Builder<Shape> {
    public:
        BUILDER(BeanFactory& parent, const document& doc);

        virtual sp<Shape> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Numeric>> _radius;

    };

private:
    float _radius;
};

}
}
}

#endif
