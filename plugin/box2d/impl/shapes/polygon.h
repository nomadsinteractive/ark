#ifndef ARK_PLUGIN_BOX2D_IMPL_SHAPES_POLYGON_H_
#define ARK_PLUGIN_BOX2D_IMPL_SHAPES_POLYGON_H_

#include <vector>

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "box2d/inf/shape.h"

namespace ark {
namespace plugin {
namespace box2d {

class Polygon : public Shape {
public:
    Polygon(const std::vector<V2>& vertices);

    virtual void apply(b2Body* body, const sp<Size>& size, const BodyCreateInfo& createInfo) override;

//  [[plugin::builder("polygon")]]
    class BUILDER : public Builder<Shape> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Shape> build(const Scope& args) override;

    private:
        std::vector<V2> _vertices;

    };

private:
    std::vector<V2> _vertices;
};

}
}
}

#endif
