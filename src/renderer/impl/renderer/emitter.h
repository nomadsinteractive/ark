#ifndef ARK_RENDERER_IMPL_RENDERER_EMITTER_H_
#define ARK_RENDERER_IMPL_RENDERER_EMITTER_H_

#include <vector>

#include "core/base/api.h"
#include "core/base/bean_factory.h"
#include "core/inf/builder.h"
#include "core/inf/holder.h"
#include "core/inf/runnable.h"
#include "core/epi/disposed.h"
#include "core/types/safe_ptr.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v3.h"
#include "graphics/inf/renderer.h"

#include "renderer/forwarding.h"

namespace ark {

//[[script::bindings::holder]]
class ARK_API Emitter : public Holder, public Renderer, public Disposed {
private:
    struct Source {
        Source(const sp<Integer>& type, const sp<Vec3>& position, const sp<Size>& size, const Scope& arguments);

        Scope _arguments;

        SafePtr<Integer> _type;
        SafePtr<Vec3> _position;
        sp<Size> _size;
    };

public:
    Emitter(const sp<ResourceLoaderContext>& resourceLoaderContext, const sp<Source>& source, const sp<Clock>& clock, const sp<LayerContext>& layerContext, const std::vector<document>& particleDescriptor, BeanFactory& beanFactory, bool disposed);

    virtual void traverse(const Visitor& visitor) override;
    virtual void render(RenderRequest& renderRequest, const V3& position) override;

//  [[script::bindings::property]]
    bool active();
//  [[script::bindings::property]]
    void setActive(bool active);

//  [[script::bindings::auto]]
    void activate();
//  [[script::bindings::auto]]
    void deactivate();

//  [[plugin::resource-loader]]
    class BUILDER : public Builder<Emitter> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext, bool disposed = true);

        virtual sp<Emitter> build(const Scope& args) override;

    private:
        BeanFactory _factory;
        document _manifest;

        sp<ResourceLoaderContext> _resource_loader_context;

        sp<Builder<Integer>> _type;
        SafePtr<Builder<Vec3>> _position;
        SafePtr<Builder<Size>> _size;
        sp<Builder<LayerContext>> _layer_context;
        bool _disposed;
    };

//  [[plugin::resource-loader("emitter")]]
    class RENDERER_BUILDER : public Builder<Renderer> {
    public:
        RENDERER_BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        BUILDER _delegate;
    };

private:
    class Particale {
    public:
        Particale(const sp<Source>& stub, const document& doc, BeanFactory& factory);
        Particale(const Particale& other) = default;

        uint64_t show(const V3& position, uint64_t tick, const sp<LayerContext>& layerContext);

    private:
        sp<Vec3> makePosition(const V3& position) const;

    private:
        sp<Source> _stub;

        sp<Builder<Integer>> _type;
        sp<Builder<Vec3>> _position;
        sp<Builder<Size>> _size;
        SafePtr<Builder<Transform>> _transform;
        SafePtr<Builder<Varyings>> _varyings;
        sp<Builder<Boolean>> _disposed;

        uint64_t _interval;
        uint64_t _last_emit_tick;
        V3 _source_position;
    };

    class Stub : public Runnable {
    public:
        Stub(const sp<Clock>& clock, const sp<LayerContext>& layerContext, const sp<Source>& source, const std::vector<document>& particleDescriptor, BeanFactory& beanFactory);

        virtual void run() override;

    private:
        uint64_t emit(uint64_t tick);

    private:
        sp<Clock> _clock;
        sp<LayerContext> _layer_context;

        sp<Source> _source;

        std::vector<Particale> _particles;
        uint64_t _next_tick;

        friend class Emitter;
    };

private:
    void doActivate();

private:
    sp<Stub> _stub;
    sp<RenderController> _render_controller;

};

}

#endif
