#include "graphics/components/layer.h"

#include "core/base/bean_factory.h"

#include "graphics/base/layer_context.h"
#include "graphics/base/render_command_pipeline.h"
#include "graphics/base/render_layer.h"
#include "graphics/components/render_object.h"
#include "graphics/util/renderable_type.h"

#include "renderer/impl/model_loader/model_loader_cached.h"

namespace ark {

Layer::Layer()
    : Layer(nullptr)
{
}

Layer::Layer(sp<LayerContext> layerContext)
    : _layer_context(layerContext ? std::move(layerContext) : sp<LayerContext>::make())
{
}

void Layer::discard()
{
    _layer_context = nullptr;
}

const sp<Shader>& Layer::shader() const
{
    return _layer_context->shader();
}

const OptionalVar<Vec3>& Layer::position() const
{
    return _layer_context->position();
}

void Layer::setPosition(sp<Vec3> position)
{
    _layer_context->setPosition(std::move(position));
}

const OptionalVar<Boolean>& Layer::visible() const
{
    return _layer_context->visible();
}

void Layer::setVisible(sp<Boolean> visible)
{
    _layer_context->visible().reset(std::move(visible));
}

const OptionalVar<Boolean>& Layer::discarded() const
{
    return _layer_context->discarded();
}

sp<ModelLoader> Layer::modelLoader() const
{
    return _layer_context ? _layer_context->modelLoader() : nullptr;
}

const sp<LayerContext>& Layer::context() const
{
    return _layer_context;
}

void Layer::add(const sp<RenderObject>& renderObject, sp<Boolean> discarded, sp<Updatable> updatable, const enums::InsertPosition insertPosition)
{
    if(insertPosition == enums::INSERT_POSITION_BACK)
        _layer_context->pushBack(RenderableType::create(renderObject, discarded ? std::move(discarded) : sp<Boolean>(renderObject->discarded()), std::move(updatable)));
    else
    {
        ASSERT(insertPosition == enums::INSERT_POSITION_FRONT);
        _layer_context->pushFront(RenderableType::create(renderObject, discarded ? std::move(discarded) : sp<Boolean>(renderObject->discarded()), std::move(updatable)));
    }
}

void Layer::clear()
{
    _layer_context->clear();
}

}
