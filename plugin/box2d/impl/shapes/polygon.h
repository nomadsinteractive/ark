#pragma once

#include <vector>

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "box2d/inf/shape.h"

namespace ark::plugin::box2d {

class Polygon : public Shape {
public:
    Polygon(const std::vector<V2>& vertices);

    virtual void apply(b2BodyId body, const V3& size, const BodyCreateInfo& createInfo) override;

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
