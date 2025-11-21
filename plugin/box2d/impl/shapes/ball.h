#pragma once

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "box2d/inf/shape.h"

namespace ark::plugin::box2d {

class Ball : public Shape {
public:

    void apply(b2BodyId body, const V3& size, const BodyCreateInfo& createInfo) override;

//  [[plugin::builder::by-value("ball")]]
    class BUILDER : public Builder<Shape> {
    public:
        BUILDER();

        virtual sp<Shape> build(const Scope& args) override;

    };
};

}
