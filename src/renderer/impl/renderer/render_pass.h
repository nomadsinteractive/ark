#pragma once

#include <map>

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/inf/renderer.h"
#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/inf/model_loader.h"

namespace ark {

class RenderPass : public Renderer {
public:
    RenderPass(sp<Shader> shader, Buffer vertexBuffer, Buffer indexBuffer, ModelLoader::RenderMode mode, sp<Integer> drawCount, PipelineBindings::DrawProcedure drawProcedure, const std::map<uint32_t, sp<Uploader>>& dividedUploaders);

    virtual void render(RenderRequest& renderRequest, const V3& position) override;

//  [[plugin::resource-loader("render-pass")]]
    class BUILDER : public Builder<Renderer> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        sp<Builder<Shader>> _shader;
        sp<Builder<Buffer>> _vertex_buffer;
        sp<Builder<Buffer>> _index_buffer;
        ModelLoader::RenderMode _mode;
        sp<Builder<Integer>> _draw_count;
        std::map<uint32_t, sp<Builder<Uploader>>> _divided_uploaders;
        PipelineBindings::DrawProcedure _draw_precedure;
        sp<RenderController> _render_controller;
    };

private:
    sp<Shader> _shader;
    Buffer _index_buffer;
    sp<Integer> _draw_count;
    PipelineBindings::DrawProcedure _draw_procedure;
    sp<ShaderBindings> _shader_bindings;
};

}
