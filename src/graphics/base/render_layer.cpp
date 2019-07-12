#include "graphics/base/render_layer.h"

#include "core/base/observer.h"
#include "core/epi/notifier.h"

#include "graphics/base/camera.h"
#include "graphics/base/layer_context.h"
#include "graphics/base/render_object.h"
#include "graphics/base/render_request.h"
#include "graphics/base/size.h"

#include "renderer/base/drawing_context.h"
#include "renderer/base/drawing_buffer.h"
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
      _render_controller(resourceLoaderContext->renderController()), _shader_bindings(_render_model->makeShaderBindings(_shader)), _notifier(sp<Notifier>::make()),
      _dirty(_notifier->createDirtyFlag()), _layer_context(sp<LayerContext>::make(renderModel, _notifier, Layer::TYPE_DYNAMIC)), _stride(shader->input()->getStream(0).stride())
{
}

RenderLayer::Snapshot::Snapshot(const sp<Stub>& stub)
    : _stub(stub)
{
    _stub->_layer_context->takeSnapshot(*this, stub->_memory_pool);

    Layer::Type combined = Layer::TYPE_UNSPECIFIED;

    for(const sp<LayerContext>& i : stub->_layer_contexts)
    {
        i->takeSnapshot(*this, stub->_memory_pool);
        DWARN(combined != Layer::TYPE_STATIC || i->layerType() != Layer::TYPE_DYNAMIC, "Combining static and dynamic layers together leads to low efficiency");
        if(combined != Layer::TYPE_DYNAMIC)
            combined = i->layerType();
    }

    _stub->_render_model->postSnapshot(_stub->_render_controller, *this);

    _ubos = _stub->_shader->snapshot(_stub->_memory_pool);

    if(combined == Layer::TYPE_DYNAMIC)
        _flag = SNAPSHOT_FLAG_DYNAMIC;
    else
    {
        bool dirty = _stub->_dirty->val();
        const Buffer& vbo = _stub->_shader_bindings->vertexBuffer();
        const Buffer& ibo = _stub->_shader_bindings->indexBuffer();
        if(vbo.size() == 0 || ibo.size() == 0)
            _flag = SNAPSHOT_FLAG_STATIC_INITIALIZE;
        else
            _flag = dirty ? SNAPSHOT_FLAG_STATIC_MODIFIED : SNAPSHOT_FLAG_STATIC_REUSE;
    }
}

sp<RenderCommand> RenderLayer::Snapshot::render(float x, float y)
{
    if(_items.size() > 0)
    {
        DrawingBuffer buf(_stub->_resource_loader_context, _stub->_shader_bindings, _items.size(), _stub->_stride);
        _stub->_render_model->start(buf, *this);

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

        DrawingContext drawingContext(_stub->_shader, _stub->_shader_bindings, std::move(_ubos),
                                      buf.vertices().toSnapshot(_stub->_shader_bindings->vertexBuffer()),
                                      buf.indices(),
                                      static_cast<int32_t>(_items.size()));
        if(buf.isInstanced())
            drawingContext._instanced_array_snapshots = buf.makeDividedBufferSnapshots();

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
    return Snapshot(_stub);
}

sp<LayerContext> RenderLayer::makeContext(Layer::Type layerType) const
{
    const sp<LayerContext> layerContext = sp<LayerContext>::make(_stub->_render_model, _stub->_notifier, layerType);
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
