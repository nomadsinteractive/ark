#include "renderer/impl/renderer/emitter.h"

#include "core/ark.h"
#include "core/base/clock.h"
#include "core/base/object_pool.h"
#include "core/dom/dom_document.h"
#include "core/impl/boolean/boolean_by_timeout.h"
#include "core/impl/boolean/boolean_by_weak_ref.h"
#include "core/impl/variable/variable_op2.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/util/boolean_util.h"
#include "core/util/holder_util.h"
#include "core/util/operators.h"

#include "graphics/base/layer.h"
#include "graphics/base/layer_context.h"
#include "graphics/base/render_object.h"
#include "graphics/base/size.h"
#include "graphics/impl/vec/vec3_impl.h"
#include "graphics/util/vec3_util.h"

#include "renderer/base/resource_loader_context.h"


namespace ark {

Emitter::Emitter(const sp<ResourceLoaderContext>& resourceLoaderContext, const sp<Source>& source, const sp<Clock>& clock, const sp<LayerContext>& layerContext, const std::vector<document>& particleDescriptor, BeanFactory& beanFactory)
    : _stub(sp<Stub>::make(clock, layerContext, source, particleDescriptor, beanFactory)), _render_controller(resourceLoaderContext->renderController()), _disposed(sp<BooleanWrapper>::make(true))
{
}

void Emitter::traverse(const Holder::Visitor& visitor)
{
    HolderUtil::visit(_stub->_source->_position, visitor);
    HolderUtil::visit(_stub->_source->_size, visitor);
    HolderUtil::visit(_stub->_source->_type, visitor);
    HolderUtil::visit(_stub->_source->_arguments, visitor);
}

bool Emitter::active()
{
    return !_disposed->val();
}

void Emitter::setActive(bool active)
{
    if(_disposed->val() == active)
    {
        _disposed->set(!active);
        if(active)
            doActivate();
    }
}

void Emitter::activate()
{
    DWARN(_disposed->val(), "Emitter activated already");
    if(_disposed->val())
    {
        doActivate();
        _disposed->set(false);
    }
}

void Emitter::deactivate()
{
    DWARN(!_disposed->val(), "Emitter has not been activated");
    _disposed->set(true);
}

void Emitter::doActivate()
{
    _render_controller->addPreUpdateRequest(_stub, BooleanUtil::__or__(_disposed, sp<BooleanByWeakRef<Boolean>>::make(_disposed, 1)));
}

Emitter::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _factory(factory), _manifest(manifest), _resource_loader_context(resourceLoaderContext),
      _type(factory.getBuilder<Integer>(manifest, Constants::Attributes::TYPE)),
      _position(factory.getBuilder<Vec3>(manifest, Constants::Attributes::POSITION)),
      _size(factory.getBuilder<Size>(manifest, Constants::Attributes::SIZE)),
      _layer_context(sp<LayerContext::BUILDER>::make(factory, manifest, false))
{
}

sp<Emitter> Emitter::BUILDER::build(const sp<Scope>& args)
{
    const sp<Source> stub = sp<Source>::make(_resource_loader_context, _type->build(args), _position->build(args), _size->build(args), args);
    return sp<Emitter>::make(_resource_loader_context, stub, Ark::instance().clock(), _layer_context->build(args), _manifest->children(), _factory);
}

Emitter::Particale::Particale(const sp<Source>& stub, const document& manifest, BeanFactory& factory)
    : _stub(stub), _last_emit_tick(0)
{
    _interval = Documents::ensureAttribute<Clock::Interval>(manifest, Constants::Attributes::INTERVAL).usec();
    DCHECK(_interval, "Interval must be greater than 0");

    _type = factory.getBuilder<Integer>(manifest, Constants::Attributes::TYPE);
    _position = factory.getBuilder<Vec3>(manifest, Constants::Attributes::POSITION);
    _size = factory.getBuilder<Size>(manifest, Constants::Attributes::SIZE);
    _transform = factory.getBuilder<Transform>(manifest, Constants::Attributes::TRANSFORM);
    _varyings = factory.getBuilder<Varyings>(manifest, Constants::Attributes::VARYINGS);
    _disposed = factory.ensureBuilder<Boolean>(manifest, Constants::Attributes::DISPOSED);
}

uint64_t Emitter::Particale::show(const V3& position, uint64_t tick, const sp<LayerContext>& layerContext)
{
    uint32_t iteration = 1;
    uint64_t elpased = tick - _last_emit_tick;
    uint64_t dt;
    V3 delta;
    if(_last_emit_tick)
    {
        uint32_t ic = static_cast<uint32_t>(elpased / _interval);
        delta = (position - _source_position) / static_cast<float>(iteration);
        dt = elpased / iteration;
        _last_emit_tick += (_interval * ic);
    }
    else
    {
        _source_position = position;
        _last_emit_tick = tick;
        return _last_emit_tick + _interval;
    }

    const sp<Integer> type = _type ? _type->build(_stub->_arguments) : static_cast<sp<Integer>>(_stub->_type);
    const sp<Size> size = _size ? _size->build(_stub->_arguments) : _stub->_size;
    const sp<Transform> transform = _transform->build(_stub->_arguments);
    const sp<Varyings> varyings = _varyings->build(_stub->_arguments);

    for(uint32_t i = 0; i < iteration; i++)
    {
        _source_position += delta;
        const sp<Vec3> position = makePosition(_stub->_object_pool, _source_position);
        const sp<Boolean> disposed = _disposed->build(_stub->_arguments);
        const sp<RenderObject> renderObject = _stub->_object_pool->obtain<RenderObject>(type, position, size, transform, varyings);
        DWARN(disposed, "You're creating particles that will NEVER die, is that what you really want?");
        layerContext->addRenderObject(renderObject, disposed);
    }
    return _last_emit_tick + _interval;
}

sp<Vec3> Emitter::Particale::makePosition(ObjectPool& objectPool, const V3& position) const
{
    if(_position)
        return objectPool.obtain<VariableOP2<V3, V3, Operators::Add<V3>, sp<Vec3>, V3>>(_position->build(_stub->_arguments), position);
    return objectPool.obtain<Vec3Impl>(position.x(), position.y(), position.z());
}

Emitter::Source::Source(const sp<ResourceLoaderContext>& resourceLoaderContext, const sp<Integer>& type, const sp<Vec3>& position, const sp<Size>& size, const sp<Scope>& arguments)
    : _arguments(sp<Scope>::make(arguments)), _type(type), _position(position), _size(size),
      _object_pool(resourceLoaderContext->objectPool())
{
}

Emitter::Stub::Stub(const sp<Clock>& clock, const sp<LayerContext>& layerContext, const sp<Emitter::Source>& source, const std::vector<document>& particleDescriptor, BeanFactory& beanFactory)
    : _clock(clock), _layer_context(layerContext), _source(source), _next_tick(0)
{
    for(const document& i : particleDescriptor)
        _particles.push_back(Particale(_source, i, beanFactory));
}

void Emitter::Stub::run()
{
    uint64_t tick = _clock->tick();
    if(tick > _next_tick)
        _next_tick = emit(tick);
}

uint64_t Emitter::Stub::emit(uint64_t tick)
{
    uint64_t nextTick = std::numeric_limits<std::uint64_t>::max();
    const V3 position = _source->_position->val();
    for(Particale& i : _particles)
        nextTick = std::min(i.show(position, tick, _layer_context), nextTick);
    return nextTick;
}

}
