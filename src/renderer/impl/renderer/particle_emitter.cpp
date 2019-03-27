#include "renderer/impl/renderer/particle_emitter.h"

#include "core/ark.h"
#include "core/base/clock.h"
#include "core/base/object_pool.h"
#include "core/dom/dom_document.h"
#include "core/impl/boolean/boolean_by_timeout.h"
#include "core/impl/variable/variable_op2.h"
#include "core/util/bean_utils.h"
#include "core/util/operators.h"

#include "graphics/base/layer.h"
#include "graphics/base/render_object.h"
#include "graphics/base/v2.h"
#include "graphics/impl/vec/vec_with_translation.h"
#include "graphics/impl/vec/vec2_impl.h"

#include "renderer/base/resource_loader_context.h"


namespace ark {

ParticleEmitter::ParticleEmitter(const sp<Stub>& stub, const sp<Clock>& clock,
                                 const sp<LayerContext>& layerContext, const List<document>& particleDescriptor, BeanFactory& beanFactory)
    : _stub(stub), _layer_context(layerContext), _clock(clock), _next_tick(0)
{
    for(const document& doc : particleDescriptor)
        _particles.push_back(Particale(_stub, doc, beanFactory));
}

void ParticleEmitter::render(RenderRequest& /*renderRequest*/, float /*x*/, float /*y*/)
{
    uint64_t tick = _clock->tick();
    if(tick > _next_tick)
        _next_tick = emitParticles(tick);
}

uint64_t ParticleEmitter::emitParticles(uint64_t tick)
{
    uint64_t nextTick = std::numeric_limits<std::uint64_t>::max();
    const V position = _stub->_position->val();
    for(Particale& i : _particles)
        nextTick = std::min(i.show(position.x(), position.y(), _clock, tick, _layer_context), nextTick);
    return nextTick;
}

ParticleEmitter::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _factory(factory), _manifest(manifest), _resource_loader_context(resourceLoaderContext),
      _clock(Ark::instance().clock()),
      _type(factory.getBuilder<Numeric>(manifest, Constants::Attributes::TYPE)),
      _position(factory.getBuilder<Vec>(manifest, Constants::Attributes::POSITION)),
      _size(factory.getBuilder<Size>(manifest, Constants::Attributes::SIZE)),
      _layer_context(sp<LayerContext::BUILDER>::make(factory, manifest, false))
{
}

sp<Renderer> ParticleEmitter::BUILDER::build(const sp<Scope>& args)
{
    const sp<Stub> stub = sp<Stub>::make(_resource_loader_context, BeanUtils::toInteger(_type, args), _position->build(args), _size->build(args), args);
    return sp<Renderer>::adopt(new ParticleEmitter(stub, _clock, _layer_context->build(args), _manifest->children(), _factory));
}

ParticleEmitter::Particale::Particale(const sp<Stub>& stub, const document& manifest, BeanFactory& factory)
    : _stub(stub), last_emit_tick(0)
{
    const document im = manifest->getChild("iteration");
    if(im)
        _iteration = sp<Iteration>::make(factory, im);
    _interval = Documents::ensureAttribute<Clock::Interval>(manifest, Constants::Attributes::INTERVAL).usec();
    DCHECK(_interval, "Interval must be greater than 0");

    _type = factory.getBuilder<Numeric>(manifest, Constants::Attributes::TYPE);
    _position = factory.getBuilder<Vec>(manifest, Constants::Attributes::POSITION);
    _size = factory.getBuilder<Size>(manifest, Constants::Attributes::SIZE);
    _transform = factory.getBuilder<Transform>(manifest, Constants::Attributes::TRANSFORM);
    _varyings = factory.getBuilder<Varyings>(manifest, Constants::Attributes::VARYINGS);
    _lifecycle = factory.ensureBuilder<Disposed>(manifest, Constants::Attributes::EXPIRED);
}

uint64_t ParticleEmitter::Particale::show(float x, float y, const sp<Clock>& clock, uint64_t tick, const sp<LayerContext>& layerContext)
{
    uint32_t count;
    uint64_t elpased = tick - last_emit_tick;
    uint64_t dt, let = 0;
    float dx, dy;
    if(last_emit_tick)
    {
        uint32_t ic = static_cast<uint32_t>(elpased / _interval);
        count = ic * (_iteration ? _iteration->count() : 1);
        dx = (x - _x) / count;
        dy = (y - _y) / count;
        dt = elpased / count;
        last_emit_tick += (_interval * ic);
    }
    else
    {
        _x = x;
        _y = y;
        last_emit_tick = tick;
        return last_emit_tick + _interval;
    }

    uint32_t type = _type ? BeanUtils::toInteger(_type, _stub->_arguments) : _stub->_type;
    const sp<Size> size = _size ? _size->build(_stub->_arguments) : _stub->_size;
    const sp<Transform> transform = _transform->build(_stub->_arguments);
    const sp<Varyings> filter = _varyings->build(_stub->_arguments);
    const sp<Numeric> duration = clock->duration();

    for(uint32_t i = 0; i < count; i++)
    {
        if(_iteration)
        {
            _iteration->doIteration(_stub->_arguments, _stub->_object_pool, duration, let);
            let += dt;
        }

        _x += dx;
        _y += dy;
        const sp<Vec> position = makePosition(_stub->_object_pool, _x , _y);
        const sp<Disposed> lifecycle = _lifecycle->build(_stub->_arguments);
        const sp<RenderObject> renderObject = _stub->_object_pool->obtain<RenderObject>(
                    type, position,
                    size, transform, filter);
        DWARN(lifecycle, "You're creating particles that will NEVER die, is that what you really want?");
        layerContext->addRenderObject(renderObject, lifecycle);
    }
    return last_emit_tick + _interval;
}

sp<Vec> ParticleEmitter::Particale::makePosition(ObjectPool& objectPool, float x, float y) const
{
    if(_position)
        return objectPool.obtain<VecWithTranslation<V>>(_position->build(_stub->_arguments), V(x, y));
    return objectPool.obtain<VecImpl>(x, y);
}

ParticleEmitter::Stub::Stub(const sp<ResourceLoaderContext>& resourceLoaderContext, uint32_t type, const sp<Vec>& position, const sp<Size>& size, const sp<Scope>& arguments)
    : _arguments(sp<Scope>::make(arguments)), _type(type), _position(position), _size(size),
      _object_pool(resourceLoaderContext->objectPool())
{
}

ParticleEmitter::Iteration::Iteration(BeanFactory& factory, const document& manifest)
    : _name(Documents::ensureAttribute(manifest, Constants::Attributes::NAME)), _count(Documents::ensureAttribute<uint32_t>(manifest, "count"))
{
    for(const document& i : manifest->children("numeric"))
    {
        const String& name = Documents::ensureAttribute(i, Constants::Attributes::NAME);
        const sp<Builder<Numeric>> builder = factory.ensureBuilder<Numeric>(Documents::ensureAttribute(i, Constants::Attributes::VALUE));
        _numerics.push_back(std::pair<String, sp<Builder<Numeric>>>(name, builder));
    }
}

void ParticleEmitter::Iteration::doIteration(const sp<Scope>& scope, const sp<ObjectPool>& objectPool, const sp<Numeric>& duration, uint64_t baseline)
{
    float translate = baseline / 1000000.0f;
    scope->put<Numeric>(_name, objectPool->obtain<VariableOP2<float, float, Operators::Add<float>, sp<Numeric>, float>>(duration, translate));
    for(const auto& i : _numerics)
        scope->put(i.first, i.second->build(scope));
}

uint32_t ParticleEmitter::Iteration::count() const
{
    return _count;
}

}
