#ifndef ARK_PLUGIN_BOX2D_IMPL_SHAPES_ARC_H_
#define ARK_PLUGIN_BOX2D_IMPL_SHAPES_ARC_H_

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "box2d/inf/shape.h"

namespace ark {
namespace plugin {
namespace box2d {

class Arc : public Shape {
public:
    Arc(float radius, uint32_t sampleCount, float a, float b);

    virtual void apply(b2Body* body, float density, float friction) override;

//  [[plugin::builder("arc")]]
    class BUILDER : public Builder<Shape> {
    public:
        BUILDER(BeanFactory& parent, const document& doc);

        virtual sp<Shape> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Numeric>> _radius;
        sp<Builder<Numeric>> _sample_count;
        sp<Builder<Numeric>> _a;
        sp<Builder<Numeric>> _b;

    };

private:
    float _radius;
    uint32_t _sample_count;
    float _a, _b;
};

}
}
}

#endif
