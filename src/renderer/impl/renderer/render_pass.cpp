#include "renderer/impl/renderer/render_pass.h"

#include "graphics/base/render_request.h"

#include "renderer/base/drawing_context.h"
#include "renderer/base/model.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/shader.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/base/vertex_stream.h"
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
        PipelineInput::Attributes attributes(_pipeline_input);
        uint32_t size = static_cast<uint32_t>(_vertices->length() * stride);
        std::vector<uint8_t> buf(size);
        VertexStream stream(attributes, false, buf.data(), size, stride);
        _vertices->write(stream, V3(1.0f));
        uploader.write(buf.data(), size, 0);
    }

private:
    sp<Vertices> _vertices;
    sp<PipelineInput> _pipeline_input;
};

}

RenderPass::RenderPass(sp<Shader> shader, Buffer vertexBuffer, Buffer indexBuffer, ModelLoader::RenderMode mode, sp<Integer> drawCount)
    : _shader(std::move(shader)), _vertex_buffer(std::move(vertexBuffer)), _index_buffer(std::move(indexBuffer)), _draw_count(std::move(drawCount)),
      _shader_bindings(_shader->makeBindings(mode, _index_buffer ? PipelineBindings::RENDER_PROCEDURE_DRAW_ELEMENTS_INSTANCED : PipelineBindings::RENDER_PROCEDURE_DRAW_ARRAYS))
{
}

void RenderPass::render(RenderRequest& renderRequest, const V3& /*position*/)
{
    uint32_t drawCount = static_cast<uint32_t>(_draw_count->val());
    if(_index_buffer)
    {
        DrawingContext drawingContext(_shader_bindings, _shader_bindings->attachments(), _shader->takeUBOSnapshot(renderRequest), _vertex_buffer.snapshot(),
                                      _index_buffer.snapshot(), DrawingContext::ParamDrawElementsInstanced(0, static_cast<uint32_t>(_index_buffer.size() / sizeof(element_index_t)), drawCount, {}));
        renderRequest.addRequest(drawingContext.toRenderCommand(renderRequest));
    }
    else
    {
        DrawingContext drawingContext(_shader_bindings, _shader_bindings->attachments(), _shader->takeUBOSnapshot(renderRequest), _vertex_buffer.snapshot(),
                                      Buffer::Snapshot(), DrawingContext::ParamDrawElements(0, drawCount));
        renderRequest.addRequest(drawingContext.toRenderCommand(renderRequest));
    }
}

RenderPass::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_loader_context(resourceLoaderContext), _shader(factory.ensureBuilder<Shader>(manifest, Constants::Attributes::SHADER)),
      _model(manifest->getChild("model")), _model_importer(_model ? factory.getBuilder<ModelLoader::Importer>(_model, "importer") : nullptr),
      _vertex_buffer(factory.getBuilder<Buffer>(manifest, "vertex-buffer")), _index_buffer(factory.getBuilder<Buffer>(manifest, "index-buffer")),
      _mode(Documents::getAttribute<ModelLoader::RenderMode>(manifest, "mode", ModelLoader::RENDER_MODE_TRIANGLES)),
      _draw_count(factory.ensureBuilder<Integer>(manifest, "draw-count"))
{
    DCHECK(_model_importer || _vertex_buffer, "RenderPass should have one model-importer or vertex-buffer defined");
}

sp<Renderer> RenderPass::BUILDER::build(const Scope& args)
{
    if(_model_importer)
    {
        const sp<ModelLoader::Importer> importer = _model_importer->build(args);
        Model model = importer->import(_model, Rect(0, 0, 1.0f, 1.0f));
        sp<Shader> shader = _shader->build(args);
        Buffer vertices = _resource_loader_context->renderController()->makeVertexBuffer(Buffer::USAGE_STATIC, sp<VerticesBufferUploader>::make(model.vertices(), shader->input()));
        Buffer indices = _resource_loader_context->renderController()->makeIndexBuffer(Buffer::USAGE_STATIC, model.indices());
        return sp<RenderPass>::make(std::move(shader), std::move(vertices), std::move(indices), _mode, _draw_count->build(args));
    }
    return sp<RenderPass>::make(_shader->build(args), _vertex_buffer->build(args), _index_buffer ? _index_buffer->build(args) : Buffer(), _mode, _draw_count->build(args));
}

}
