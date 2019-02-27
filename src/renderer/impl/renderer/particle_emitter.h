#ifndef ARK_RENDERER_IMPL_RENDERER_PARTICLE_EMITTER_H_
#define ARK_RENDERER_IMPL_RENDERER_PARTICLE_EMITTER_H_

#include "core/base/bean_factory.h"
#include "core/base/clock.h"
#include "core/collection/list.h"
#include "core/inf/builder.h"
#include "core/types/class.h"
#include "core/types/safe_ptr.h"
#include "core/types/shared_ptr.h"

#include "graphics/inf/renderer.h"
#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

namespace ark {

//[[core::class]]
class ParticleEmitter : public Renderer {
private:
    struct Stub {
        Stub(const sp<ResourceLoaderContext>& resourceLoaderContext, uint32_t type, const sp<Vec>& position, const sp<Size>& size, const sp<Scope>& arguments);

        sp<Scope> _arguments;

        uint32_t _type;
        SafePtr<Vec> _position;
        sp<Size> _size;

        sp<ObjectPool> _object_pool;
    };

public:
    ParticleEmitter(const sp<Stub>& stub, const sp<Clock>& clock, const sp<RenderLayer>& renderLayer, const List<document>& particleDescriptor, BeanFactory& beanFactory);

    virtual void render(RenderRequest& renderRequest, float x, float y) override;

//  [[plugin::resource-loader("particle-emitter")]]
    class BUILDER : public Builder<Renderer> {
    public:
        BUILDER(BeanFactory& parent, const document& doc, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Renderer> build(const sp<Scope>& args) override;

    private:
        BeanFactory _factory;
        document _manifest;

        sp<ResourceLoaderContext> _resource_loader_context;

        sp<Clock> _clock;
        sp<Builder<Numeric>> _type;
        SafePtr<Builder<Vec>> _position;
        SafePtr<Builder<Size>> _size;
        sp<Builder<RenderLayer>> _render_layer;

    };

private:
    class Iteration {
    public:
        Iteration(BeanFactory& factory, const document& manifest);

        void doIteration(const sp<Scope>& scope, const sp<ObjectPool>& objectPool, const sp<Numeric>& duration, uint64_t baseline);

        uint32_t count() const;

    private:
        String _name;
        uint32_t _count;

        List<std::pair<String, sp<Builder<Numeric>>>> _numerics;
    };

    class Particale {
    public:
        Particale(const sp<Stub>& stub, const document& doc, BeanFactory& args);
        Particale(const Particale& other) = default;

        uint64_t show(float x, float y, const sp<Clock>& clock, uint64_t tick, const sp<RenderLayer>& renderLayer);

    private:
        sp<Vec> makePosition(ObjectPool& objectPool, float x, float y) const;

    private:
        sp<Stub> _stub;

        sp<Builder<Numeric>> _type;
        sp<Builder<Vec>> _position;
        sp<Builder<Size>> _size;
        SafePtr<Builder<Transform>> _transform;
        SafePtr<Builder<Varyings>> _varyings;
        sp<Builder<Disposable>> _lifecycle;

        sp<Iteration> _iteration;

        uint64_t _interval;

        float _x, _y;
        uint64_t last_emit_tick;
    };

    uint64_t emitParticles(uint64_t tick);

private:
    sp<Stub> _stub;
    sp<RenderLayer> _render_layer;
    List<Particale> _particles;
    sp<Clock> _clock;

    uint64_t _next_tick;

    friend class BUILDER;
    friend class Particale;

};

}

#endif
