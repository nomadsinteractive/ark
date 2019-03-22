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
    Arc(uint32_t sampleCount, float a, float b);

    virtual void apply(b2Body* body, const sp<Size>& size, const BodyCreateInfo& createInfo) override;

//  [[plugin::builder("arc")]]
    class BUILDER : public Builder<Shape> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Shape> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Numeric>> _sample_count;
        sp<Builder<Numeric>> _a;
        sp<Builder<Numeric>> _b;

    };

private:
    uint32_t _sample_count;
    float _a, _b;
};

}
}
}

#endif
