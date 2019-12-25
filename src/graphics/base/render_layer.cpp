#include "graphics/base/render_layer.h"

#include "core/base/observer.h"
#include "core/epi/notifier.h"

#include "graphics/base/camera.h"
#include "graphics/base/layer_context.h"
#include "graphics/base/render_object.h"
#include "graphics/base/render_request.h"
#include "graphics/base/size.h"
#include "graphics/base/v4.h"
#include "graphics/util/matrix_util.h"

#include "renderer/base/drawing_context.h"
#include "renderer/base/drawing_buffer.h"
#include "renderer/base/model.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/pipeline_input.h"
#include "renderer/base/render_context.h"
#include "renderer/base/render_engine.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/shader.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/base/vertex_stream.h"
#include "renderer/inf/model_loader.h"

#include "renderer/inf/pipeline.h"
#include "renderer/inf/pipeline_factory.h"
#include "renderer/inf/render_model.h"

namespace ark {

RenderLayer::Stub::Stub(const sp<RenderModel>& renderModel, const sp<Shader>& shader, const sp<Vec4>& scissor, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _render_model(renderModel), _shader(shader), _scissor(scissor), _resource_loader_context(resourceLoaderContext),
      _render_controller(resourceLoaderContext->renderController()), _shader_bindings(_render_model->makeShaderBindings(_shader)), _notifier(sp<Notifier>::make()),
      _dirty(_notifier->createDirtyFlag()), _layer(sp<Layer>::make(sp<LayerContext>::make(renderModel, _notifier, Layer::TYPE_DYNAMIC))), _stride(shader->input()->getStream(0).stride())
{
    DCHECK(!_scissor || _shader_bindings->pipelineBindings()->hasFlag(PipelineBindings::FLAG_DYNAMIC_SCISSOR, PipelineBindings::FLAG_DYNAMIC_SCISSOR_BITMASK), "RenderLayer has a scissor while its Shader has no FLAG_DYNAMIC_SCISSOR set");
}

RenderLayer::Snapshot::Snapshot(RenderRequest& renderRequest, const sp<Stub>& stub)
    : _stub(stub)
{
    _stub->_layer->context()->takeSnapshot(*this, renderRequest);

    Layer::Type combined = Layer::TYPE_UNSPECIFIED;

    for(const sp<LayerContext>& i : stub->_layer_contexts)
    {
        i->takeSnapshot(*this, renderRequest);
        DWARN(combined != Layer::TYPE_STATIC || i->layerType() != Layer::TYPE_DYNAMIC, "Combining static and dynamic layers together leads to low efficiency");
        if(combined != Layer::TYPE_DYNAMIC)
            combined = i->layerType();
    }

    _stub->_render_model->postSnapshot(_stub->_render_controller, *this);

    _ubos = _stub->_shader->snapshot(renderRequest.allocator());

    if(_stub->_scissor && _stub->_scissor->update(renderRequest.timestamp()))
    {
        V4 s = _stub->_scissor->val();
        _scissor = Rect(s.x(), s.y(), s.z(), s.w());
    }

    const Buffer& vbo = _stub->_shader_bindings->vertexBuffer();

    bool dirty = _stub->_dirty->val();
    if(vbo.size() == 0)
        _flag = SNAPSHOT_FLAG_RELOAD;
    else if(combined != Layer::TYPE_STATIC)
        _flag = dirty ? SNAPSHOT_FLAG_RELOAD : SNAPSHOT_FLAG_DYNAMIC_UPDATE;
    else
        _flag = dirty ? SNAPSHOT_FLAG_STATIC_MODIFIED : SNAPSHOT_FLAG_STATIC_REUSE;
}

void RenderLayer::Snapshot::doRenderModelSnapshot(const RenderRequest& renderRequest, DrawingBuffer& buf) const
{
    _stub->_render_model->start(buf, *this);
    if(_flag == SNAPSHOT_FLAG_RELOAD)
    {
        VertexStream writer = buf.makeVertexStream(renderRequest, buf.vertices()._grow_capacity, 0);
        for(const Renderable::Snapshot& i : _items)
        {
            writer.setRenderObject(i);
            writer.setTranslate(i._position);
            _stub->_render_model->load(writer, i);
            if(buf.isInstanced())
            {
                Buffer::Builder& sBuilder = buf.getInstancedArrayBuilder(1);
                sBuilder.next();
                M4 matrix = MatrixUtil::scale(MatrixUtil::translate(i._transform.toMatrix(), i._position), i._size);
                sBuilder.write(matrix);
            }
        }
    }
    else
    {
        size_t step = buf.vertices()._grow_capacity / _items.size();
        size_t offset = 0;
        for(const Renderable::Snapshot& i : _items)
        {
            if(i._dirty)
            {
                VertexStream writer = buf.makeVertexStream(renderRequest, step, offset);
                writer.setRenderObject(i);
                writer.setTranslate(i._position);
                _stub->_render_model->load(writer, i);
            }
            offset += step;
        }
    }
}

void RenderLayer::Snapshot::doModelLoaderSnapshot(const RenderRequest& renderRequest, DrawingBuffer& buf) const
{
    const sp<Model>& unitModel = _stub->_model_loader->unitModel();
    if(unitModel)
    {
        size_t bufferSize = unitModel->vertices()->size() * _items.size();
    }
}

sp<RenderCommand> RenderLayer::Snapshot::render(const RenderRequest& renderRequest, const V3& /*position*/)
{
    if(_items.size() > 0)
    {
        DrawingBuffer buf(renderRequest, _stub->_shader_bindings, _stub->_stride);

        doRenderModelSnapshot(renderRequest, buf);

        DrawingContext drawingContext(_stub->_shader, _stub->_shader_bindings, std::move(_ubos),
                                      buf.vertices().toSnapshot(_stub->_shader_bindings->vertexBuffer()),
                                      buf.indices(),
                                      static_cast<int32_t>(_items.size()));

        if(_stub->_scissor)
            drawingContext._parameters._scissor = _stub->_render_controller->renderEngine()->toRendererScissor(_scissor);

        if(buf.isInstanced())
            drawingContext._instanced_array_snapshots = buf.makeDividedBufferSnapshots();

        return drawingContext.toRenderCommand();
    }
    DrawingContext drawingContext(_stub->_shader, _stub->_shader_bindings, std::move(_ubos));
    return drawingContext.toRenderCommand();
}

RenderLayer::RenderLayer(const sp<RenderModel>& model, const sp<Shader>& shader, const sp<Vec4>& scissor, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : RenderLayer(sp<Stub>::make(model, shader, scissor, resourceLoaderContext))
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
    return _stub->_layer->context();
}

RenderLayer::Snapshot RenderLayer::snapshot(RenderRequest& renderRequest) const
{
    return Snapshot(renderRequest, _stub);
}

const sp<Layer>& RenderLayer::layer() const
{
    return _stub->_layer;
}

sp<LayerContext> RenderLayer::makeContext(Layer::Type layerType) const
{
    const sp<LayerContext> layerContext = sp<LayerContext>::make(_stub->_render_model, _stub->_notifier, layerType);
    _stub->_layer_contexts.push_back(layerContext);
    return layerContext;
}

sp<Layer> RenderLayer::makeLayer(Layer::Type layerType) const
{
    return sp<Layer>::make(makeContext(layerType));
}

void RenderLayer::render(RenderRequest& renderRequest, const V3& position)
{
    _stub->_layer->render(renderRequest, position);
    renderRequest.addBackgroundRequest(*this, position);
}

RenderLayer::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : BUILDER(factory, manifest, resourceLoaderContext, factory.ensureBuilder<RenderModel>(manifest, Constants::Attributes::MODEL))
{
}

RenderLayer::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext, sp<Builder<RenderModel>> renderModel, sp<Builder<Shader>> shader)
    : _resource_loader_context(resourceLoaderContext), _model(std::move(renderModel)),
      _shader(shader ? std::move(shader) : Shader::fromDocument(factory, manifest, resourceLoaderContext)), _scissor(factory.getBuilder<Vec4>(manifest, "scissor"))
{
}

sp<RenderLayer> RenderLayer::BUILDER::build(const Scope& args)
{
    return sp<RenderLayer>::make(_model->build(args), _shader->build(args), _scissor->build(args), _resource_loader_context);
}

RenderLayer::RENDERER_BUILDER::RENDERER_BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _impl(factory, manifest, resourceLoaderContext)
{
}

sp<Renderer> RenderLayer::RENDERER_BUILDER::build(const Scope& args)
{
    return _impl.build(args);
}

}
