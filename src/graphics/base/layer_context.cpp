#include "graphics/base/layer_context.h"

#include "core/base/constants.h"
#include "core/types/global.h"
#include "core/util/log.h"
#include "core/util/updatable_util.h"

#include "graphics/base/layer.h"
#include "graphics/base/render_object.h"
#include "graphics/base/layer_context_snapshot.h"
#include "graphics/util/renderable_type.h"

#include "renderer/base/model.h"
#include "renderer/base/pipeline_input.h"
#include "renderer/base/shader.h"

namespace ark {

LayerContext::LayerContext(sp<Shader> shader, sp<ModelLoader> modelLoader, sp<Vec3> position, sp<Boolean> visible, sp<Boolean> discarded, sp<Varyings> varyings)
    : _shader(std::move(shader)), _model_loader(std::move(modelLoader)), _position(std::move(position)), _visible(std::move(visible), true), _discarded(discarded ? std::move(discarded) : nullptr, false),
      _varyings(std::move(varyings)), _layer_type(Layer::TYPE_DYNAMIC), _reload_requested(false)
{
}

bool LayerContext::update(uint64_t timestamp)
{
    return _timestamp.update(timestamp);
}

const sp<Shader>& LayerContext::shader() const
{
    return _shader;
}

const SafeVar<Vec3>& LayerContext::position() const
{
    return _position;
}

void LayerContext::setPosition(sp<Vec3> position)
{
    _position.reset(std::move(position));
    _timestamp.markDirty();
}

SafeVar<Boolean>& LayerContext::visible()
{
    return _visible;
}

const SafeVar<Boolean>& LayerContext::visible() const
{
    return _visible;
}

const SafeVar<Boolean>& LayerContext::discarded() const
{
    return _discarded;
}

const sp<ModelLoader>& LayerContext::modelLoader() const
{
    return _model_loader;
}

void LayerContext::setModelLoader(sp<ModelLoader> modelLoader)
{
    _model_loader = std::move(modelLoader);
}

Layer::Type LayerContext::layerType() const
{
    return _layer_type;
}

void LayerContext::add(sp<Renderable> renderable, sp<Updatable> isDirty, sp<Boolean> discarded)
{
    _renderable_created.push_back(RenderableType::create(std::move(renderable), std::move(isDirty), std::move(discarded)));
}

void LayerContext::clear()
{
    for(auto& [i, j] : _renderables)
        j = Renderable::RENDERABLE_STATE_DISCARDED;
    _renderable_created.clear();
}

void LayerContext::dispose()
{
    _discarded.reset(Global<Constants>()->BOOLEAN_TRUE);
}

const sp<Varyings>& LayerContext::varyings() const
{
    return _varyings;
}

void LayerContext::setVaryings(sp<Varyings> varyings)
{
    _varyings = std::move(varyings);
}

void LayerContext::markDirty()
{
    _timestamp.markDirty();
}

bool LayerContext::processNewCreated()
{
    if(_renderable_created.empty())
        return false;

    for(sp<Renderable>& i : _renderable_created)
    {
        LOGD("create: %p", i.get());
        addElementState(i.get());
        _renderables.emplace_back(std::move(i), Renderable::State(Renderable::RENDERABLE_STATE_NEW));
    }
    _renderable_created.clear();
    return true;
}

LayerContextSnapshot LayerContext::snapshot(RenderLayer renderLayer, RenderRequest& renderRequest, const PipelineInput& pipelineInput)
{
    const bool dirty = UpdatableUtil::update(renderRequest.timestamp(), _position, _visible, _discarded, _varyings);
    if(!_varyings)
        _varyings = sp<Varyings>::make(pipelineInput);
    return LayerContextSnapshot{dirty, _position.val(), _visible.val(), _discarded.val(), _varyings->snapshot(pipelineInput, renderRequest.allocator()), std::move(renderLayer)};
}

LayerContext::ElementState& LayerContext::addElementState(void* key)
{
    DASSERT(_element_states.find(key) == _element_states.end());
    return _element_states.insert(std::make_pair(key, ElementState{})).first->second;
}

LayerContext::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, Layer::Type layerType)
    : _layer(factory.getBuilder<Layer>(manifest, constants::LAYER)),
      _render_layer(_layer ? nullptr : factory.getBuilder<RenderLayer>(manifest, constants::RENDER_LAYER)), _layer_type(layerType)
{
    CHECK(_layer || _render_layer, "LayerContext must be associated with one Layer or RenderLayer");
}

sp<LayerContext> LayerContext::BUILDER::build(const Scope& args)
{
    if(_layer)
        return _layer->build(args)->context();
    const sp<RenderLayer> renderLayer = _render_layer->build(args);
    if(_layer_type != Layer::TYPE_UNSPECIFIED)
        return renderLayer->addLayerContext();
    return renderLayer->context();
}

}
