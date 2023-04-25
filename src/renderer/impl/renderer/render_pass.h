#pragma once

#include <vector>

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"
#include "core/types/safe_ptr.h"

#include "graphics/inf/renderer.h"
#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"
#include "renderer/inf/model_loader.h"

namespace ark {

class RenderPass : public Renderer {
public:
    RenderPass(sp<Shader> shader, Buffer vertexBuffer, Buffer indexBuffer, ModelLoader::RenderMode mode, sp<Integer> drawCount, std::vector<std::pair<uint32_t, Buffer>> dividedBuffers);

    virtual void render(RenderRequest& renderRequest, const V3& position) override;

//  [[plugin::resource-loader("render-pass")]]
    class BUILDER : public Builder<Renderer> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        sp<Builder<Shader>> _shader;
        sp<Builder<Buffer>> _vertex_buffer;
        SafePtr<Builder<Buffer>> _index_buffer;
        ModelLoader::RenderMode _mode;
        sp<Builder<Integer>> _draw_count;
        std::vector<std::pair<uint32_t, sp<Builder<Uploader>>>> _divided_buffer_uploaders;
        sp<RenderController> _render_controller;
    };

private:
    sp<Shader> _shader;
    Buffer _index_buffer;
    sp<Integer> _draw_count;
    sp<ShaderBindings> _shader_bindings;
    std::vector<std::pair<uint32_t, Buffer>> _divided_buffers;
};

}
