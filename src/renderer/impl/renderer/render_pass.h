#pragma once

#include "core/inf/builder.h"
#include "core/base/enum.h"
#include "core/types/safe_var.h"
#include "core/types/shared_ptr.h"

#include "graphics/inf/renderer.h"
#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"

namespace ark {

//[[script::bindings::extends(Renderer)]]
class ARK_API RenderPass final : public Renderer {
public:
//  [[script::bindings::auto]]
    RenderPass(sp<Shader> shader, Buffer vertexBuffer, Buffer indexBuffer, sp<Integer> offset, sp<Integer> drawCount, enums::DrawProcedure drawProcedure, enums::DrawMode drawMode = enums::DRAW_MODE_TRIANGLES, Vector<std::pair<uint32_t, Buffer>> instanceBuffers = {}, Buffer indirectBuffer = {});

    void render(RenderRequest& renderRequest, const V3& position, const sp<DrawDecorator>& drawDecorator) override;

//  [[plugin::builder("render-pass")]]
    class BUILDER final : public Builder<Renderer> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        sp<Renderer> build(const Scope& args) override;

    private:
        sp<Builder<Shader>> _shader;
        sp<Builder<Buffer>> _vertex_buffer;
        sp<Builder<Buffer>> _index_buffer;
        sp<Builder<Buffer>> _indirect_buffer;
        SafeBuilder<Integer> _offset;
        sp<Builder<Integer>> _draw_count;
        enums::DrawMode _mode;
        enums::DrawProcedure _draw_procedure;
        Vector<std::pair<uint32_t, sp<Builder<Buffer>>>> _instance_buffers;
    };

private:
    sp<Shader> _shader;
    Buffer _index_buffer;
    SafeVar<Integer> _offset;
    sp<Integer> _draw_count;
    enums::DrawProcedure _draw_procedure;
    Buffer _indirect_buffer;
    sp<PipelineBindings> _pipeline_bindings;
};

}
