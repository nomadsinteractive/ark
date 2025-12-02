#pragma once

#include "core/inf/builder.h"

#include "renderer/forwarding.h"

namespace ark::plugin::sdl3_gpu {

class RendererFactorySDL3_GPU {
public:

//  [[plugin::builder::by-value("sdl3-gpu")]]
    class BUILDER final : public Builder<RendererFactory> {
    public:
        BUILDER() = default;

        sp<RendererFactory> build(const Scope& args) override;
    };

};

}
