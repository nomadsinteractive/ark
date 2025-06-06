#pragma once

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/inf/renderer.h"
#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

namespace ark {

//[[script::bindings::extends(Renderer)]]
class ARK_API ComputePass final : public Renderer {
public:
//  [[script::bindings::auto]]
    ComputePass(sp<Shader> shader);

    void render(RenderRequest& renderRequest, const V3& position, const sp<DrawDecorator>& drawDecorator) override;

//  [[plugin::builder("compute-pass")]]
    class BUILDER final : public Builder<Renderer> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        sp<Renderer> build(const Scope& args) override;

    private:
        sp<Builder<Shader>> _shader;
    };

private:
    sp<Shader> _shader;
    sp<PipelineBindings> _pipeline_bindings;
};

}
