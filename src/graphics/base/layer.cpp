#include "graphics/base/layer.h"

#include "graphics/base/layer_context.h"
#include "graphics/base/render_object.h"
#include "graphics/base/render_request.h"
#include "graphics/base/size.h"

#include "renderer/base/resource_loader_context.h"
#include "renderer/base/drawing_context.h"
#include "renderer/base/model_buffer.h"
#include "renderer/base/shader.h"
#include "renderer/base/shader_bindings.h"

#include "renderer/inf/render_model.h"
#include "renderer/inf/pipeline.h"
#include "renderer/inf/pipeline_factory.h"

namespace ark {

Layer::Stub::Stub(const sp<RenderModel>& model, const sp<Shader>& shader, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _model(model), _shader(shader), _resource_loader_context(resourceLoaderContext), _memory_pool(resourceLoaderContext->memoryPool()),
      _render_controller(resourceLoaderContext->renderController()), _shader_bindings(_model->makeShaderBindings(_render_controller, shader->pipelineLayout())),
      _last_rendered_count(0)
{
}

Layer::Item::Item(float x, float y, const sp<RenderObject>& renderObject)
    : x(x), y(y), _render_object(renderObject)
{
}

Layer::Snapshot::Snapshot(const sp<Stub>& stub)
    : _stub(stub), _ubos(stub->_shader->snapshot(_stub->_memory_pool)), _dirty(stub->_items.size() != stub->_last_rendered_count)
{
    for(const Item& i : stub->_items)
    {
        RenderObject::Snapshot snapshot = i._render_object->snapshot(stub->_memory_pool);
        snapshot._position = V(snapshot._position.x() + i.x, snapshot._position.y() + i.y, snapshot._position.z());
        _items.push_back(snapshot);
    }

    size_t size = stub->_layer_contexts.size();
    for(const sp<LayerContext>& i : stub->_layer_contexts)
        _dirty = i->takeSnapshot(*this, stub->_memory_pool) || _dirty;
    _dirty = _dirty || size != stub->_layer_contexts.size();

    _stub->_last_rendered_count = _stub->_items.size();
    _stub->_items.clear();
}

sp<RenderCommand> Layer::Snapshot::render(float x, float y)
{
    if(_items.size() > 0)
    {
        ModelBuffer buf(_stub->_resource_loader_context, _stub->_shader_bindings, _items.size(), _stub->_shader->stride());
        _stub->_model->start(buf, _stub->_render_controller, *this);

        for(const auto& i : _items)
        {
            buf.setRenderObject(i);
            buf.setTranslate(V3(x + i._position.x(), y + i._position.y(), i._position.z()));
            _stub->_model->load(buf, i._type, i._size);
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
        DrawingContext drawingContext(_stub->_shader, _stub->_shader_bindings, std::move(_ubos), _stub->_shader_bindings->vertexBuffer().snapshot(buf.vertices().makeUploader()), buf.indices(), static_cast<int32_t>(_items.size()));
        if(buf.isInstanced())
            drawingContext._instanced_array_snapshots = buf.makeInstancedBufferSnapshots();

        return drawingContext.toRenderCommand(_stub->_resource_loader_context->objectPool());
    }
    return nullptr;
}

Layer::Layer(const sp<RenderModel>& model, const sp<Shader>& shader, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _stub(sp<Stub>::make(model, shader, resourceLoaderContext))
{
}

void Layer::draw(float x, float y, const sp<RenderObject>& renderObject)
{
    _stub->_items.emplace_back(x, y, renderObject);
}

const sp<RenderModel>& Layer::model() const
{
    return _stub->_model;
}

Layer::Snapshot Layer::snapshot() const
{
    Snapshot snapshot(_stub);
    _stub->_model->postSnapshot(_stub->_render_controller, snapshot);
    return snapshot;
}

sp<LayerContext> Layer::makeContext()
{
    const sp<LayerContext> layerContext = sp<LayerContext>::make();
    _stub->_layer_contexts.push_back(layerContext);
    return layerContext;
}

void Layer::measure(int32_t type, Size& size)
{
    const Metrics metrics = _stub->_model->measure(type);
    size.setWidth(metrics.size.x());
    size.setHeight(metrics.size.y());
}

void Layer::render(RenderRequest& renderRequest, float x, float y)
{
    renderRequest.addBackgroundRequest(*this, x, y);
}

Layer::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_loader_context(resourceLoaderContext), _model(factory.ensureBuilder<RenderModel>(manifest, Constants::Attributes::MODEL)),
      _shader(Shader::fromDocument(factory, manifest, resourceLoaderContext)) {
}

sp<Layer> Layer::BUILDER::build(const sp<Scope>& args)
{
    return sp<Layer>::make(_model->build(args), _shader->build(args), _resource_loader_context);
}

}
