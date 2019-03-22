#include "graphics/base/render_layer.h"

#include "core/base/observer.h"
#include "core/epi/notifier.h"

#include "graphics/base/camera.h"
#include "graphics/base/layer_context.h"
#include "graphics/base/render_object.h"
#include "graphics/base/render_request.h"
#include "graphics/base/size.h"

#include "renderer/base/drawing_context.h"
#include "renderer/base/model_buffer.h"
#include "renderer/base/pipeline_input.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/shader.h"
#include "renderer/base/shader_bindings.h"

#include "renderer/inf/pipeline.h"
#include "renderer/inf/pipeline_factory.h"
#include "renderer/inf/render_model.h"

namespace ark {

RenderLayer::Stub::Stub(const sp<RenderModel>& renderModel, const sp<Shader>& shader, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _render_model(renderModel), _shader(shader), _resource_loader_context(resourceLoaderContext), _memory_pool(resourceLoaderContext->memoryPool()),
      _render_controller(resourceLoaderContext->renderController()), _shader_bindings(_render_model->makeShaderBindings(_render_controller, shader->pipelineLayout())),
      _notifier(sp<Notifier>::make()), _layer_context(sp<LayerContext>::make(renderModel, _notifier)), _dirty(_notifier->createObserver()),
      _stride(shader->input()->getStream(0).stride())
{
}

RenderLayer::Snapshot::Snapshot(const sp<Stub>& stub)
    : _stub(stub), _ubos(stub->_shader->snapshot(_stub->_memory_pool))
{
    stub->_layer_context->takeSnapshot(*this, stub->_memory_pool);

    for(const sp<LayerContext>& i : stub->_layer_contexts)
        i->takeSnapshot(*this, stub->_memory_pool);

    _dirty = stub->_dirty->val() || true;
}

sp<RenderCommand> RenderLayer::Snapshot::render(float x, float y)
{
    if(_items.size() > 0)
    {
        ModelBuffer buf(_stub->_resource_loader_context, _stub->_shader_bindings, _items.size(), _stub->_stride);
        _stub->_render_model->start(buf, *this);
        if(_dirty)
        {
            for(const RenderObject::Snapshot& i : _items)
            {
                buf.setRenderObject(i);
                buf.setTranslate(V3(x + i._position.x(), y + i._position.y(), i._position.z()));
                _stub->_render_model->load(buf, i);
                if(buf.isInstanced())
                {
                    Buffer::Builder& sBuilder = buf.getInstancedArrayBuilder(1);
                    sBuilder.next();
                    Matrix matrix = i._transform.toMatrix();
                    matrix.translate(i._position.x(), i._position.y(), i._position.z());
                    matrix.scale(i._size.x(), i._size.y(), i._size.z());
                    sBuilder.write(matrix);
                }
            }
        }
        const Buffer& vertexBuffer = _stub->_shader_bindings->vertexBuffer();
        Buffer::Snapshot vertexSnapshot = _dirty ? vertexBuffer.snapshot(buf.vertices().makeUploader()) : vertexBuffer.snapshot();
        DrawingContext drawingContext(_stub->_shader, _stub->_shader_bindings, std::move(_ubos), vertexSnapshot, buf.indices(), static_cast<int32_t>(_items.size()));
        if(buf.isInstanced())
            drawingContext._instanced_array_snapshots = buf.makeInstancedBufferSnapshots();

        return drawingContext.toRenderCommand(_stub->_resource_loader_context->objectPool());
    }
    DrawingContext drawingContext(_stub->_shader, _stub->_shader_bindings, std::move(_ubos));
    return drawingContext.toRenderCommand(_stub->_resource_loader_context->objectPool());
}

RenderLayer::RenderLayer(const sp<RenderModel>& model, const sp<Shader>& shader, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : RenderLayer(sp<Stub>::make(model, shader, resourceLoaderContext))
{
}

RenderLayer::RenderLayer(const sp<RenderLayer::Stub>& stub)
    : _stub(stub)
{
}

const sp<RenderModel>& RenderLayer::model() const
{
    return _stub->_render_model;
}

const sp<LayerContext>& RenderLayer::context() const
{
    return _stub->_layer_context;
}

RenderLayer::Snapshot RenderLayer::snapshot() const
{
    Snapshot snapshot(_stub);
    _stub->_render_model->postSnapshot(_stub->_render_controller, snapshot);
    return snapshot;
}

sp<LayerContext> RenderLayer::makeContext() const
{
    const sp<LayerContext> layerContext = sp<LayerContext>::make(_stub->_render_model, _stub->_notifier);
    _stub->_layer_contexts.push_back(layerContext);
    return layerContext;
}

void RenderLayer::render(RenderRequest& renderRequest, float x, float y)
{
    _stub->_layer_context->renderRequest(V2(x, y));
    renderRequest.addBackgroundRequest(*this, x, y);
}

RenderLayer::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_loader_context(resourceLoaderContext), _model(factory.ensureBuilder<RenderModel>(manifest, Constants::Attributes::MODEL)),
      _shader(Shader::fromDocument(factory, manifest, resourceLoaderContext)) {
}

sp<RenderLayer> RenderLayer::BUILDER::build(const sp<Scope>& args)
{
    return sp<RenderLayer>::make(_model->build(args), _shader->build(args), _resource_loader_context);
}

RenderLayer::RENDERER_BUILDER::RENDERER_BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _impl(factory, manifest, resourceLoaderContext)
{
}

sp<Renderer> RenderLayer::RENDERER_BUILDER::build(const sp<Scope>& args)
{
    return _impl.build(args);
}

}
