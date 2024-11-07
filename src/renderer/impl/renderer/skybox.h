#pragma once

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/inf/renderer.h"
#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"

namespace ark {

class Skybox final : public Renderer {
public:
    Skybox(const sp<Shader>& shader, const sp<Texture>& texture, RenderController& renderController);

    void render(RenderRequest& renderRequest, const V3& position) override;

//  [[plugin::resource-loader("skybox")]]
    class BUILDER final : public Builder<Renderer> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        sp<Renderer> build(const Scope& args) override;

    private:
        sp<ResourceLoaderContext> _resource_loader_context;

        sp<Builder<Shader>> _shader;
        sp<Builder<Texture>> _texture;
    };

private:
    sp<Shader> _shader;
    sp<PipelineBindings> _pipeline_bindings;
    Buffer::Snapshot _ib_snapshot;
};

}
