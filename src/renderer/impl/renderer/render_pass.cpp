#include "renderer/impl/renderer/render_pass.h"

#include "graphics/base/render_request.h"

#include "renderer/base/drawing_context.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/shader.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/base/vertex_writer.h"
#include "renderer/inf/uploader.h"
#include "renderer/inf/vertices.h"

namespace ark {

namespace {

class VerticesBufferUploader : public Uploader {
public:
    VerticesBufferUploader(sp<Vertices> vertices, sp<PipelineInput> pipelineInput)
        : Uploader(vertices->length() * pipelineInput->getStream(0).stride()), _vertices(std::move(vertices)), _pipeline_input(std::move(pipelineInput)) {
    }

    virtual void upload(Writable& uploader) override {
        size_t stride = _pipeline_input->getStream(0).stride();
        PipelineInput::AttributeOffsets attributes(_pipeline_input);
        uint32_t size = static_cast<uint32_t>(_vertices->length() * stride);
        std::vector<uint8_t> buf(size);
        VertexWriter stream(attributes, false, buf.data(), size, stride);
        _vertices->write(stream, V3(1.0f));
        uploader.write(buf.data(), size, 0);
    }

private:
    sp<Vertices> _vertices;
    sp<PipelineInput> _pipeline_input;
};

}

RenderPass::RenderPass(sp<Shader> shader, Buffer vertexBuffer, Buffer indexBuffer, ModelLoader::RenderMode mode, sp<Integer> drawCount)
    : _shader(std::move(shader)), _index_buffer(std::move(indexBuffer)), _draw_count(std::move(drawCount)),
      _shader_bindings(_shader->makeBindings(std::move(vertexBuffer), mode, _index_buffer ? PipelineBindings::RENDER_PROCEDURE_DRAW_ELEMENTS : PipelineBindings::RENDER_PROCEDURE_DRAW_ARRAYS))
{
}

void RenderPass::render(RenderRequest& renderRequest, const V3& /*position*/)
{
    uint32_t drawCount = static_cast<uint32_t>(_draw_count->val());
    const Buffer& vertices = _shader_bindings->vertices();

    DrawingContext drawingContext(_shader_bindings, _shader_bindings->attachments(), _shader->takeUBOSnapshot(renderRequest), _shader->takeSSBOSnapshot(renderRequest), vertices.snapshot(),
                                  _index_buffer.snapshot(), DrawingContext::ParamDrawElements(0, drawCount));
    renderRequest.addRequest(drawingContext.toRenderCommand(renderRequest));
}

RenderPass::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _shader(factory.ensureBuilder<Shader>(manifest, Constants::Attributes::SHADER)), _vertex_buffer(factory.ensureBuilder<Buffer>(manifest, "vertex-buffer")),
      _index_buffer(factory.getBuilder<Buffer>(manifest, "index-buffer")), _mode(Documents::getAttribute<ModelLoader::RenderMode>(manifest, "mode", ModelLoader::RENDER_MODE_TRIANGLES)),
      _draw_count(factory.ensureBuilder<Integer>(manifest, "draw-count"))
{
}

sp<Renderer> RenderPass::BUILDER::build(const Scope& args)
{
    return sp<RenderPass>::make(_shader->build(args), _vertex_buffer->build(args), _index_buffer ? _index_buffer->build(args) : Buffer(), _mode, _draw_count->build(args));
}

}
