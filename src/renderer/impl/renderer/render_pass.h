#pragma once

#include <map>

#include "core/inf/builder.h"
#include "core/base/enum.h"
#include "core/types/shared_ptr.h"

#include "graphics/inf/renderer.h"
#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"

namespace ark {

//[[script::bindings::extends(Renderer)]]
class ARK_API RenderPass : public Renderer {
public:
//  [[script::bindings::auto]]
    RenderPass(sp<Shader> shader, Buffer vertexBuffer, Buffer indexBuffer, sp<Integer> drawCount, Enum::RenderMode mode, Enum::DrawProcedure drawProcedure, const std::map<uint32_t, sp<Uploader>>& dividedUploaders);

    virtual void render(RenderRequest& renderRequest, const V3& position) override;

//  [[plugin::builder("render-pass")]]
    class BUILDER : public Builder<Renderer> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        sp<Builder<Shader>> _shader;
        sp<Builder<Buffer>> _vertex_buffer;
        sp<Builder<Buffer>> _index_buffer;
        sp<Builder<Integer>> _draw_count;
        Enum::RenderMode _mode;
        Enum::DrawProcedure _draw_precedure;
        std::map<uint32_t, sp<Builder<Uploader>>> _divided_uploaders;
    };

private:
    sp<Shader> _shader;
    Buffer _index_buffer;
    sp<Integer> _draw_count;
    Enum::DrawProcedure _draw_procedure;
    sp<PipelineBindings> _pipeline_bindings;
};

}
